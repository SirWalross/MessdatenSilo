"""Convert csv data into mat files to read into matlab.

Combines the files from one week and converts it into a single '.mat' file.
"""

from datetime import datetime, timedelta
from pathlib import Path
import glob
from time import strptime
from typing import Dict
import shutil
import numpy as np
import scipy.io

data: Dict[str, np.ndarray] = {}  # a dict of numpy array's, one for each week

header = ["Timestamp"] + [f"dms{i+1}" for i in range(4)] + [f"temp{i+1}" for i in range(4)] + ["n"]
start_time: float = 0


def convertfunc(x: bytes) -> float:
    global start_time
    if start_time == 0:
        start_time = datetime.strptime(x.decode("utf-8"), "%Y-%m-%d %H:%M:%S").timestamp()
    return datetime.strptime(x.decode("utf-8"), "%Y-%m-%d %H:%M:%S").timestamp() - start_time


files = sorted(glob.glob(str(Path.joinpath(Path(__file__).parent, "data", "log.*.log"))))

for file in files:
    date = datetime(*strptime(Path(file).suffixes[0][1:].split("_")[0], "%Y-%m-%d")[:6]) - timedelta(days=2)
    week_start = (date - timedelta(days=date.weekday()) + timedelta(days=2)).strftime("%Y-%m-%d")

    csv_data = np.genfromtxt(file, skip_header=1, delimiter=",", converters={0: convertfunc})

    # either add the data from one day to already existing entry for that week or create new entry
    if week_start in data.keys():
        data[week_start] = np.vstack((data[week_start], csv_data))
    else:
        data[week_start] = csv_data

Path(f"{Path(__file__).parent}/out").mkdir(parents=True, exist_ok=True)

# save each week as seperate '.mat' file in 'out' folder
for week_start, arr in data.items():
    scipy.io.savemat(
        f"{Path(__file__).parent}/out/data.{week_start}.mat",
        mdict={name: column for name, column in zip(header, np.split(arr, arr.shape[1], axis=1))},
    )

# zip 'out' folder
shutil.make_archive("data", "zip", "out")

# Update CHANGELOG.md
with open(str(Path.joinpath(Path(__file__).parent, "CHANGELOG.md")), "w+") as f:
    f.write("## Messdaten vom Silo von den Wochen:\n" + "\n".join(["- " + key for key in data.keys()]))

