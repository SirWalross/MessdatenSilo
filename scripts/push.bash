#!/bin/bash

ls -la .
ls -la data
ls -la logs
ps aux

scripts/./write.bash

pip3 install multiprocessing_logging
python3 measure.py

git add .
git commit -m "New data" && git push --force
echo -e "Starting python script:\n"
loop_count = 1
while : ; do
    python3 main.py
    return_code=$?
    if [[ "$return_code" -eq 0 ]]; then
        echo -e "\nPython script ended"
        break
    fi
    if [[ "$loop_count" -eq 10 ]]; then
        echo -e "\nPython script failed too many times, trying again tomorrow"
        break
    fi
    echo -e "Ended with error, waiting and trying again"
    loop_count=$((loop_count + 1))
    sleep $((loop_count * 60))
done