/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : common.h
* Creation Date : 06-11-2012
* Last Modified : Tue 06 Nov 2012 03:05:34 PM EET
* Created By : Greg Liras <gregliras@gmail.com>
_._._._._._._._._._._._._._._._._._._._._.*/

#ifndef DEBUG_FUNC
#define DEBUG_FUNC

#if main_DEBUG
#define debug(fmt,arg...)     fprintf(stdout, "%s: " fmt, __func__ , ##arg)
#else
#define debug(fmt,arg...)     do { } while(0)
#endif /* main_DEBUG */

#endif /* DEBUG_FUNC */

#ifndef COMMON_H
#define COMMON_H



#include <stdlib.h>
#include <stdio.h>

double *allocate_2d(int N, int M);
double *allocate_2d_with_padding(int N, int M, int max_rank);
double *parse_matrix_2d(FILE *fp, int N, int M, double *A);
void fprint_matrix_2d(FILE *fp, int N, int M, double *A);
void print_matrix_2d(int N, int M, double *A);

#endif /* COMMON_H */
