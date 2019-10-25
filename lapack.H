/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#if defined(HPUX) && !defined(__GNUC__)
#  define NO_UNDERSCORE_IN_FORTRAN_NAMES
#endif

#ifdef NO_UNDERSCORE_IN_FORTRAN_NAMES
#  define Fortran(x) x
#else
#  define Fortran(x) x##_
#endif

#ifdef UNICOS
#  define RGETRF SGETRF
#  define RPOTRF SPOTRF
#  define RGETRI SGETRI
#  define RGETRS SGETRS
#  define RGESV  SGESV
#  define RGELS  SGELS
#  define RGEEV  SGEEV
#  define RGESVD SGESVD
#  define RGEMM  SGEMM
#  define RDOT   SDOT
#  define RAXPY  SAXPY
#else
#  define RGETRF Fortran(dgetrf)
#  define RPOTRF Fortran(dpotrf)
#  define RGETRI Fortran(dgetri)
#  define RGETRS Fortran(dgetrs)
#  define RGESV  Fortran(dgesv)
#  define RGELS  Fortran(dgels)
#  define RGEEV  Fortran(dgeev)
#  define RGESVD Fortran(dgesvd)
#  define CGETRF Fortran(zgetrf)
#  define CPOTRF Fortran(zpotrf)
#  define CGETRI Fortran(zgetri)
#  define CGETRS Fortran(zgetrs)
#  define CGESV  Fortran(zgesv)
#  define CGELS  Fortran(zgels)
#  define CGEEV  Fortran(zgeev)
#  define CGESVD Fortran(zgesvd)
#  define RGEMM  Fortran(dgemm)
#  define CGEMM  Fortran(zgemm)
#  define RDOT   Fortran(ddot)
#  define RAXPY  Fortran(daxpy)
#  define CAXPY  Fortran(zaxpy)
#endif

extern "C" {

/* LAPACK routines: */
/* ---------------- */
	
void RGETRF (const Tint& M, const Tint& N,
			 Treal A[], const Tint& LDA,
			 Tint IPIV[],
			 Tint& INFO);

void RPOTRF(const char*UPLO,
			const Tint& N,
			Treal A[], const Tint& LDA,
			Tint& INFO);

void RGETRI (const Tint& N,
			 Treal A[], const Tint& LDA,
			 const Tint IPIV[],
			 Treal WORK[], const Tint& LWORK,
			 Tint& INFO);

void RGETRS (const char*TRANS,
			 const Tint& N,
			 const Tint& NRHS,
			 const Treal A[], const Tint& LDA,
			 const Tint IPIV[],
			 Treal B[], const Tint& LDB,
			 Tint& INFO);

void RGESV (const Tint& N,
			const Tint& NRHS,
			Treal A[],
			const Tint& LDA,
			Tint IPIV[],
			Treal B[],
			const Tint& LDB,
			Tint& INFO);

void RGELS (const char*TRANSA,
			const Tint& M,
			const Tint& N,
			const Tint& NRHS,
			Treal A[],
			const Tint& LDA,
			Treal B[],
			const Tint& LDB,
			Treal WORK[],
			const Tint& LWORK,
			Tint& INFO);

void RGEEV (const char*JOBVL, const char*JOBVR,
			const Tint& N,
			Treal A[],
			const Tint& LDA,
			Treal WR[], Treal WI[],
			Treal VL[], const Tint& LDVL,
			Treal VR[], const Tint& LDVR,
			Treal WORK[], const Tint& LWORK,
			Tint& info);

void RGESVD (const char*JOBU, const char*JOBVT,
			 const Tint& M, const Tint& N,
			 Treal A[], const Tint& LDA,
			 Treal S[],
			 Treal U[], const Tint& LDU,
			 Treal VT[], const Tint& LDVT,
			 Treal WORK[], const Tint& LWORK,
			 Tint& INFO);
		 

void CGETRF (const Tint& M, const Tint& N,
			 Tcomplex A[], const Tint& LDA,
			 Tint IPIV[],
			 Tint& INFO);

void CPOTRF(const char*UPLO,
			const Tint& N,
			Tcomplex A[], const Tint& LDA,
			Tint& INFO);

void CGETRI (const Tint& N,
			 Tcomplex A[], const Tint& LDA,
			 const Tint IPIV[],
			 Tcomplex WORK[], const Tint& LWORK,
			 Tint& INFO);

void CGETRS (const char*TRANS,
			 const Tint& N,
			 const Tint& NRHS,
			 const Tcomplex A[], const Tint& LDA,
			 const Tint IPIV[],
			 Tcomplex B[], const Tint& LDB,
			 Tint& INFO);

void CGESV (const Tint& N,
			const Tint& NRHS,
			Tcomplex A[],
			const Tint& LDA,
			Tint IPIV[],
			Tcomplex B[],
			const Tint& LDB,
			Tint& INFO);

void CGELS (const char*TRANSA,
			const Tint& M,
			const Tint& N,
			const Tint& NRHS,
			Tcomplex A[],
			const Tint& LDA,
			Tcomplex B[],
			const Tint& LDB,
			Tcomplex WORK[],
			const Tint& LWORK,
			Tint& INFO);

void CGEEV (const char*JOBVL, const char*JOBVR,
			const Tint& N,
			Tcomplex A[], const Tint& LDA,
			Tcomplex W[],
			Tcomplex VL[], const Tint& LDVL,
			Tcomplex VR[], const Tint& LDVR,
			Tcomplex WORK[], const Tint& LWORK,
			Treal RWORK[],
			Tint& INFO);

void CGESVD (const char*JOBU, const char*JOBVT,
			 const Tint& M, const Tint& N,
			 Tcomplex A[], const Tint& LDA,
			 Treal S[],
			 Tcomplex U[], const Tint& LDU,
			 Tcomplex VT[], const Tint& LDVT,
			 Tcomplex WORK[], const Tint& LWORK,
			 Treal RWORK[],
			 Tint& INFO);

/* Level-1 BLAS routines: */
/* ---------------------- */

Treal RDOT (const Tint& N, const Treal x[], const Tint& incx, const Treal y[], const Tint& incy);

void RAXPY(const Tint& N,
		   const Treal& alpha,
		   const Treal x[], const Tint& incx,
		   Treal y[], const Tint& incy);

void CAXPY(const Tint& N,
		   const Tcomplex& alpha,
		   const Tcomplex x[], const Tint& incx,
		   Tcomplex y[], const Tint& incy);

/* Level-3 BLAS routines: */
/* ---------------------- */
	
void RGEMM (const char* TRANSA, const char* TRANSB,
			const Tint& M, const Tint& N, const Tint& K,
			const Treal& ALPHA,
			const Treal A[], const Tint& LDA,
			const Treal B[], const Tint& LDB,
			const Treal& BETA,
			Treal C[], const Tint& LDC);

void CGEMM (const char* TRANSA, const char* TRANSB,
			const Tint& M, const Tint& N, const Tint& K,
			const Tcomplex& ALPHA,
			const Tcomplex A[], const Tint& LDA,
			const Tcomplex B[], const Tint& LDB,
			const Tcomplex& BETA,
			Tcomplex C[], const Tint& LDC);

}


