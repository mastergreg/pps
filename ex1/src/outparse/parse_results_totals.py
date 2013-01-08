#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : parse_results.py
# Creation Date : 13-12-2012
# Last Modified : Thu 13 Dec 2012 02:28:16 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from sys import argv
run_order = ['ptp-cont', 'ptp-cyc', 'coll-cont', 'coll-cyc']
file_order = ['2048', '4096', '6144']
def main():
    f = open(argv[1])
    data = map(lambda x: float(x.split(":")[1].strip()), f.readlines())
    f.close()
    for i in range(3):
        print "File size:", file_order[i], "Run type: serial", "Total Time = ", data[i*25]
        current_block=data[i*25+1:(i+1)*25]
        for j in range(6):
            for l in range(4):
                print "File size:", file_order[i], "NThreads:", 2**(j+1), "Run type:", run_order[l],  "Total Time = ", current_block[j*4+l]

if __name__=="__main__":
    main()

