/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : main.c
* Creation Date : 30-10-2012
* Last Modified : Thu 20 Dec 2012 01:27:37 PM EET
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
    double **A2D;
    int flag = 1;

    double l;
    double sec;

    FILE *fp = NULL;
    usage(argc, argv);
    /*
     * Allocate me!
     */

    Matrix *mat = get_matrix(argv[1],0, CONTINUOUS);
    N = mat->N;
    A = mat->A;
    A2D = appoint_2D(A, N, N);



    int chunk = N/omp_get_max_threads();
    double *Ai;
    double *Ak;
    chunk = 1;

    sec = timer();

    for (k = 0; k < N - 1; k++)
    {
#pragma omp parallel private(Ak)
        {
            if(flag)
            {
                printf("%d %d\n", omp_get_num_threads(), omp_get_max_threads());
                flag=0;
            }
            Ak = A2D[k];
#pragma omp for schedule(static, chunk) private(l,j, Ai)
            for (i = k + 1; i < N; i++)
            {
                Ai = A2D[i];

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
