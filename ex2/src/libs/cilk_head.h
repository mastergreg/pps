#ifndef CILK_HEAD_H
#define CILK_HEAD_H

#if USE_CILKC

#define SPAWN spawn
#define SYNC sync
#define ABORT abort
#define CILK cilk

#elif USE_GCC_CILK

#define SPAWN cilk_spawn
#define SYNC cilk_sync
#define ABORT 
#define CILK 

#endif
