/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : common.h
 * Creation Date : 06-11-2012
 * Last Modified : Wed 12 Dec 2012 08:25:26 PM EET
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


/* Operation Mode */
enum OPMODE { CONTINUOUS, CYCLIC, OPMODE_SIZE };
typedef enum OPMODE OPMODE;

#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define MAX(a,b)  ((a) > (b)) ? (a) : (b)

#include <stdlib.h>
#include <stdio.h>

struct time_struct {
    struct timeval latest_timestamp;
    struct timeval current_duration;
};

typedef struct time_struct time_struct;


void time_struct_init(time_struct ts);
void time_struct_set_timestamp(time_struct ts);
void time_struct_add_timestamp(time_struct ts);

struct Matrix {
    int N;
    double *A;
};

typedef struct Matrix Matrix;

Matrix *get_matrix(char *filename, int max_rank, OPMODE operation);
double **appoint_2D(double *A, int N, int M);
void fprint_matrix_2d(FILE *fp, int N, int M, double *A);
void print_matrix_2d(int N, int M, double *A);
double timer(void);
void usage(int argc, char **argv);
void *  get_propagation(int argc, char **argv);

void upper_triangularize(int N, double **Ap2D);

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
