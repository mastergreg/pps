#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : parse_results.py
# Creation Date : 13-12-2012
# Last Modified : Thu 13 Dec 2012 01:24:14 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from sys import argv
run_order = ['ptp-cont', 'ptp-cyc', 'coll-cont', 'coll-cyc']
file_order = ['2048', '4096', '6144']
def main():
    f = open(argv[1])
    data = map(lambda x: float(x.split(":")[2].strip()), f.readlines())
    f.close()
    for i in range(3):
        current_block=data[i*504:(i+1)*504]
        k=2
        start = 0
        for j in range(6):
            for l in range(4):
                maximum=max(current_block[start:start+k])
                start += k
                print "File size:", file_order[i], "NThreads:", k, "Run type:", run_order[l], "Comm Time = ", maximum
            k*=2

if __name__=="__main__":
    main()

