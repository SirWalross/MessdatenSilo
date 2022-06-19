#
# Example of how to profile a Python app with multiple processes
# by logging events and opening the resulting trace file in chrome://tracing.
#

# pip install multiprocessing_logging

from functools import wraps
import json
import logging
import os
import time
import threading
import traceback
from typing import Any
import serial
import serial.serialutil
import sys
import datetime
import yaml
from pathlib import Path

import numpy as np
from multiprocessing_logging import install_mp_handler

from main import logger, data_logger, fh, get_offset, setup_loggers

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
def read(connection: serial.Serial, data: np.ndarray, off: int):
    for i in range(4):
        recv = read_value(connection)
        data[i + off] += float(convert(recv))


@log_profile("write")
def write(connection: serial.Serial):
    connection.write(1)


@log_profile("offset")
def offset(connection: serial.Serial) -> int:
    return 0 if int(convert(connection.readline())) == 1.0 else 4


@log_profile("write_data")
def write_data(data: np.ndarray, n: int, factors: np.ndarray, offsets: np.ndarray):
    data_logger.info(",".join([f"{(value/n) * factors[i] - offsets[i]:.5f}" for i, value in enumerate(data)]) + f",{n}")
    logger.debug("Wrote data")


def convert(data) -> str:
    return str(data).replace("b'", "").replace("'", "").replace("\\r\\n", "")


@log_profile("get_data")
def get_data(con1: serial.Serial, con2: serial.Serial) -> np.ndarray:
    data = np.zeros((8,))
    try:
        for connection in [con1, con2]:
            write(connection)
            off = offset(connection)
            read(connection, data, off)

    except (TypeError, ValueError):
        # may occur if no data was read over serial
        logger.info(f"Didn't receive data from arduino", exc_info=True)
    return data


@log_profile("loop")
def loop(con1: serial.Serial, con2: serial.Serial, factors: np.ndarray, offsets: np.ndarray):
    last_write = time.time()
    delta_time = 30
    n = 0
    data = np.zeros((8,))

    while time.time() - last_write < delta_time:
        data += get_data(con1, con2)
        n += 1
    write_data(data, n, factors, offsets)


@log_profile("main")
def main(config: Any) -> None:
    print("Starting")
    Path(f"{Path(__file__).parent}/test_data").mkdir(parents=True, exist_ok=True)
    Path(f"{Path(__file__).parent}/test_logs").mkdir(parents=True, exist_ok=True)

    setup_loggers(config, "test_data", "test_logs")

    delta_time = config["Data"]["delta_time"]  # log averaged out data every n seconds
    end_time = datetime.datetime.combine(datetime.date.today(), datetime.time(1, 0, 0, 0))

    logger.warning("Starting")

    factors: np.ndarray = np.hstack((np.array(config["Data"]["factors"]), np.ones((4,))))
    offsets: np.ndarray = np.hstack((get_offset(), np.zeros((4,))))

    logger.info(
        f"Factors: {', '.join(f'{factor:.3f}' for factor in factors[:4])}, Offset: {', '.join(f'{offset:.3f}' for offset in offsets[:4])}"
    )

    with serial.Serial("/dev/ttyACM0", 9600, timeout=3) as con1, serial.Serial("/dev/ttyACM1", 9600, timeout=3) as con2:
        for _ in range(100):
            loop(con1, con2, factors, offsets)

    fh[0].doRollover() # rollover the current data log file
        
    logger.warning("Finished")


if __name__ == "__main__":
    main(yaml.safe_load(open(f"{Path(__file__).parent}/config.yml")))
    convert_log_to_trace("profiling.log", "profiling_trace.json")
