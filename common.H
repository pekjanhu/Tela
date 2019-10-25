/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef COMMON_H

#ifdef __GNUC__
#  pragma interface
#endif
// Common definitions for Tela kernel

#ifdef VECTOR_MACHINE
#  define SUPPORT_LOOP_TILING 0
#else
#  define SUPPORT_LOOP_TILING 1
#  define TILE_BLOCKSIZE 512
#endif

/* The TILE_BLOCKSIZE is the length of tile vectors. It should be smaller than
   cachesize/(8*n), where cachesize is in bytes and n is the maximum expected number
   of data vectors in one tiled region (e.g., n=20 might be a good guess).
   If TILE_BLOCKSIZE is too small (smaller than 200, say), interpreter overhead
   starts to reduce performance. If it is too large, the tiles will not fit in
   the cache, and performance is again reduced. For AMD Athlon-Thunderbird/700MHz
   with 256 kB cache, TILE_BLOCKSIZE=512 or 1024 seems to be optimal.
   The default is 512 which should not be too bad for machines with smaller caches
   also. */

struct global {
#if SLOW_FLOATING_POINT_MATH
	static int nops;		// Accumulates number of operations involved when executing current instruction
#else
	static double nops;		// Accumulates number of operations involved when executing current instruction
#endif
	static int lineno;		// Keeps track of source line number when parsing
	static int NTprg;		// Total number of Tprg objects in the system
	static int NTobject;	// Total number of Tobject's in the system
	static int interrupted;	// Nonzero after Control-C of Control-G hit by user
	static int debugging;	// Nonzero after Control-G hit by user
	static char *argv0;		// Name of the program (argv[0] from main())
    static ostream* ctfile;	// Points to output C-tela file when compiling (usually null)
	static size_t memuse;	// Sum of the sizes of allocated memory blocks (using operator new), or 0 if not available
	static size_t memalloc;	// memuse + fragmentation overhead, or 0 if not available
	static unsigned int Nalloc;		// Number of allocated blocks, or 0 if not available
	static unsigned int Nmem;		// Total number of new operator calls, or 0 if not available
	static unsigned int FLlen;		// Freelist length
#if !HAVE_RUSAGE && HAVE_SYS_PROCFS_H
	static int procfd;				// File descriptor to open /proc/$$ file (used in CPUSeconds())
#endif
#if SUPPORT_LOOP_TILING
	static int tile_in_use;		// 1 if we are in tiled loop, 0 otherwise
	static Tint tile_start;		// i=tile_start..tile_end in flat operation loops
	static Tint tile_end;
	static Tint tile_i;			// runs from 0..Ntiles-1, number of currently executing tile in current VLOOP..ENDV region
	static Tint tile_Ntiles;	// number of tiles in current VLOOP..ENDV region
	static Tint tile_arraylength;	// length of array participating in VLOOP..ENDV region
#endif
};

struct flags {		// Various flags controlled from command line
	static int verbose;
	static int echo;
	static int batch;
	static int silent;
	static int checkRO;		// nonzero if check agains modification of input arguments is done
	static int debugquery;	// nonzero if pressing Control-C causes a query whether to enter debug level
	static int optimize;	// nonzero if we try to optimize flatcode
	static int profiling;	// nonzero if we are doing profiling (tela -p)
};

extern void SetHardInterruptHandler();		// from tela.C
extern void SetSoftInterruptHandler();		// from tela.C
extern void SetControlGInterruptHandler();	// from tela.C

/* Wrap very time consuming calls with CATCH_INTERRUPTS, for example
   the Level-3 BLAS routines so they can be interrupted by the user
   by hitting Control-C. Code wrapped with CATHC_INTERRUPTS should
   not have any new/delete/malloc/free calls. It should only contain
   straightforward code that can be interrupted at any point. */
#define CATCH_INTERRUPTS(code)\
    SetHardInterruptHandler();\
	code;\
	SetSoftInterruptHandler()

#define COMMON_H

#endif
