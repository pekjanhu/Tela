/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

// Common definitions

#ifndef DEF_H

#include "deftyp.h"		/* deftyp.h contains part of the definitions */

#ifdef __GNUC__
#  if __GNUC__ >= 3
#    define USE_STRINGSTREAM 1
#  else
#    define USE_STRINGSTREAM 0
#  endif
#else
#  define USE_STRINGSTREAM 1
#endif

// ---------------------- User-editable ---------------------------

#define MAXRANK 6			/* maximum rank of array objects. */
    // Basic value is 4. You may increase it up to 8 just by
    // changing it here. Beyond 8 you also have to modify gatscat.C and numerics.C.
    // Increasing it from 4 may somewhat slow down performance and
    // will make Tela use more memory.

#ifdef UNICOS
#  define USE_MEMCPY 0
#else
#  define USE_MEMCPY 1
#endif
    // Define if your memcpy(3) function is good (possibly faster than for-loop)
    // Defining USE_MEMCPY also uses memset(3) for array zeroing

// ------------------- End of user-editable ------------------------

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cmath>
using namespace std;
#if HAVE_IEEEFP_H
#  include <ieeefp.h>
#endif
#if STDC_HEADERS || HAVE_STRING_H
#  include <cstring>
#  if !STDC_HEADERS && HAVE_MEMORY_H
#    include <memory.h>
#  endif
#else
#  include <strings.h>
#  define strchr index
#  define strrchr rindex
#  define memcpy(d,s,n) bcopy((s),(d),(n))
#  define memcmp(s1,s2,n) bcmp((s1),(s2),(n))
#  define memset(s,towhat,n) bzero((s),(n)); if (towhat) {cerr<<"bzero can only zero things\n"; exit(1);}
#endif

#ifdef CONVEX		/* CONVEX CC std header forgets to declare remove and strdup */
  extern "C" int remove(const char *);
  extern "C" char *strdup(const char *);
#endif

// MAXFILENAME will become max(1024,FILENAME_MAX)
#ifdef FILENAME_MAX
#  if FILENAME_MAX > 1024
#    define MAXFILENAME FILENAME_MAX
#  else
#    define MAXFILENAME 1024
#  endif
#else
#  define MAXFILENAME 1024
#endif

// ---------- Fundamental types and constants ----------

#if defined(LONGLONG_POINTERS)
typedef long long int TPtrInt;
#elif defined(LONG_POINTERS)
typedef long int TPtrInt;
#else
typedef int TPtrInt;
#endif
// TPtrInt must be integer large enough to accommodate a pointer (system dependent)
// in addition, it must not be smaller than Tint or int

typedef unsigned char Tchar;// use unsigned char for 8-bit chars
#if (defined(__CHAR_UNSIGNED__) && defined(OLD_ATT)) || defined(_UNICOS)
#  undef TCHAR_NOTEQUAL_CHAR
#else
#  define TCHAR_NOTEQUAL_CHAR
#endif

// Below, 'HPM' means 'hardware performance monitor' a' la Cray
typedef unsigned long int T_Ninstructions;		// 'HPM' instruction counter type
#if SLOW_FLOATING_POINT_MATH
typedef unsigned long T_Noperations;			// 'HPM' operation counter type
#else
typedef double T_Noperations;					// 'HPM' operation counter type
#endif
    // If double addition is reasonably fast on your system, use double for T_Noperations,
    // possibly also for T_Ninstructions.

#define ArrayBase 1		/* First array element has index 1. Don't change. */

#if USE_ATT_COMPLEX_CLASS
#  include <complex.h>
#  define Tcomplex complex
   typedef Tcomplex (*TComplexScalarFunction)(Tcomplex);
#else
   // The customized Tcomplex class
#  include "Tcomplex.H"
   typedef Tcomplex (*TComplexScalarFunction) (const Tcomplex&);
#endif

// The macro CPUSeconds() should return the CPU seconds used by the
// process as a Treal (or any floating type) number

#ifdef UNICOS

extern "C" double SECOND(void);
#define CPUSeconds() SECOND()

#elif HAVE_RUSAGE

extern "C" {
#include <sys/time.h>
#include <sys/resource.h>
#if HAVE_SYS_RUSAGE_H
#  include <sys/rusage.h>
#endif
#if HAVE_UNISTD_H
#  include <unistd.h>
#endif
}

//#if !defined(OSF1) && !defined(__linux)
//extern "C" int getrusage(int, struct rusage *);
//#endif
#ifndef RUSAGE_SELF
#  define RUSAGE_SELF 0
#endif

extern struct rusage struct_ru;	// This is declared here and defined in error.C

// Some Sun versions do not have the member tv_usec in struct timeval, but
// instead have tv_nsec. Autoconf has hopefully noticed this...
#if TIMEVAL_HAS_USEC
#  define CPUSeconds1() 1E-6*(struct_ru.ru_utime.tv_usec + struct_ru.ru_stime.tv_usec)
#elif TIMEVAL_HAS_NSEC
#  define CPUSeconds1() 1E-9*(struct_ru.ru_utime.tv_nsec + struct_ru.ru_stime.tv_nsec)
#else
#  define CPUSeconds1() 0
#endif

#define CPUSeconds() \
	(getrusage(RUSAGE_SELF,&struct_ru),\
  	 struct_ru.ru_utime.tv_sec + struct_ru.ru_stime.tv_sec + CPUSeconds1())

#elif HAVE_SYS_PROCFS_H && HAVE_UNISTD_H	/* SysVr4:ish procfs based CPUSeconds() macro */

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/procfs.h>
#include <unistd.h>
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

extern prstatus_t struct_pr; // This is declared here and defined in error.C

#define CPUSeconds() \
      (ioctl(global::procfd,PIOCSTATUS,&struct_pr),\
       struct_pr.pr_utime.tv_sec+struct_pr.pr_stime.tv_sec +\
       1e-9*(struct_pr.pr_utime.tv_nsec+struct_pr.pr_stime.tv_nsec))

#elif HAVE_TIME_H && HAVE_CLOCKS_PER_SEC		/* HPUX must use clock() */

// clock() function probably exists on all Unix systems, but it may wrap around,
// so we try to avoid using it if other methods are available.
#include <time.h>
#define CPUSeconds() (clock()/(float)CLOCKS_PER_SEC)

#else		/* Last resort: dummy CPUSeconds() */

#define CPUSeconds() 0

#endif

#ifdef UNICOS
#  define VECTORIZED _Pragma("_CRI ivdep");
#  define NOVECTOR _Pragma("_CRI novector");
#else
#  define VECTORIZED
#  define NOVECTOR
#endif

// MAXTINT should be the largest possibly Tint quantity
#ifndef MAXTINT
#  define MAXTINT (~(1 << (8*sizeof(Tint)-1)))
#endif

// Some compilers (most notably Cfront) cannot inline complicated functions.
// Such functions are marked INLINE.
#if defined(__GNUC__) || defined(UNICOS)
#  define INLINE inline
#else
#  define INLINE
#endif

// ---------- Commonly used inlines and macros ----------

inline int min(int x, int y) {return (x < y) ? x : y;}
inline Treal min(Treal x, Treal y) {return (x < y) ? x : y;}

inline int max(int x, int y) {return (x > y) ? x : y;}
inline Treal max(Treal x, Treal y) {return (x > y) ? x : y;}

//inline int round(Treal x) {return (x < 0) ? (int)(x-0.5) : (int)(0.5+x);}
inline int iround(Treal x) {return (x < 0) ? (int)(x-0.5) : (int)(0.5+x);}

#ifdef TCHAR_NOTEQUAL_CHAR
inline size_t strlen(const Tchar *s) {return strlen((const char*)s);}
inline Tchar *strchr(const Tchar*s, int c) {return (Tchar*)strchr((const char*)s,c);}
inline Tchar *strrchr(const Tchar*s, int c) {return (Tchar*)strrchr((const char*)s,c);}
inline Tchar* strstr(const Tchar*haystack, const Tchar*needle) {return (Tchar*)strstr((const char*)haystack,(const char*)needle);}
inline Tchar *strcpy(Tchar*dest, const Tchar*src) {return (Tchar*)strcpy((char*)dest,(const char*)src);}
inline Tchar *strcat(Tchar*dest, const Tchar*src) {return (Tchar*)strcat((char*)dest,(const char*)src);}
inline int strcmp(const Tchar*s1, const Tchar*s2) {return strcmp((const char*)s1,(const char*)s2);}
inline Tchar *strdup(const Tchar*s) {return (Tchar*)strdup((const char*)s);}
#if defined(BROKEN_UCHAR_OUTPUTTER)
  inline ostream& operator<<(ostream& o, const Tchar tstr[]) {return o << (const char*)tstr;}
#endif
#endif

#ifdef TELASPECIFIC_NEWDELETE

// Our own new & delete operators are defined in tela.C
extern void *operator new(size_t sz);
extern void operator delete(void *ptr);

#endif

#define exit(x) _exit(x)	/* When we want to exit, it is not necessary to try to call global destructors */

// MAX_TELAPATH_LENGTH is the maximum number of directory components
// in TELAPATH; defined in tela.C
extern const size_t MAX_TELAPATH_LENGTH;
extern Tchar* TelaPath[];
extern size_t TelaPathLength;

#define DEF_H

#endif
