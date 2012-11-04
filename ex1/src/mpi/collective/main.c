/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Sun 04 Nov 2012 09:31:47 PM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define NTHREADS 5

static double *allocate_2d(int N, int M)
{
    double *A;
    A = malloc(N*M*sizeof(double*));
    return A;
}

static double *parse_matrix_2d(int N, int M, double *A)
{
    int i,j;
    double *p;
    p = A;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            scanf("%lf", p++);
        }
    }
    return A;
}


int main(int argc, char **argv)
{
    int i,j,k;
    int N;
    double l;
    int rank;
    double *Ak;
    double *A;
    double *Ai;
    int *counts;
    int *displs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        scanf("%d\n", &N);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);


    /* Allocations */
    counts = malloc(N*sizeof(int));
    displs = malloc(N*sizeof(int));
    for (i = 0; i < N; i++) {
        counts[i] = N*i;
        displs[i] = N*i;
    }

    for (i = (N - (N % NTHREADS)); i < N; i++) {
        counts[i] = 0;
        displs[i] = 0;
    }

    Ak = malloc(N*sizeof(double)); // Buffer for broadcasting the k-th row

    if (rank == 0) {
        /* Root Allocates the whole table */
        A = allocate_2d(N, N);
        parse_matrix_2d(N, N, A);
    } else {
        /* Others allocate a buffer for the row they'll test */
        Ai = malloc(N*sizeof(double));
    }



    for (k = 0; k < N - 1; k++) {
        if (rank == 0) {
            Ak = &A[k*N];
            printf("k = %d\n", k);
        }
        /* Send everyone the k-th row, and scatter the rest of the table */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        printf("i'm rank %d BEFORE the scatter and N is %d\n", rank,N);
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Scatter(&A[N * (k + 1)], N/NTHREADS, MPI_DOUBLE,Ai, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);
        printf("i'm rank %d AFTER the scatter, check out my bitchin' Ab \n", rank);
        MPI_Barrier(MPI_COMM_WORLD);

        if (rank != 0) {
            l = Ai[k] / Ak[k];

            for (j = k; j < N; j++) {
                Ai[j] = Ai[j] -l*Ak[j];
            }

        }

        MPI_Barrier(MPI_COMM_WORLD);
        printf("Gather your shit!\n");
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Gather(Ai, N/NTHREADS, MPI_DOUBLE, &(A[N * (k + 1)]), N, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);
    }

    MPI_Finalize();
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%lf\t", A[i*N+j]);
        }
        printf("\n");
    }

    return 0;
}
