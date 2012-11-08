/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Thu 08 Nov 2012 09:49:47 AM EET
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
    int i, j, k;
    int N;
    double l;
    int rank;
    int max_rank;
    int completed_rows;
    double *Ak = NULL;
    double *A = NULL;
    double sec = 0;
    int last_rank;

    int ret = 0;
    FILE *fp = NULL;
    usage(argc, argv);

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

    Ak = malloc(N*sizeof(double)); // Buffer for broadcasting the k-th row

    /* Everyone allocates the whole table */
    debug("Max rank = %d\n", max_rank);
    if((A = allocate_2d_with_padding(N, N, max_rank)) == NULL) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    /* Root Parses file */
    if (rank == 0) {
        if(parse_matrix_2d(fp, N, N, A) == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fclose(fp);
        fp = NULL;
    }
    /* And distributes the table */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(A, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
 

    if(rank == 1) {
        print_matrix_2d(N, N, A);
    }

    last_rank = (N - 1) % max_rank;

    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1; k++) {
        /* (k % max_rank) is the broadcaster for the k-th row */
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == (k % max_rank)) {
            debug("rank %d broadcasting\n: ", rank);
            Ak = memcpy(Ak, &A[k * N], N*sizeof(double));
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(Ak, N, MPI_DOUBLE, (k % max_rank), MPI_COMM_WORLD);

        /* Last process to run is (N-1) % max_rank */
        if (rank == last_rank) {
            memcpy(&A[k * N], Ak, N*sizeof(double));
        }

        MPI_Barrier(MPI_COMM_WORLD);
        for (i = rank; i < N ; i+=max_rank) {
            if (i > k) {
                l = A[(i * N) + k] / Ak[k];
                for (j = k; j < N; j++) {
                    A[(i * N) + j] = A[(i * N) + j] - l * Ak[j];
                }
            }
        }

#if main_DEBUG
        if (rank == last_rank) {
            debug("LAST RANK AFTER k = %d:\n",k);
            print_matrix_2d(N,N,A);
        }
#endif



    }

    MPI_Barrier(MPI_COMM_WORLD);
    ret = MPI_Finalize();
    if (rank == 0) {
        sec = timer();
        printf("Calc Time: %lf\n", sec);
    }

    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }

    if (rank == last_rank) {
        //print_matrix_2d(N, N, A);
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
    }
    free(A);
    free(Ak);

    return 0;
}
