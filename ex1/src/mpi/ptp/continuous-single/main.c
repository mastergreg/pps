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


int get_bcaster(int *ccounts, int bcaster) 
{
    if (ccounts[bcaster]-- > 0 ){
        return bcaster;
    } else {
        return bcaster+1;
    }
}

void get_displs(int *counts, int max_rank, int *displs) 
{
    int j;
    displs[0] = 0;
    for (j = 1; j < max_rank ; j++) {
        displs[j] = displs[j - 1] + counts[j - 1];
    }
}

int max(int a, int b) 
{
    return a > b ? a : b;
}

void process_rows(int k, int rank, int N, int max_rank, int block_rows, int *displs, double *A)
{
    /*      performs the calculations for a given set of rows.
     */
    int j, w;
    double l;
    int start = max(displs[rank], k+1);
    for (w = start; w < (start + block_rows) && w < N; w++){
        l = A[(w * N) + k] / A[(k * N) + k];
        for (j = k; j < N; j++) {
            A[(w * N) + j] = A[(w * N) + j] - l * A[(k * N) + j];
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
    int j;
    int N;
    int rank;
    int max_rank;
    int block_rows;
    int *counts;
    int *displs;
    int *ccounts;
    int ret = 0;
    int bcaster = 0;
    double sec;
    double *A = NULL;
    FILE *fp = NULL;


    usage(argc, argv);

    Matrix *mat = get_matrix(argv[1]);
    N = mat->N;
    A = mat->A;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    MPI_Barrier(MPI_COMM_WORLD);
    
    counts = malloc(max_rank * sizeof(int));
    displs = malloc(max_rank * sizeof(int));
    ccounts = malloc(max_rank * sizeof(int));

    distribute_rows(max_rank, N, counts);
    get_displs(counts, max_rank, displs);
    memcpy(ccounts, counts, max_rank * sizeof(int));

#if main_DEBUG
        printf("CCounts is :\n");
        for (j = 0; j < max_rank ; j++) {
            printf("%d\n", ccounts[j]);
        }
#endif

    MPI_Barrier(MPI_COMM_WORLD);

    block_rows = counts[rank];

    /* Start Timing */
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1; k++) {
        bcaster = get_bcaster(ccounts, bcaster);

        debug(" broadcaster is %d\n", bcaster);
        MPI_Barrier(MPI_COMM_WORLD);
        propagate_with_flooding(&A[k * N], N, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);

        process_rows(k, rank, N, max_rank, block_rows, displs, A);
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

    if(rank == (max_rank - 1)) {
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
    }
    free(A);
    free(counts);
    free(ccounts);
    free(displs);

    return 0;
}
