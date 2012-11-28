/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : common.c
 * Creation Date : 06-11-2012
 * Last Modified : Wed 28 Nov 2012 02:55:35 AM EET
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

static double *parse_matrix_2d_cyclic(FILE *fp, int N, int M, double *A, int max_rank)
{
    int i,j;
    double *p;
    int workload = N / max_rank + 1;
    int remainder = N % max_rank;
    for(i = 0; i < workload - 1; i++) {
        for(j = 0; j < max_rank; j++) {
            p = &A[j*workload+i];
            if(fread(p, M*sizeof(double), 1, fp) != 1) {
                return NULL;
            }
        }
    }
    for(i = 0; i < remainder; i++) {
        p = &A[i*workload-1];
        if(fread(p, M*sizeof(double), 1, fp) != 1) {
            return NULL;
        }
    }
    for(i = remainder; i > 0; i--) {
        p = &A[i*workload-1];
        memset(p, 0, M*sizeof(double));
    }

    return A;
}

static double *parse_matrix_2d(FILE *fp, int N, int M, double *A)
{
    return parse_matrix_2d_cyclic(fp, N, M, A, 1);
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
    if(argc != 3) {
        printf("Usage: %s <matrix file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

Matrix *get_matrix(char *filename, int max_rank)
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
    if(parse_matrix_2d(fp, N, N, A) == NULL) {
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
void propagate_with_send(void *buffer, int count, MPI_Datatype datatype, \
        int root, MPI_Comm comm)
{
    int rank;
    int i;
    int max_rank;

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
        MPI_Status status;
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

    if(root != 0) {
        if(rank == root) {
            MPI_Send(buffer, count, datatype, 0, root, comm);
        }
        if(rank == 0) {
            MPI_Status status;
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
#endif /* USE_MPI */
