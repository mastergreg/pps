#!/usr/bin/env python
# -*- coding: utf-8
# #* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-. # File Name : parse_results.py # Creation Date : 13-12-2012
# Last Modified : Tue 08 Jan 2013 04:03:05 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from sys import argv
file_order = ['512', '1024', '2048']
def main():
    f = open(argv[1])
    data = map(lambda x: float(x.split(":")[1].strip()), f.readlines())
    f.close()
    current=0
    for i in range(len(file_order)):
        print "==============="
        print "File size:", file_order[i], "Run type: serial", "Time = ", data[current]
        for j in range(8):
            print "File size:", file_order[i], "NThreads:", int(data[current+1]), "Time = ", data[current+2]
            current = current + 2
        current = current + 1

if __name__=="__main__":
    main()

