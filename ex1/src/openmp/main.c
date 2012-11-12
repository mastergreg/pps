/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : main.c
* Creation Date : 30-10-2012
* Last Modified : Mon 12 Nov 2012 08:54:16 PM EET
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
    double divisor;
    double *A2;
    chunk = 1;

    sec = timer();

    for (k = 0; k < N - 1; k++)
    {
#pragma omp parallel private(divisor)
        {
            divisor = A[k * N + k];
#pragma omp for schedule(static, chunk) private(l,j, A2)
            for (i = k + 1; i < N; i++)
            {
                A2 = &A[i * N];

                l = A2[k] / divisor;
                for (j = k; j < N; j++)
                {
                    A2[j] = A2[j] - l * A[k * N + j];
                }
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
