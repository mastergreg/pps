#!/usr/bin/env python
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : generate.py
# Creation Date : 30-10-2012
# Last Modified : Sun 11 Nov 2012 09:52:17 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from __future__ import print_function
from random import random
from sys import argv

def help():
    print("""Usage: {0} <matrix size> <output file>""".format(argv[0]))
    exit(1)
def main():
    if not len(argv) == 3:
        help()
    else:
        out = open(argv[2], "w")
        lim = int(argv[1])
        out.write("{0}\n".format(lim))
        for i in range(lim):
            for j in range(lim):
                out.write("{0}\t".format(100*random()))
            out.write("\n")
        out.close()

if __name__=="__main__":
    main()

