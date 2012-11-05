/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Mon 05 Nov 2012 10:07:48 AM EET
 * Created By : Greg Liras <gregliras@gmail.com>
 * Created By : Alex Maurogiannis <nalfemp@gmail.com>
 _._._._._._._._._._._._._._._._._._._._._.*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <signal.h>
#include <unistd.h>

#if main_DEBUG
#define debug(fmt,arg...)     printf("%s: " fmt, __func__ , ##arg)
#else
#define debug(fmt,arg...)     do { } while(0)
#endif

#define NTHREADS 2

static double *allocate_2d(int N, int M)
{
    double *A;
    A = malloc(N*M*sizeof(double));
    return A;
}

static double *parse_matrix_2d(FILE *fp, int N, int M, double *A)
{
    int i,j;
    double *p;
    p = A;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            if(!fscanf(fp, "%lf", p++)) {
                return NULL;
            }
        }
    }
    return A;
}

static void print_matrix_2d(int N, int M, double *A)
{
    int i,j;
    double *p;
    p = A;
    for (j = 0; j < M; j++) {
        printf("=");
    }
    printf("\n");
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            printf("%lf ", *p++);
        }
        printf("\n");
    }
    for (j = 0; j < M; j++) {
        printf("=");
    }
    printf("\n");
}




int main(int argc, char **argv)
{
    int j,k;
    int N;
    double l;
    int rank;
    double *Ak = NULL;
    double *A = NULL;
    double *Ai = NULL;


    int ret = 0;
    FILE *fp = NULL;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        debug("rank: %d opens file: %s\n", rank, argv[1]);
        fp = fopen(argv[1], "r");
        if(fp) {
            if(!fscanf(fp, "%d\n", &N)) {
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        else {
            raise(SIGKILL);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);


    Ak = malloc(N*sizeof(double)); // Buffer for broadcasting the k-th row
    Ai = malloc(N*sizeof(double)); // Buffer for scattering the i-th row

    if (rank == 0) {
        /* Root Allocates the whole table */
        A = allocate_2d(N, N);
        if(!parse_matrix_2d(fp, N, N, A)) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fclose(fp);
        fp = NULL;
    } 



    for (k = 0; k < N - 1; k++) {
        if (rank == 0) {
            Ak = &A[k*N];
            debug("k %d\n", k);
        }
        /* Send everyone the k-th row, and scatter the rest of the table */
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Bcast(Ak, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Scatter(&A[N * (k + 1)], N, MPI_DOUBLE, Ai, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

#if main_DEBUG
        if(rank == 0) {
            print_matrix_2d(N, N, A);
        }
#endif

        // every run has the same k but they have different ranks
        l = Ai[k + rank] / Ak[k + rank];
        for (j = k + rank; j < N; j++) {
            Ai[j] = Ai[j] -l*Ak[j];
        }

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gather(Ai, N , MPI_DOUBLE, &(A[N * (k + 1)]), N, MPI_DOUBLE, 0 ,MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }
#if MAIN_DEBUG
    if(rank == 0) {
        print_matrix_2d(N, N, A);
    }
#endif

    MPI_Barrier(MPI_COMM_WORLD);

    ret = MPI_Finalize();
    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }


    if(rank == 0) {
        sleep(1);
        print_matrix_2d(N, N, A);
    }

    return 0;
}
