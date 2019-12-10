import os
import sys
import time

def datapath(name):
    return os.path.sep.join([os.path.abspath(os.path.dirname(__file__)), "..", "data", name])

with open(datapath("output.csv"), "w") as fo:
    fo.write("")

with open(datapath("iris.csv")) as fi:
    for line in fi:
        with open(datapath("output.csv"), "a") as fo:
            fo.write(line)
        time.sleep(0.5)
