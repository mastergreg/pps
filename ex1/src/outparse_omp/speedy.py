#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : speedy.py
# Creation Date : 13-12-2012
# Last Modified : Tue 08 Jan 2013 03:58:22 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/


from sys import argv,stderr

threads=[1, 2, 3, 4, 5, 6, 7, 8]
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

    for s,t in zip(speedups,threads):
        print t, s

if __name__=="__main__":
    main()

