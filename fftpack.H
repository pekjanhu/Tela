/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

extern "C" {

extern void cfftf(Tint N, Tcomplex data[], const Treal wrk[]);
extern void cfftb(Tint N, Tcomplex data[], const Treal wrk[]);
extern void cffti(Tint N, Treal wrk[]);

extern void rfftf(Tint N, Treal data[], const Treal wrk[]);
extern void rfftb(Tint N, Treal data[], const Treal wrk[]);
extern void rffti(Tint N, Treal wrk[]);

extern void sint(Tint N, Treal data[], const Treal wrk[]);
extern void sinti(Tint N, Treal wrk[]);

extern void cost(Tint N, Treal data[], const Treal wrk[]);
extern void costi(Tint N, Treal wrk[]);

extern void sinqf(Tint N, Treal data[], const Treal wrk[]);
extern void sinqb(Tint N, Treal data[], const Treal wrk[]);
extern void sinqi(Tint N, Treal wrk[]);

extern void cosqf(Tint N, Treal data[], const Treal wrk[]);
extern void cosqb(Tint N, Treal data[], const Treal wrk[]);
extern void cosqi(Tint N, Treal wrk[]);

#define CFFTF cfftf
#define CFFTB cfftb
#define CFFTI cffti
#define RFFTF rfftf
#define RFFTB rfftb
#define RFFTI rffti
#define SINTI sinti
#define SINT sint
#define COSTI costi
#define COST cost
#define SINQF sinqf
#define SINQB sinqb
#define SINQI sinqi
#define COSQF cosqf
#define COSQB cosqb
#define COSQI cosqi

}

#if 0

#if defined(HPUX) && !defined(__GNUC__)
#  define NO_UNDERSCORE_IN_FORTRAN_NAMES
#endif

#ifdef NO_UNDERSCORE_IN_FORTRAN_NAMES
#  define Fortran(x) x
#else
#  define Fortran(x) x##_
#endif

#ifndef UNICOS

#  define CFFTF Fortran(cfftf)		// complex transform
#  define CFFTB Fortran(cfftb)		// its inverse
#  define CFFTI Fortran(cffti)		// initializer of the above routine pair

#  define RFFTF Fortran(rfftf)		// real transform
#  define RFFTB Fortran(rfftb)		// its inverse
#  define RFFTI Fortran(rffti)		// initializer for rfft

#  define SINT Fortran(sint)		// sine transform
#  define SINTI Fortran(sinti)		// initializer for sint

#  define COST Fortran(cost)		// cosine transform
#  define COSTI Fortran(costi)		// initializer for cost

#  define SINQF Fortran(sinqf)		// quarter-wave sine transform
#  define SINQB Fortran(sinqb)		// its inverse
#  define SINQI Fortran(sinqi)		// initializer for sinq

#  define COSQF Fortran(cosqf)		// quarter-wave cosine transform
#  define COSQB Fortran(cosqb)		// its inverse
#  define COSQI Fortran(cosqi)		// initializer for cosq

#endif

extern "C" {

// FFTPACK functions
	
extern void CFFTF(const Tint& N, Tcomplex data[], const Treal wrk[]);
extern void CFFTB(const Tint& N, Tcomplex data[], const Treal wrk[]);
extern void CFFTI(const Tint& N, Treal wrk[]);

extern void RFFTF(const Tint& N, Treal data[], const Treal wrk[]);
extern void RFFTB(const Tint& N, Treal data[], const Treal wrk[]);
extern void RFFTI(const Tint& N, Treal wrk[]);

extern void SINT(const Tint& N, Treal data[], const Treal wrk[]);
extern void SINTI(const Tint& N, Treal wrk[]);

extern void COST(const Tint& N, Treal data[], const Treal wrk[]);
extern void COSTI(const Tint& N, Treal wrk[]);

extern void SINQF(const Tint& N, Treal data[], const Treal wrk[]);
extern void SINQB(const Tint& N, Treal data[], const Treal wrk[]);
extern void SINQI(const Tint& N, Treal wrk[]);

extern void COSQF(const Tint& N, Treal data[], const Treal wrk[]);
extern void COSQB(const Tint& N, Treal data[], const Treal wrk[]);
extern void COSQI(const Tint& N, Treal wrk[]);

}

#endif
