#ifdef __CUDA_ARCH__
#undef __CUDA_ARCH__
#endif
#define __CUDA_ARCH__ 130
#ifndef CUDA_2
#define BOUNDS(a,b,c,d) __launch_bounds__( (a)*(b)*(c), d )
#else 
#define BOUNDS(a,b,c,d) 
#endif 
#define __global   
#define __private  
#define __local    
#define __constant 
#ifndef M_PI 
#define M_PI 3.14159265358979323846
#endif  
typedef unsigned int uint; 
extern "C" {
__global__ void swan_fast_fill( uint4 *ptr, int len ) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if( idx<len) {
		ptr[idx] = make_uint4( 0,0,0,0 );
	}
}

__global__ void swan_fast_fill_word( uint *ptr, int len ) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if( idx<len) {
		ptr[idx] = 0;
	}
}




__global__ void canary( int N ) {
//	int idx = threadIdx.x + blockDim.x * blockIdx.x;
//	if( idx < N ) {
//		out[idx] = in[idx];
//	}
}


}
