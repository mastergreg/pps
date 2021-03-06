#!/bin/bash
set -e


NTHREADS=2
FILENAME=mat5.txt
OUTFILE="${FILENAME%txt}out"


if [[ 'x'$1 == 'x' ]]; 
then
    echo "No arguements, simple run, for debug ./run.sh d, for valgrind use v"
    make -B
    mpirun -np ${NTHREADS} ./main.exec ${FILENAME} ${OUTFILE}
else
    if [[ 'x'$1 == 'xv' ]];
    then
        make -B DEBUG=y
        echo "Running with valgrind"
        MPIWRAP_DEBUG=err \
            LD_PRELOAD=/usr/lib/valgrind/libmpiwrap-x86-linux.so \
            mpirun -np ${NTHREADS} \
            valgrind --suppressions=val.supp --track-origins=yes --show-reachable=yes --leak-check=full --log-file=val.out \
            ./main.exec ${FILENAME} ${OUTFILE} 1
    else
        if [[ 'x'$1 == 'xd' ]];
        then
            make -B DEBUG=y
            echo "Running with debug"
            mpirun -np ${NTHREADS} ./main.exec ${FILENAME} ${OUTFILE} 1
        else
            echo "Bad arguements, no run, for debug ./run.sh d"
        fi
    fi
fi
