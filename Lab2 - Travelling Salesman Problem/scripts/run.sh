#! /bin/bash

if [ -f tsp ]; then
	if [ "$#" == 4 ] && [ "$3" == "--numThreads" ]; then
        ./tsp "$1" "$2" "$4"
    else
        echo "Illegal number of parameters ($#)"
    fi
else
    echo "Executable not found, run compile.sh first"
fi