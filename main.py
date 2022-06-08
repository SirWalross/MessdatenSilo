import datetime
import yaml
import time
from typing import Any, List, Optional
import serial
from pathlib import Path
import logging
import logging.handlers
import sys
import glob

import numpy as np

logger: logging.Logger
data_logger: logging.Logger
fh: List[logging.FileHandler] = []


class TimedRotatingFileHandlerWithHeader(logging.handlers.TimedRotatingFileHandler):
    def __init__(
        self, filename: str, when: str = "s", interval: int = 2, backupCount: int = 0, atTime: Optional[datetime.time] = None, header=""
    ):
        self._header = header
        self.first = True
        super().__init__(filename, when=when, interval=interval, backupCount=backupCount, atTime=atTime)
        self.namer = self._namer

    @staticmethod
    def _namer(filename: str) -> str:
        return Path.joinpath(Path(filename).parent, f"log{Path(filename).suffix}.log").__str__()

    def emit(self, record):
        try:
            if self.shouldRollover(record) or self.first:
                if self.shouldRollover(record):
                    self.doRollover()
                stream = self.stream
                if self._header:
                    stream.write(self._header + self.terminator)
            else:
                stream = self.stream
            msg = self.format(record)
            stream.write(msg + self.terminator)
            self.first = False
            self.flush()
        except RecursionError:
            raise
        except Exception:
            self.handleError(record)


def handle_exception(exc_type, exc_value, exc_traceback):
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_traceback)
        return

    logger.error("Uncaught exception", exc_info=(exc_type, exc_value, exc_traceback))


def convert(data) -> str:
    return str(data).replace("b'", "").replace("'", "").replace("\\r\\n", "")


def get_offset() -> np.ndarray:
    files = sorted(glob.glob(str(Path.joinpath(Path(__file__).parent, "data", "log.*.log"))))

    if files:
        for file in files[::-1]:
            with open(file, "r") as f:
                lines = f.readlines()
                if len(lines) < 2:
                    # file didn't contain any data
                    continue
                difference = datetime.datetime.now() - datetime.datetime.strptime(lines[-1].split(",")[0], "%Y-%m-%d %H:%M:%S")
                if difference > datetime.timedelta(seconds=60 * 5):
                    logger.warning(
                        f"Time difference between last logged value and current time is very large: {difference.total_seconds():.0f}s."
                    )
                return np.array([float(num) for num in lines[-1].split(",")[1:5]])
    # didn't find any old files, so no offset
    logger.warning(f"Didn't find any old offsets, so starting at 0.")
    return np.array([0, 0, 0, 0])


sys.excepthook = handle_exception


def setup_loggers(config: Any) -> None:
    global data_logger, logger, fh
    data_logger = logging.getLogger("data_logger")
    data_logger.setLevel(logging.DEBUG)
    bf = logging.Formatter("{asctime},{message}", datefmt=r"%Y-%m-%d %H:%M:%S", style="{")
    fh.append(
        TimedRotatingFileHandlerWithHeader(
            header=f"Timestamp,{','.join([f'dms{i+1}' for i in range(4)])},{','.join([f'temp{i+1}' for i in range(4)])},n",
            filename=f"{Path(__file__).parent}/data/{config['DataLogger']['filename']}",
            when='h',
            interval=25,
            backupCount=config["DataLogger"]["backupCount"],
        )
    )
    fh.append(logging.StreamHandler(sys.stdout))

    for i in range(2):
        fh[i].setLevel(getattr(logging, config["DataLogger"]["levels"][i]))
        fh[i].setFormatter(bf)
        data_logger.addHandler(fh[i])

    logger = logging.getLogger("main_logger")
    logger.setLevel(logging.INFO)
    bf = logging.Formatter("{asctime}, {levelname}, [{name}.{funcName}:{lineno}]\t{message}", datefmt=r"%Y-%m-%d %H:%M:%S", style="{")
    fh.append(
        logging.handlers.RotatingFileHandler(
            filename=f"{Path(__file__).parent}/logs/{config['InfoLogger']['filename']}",
            maxBytes=config["InfoLogger"]["maxBytes"],
            backupCount=config["InfoLogger"]["backupCount"],
        )
    )
    fh.append(logging.StreamHandler(sys.stdout))

    for i in range(2):
        fh[i + 2].setLevel(getattr(logging, config["InfoLogger"]["levels"][i]))
        fh[i + 2].setFormatter(bf)
        logger.addHandler(fh[i + 2])


def main(config: Any) -> None:
    Path(f"{Path(__file__).parent}/data").mkdir(parents=True, exist_ok=True)
    Path(f"{Path(__file__).parent}/logs").mkdir(parents=True, exist_ok=True)

    setup_loggers(config)

    delta_time = config["Data"]["delta_time"]  # log averaged out data every n seconds
    end_time = datetime.datetime.combine(datetime.date.today(), datetime.time(23, 59, 59, 999999))  # end at 23:59:59 of the day

    logger.warning("Starting")

    factors = np.hstack((np.array(config["Data"]["factors"]), np.ones((4,))))
    offsets = np.hstack((get_offset(), np.zeros((4,))))

    logger.info(
        f"Factors: {', '.join(f'{factor:.3f}' for factor in factors[:4])}, Offset: {', '.join(f'{offset:.3f}' for offset in offsets[:4])}"
    )

    with serial.Serial("/dev/ttyACM0", 9600, timeout=3) as con1, serial.Serial("/dev/ttyACM1", 9600, timeout=3) as con2:
        logger.warning("Connected to serial ports")
        last_write = time.time()
        data = np.zeros((8,))
        n = 0
        recv1, recv2 = None, None
        off1, off2 = None, None
        while datetime.datetime.now() - datetime.timedelta(seconds=delta_time) < end_time:
            con1.write(1)
            con2.write(2)

            try:
                new_data = data.copy()

                # offsets for writing data of each arduino in correct column
                off1 = 0 if int(convert(con1.readline())) == 1.0 else 4
                off2 = 4 if int(convert(con2.readline())) == 2.0 else 0

                # read data
                for i in range(4):
                    recv1 = con1.readline()
                    recv2 = con2.readline()
                    new_data[i + off1] += float(convert(recv1))
                    new_data[i + off2] += float(convert(recv2))
                    recv1, recv2 = None, None
                n += 1
                off1, off2 = None, None
                data = new_data
            except (TypeError, ValueError):
                # may occur if no data was read over serial
                logger.info(f"Didn't receive data from arduino, off1: {off1}, off2: {off2}, recv1: {recv1}, recv2: {recv2}")

            if time.time() - last_write > delta_time:
                # write data
                data_logger.info(",".join([f"{value/n * factors + offsets:.5f}" for value in data]) + f",{n}")
                logger.debug("Wrote data")
                n = 0
                data = np.zeros((8,))
                last_write = time.time()

    fh[0].doRollover()

    logger.warning("Finished")


if __name__ == "__main__":
    main(yaml.safe_load(open(f"{Path(__file__).parent}/config.yml")))
