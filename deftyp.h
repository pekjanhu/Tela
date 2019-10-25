/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1996 Pekka Janhunen
 */

/* This file contains definitions that could as well be in def.H.
   However, def.H contains C++ constructs and so can not be included
   from C compilation. The files vecfftpack.c and fftpack.c are
   compiled using C compiler to avoid C++ compiler bugs on some cases. */

typedef int Tint;			/* the user-visible integer type */
typedef double Treal;		/* the user-visible real type */

