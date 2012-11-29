/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : common.c
 * Creation Date : 06-11-2012
 * Last Modified : Thu 29 Nov 2012 03:18:11 PM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include "common.h"
#include <sys/time.h>
#include <string.h>




static double *allocate_2d(int N, int M)
{
    double *A;
    A = malloc(N * M * sizeof(double));
    return A;
}

static double *allocate_2d_with_padding(int N, int M, int max_rank)
{
    return allocate_2d(N+max_rank, M);
}

static double *parse_matrix_2d_cyclic(FILE *fp, unsigned int N, unsigned int M, double *A, int max_rank)
{
    int i,j;
    double *p;
    int workload = N / max_rank + 1;
    int remainder = N % max_rank;
    double **A2D = appoint_2D(A, N, M);

    for(i = 0; i < workload - 1; i++) {
        for(j = 0; j < max_rank; j++) {
            p = A2D[j*workload + i];

            if(fread(p, sizeof(double), M, fp) != M) {
                return NULL;
            }
        }
    }

    /* this loop reads any remaining data from the file */
    for(i = 1; i <= remainder; i++) {
        p = A2D[i*workload] - 1; 
        if(fread(p, sizeof(double), M, fp) != M) {
            return NULL;
        }
    }

    /* this loop memsets the final line of the bottom parts */
    for(i = remainder; i > 0; i--) {
        p = A2D[i*workload] - 1;
        memset(p, 0, M*sizeof(double));
    }

    free(A2D);
    return A;
}

static double *parse_matrix_2d(FILE *fp, int N, int M, double *A, int max_rank, OPMODE operation)
{
    switch(operation) {
        case CONTINUOUS:
            return parse_matrix_2d_cyclic(fp, N, M, A, 1);
        case CYCLIC:
            return parse_matrix_2d_cyclic(fp, N, M, A, max_rank);
        default:
            return NULL;
    }
}

void fprint_matrix_2d(FILE *fp, int N, int M, double *A)
{
    int i,j;
    double *p;
    p = A;
    for (j = 0; j < M; j++) {
        fprintf(fp, "=");
    }
    fprintf(fp, "\n");
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            fprintf(fp, "%lf\t", *p++);
        }
        fprintf(fp, "\n");
    }
    for (j = 0; j < M; j++) {
        fprintf(fp, "=");
    }
    fprintf(fp, "\n");
} 

void print_matrix_2d(int N, int M, double *A)
{
    fprint_matrix_2d(stdout, N, M, A);
}

double timer(void)
{
    static double seconds = 0;
    static int operation = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (operation == 0) {
        seconds = tv.tv_sec + (((double) tv.tv_usec)/1e6);
        operation = 1;
        return 0;
    }
    else {
        operation = 0;
        return tv.tv_sec + (((double) tv.tv_usec)/1e6) - seconds;
    }
}

void usage(int argc, char **argv)
{
#ifdef USE_MPI /* USE_MPI */
    if(argc != 4) {
        printf("Usage: %s <matrix file> <output file> [propagation mode: default=0 (ptp)]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
#else  
    if(argc != 3) {
        printf("Usage: %s <matrix file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
#endif 
}

Matrix *get_matrix(char *filename, int max_rank, OPMODE operation)
{
    FILE *fp;
    double *A;
    int N;
    Matrix *mat;

    if(NULL == (mat = malloc(sizeof(struct Matrix)))) {
        debug("Could not allocate empty Matrix\n");
        exit(EXIT_FAILURE);
    }
    fp = fopen(filename, "rb");
    if(fp) {
        if(fread(&N, sizeof(int), 1, fp) != 1) {
            debug("Could not read N from file\n");
            exit(EXIT_FAILURE);
        }
    }
    if((A = allocate_2d_with_padding(N, N, max_rank)) == NULL) {
        debug("Could not allocate enough contiguous memory\n");
        exit(EXIT_FAILURE);
    }
    if(parse_matrix_2d(fp, N, N, A, max_rank, operation) == NULL) {
        debug("Could not parse matrix\n");
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    mat->N = N;
    mat->A = A;
    return mat;
}

double **appoint_2D(double *A, int N, int M)
{
    int i;
    double **A2D = (double **) malloc(N*sizeof(double *));
    /* sanity check */
    if(NULL == A2D) {
        return NULL;
    }
    for(i = 0; i < N; i++) {
        A2D[i] = &A[i*M];
    }
    return A2D;
}

#ifdef USE_MPI /* USE_MPI */

/* get operation mode from the third argument.
 * 1 for continuous, 0 for ptp */
void * get_propagation(int argc, char **argv)
{
    if (argc > 3) {
        if (argv[3][0] == '1') {
            return &MPI_Bcast;
        }
    } 
    return &propagate_with_flooding;
}

void propagate_with_send(void *buffer, int count, MPI_Datatype datatype, \
        int root, MPI_Comm comm)
{
    int rank;
    int i;
    int max_rank;
    MPI_Status status;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &max_rank);
    if(rank == root) {
        for(i = 0; i < max_rank; i++) {
            if(i == rank) {
                continue;
            }
            else {
                debug("%d\n", i);
                MPI_Send(buffer, count, datatype, i, root, comm);
            }
        }
    }
    else {
        MPI_Recv(buffer, count, datatype, root, root, comm, &status);
    }
}

void propagate_with_flooding(void *buffer, int count , MPI_Datatype datatype, \
        int root, MPI_Comm comm)
{
    int rank;
    int max_rank;
    int cur;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &max_rank);
    MPI_Status status;

    if(root != 0) {
        if(rank == root) {
            MPI_Send(buffer, count, datatype, 0, root, comm);
        }
        if(rank == 0) {
            MPI_Recv(buffer, count, datatype, root, root, comm, &status);
        }
    }

    if(rank != 0) {
        MPI_Status status;
        MPI_Recv(buffer, count, datatype, (rank-1)/2, root, comm, &status);
    }
    cur = 2*rank+1;
    if(cur < max_rank) {
        MPI_Send(buffer, count, datatype, cur, root, comm);
    }
    if(++cur < max_rank) {
        MPI_Send(buffer, count, datatype, cur, root, comm);
    }
}

/* Returns the displacements table in rows */
void get_displs(int *counts, int max_rank, int *displs) 
{
    int j;
    displs[0] = 0;
    for (j = 1; j < max_rank ; j++) {
        displs[j] = displs[j - 1] + counts[j - 1];
    }
}

/*  distributes the rows in a continuous fashion */
void get_counts(int max_rank, int N, int *counts) 
{
    int j, k;
    int rows = N;

    /* Initialize counts */
    for (j = 0; j < max_rank ; j++) {
        counts[j] = (rows / max_rank);
    }

    /* Distribute the indivisible leftover */
    if (rows / max_rank != 0) {
        j = rows % max_rank;    
        for (k = 0; k < max_rank && j > 0; k++, j--) {
            counts[k] += 1;
        }
    } 
    else {
        for (k = 0; k < max_rank; k++) {
            counts[k] = 1;
        }
    }
}


/* Gather everything to root */
void gather_to_root_cyclic(double **Ap2D, int max_rank, int rank, int root, double **A2D, int N, int M)
{
    int i;
    int bcaster;
    int current_row;
    MPI_Status status;
    for(i = 0; i < N; i++) {
        bcaster = i % max_rank;
        current_row = i / max_rank;
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank == bcaster) {
            if(bcaster == root) {
                memcpy(A2D[i], Ap2D[current_row], M*sizeof(double));
            }
            else {
                MPI_Send(Ap2D[current_row], M, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
            }
        }
        else if (rank == root) {
            MPI_Recv(A2D[i], M, MPI_DOUBLE, bcaster, i, MPI_COMM_WORLD, &status);
        }
    }
}


#endif /* USE_MPI */
