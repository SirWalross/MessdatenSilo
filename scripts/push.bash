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

echo -e "Starting python script:\n"
if python3 main.py then
    rm data/data
fi
echo -e "\nPython script ended"

git pull

last_commit_name = $(git log -1 --pretty=%B | cat)

if [[ $string == *"New data"* ]]; then
    # check if the last commit was "New data"
    # if yes overwrite the commit
    git reset --soft HEAD~1
fi

git add .
git commit -m "New data" && git push --force