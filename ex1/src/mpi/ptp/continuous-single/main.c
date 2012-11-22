/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Tue 13 Nov 2012 12:11:50 PM EET
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


/* Returns the rank of the broadcaster given the previous one */
int get_bcaster(int *ccounts, int bcaster) 
{
    int result;
    if (ccounts[bcaster] > 0 ){
        result =  bcaster;
    } 
    else {
        result = bcaster+1;
    }
    ccounts[result]--;
    return result;
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

/*      performs the calculations for a given set of rows.
 *      Each thread is assigned a continuous quantity of rows
 */
void process_rows(int k, int rank, int N, int *counts, int *ccounts, \
        double **Ap2D, double *Ak)
{
    int j, w;
    double l;
    int start = counts[rank] - ccounts[rank];
    for (w = start; w < counts[rank]; w++) {
        l = Ap2D[w][k] / Ak[k];
        for (j = k; j < N; j++) {
            Ap2D[w][j] = Ap2D[w][j] - l * Ak[j];
        }
    }
}

/*  distributes the rows in a continuous fashion */
void distribute_rows(int max_rank, int N, int *counts) 
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
        } else {
            for (k = 0; k < max_rank; k++){
                counts[k] = 1;
            }
        }
        
}
                

int main(int argc, char **argv)
{
    int k;
    int i;
    int N;
    int rank;
    int workload;
    int max_rank;
    int *counts;
    int *displs;
    int *ccounts;
    int ret = 0;
    int bcaster = 0;
    double sec;
    double *A = NULL;
    double **A2D = NULL;
    double *Ap = NULL;
    double **Ap2D = NULL;
    double *Ak = NULL;
    FILE *fp = NULL;
    MPI_Datatype row_type;

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
    /* And propagates N */
    MPI_Barrier(MPI_COMM_WORLD);
    propagate_with_flooding(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    workload = (N / max_rank) + 1; // Max number of rows for each thread

    /* Allocations */
    Ak = malloc(N * sizeof(double));
    Ap = malloc(workload * N * sizeof(double));
    counts = malloc(max_rank * sizeof(int));
    displs = malloc(max_rank * sizeof(int));
    ccounts = malloc(max_rank * sizeof(int));

    /* Initializations */
    distribute_rows(max_rank, N, counts);
    get_displs(counts, max_rank, displs);
    memcpy(ccounts, counts, max_rank * sizeof(int));

    
    // TODO: Change to distribute rows in a point-point way
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_vector(1, N, N, MPI_DOUBLE, &row_type);
    MPI_Type_commit(&row_type);
    MPI_Scatterv(A, counts, displs, row_type, 
            Ap, workload, row_type, 0, MPI_COMM_WORLD);
    MPI_Type_free(&row_type);

    Ap2D = appoint_2D(Ap, N, N);

    /* Start Timing */
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1; k++) {
        /* Find who owns the k-th row */
        bcaster = get_bcaster(ccounts, bcaster);

        /* The broadcaster puts his k-th row in the Ak buffer */
        if (rank == bcaster){
            i = counts[bcaster] - ccounts[bcaster];
            memcpy(Ak, Ap2D[i], N * sizeof(double));
        }
        /* Everyone receives the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        propagate_with_flooding(Ak, N, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);
        /* Root collects all the broadcasts to fill the final matrix */
        if (rank == 0) {
            memcpy(A2D[k], Ak, N * sizeof(double));
        }

        /* And off you go to work. */
        process_rows(k, rank, N, counts, ccounts, Ap2D, Ak);
    }

    { /* This will collect the final row we need to root */
        bcaster = max_rank - 1;
        if (rank == bcaster){
            memcpy(Ak, Ap2D[counts[bcaster] - 1], N * sizeof(double));
        }
        /* Everyone receives the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        propagate_with_flooding(Ak, N, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        /* Root collects all the broadcasts to fill the final matrix */
        if (rank == 0) {
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
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
    }
    free(A);
    free(A2D);
    free(Ap);
    free(Ap2D);
    free(Ak);
    free(counts);
    free(ccounts);
    free(displs);

    return 0;
}
