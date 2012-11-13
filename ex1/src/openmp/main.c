/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : main.c
* Creation Date : 30-10-2012
* Last Modified : Tue 13 Nov 2012 12:02:29 PM EET
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

    Matrix *mat = get_matrix(argv[1]);
    N = mat->N;
    A = mat->A;



    int chunk = N/omp_get_max_threads();
    double *Ai;
    double *Ak;
    chunk = 1;

    sec = timer();

    for (k = 0; k < N - 1; k++)
    {
#pragma omp parallel private(Ak)
        {
            Ak = &A[k * N];
#pragma omp for schedule(static, chunk) private(l,j, Ai)
            for (i = k + 1; i < N; i++)
            {
                Ai = &A[i * N];

                l = Ai[k] / Ak[k];
                for (j = k; j < N; j++)
                {
                    Ai[j] = Ai[j] - l * Ak[j];
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
