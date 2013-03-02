// vim: set syntax=opencl:
/*
 *  kernels.cl : OpenCL kernels for DMV multiplication
 */ 

typedef float   value_t; 

/*
 *  Naive kernel
 */ 
__kernel void naive(__global const value_t *a, \
                    __global const value_t *x, __global value_t *y, uint n)
{
    uint i;
    const uint tid = get_global_id(0);
    
    if (tid <= n) {
        const __global value_t *row = &a[tid*n];
        __private value_t product = 0;
        for (i = 0; i < n; ++i)
            product += row[i] * x[i];
        y[tid] = product;
    }
}

/*
 *  Coalesced memory acceses
 */
__kernel void coalesced(__global const value_t *a, \
                    __global const value_t *x, __global value_t *y, uint n)
{
    /*
     * FILLME: fill the code for the coalesced kernel.
     */ 
}

/*
 *  Use of shared memory
 */
__kernel void shmem(__global const value_t *a, \
                    __global  const value_t *x,__global value_t *y, uint n)
{
    /*
     * FILLME: fill the code for the shared memory kernel.
     */ 
}
