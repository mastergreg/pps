#!/bin/bash
make
mpirun -np 6 ./main.exec < mat.txt
