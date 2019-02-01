#! /bin/bash

if [ -f hypersort ]; then
	if [ "$#" == 4 ] && [ "$3" == "--numProcess" ]; then
        mpirun -np "$4" ./hypersort "$1" "$2"
    else
        echo "Illegal number of parameters ($#)"
    fi
else
    echo "Executable not found, run compile.sh first"
fi
