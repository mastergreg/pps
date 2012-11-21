/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : common.h
 * Creation Date : 06-11-2012
 * Last Modified : Thu 22 Nov 2012 01:22:41 AM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#ifndef DEBUG_FUNC
#define DEBUG_FUNC

#if main_DEBUG
#define debug(fmt,arg...)     fprintf(stdout, "%s: " fmt, __func__ , ##arg)
#else
#define debug(fmt,arg...)     do { } while(0)
#endif /* main_DEBUG */

#endif /* DEBUG_FUNC */

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdio.h>

struct Matrix {
    int N;
    double *A;
};

typedef struct Matrix Matrix;

Matrix *get_matrix(char *filename);
double **appoint_2D(double *A, int N, int M);
void fprint_matrix_2d(FILE *fp, int N, int M, double *A);
void print_matrix_2d(int N, int M, double *A);
double timer(void);
void usage(int argc, char **argv);


#ifdef USE_MPI /* USE_MPI */
#include <mpi.h>
void propagate_with_send(void *buffer, int count , MPI_Datatype datatype, \
        int root, MPI_Comm comm);
void propagate_with_flooding(void *buffer, int count , MPI_Datatype datatype, \
        int root, MPI_Comm comm);
#endif /* USE_MPI */

#endif /* COMMON_H */
