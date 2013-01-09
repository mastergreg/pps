#!/bin/bash

data="openmp_results"

outfile="openmp_times"
parser="./parse_results_omp.py"
speedy="./speedy.py"

${parser} ${data} > ${outfile}

for size in 512 1024 2048
do
    grep "${size}" "${outfile}" | grep -v "serial" > "${size}_${outfile}"
    grep "${size}" "${outfile}" | grep "serial" > "${size}_${outfile}_serial"

    ${speedy} "${size}_${outfile}" "${size}_${outfile}_serial"  > "${size}_${outfile}_speedup"

    gnuplot << EOF
    set terminal png size 1024, 768
    set output "${size}_speedup.png"
    set title "${size} speedup"
    set key right outside top
    set grid
    set xlabel "OPENMP Threads"
    set ylabel "Speedup"
    plot  "${size}_${outfile}_speedup" title "${size}" with lines
EOF
done
