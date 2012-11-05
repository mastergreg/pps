#!/bin/bash


NTHREADS=12


if [[ 'x'$1 == 'x' ]]; 
then
    echo "No arguements, simple run, for debug ./run.sh d"
    make -B
    mpirun -np ${NTHREADS} ./main.exec mat.txt
else
    if [[ 'x'$1 == 'xd' ]];
    then
        make -B DEBUG=y
        echo "Running with valgrind"
        MPIWRAP_DEBUG=warn \
            LD_PRELOAD=/usr/lib/valgrind/libmpiwrap-x86-linux.so \
            mpirun -np ${NTHREADS} valgrind --leak-check=full --log-file=val.out ./main.exec mat.txt
    else
        echo "Bad arguements, no run, for debug ./run.sh d"
    fi
fi
