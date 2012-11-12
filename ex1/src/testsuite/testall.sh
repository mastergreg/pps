#!/bin/bash
set -e


genpathpath=../generator/generate.py
diffpath=../diffpy/diff.py
serialpath=../serial/main.exec
#testfilesSizes=(5 15 42 100 1000)
testfilesSizes=(100 1000)
#testfiles=(mat_5.txt mat_15.txt mat_42.txt mat_100.txt mat_1000.txt mat_10000.txt)
#MPItestfolders=( ../openmp/ ../mpi/ptp/continuous-single ../mpi/ptp/hybrid ../mpi/collective/hybrid ../mpi/collective/continuous-single)
OPENMPtestfolders=( ../openmp)
NTHREADS=2

nr=${#testfiles[@]}

for i in ${testfilesSizes[@]}
do
    testfiles[${i}]="mat_${i}.txt"
    ${genpathpath} ${i} "mat_${i}.txt"
done
for i in ${testfiles[@]}
do
    out="${j//\.\.\//}"
    serialfile="serial_${i%txt}out"
    ${serialpath} ${i} ${serialfile}
done
for j in ${MPItestfolders[@]}
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

for j in ${OPENMPtestfolders[@]}
do
    echo $j
    for i in ${testfiles[@]}
    do
        echo $i
        out="${j//\.\.\//}"
        outfile="${out//\//_}_${i%txt}out"
        serialfile="serial_${i%txt}out"
#        echo "mpirun -np ${NTHREADS} ${j}/main.exec ${i} ${outfile}"
        ${j}/main.exec ${i} ${outfile}
        ${diffpath} ${serialfile} ${outfile}
    done
done

