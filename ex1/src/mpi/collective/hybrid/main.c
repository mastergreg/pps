/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
 * File Name : main.c
 * Creation Date : 30-10-2012
 * Last Modified : Thu 29 Nov 2012 02:45:14 PM EET
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

/* Returns the displacements table in rows */
void get_displs(int *counts, int max_rank, int *displs) 
{
    int j;
    displs[0] = 0;
    for (j = 1; j < max_rank ; j++) {
        displs[j] = displs[j - 1] + counts[j - 1];
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

void process_rows(int k, int rank, int N, int workload, int max_rank, \
        double **Ap2D, double *Ak)
{
    /*      performs the calculations for a given set of rows.
     *      In this hybrid version each thread is assigned blocks of 
     *      continuous rows in a cyclic manner.
     */
    int j, w;
    double l;
    int start;

    start = k / max_rank;
    /* If you have broadcasted, dont do calculations for that row */
    if (rank <= (k % max_rank)){
        start++;
    }
    for (w = start; w < workload; w++) {
        l = Ap2D[w][k] / Ak[k];
        for (j = k; j < N; j++) {
            Ap2D[w][j] = Ap2D[w][j] - l * Ak[j];
        }
    }
}

void gather_to_root_cyclic(double **Ap2D, int max_rank, int rank, int root, double **A2D, int N, int M)
{
    int i;
    int bcaster;
    MPI_Status status;
    for(i = 0; i < N; i++) {
        bcaster = i % max_rank;
        if(rank == bcaster) {
            if(bcaster == root) {
                memcpy(A2D[i], Ap2D[i / max_rank], M*sizeof(double));
            }
            else {
                MPI_Send(Ap2D[i / max_rank], M, MPI_DOUBLE, 0, i, MPI_COMM_WORLD);
            }
        }
        else if (rank == root) {
            debug("%d %d\n", bcaster, i);
            MPI_Recv(A2D[i], M, MPI_DOUBLE, 0, i, MPI_COMM_WORLD, &status);
        }
    }
}


int main(int argc, char **argv)
{
    int k;
    int i;
    int N;
    int rank;
    int max_rank;
    int bcaster;
    int workload;
    int collect_index;
    int ret = 0;
    int *counts;
    int *displs;
    double *A = NULL;
    double **A2D = NULL;
    double *Ap = NULL;
    double **Ap2D = NULL;
    double *Ak = NULL;
    double sec = 0;
    FILE *fp = NULL;
    MPI_Datatype row_type;

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
        A2D = appoint_2D(A, N, N);
    }

    /* And broadcasts N */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    workload = (N / max_rank) + 1;

    /* Allocations */
    Ak = malloc(N * sizeof(double));
    Ap = malloc(workload * N * sizeof(double));
    counts = malloc(max_rank * sizeof(int));
    displs = malloc(max_rank * sizeof(int));

    /* Initializations */
    distribute_rows(max_rank, N, counts);
    get_displs(counts, max_rank, displs);
    Ap2D = appoint_2D(Ap, workload, N);
    collect_index = 0;

    /* Scatter the table to each thread's Ap */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_vector(1, N, N, MPI_DOUBLE, &row_type);
    MPI_Type_commit(&row_type);
    MPI_Scatterv(A, counts, displs, row_type, \
        Ap, workload, row_type, 0, MPI_COMM_WORLD);
    MPI_Type_free(&row_type);

    /* Start Timing */
    if(rank == 0) {
        sec = timer();
    }

    for (k = 0; k < N - 1; k++) {
        /* Find who owns the k-th row */
        bcaster = k % max_rank;

        /* The broadcaster puts what's needed of his k-th
         * row in the Ak buffer */
        if (rank == bcaster) {
            i = k / max_rank;
            memcpy(&Ak[k], &Ap2D[i][k], (N-k) * sizeof(double));
        }

        /* Everyone receives the k-th row */
        MPI_Bcast(&Ak[k], N-k, MPI_DOUBLE, bcaster, MPI_COMM_WORLD);


        /* And off you go to work. */
        process_rows(k, rank, N, workload, max_rank, Ap2D, Ak);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        sec = timer();
        printf("Calc Time: %lf\n", sec);
    }
    /* Broadcast the last row to root (TODO: we can change it to send, right?)*/
    /* Root collects all the broadcasts to fill the final matrix */


    /* Gather the table from each thread's Ap */
    gather_to_root_cyclic(Ap2D, max_rank, rank, 0, A2D, N, N);

    ret = MPI_Finalize();
    if(ret == 0) {
        debug("%d FINALIZED!!! with code: %d\n", rank, ret);
    }
    else {
        debug("%d NOT FINALIZED!!! with code: %d\n", rank, ret);
    }

    if (rank == 0) {
        upper_triangularize(N, A2D);
        fp = fopen(argv[2], "w");
        fprint_matrix_2d(fp, N, N, A);
        fclose(fp);
    }
    free(A);

    return 0;
}
