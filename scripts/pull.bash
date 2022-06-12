#!/bin/bash

# wait until all other instances of the script are finished
while pidof -o %PPID -x "pull.bash">/dev/null; do
    # check every 60 seconds
    sleep 60
done

cd `dirname "$0"`/..
echo -e "\n\n#####################################################################"
echo $( date '+%F %H:%M:%S' ) | tee -a bash.log
git pull 2>&1 | tee -a bash.log
echo "" | tee -a bash.log
scripts/./push.bash 2>&1 | tee -a bash.log
echo "" | tee -a bash.log