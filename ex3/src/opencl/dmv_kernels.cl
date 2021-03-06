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
                    __global const value_t *x, __global value_t *y, uint n, 
                    __local value_t *pProd)
{
    __private value_t product = 0;

    for(uint i = get_group_id(0); i < n; i += get_num_groups(0)) {
        const __global value_t *row = &a[i*n];
        product = 0;
        for(uint j = get_local_id(0); j < n; j+= get_local_size(0)) {
            product += row[j] * x[j];
        }
        pProd[get_local_id(0)] = product;


        /* Logarithmic Parallel Reduction */
        for(uint mystep = get_local_size(0) >> 1; mystep > 0; mystep >>= 1) {
            barrier(CLK_LOCAL_MEM_FENCE);

            if(get_local_id(0) < mystep) {
                pProd[get_local_id(0)] += pProd[get_local_id(0) + mystep];
            }
        }
        if(get_local_id(0) == 0) {
            y[i] = pProd[0]; }


        /* Serial Reduction
        if (get_local_id(0) == 0) {
            value_t final_prod = 0;
            for (uint t = 0; t < get_local_size(0); ++t){
                final_prod += pProd[t];
            }
            y[i] = final_prod;
        }
        */

        barrier(CLK_LOCAL_MEM_FENCE);
    }
}

__kernel void shmem(__global const value_t *a, \
                    __global  const value_t *x,__global value_t *y, uint n,
                    uint w, __local value_t *pProd, __local value_t * x_loc)
{
    if (get_global_id(0) < n) {
        /* W represents the number of elements to be prefetched from x */
        uint W;

        /* When the distribution isn't evenly divisible in blocks, 
         * W needs to be reduced for the last block */
        if ((n % get_local_size(0) != 0) && 
                 ((get_global_id(0) / get_local_size(0)) 
                          == (n / get_local_size(0)))) {
            uint leftover = n % get_local_size(0);
            W = w < leftover ? w : leftover;
        } else {
            W = w;
        }

        pProd[get_local_id(0)] = 0;

        /* Each thread processes elements from one row in chunks of W */
        for (uint k = 0; k < n; k += W) {
                /* Prefetch W elements of x in local memory each iteration*/
                if (get_local_id(0) < W){
                    x_loc[get_local_id(0)] = x[k + get_local_id(0)];
                }
                barrier(CLK_LOCAL_MEM_FENCE);

                /* Calculate the product for W (or less) elements */
                for (uint j = 0; (j < W) && (k+j < n); j++) {
                    pProd[get_local_id(0)] += 
                            a[get_global_id(0) + (k + j) * n] * x_loc[j];
                }
                barrier(CLK_LOCAL_MEM_FENCE);
        }
        y[get_global_id(0)] = pProd[get_local_id(0)];
    }
}
