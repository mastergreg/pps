/*
 *  error.c -- Error handling routines
 *
 *  Copyright (C) 2010-2012, Computing Systems Laboratory (CSLab)
 *  Copyright (C) 2010-2012, Vasileios Karakasis
 */ 
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error.h"
#include <CL/opencl.h>

#define ERRMSG_LEN  4096

char    *program_name = NULL;

static void
do_error(int use_errno, const char *fmt, va_list arg)
{
    char    errmsg[ERRMSG_LEN];
    
    *errmsg = '\0';   /* ensure initialization of errmsg */

    if (program_name)
        snprintf(errmsg, ERRMSG_LEN, "%s: ", program_name);

    vsnprintf(errmsg + strlen(errmsg), ERRMSG_LEN, fmt, arg);
    if (use_errno && errno)
        /* errno is set */
        snprintf(errmsg + strlen(errmsg), ERRMSG_LEN,
                 ": %s", strerror(errno));

    strncat(errmsg, "\n", ERRMSG_LEN);
    fflush(stdout);     /* in case stdout and stderr are the same */
    fputs(errmsg, stderr);
    fflush(NULL);
    return;
}

void
set_program_name(char *path)
{
    if (!program_name)
        program_name = strdup(path);
    if (!program_name)
        error(1, "strdup failed");
}

void
warning(int use_errno, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_error(use_errno, fmt, ap);
    va_end(ap);
    return;
}

void
error(int use_errno, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_error(use_errno, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void
fatal(int use_errno, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_error(use_errno, fmt, ap);
    va_end(ap);
    abort();
    exit(EXIT_FAILURE);     /* should not get here */
}

int cl_error(int code)
{
    switch(code) {
        case CL_DEVICE_NOT_FOUND:                printf("CL_DEVICE_NOT_FOUND \n"); break;
        case CL_DEVICE_NOT_AVAILABLE:            printf("CL_DEVICE_NOT_AVAILABLE \n"); break;
        case CL_COMPILER_NOT_AVAILABLE:          printf("CL_COMPILER_NOT_AVAILABLE \n"); break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:   printf("CL_MEM_OBJECT_ALLOCATION_FAILURE\n"); break;
        case CL_OUT_OF_RESOURCES:                printf("CL_OUT_OF_RESOURCES \n"); break;
        case CL_OUT_OF_HOST_MEMORY:              printf("CL_OUT_OF_HOST_MEMORY\n"); break;
        case CL_PROFILING_INFO_NOT_AVAILABLE:    printf("CL_PROFILING_INFO_NOT_AVAILABLE\n"); break;
        case CL_MEM_COPY_OVERLAP:                printf("CL_MEM_COPY_OVERLAP\n"); break;
        case CL_IMAGE_FORMAT_MISMATCH:           printf("CL_IMAGE_FORMAT_MISMATCH\n"); break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:      printf("CL_IMAGE_FORMAT_NOT_SUPPORTED \n"); break;
        case CL_BUILD_PROGRAM_FAILURE:           printf("CL_BUILD_PROGRAM_FAILURE \n"); break;
        case CL_MAP_FAILURE:                     printf("CL_MAP_FAILURE \n"); break;
        case CL_INVALID_VALUE:                   printf("CL_INVALID_VALUE \n"); break;
        case CL_INVALID_DEVICE_TYPE:             printf("CL_INVALID_DEVICE_TYPE \n"); break;
        case CL_INVALID_PLATFORM:                printf("CL_INVALID_PLATFORM \n"); break;
        case CL_INVALID_DEVICE:                  printf("CL_INVALID_DEVICE \n"); break;
        case CL_INVALID_CONTEXT:                 printf("CL_INVALID_CONTEXT \n"); break;
        case CL_INVALID_QUEUE_PROPERTIES:        printf("CL_INVALID_QUEUE_PROPERTIES \n"); break;
        case CL_INVALID_COMMAND_QUEUE:           printf("CL_INVALID_COMMAND_QUEUE \n"); break;
        case CL_INVALID_HOST_PTR:                printf("CL_INVALID_HOST_PTR \n"); break;
        case CL_INVALID_MEM_OBJECT:              printf("CL_INVALID_MEM_OBJECT \n"); break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: printf("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR \n"); break;
        case CL_INVALID_IMAGE_SIZE:              printf("CL_INVALID_IMAGE_SIZE \n"); break;
        case CL_INVALID_SAMPLER:                 printf("CL_INVALID_SAMPLER \n"); break;
        case CL_INVALID_BINARY:                  printf("CL_INVALID_BINARY \n"); break;
        case CL_INVALID_BUILD_OPTIONS:           printf("CL_INVALID_BUILD_OPTIONS \n"); break;
        case CL_INVALID_PROGRAM:                 printf("CL_INVALID_PROGRAM \n"); break;
        case CL_INVALID_PROGRAM_EXECUTABLE:      printf("CL_INVALID_PROGRAM_EXECUTABLE \n"); break;
        case CL_INVALID_KERNEL_NAME:             printf("CL_INVALID_KERNEL_NAME \n"); break;
        case CL_INVALID_KERNEL_DEFINITION:       printf("CL_INVALID_KERNEL_DEFINITION \n"); break;
        case CL_INVALID_KERNEL:                  printf("CL_INVALID_KERNEL \n"); break;
        case CL_INVALID_ARG_INDEX:               printf("CL_INVALID_ARG_INDEX \n"); break;
        case CL_INVALID_ARG_VALUE:               printf("CL_INVALID_ARG_VALUE \n"); break;
        case CL_INVALID_ARG_SIZE:                printf("CL_INVALID_ARG_SIZE \n"); break;
        case CL_INVALID_KERNEL_ARGS:             printf("CL_INVALID_KERNEL_ARGS \n"); break;
        case CL_INVALID_WORK_DIMENSION:          printf("CL_INVALID_WORK_DIMENSION \n"); break;
        case CL_INVALID_WORK_GROUP_SIZE:         printf("CL_INVALID_WORK_GROUP_SIZE \n"); break;
        case CL_INVALID_WORK_ITEM_SIZE:          printf("CL_INVALID_WORK_ITEM_SIZE \n"); break;
        case CL_INVALID_GLOBAL_OFFSET:           printf("CL_INVALID_GLOBAL_OFFSET \n"); break;
        case CL_INVALID_EVENT_WAIT_LIST:         printf("CL_INVALID_EVENT_WAIT_LIST \n"); break;
        case CL_INVALID_EVENT:                   printf("CL_INVALID_EVENT \n"); break;
        case CL_INVALID_OPERATION:               printf("CL_INVALID_OPERATION \n"); break;
        case CL_INVALID_GL_OBJECT:               printf("CL_INVALID_GL_OBJECT \n"); break;
        case CL_INVALID_BUFFER_SIZE:             printf("CL_INVALID_BUFFER_SIZE \n"); break;
        case CL_INVALID_MIP_LEVEL:               printf("CL_INVALID_MIP_LEVEL \n"); break;
        case CL_INVALID_GLOBAL_WORK_SIZE:        printf("CL_INVALID_GLOBAL_WORK_SIZE \n"); break;
    }
    return code;
}
