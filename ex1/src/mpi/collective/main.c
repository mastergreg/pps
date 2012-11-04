/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Mon 05 Nov 2012 01:36:56 AM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>


#define NTHREADS 2

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

static void print_matrix_2d(int N, int M, double *A)
{
    int i,j;
    double *p;
    p = A;
    for (j = 0; j < M; j++) {
        printf("=");
    }
    printf("\n");
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            printf("%lf ", *p++);
        }
        printf("\n");
    }
    for (j = 0; j < M; j++) {
        printf("=");
    }
    printf("\n");
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

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        scanf("%d\n", &N);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);


    Ak = malloc(N*sizeof(double)); // Buffer for broadcasting the k-th row
    Ai = malloc(N*sizeof(double)); // Buffer for scattering the i-th row

    if (rank == 0) {
        /* Root Allocates the whole table */
        A = allocate_2d(N, N);
        parse_matrix_2d(N, N, A);
    } 



    for (k = 0; k < N - 1; k++) {
        if (rank == 0) {
            Ak = &A[k*N];
        }
        /* Send everyone the k-th row, and scatter the rest of the table */
        MPI_Barrier(MPI_COMM_WORLD);
        printf("k %d\n", k);
        MPI_Bcast(Ak, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Scatter(&A[N * (k + 1)], N, MPI_DOUBLE, Ai, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        if(rank == 0) {
            print_matrix_2d(N, N, A);
        }

        // every run has the same k but they have different ranks
        l = Ai[k + rank] / Ak[k + rank];
        for (j = k + rank; j < N; j++) {
            Ai[j] = Ai[j] -l*Ak[j];
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gather(Ai, N , MPI_DOUBLE, &(A[N * (k + 1)]), N, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
    if(rank == 0) {
        print_matrix_2d(N, N, A);
    }

    


    //if(rank == 0) {
    //    free(A);
    //}
    //free(Ak);
    //free(Ai);
    //A = NULL;
    //Ak = NULL;
    //Ai = NULL;

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("%lf\t", A[i*N+j]);
        }
        printf("\n");
    }

    return 0;
}
