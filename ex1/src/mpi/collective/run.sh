#!/bin/bash


NTHREADS=2
FILENAME=mat.txt


if [[ 'x'$1 == 'x' ]]; 
then
    echo "No arguements, simple run, for debug ./run.sh d, for valgrind use v"
    make -B
    mpirun -np ${NTHREADS} ./main.exec ${FILENAME}
else
    if [[ 'x'$1 == 'xv' ]];
    then
        make -B DEBUG=y
        echo "Running with valgrind"
        MPIWRAP_DEBUG=warn \
            LD_PRELOAD=/usr/lib/valgrind/libmpiwrap-x86-linux.so \
            mpirun -np ${NTHREADS} valgrind --show-reachable=yes --leak-check=full --log-file=val.out ./main.exec ${FILENAME}
    else
        if [[ 'x'$1 == 'xd' ]];
        then
            make -B DEBUG=y
            echo "Running with debug"
            mpirun -np ${NTHREADS} ./main.exec ${FILENAME}
        else
            echo "Bad arguements, no run, for debug ./run.sh d"
        fi
    fi
fi
