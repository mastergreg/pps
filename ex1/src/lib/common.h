/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : common.h
 * Creation Date : 06-11-2012
 * Last Modified : Thu 29 Nov 2012 03:01:56 PM EET
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




#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define MAX(a,b)  ((a) > (b)) ? (a) : (b)

#include <stdlib.h>
#include <stdio.h>

struct Matrix {
    int N;
    double *A;
};

typedef struct Matrix Matrix;

Matrix *get_matrix(char *filename, int max_rank);
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
void gather_to_root_cyclic(double **Ap2D, int max_rank, int rank, int root, double **A2D, int N, int M);
void get_counts(int max_rank, int N, int *counts);
void get_displs(int *counts, int max_rank, int *displs);
#endif /* USE_MPI */

#endif /* COMMON_H */
