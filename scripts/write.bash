#!/bin/bash

cd `dirname "$0"`/../sketches

fqbn=arduino:avr:uno
core=arduino:avr

id=$(python3 ../scripts/serial_id.py)

if [[ $id == "1" ]]
then
    serial_dms=/dev/ttyACM0
    serial_temp=/dev/ttyACM1
elif [[ $id == "2" ]]
then
    serial_temp=/dev/ttyACM0
    serial_dms=/dev/ttyACM1
else
    echo -e "\x1b[31mError: Something went wrong.\x1b[0m"
    echo -e "Exiting"
    exit 1
fi

echo -e "Info: DMS arduino: $serial_dms, Temp arduino: $serial_temp"

echo "Checking connected devices..."

device_list=$(arduino-cli board list)
connected_devices=$(echo "$device_list" | grep "^\(\($serial_dms\)\|\($serial_temp\)\).*$fqbn.*$core" | wc -l)

if [[ $connected_devices != "2" ]] 
then
    echo -e "\x1b[31mError: not all arduino devices are connected, only found: $connected_devices.\x1b[0m"
    echo -e "Connected devices are:\n$device_list"
    echo -e "Exiting."
    exit 1
fi

echo "Checking installed cores..."

core_list=$(arduino-cli core list)
core_list_installed=$(echo "$core_list" | grep "^$core")

if [[ -z $core_list_installed ]]
then
    echo -e "\x1b[33mWarning: Core $core for board $fqbn is not installed.\x1b[0m"

    while true; do
        read -p "Would you like to install the core? [Y/n]: " yn
        case $yn in
            [Yy]*) break  ;;  
            [Nn]*) echo "Aborted" ; exit 1 ;;
        esac
    done

    echo -e "Installing..."

    arduino-cli core install $core

    core_list=$(arduino-cli core list)
    core_list_installed=$(echo "$core_list" | grep "^$core")

    if [[ -z $core_list_installed ]]
    then
        echo -e "\x1b[31mError: failed to install core $core for board $fqbn.\x1b[0m\n"
        echo -e "Installed cores are:\n$core_list"
        echo -e "Exiting."
        exit 1
    fi
fi

echo "Compiling sketches..."

compile=$(arduino-cli compile --fqbn $fqbn DmsMessung)
if [[ $? -gt 0 ]]
then
    echo -e "\x1b[31mError: Compilation of DmsMessung failed:\x1b[0m"
    echo -e "$compile"
    echo -e "Exiting."
    exit 1
fi

compile=$(arduino-cli compile --fqbn $fqbn Temperaturmessung)
if [[ $? -gt 0 ]]
then
    echo -e "\x1b[31mError: Compilation of Temperaturmessung failed:\x1b[0m"
    echo -e "$compile"
    echo -e "Exiting."
    exit 1
fi

echo "Uploading sketches..."

upload=$(arduino-cli upload --port $serial_dms --fqbn $fqbn DmsMessung)
if [[ $? -gt 0 ]]
then
    echo -e "\x1b[31mError: Upload of DmsMessung failed:\x1b[0m"
    echo -e "$upload"
    echo -e "Exiting."
    exit 1
fi

upload=$(arduino-cli upload --port $serial_temp --fqbn $fqbn Temperaturmessung)
if [[ $? -gt 0 ]]
then
    echo -e "\x1b[31mError: Upload of Temperaturmessung failed:\x1b[0m"
    echo -e "$upload"
    echo -e "Exiting."
    exit 1
fi

echo "Finished"
