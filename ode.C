/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "ode.H"
#endif

#include <cmath>
#include <iostream>
#include "ode.H"

#define NVARS_MAX 4
#define sqr(x) ((x)*(x))

static void nrerror(const char *msg)
{
	cerr << "warning: " << msg << " (ode.C)\n";
}

static void mmid(Treal y[], Treal dydx[], int nvar, Treal xs, Treal htot, int nstep, Treal yout[],
				 void (*derivs)(Treal, Treal[], Treal[]))
{
	int n,i;
	Treal x,swap,h2,h;
	Treal ym[NVARS_MAX+1],yn[NVARS_MAX+1];
	h = htot/nstep;
	for (i=1; i<=nvar; i++) {
		ym[i] = y[i];
		yn[i] = y[i] + h*dydx[i];
	}
	x = xs + h;
	(*derivs)(x,yn,yout);
	h2 = 2.0*h;
	for (n=2; n<=nstep; n++) {
		for (i=1; i<=nvar; i++) {
			swap = ym[i] + h2*yout[i];
			ym[i] = yn[i];
			yn[i] = swap;
		}
		x+= h;
		(*derivs)(x,yn,yout);
	}
	for (i=1; i<=nvar; i++)
		yout[i] = 0.5*(ym[i] + yn[i] + h*yout[i]);
}

#define KMAXX 8				/* maximum row number used in extrapolation */
#define IMAXX (KMAXX+1)

static Treal *dmatrix,*xvector;				// pointers to matrix and vector used by pzextr or rzextr

static void pzextr(int iest, Treal xest, Treal yest[], Treal yz[], Treal dy[], int nv)
{
	int k1,j;
	Treal q,f2,f1,delta;
	Treal c[NVARS_MAX+1];
	xvector[iest] = xest;
	for (j=1; j<=nv; j++) dy[j] = yz[j] = yest[j];
	if (iest == 1) {
		for (j=1; j<=nv; j++)
			dmatrix[j*(KMAXX+1)+1] = yest[j];
	} else {
		for (j=1; j<=nv; j++) c[j] = yest[j];
		for (k1=1; k1<iest; k1++) {
			delta = 1.0/(xvector[iest-k1] - xest);
			f1 = xest*delta;
			f2 = xvector[iest-k1]*delta;
			for (j=1; j<=nv; j++) {
				q = dmatrix[j*(KMAXX+1)+k1];
				dmatrix[j*(KMAXX+1)+k1] = dy[j];
				delta = c[j]-q;
				dy[j] = f1*delta;
				c[j] = f2*delta;
				yz[j]+= dy[j];
			}
		}
		for (j=1; j<=nv; j++) dmatrix[j*(KMAXX+1)+iest] = dy[j];
	}
}

void bsstep(Treal y[], Treal dydx[], int nv, Treal& xx, Treal htry, Treal eps,
			Treal yscal[], Treal &hdid, Treal &hnext,
			void (*derivs)(Treal, Treal[], Treal[]))
{
	const Treal SAFE1 = 0.25;	// safety factors
	const Treal SAFE2 = 0.7;
	const Treal REDMAX = 1e-5;	// maximum factor for stepsize reduction
	const Treal REDMIN = 0.8;	// minimum factor for stepsize reduction
	const Treal TINY = 1e-30;	// prevents division by zero
	const Treal SCALMX = 0.1;	// 1/SCALMX is the maximum factor by which a stepsize can be increased
	int i,iq,k,kk,km=0;
	static int first=1,kmax,kopt;
	static Treal epsold = -1.0, xnew;
	Treal eps1,errmax,fact,h,red=0,scale=0,work,wrkmin,xest;
	static Treal a[IMAXX+1];
	static Treal alf[KMAXX+1][KMAXX+1];
	static int nseq[IMAXX+1] = {0,2,4,6,8,10,12,14,16,18};
	int reduct,exitflag=0;
	dmatrix = new Treal [(nv+1)*(KMAXX+1)];
	xvector = new Treal [KMAXX+1];
	Treal err[KMAXX+1];
	Treal yerr[NVARS_MAX+1];
	Treal ysav[NVARS_MAX+1];
	Treal yseq[NVARS_MAX+1];
	if (eps != epsold) {		// new tolerance, reinitialize
		hnext = xnew = -1.0e29;		// impossible values
		eps1 = SAFE1*eps;
		a[1] = nseq[1] + 1;			// compute work coefficients Ak
		for (k=1; k<=KMAXX; k++) a[k+1] = a[k] + nseq[k+1];
		for (iq=2; iq<=KMAXX; iq++) {
			for (k=1; k<iq; k++)
				alf[k][iq] = pow(eps1,(a[k+1]-a[iq+1])/((a[iq+1]-a[1]+1.0)*(2*k+1)));
		}
		epsold = eps;
		for (kopt=2; kopt<KMAXX; kopt++)		// determine optimal row number for convergence
			if (a[kopt+1] > a[kopt]*alf[kopt-1][kopt]) break;
		kmax = kopt;
	}
	h = htry;
	for (i=1; i<=nv; i++) ysav[i] = y[i];		// save starting values
	if (xx != xnew || h != hnext) {		// new stepsize or new integration: re-establish order window
		first = 1;
		kopt = kmax;
	}
	reduct = 0;
	for (;;) {
		for (k=1; k<=kmax; k++) {	// evaluate sequence of modified midpoint integrations
			xnew = xx + h;
			if (xnew == xx) nrerror("stepsize underflow in bsstep");
			mmid(ysav,dydx,nv,xx,h,nseq[k],yseq,derivs);
			xest = sqr(h/nseq[k]);
			pzextr(k,xest,yseq,y,yerr,nv);
			if (k != 1) {
				errmax = TINY;
				for (i=1; i<=nv; i++) errmax = max(errmax,fabs(yerr[i]/yscal[i]));
				errmax/= eps;
				km = k-1;
				err[km] = pow(errmax/SAFE1,1.0/(2*km+1));
			}
			if (k != 1 && (k >= kopt-1 || first)) {
				if (errmax < 1.0) {
					exitflag = 1;		// converged
					break;
				}
				if (k == kmax || k == kopt+1) {
					red = SAFE2/err[km];
					break;
				} else if (k == kopt && alf[kopt-1][kopt] < err[km]) {
					red = 1.0/err[km];
					break;
				} else if (kopt == kmax && alf[km][kmax-1] < err[km]) {
					red = alf[km][kmax-1]*SAFE2/err[km];
					break;
				} else if (alf[km][kopt] < err[km]) {
					red = alf[km][kopt-1]/err[km];
					break;
				}
			}
		}
		if (exitflag) break;
		red = min(red,REDMIN);
		red = max(red,REDMAX);
		h*= red;
		reduct = 1;
	}
	xx = xnew;
	hdid = h;
	first = 0;
	wrkmin = 1.0e35;
	for (kk=1; kk<=km; kk++) {
		fact = max(err[kk],SCALMX);
		work = fact*a[kk+1];
		if (work < wrkmin) {
			scale = fact;
			wrkmin = work;
			kopt = kk+1;
		}
	}
	hnext = h/scale;
	if (kopt >= k && kopt != kmax && !reduct) {
		// check for possible order increase, but not if stepsize was just reduced
		fact = max(scale/alf[kopt-1][kopt],SCALMX);
		if (a[kopt+1]*fact <= wrkmin) {
			hnext = h/fact;
			kopt++;
		}
	}
	delete [] xvector;
	delete [] dmatrix;
}

#undef IMAXX
#undef KMAXX

void odeint(Treal ystart[], int nvar, Treal x1, Treal x2, Treal eps, Treal h1, Treal hmin,
			int& nok, int& nbad,
			void (*derivs)(Treal, Treal[], Treal[]),
			void (*rkqc)(Treal[],Treal[],int,Treal&,Treal,Treal,
						 Treal[],Treal&,Treal&,void (*)(Treal, Treal[], Treal[])))
{
	if (nvar > NVARS_MAX) {
		cerr << "*** odeint: nvars=" << nvar << " too large\n";
		exit(1);
	}
	const int MAXSTP = 10000;
	const Treal TINY = 1.0e-30;
	int nstp,i;
	Treal x,hnext,hdid,h;
	Treal yscal[NVARS_MAX+1];
	Treal y[NVARS_MAX+1];
	Treal dydx[NVARS_MAX+1];
	x=x1;
	h=(x2 > x1) ? fabs(h1) : -fabs(h1);
	nok = nbad = 0;
	for (i=1;i<=nvar;i++) y[i]=ystart[i];
	for (nstp=1;nstp<=MAXSTP;nstp++) {
		(*derivs)(x,y,dydx);
		for (i=1;i<=nvar;i++)
			yscal[i]=fabs(y[i])+fabs(dydx[i]*h)+TINY;
		if ((x+h-x2)*(x+h-x1) > 0.0) h=x2-x;
		(*rkqc)(y,dydx,nvar,x,h,eps,yscal,hdid,hnext,derivs);
		if (hdid == h) ++nok; else ++nbad;
		if ((x-x2)*(x2-x1) >= 0.0) {
			for (i=1;i<=nvar;i++) ystart[i]=y[i];
			return;
		}
		if (fabs(hnext) <= hmin) {nrerror("step size too small in odeint()"); break;}
		h=hnext;
	}
	nrerror("too many steps in routine odeint()");
}

#undef MAXSTP
#undef TINY
