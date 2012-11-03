#!/bin/bash
make
mpirun -np 6 ./main < mat.txt
