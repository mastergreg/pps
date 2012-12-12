/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Thu 29 Nov 2012 05:58:51 PM EET
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
        start = k % workload + 1;
    /* If before broadcaster, dont do anything */
    } else {
        start = workload;
    }

    for (w = start; w < workload; w++) {
        l = Ap2D[w][k] / Ak[k];
        for (j = k; j < N; j++) {
            Ap2D[w][j] = Ap2D[w][j] - l * Ak[j];
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
    void (*propagate) (void*, int, MPI_Datatype, int, MPI_Comm);
    time_struct ts;

    usage(argc, argv);
    propagate = get_propagation(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    /* Root gets the matrix */
    if (rank == 0){
        Matrix *mat = get_matrix(argv[1], max_rank, CONTINUOUS);
        N = mat->N;
        A = mat->A;
        A2D = appoint_2D(A, N, N);
    }
    /* And broadcasts N */
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
    MPI_Scatter(A, workload * N, MPI_DOUBLE, \
            Ap, workload * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    Ap2D = appoint_2D(Ap, workload, N);

    /* Init Communication Timer */
    time_struct_init(&ts);

    /* Start Total Timer */
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
            memcpy(&Ak[k], &Ap2D[i][k], (N-k) * sizeof(double));
        }

        /* Everyone receives the k-th row */
        time_struct_set_timestamp(&ts);
        (*propagate) (&Ak[k], N-k, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);
        time_struct_add_timestamp(&ts);

        /* And off you go to work. */
        process_rows(k, rank, N, workload, Ap2D, Ak);
    }

    /* Stop Timing */
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        sec = timer();
        printf("Calc Time: %lf\n", sec);
    }

    printf("Rank: %d Comm Time: %lf\n", rank, get_seconds(&ts));

    /* Gather the table from each thread's Ap */
    MPI_Gather(Ap, workload * N, MPI_DOUBLE, \
            A, workload * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    ret = MPI_Finalize();
    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }

    /* Format and output solved matrix */
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
