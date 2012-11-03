#!/bin/bash
make
mpirun -np 4 ./main < mat.txt
