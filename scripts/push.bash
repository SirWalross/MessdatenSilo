#!/bin/bash

# wait until all other instances of the script are finished
while pidof  -x "python3 main.py">/dev/null; do
    # check every 60 seconds
    echo "python3 main.py is still running, checking in 60 seconds."
    sleep 60
done
sleep 10

# scripts/./write.bash

# sleep 10

# python3 measure.py

# sleep 10

echo -e "Starting python script:\n"
loop_count=1
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

git pull
git add .
git commit -m "New data" && git push --force