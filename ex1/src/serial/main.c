/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : main.c
* Creation Date : 30-10-2012
* Last Modified : Mon 05 Nov 2012 06:22:27 PM EET
* Created By : Greg Liras <gregliras@gmail.com>
* Created By : Alex Maurogiannis <nalfemp@gmail.com>
_._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <stdlib.h>


int main(void)
{
    int i,j,k;
    int N;
    double **A;
    double l;

    /*
     * Allocate me!
     */

    scanf("%d\n", &N);

    A = malloc(N*sizeof(double*));
    for (k = 0; k < N; k++)
    {
        A[k] = malloc(N*sizeof(double));
    }

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            scanf("%lf", &A[i][j]);
        }
    }



    for (k = 0; k < N - 1; k++)
    {
        for (i = k + 1; i < N; i++)
        {
            l = A[i][k] / A[k][k];
            for (j = k; j < N; j++)
            {
                A[i][j] = A[i][j] -l*A[k][j];
            }
        }
    }

    //for (i = 0; i < N; i++)
    //{
    //    for (j = 0; j < N; j++)
    //    {
    //        printf("%lf\t", A[i][j]);
    //    }
    //    printf("\n");
    //}

    return 0;
}
