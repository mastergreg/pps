#!/bin/bash
set -e


genpathpath=../generator/generate.py
diffpath=../diffpy/diff.py
serialpath=../serial/main.exec
#testfilesSizes=(5 15 42 100 1000 10000)
testfilesSizes=(5 15 42 100 1000)
#testfiles=(mat_5.txt mat_15.txt mat_42.txt mat_100.txt mat_1000.txt mat_10000.txt)
testfiles=(mat_5.txt mat_15.txt mat_42.txt mat_100.txt mat_1000.txt)
testfolders=(../mpi/collective/cyclic ../mpi/collective/continuous)
NTHREADS=2

nr=${#testfiles[@]}

for i in ${testfilesSizes[@]}
do
    ${genpathpath} ${i} "mat_${i}.txt"
done
for i in ${testfiles[@]}
do
    out="${j//\.\.\//}"
    serialfile="serial_${i%txt}out"
    ${serialpath} ${i} ${serialfile}
done
for j in ${testfolders[@]}
do
    echo $j
    for i in ${testfiles[@]}
    do
        echo $i
        out="${j//\.\.\//}"
        outfile="${out//\//_}_${i%txt}out"
        serialfile="serial_${i%txt}out"
#        echo "mpirun -np ${NTHREADS} ${j}/main.exec ${i} ${outfile}"
        mpirun -np ${NTHREADS} ${j}/main.exec ${i} ${outfile}
        ${diffpath} ${serialfile} ${outfile}
    done
done

