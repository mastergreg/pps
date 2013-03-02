// vim: set syntax=opencl:
/*
 *  kernels.cl : OpenCL kernels for DMV multiplication
 */ 

/*
 *  Naive kernel
 */ 
__kernel void dmv_gpu_naive(__global const value_t *a,
                    __global const value_t *x, __global value_t *y, size_t n)
{
    /*
     * FILLME: fill the code for the naive kernel.
     */ 
}

/*
 *  Coalesced memory acceses
 */
__kernel void dmv_gpu_coalesced(__global const value_t *a,
                    __global const value_t *x, __global value_t *y, size_t n)
{
    /*
     * FILLME: fill the code for the coalesced kernel.
     */ 
}

/*
 *  Use of shared memory
 */
__kernel void dmv_gpu_shmem(__global const value_t *a,
                    __global  const value_t *x,__global value_t *y, size_t n)
{
    /*
     * FILLME: fill the code for the shared memory kernel.
     */ 
}
