/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : main.c
* Creation Date : 30-10-2012
* Last Modified : Mon 12 Nov 2012 08:09:14 PM EET
* Created By : Greg Liras <gregliras@gmail.com>
* Created By : Alex Maurogiannis <nalfemp@gmail.com>
_._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


#include "common.h"

int main(int argc, char **argv)
{
    int i,j,k;
    int N;
    double *A;
    double l;
    double sec;

    FILE *fp = NULL;
    usage(argc, argv);
    /*
     * Allocate me!
     */
    fp = fopen(argv[1], "r");
    if(fp) {
        if(!fscanf(fp, "%d\n", &N)) {
            exit(EXIT_FAILURE);
        }
    }

    if((A = allocate_2d(N, N)) == NULL) {
        exit(EXIT_FAILURE);
    }
    if(parse_matrix_2d(fp, N, N, A) == NULL) {
        exit(EXIT_FAILURE);
    }


    int chunk = N/omp_get_max_threads();

    sec = timer();

    for (k = 0; k < N - 1; k++)
    {
#pragma omp parallel for schedule(static, chunk) private(l,j)
        for (i = k + 1; i < N; i++)
        {
            l = A[i * N + k] / A[k * N + k];
            for (j = k; j < N; j++)
            {
                A[i * N + j] = A[i * N + j] - l * A[k * N + j];
            }
        }
    }
    sec = timer();
    printf("Calc Time: %lf\n", sec);

    fp = fopen(argv[2], "w");
    fprint_matrix_2d(fp, N, N, A);
    fclose(fp);
    free(A);

    return 0;
}
