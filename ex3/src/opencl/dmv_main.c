// vim: set syntax=opencl:
/*
 *  dmv_main.cl -- DMV front-end program.
 *
 *  Copyright (C) 2010-2012, Computing Systems Laboratory (CSLab)
 *  Copyright (C) 2010-2012, Vasileios Karakasis
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <CL/opencl.h>
#include "alloc.h"
#include "dmv.h"
#include "error.h"
#include "timer.h"

#ifndef VALUES_MAX
#   define VALUES_MAX MAKE_VALUE_CONSTANT(1.)
#endif

#ifndef EPS
#   define EPS MAKE_VALUE_CONSTANT(1.e-6)
#endif

#ifndef NR_ITER
#   define NR_ITER 100
#endif

#define MAX_SOURCE_SIZE (0x100000)

static inline size_t min(size_t a, size_t b)
{
    return a < b ? a : b;
}

static void check_result(const value_t *test, const value_t *orig, size_t n)
{
    printf("Checking ... ");
    size_t  i_fail = vec_equals(test, orig, n, EPS);
    if (!i_fail) {
        printf("PASSED\n");
    } else {
        printf("FAILED (index: %ld)\n", i_fail - 1);
        printf("%" VALUE_FORMAT " != " "%" VALUE_FORMAT "\n",
               test[i_fail-1], orig[i_fail-1]);
    }
}

static void report_results(xtimer_t *timer, size_t n)
{
    double  elapsed_time = timer_elapsed_time(timer);
    size_t  flops        = 2*n*n*NR_ITER;

    printf("Elapsed time: %lf s\n", elapsed_time);
    printf("Performance:  %lf Gflop/s\n", flops*1.e-9 / elapsed_time);
}

static void print_usage()
{
    printf("Usage: [GPU_KERNEL=<kernel_no>] [GPU_BLOCK_SIZE=<size>] "
           "%s <matrix size>\n", program_name);
    printf("GPU_KERNEL defaults to 0\n");
    printf("GPU_BLOCK_SIZE defaults to 256\n");
    printf("Available kernels [id:name]:\n");
    size_t i;
    for (i = 0; i < GPU_KERNEL_END; ++i) {
        printf("\t%zd:%s\n", i, gpu_kernels[i].name);
    }
}

int main(int argc, char **argv)
{
    set_program_name(argv[0]);
    if (argc < 2) {
        warning(0, "too few arguments");
        print_usage();
        exit(EXIT_FAILURE);
    }

    uint n = (uint) atoi(argv[1]);
    if (!n)
        error(0, "invalid argument: %s", argv[1]);

    /* Read block size and kernel to launch from the environment */
    const char *env_gpu_kernel = getenv("GPU_KERNEL");
    const char *env_gpu_block_size = getenv("GPU_BLOCK_SIZE");
    int kern = (env_gpu_kernel) ? atoi(env_gpu_kernel) : GPU_NAIVE;
    int block_size = (env_gpu_block_size) ? atoi(env_gpu_block_size) : 256;
    size_t orig_n = n;  // original matrix size
    //int grid_size = 1;  // FILLME: compute the grid size

    /*
     *  FILLME: you can optionally adjust appropriately (increase
     *          only) the matrix size here if that helps you with your
     *          kernel code, e.g., to avoid divergent warps.
     */ 

    printf("Matrix size: %zd\n", orig_n);
    printf("Adjusted matrix size: %u\n", n);

    /*
     * Allocate the structures.
     * 
     * Initialization to zero is crucial if you adjusted the matrix
     * size.
     */
    value_t **A = (value_t **) calloc_2d(n, n, sizeof(**A));
    if (!A)
        error(1, "alloc_2d failed");

    value_t *x = (value_t *) calloc(n, sizeof(*x));
    if (!x)
        error(1, "malloc failed");

    value_t *y_serial = (value_t *) calloc(n, sizeof(*y_serial));
    if (!y_serial)
        error(1, "malloc failed");
    
    value_t *y = (value_t *) calloc(n, sizeof(*y));
    if (!y)
        error(1, "malloc failed");

    /* Initialize */
    srand48(0);
    mat_init_rand(A, orig_n, VALUES_MAX);
    vec_init_rand(x, orig_n, VALUES_MAX);
    vec_init(y_serial, orig_n, MAKE_VALUE_CONSTANT(0.0));
    vec_init(y, orig_n, MAKE_VALUE_CONSTANT(0.0));

    /* Setup timers */
    xtimer_t timer;

    /* Compute serial */
#ifdef SERIAL_KERNEL
    printf(">>>> Begin of record <<<<\n");
    printf("Serial version:\n");
    timer_clear(&timer);
    timer_start(&timer);
    for (size_t i = 0; i < NR_ITER; ++i)
        dmv_serial(A, x, y_serial, orig_n);
    timer_stop(&timer);
    report_results(&timer, orig_n);
    printf(">>>> End of record <<<<\n");
#endif  // SERIAL_KERNEL

#ifdef OPENMP_KERNEL
    /* Compute OpenMP */
    printf(">>>> Begin of record <<<<\n");
    printf("OpenMP version:\n");
    timer_clear(&timer);
    timer_start(&timer);
    for (size_t i = 0; i < NR_ITER; ++i)
        dmv_omp(A, x, y, orig_n);
    timer_stop(&timer);
#ifndef _NOCHECK_
    check_result(y, y_serial, orig_n);
#endif
    report_results(&timer, orig_n);
    printf(">>>> End of record <<<<\n");
#endif  // OPENMP_KERNEL

#ifdef GPU_KERNEL
    /*
     *  FILLME: Set up the blocks, grid and shared memory depending on
     *          the kernel. Make any transformations to the input
     *          matrix here.
     */ 
    cl_int errv = 0;
    cl_context context;
    cl_command_queue queue;
    cl_device_id device=NULL;
    cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_program program = NULL;
	cl_kernel kernel = NULL;


    FILE *fp;
	char fileName[] = "./dmv_kernels.cl";
	char *source_str;
	size_t source_size;
	
    /* Initialization Begin */
    // Platform
	errv = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    if (errv != CL_SUCCESS) {
        printf("Error getting platform id\n");
        exit(errv);
    }
    printf("Success getting platform id\n");
    // Device
    errv = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device, &ret_num_devices);
    if (errv != CL_SUCCESS) {
        printf("Error getting device ids\n");
        exit(errv);
    }
    printf("Success getting device ids\n");
    // Context
    context = clCreateContext(0, 1, &device, NULL, NULL, &errv);
    if (errv != CL_SUCCESS) {
        printf("Error creating context\n");
        exit(errv);
    }
    printf("Success creating context\n");
    // Command-queue
    queue = clCreateCommandQueue(context, device, 0, &errv);
    if (errv != CL_SUCCESS) {
        printf("Error creating command queue \n");
        exit(errv);
    }
    printf("Success creating command queue\n");
    /* Initialization Complete */

    size_t shmem_size = 0;  // FILLME: set up the shared memory size

    printf(">>>> Begin of record <<<<\n");
    //printf("Block size: %dx%d\n", gpu_block.x, gpu_block.y);
    //printf("Grid size : %dx%d\n", gpu_grid.x, gpu_grid.y);
    printf("Shared memory size: %ld bytes\n", shmem_size);

    /* GPU allocations */
    //value_t *gpu_A = (value_t *) gpu_alloc(n*n*sizeof(*gpu_A));

    cl_mem gpu_A = clCreateBuffer(context,  CL_MEM_READ_ONLY | \
            CL_MEM_ALLOC_HOST_PTR, n * n * sizeof(value_t), A, &errv);
    if (!gpu_A)
        error(0, "gpu_alloc failed: %s", errv);
    
    cl_mem gpu_x = clCreateBuffer(context,  CL_MEM_READ_ONLY | \
            CL_MEM_ALLOC_HOST_PTR, n * sizeof(value_t), x, &errv);
    if (!gpu_x)
        error(0, "gpu_alloc failed: %s", errv);

    vec_init(y, n, MAKE_VALUE_CONSTANT(0.0));
    cl_mem gpu_y = clCreateBuffer(context,  CL_MEM_WRITE_ONLY | \
            CL_MEM_ALLOC_HOST_PTR, n * sizeof(value_t), y, &errv);
    if (!gpu_y)
        error(0, "gpu_alloc failed: %s", errv);

    if (kern >= GPU_KERNEL_END)
        error(0, "the requested kernel does not exist");

    printf("GPU kernel version: %s\n", gpu_kernels[kern].name);

	/* Load the source code containing the kernels*/
    fp = fopen(fileName, "r");
        if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
	 
	/* Create Program from the source */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
                            (const size_t *)&source_size, &errv);
    if (!program)
        error(0, "Could not read program: %s", errv);

	/* Build Kernel Program */
	errv = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (errv != CL_SUCCESS) {
        printf("Error building kernel: %d\n", errv);
    }

    /* Generate Build Log */
    char * build_log;
    size_t log_size;
    errv = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, \
                            0, NULL, &log_size);
    build_log = (char *) malloc((log_size+1) * sizeof(char));
    errv |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                            log_size, build_log, NULL);
    if (errv != CL_SUCCESS) {
        printf("Error generating program build log\n");
        exit(errv);
    }
    build_log[log_size]= '\0';
    printf("\n%s\n", build_log);
    free(build_log);

	/* Create OpenCL Kernel */
	kernel = clCreateKernel(program, gpu_kernels[kern].name, &errv);

    /* Set the Kernel Arguments */
    errv = clSetKernelArg(kernel, 0, sizeof(cl_mem), &gpu_A);
    errv |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &gpu_x);
    errv |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &gpu_y);
    errv |= clSetKernelArg(kernel, 3, sizeof(uint), &n);
    if (errv != CL_SUCCESS) {
        printf("Error setting kernel arguments\n");
        exit(errv);
    }

    /* Calculate grid and block sizes */
    const size_t local_ws = min(block_size, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    size_t global_ws = n / local_ws;
    if (n % local_ws != 0) {
        global_ws++;
    }


    global_ws *= local_ws;
    printf("N: %u\n",n);
    printf("Global work-items: %lu\n", global_ws);
    printf("Local work-items: %lu\n", local_ws);
    printf("\n");
    
    /* Start Timing and Enqueue the kernel */
    timer_clear(&timer);
    timer_start(&timer);
    for (size_t i = 0; i < NR_ITER; ++i) {
        errv |= clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_ws, \
                &local_ws, 0, NULL, NULL);
    }
    timer_stop(&timer);

    if (errv != CL_SUCCESS) {
        printf("Error enqueuing kernel %d\n", errv);
        exit(errv);
    }
   
    /* Read the Result */
	errv = clEnqueueReadBuffer(queue, gpu_y, CL_TRUE, 0, \
                n * sizeof(value_t), y, 0, NULL, NULL);
    if (errv != CL_SUCCESS) {
        printf("Error enqueuing read buffer\n");
        exit(errv);
    }
    errv = clFlush(queue);
    if (errv != CL_SUCCESS) {
        printf("Error flushing queue\n");
        exit(errv);
    }

#ifndef _NOCHECK_
    check_result(y, y_serial, orig_n);
#endif

    report_results(&timer, orig_n);
    printf(">>>> End of record <<<<\n");

    /* Cleanup */
	errv = clFinish(queue);
	errv = clReleaseKernel(kernel);
	errv = clReleaseProgram(program);
	errv = clReleaseMemObject(gpu_A);
    errv = clReleaseMemObject(gpu_x);
	errv = clReleaseMemObject(gpu_y);
	errv = clReleaseCommandQueue(queue);
	errv = clReleaseContext(context);
	 
#endif  // GPU_KERNEL 
    /* Free resources on host */
    free_2d((void **) A);
    free(x);
    free(y);
    free(y_serial);

#ifdef GPU_KERNEL
    /* Free resources on GPU */
    //gpu_free(gpu_A);
    //gpu_free(gpu_x);
    //gpu_free(gpu_y);
#endif  // GPU_KERNEL 

    return EXIT_SUCCESS;
}
