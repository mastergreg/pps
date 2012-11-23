/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Thu 22 Nov 2012 06:07:13 PM EET
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

/* Turns a 2D matrix to upper triangular */
void upper_triangularize(int N, double **Ap2D)
{
    int i,j;
    for (i=1; i < N; i++) {
        for (j=0; j < i; j++) {
            Ap2D[i][j] = 0;
        }
    }
}

/*      performs the calculations for a given set of rows.
 *      Each thread is assigned a continuous quantity of rows
 */
void process_rows(int k, int rank, int N, int workload, double **Ap2D, double *Ak)
{
    int j, w;
    double l;
    int start;

    /* If after the broadcaster, do all your rows */
    if (rank > (k / workload)) {
        start = 0;
    /* If broadcaster, do all rows after the one you bcasted */
    } else if ( rank == (k / workload)) {
        start = k % workload;
    /* If before broadcaster, dont do anything */
    } else {
        start = workload;
    }

    if (rank == 1 ) {
        printf("rank: %d k: %d\n", rank, k);
    }

    for (w = start; w < workload; w++) {
        if (rank == 1 ) {
            printf("before %d-th row\n", w);
            print_matrix_2d(1,N,Ap2D[w]);
        }
        l = Ap2D[w][k] / Ak[k];
        for (j = k; j < N; j++) {
            Ap2D[w][j] = Ap2D[w][j] - l * Ak[j];
        }

        if (rank == 1 ) {
            printf("after %d-th row\n", w);
            print_matrix_2d(1,N,Ap2D[w]);
        }
    }
}


int main(int argc, char **argv)
{
    int k;
    int N;
    int i;
    int rank;
    int max_rank;
    int workload;
    int ret = 0;
    int bcaster = 0;
    double sec;
    double *A = NULL;
    double **A2D = NULL;
    double *Ap = NULL;
    double **Ap2D = NULL;
    double *Ak = NULL;
    FILE *fp = NULL;

    usage(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    /* Root gets the matrix */
    if (rank == 0){
        Matrix *mat = get_matrix(argv[1], max_rank);
        N = mat->N;
        A = mat->A;
        A2D = appoint_2D(A, N, N);
    }
    /* And broadcasts N */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    workload = (N / max_rank) + 1; // Max number of rows for each thread

#if main_DEBUG
    if (rank == 0){
        debug("A: \n");
        print_matrix_2d(N, N, A);
    }
#endif

    /* Allocations */
    Ak = malloc(N * sizeof(double));
    Ap = malloc(workload * N * sizeof(double));

    /* Scatter the table to each thread's Ap */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatter(A, workload * N, MPI_DOUBLE, \
            Ap, workload * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    Ap2D = appoint_2D(Ap, N, N);

    /* Start Timing */
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1 ; k++) {
        /* Find who owns the k-th row */
        bcaster = k / workload; 
            
        /* The broadcaster puts his k-th row in the Ak buffer */
        if (rank == bcaster) {
            i = k % workload;
            memcpy(Ak, Ap2D[i], N * sizeof(double));
        }

        /* Everyone receives the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&Ak[k], N-k, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        if (rank == 0) {
            debug("k: %d and the rows broadcasted was:\n", k);
            print_matrix_2d(1,N-k,&Ak[k]);
        }

        /* Root collects all the broadcasts to fill the final matrix */
        if (rank == 0) {
            memcpy(A2D[k], Ak, N * sizeof(double));
        }

        /* And off you go to work. */
        process_rows(k, rank, N, workload, Ap2D, Ak);
    }

    { /* This will collect the final data we need to root */
        /* Find who owns the k-th row */
        bcaster = max_rank - 1;
            
        /* The broadcaster puts his k-th row in the Ak buffer */
        if (rank == bcaster){
            memcpy(Ak, Ap2D[k % workload], N * sizeof(double));
        }

        /* Everyone receives the last double of the last row */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&Ak[k], 1, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        /* Root collects all the broadcasts to fill the final matrix */
        if (rank == 0) {
            printf("final: \n", k);
            print_matrix_2d(1,N-k,&Ak[k]);
            memcpy(A2D[N-1], Ak, N * sizeof(double));
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
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

    if(rank == 0) {
        upper_triangularize(N, A2D);
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
        free(A);
        free(A2D);
    }
    free(Ap);
    free(Ap2D);
    free(Ak);

    return 0;
}
