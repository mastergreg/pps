/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : main.c
* Creation Date : 30-10-2012
* Last Modified : Tue 30 Oct 2012 06:30:19 PM EET
* Created By : Greg Liras <gregliras@gmail.com>
* Created By : Alex Maurogiannis <nalfemp@gmail.com>
_._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>


int main(int argc, char **argv)
{
    int i,j,k;
    int N;
    double l;
    int rank;
    double *Ak;
    double **A;
    double *Ai;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double Aa[40];
    for (i = 0; i <40; i++){
        Aa[i]=(double) i;
    }
    double Ab[10];
    for (i = 0; i <40; i++){
        Ab[i]=Aa[i];
    }

    if (rank == 0) {
        scanf("%d\n", &N);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);


    /* Allocations */

    Ak = malloc(N*sizeof(double)); // Buffer for broadcasting the k-th row

    if (rank == 0) {
    /* Root Allocates the whole table */
    A = malloc(N*sizeof(double*));
        for (k = 0; k < N; k++) {
            A[k] = malloc(N*sizeof(double));
        }

        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                scanf("%lf", &A[i][j]);
            }
        }

    } else {
        /* Others allocate a buffer for the row they'll test */
        Ai = malloc(N*sizeof(double));
    }

    for (k = 0; k < N - 1; k++) {
        if (rank == 0) {
                Ak = &A[k][0];
            }

            /* Send everyone the k-th row, and scatter the rest of the table */
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Bcast(Ak, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

            printf("i'm rank %d BEFORE the scatter and N is %d\n", rank,N);
            MPI_Barrier(MPI_COMM_WORLD);

            MPI_Scatter(Aa, 40, MPI_DOUBLE, Ab, 10 , MPI_DOUBLE, 0, MPI_COMM_WORLD);
            //            MPI_Scatter(A[k+1], N*(N-k-1), MPI_DOUBLE,
 //                       Ai, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            printf("i'm rank %d AFTER the scatter, check out my bitchin' Ab \n", rank);

            for (i=0;i<10;i++){
                printf("%lf\n", Ab[i]);
            }
            if (rank != 0) {
                l = Ai[k] / Ak[k];

                for (j = k; j < N; j++) {
                    Ai[j] = Ai[j] -l*Ak[j];
                }

            }
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Gather(Ai, N, MPI_DOUBLE,
                       &(A[k+1][0]), N, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);
    }

    MPI_Finalize();
    for (i = 0;
i < N;
i++) {
    for (j = 0; j < N; j++) {
            printf("%lf\t", A[i][j]);
        }
        printf("\n");
    }

    return 0;
}
