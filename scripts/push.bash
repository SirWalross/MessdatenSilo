#!/bin/bash

# wait until all other instances of the script are finished
while pidof  -x "python3 main.py">/dev/null; do
    # check every 60 seconds
    echo "python3 main.py is still running, checking in 60 seconds."
    sleep 60
done
sleep 10

scripts/./write.bash

sleep 10

python3 measure.py

sleep 10

echo -e "Starting python script:\n"
python3 main.py
echo -e "\nPython script ended"

git pull
git add .
git commit -m "New data" && git push --force