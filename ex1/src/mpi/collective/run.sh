#!/bin/bash
make
mpirun -np 5 ./main.exec < mat.txt
