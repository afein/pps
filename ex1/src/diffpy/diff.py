#!/usr/bin/env python3
# -*- coding: utf-8
#
#* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
# File Name : diff.py
# Creation Date : 06-11-2012
# Last Modified : Tue 06 Nov 2012 04:39:13 PM EET
# Created By : Greg Liras <gregliras@gmail.com>
#_._._._._._._._._._._._._._._._._._._._._.*/

from sys import argv

def help():
    print("""Usage: {0} <file1> <file2>""".format(argv[0]))
    exit(1)

def parse(f):
    return [map(float, line.split()) for line in f.readlines()[1:-1]]


def diff(d1,d2):
    diffed = []
    for l1,l2 in zip(d1,d2):
        diffed.append([x-y for (x,y) in zip(l1, l2)])
    return diffed
def main():
    if len(argv) != 3:
        help()
    else:
        f1 = open(argv[1], 'r')
        d1 = parse(f1)
        f1.close()
        f2 = open(argv[1], 'r')
        d2 = parse(f2)
        f2.close()

        diffed = diff(d1, d2)
        for i,vi in enumerate(diffed):
            for j,vj in enumerate(vi):
                if vj > 0:
                    print("<{0}> <{1}>: Failed with error value {2} on A[{3}][{4}]".format(argv[1], argv[2], vj, i, j))
                    exit(1)
        print("<{0}> <{1}> Pass :)".format(argv[1], argv[2]))
        exit(0)



if __name__=="__main__":
    main()

