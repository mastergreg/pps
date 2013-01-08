#!/bin/bash

data="12-13-04-08-42.out"


speedy="./speedy.py"

totals_infile="results_for_totals"
totals_outfile="total_times"
totals_parser="./parse_results_totals.py"

comm_infile="results_for_comm"
comm_outfile="comm_times"
comm_parser="./parse_results_comm.py"

run_types=('ptp-cont' 'ptp-cyc' 'coll-cont' 'coll-cyc')

grep "Comm" "${data}" > "${comm_infile}"
grep "Calc" "${data}" > "${totals_infile}"

${totals_parser} ${totals_infile} > ${totals_outfile}
${comm_parser} ${comm_infile} > ${comm_outfile}


grep 'serial' "${totals_outfile}" > "${totals_outfile}_serial"
egrep 'NThreads: (8|16|32|64)' "${comm_outfile}" > "${comm_outfile}_8_16_32_64"
egrep 'NThreads: (8|16|32|64)' "${totals_outfile}" > "${totals_outfile}_8_16_32_64"

for size in 2048 4096 6144
do
    grep "${size}" "${comm_outfile}_8_16_32_64" > "${size}_${comm_outfile}_8_16_32_64"
    grep "${size}" "${totals_outfile}_8_16_32_64" > "${size}_${totals_outfile}_8_16_32_64"
    grep "${size}" "${totals_outfile}" | grep -v "serial" > "${size}_${totals_outfile}"
    grep "${size}" "${totals_outfile}_serial" > "${size}_${totals_outfile}_serial"

    ${speedy} "${size}_${totals_outfile}" "${size}_${totals_outfile}_serial" > "${size}_${totals_outfile}_speedup"

    for t in ${run_types[@]}
    do
        grep $t "${size}_${totals_outfile}_speedup" > "${size}_${t}_${totals_outfile}_speedup"
        grep $t "${size}_${totals_outfile}_8_16_32_64" | awk '{print $5 " " $12}' > ${size}_${t}_${totals_outfile}
        grep $t "${size}_${comm_outfile}_8_16_32_64" | awk '{print $5 " " $12}' > ${size}_${t}_${comm_outfile}
    done




    gnuplot << EOF
    set terminal png size 1024, 768
    set output "${size}_speedups.png"
    set title "${size} speedups"
    set key right outside top
    set grid
    set xlabel "MPI Threads"
    set ylabel "Speedup"
    plot  "${size}_${run_types[0]}_${totals_outfile}_speedup" title "${run_types[0]}" with lines, \
          "${size}_${run_types[1]}_${totals_outfile}_speedup" title "${run_types[1]}" with lines, \
          "${size}_${run_types[2]}_${totals_outfile}_speedup" title "${run_types[2]}" with lines, \
          "${size}_${run_types[3]}_${totals_outfile}_speedup" title "${run_types[3]}" with lines
EOF
done

yrange[2048]=3
yrange[4096]=20
yrange[6144]=65

for size in 2048 4096 6144
do

    gnuplot << EOF
    set terminal png size 1024, 768
    set output "${size}_times.png"
    set title "${size} times"
    set ylabel "Times"
    set boxwidth 0.5
    set grid
    set style data histograms
    set style histogram cluster gap 0
    set style fill solid border -1
    set multiplot layout 1,4
    set yrange[0:${yrange[${size}]}]
    set title "ptp-cont"
    plot "${size}_${run_types[0]}_${totals_outfile}" using 2:xtic(1) lt 1 title "Total" with boxes, \
         "${size}_${run_types[0]}_${comm_outfile}" using 2:xtic(1) lt 3 title "Comm" with boxes
    unset ylabel
    set title "ptp-cyc"
    plot "${size}_${run_types[1]}_${totals_outfile}" using 2:xtic(1) lt 1 notitle with boxes, \
                        "${size}_${run_types[1]}_${comm_outfile}" using 2:xtic(1) lt 3 notitle with boxes
    set title "coll-cont"
    plot "${size}_${run_types[2]}_${totals_outfile}" using 2:xtic(1) lt 1 notitle with boxes, \
                        "${size}_${run_types[2]}_${comm_outfile}" using 2:xtic(1) lt 3 notitle with boxes
    set title "coll-cyc"
    plot "${size}_${run_types[3]}_${totals_outfile}" using 2:xtic(1) lt 1 notitle with boxes, \
                        "${size}_${run_types[3]}_${comm_outfile}" using 2:xtic(1) lt 3 notitle with boxes
    unset multiplot
    set key 
EOF

done
