import os
import sys
import time

if len(sys.argv) < 2:
    print("Supply an input file name")
    sys.exit(1)

infile = sys.argv[1]

def datapath(name):
    return os.path.sep.join([os.path.abspath(os.path.dirname(__file__)), "..", "data", name])

with open(datapath("output.csv"), "w") as fo:
    fo.write("")

with open(infile) as fi:
    for line in fi:
        with open(datapath("output.csv"), "a") as fo:
            fo.write(line)
        time.sleep(0.1)
