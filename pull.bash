#!/bin/bash

cd `dirname "$0"`
echo $( date '+%F %H:%M:%S' ) | tee -a bash.log
git pull 2>&1 | tee -a bash.log
echo "" | tee -a bash.log
./push.bash 2>&1 | tee -a bash.log
echo "" | tee -a bash.log