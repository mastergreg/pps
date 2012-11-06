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


int main(int argc, char **argv)
{
    int j, k;
    int N;
    double l;
    int rank;
    int max_rank;
    int completed_rows;
    double *Ak = NULL;
    double *A = NULL;
    double *Ai = NULL;


    int ret = 0;
    FILE *fp = NULL;
    if(argc != 2) {
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


    Ak = malloc(N*sizeof(double)); // Buffer for broadcasting the k-th row
    Ai = malloc(N*sizeof(double)); // Buffer for scattering the i-th row

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



    for (k = 0; k < N - 1; k++) {
        if (rank == 0) {
            Ak = memcpy(Ak, &A[k * N], N*sizeof(double));
            debug("k %d\n", k);
        }
        /* Send everyone the k-th row, and scatter the rest of the table */
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        for(completed_rows = 0; completed_rows < N - max_rank; completed_rows+=max_rank) {
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Scatter(&A[N * (k + 1 + completed_rows)], N, MPI_DOUBLE, Ai, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#if main_DEBUG
            if(rank == 0) {
                debug("Hi i'm %d. Ak: %p Ai %p\n", rank, Ai, Ak); 
                print_matrix_2d(1, N, Ak);
                print_matrix_2d(1, N, Ai);
            }
#endif

            MPI_Barrier(MPI_COMM_WORLD);
            l = Ai[k] / Ak[k];
            for (j = k; j < N; j++) {
                Ai[j] = Ai[j] - l * Ak[j];
            }

            MPI_Barrier(MPI_COMM_WORLD);
#if main_DEBUG
            if(rank == 0) {
                printf("BEFORE GATHER:\n");
                print_matrix_2d(N, N, A);
            }
#endif
            MPI_Gather(Ai, N , MPI_DOUBLE, &A[N * (k + 1 + completed_rows)], N, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);
#if main_DEBUG
            if(rank == 0) {
                printf("AFTER GATHER:\n");
                print_matrix_2d(N, N, A);
            }
#endif

        }
        MPI_Barrier(MPI_COMM_WORLD);
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
