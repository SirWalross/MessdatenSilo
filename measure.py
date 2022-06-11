#
# Example of how to profile a Python app with multiple processes
# by logging events and opening the resulting trace file in chrome://tracing.
#

# pip install multiprocessing_logging

from functools import wraps
import json
import logging
from multiprocessing import Pool
import os
import random
import time
import threading
import serial
import datetime

from multiprocessing_logging import install_mp_handler

# we want to be able to log from multiple processes
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger()
install_mp_handler()

# separate logger that only stores events into a file
prof_logger = logging.getLogger("profiling")
# not not propagate to root logger, we want to store this separately
prof_logger.propagate = False
handler = logging.FileHandler("profiling.log", "w+")
handler.setFormatter(logging.Formatter("%(message)s"))
prof_logger.addHandler(handler)
install_mp_handler(prof_logger)


def log_event(**kwargs):
    prof_logger.debug(json.dumps(kwargs))


def time_usec() -> int:
    return int(round(1e6 * time.time()))


base_info = {
    "pid": os.getpid(),
    "tid": threading.current_thread().ident,
}


def log_profile(category: str = None):
    def decorator(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            # format compatible with chrome://tracing
            # info: https://www.gamasutra.com/view/news/176420/Indepth_Using_Chrometracing_to_view_your_inline_profiling_data.php

            args_str = {i: f"{arg}" for i, arg in enumerate(args)}

            start_time = time_usec()
            log_event(ph="B", ts=start_time, name=f.__name__, cat=category, args=args_str, **base_info)

            result = f(*args, **kwargs)

            end_time = time_usec()
            duration = end_time - start_time
            # TODO: duration could possibly be computed afterwards (if we can pair the events correctly)
            log_event(ph="E", ts=end_time, duration=duration, name=f.__name__, cat=category, args=args_str, **base_info)

            return result

        return wrapper

    return decorator


def convert_log_to_trace(log_file, trace_file):
    with open(trace_file, "wt") as output, open(log_file, "rt") as input:
        events = [json.loads(line) for line in input]
        json.dump({"traceEvents": events}, output)


@log_profile("read_value")
def read_value(connection: serial.Serial) -> bytes:
    return connection.readline()


@log_profile("read")
def read(connection: serial.Serial):
    for _ in range(4):
        recv1 = read_value(connection)
        float(convert(recv1))


@log_profile("write")
def write(connection: serial.Serial):
    connection.write(1)


@log_profile("offset")
def offset(connection: serial.Serial):
    return 0 if int(convert(connection.readline())) == 1.0 else 4


@log_profile("write_data")
def write_data():
    print("writing data")
    time.sleep(10e-3)


def convert(data) -> str:
    return str(data).replace("b'", "").replace("'", "").replace("\\r\\n", "")


@log_profile("get_data")
def get_data(con1: serial.Serial, con2: serial.Serial):
    try:
        for connection in [con1, con2]:
            write(connection)
            offset(connection)
            read(connection)

    except (TypeError, ValueError):
        # may occur if no data was read over serial
        log_event(ph="I", ts=time_usec(), name="NoData", cat="NoData", **base_info)
        print("Didn't receive data from arduino")


@log_profile("loop")
def loop(con1: serial.Serial, con2: serial.Serial):
    last_write = time.time()
    delta_time = 30
    while time.time() - last_write < delta_time:
        get_data(con1, con2)
    write_data()


def main() -> None:
    with serial.Serial("/dev/ttyACM0", 9600, timeout=3) as con1, serial.Serial("/dev/ttyACM1", 9600, timeout=3) as con2:
        for _ in range(50):
            loop(con1, con2)


convert_log_to_trace("profiling.log", "profiling_trace.json")
