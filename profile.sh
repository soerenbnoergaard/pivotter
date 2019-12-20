#!/bin/bash
sudo perf record -g ./pivotter -x2 -y3 -u1 data/it.cel.sets.p2.csv
sudo perf report
