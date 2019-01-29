#! /bin/bash

if [ -f generate ]; then
	if [ "$#" == 1 ]; then
        ./generate "$1"
    else
        echo "Illegal number of parameters ($#)"
    fi
else
    echo "Executable not found, run compile.sh first"
fi
