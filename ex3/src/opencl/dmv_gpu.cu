// vim: set syntax=opencl:
/*
 *  dmv_gpu.cu -- Template for DMV GPU kernels
 *
 *  Copyright (C) 2010-2013, Computing Systems Laboratory (CSLab)
 *  Copyright (C) 2010-2013, Vasileios Karakasis
 */ 
#include <stdio.h>
#include "dmv.h"

/*
 *  Utility function to get the thread ID within the
 *  global working space.
 */ 
__device__ int get_global_tid()
{
    return (gridDim.x*blockIdx.y + blockIdx.x)*blockDim.x*blockDim.y +
        blockDim.x*threadIdx.y + threadIdx.x;
}

/*
 *  Utility function to get the thread ID within the
 *  local/block working space.
 */ 
__device__ int get_local_tid()
{
    return blockDim.x*threadIdx.y + threadIdx.x;
}

/*
 *  Naive kernel
 */ 
__global__ void dmv_gpu_naive(const value_t *a, const value_t *x, value_t *y,
                              size_t n)
{
    /*
     * FILLME: fill the code for the naive kernel.
     */ 
}

/*
 *  Coalesced memory acceses
 */
__global__ void dmv_gpu_coalesced(const value_t *a, const value_t *x,
                                  value_t *y, size_t n)
{
    /*
     * FILLME: fill the code for the coalesced kernel.
     */ 
}

/*
 *  Use of shared memory
 */
__global__ void dmv_gpu_shmem(const value_t *a, const value_t *x, value_t *y,
                              size_t n)
{
    /*
     * FILLME: fill the code for the shared memory kernel.
     */ 
}
