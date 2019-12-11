#!/bin/bash
sudo perf record ./pivotter 2 3 1 data/it.cel.sets.p2.csv
sudo perf report