#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : speedy.py
# Creation Date : 13-12-2012
# Last Modified : Thu 13 Dec 2012 02:23:23 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/


from sys import argv,stderr

run_order = ['ptp-cont', 'ptp-cyc', 'coll-cont', 'coll-cyc']
threads=[2, 4, 8, 16, 32, 64]
def spdup(current, base):
    return base/current


def main():
    f = open(argv[1])
    data = map(lambda x: float(x.split("=")[1]), f.readlines())
    f.close()
    f = open(argv[2])
    base =  map(lambda x: float(x.split("=")[1]), f.readlines())[0]
    f.close()
    speedups = map(lambda x: spdup(x, base), data)
    t = 0

    for s in speedups:
        print threads[t/4], s, "Run type:", run_order[t%4]
        t += 1

if __name__=="__main__":
    main()

