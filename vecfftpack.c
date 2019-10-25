/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1996 Pekka Janhunen
 */

/*
  vecfftpack.c : A set of vectorized FFT routines (many transforms with one call).
  Algorithmically based on Fortran-77 FFTPACK by Paul N. Swarztrauber (Version 4, 1985).
  As compared to FFTPACK routines, these functions take three extra integer parameters:
  lot, step and jump.
  Lot is the number of transforms to perform with the call.
  Step is the address difference between consecutive components in each data vector.
  Jump is the address difference between corresponding elements of consecutive
  vectors.
  These parameters are arbitrary and thus it is possible to take the transforms along
  rows or columns of a matrix.

  Assumption throughout is that sizeof(Tint) <= sizeof(Treal).
  E.g. on modern 64-bit RISC machines you cannot define Treal=float, Tint=long.
  This is checked for in the code.

  Compilation outside Tela: cc -DSTANDALONE vecfftpack.c.
  Vectorized compilation on Cray: cc -DUNICOS -DSTANDALONE vecfftpack.c.
  If you don't define STANDALONE it tries to include "deftyp.H".
  
  This file is also C++ compatible.
  
  Pekka Janhunen 23.2.1995
*/


/* isign is +1 for backward and -1 for forward transforms */

#include <math.h>
#include <stdio.h>

#ifdef STANDALONE
#  define Treal double
#  define Tint int
#else
#  include "deftyp.h"
#endif

#define ref(u,a) u[p*jump+(a)*step]
#define ref0(u,a) u[2*p*jump+2*((a)/2)*step+(a)%2]
#define ref1(u,a) u[2*p*jump+2*(((a)+1)/2)*step+((a)+1)%2]

#define MAXFAC 13

#define VECARGS_FORMAL , int lot, int step, int jump
#define VECARGS_ACTUAL ,lot,step,jump
#define VECVARS Tint p
#ifdef UNICOS
#  define VECLOOP _Pragma("_CRI ivdep"); for (p=0; p<lot; p++)
#else
#  define VECLOOP for (p=0; p<lot; p++)
#endif

static const Treal pi       = 3.14159265358979;
static const Treal twopi    = 6.28318530717959;
static const Treal taur     = -0.5;
static const Treal taui     = 0.866025403784439;		/* sqrt(3)/2 */
static const Treal tr11     = 0.309016994374947;		/* sin(pi/10) */
static const Treal ti11     = 0.951056516295154;		/* cos(pi/10) */
static const Treal tr12     = -0.809016994374947;		/* -cos(pi/5) */
static const Treal ti12     = 0.587785252292473;		/* sin(pi/5) */
static const Treal sqrt2    = 1.4142135623731;
static const Treal tsqrt2   = 2.82842712474619;			/* 2*sqrt(2) */
static const Treal invsqrt2 = 0.7071067811865475;		/* 1/sqrt(2) */
static const Treal sqrt3    = 1.73205080756888;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UNICOS
#pragma _CRI noinline passf2,passf3,passf4,passf5,passf
#pragma _CRI noinline radf2,radf3,radf4,radf5,radfg
#pragma _CRI noinline radb2,radb3,radb4,radb5,radbg
#pragma _CRI noinline cfftf1,rfftf1
#pragma _CRI inline cosqf1,cosqb1
#pragma _CRI inline factorize,rfftb1,rffti1,sint1
#endif

/* ----------------------------------------------------------------------
   passf2, passf3, passf4, passf5, passf. Complex FFT passes fwd and bwd.
   ---------------------------------------------------------------------- */

static void passf2(Tint ido, Tint l1, const Treal cc[], Treal ch[], const Treal wa1[], int isign VECARGS_FORMAL)
/* isign==+1 for backward transform */
{
	VECVARS;
    Tint i, k, ah, ac;
    Treal ti2, tr2;
    if (ido <= 2) {
		for (k=0; k<l1; k++) {
			ah = k*ido;
			ac = 2*k*ido;
			VECLOOP {
			ref0(ch,ah)           = ref0(cc,ac) + ref0(cc,ac + ido);
			ref0(ch,ah + ido*l1)  = ref0(cc,ac) - ref0(cc,ac + ido);
			ref1(ch,ah)           = ref1(cc,ac) + ref1(cc,ac + ido);
			ref1(ch,ah + ido*l1)  = ref1(cc,ac) - ref1(cc,ac + ido);
			}
		}
	} else {
		for (k=0; k<l1; k++) {
			for (i=0; i<ido-1; i+=2) {
				ah = i + k*ido;
				ac = i + 2*k*ido;
				VECLOOP {
				ref0(ch,ah)         = ref0(cc,ac) + ref0(cc,ac + ido);
				tr2                 = ref0(cc,ac) - ref0(cc,ac + ido);
				ref1(ch,ah)         = ref1(cc,ac) + ref1(cc,ac + ido);
				ti2                 = ref1(cc,ac) - ref1(cc,ac + ido);
				ref1(ch,ah+l1*ido)  = wa1[i]*ti2 + isign*wa1[i+1]*tr2;
				ref0(ch,ah+l1*ido)  = wa1[i]*tr2 - isign*wa1[i+1]*ti2;
				}
			}
		}
	}
} /* passf2 */


static void passf3(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				   const Treal wa1[], const Treal wa2[], int isign VECARGS_FORMAL)
/* isign==+1 for backward transform */
{
	VECVARS;
    Tint i, k, ac, ah;
    Treal ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
    if (ido == 2) {
		for (k=1; k<=l1; k++) {
			VECLOOP {
			ac = (3*k - 2)*ido;
			tr2 = ref0(cc,ac) + ref0(cc,ac + ido);
			cr2 = ref0(cc,ac - ido) + taur*tr2;
			ah = (k - 1)*ido;
			ref0(ch, ah) = ref0(cc,ac - ido) + tr2;

			ti2 = ref1(cc,ac) + ref1(cc,ac + ido);
			ci2 = ref1(cc,ac - ido) + taur*ti2;
			ref1(ch, ah) = ref1(cc,ac - ido) + ti2;

			cr3 = isign*taui*(ref0(cc,ac) - ref0(cc,ac + ido));
			ci3 = isign*taui*(ref1(cc,ac) - ref1(cc,ac + ido));
			ref0(ch, ah + l1*ido) = cr2 - ci3;
			ref0(ch, ah + 2*l1*ido) = cr2 + ci3;
			ref1(ch, ah + l1*ido) = ci2 + cr3;
			ref1(ch, ah + 2*l1*ido) = ci2 - cr3;
			}
		}
	} else {
		for (k=1; k<=l1; k++) {
			for (i=0; i<ido-1; i+=2) {
				VECLOOP {
				ac = i + (3*k - 2)*ido;
				tr2 = ref0(cc,ac) + ref0(cc,ac + ido);
				cr2 = ref0(cc,ac - ido) + taur*tr2;
				ah = i + (k-1)*ido;
				ref0(ch, ah) = ref0(cc,ac - ido) + tr2;
				ti2 = ref1(cc,ac) + ref1(cc,ac + ido);
				ci2 = ref1(cc,ac - ido) + taur*ti2;
				ref1(ch, ah) = ref1(cc,ac - ido) + ti2;
				cr3 = isign*taui*(ref0(cc,ac) - ref0(cc,ac + ido));
				ci3 = isign*taui*(ref1(cc,ac) - ref1(cc,ac + ido));
				dr2 = cr2 - ci3;
				dr3 = cr2 + ci3;
				di2 = ci2 + cr3;
				di3 = ci2 - cr3;
				ref1(ch, ah + l1*ido) = wa1[i]*di2 + isign*wa1[i+1]*dr2;
				ref0(ch, ah + l1*ido) = wa1[i]*dr2 - isign*wa1[i+1]*di2;
				ref1(ch, ah + 2*l1*ido) = wa2[i]*di3 + isign*wa2[i+1]*dr3;
				ref0(ch, ah + 2*l1*ido) = wa2[i]*dr3 - isign*wa2[i+1]*di3;
				}
			}
		}
	}
} /* passf3 */


static void passf4(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				   const Treal wa1[], const Treal wa2[], const Treal wa3[], int isign VECARGS_FORMAL)
/* isign == -1 for forward transform and +1 for backward transform */
{
	VECVARS;
    Tint i, k, ac, ah;
    Treal ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    if (ido == 2) {
		for (k=0; k<l1; k++) {
			VECLOOP {
			ac = 4*k*ido;
			ti1 = ref1(cc,ac) - ref1(cc,ac + 2*ido);
			ti2 = ref1(cc,ac) + ref1(cc,ac + 2*ido);
			tr4 = ref1(cc,ac + 3*ido) - ref1(cc,ac + ido);
			ti3 = ref1(cc,ac + ido) + ref1(cc,ac + 3*ido);
			tr1 = ref0(cc,ac) - ref0(cc,ac + 2*ido);
			tr2 = ref0(cc,ac) + ref0(cc,ac + 2*ido);
			ti4 = ref0(cc,ac + ido) - ref0(cc,ac + 3*ido);
			tr3 = ref0(cc,ac + ido) + ref0(cc,ac + 3*ido);
			ah = k*ido;
			ref0(ch, ah) = tr2 + tr3;
			ref0(ch, ah + 2*l1*ido) = tr2 - tr3;
			ref1(ch, ah) = ti2 + ti3;
			ref1(ch, ah + 2*l1*ido) = ti2 - ti3;
			ref0(ch, ah + l1*ido) = tr1 + isign*tr4;
			ref0(ch, ah + 3*l1*ido) = tr1 - isign*tr4;
			ref1(ch, ah + l1*ido) = ti1 + isign*ti4;
			ref1(ch, ah + 3*l1*ido) = ti1 - isign*ti4;
			}
		}
	} else {
		for (k=0; k<l1; k++) {
			for (i=0; i<ido-1; i+=2) {
				VECLOOP {
				ac = i + 1 + 4*k*ido;
				ti1 = ref0(cc,ac) - ref0(cc,ac + 2*ido);
				ti2 = ref0(cc,ac) + ref0(cc,ac + 2*ido);
				ti3 = ref0(cc,ac + ido) + ref0(cc,ac + 3*ido);
				tr4 = ref0(cc,ac + 3*ido) - ref0(cc,ac + ido);
				tr1 = ref0(cc,ac - 1) - ref0(cc,ac + 2*ido - 1);
				tr2 = ref0(cc,ac - 1) + ref0(cc,ac + 2*ido - 1);
				ti4 = ref0(cc,ac + ido - 1) - ref0(cc,ac + 3*ido - 1);
				tr3 = ref0(cc,ac + ido - 1) + ref0(cc,ac + 3*ido - 1);
				ah = i + k*ido;
				ref0(ch, ah) = tr2 + tr3;
				cr3 = tr2 - tr3;
				ref0(ch, ah + 1) = ti2 + ti3;
				ci3 = ti2 - ti3;
				cr2 = tr1 + isign*tr4;
				cr4 = tr1 - isign*tr4;
				ci2 = ti1 + isign*ti4;
				ci4 = ti1 - isign*ti4;
				ref0(ch, ah + l1*ido) = wa1[i]*cr2 - isign*wa1[i + 1]*ci2;
				ref0(ch, ah + l1*ido + 1) = wa1[i]*ci2 + isign*wa1[i + 1]*cr2;
				ref0(ch, ah + 2*l1*ido) = wa2[i]*cr3 - isign*wa2[i + 1]*ci3;
				ref0(ch, ah + 2*l1*ido + 1) = wa2[i]*ci3 + isign*wa2[i + 1]*cr3;
				ref0(ch, ah + 3*l1*ido) = wa3[i]*cr4 -isign*wa3[i + 1]*ci4;
				ref0(ch, ah + 3*l1*ido + 1) = wa3[i]*ci4 + isign*wa3[i + 1]*cr4;
				}
			}
		}
	}
} /* passf4 */


static void passf5(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				   const Treal wa1[], const Treal wa2[], const Treal wa3[], const Treal wa4[],
				   int isign VECARGS_FORMAL)
/* isign == -1 for forward transform and +1 for backward transform */
{
	VECVARS;
    Tint i, k, ac, ah;
    Treal ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, ti2, ti3,
	     ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
    if (ido == 2) {
		for (k = 1; k <= l1; ++k) {
			VECLOOP {
			ac = (5*k - 4)*ido + 1;
			ti5 = ref0(cc,ac) - ref0(cc,ac + 3*ido);
			ti2 = ref0(cc,ac) + ref0(cc,ac + 3*ido);
			ti4 = ref0(cc,ac + ido) - ref0(cc,ac + 2*ido);
			ti3 = ref0(cc,ac + ido) + ref0(cc,ac + 2*ido);
			tr5 = ref0(cc,ac - 1) - ref0(cc,ac + 3*ido - 1);
			tr2 = ref0(cc,ac - 1) + ref0(cc,ac + 3*ido - 1);
			tr4 = ref0(cc,ac + ido - 1) - ref0(cc,ac + 2*ido - 1);
			tr3 = ref0(cc,ac + ido - 1) + ref0(cc,ac + 2*ido - 1);
			ah = (k - 1)*ido;
			ref0(ch, ah) = ref0(cc,ac - ido - 1) + tr2 + tr3;
			ref0(ch, ah + 1) = ref0(cc,ac - ido) + ti2 + ti3;
			cr2 = ref0(cc,ac - ido - 1) + tr11*tr2 + tr12*tr3;
			ci2 = ref0(cc,ac - ido) + tr11*ti2 + tr12*ti3;
			cr3 = ref0(cc,ac - ido - 1) + tr12*tr2 + tr11*tr3;
			ci3 = ref0(cc,ac - ido) + tr12*ti2 + tr11*ti3;
			cr5 = isign*(ti11*tr5 + ti12*tr4);
			ci5 = isign*(ti11*ti5 + ti12*ti4);
			cr4 = isign*(ti12*tr5 - ti11*tr4);
			ci4 = isign*(ti12*ti5 - ti11*ti4);
			ref0(ch, ah + l1*ido) = cr2 - ci5;
			ref0(ch, ah + 4*l1*ido) = cr2 + ci5;
			ref0(ch, ah + l1*ido + 1) = ci2 + cr5;
			ref0(ch, ah + 2*l1*ido + 1) = ci3 + cr4;
			ref0(ch, ah + 2*l1*ido) = cr3 - ci4;
			ref0(ch, ah + 3*l1*ido) = cr3 + ci4;
			ref0(ch, ah + 3*l1*ido + 1) = ci3 - cr4;
			ref0(ch, ah + 4*l1*ido + 1) = ci2 - cr5;
			}
		}
	} else {
		for (k=1; k<=l1; k++) {
			for (i=0; i<ido-1; i+=2) {
				VECLOOP {
				ac = i + 1 + (k*5 - 4)*ido;
				ti5 = ref0(cc,ac) - ref0(cc,ac + 3*ido);
				ti2 = ref0(cc,ac) + ref0(cc,ac + 3*ido);
				ti4 = ref0(cc,ac + ido) - ref0(cc,ac + 2*ido);
				ti3 = ref0(cc,ac + ido) + ref0(cc,ac + 2*ido);
				tr5 = ref0(cc,ac - 1) - ref0(cc,ac + 3*ido - 1);
				tr2 = ref0(cc,ac - 1) + ref0(cc,ac + 3*ido - 1);
				tr4 = ref0(cc,ac + ido - 1) - ref0(cc,ac + 2*ido - 1);
				tr3 = ref0(cc,ac + ido - 1) + ref0(cc,ac + 2*ido - 1);
				ah = i + (k - 1)*ido;
				ref0(ch, ah) = ref0(cc,ac - ido - 1) + tr2 + tr3;
				ref0(ch, ah + 1) = ref0(cc,ac - ido) + ti2 + ti3;
				cr2 = ref0(cc,ac - ido - 1) + tr11*tr2 + tr12*tr3;

				ci2 = ref0(cc,ac - ido) + tr11*ti2 + tr12*ti3;
				cr3 = ref0(cc,ac - ido - 1) + tr12*tr2 + tr11*tr3;

				ci3 = ref0(cc,ac - ido) + tr12*ti2 + tr11*ti3;
				cr5 = isign*(ti11*tr5 + ti12*tr4);
				ci5 = isign*(ti11*ti5 + ti12*ti4);
				cr4 = isign*(ti12*tr5 - ti11*tr4);
				ci4 = isign*(ti12*ti5 - ti11*ti4);
				dr3 = cr3 - ci4;
				dr4 = cr3 + ci4;
				di3 = ci3 + cr4;
				di4 = ci3 - cr4;
				dr5 = cr2 + ci5;
				dr2 = cr2 - ci5;
				di5 = ci2 - cr5;
				di2 = ci2 + cr5;
				ref0(ch, ah + l1*ido) = wa1[i]*dr2 - isign*wa1[i+1]*di2;
				ref0(ch, ah + l1*ido + 1) = wa1[i]*di2 + isign*wa1[i+1]*dr2;
				ref0(ch, ah + 2*l1*ido) = wa2[i]*dr3 - isign*wa2[i+1]*di3;
				ref0(ch, ah + 2*l1*ido + 1) = wa2[i]*di3 + isign*wa2[i+1]*dr3;
				ref0(ch, ah + 3*l1*ido) = wa3[i]*dr4 - isign*wa3[i+1]*di4;
				ref0(ch, ah + 3*l1*ido + 1) = wa3[i]*di4 + isign*wa3[i+1]*dr4;
				ref0(ch, ah + 4*l1*ido) = wa4[i]*dr5 - isign*wa4[i+1]*di5;
				ref0(ch, ah + 4*l1*ido + 1) = wa4[i]*di5 + isign*wa4[i+1]*dr5;
				}
			}
		}
	}
} /* passf5 */


static void passf(Tint *nac, Tint ido, Tint ip, Tint l1, Tint idl1,
				  Treal cc[], Treal ch[],
				  const Treal wa[], int isign VECARGS_FORMAL)
/* isign is -1 for forward transform and +1 for backward transform */
{
	VECVARS;
    Tint idij, idlj, idot, ipph, i, j, k, l, jc, lc, ik, nt, idj, idl, inc,idp;
    Treal wai, war;

    idot = ido / 2;
    nt = ip*idl1;
    ipph = (ip + 1) / 2;
    idp = ip*ido;
    if (ido >= l1) {
		for (j=1; j<ipph; j++) {
			jc = ip - j;
			for (k=0; k<l1; k++) {
				for (i=0; i<ido; i++) {
					VECLOOP {
					ref0(ch, i + (k + j*l1)*ido) =
						ref0(cc,i + (j + k*ip)*ido) + ref0(cc,i + (jc + k*ip)*ido);
					ref0(ch, i + (k + jc*l1)*ido) =
						ref0(cc,i + (j + k*ip)*ido) - ref0(cc,i + (jc + k*ip)*ido);
					}
				}
			}
		}
		for (k=0; k<l1; k++)
			for (i=0; i<ido; i++) {
				VECLOOP
					ref0(ch, i + k*ido) = ref0(cc,i + k*ip*ido);
			}
	} else {
		for (j=1; j<ipph; j++) {
			jc = ip - j;
			for (i=0; i<ido; i++) {
				for (k=0; k<l1; k++) {
					VECLOOP {
					ref0(ch, i + (k + j*l1)*ido) = ref0(cc,i + (j + k*ip)*ido) + ref0(cc,i + (jc + k*ip)*ido);
					ref0(ch, i + (k + jc*l1)*ido) = ref0(cc,i + (j + k*ip)*ido) - ref0(cc,i + (jc + k*ip)*ido);
					}
				}
			}
		}
		for (i=0; i<ido; i++)
			for (k=0; k<l1; k++) {
				VECLOOP
					ref0(ch, i + k*ido) = ref0(cc,i + k*ip*ido);
			}
	}
	
	idl = 2 - ido;
    inc = 0;
    for (l=1; l<ipph; l++) {
		lc = ip - l;
		idl += ido;
		for (ik=0; ik<idl1; ik++) {
			VECLOOP {
			ref0(cc, ik + l*idl1) = ref0(ch, ik) + wa[idl - 2]*ref0(ch, ik + idl1);
			ref0(cc, ik + lc*idl1) = isign*wa[idl-1]*ref0(ch, ik + (ip-1)*idl1);
			}
		}
		idlj = idl;
		inc += ido;
		for (j=2; j<ipph; j++) {
			jc = ip - j;
			idlj += inc;
			if (idlj > idp) idlj -= idp;
			war = wa[idlj - 2];
			wai = wa[idlj-1];
			for (ik=0; ik<idl1; ik++) {
			VECLOOP {
				ref0(cc, ik + l*idl1) += war*ref0(ch, ik + j*idl1);
				ref0(cc, ik + lc*idl1) += isign*wai*ref0(ch, ik + jc*idl1);
			}
			}
		}
    }
    for (j=1; j<ipph; j++)
		for (ik=0; ik<idl1; ik++) {
			VECLOOP
				ref0(ch, ik) += ref0(ch, ik + j*idl1);
		}
    for (j=1; j<ipph; j++) {
		jc = ip - j;
		for (ik=1; ik<idl1; ik+=2) {
			VECLOOP {
			ref0(ch, ik - 1 + j*idl1) = ref0(cc, ik - 1 + j*idl1) - ref0(cc, ik + jc*idl1);
			ref0(ch, ik - 1 + jc*idl1) = ref0(cc, ik - 1 + j*idl1) + ref0(cc, ik + jc*idl1);
			ref0(ch, ik + j*idl1) = ref0(cc, ik + j*idl1) + ref0(cc, ik - 1 + jc*idl1);
			ref0(ch, ik + jc*idl1) = ref0(cc, ik + j*idl1) - ref0(cc, ik - 1 + jc*idl1);
			}
		}
    }
    if (ido == 2) {*nac = 1; return;}
    *nac = 0;
    for (ik=0; ik<idl1; ik++) {
		VECLOOP
			ref0(cc, ik) = ref0(ch, ik);
	}
    for (j=1; j<ip; j++) {
		for (k=0; k<l1; k++) {
			VECLOOP {
			ref0(cc, (k + j*l1)*ido + 0) = ref0(ch, (k + j*l1)*ido + 0);
			ref0(cc, (k + j*l1)*ido + 1) = ref0(ch, (k + j*l1)*ido + 1);
			}
		}
    }
    if (idot <= l1) {
		idij = 0;
		for (j=1; j<ip; j++) {
			idij += 2;
			for (i=3; i<ido; i+=2) {
				idij += 2;
				for (k=0; k<l1; k++) {
					VECLOOP {
					ref0(cc, i - 1 + (k + j*l1)*ido) =
						wa[idij - 2]*ref0(ch, i - 1 + (k + j*l1)*ido) -
						isign*wa[idij-1]*ref0(ch, i + (k + j*l1)*ido);
					ref0(cc, i + (k + j*l1)*ido) =
						wa[idij - 2]*ref0(ch, i + (k + j*l1)*ido) +
						isign*wa[idij-1]*ref0(ch, i - 1 + (k + j*l1)*ido);
					}
				}
			}
		}
	} else {
		idj = 2 - ido;
		for (j=1; j<ip; j++) {
			idj += ido;
			for (k = 0; k < l1; k++) {
				idij = idj;
				for (i=3; i<ido; i+=2) {
					idij += 2;
					VECLOOP {
					ref0(cc, i - 1 + (k + j*l1)*ido) =
						wa[idij - 2]*ref0(ch, i - 1 + (k + j*l1)*ido) -
						isign*wa[idij-1]*ref0(ch, i + (k + j*l1)*ido);
					ref0(cc, i + (k + j*l1)*ido) =
						wa[idij - 2]*ref0(ch, i + (k + j*l1)*ido) +
						isign*wa[idij-1]*ref0(ch, i - 1 + (k + j*l1)*ido);
					}
				}
			}
		}
	}
} /* passf */


/* ----------------------------------------------------------------------
   radf2,radb2, radf3,radb3, radf4,radb4, radf5,radb5, radfg,radbg.
   Real FFT passes fwd and bwd.
   ---------------------------------------------------------------------- */

static void radf2(Tint ido, Tint l1, const Treal cc[], Treal ch[], const Treal wa1[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ti2, tr2;
    for (k=0; k<l1; k++) {
		VECLOOP {
		ref(ch, 2*k*ido) =
			ref(cc,k*ido) + ref(cc,(k + l1)*ido);
		ref(ch, (2*k+1)*ido + ido-1) =
			ref(cc,k*ido) - ref(cc,(k + l1)*ido);
		}
    }
    if (ido < 2) return;
	if (ido != 2) {
		for (k=0; k<l1; k++) {
			for (i=2; i<ido; i+=2) {
				ic = ido - i;
				VECLOOP {
				tr2 = wa1[i - 2]*ref(cc, i-1 + (k + l1)*ido) + wa1[i - 1]*ref(cc, i + (k + l1)*ido);
				ti2 = wa1[i - 2]*ref(cc, i + (k + l1)*ido) - wa1[i - 1]*ref(cc, i-1 + (k + l1)*ido);
				ref(ch, i + 2*k*ido) = ref(cc,i + k*ido) + ti2;
				ref(ch, ic + (2*k+1)*ido) = ti2 - ref(cc,i + k*ido);
				ref(ch, i - 1 + 2*k*ido) = ref(cc,i - 1 + k*ido) + tr2;
				ref(ch, ic - 1 + (2*k+1)*ido) = ref(cc,i - 1 + k*ido) - tr2;
				}
			}
		}
		if (ido % 2 == 1) return;
	}
	for (k=0; k<l1; k++) {
		VECLOOP {
		ref(ch, (2*k+1)*ido) = -ref(cc,ido-1 + (k + l1)*ido);
		ref(ch, ido-1 + 2*k*ido) = ref(cc,ido-1 + k*ido);
		}
	}
} /* radf2 */


static void radb2(Tint ido, Tint l1, const Treal cc[], Treal ch[], const Treal wa1[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ti2, tr2;
    for (k=0; k<l1; k++) {
		VECLOOP {
		ref(ch, k*ido) =
			ref(cc,2*k*ido) + ref(cc,ido-1 + (2*k+1)*ido);
		ref(ch, (k + l1)*ido) =
			ref(cc,2*k*ido) - ref(cc,ido-1 + (2*k+1)*ido);
		}
    }
    if (ido < 2) return;
    if (ido != 2) {
		for (k = 0; k < l1; ++k) {
			for (i = 2; i < ido; i += 2) {
				ic = ido - i;
				VECLOOP {
				ref(ch, i-1 + k*ido) =
					ref(cc,i-1 + 2*k*ido) + ref(cc,ic-1 + (2*k+1)*ido);
				tr2 = ref(cc,i-1 + 2*k*ido) - ref(cc,ic-1 + (2*k+1)*ido);
				ref(ch, i + k*ido) =
					ref(cc,i + 2*k*ido) - ref(cc,ic + (2*k+1)*ido);
				ti2 = ref(cc,i + (2*k)*ido) + ref(cc,ic + (2*k+1)*ido);
				ref(ch, i-1 + (k + l1)*ido) =
					wa1[i - 2]*tr2 - wa1[i - 1]*ti2;
				ref(ch, i + (k + l1)*ido) =
					wa1[i - 2]*ti2 + wa1[i - 1]*tr2;
				}
			}
		}
		if (ido % 2 == 1) return;
	}
    for (k = 0; k < l1; k++) {
		VECLOOP {
		ref(ch, ido-1 + k*ido) = 2*ref(cc,ido-1 + 2*k*ido);
		ref(ch, ido-1 + (k + l1)*ido) = -2*ref(cc,(2*k+1)*ido);
		}
    }
} /* radb2 */


static void radf3(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				  const Treal wa1[], const Treal wa2[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ci2, di2, di3, cr2, dr2, dr3, ti2, ti3, tr2, tr3;
    for (k=0; k<l1; k++) {
		VECLOOP {
		cr2 = ref(cc,(k + l1)*ido) + ref(cc,(k + 2*l1)*ido);
		ref(ch, 3*k*ido) = ref(cc,k*ido) + cr2;
		ref(ch, (3*k+2)*ido) = taui*(ref(cc,(k + l1*2)*ido) - ref(cc,(k + l1)*ido));
		ref(ch, ido-1 + (3*k + 1)*ido) = ref(cc,k*ido) + taur*cr2;
		}
    }
    if (ido == 1) return;
    for (k=0; k<l1; k++) {
		for (i=2; i<ido; i+=2) {
			ic = ido - i;
			VECLOOP {
			dr2 = wa1[i - 2]*ref(cc,i - 1 + (k + l1)*ido) + 
				wa1[i - 1]*ref(cc,i + (k + l1)*ido);
			di2 = wa1[i - 2]*ref(cc,i + (k + l1)*ido) - wa1[i - 1]*ref(cc,i - 1 + (k + l1)*ido);
			dr3 = wa2[i - 2]*ref(cc,i - 1 + (k + l1*2)*ido) + wa2[i - 1]*ref(cc,i + (k + l1*2)*ido);
			di3 = wa2[i - 2]*ref(cc,i + (k + l1*2)*ido) - wa2[i - 1]*ref(cc,i - 1 + (k + l1*2)*ido);
			cr2 = dr2 + dr3;
			ci2 = di2 + di3;
			ref(ch, i - 1 + 3*k*ido) = ref(cc,i - 1 + k*ido) + cr2;
			ref(ch, i + 3*k*ido) = ref(cc,i + k*ido) + ci2;
			tr2 = ref(cc,i - 1 + k*ido) + taur*cr2;
			ti2 = ref(cc,i + k*ido) + taur*ci2;
			tr3 = taui*(di2 - di3);
			ti3 = taui*(dr3 - dr2);
			ref(ch, i - 1 + (3*k + 2)*ido) = tr2 + tr3;
			ref(ch, ic - 1 + (3*k + 1)*ido) = tr2 - tr3;
			ref(ch, i + (3*k + 2)*ido) = ti2 + ti3;
			ref(ch, ic + (3*k + 1)*ido) = ti3 - ti2;
			}
		}
    }
} /* radf3 */


static void radb3(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				  const Treal wa1[], const Treal wa2[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ci2, ci3, di2, di3, cr2, cr3, dr2, dr3, ti2, tr2;
    for (k=0; k<l1; k++) {
		VECLOOP {
		tr2 = 2*ref(cc,ido-1 + (3*k + 1)*ido);
		cr2 = ref(cc,3*k*ido) + taur*tr2;
		ref(ch, k*ido) = ref(cc,3*k*ido) + tr2;
		ci3 = 2*taui*ref(cc,(3*k + 2)*ido);
		ref(ch, (k + l1)*ido) = cr2 - ci3;
		ref(ch, (k + 2*l1)*ido) = cr2 + ci3;
		}
    }
    if (ido == 1) return;
    for (k=0; k<l1; k++) {
		for (i=2; i<ido; i+=2) {
			ic = ido - i;
			VECLOOP {
			tr2 = ref(cc,i - 1 + (3*k + 2)*ido) + ref(cc,ic - 1 + (3*k + 1)*ido);
			cr2 = ref(cc,i - 1 + 3*k*ido) + taur*tr2;
			ref(ch, i - 1 + k*ido) = ref(cc,i - 1 + 3*k*ido) + tr2;
			ti2 = ref(cc,i + (3*k + 2)*ido) - ref(cc,ic + (3*k + 1)*ido);
			ci2 = ref(cc,i + 3*k*ido) + taur*ti2;
			ref(ch, i + k*ido) = ref(cc,i + 3*k*ido) + ti2;
			cr3 = taui*(ref(cc,i - 1 + (3*k + 2)*ido) - ref(cc,ic - 1 + (3*k + 1)*ido));
			ci3 = taui*(ref(cc,i + (3*k + 2)*ido) + ref(cc,ic + (3*k + 1)*ido));
			dr2 = cr2 - ci3;
			dr3 = cr2 + ci3;
			di2 = ci2 + cr3;
			di3 = ci2 - cr3;
			ref(ch, i - 1 + (k + l1)*ido) = wa1[i - 2]*dr2 - wa1[i - 1]*di2;
			ref(ch, i + (k + l1)*ido) = wa1[i - 2]*di2 + wa1[i - 1]*dr2;
			ref(ch, i - 1 + (k + 2*l1)*ido) = wa2[i - 2]*dr3 - wa2[i - 1]*di3;
			ref(ch, i + (k + 2*l1)*ido) = wa2[i - 2]*di3 + wa2[i - 1]*dr3;
			}
		}
    }
} /* radb3 */


static void radf4(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				  const Treal wa1[], const Treal wa2[], const Treal wa3[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    for (k=0; k<l1; k++) {
		VECLOOP {
		tr1 = ref(cc,(k + l1)*ido) + ref(cc,(k + 3*l1)*ido);
		tr2 = ref(cc,k*ido) + ref(cc,(k + 2*l1)*ido);
		ref(ch, 4*k*ido) = tr1 + tr2;
		ref(ch, ido-1 + (4*k + 3)*ido) = tr2 - tr1;
		ref(ch, ido-1 + (4*k + 1)*ido) = ref(cc,k*ido) - ref(cc,(k + 2*l1)*ido);
		ref(ch, (4*k + 2)*ido) = ref(cc,(k + 3*l1)*ido) - ref(cc,(k + l1)*ido);
		}
    }
    if (ido < 2) return;
    if (ido != 2) {
		for (k=0; k<l1; k++) {
			for (i=2; i<ido; i += 2) {
				VECLOOP {
				ic = ido - i;
				cr2 = wa1[i - 2]*ref(cc,i - 1 + (k + l1)*ido) + wa1[i - 1]*ref(cc,i + (k + l1)*ido);
				ci2 = wa1[i - 2]*ref(cc,i + (k + l1)*ido) - wa1[i - 1]*ref(cc,i - 1 + (k + l1)*ido);
				cr3 = wa2[i - 2]*ref(cc,i - 1 + (k + 2*l1)*ido) + wa2[i - 1]*ref(cc,i + (k + 2*l1)*ido);
				ci3 = wa2[i - 2]*ref(cc,i + (k + 2*l1)*ido) - wa2[i - 1]*ref(cc,i - 1 + (k + 2*l1)*ido);
				cr4 = wa3[i - 2]*ref(cc,i - 1 + (k + 3*l1)*ido) + wa3[i - 1]*ref(cc,i + (k + 3*l1)*ido);
				ci4 = wa3[i - 2]*ref(cc,i + (k + 3*l1)*ido) - wa3[i - 1]*ref(cc,i - 1 + (k + 3*l1)*ido);
				tr1 = cr2 + cr4;
				tr4 = cr4 - cr2;
				ti1 = ci2 + ci4;
				ti4 = ci2 - ci4;
				ti2 = ref(cc,i + k*ido) + ci3;
				ti3 = ref(cc,i + k*ido) - ci3;
				tr2 = ref(cc,i - 1 + k*ido) + cr3;
				tr3 = ref(cc,i - 1 + k*ido) - cr3;
				ref(ch, i - 1 + 4*k*ido) = tr1 + tr2;
				ref(ch, ic - 1 + (4*k + 3)*ido) = tr2 - tr1;
				ref(ch, i + 4*k*ido) = ti1 + ti2;
				ref(ch, ic + (4*k + 3)*ido) = ti1 - ti2;
				ref(ch, i - 1 + (4*k + 2)*ido) = ti4 + tr3;
				ref(ch, ic - 1 + (4*k + 1)*ido) = tr3 - ti4;
				ref(ch, i + (4*k + 2)*ido) = tr4 + ti3;
				ref(ch, ic + (4*k + 1)*ido) = tr4 - ti3;
				}
			}
		}
		if (ido % 2 == 1) return;
	}
    for (k=0; k<l1; k++) {
		VECLOOP {
		ti1 = -invsqrt2*(ref(cc,ido-1 + (k + l1)*ido) + ref(cc,ido-1 + (k + 3*l1)*ido));
		tr1 = invsqrt2*(ref(cc,ido-1 + (k + l1)*ido) - ref(cc,ido-1 + (k + 3*l1)*ido));
		ref(ch, ido-1 + 4*k*ido) = tr1 + ref(cc,ido-1 + k*ido);
		ref(ch, ido-1 + (4*k + 2)*ido) = ref(cc,ido-1 + k*ido) - tr1;
		ref(ch, (4*k + 1)*ido) = ti1 - ref(cc,ido-1 + (k + 2*l1)*ido);
		ref(ch, (4*k + 3)*ido) = ti1 + ref(cc,ido-1 + (k + 2*l1)*ido);
		}
    }
} /* radf4 */


static void radb4(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				  const Treal wa1[], const Treal wa2[], const Treal wa3[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ci2, ci3, ci4, cr2, cr3, cr4, ti1, ti2, ti3, ti4, tr1, tr2, tr3, tr4;
    for (k = 0; k < l1; k++) {
		VECLOOP {
		tr1 = ref(cc,4*k*ido) - ref(cc,ido-1 + (4*k + 3)*ido);
		tr2 = ref(cc,4*k*ido) + ref(cc,ido-1 + (4*k + 3)*ido);
		tr3 = ref(cc,ido-1 + (4*k + 1)*ido) + ref(cc,ido-1 + (4*k + 1)*ido);
		tr4 = ref(cc,(4*k + 2)*ido) + ref(cc,(4*k + 2)*ido);
		ref(ch, k*ido) = tr2 + tr3;
		ref(ch, (k + l1)*ido) = tr1 - tr4;
		ref(ch, (k + 2*l1)*ido) = tr2 - tr3;
		ref(ch, (k + 3*l1)*ido) = tr1 + tr4;
		}
    }
    if (ido < 2) return;
    if (ido != 2) {
		for (k = 0; k < l1; ++k) {
			for (i = 2; i < ido; i += 2) {
				ic = ido - i;
				VECLOOP {
				ti1 = ref(cc,i + 4*k*ido) + ref(cc,ic + (4*k + 3)*ido);
				ti2 = ref(cc,i + 4*k*ido) - ref(cc,ic + (4*k + 3)*ido);
				ti3 = ref(cc,i + (4*k + 2)*ido) - ref(cc,ic + (4*k + 1)*ido);
				tr4 = ref(cc,i + (4*k + 2)*ido) + ref(cc,ic + (4*k + 1)*ido);
				tr1 = ref(cc,i - 1 + 4*k*ido) - ref(cc,ic - 1 + (4*k + 3)*ido);
				tr2 = ref(cc,i - 1 + 4*k*ido) + ref(cc,ic - 1 + (4*k + 3)*ido);
				ti4 = ref(cc,i - 1 + (4*k + 2)*ido) - ref(cc,ic - 1 + (4*k + 1)*ido);
				tr3 = ref(cc,i - 1 + (4*k + 2)*ido) + ref(cc,ic - 1 + (4*k + 1)*ido);
				ref(ch, i - 1 + k*ido) = tr2 + tr3;
				cr3 = tr2 - tr3;
				ref(ch, i + k*ido) = ti2 + ti3;
				ci3 = ti2 - ti3;
				cr2 = tr1 - tr4;
				cr4 = tr1 + tr4;
				ci2 = ti1 + ti4;
				ci4 = ti1 - ti4;
				ref(ch, i - 1 + (k + l1)*ido) = wa1[i - 2]*cr2 - wa1[i - 1]*ci2;
				ref(ch, i + (k + l1)*ido) = wa1[i - 2]*ci2 + wa1[i - 1]*cr2;
				ref(ch, i - 1 + (k + 2*l1)*ido) = wa2[i - 2]*cr3 - wa2[i - 1]*ci3;
				ref(ch, i + (k + 2*l1)*ido) = wa2[i - 2]*ci3 + wa2[i - 1]*cr3;
				ref(ch, i - 1 + (k + 3*l1)*ido) = wa3[i - 2]*cr4 - wa3[i - 1]*ci4;
				ref(ch, i + (k + 3*l1)*ido) = wa3[i - 2]*ci4 + wa3[i - 1]*cr4;
				}
			}
		}
		if (ido % 2 == 1) return;
	}
    for (k = 0; k < l1; k++) {
		VECLOOP {
		ti1 = ref(cc,(4*k + 1)*ido) + ref(cc,(4*k + 3)*ido);
		ti2 = ref(cc,(4*k + 3)*ido) - ref(cc,(4*k + 1)*ido);
		tr1 = ref(cc,ido-1 + 4*k*ido) - ref(cc,ido-1 + (4*k + 2)*ido);
		tr2 = ref(cc,ido-1 + 4*k*ido) + ref(cc,ido-1 + (4*k + 2)*ido);
		ref(ch, ido-1 + k*ido) = tr2 + tr2;
		ref(ch, ido-1 + (k + l1)*ido) = sqrt2*(tr1 - ti1);
		ref(ch, ido-1 + (k + 2*l1)*ido) = ti2 + ti2;
		ref(ch, ido-1 + (k + 3*l1)*ido) = -sqrt2*(tr1 + ti1);
		}
    }
} /* radb4 */


static void radf5(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				  const Treal wa1[], const Treal wa2[], const Treal wa3[], const Treal wa4[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ci2, di2, ci4, ci5, di3, di4, di5, ci3, cr2, cr3, dr2, dr3, dr4, dr5,
	     cr5, cr4, ti2, ti3, ti5, ti4, tr2, tr3, tr4, tr5;
    for (k = 0; k < l1; k++) {
		VECLOOP {
		cr2 = ref(cc,(k + 4*l1)*ido) + ref(cc,(k + l1)*ido);
		ci5 = ref(cc,(k + 4*l1)*ido) - ref(cc,(k + l1)*ido);
		cr3 = ref(cc,(k + 3*l1)*ido) + ref(cc,(k + 2*l1)*ido);
		ci4 = ref(cc,(k + 3*l1)*ido) - ref(cc,(k + 2*l1)*ido);
		ref(ch, 5*k*ido) = ref(cc,k*ido) + cr2 + cr3;
		ref(ch, ido-1 + (5*k + 1)*ido) = ref(cc,k*ido) + tr11*cr2 + tr12*cr3;
		ref(ch, (5*k + 2)*ido) = ti11*ci5 + ti12*ci4;
		ref(ch, ido-1 + (5*k + 3)*ido) = ref(cc,k*ido) + tr12*cr2 + tr11*cr3;
		ref(ch, (5*k + 4)*ido) = ti12*ci5 - ti11*ci4;
		}
    }
    if (ido == 1) return;
    for (k = 0; k < l1; ++k) {
		for (i = 2; i < ido; i += 2) {
			VECLOOP {
			ic = ido - i;
			dr2 = wa1[i - 2]*ref(cc,i - 1 + (k + l1)*ido) + wa1[i - 1]*ref(cc,i + (k + l1)*ido);
			di2 = wa1[i - 2]*ref(cc,i + (k + l1)*ido) - wa1[i - 1]*ref(cc,i - 1 + (k + l1)*ido);
			dr3 = wa2[i - 2]*ref(cc,i - 1 + (k + 2*l1)*ido) + wa2[i - 1]*ref(cc,i + (k + 2*l1)*ido);
			di3 = wa2[i - 2]*ref(cc,i + (k + 2*l1)*ido) - wa2[i - 1]*ref(cc,i - 1 + (k + 2*l1)*ido);
			dr4 = wa3[i - 2]*ref(cc,i - 1 + (k + 3*l1)*ido) + wa3[i - 1]*ref(cc,i + (k + 3*l1)*ido);
			di4 = wa3[i - 2]*ref(cc,i + (k + 3*l1)*ido) - wa3[i - 1]*ref(cc,i - 1 + (k + 3*l1)*ido);
			dr5 = wa4[i - 2]*ref(cc,i - 1 + (k + 4*l1)*ido) + wa4[i - 1]*ref(cc,i + (k + 4*l1)*ido);
			di5 = wa4[i - 2]*ref(cc,i + (k + 4*l1)*ido) - wa4[i - 1]*ref(cc,i - 1 + (k + 4*l1)*ido);
			cr2 = dr2 + dr5;
			ci5 = dr5 - dr2;
			cr5 = di2 - di5;
			ci2 = di2 + di5;
			cr3 = dr3 + dr4;
			ci4 = dr4 - dr3;
			cr4 = di3 - di4;
			ci3 = di3 + di4;
			ref(ch, i - 1 + 5*k*ido) = ref(cc,i - 1 + k*ido) + cr2 + cr3;
			ref(ch, i + 5*k*ido) = ref(cc,i + k*ido) + ci2 + ci3;
			tr2 = ref(cc,i - 1 + k*ido) + tr11*cr2 + tr12*cr3;
			ti2 = ref(cc,i + k*ido) + tr11*ci2 + tr12*ci3;
			tr3 = ref(cc,i - 1 + k*ido) + tr12*cr2 + tr11*cr3;
			ti3 = ref(cc,i + k*ido) + tr12*ci2 + tr11*ci3;
			tr5 = ti11*cr5 + ti12*cr4;
			ti5 = ti11*ci5 + ti12*ci4;
			tr4 = ti12*cr5 - ti11*cr4;
			ti4 = ti12*ci5 - ti11*ci4;
			ref(ch, i - 1 + (5*k + 2)*ido) = tr2 + tr5;
			ref(ch, ic - 1 + (5*k + 1)*ido) = tr2 - tr5;
			ref(ch, i + (5*k + 2)*ido) = ti2 + ti5;
			ref(ch, ic + (5*k + 1)*ido) = ti5 - ti2;
			ref(ch, i - 1 + (5*k + 4)*ido) = tr3 + tr4;
			ref(ch, ic - 1 + (5*k + 3)*ido) = tr3 - tr4;
			ref(ch, i + (5*k + 4)*ido) = ti3 + ti4;
			ref(ch, ic + (5*k + 3)*ido) = ti4 - ti3;
			}
		}
    }
} /* radf5 */


static void radb5(Tint ido, Tint l1, const Treal cc[], Treal ch[], 
				  const Treal wa1[], const Treal wa2[], const Treal wa3[], const Treal wa4[] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k, ic;
    Treal ci2, ci3, ci4, ci5, di3, di4, di5, di2, cr2, cr3, cr5, cr4, ti2, ti3,
	     ti4, ti5, dr3, dr4, dr5, dr2, tr2, tr3, tr4, tr5;
    for (k = 0; k < l1; k++) {
		VECLOOP {
		ti5 = 2*ref(cc,(5*k + 2)*ido);
		ti4 = 2*ref(cc,(5*k + 4)*ido);
		tr2 = 2*ref(cc,ido-1 + (5*k + 1)*ido);
		tr3 = 2*ref(cc,ido-1 + (5*k + 3)*ido);
		ref(ch, k*ido) = ref(cc,5*k*ido) + tr2 + tr3;
		cr2 = ref(cc,5*k*ido) + tr11*tr2 + tr12*tr3;
		cr3 = ref(cc,5*k*ido) + tr12*tr2 + tr11*tr3;
		ci5 = ti11*ti5 + ti12*ti4;
		ci4 = ti12*ti5 - ti11*ti4;
		ref(ch, (k + l1)*ido) = cr2 - ci5;
		ref(ch, (k + 2*l1)*ido) = cr3 - ci4;
		ref(ch, (k + 3*l1)*ido) = cr3 + ci4;
		ref(ch, (k + 4*l1)*ido) = cr2 + ci5;
		}
    }
    if (ido == 1) return;
    for (k = 0; k < l1; ++k) {
		for (i = 2; i < ido; i += 2) {
			VECLOOP {
			ic = ido - i;
			ti5 = ref(cc,i + (5*k + 2)*ido) + ref(cc,ic + (5*k + 1)*ido);
			ti2 = ref(cc,i + (5*k + 2)*ido) - ref(cc,ic + (5*k + 1)*ido);
			ti4 = ref(cc,i + (5*k + 4)*ido) + ref(cc,ic + (5*k + 3)*ido);
			ti3 = ref(cc,i + (5*k + 4)*ido) - ref(cc,ic + (5*k + 3)*ido);
			tr5 = ref(cc,i - 1 + (5*k + 2)*ido) - ref(cc,ic - 1 + (5*k + 1)*ido);
			tr2 = ref(cc,i - 1 + (5*k + 2)*ido) + ref(cc,ic - 1 + (5*k + 1)*ido);
			tr4 = ref(cc,i - 1 + (5*k + 4)*ido) - ref(cc,ic - 1 + (5*k + 3)*ido);
			tr3 = ref(cc,i - 1 + (5*k + 4)*ido) + ref(cc,ic - 1 + (5*k + 3)*ido);
			ref(ch, i - 1 + k*ido) = ref(cc,i - 1 + 5*k*ido) + tr2 + tr3;
			ref(ch, i + k*ido) = ref(cc,i + 5*k*ido) + ti2 + ti3;
			cr2 = ref(cc,i - 1 + 5*k*ido) + tr11*tr2 + tr12*tr3;

			ci2 = ref(cc,i + 5*k*ido) + tr11*ti2 + tr12*ti3;
			cr3 = ref(cc,i - 1 + 5*k*ido) + tr12*tr2 + tr11*tr3;

			ci3 = ref(cc,i + 5*k*ido) + tr12*ti2 + tr11*ti3;
			cr5 = ti11*tr5 + ti12*tr4;
			ci5 = ti11*ti5 + ti12*ti4;
			cr4 = ti12*tr5 - ti11*tr4;
			ci4 = ti12*ti5 - ti11*ti4;
			dr3 = cr3 - ci4;
			dr4 = cr3 + ci4;
			di3 = ci3 + cr4;
			di4 = ci3 - cr4;
			dr5 = cr2 + ci5;
			dr2 = cr2 - ci5;
			di5 = ci2 - cr5;
			di2 = ci2 + cr5;
			ref(ch, i - 1 + (k + l1)*ido) = wa1[i - 2]*dr2 - wa1[i - 1]*di2;
			ref(ch, i + (k + l1)*ido) = wa1[i - 2]*di2 + wa1[i - 1]*dr2;
			ref(ch, i - 1 + (k + 2*l1)*ido) = wa2[i - 2]*dr3 - wa2[i - 1]*di3;
			ref(ch, i + (k + 2*l1)*ido) = wa2[i - 2]*di3 + wa2[i - 1]*dr3;
			ref(ch, i - 1 + (k + 3*l1)*ido) = wa3[i - 2]*dr4 - wa3[i - 1]*di4;
			ref(ch, i + (k + 3*l1)*ido) = wa3[i - 2]*di4 + wa3[i - 1]*dr4;
			ref(ch, i - 1 + (k + 4*l1)*ido) = wa4[i - 2]*dr5 - wa4[i - 1]*di5;
			ref(ch, i + (k + 4*l1)*ido) = wa4[i - 2]*di5 + wa4[i - 1]*dr5;
			}
		}
    }
} /* radb5 */


static void radfg(Tint ido, Tint ip, Tint l1, Tint idl1,
				  Treal cc[], Treal ch[], const Treal wa[] VECARGS_FORMAL)
{
	VECVARS;
    Tint idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is, nbd;
    Treal dcc, ai1, ai2, ar1, ar2, ds2, dcp, arg, dsp, ar1h, ar2h;
    arg = twopi / ip;
    dcp = cos(arg);
    dsp = sin(arg);
    ipph = (ip + 1) / 2;
    nbd = (ido - 1) / 2;
    if (ido != 1) {
		for (ik=0; ik<idl1; ik++) {VECLOOP ref(ch, ik) = ref(cc, ik);}
		for (j=1; j<ip; j++)
			for (k=0; k<l1; k++) {
				VECLOOP
					ref(ch, (k + j*l1)*ido) = ref(cc, (k + j*l1)*ido);
			}
		if (nbd <= l1) {
			is = -ido;
			for (j=1; j<ip; j++) {
				is += ido;
				idij = is-1;
				for (i=2; i<ido; i+=2) {
					idij += 2;
					for (k=0; k<l1; k++) {
						VECLOOP {
						ref(ch, i - 1 + (k + j*l1)*ido) =
							wa[idij - 1]*ref(cc, i - 1 + (k + j*l1)*ido) + wa[idij]*ref(cc, i + (k + j*l1)*ido);
						ref(ch, i + (k + j*l1)*ido) =
							wa[idij - 1]*ref(cc, i + (k + j*l1)*ido) - wa[idij]*ref(cc, i - 1 + (k + j*l1)*ido);
						}
					}
				}
			}
		} else {
			is = -ido;
			for (j=1; j<ip; j++) {
				is += ido;
				for (k=0; k<l1; k++) {
					idij = is-1;
					for (i=2; i<ido; i+=2) {
						idij += 2;
						VECLOOP {
						ref(ch, i - 1 + (k + j*l1)*ido) =
							wa[idij - 1]*ref(cc, i - 1 + (k + j*l1)*ido) + wa[idij]*ref(cc, i + (k + j*l1)*ido);
						ref(ch, i + (k + j*l1)*ido) =
							wa[idij - 1]*ref(cc, i + (k + j*l1)*ido) - wa[idij]*ref(cc, i - 1 + (k + j*l1)*ido);
						}
					}
				}
			}
		}
		if (nbd >= l1) {
			for (j=1; j<ipph; j++) {
				jc = ip - j;
				for (k=0; k<l1; k++) {
					for (i=2; i<ido; i+=2) {
						VECLOOP {
						ref(cc, i - 1 + (k + j*l1)*ido) =
							ref(ch, i - 1 + (k + j*l1)*ido) + ref(ch, i - 1 + (k + jc*l1)*ido);
						ref(cc, i - 1 + (k + jc*l1)*ido) =
							ref(ch, i + (k + j*l1)*ido) - ref(ch, i + (k + jc*l1)*ido);
						ref(cc, i + (k + j*l1)*ido) =
							ref(ch, i + (k + j*l1)*ido) + ref(ch, i + (k + jc*l1)*ido);
						ref(cc, i + (k + jc*l1)*ido) =
							ref(ch, i - 1 + (k + jc*l1)*ido) - ref(ch, i - 1 + (k + j*l1)*ido);
						}
					}
				}
			}
		} else {
			for (j=1; j<ipph; j++) {
				jc = ip - j;
				for (i=2; i<ido; i+=2) {
					for (k=0; k<l1; k++) {
						VECLOOP {
						ref(cc, i - 1 + (k + j*l1)*ido) =
							ref(ch, i - 1 + (k + j*l1)*ido) + ref(ch, i - 1 + (k + jc*l1)*ido);
						ref(cc, i - 1 + (k + jc*l1)*ido) =
							ref(ch, i + (k + j*l1)*ido) - ref(ch, i + (k + jc*l1)*ido);
						ref(cc, i + (k + j*l1)*ido) =
							ref(ch, i + (k + j*l1)*ido) + ref(ch, i + (k + jc*l1)*ido);
						ref(cc, i + (k + jc*l1)*ido) =
							ref(ch, i - 1 + (k + jc*l1)*ido) - ref(ch, i - 1 + (k + j*l1)*ido);
						}
					}
				}
			}
		}
	} else {	/* now ido == 1 */
		for (ik=0; ik<idl1; ik++) {VECLOOP ref(cc, ik) = ref(ch, ik);}
	}
	for (j=1; j<ipph; j++) {
		jc = ip - j;
		for (k=0; k<l1; k++) {
			VECLOOP {
			ref(cc, (k + j*l1)*ido) = ref(ch, (k + j*l1)*ido) + ref(ch, (k + jc*l1)*ido);
			ref(cc, (k + jc*l1)*ido) = ref(ch, (k + jc*l1)*ido) - ref(ch, (k + j*l1)*ido);
			}
		}
    }

    ar1 = 1;
    ai1 = 0;
    for (l=1; l<ipph; l++) {
		lc = ip - l;
		ar1h = dcp*ar1 - dsp*ai1;
		ai1 = dcp*ai1 + dsp*ar1;
		ar1 = ar1h;
		for (ik=0; ik<idl1; ik++) {
			VECLOOP {
			ref(ch, ik + l*idl1) = ref(cc, ik) + ar1*ref(cc, ik + idl1);
			ref(ch, ik + lc*idl1) = ai1*ref(cc, ik + (ip-1)*idl1);
			}
		}
		dcc = ar1;
		ds2 = ai1;
		ar2 = ar1;
		ai2 = ai1;
		for (j=2; j<ipph; j++) {
			jc = ip - j;
			ar2h = dcc*ar2 - ds2*ai2;
			ai2 = dcc*ai2 + ds2*ar2;
			ar2 = ar2h;
			for (ik=0; ik<idl1; ik++) {
				VECLOOP {
				ref(ch, ik + l*idl1) += ar2*ref(cc, ik + j*idl1);
				ref(ch, ik + lc*idl1) += ai2*ref(cc, ik + jc*idl1);
				}
			}
		}
    }
    for (j=1; j<ipph; j++)
		for (ik=0; ik<idl1; ik++) {
			VECLOOP
				ref(ch, ik) += ref(cc, ik + j*idl1);
		}

	if (ido >= l1) {
		for (k=0; k<l1; k++) {
			for (i=0; i<ido; i++) {
				VECLOOP {
				ref(cc,i + k*ip*ido) = ref(ch, i + k*ido);
				}
			}
		}
	} else {
		for (i=0; i<ido; i++) {
			for (k=0; k<l1; k++) {
				VECLOOP {
				ref(cc,i + k*ip*ido) = ref(ch, i + k*ido);
				}
			}
		}
	}
    for (j=1; j<ipph; j++) {
		jc = ip - j;
		j2 = 2*j;
		for (k=0; k<l1; k++) {
			VECLOOP {
			ref(cc,ido-1 + (j2 - 1 + k*ip)*ido) =
				ref(ch, (k + j*l1)*ido);
			ref(cc,(j2 + k*ip)*ido) =
				ref(ch, (k + jc*l1)*ido);
			}
		}
    }
    if (ido == 1) return;
    if (nbd >= l1) {
		for (j=1; j<ipph; j++) {
			jc = ip - j;
			j2 = 2*j;
			for (k=0; k<l1; k++) {
				for (i=2; i<ido; i+=2) {
					ic = ido - i;
					VECLOOP {
					ref(cc,i - 1 + (j2 + k*ip)*ido) =
						ref(ch, i - 1 + (k + j*l1)*ido) + ref(ch, i - 1 + (k + jc*l1)*ido);
					ref(cc,ic - 1 + (j2 - 1 + k*ip)*ido) =
						ref(ch, i - 1 + (k + j*l1)*ido) - ref(ch, i - 1 + (k + jc*l1)*ido);
					ref(cc,i + (j2 + k*ip)*ido) =
						ref(ch, i + (k + j*l1)*ido) + ref(ch, i + (k + jc*l1)*ido);
					ref(cc,ic + (j2 - 1 + k*ip)*ido) =
						ref(ch, i + (k + jc*l1)*ido) - ref(ch, i + (k + j*l1)*ido);
					}
				}
			}
		}
	} else {
		for (j=1; j<ipph; j++) {
			jc = ip - j;
			j2 = 2*j;
			for (i=2; i<ido; i+=2) {
				ic = ido - i;
				for (k=0; k<l1; k++) {
					VECLOOP {
					ref(cc,i - 1 + (j2 + k*ip)*ido) = ref(ch, i - 1 + (k + j*l1)*ido) + ref(ch, i - 1 + (k + jc*l1)*ido);
					ref(cc,ic - 1 + (j2 - 1 + k*ip)*ido) = ref(ch, i - 1 + (k + j*l1)*ido) - ref(ch, i - 1 + (k + jc*l1)*ido);
					ref(cc,i + (j2 + k*ip)*ido) = ref(ch, i + (k + j*l1)*ido) + ref(ch, i + (k + jc*l1)*ido);
					ref(cc,ic + (j2 - 1 + k*ip)*ido) = ref(ch, i + (k + jc*l1)*ido) - ref(ch, i + (k + j*l1)*ido);
					}
				}
			}
		}
	}
} /* radfg */


static void radbg(Tint ido, Tint ip, Tint l1, Tint idl1,
				  Treal cc[], Treal ch[], const Treal wa[] VECARGS_FORMAL)
{
	VECVARS;
    Tint idij, ipph, i, j, k, l, j2, ic, jc, lc, ik, is;
    Treal dcc, ai1, ai2, ar1, ar2, ds2;
    Tint nbd;
    Treal dcp, arg, dsp, ar1h, ar2h;
    arg = twopi / ip;
    dcp = cos(arg);
    dsp = sin(arg);
    nbd = (ido - 1) / 2;
    ipph = (ip + 1) / 2;
    if (ido >= l1) {
		for (k=0; k<l1; k++) {
			for (i=0; i<ido; i++) {
				VECLOOP {
				ref(ch, i + k*ido) = ref(cc,i + k*ip*ido);
				}
			}
		}
	} else {
		for (i=0; i<ido; i++) {
			for (k=0; k<l1; k++) {
				VECLOOP {
				ref(ch, i + k*ido) = ref(cc,i + k*ip*ido);
				}
			}
		}
	}
    for (j=1; j<ipph; j++) {
		jc = ip - j;
		j2 = 2*j;
		for (k=0; k<l1; k++) {
			VECLOOP {
			ref(ch, (k + j*l1)*ido) = ref(cc,ido-1 + (j2 - 1 + k*ip)*ido) + ref(cc,ido-1 + (j2 - 1 + k*ip)*ido);
			ref(ch, (k + jc*l1)*ido) = ref(cc,(j2 + k*ip)*ido) + ref(cc,(j2 + k*ip)*ido);
			}
		}
    }

    if (ido != 1) {
		if (nbd >= l1) {
			for (j=1; j<ipph; j++) {
				jc = ip - j;
				for (k=0; k<l1; k++) {
					for (i=2; i<ido; i+=2) {
						ic = ido - i;
						VECLOOP {
						ref(ch, i - 1 + (k + j*l1)*ido) =
							ref(cc,i - 1 + (2*j + k*ip)*ido) + ref(cc,ic - 1 + (2*j - 1 + k*ip)*ido);
						ref(ch, i - 1 + (k + jc*l1)*ido) =
							ref(cc,i - 1 + (2*j + k*ip)*ido) - ref(cc,ic - 1 + (2*j - 1 + k*ip)*ido);
						ref(ch, i + (k + j*l1)*ido) =
							ref(cc,i + (2*j + k*ip)*ido) - ref(cc,ic + (2*j - 1 + k*ip)*ido);
						ref(ch, i + (k + jc*l1)*ido) =
							ref(cc,i + (2*j + k*ip)*ido) + ref(cc,ic + (2*j - 1 + k*ip)*ido);
						}
					}
				}
			}
		} else {
			for (j=1; j<ipph; j++) {
				jc = ip - j;
				for (i=2; i<ido; i+=2) {
					ic = ido - i;
					for (k=0; k<l1; k++) {
						VECLOOP {
						ref(ch, i - 1 + (k + j*l1)*ido) =
							ref(cc,i - 1 + (2*j + k*ip)*ido) + ref(cc,ic - 1 + (2*j - 1 + k*ip)*ido);
						ref(ch, i - 1 + (k + jc*l1)*ido) =
							ref(cc,i - 1 + (2*j + k*ip)*ido) - ref(cc,ic - 1 + (2*j - 1 + k*ip)*ido);
						ref(ch, i + (k + j*l1)*ido) =
							ref(cc,i + (2*j + k*ip)*ido) - ref(cc,ic + (2*j - 1 + k*ip)*ido);
						ref(ch, i + (k + jc*l1)*ido) =
							ref(cc,i + (2*j + k*ip)*ido) + ref(cc,ic + (2*j - 1 + k*ip)*ido);
						}
					}
				}
			}
		}
	}

	ar1 = 1;
    ai1 = 0;
    for (l=1; l<ipph; l++) {
		lc = ip - l;
		ar1h = dcp*ar1 - dsp*ai1;
		ai1 = dcp*ai1 + dsp*ar1;
		ar1 = ar1h;
		for (ik=0; ik<idl1; ik++) {
			VECLOOP {
			ref(cc, ik + l*idl1) = ref(ch, ik) + ar1*ref(ch, ik + idl1);
			ref(cc, ik + lc*idl1) = ai1*ref(ch, ik + (ip-1)*idl1);
			}
		}
		dcc = ar1;
		ds2 = ai1;
		ar2 = ar1;
		ai2 = ai1;
		for (j=2; j<ipph; j++) {
			jc = ip - j;
			ar2h = dcc*ar2 - ds2*ai2;
			ai2 = dcc*ai2 + ds2*ar2;
			ar2 = ar2h;
			for (ik=0; ik<idl1; ik++) {
				VECLOOP {
				ref(cc, ik + l*idl1) += ar2*ref(ch, ik + j*idl1);
				ref(cc, ik + lc*idl1) += ai2*ref(ch, ik + jc*idl1);
				}
			}
		}
    }
    for (j=1; j<ipph; j++) {
		for (ik=0; ik<idl1; ik++) {
			VECLOOP {
			ref(ch, ik) += ref(ch, ik + j*idl1);
			}
		}
    }
    for (j=1; j<ipph; j++) {
		jc = ip - j;
		for (k=0; k<l1; k++) {
			VECLOOP {
			ref(ch, (k + j*l1)*ido) = ref(cc, (k + j*l1)*ido) - ref(cc, (k + jc*l1)*ido);
			ref(ch, (k + jc*l1)*ido) = ref(cc, (k + j*l1)*ido) + ref(cc, (k + jc*l1)*ido);
			}
		}
    }
	
    if (ido == 1) return;
	if (nbd >= l1) {
		for (j=1; j<ipph; j++) {
			jc = ip - j;
			for (k=0; k<l1; k++) {
				for (i=2; i<ido; i+=2) {
					VECLOOP {
					ref(ch, i - 1 + (k + j*l1)*ido) = ref(cc, i - 1 + (k + j*l1)*ido) - ref(cc, i + (k + jc*l1)*ido);
					ref(ch, i - 1 + (k + jc*l1)*ido) = ref(cc, i - 1 + (k + j*l1)*ido) + ref(cc, i + (k + jc*l1)*ido);
					ref(ch, i + (k + j*l1)*ido) = ref(cc, i + (k + j*l1)*ido) + ref(cc, i - 1 + (k + jc*l1)*ido);
					ref(ch, i + (k + jc*l1)*ido) = ref(cc, i + (k + j*l1)*ido) - ref(cc, i - 1 + (k + jc*l1)*ido);
					}
				}
			}
		}
	} else {
		for (j=1; j<ipph; j++) {
			jc = ip - j;
			for (i=2; i<ido; i+=2) {
				for (k=0; k<l1; k++) {
					VECLOOP {
					ref(ch, i - 1 + (k + j*l1)*ido) = ref(cc, i - 1 + (k + j*l1)*ido) - ref(cc, i + (k + jc*l1)*ido);
					ref(ch, i - 1 + (k + jc*l1)*ido) = ref(cc, i - 1 + (k + j *l1)*ido) + ref(cc, i + (k + jc*l1)*ido);
					ref(ch, i + (k + j*l1)*ido) = ref(cc, i + (k + j*l1)*ido) + ref(cc, i - 1 + (k + jc*l1)*ido);
					ref(ch, i + (k + jc*l1)*ido) = ref(cc, i + (k + j*l1)*ido) - ref(cc, i - 1 + (k + jc*l1)*ido);
					}
				}
			}
		}
	}
    for (ik=0; ik<idl1; ik++) {VECLOOP ref(cc, ik) = ref(ch, ik);}
    for (j=1; j<ip; j++)
		for (k=0; k<l1; k++) {
			VECLOOP
				ref(cc, (k + j*l1)*ido) = ref(ch, (k + j*l1)*ido);
		}
    if (nbd <= l1) {
		is = -ido;
		for (j=1; j<ip; j++) {
			is += ido;
			idij = is-1;
			for (i=2; i<ido; i+=2) {
				idij += 2;
				for (k=0; k<l1; k++) {
					VECLOOP {
					ref(cc, i - 1 + (k + j*l1)*ido) =
						wa[idij - 1]*ref(ch, i - 1 + (k + j*l1)*ido) - wa[idij]*ref(ch, i + (k + j*l1)*ido);
					ref(cc, i + (k + j*l1)*ido) =
						wa[idij - 1]*ref(ch, i + (k + j*l1)*ido) + wa[idij]*ref(ch, i - 1 + (k + j*l1)*ido);
					}
				}
			}
		}
	} else {
		is = -ido;
		for (j=1; j<ip; j++) {
			is += ido;
			for (k=0; k<l1; k++) {
				idij = is;
				for (i=2; i<ido; i+=2) {
					idij += 2;
					VECLOOP {
					ref(cc, i - 1 + (k + j*l1)*ido) =
						wa[idij-1]*ref(ch, i - 1 + (k + j*l1)*ido) - wa[idij]*ref(ch, i + (k + j*l1)*ido);
					ref(cc, i + (k + j*l1)*ido) =
						wa[idij-1]*ref(ch, i + (k + j*l1)*ido) + wa[idij]*ref(ch, i - 1 + (k + j*l1)*ido);
					}
				}
			}
		}
	}
} /* radbg */


/* ----------------------------------------------------------------------
   cfftf1, cfftf, cfftb, cffti1, cffti. Complex FFTs.
   ---------------------------------------------------------------------- */

static void cfftf1(Tint n, Treal c[], Treal ch[], const Treal wa[], const Tint ifac[MAXFAC+2], int isign VECARGS_FORMAL)
{
	VECVARS;
    Tint idot, i;
    Tint k1, l1=1, l2;
    Tint na=0, nf=ifac[1], ip, iw=0, ix2, ix3, ix4, nac, ido, idl1;
	Treal *cinput, *coutput;
    for (k1=2; k1<=nf+1; k1++) {
		ip = ifac[k1];
		l2 = ip*l1;
		ido = n / l2;
		idot = 2*ido;
		idl1 = idot*l1;
		if (na) {
			cinput = ch;
			coutput = c;
		} else {
			cinput = c;
			coutput = ch;
		}
		switch (ip) {
		 case 4:
			ix2 = iw + idot;
			ix3 = ix2 + idot;
			passf4(idot, l1, cinput, coutput, wa+iw, wa+ix2, wa+ix3, isign VECARGS_ACTUAL);
			na = !na;
			break;
		 case 2:
			passf2(idot, l1, cinput, coutput, wa+iw, isign VECARGS_ACTUAL);
			na = !na;
			break;
		 case 3:
			ix2 = iw + idot;
			passf3(idot, l1, cinput, coutput, wa+iw, wa+ix2, isign VECARGS_ACTUAL);
			na = !na;
			break;
		 case 5:
			ix2 = iw + idot;
			ix3 = ix2 + idot;
			ix4 = ix3 + idot;
			passf5(idot, l1, cinput, coutput, wa+iw, wa+ix2, wa+ix3, wa+ix4, isign VECARGS_ACTUAL);
			na = !na;
			break;
		 default:
			passf(&nac, idot, ip, l1, idl1, cinput, coutput, wa+iw, isign VECARGS_ACTUAL);
			if (nac) na = !na;
		}
		l1 = l2;
		iw += (ip - 1)*idot;
    }
	if (na)
		for (i=0; i<2*n; i++) {VECLOOP ref0(c,i) = ref0(ch,i);}
} /* cfftf1 */


void veccfftf(Tint n, Treal c[], Treal work[] VECARGS_FORMAL)
/* Complex forward transform.
   Work array length at least 2*((lot-1)*jump + n*step) + 2*n + MAXFAC + 2. */
{
    Tint iw1, iw2;
    if (n <= 1) return;
    iw1 = 2*((lot-1)*jump + n*step);
    iw2 = iw1 + 2*n;
    cfftf1(n, c, work, work+iw1, (Tint*)(work+iw2), -1 VECARGS_ACTUAL);
} /* cfftf */


void veccfftb(Tint n, Treal c[], Treal work[] VECARGS_FORMAL)
/* Complex backward transform.
   Work array length at least 2*((lot-1)*jump + n*step) + 2*n + MAXFAC + 2. */
{
    Tint iw1, iw2;
    if (n <= 1) return;
    iw1 = 2*((lot-1)*jump + n*step);
    iw2 = iw1 + 2*n;
    cfftf1(n, c, work, work+iw1, (Tint*)(work+iw2), +1 VECARGS_ACTUAL);
} /* cfftb */


static void factorize(Tint n, Tint ifac[MAXFAC+2])
/* Factorize n in factors of 2,3,4,5 and rest. On exit,
   ifac[0] contains n and ifac[1] contains number of factors,
   the factors start from ifac[2]. */
{
    static const Tint ntryh[4] = {3,4,2,5};
    Tint ntry=3, i, j=0, ib, nf=0, nl=n, nq, nr;
	if (sizeof(Tint) > sizeof(Treal)) {
		fprintf(stderr,"*** vecfftpack: sizeof(Tint) > sizeof(Treal)\n");
		exit(1);
	}
 startloop:
	if (j < 4)
		ntry = ntryh[j];
	else
		ntry+= 2;
	j++;
	do {
		nq = nl / ntry;
		nr = nl - ntry*nq;
		if (nr != 0) goto startloop;
		nf++;
		ifac[nf + 1] = ntry;
		nl = nq;
		if (ntry == 2 && nf != 1) {
			for (i=2; i<=nf; i++) {
				ib = nf - i + 2;
				ifac[ib + 1] = ifac[ib];
			}
			ifac[2] = 2;
		}
	} while (nl != 1);
    ifac[0] = n;
    ifac[1] = nf;
}


static void cffti1(Tint n, Treal wa[], Tint ifac[MAXFAC+2])
{
    Treal arg, argh, argld, fi;
    Tint ido, i, j, idot, ipm;
    Tint i1, k1, l1, l2;
    Tint ld, ii, nf, ip;
	factorize(n,ifac);
	nf = ifac[1];
    argh = twopi/(Treal)n;
    i = 1;
    l1 = 1;
    for (k1=1; k1<=nf; k1++) {
		ip = ifac[k1+1];
		ld = 0;
		l2 = l1*ip;
		ido = n / l2;
		idot = ido + ido + 2;
		ipm = ip - 1;
		for (j=1; j<=ipm; j++) {
			i1 = i;
			wa[i-1] = 1;
			wa[i] = 0;
			ld += l1;
			fi = 0;
			argld = ld*argh;
			for (ii=4; ii<=idot; ii+=2) {
				i+= 2;
				fi+= 1;
				arg = fi*argld;
				wa[i-1] = cos(arg);
				wa[i] = sin(arg);
			}
			if (ip > 5) {
				wa[i1-1] = wa[i-1];
				wa[i1] = wa[i];
			}
		}
		l1 = l2;
    }
} /* cffti1 */


void veccffti(Tint n, Treal work[] VECARGS_FORMAL)
/* Initialize complex forward or backward transform.
   Work array length at least 2*((lot-1)*jump + n*step) + 2*n + MAXFAC + 2. */
{
    Tint iw1, iw2;
    if (n <= 1) return;
	iw1 = 2*((lot-1)*jump + n*step);
    iw2 = iw1 + 2*n;
    cffti1(n, work+iw1, (Tint*)(work+iw2));
} /* cffti */

/* ----------------------------------------------------------------------
   rfftf1, rfftb1, rfftf, rfftb, rffti1, rffti. Real FFTs.
   ---------------------------------------------------------------------- */

static void rfftf1(Tint n, Treal c[], Treal ch[], const Treal wa[], const Tint ifac[MAXFAC+2] VECARGS_FORMAL)
{
	VECVARS;
    Tint i;
    Tint k1, l1, l2=n, na=1, kh, nf=ifac[1], ip, iw=n-1, ix2, ix3, ix4, ido, idl1;
	Treal *cinput, *coutput;
    for (k1=1; k1<=nf; k1++) {
		kh = nf - k1;
		ip = ifac[kh + 2];
		l1 = l2 / ip;
		ido = n / l2;
		idl1 = ido*l1;
		iw -= (ip - 1)*ido;
		na = !na;
		if (na) {
			cinput = ch;
			coutput = c;
		} else {
			cinput = c;
			coutput = ch;
		}
		switch (ip) {
		 case 4:
			ix2 = iw + ido;
			ix3 = ix2 + ido;
			radf4(ido, l1, cinput, coutput, wa+iw, wa+ix2, wa+ix3 VECARGS_ACTUAL);
			break;
		 case 2:
			radf2(ido, l1, cinput, coutput, wa+iw VECARGS_ACTUAL);
			break;
		 case 3:
			ix2 = iw + ido;
			radf3(ido, l1, cinput, coutput, wa+iw, wa+ix2 VECARGS_ACTUAL);
			break;
		 case 5:
			ix2 = iw + ido;
			ix3 = ix2 + ido;
			ix4 = ix3 + ido;
			radf5(ido, l1, cinput, coutput, wa+iw, wa+ix2, wa+ix3, wa+ix4 VECARGS_ACTUAL);
			break;
		 default:
			if (ido == 1) na = !na;
			if (na)
				radfg(ido, ip, l1, idl1, ch, c, wa+iw VECARGS_ACTUAL);
			else
				radfg(ido, ip, l1, idl1, c, ch, wa+iw VECARGS_ACTUAL);
			na = !na;
		}
		l2 = l1;
    }
    if (na) return;
    for (i=0; i<n; i++) {VECLOOP ref(c,i) = ref(ch,i);}
} /* rfftf1 */


static void rfftb1(Tint n, Treal c[], Treal ch[], const Treal wa[], const Tint ifac[MAXFAC+2] VECARGS_FORMAL)
{
	VECVARS;
    Tint i, k1, l1=1, l2, na=0, nf=ifac[1], ip, iw=0, ix2, ix3, ix4, ido, idl1;
	Treal *cinput, *coutput;
    for (k1=1; k1<=nf; k1++) {
		ip = ifac[k1 + 1];
		l2 = ip*l1;
		ido = n / l2;
		idl1 = ido*l1;
		if (na) {
			cinput = ch;
			coutput = c;
		} else {
			cinput = c;
			coutput = ch;
		}
		switch (ip) {
		 case 4:
			ix2 = iw + ido;
			ix3 = ix2 + ido;
			radb4(ido, l1, cinput, coutput, wa+iw, wa+ix2, wa+ix3 VECARGS_ACTUAL);
			na = !na;
			break;
		 case 2:
			radb2(ido, l1, cinput, coutput, wa+iw VECARGS_ACTUAL);
			na = !na;
			break;
		 case 3:
			ix2 = iw + ido;
			radb3(ido, l1, cinput, coutput, wa+iw, wa+ix2 VECARGS_ACTUAL);
			na = !na;
			break;
		 case 5:
			ix2 = iw + ido;
			ix3 = ix2 + ido;
			ix4 = ix3 + ido;
			radb5(ido, l1, cinput, coutput, wa+iw, wa+ix2, wa+ix3, wa+ix4 VECARGS_ACTUAL);
			na = !na;
			break;
		 default:
			radbg(ido, ip, l1, idl1, cinput, coutput, wa+iw VECARGS_ACTUAL);
			if (ido == 1) na = !na;
		}
		l1 = l2;
		iw += (ip - 1)*ido;
    }
    if (na)
		for (i=0; i<n; i++) {VECLOOP ref(c,i) = ref(ch,i);}
} /* rfftb1 */


void vecrfftf(Tint n, Treal r[], Treal work[] VECARGS_FORMAL)
/* Real forward transform.
   Work array length at least (lot-1)*jump + n*step + n + MAXFAC + 2. */
{
    if (n <= 1) return;
    rfftf1(n, r, work, work+(lot-1)*jump+n*step, (Tint*)(work+(lot-1)*jump+n*step+n) VECARGS_ACTUAL);
} /* rfftf */


void vecrfftb(Tint n, Treal r[], Treal work[] VECARGS_FORMAL)
/* Real backward transform.
   Work array length at least (lot-1)*jump + n*step + n + MAXFAC + 2. */
{
    if (n <= 1) return;
    rfftb1(n, r, work, work+(lot-1)*jump+n*step, (Tint*)(work+(lot-1)*jump+n*step+n) VECARGS_ACTUAL);
} /* rfftb */


static void rffti1(Tint n, Treal wa[], Tint ifac[MAXFAC+2])
{
    Treal arg, argh, argld, fi;
    Tint i, j;
    Tint k1, l1, l2;
    Tint ld, ii, nf, ip, is;
    Tint ido, ipm;
    Tint nfm1;
	factorize(n,ifac);
	nf = ifac[1];
    argh = twopi / (Treal) (n);
    is = 0;
    nfm1 = nf - 1;
    l1 = 1;
    if (nfm1 == 0) return;
    for (k1 = 1; k1 <= nfm1; k1++) {
		ip = ifac[k1 + 1];
		ld = 0;
		l2 = l1*ip;
		ido = n / l2;
		ipm = ip - 1;
		for (j = 1; j <= ipm; ++j) {
			ld += l1;
			i = is;
			argld = (Treal) ld*argh;
			fi = 0;
			for (ii = 3; ii <= ido; ii += 2) {
				i += 2;
				fi += 1;
				arg = fi*argld;
				wa[i - 2] = cos(arg);
				wa[i - 1] = sin(arg);
			}
			is += ido;
		}
		l1 = l2;
    }
} /* rffti1 */


void vecrffti(Tint n, Treal work[] VECARGS_FORMAL)
/* Initialize real forward or backward transform.
   Work array length at least (lot-1)*jump + n*step + n + MAXFAC + 2. */
{
    if (n <= 1) return;
    rffti1(n, work+(lot-1)*jump+n*step, (Tint*)(work+(lot-1)*jump+n*step+n));
} /* rffti */

/* ----------------------------------------------------------------------
   cosqf1, cosqb1, cosqf, cosqb, cosqi. Quarter-wave cos-FFTs.
   ---------------------------------------------------------------------- */

static void cosqf1(Tint n, Treal x[], Treal w[], Treal xh[] VECARGS_FORMAL)
{
	VECVARS;
    Tint modn, i, k;
    Tint kc, np2, ns2;
    Treal xim1;
    ns2 = (n + 1) / 2;
    np2 = n + 2;
    for (k=1; k<ns2; k++) {
		kc = n - k;
		VECLOOP {
		ref(xh,k) = ref(x,k) + ref(x,kc);
		ref(xh,kc) = ref(x,k) - ref(x,kc);
		}
    }
    modn = n % 2;
    if (modn == 0) {
		VECLOOP ref(xh,ns2) = ref(x,ns2) + ref(x,ns2);
	}
    for (k=1; k<ns2; k++) {
		kc = n - k;
		VECLOOP {
		ref(x,k) = w[k - 1]*ref(xh,kc) + w[kc - 1]*ref(xh,k);
		ref(x,kc) = w[k - 1]*ref(xh,k) - w[kc - 1]*ref(xh,kc);
		}
    }
    if (modn == 0) {
		VECLOOP ref(x,ns2) = w[ns2-1]*ref(xh,ns2);
	}
    vecrfftf(n, x, xh VECARGS_ACTUAL);
    for (i=2; i<n; i+=2) {
		VECLOOP {
		xim1 = ref(x,i - 1) - ref(x,i);
		ref(x,i) = ref(x,i - 1) + ref(x,i);
		ref(x,i - 1) = xim1;
		}
    }
} /* cosqf1 */


static void cosqb1(Tint n, Treal x[], Treal w[], Treal xh[] VECARGS_FORMAL)
{
	VECVARS;
    Tint modn, i, k;
    Tint kc, ns2;
    Treal xim1;
    ns2 = (n + 1) / 2;
    for (i=2; i<n; i+=2) {
		VECLOOP {
		xim1 = ref(x,i - 1) + ref(x,i);
		ref(x,i) -= ref(x,i - 1);
		ref(x,i - 1) = xim1;
		}
    }
    VECLOOP ref(x,0) += ref(x,0);
    modn = n % 2;
    if (modn == 0) {
		VECLOOP ref(x,n-1) += ref(x,n-1);
	}
    vecrfftb(n, x, xh VECARGS_ACTUAL);
    for (k=1; k<ns2; k++) {
		kc = n - k;
		VECLOOP {
		ref(xh,k) = w[k - 1]*ref(x,kc) + w[kc - 1]*ref(x,k);
		ref(xh,kc) = w[k - 1]*ref(x,k) - w[kc - 1]*ref(x,kc);
		}
    }
    if (modn == 0) {
		VECLOOP ref(x,ns2) = w[ns2-1]*(ref(x,ns2) + ref(x,ns2));
	}
    for (k=1; k<ns2; k++) {
		kc = n - k;
		VECLOOP {
		ref(x,k) = ref(xh,k) + ref(xh,kc);
		ref(x,kc) = ref(xh,k) - ref(xh,kc);
		}
    }
	VECLOOP ref(x,0) += ref(x,0);
} /* cosqb1 */


void veccosqf(Tint n, Treal x[], Treal work[] VECARGS_FORMAL)
/* Quarter-wave forward cosine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
	VECVARS;
    Treal tsqx;
    if (n < 2) {
		return;
    } else if (n == 2) {
		VECLOOP {
		tsqx = sqrt2*ref(x,1);
		ref(x,1) = ref(x,0) - tsqx;
		ref(x,0) += tsqx;
	    }
	}
    else
		cosqf1(n, x, work, work+(lot-1)*jump+n*step VECARGS_ACTUAL);
} /* cosqf */


void veccosqb(Tint n, Treal x[], Treal work[] VECARGS_FORMAL)
/* Quarter-wave backward cosine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
	VECVARS;
    Treal x1;
    if (n < 2) {
		VECLOOP ref(x,0) *= 4;
    } else if (n == 2) {
		VECLOOP {
		x1 = 4*(ref(x,0) + ref(x,1));
		ref(x,1) = tsqrt2*(ref(x,0) - ref(x,1));
		ref(x,0) = x1;
		}
    } else
		cosqb1(n, x, work, work+(lot-1)*jump+n*step VECARGS_ACTUAL);
} /* cosqb */


void veccosqi(Tint n, Treal work[] VECARGS_FORMAL)
/* Initialize quarter-wave forward or backward cosine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
    Tint k;
    Treal dt;
    dt = 0.5*pi/n;
    for (k=0; k<n; k++)
		work[k] = cos((k+1)*dt);
    vecrffti(n, work+(lot-1)*jump+n*step VECARGS_ACTUAL);
} /* cosqi */

/* ----------------------------------------------------------------------
   cost, costi. Full-wave cos-FFTs.
   ---------------------------------------------------------------------- */

void veccost(Tint n, Treal x[], Treal wsave[] VECARGS_FORMAL)
/* Forward/backward cosine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
	VECVARS;
/*	const int max_lot = 1024;*/
#define max_lot 2048
    Tint modn, i, k;
    Treal t1, t2;
    Tint kc;
    Treal xi;
    Tint nm1;
    Treal x1h;
    Tint ns2;
    Treal tx2, x1p3;
	Treal c1[max_lot], xim2[max_lot];
	if (lot > max_lot) {
		fprintf(stderr,"*** veccost: lot=%d > %d. Increase max_lot in vecfftpack.c:veccost().\n",lot,max_lot);
		return;
	}
    nm1 = n - 1;
    ns2 = n / 2;
    if (n - 2 < 0)
		return;
    else if (n == 2) {
		VECLOOP {
		x1h = ref(x,0) + ref(x,1);
		ref(x,1) = ref(x,0) - ref(x,1);
		ref(x,0) = x1h;
		}
    } else if (n == 3) {
		VECLOOP {
		x1p3 = ref(x,0) + ref(x,2);
		tx2 = ref(x,1) + ref(x,1);
		ref(x,1) = ref(x,0) - ref(x,2);
		ref(x,0) = x1p3 + tx2;
		ref(x,2) = x1p3 - tx2;
		}
	} else {
		VECLOOP {
		c1[p] = ref(x,0) - ref(x,n-1);
		ref(x,0) += ref(x,n-1);
		}
		for (k=1; k<ns2; k++) {
			kc = nm1 - k;
			VECLOOP {
			t1 = ref(x,k) + ref(x,kc);
			t2 = ref(x,k) - ref(x,kc);
			c1[p] += wsave[kc]*t2;
			t2 = wsave[k]*t2;
			ref(x,k) = t1 - t2;
			ref(x,kc) = t1 + t2;
			}
		}
		modn = n % 2;
		if (modn != 0) {
			VECLOOP ref(x,ns2) *= 2;
		}
		vecrfftf(nm1, x, wsave+(lot-1)*jump+n*step VECARGS_ACTUAL);
		VECLOOP {
		xim2[p] = ref(x,1);
		ref(x,1) = c1[p];
	    }
		for (i=3; i<n; i+=2) {
			VECLOOP {
			xi = ref(x,i);
			ref(x,i) = ref(x,i - 2) - ref(x,i - 1);
			ref(x,i - 1) = xim2[p];
			xim2[p] = xi;
			}
		}
		if (modn != 0) {VECLOOP ref(x,n-1) = xim2[p];}
	}
} /* cost */


void veccosti(Tint n, Treal wsave[] VECARGS_FORMAL)
/* Initialize forward/backward cosine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
    Tint k,kc,ns2;
    Treal dt;
    if (n <= 3) return;
    ns2 = n / 2;
    dt = pi / (n-1);
    for (k = 1; k < ns2; k++) {
		kc = n - k - 1;
		wsave[k] = 2*sin(k*dt);
		wsave[kc] = 2*cos(k*dt);
    }
    vecrffti(n-1, wsave+(lot-1)*jump+n*step VECARGS_ACTUAL);
} /* costi */


/* ----------------------------------------------------------------------
   sinqf, sinqb, sinqi. Quarter-wave sin-FFTs. These call cosqf etc.
   ---------------------------------------------------------------------- */

void vecsinqf(Tint n, Treal x[], Treal wsave[] VECARGS_FORMAL)
/* Quarter-wave forward sine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
	VECVARS;
    Tint k;
    Treal xhold;
    Tint kc, ns2;
    if (n <= 1) return;
    ns2 = n / 2;
    for (k=0; k<ns2; k++) {
		kc = n - k - 1;
		VECLOOP {
		xhold = ref(x,k);
		ref(x,k) = ref(x,kc);
		ref(x,kc) = xhold;
		}
    }
    veccosqf(n, x, wsave VECARGS_ACTUAL);
    for (k=1; k<n; k+=2) {
		VECLOOP
			ref(x,k) = -ref(x,k);
	}
} /* sinqf */


void vecsinqb(Tint n, Treal x[], Treal wsave[] VECARGS_FORMAL)
/* Quarter-wave backward sine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
	VECVARS;
    Tint k;
    Treal xhold;
    Tint kc, ns2;
    if (n <= 1) {
		VECLOOP ref(x,0) *= 4;
		return;
	}
    ns2 = n / 2;
    for (k=1; k<n; k+=2) {VECLOOP ref(x,k) = -ref(x,k);}
    veccosqb(n, x, wsave VECARGS_ACTUAL);
    for (k=0; k<ns2; k++) {
		kc = n - k - 1;
		VECLOOP {
		xhold = ref(x,k);
		ref(x,k) = ref(x,kc);
		ref(x,kc) = xhold;
		}
    }
} /* sinqb */


void vecsinqi(Tint n, Treal wsave[] VECARGS_FORMAL)
/* Initialize quarter-wave forward or backward sine transform.
   Work array length at least 2*((lot-1)*jump + n*step) + n + MAXFAC + 2. */
{
    veccosqi(n, wsave VECARGS_ACTUAL);
}

/* ----------------------------------------------------------------------
   sint1, sint, sinti. Full-wave sin-FFTs.
   ---------------------------------------------------------------------- */

#define refnp1(u,a) u[p*jump1+(a)*step]
		
static void sint1(Tint n, Treal war[], Treal war2[], const Treal was[], Treal xh[], const Treal x[],
				  const Tint ifac[MAXFAC+2] VECARGS_FORMAL)
/* Dimensions:
   was[n/2], x[n+1], xh and war2 are full grids with n+1, war is full grid with n */
{
	VECVARS;
    Tint modn, i, k;
    Treal xhold, t1, t2;
    Tint kc, ns2;
	const Tint jump1 = ((step==1) ? (jump+1) : (jump));
    if (n == 1) {
		VECLOOP ref(war,0) *= 2;
    } else if (n == 2) {
		VECLOOP {
		xhold = sqrt3*(ref(war,0) + ref(war,1));
		ref(war,1) = sqrt3*(ref(war,0) - ref(war,1));
		ref(war,0) = xhold;
		}
    } else {
		ns2 = n / 2;
		VECLOOP refnp1(xh,0) = 0;
		for (k=0; k<ns2; k++) {
			kc = n - k - 1;
			VECLOOP {
			t1 = ref(war,k) - ref(war,kc);
			t2 = was[k]*(ref(war,k) + ref(war,kc));
			refnp1(xh,k + 1) = t1 + t2;
			refnp1(xh,kc + 1) = t2 - t1;
			}
		}
		modn = n % 2;
		if (modn != 0) {
			VECLOOP
				refnp1(xh,ns2 + 1) = 4*ref(war,ns2);
		}
		for (i=0; i<n; i++) {VECLOOP refnp1(war2,i) = ref(war,i);}
		rfftf1(n+1, xh, war2, x, ifac, lot,step,step==1?jump+1:jump);
		VECLOOP ref(war,0) = 0.5*refnp1(xh,0);
		for (i=2; i<n; i+=2) {
			VECLOOP {
			ref(war,i-1) = -refnp1(xh,i);
			ref(war,i) = ref(war,i-2) + refnp1(xh,i-1);
			}
		}
		if (modn == 0) {VECLOOP ref(war,n-1) = -refnp1(xh,n);}
	}
} /* sint1 */


void vecsint(Tint n, Treal x[], Treal work[] VECARGS_FORMAL)
/* Forward/backward sine transform.
   Work array length at least 2*((lot-1)*(jump+1) + (n+1)*step) + n + n/2 + 2 + MAXFAC + 2. */
{
    Tint iw1, iw2, iw3, iw4;
    iw1 = n/2;
    iw2 = iw1 + (lot-1)*(step==1?jump+1:jump) + (n + 1)*step;
	iw3 = iw2 + (lot-1)*(step==1?jump+1:jump) + (n + 1)*step;
    iw4 = iw3 + n + 1;
    sint1(n, x, work+iw1, work, work+iw2, work+iw3, (Tint*)(work+iw4) VECARGS_ACTUAL);
} /* sint */


void vecsinti(Tint n, Treal work[] VECARGS_FORMAL)
/* Initialize forward/backward sine transform.
   Work array length at least 2*((lot-1)*(jump+1) + (n+1)*step) + n + n/2 + 2 + MAXFAC + 2. */
{
    Tint k;
    Treal dt;
    if (n <= 1) return;
    dt = pi / (n+1);
    for (k=0; k<n/2; k++)
		work[k] = 2*sin((k+1)*dt);
    vecrffti(n+1, work+n/2 + (lot-1)*(step==1?jump+1:jump) + (n+1)*step, lot, step, step==1?jump+1:jump);
} /* sinti */

#ifdef __cplusplus
}
#endif
