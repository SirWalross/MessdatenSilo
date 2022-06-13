#!/bin/bash

cd `dirname "$0"`/..
echo -e "\n\n#####################################################################"
echo $( date '+%F %H:%M:%S' ) | tee -a bash.log
echo "" | tee -a bash.log
scripts/./push.bash 2>&1 | tee -a bash.log
echo "" | tee -a bash.log