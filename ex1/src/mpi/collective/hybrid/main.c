/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Tue 13 Nov 2012 12:14:06 PM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "common.h"

#define BLOCK_ROWS 1

/* Sort a pseudo-2D array cyclically by rows */
void scatter_sort(int max_rank, int N, double *A) {
    /* creates a temporary matrix to hold the distrubution *
    *  and an array of pointers to each thread's current head */
    int i, j;
    double ***T; // T[max_rank][workload][N]
    int *head;  // head[max_rank]
    int owner;
    int cur_head;
    int cur_index;
    int workload = (N / max_rank) + 1;
    debug("Threads: %d\n", max_rank);
    debug("Max Work per Thread: %d\n", workload);

    head = malloc(max_rank * sizeof(int));
    T = (double ***) malloc(max_rank * sizeof(double **));

    /* Initializations */
    for (i=0; i < max_rank; i++) {
        T[i] = (double **) malloc(workload * sizeof(double *));
        for (j=0; j < workload; j++) {
            T[i][j] = (double *) malloc(N * sizeof(double));
        }
    }
    for (i=0; i < max_rank; i++) {
        head[i] = 0; 
    }

    /* Distribute the rows to Temp cyclically */
    for (i=0; i < N; i++) {
        owner = i % max_rank;
        memcpy(T[owner][head[owner]++], &A[i * N], N * sizeof(double));

    }
    printf("After distr\n");

    /* Gather everything back to the original array */
    /* This time, they're continuous */
    cur_index = 0;
    for (i=0; i < max_rank; i++) {
        cur_head = 0;
        while (cur_head < head[i]) {
            memcpy(&A[cur_index * N], T[i][cur_head++], N * sizeof(double));
            cur_index += 1;
        }
    }
    free(T);
    free(head);
}

void process_rows(int k, int rank, int N, int max_rank, double *A)
{
    /*      performs the calculations for a given set of rows.
     *      In this hybrid version each thread is assigned blocks of 
     *      continuous rows in a cyclic manner.
     */
    int i, j, w;
    double l;
    /* For every cyclic repetition of a block */
    for (i = (rank + ((BLOCK_ROWS * max_rank) * (k / (BLOCK_ROWS * max_rank)))); i < N ; i+=(max_rank * BLOCK_ROWS)) {
        if (i > k) {
            /* Calculate each continuous row in the block*/
            for (w = i; w < (i + BLOCK_ROWS) && w < (N * N); w++){
                l = A[(w * N) + k] / A[(k * N) + k];
                for (j = k; j < N; j++) {
                    A[(w * N) + j] = A[(w * N) + j] - l * A[(k* N) + j];
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    int k;
    int N;
    int rank;
    int max_rank;
    int last_rank;
    int workload;
    int ret = 0;
    double *A = NULL;
    double sec = 0;
    FILE *fp = NULL;

    usage(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    /* Root gets the matrix */
    if (rank == 0) {
        Matrix *mat = get_matrix(argv[1], max_rank);
        N = mat->N;
        A = mat->A;
        scatter_sort(max_rank, N, A); 
    }
    /* And broadcasts N */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    workload = (N / max_rank) + 1;

    /* Allocations */
    Ak = malloc(N * sizeof(double));
    Ap = malloc(workload * N * sizeof(double));

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatterv(A, workload * N, MPI_DOUBLE, \
            Ap, workload * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    Ap2D = appoint_2D(Ap, workload, N);



    /* Start Timing */
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1; k++) {
        /* The owner of the row for this k broadcasts it*/
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&A[k * N], N, MPI_DOUBLE, ((k % (max_rank * BLOCK_ROWS)) / BLOCK_ROWS), MPI_COMM_WORLD);

        process_rows(k, rank, N, max_rank, A);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        sec = timer();
        printf("Calc Time: %lf\n", sec);
    }
    ret = MPI_Finalize();

    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }

    /* Last process has table */
    if (rank == last_rank) {
        //print_matrix_2d(N, N, A);
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
    }
    free(A);

    return 0;
}
