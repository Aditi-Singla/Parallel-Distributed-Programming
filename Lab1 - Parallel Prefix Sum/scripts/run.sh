#! /bin/bash

if [ -f par ]; then
	if [ "$#" == 2 ]; then
        ./par "$1" "$2"
    elif [ "$#" == 4 ] && [ "$3" == "--numThreads" ]; then
        ./par "$1" "$2" "$4"
    else
        echo "Illegal number of parameters ($#)"
    fi
else
    echo "Executable not found, run compile.sh first"
fi