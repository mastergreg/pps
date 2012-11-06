/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Tue 06 Nov 2012 03:04:13 PM EET
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


/* calculates the distribution arrays for scatterv and gatherv */
void get_distr_arrays(int k, int max_rank, int N, int *displs, int *counts) {
        int j;
        int rows = N - k - 1;

        /* Initialize counts */
        for (j = 0; j < max_rank ; j++) {
            counts[j] = N * (rows / max_rank);
        }

        /* Distribute the indivisible leftover */
        j = rows % max_rank;    
        for (k = 0; k < max_rank ; k++) {
            if (j > 0) {
                counts[k] += N;
                j--;
            }
        }

        /* Initialization of Displacements array */
        displs[0] = 0;
        for (j = 1; j < max_rank ; j++) {
            displs[j] = displs[j - 1] + counts[j - 1];
        }
        
#if main_DEBUG
        printf("Sendcounts is :\n");
        for (j = 0; j < max_rank ; j++) {
            printf("%d\n", counts[j]);
        }
        
        printf("Displs is :\n");
        for (j = 0; j < max_rank ; j++) {
            printf("%d\n", displs[j]);
        }
#endif
}
                

int main(int argc, char **argv)
{
    int i, j, k;
    int N;
    int rank;
    int max_rank;
    int *counts;
    int *displs;
    int ret = 0;
    double l;
    double *Ak = NULL;
    double *A = NULL;
    double *Ai = NULL;
    FILE *fp = NULL;


    if (argc != 2) {
        printf("Usage: %s <matrix file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);

    if (rank == 0) {
        debug("rank: %d opens file: %s\n", rank, argv[1]);
        fp = fopen(argv[1], "r");
        if(fp) {
            if(!fscanf(fp, "%d\n", &N)) {
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        else {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    Ak = malloc(N * sizeof(double)); // Buffer for broadcasting the k-th row
    Ai = malloc(N * ((N / max_rank) + 1) * sizeof(double)); // Buffer for scattering the rows


    
    displs = malloc(max_rank * sizeof(int));
    counts = malloc(max_rank * sizeof(int));
    if (rank == 0) {
        /* Root Allocates the whole table */
        debug("Max rank = %d\n", max_rank);
        if((A = allocate_2d_with_padding(N, N, max_rank)) == NULL) {
                //if((A = allocate_2d(N, N)) == NULL) {
                //A = allocate_2d(N, N);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if(parse_matrix_2d(fp, N, N, A) == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fclose(fp);
        fp = NULL;
    } 

    /* Start Timing */
    for (k = 0; k < N - 1; k++) {
        if (rank == 0) {
            Ak = memcpy(Ak, &A[k * N], N*sizeof(double));
            debug("k %d\n", k);
        }

        /* Send everyone the k-th row, and scatter the rest of the table */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

//        if (rank == 0){
            get_distr_arrays(k, max_rank, N, displs, counts);
//        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Scatterv(&A[N * (k + 1)], counts, displs, MPI_DOUBLE, Ai, N * (((N - k - 1) / max_rank) + 1), MPI_DOUBLE, 0, MPI_COMM_WORLD);

#if main_DEBUG
        if(rank == 0) {
            debug("Hi i'm %d. Ak: %p Ai %p\n", rank, Ai, Ak); 
            print_matrix_2d(1, N, Ak);
            print_matrix_2d(1, N, Ai);
        }
#endif

        /* Perform all assigned calculations */
        MPI_Barrier(MPI_COMM_WORLD);
        for (i = 0; i < counts[rank]; i += N ) {
            l = Ai[i+k] / Ak[k];
            for (j = k; j < N; j++) {
                Ai[i+j] = Ai[i+j] - l * Ak[j];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
#if main_DEBUG
        if(rank == 0) {
            printf("====== Ai AFTER processing root's rows: \n");
            print_matrix_2d(2, N, Ai);
            printf("MATRIX BEFORE GATHER:\n");
            print_matrix_2d(N, N, A);
        }
        printf("rank %d waiting for gather\n", rank);
#endif
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gatherv(Ai, N * (((N - k - 1) / max_rank) + 1) , MPI_DOUBLE, &A[N * (k + 1)], counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        /* If gatherv doesnt ignore zeroes, we might get some in the matrix :/ */

#if main_DEBUG
        printf("rank %d after gather\n", rank);
        if(rank == 0) {
            print_matrix_2d(N, N, A);
        }
#endif
    }

    MPI_Barrier(MPI_COMM_WORLD);
    ret = MPI_Finalize();
    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }

    if(rank == 0) {
        //print_matrix_2d(N, N, A);
        fp = fopen("mat.out", "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
        free(A);
    }
    free(Ai);
    free(Ak);

    return 0;
}
