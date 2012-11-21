/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Tue 13 Nov 2012 12:13:32 PM EET
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


/* Returns the index at which the k-th row is located for a given rank */
int get_short_index(int k, int rank, int *displs, int *counts) {
    int result;
    if (k > (displs[rank] + counts[rank])){
        result = -1;
    } else {
        if (k >= displs[rank]) {
            result = k - displs[rank]; 
        } else {
            result = 0;
        }
    }
    debug("for k: %d and rank: %d, i think the index is %d\n",k,rank,result);
    return result;
}

/* Returns the rank of the broadcaster given the previous one */
int get_bcaster(int *ccounts, int bcaster) 
{
    int result,i;
    if (ccounts[bcaster]-- > 0 ){
        result =  bcaster;
    } 
    else {
        result = bcaster+1;
    }
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
void process_rows(int k, int rank, int N, int *counts, int *displs, double *Ap, double *Ak)
{

    int j, w;
    double l;
    int start = get_short_index(k, rank, displs, counts);
    for (w = start; w < (start + counts[rank]) && w < N && w >= 0; w++){
        l = Ap[(w * N) + k] / Ak[k];
        for (j = k; j < N; j++) {
            Ap[(w * N) + j] = Ap[(w * N) + j] - l * Ak[j];
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
    } 
    else {
        for (k = 0; k < max_rank; k++) {
            counts[k] = 1;
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
    int *counts;
    int *displs;
    int *ccounts;
    int ret = 0;
    int bcaster = 0;
    double sec;
    double *A = NULL;
    double *Ap = NULL;
    double *Ak = NULL;
    FILE *fp = NULL;
    MPI_Datatype row_type;

    usage(argc, argv);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    /* Root gets the matrix */
    if (rank == 0){
        Matrix *mat = get_matrix(argv[1]);
        N = mat->N;
        A = mat->A;
    }
    /* And broadcasts N */
    //( cause we dont want others to know about A)
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    workload = (N / max_rank) + 1; // Max number of rows for each thread

    if (rank == 0){
        debug("A: \n");
        fprint_matrix_2d(stdout, N, N, A);
    }

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

    /* Scatter the table to each thread's Ap */
    MPI_Type_vector(1, N, N, MPI_DOUBLE, &row_type);
    MPI_Type_commit(&row_type);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Scatterv(A, counts, displs, row_type, 
            Ap, workload, row_type, 0, MPI_COMM_WORLD);


    if (rank == 0){
        debug("first one's matrix\n");
        fprint_matrix_2d(stdout, workload, N, Ap);
    }
    if (rank == max_rank - 1){
        debug("last one's matrix\n");
        fprint_matrix_2d(stdout, workload, N, Ap);
    }

    MPI_Type_free(&row_type);

    /* Start Timing */
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N ; k++) {
        /* Find who owns the k-th row */
        bcaster = get_bcaster(ccounts, bcaster);
        debug("k: %d bcaster is %d \n", k,bcaster);
        
        /* The broadcaster puts his k-th row in the Ak buffer */
        if (rank == bcaster){
            i = get_short_index(k, rank, displs, counts);
            memcpy(Ak, &Ap[i*N], N * sizeof(double));
        }
        /* Everyone receives the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        /* Root collects all the broadcasts to fill the final matrix */
        if ((rank == 0) && (bcaster != 0)){
            memcpy(&A[k * N], Ak, N * sizeof(double));
        }

        /* And off you go to work. Last loop is reserved for syncing root */
        if (k < (N - 1)){
            process_rows(k, rank, N, counts, displs, Ak, Ap);
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
        free(A);
    }
    free(Ap);
    free(Ak);
    free(displs);
    free(counts);
    free(ccounts);

    return 0;
}
