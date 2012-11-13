/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : common.c
 * Creation Date : 06-11-2012
 * Last Modified : Tue 13 Nov 2012 09:59:08 AM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include "common.h"
#include <sys/time.h>

double *allocate_2d(int N, int M)
{
    double *A;
    A = malloc(N * M * sizeof(double));
    return A;
}

double *allocate_2d_with_padding(int N, int M, int max_rank)
{
    double *A;
    A = allocate_2d(N + max_rank, M);
    return A;
}

double *parse_matrix_2d(FILE *fp, int N, int M, double *A)
{
    int i,j;
    double *p;
    p = A;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if(fread(p++, sizeof(double), 1, fp) != 1) {
            //if(!fscanf(fp, "%lf", p++)) {
            //if(!fscanf(fp, "%lf", p++)) {
                return NULL;
            }
        }
    }
    return A;
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
