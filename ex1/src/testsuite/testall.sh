#!/bin/bash
set -e


genpathpath=../generator/generate.exec
diffpath=../diffpy/diff.py
serialpath=../serial/main.exec
#testfilesSizes=(5 15 42 100 1000)
testfilesSizes=(100 1000 2000)
#testfiles=(mat_5.txt mat_15.txt mat_42.txt mat_100.txt mat_1000.txt mat_10000.txt)
MPItestfolders=(../mpi/ptp/continuous-single ../mpi/ptp/hybrid ../mpi/collective/hybrid ../mpi/collective/continuous-single)
OPENMPtestfolders=( ../openmp)
NTHREADS=2

nr=${#testfiles[@]}

for i in ${testfilesSizes[@]}
do
    testfiles[${i}]="mat_${i}.txt"
done

if [ 'x'$1 == 'xgen' ];
then
    for i in ${testfilesSizes[@]}
    do
        echo "Generating mat_${i}.txt"
        ${genpathpath} ${i} "mat_${i}.txt"
    done
else
    echo "Not generating files (takes too long every time right?)"
fi

# Serial execution
for i in ${testfiles[@]}
do
    out="${j//\.\.\//}"
    serialfile="serial_${i%txt}out"
    ${serialpath} ${i} ${serialfile}
done

# Parallel execution using MPI
for j in ${MPItestfolders[@]}
do
    echo $j
    for i in ${testfiles[@]}
    do
        echo $i
        out="${j//\.\.\//}"
        outfile="${out//\//_}_${i%txt}out"
        serialfile="serial_${i%txt}out"
        mpirun -np ${NTHREADS} ${j}/main.exec ${i} ${outfile}
        ${diffpath} ${serialfile} ${outfile}
    done
done

# Parallel execution using OpenMP
for j in ${OPENMPtestfolders[@]}
do
    echo $j
    for i in ${testfiles[@]}
    do
        echo $i
        out="${j//\.\.\//}"
        outfile="${out//\//_}_${i%txt}out"
        serialfile="serial_${i%txt}out"
        ${j}/main.exec ${i} ${outfile}
        ${diffpath} ${serialfile} ${outfile}
    done
done
