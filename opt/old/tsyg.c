#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* B field in nanotesla */
/* Define DEBUG to get informative messages */
/* Define IGNORE_TSYG to use just dipole field (speeds up also) */

/*#define IGNORE_TSYG*/

/*#define DEBUG*/
#define RKQC bsstep
/*#define RKQC rkqc*/

#include "tsyg.h"
#define real double
#define sqr(x) ((x)*(x))
#define pi 3.14159265358979323846264
#define toRadians(x) ((x)*(pi/180.0))
#define toDegrees(x) ((x)*(180.0/pi))
#define Infinity 1.23E30

static float tiltangle = 0.00001;
static int iopt = 4;
	/* this is the Tsyganenko IOPT variable which determines the Kp */
	/* it must be assigned a proper value before calling there rouines */

#ifdef UNICOS
#  define t89c T89C
#else
#  define t89c t89c_
#endif

extern
#ifdef c_plusplus
"C"
#endif
void t89c
	(const int *iopt, const float parmod[10], const float *tiltangle,
	 const float *x, const float *y, const float *z,
	 float *Bx, float *By, float *Bz);

void mainfield(const vector3 r, vector3 *B)
/* This is a C version of the Fortran routine DIP (in file igrf.f) by
   Tsyganenko/Pulkkinen, with zero tilt angle. */
{
	real p,u,v,t, q;
	const real sinTilt=0,cosTilt=1;
	p = sqr(r.x);
	u = sqr(r.z);
	v = 3*r.x*r.z;
	t = sqr(r.y);
	q = 30574.0*pow(p+t+u,-2.5);
	B->x = q*((t+u-2*p)*sinTilt - v*cosTilt);
	B->y = -3*r.y*q*(r.x*sinTilt + r.z*cosTilt);
	B->z = q*((p+t-2*u)*cosTilt - v*sinTilt);
}

void tsyg89(const vector3 r, vector3 *B)
{
	float parmod[10];
	float x,y,z,Bx,By,Bz;
	x = r.x; y = r.y; z = r.z;
	/*ex89kp(&iopt,&tiltangle,&x,&y,&z,&Bx,&By,&Bz);*/
	t89c(&iopt,parmod,&tiltangle,&x,&y,&z,&Bx,&By,&Bz);
	B->x = Bx;
	B->y = By;
	B->z = Bz;
}

#define VECLENGTH 3

static void totfield(const real t, const real r[VECLENGTH], real drdt[VECLENGTH])
{
	real norm;
	vector3 R, Bext,Bint;
	R.x = r[0]; R.y = r[1]; R.z = r[2];
	Bext.x = Bext.y = Bext.z = 0;
#	ifndef IGNORE_TSYG
	tsyg89(R,&Bext);
#	endif
	mainfield(R,&Bint);
	drdt[0] = -(Bext.x + Bint.x);
	drdt[1] = -(Bext.y + Bint.y);
	drdt[2] = -(Bext.z + Bint.z);
	norm = 1.0/sqrt(sqr(drdt[0]) + sqr(drdt[1]) + sqr(drdt[2]));
	drdt[0]*= norm;
	drdt[1]*= norm;
	drdt[2]*= norm;
}


static void nrerror(const char *msg) {
	fprintf(stderr,"*** Numerical Recipes error: %s\n",msg);
	exit(1);
}

/* note n must always be == VECLENGTH when rk4 is called */

static void rk4
	(const real y[VECLENGTH], real dydx[VECLENGTH], const int n,
	 const real x, const real h,
	 real yout[VECLENGTH],
	 void (*derivs)(const real, const real[VECLENGTH], real[VECLENGTH]) )
{
int i;
real xh,hh,h6;
real dym[VECLENGTH],dyt[VECLENGTH],yt[VECLENGTH];
	hh=h*0.5;
	h6=h/6.0;
	xh=x+hh;
	for (i=0;i<n;i++) yt[i]=y[i]+hh*dydx[i];
	(*derivs)(xh,yt,dyt);
	for (i=0;i<n;i++) yt[i]=y[i]+hh*dyt[i];
	(*derivs)(xh,yt,dym);
	for (i=0;i<n;i++) {
		yt[i]=y[i]+h*dym[i];
		dym[i] += dyt[i];
	}
	(*derivs)(x+h,yt,dyt);
	for (i=0;i<n;i++)
		yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);
}

static void rkqc
	(real y[VECLENGTH], real dydx[VECLENGTH], const int n,
	 real *x, const real htry, const real eps, const real yscal[VECLENGTH],
	 real *hdid, real *hnext,
	 void (*derivs)(const real, const real[VECLENGTH], real [VECLENGTH]))
{
int i;
real xsav,hh,h,temp,errmax;
real dysav[VECLENGTH],ysav[VECLENGTH],ytemp[VECLENGTH];
const real PGROW = -0.20;
const real PSHRNK = -0.25;
const real FCOR = 1.0/15.0;
const real SAFETY = 0.9;
const real ERRCON = 6E-4;
	xsav=(*x);
	for (i=0;i<n;i++) {
		ysav[i]=y[i];
		dysav[i]=dydx[i];
	}
	h=htry;
	for (;;) {
		hh=0.5*h;
		rk4(ysav,dysav,n,xsav,hh,ytemp,derivs);
		*x=xsav+hh;
		(*derivs)(*x,ytemp,dydx);
		rk4(ytemp,dydx,n,*x,hh,y,derivs);
		*x=xsav+h;
		if (*x == xsav) nrerror("Step size too small in routine RKQC");
		rk4(ysav,dysav,n,xsav,h,ytemp,derivs);
		errmax=0.0;
		for (i=0;i<n;i++) {
			ytemp[i]=y[i]-ytemp[i];
			temp=fabs(ytemp[i]/yscal[i]);
			if (errmax < temp) errmax=temp;
		}
		errmax/= eps;
		if (errmax <= 1.0) {
			*hdid=h;
			*hnext=(errmax > ERRCON ?
				SAFETY*h*exp(PGROW*log(errmax)) : 4.0*h);
			break;
		}
		h=SAFETY*h*exp(PSHRNK*log(errmax));
	}
	for (i=0;i<n;i++) y[i] += ytemp[i]*FCOR;
}

static void mmid
	(real y[VECLENGTH], real dydx[VECLENGTH], const int nvar, const real xs,
	 const real htot, const int nstep, real yout[VECLENGTH],
	 void (*derivs)(const real, const real[VECLENGTH], real [VECLENGTH]) )
{
int n,i;
real x,swap,h2,h,ym[VECLENGTH],yn[VECLENGTH];
	h=htot/nstep;
	for (i=0;i<nvar;i++) {
		ym[i]=y[i];
		yn[i]=y[i]+h*dydx[i];
	}
	x=xs+h;
	(*derivs)(x,yn,yout);
	h2=2.0*h;
	for (n=2;n<=nstep;n++) {
		for (i=0;i<nvar;i++) {
			swap=ym[i]+h2*yout[i];
			ym[i]=yn[i];
			yn[i]=swap;
		}
		x += h;
		(*derivs)(x,yn,yout);
	}
	for (i=0;i<nvar;i++)
		yout[i]=0.5*(ym[i]+yn[i]+h*yout[i]);
}

#define NUSE 7
#define IMAX 11

static void rzextr
	(int iest, real xest, real yest[VECLENGTH],
	 real yz[VECLENGTH], real dy[VECLENGTH], const int nv,
	 real x[IMAX+1], real d[VECLENGTH+1][NUSE+1])
{
int m1,k,j;
real yy,v,ddy,c,b1,b,fx[NUSE+1];
	x[iest]=xest;
	if (iest == 1)
		for (j=0;j<nv;j++) {
			yz[j]=yest[j];
			d[j][1]=yest[j];
			dy[j]=yest[j];
		}
	else {
		m1=(iest < NUSE ? iest : NUSE);
		for (k=1;k<=m1-1;k++)
			fx[k+1]=x[iest-k]/(xest!=0 ? xest : 1);
		for (j=0;j<nv;j++) {
			yy=yest[j];
			v=d[j][1];
			c=yy;
			d[j][1]=yy;
			for (k=2;k<=m1;k++) {
				b1=fx[k]*v;
				b=b1-c;
				if (b) {
					b=(c-v)/(b!=0 ? b : 1);
					ddy=c*b;
					c=b1*b;
				} else
					ddy=v;
				if (k != m1) v=d[j][k];
				d[j][k]=ddy;
				yy += ddy;
			}
			dy[j]=ddy;
			yz[j]=yy;
		}
	}
}

static void bsstep
	(real y[VECLENGTH], real dydx[VECLENGTH], const int nv,
	 real *xx, const real htry, const real eps, const real yscal[VECLENGTH],
	 real *hdid, real *hnext,
	 void (*derivs)(const real, const real[VECLENGTH], real [VECLENGTH]))
{
const real SHRINK = 0.95;
const real GROW = 1.2;
int i,j;
real x[IMAX+1], d[VECLENGTH+1][NUSE+1];
real xsav,xest,h,errmax,temp;
real ysav[VECLENGTH],dysav[VECLENGTH],yseq[VECLENGTH],yerr[VECLENGTH];
static int nseq[IMAX+1]={0,2,4,6,8,12,16,24,32,48,64,96};
	h=htry;
	xsav=(*xx);
	for (i=0;i<nv;i++) {
		ysav[i]=y[i];
		dysav[i]=dydx[i];
	}
	for (;;) {
		for (i=1;i<=IMAX;i++) {
			mmid(ysav,dysav,nv,xsav,h,nseq[i],yseq,derivs);
			xest=(temp=h/nseq[i],temp*temp);
			rzextr(i,xest,yseq,y,yerr,nv,x,d);
			errmax=0.0;
			for (j=0;j<nv;j++)
				if (errmax < fabs(yerr[j]/yscal[j]))
					errmax=fabs(yerr[j]/yscal[j]);
			errmax /= eps;
			if (errmax < 1.0) {
				*xx += h;
				*hdid=h;
				*hnext = i==NUSE? h*SHRINK : i==NUSE-1?
					h*GROW : (h*nseq[NUSE-1])/nseq[i];
				return;
			}
		}
		h *= 0.25;
		for (i=0;i<(IMAX-NUSE)/2;i++) h /= 2.0;
		if ((*xx+h) == (*xx)) nrerror("Step size underflow in BSSTEP");
	}
}

#undef IMAX
#undef NUSE


static void odeint
	(real ystart[VECLENGTH], const int nvar,
	 const real x1, const real x2, const real eps,
	 const real h1, const real hmin, const real hmax, int *nok, int *nbad,
	 void (*derivs)(const real, const real[VECLENGTH], real[VECLENGTH]),
	 int (*stopnow)(const real[VECLENGTH]),
	 int (*closeenough)(const real[VECLENGTH]),
	 void (*rkqc)(real y[VECLENGTH], real dydx[VECLENGTH], const int,
				  real *, const real, const real,
				  const real yscal[VECLENGTH],
				  real *, real *,
				  void (*derivs)(const real, const real[VECLENGTH], real[VECLENGTH])) )
{
int nstp,i;
const int MAXSTP = 10000;
const real TINY = 1.0E-30;
real x,hnext,hdid,h,xold;
real yscal[VECLENGTH],y[VECLENGTH],dydx[VECLENGTH];
real yold[VECLENGTH],dydxold[VECLENGTH];
	x=x1;
	h=(x2 > x1) ? fabs(h1) : -fabs(h1);
	*nok = (*nbad) = 0;
	for (i=0; i<nvar; i++) y[i] = ystart[i];
	for (nstp=1; nstp<=MAXSTP; nstp++) {
		(*derivs)(x,y,dydx);
		for (i=0; i<nvar; i++)
			yscal[i] = fabs(y[i]) + fabs(dydx[i]*h) + TINY;
		if ((x+h-x2)*(x+h-x1) > 0.0) h = x2-x;
		if (fabs(h) > hmax) h = hmax*h/fabs(h);
		for (i=0; i<nvar; i++) {
			yold[i] = y[i];
			dydxold[i] = dydx[i];
		}
		xold = x;
restart:
		(*rkqc)(y,dydx,nvar,&x,h,eps,yscal,&hdid,&hnext,derivs);
		if ((*stopnow)(y) && !(*closeenough)(y)) {
			for (i=0; i<nvar; i++) {
				y[i] = yold[i];
				dydx[i] = dydxold[i];
			}
			x = xold;
			h/= 3.0;
			++(*nbad);
			goto restart;
		}
		if (hdid == h) ++(*nok); else ++(*nbad);
		if ((x-x2)*(x2-x1) >= 0.0 || (*closeenough)(y)) {
			for (i=0; i<nvar; i++) ystart[i] = y[i];
			return;
		}
		if (fabs(hnext) <= hmin) nrerror("Step size too small in ODEINT");
		h = hnext;
	}
	nrerror("Too many steps in routine ODEINT");
}

void setIOPT(real ioptNew) {
	iopt = (int)(ioptNew+0.5);	/* just set the static variable */
}

static int isInSouthernHemisphere(const real r[3]) {
	return r[2] < 0;
}

static int isCloseToEqPlane(const real r[3]) {
const real zAccuracy = 0.001;	/* tolerance of final eq. plane in Earth radii */
	return fabs(r[2]) < zAccuracy;
}

const real rIonosphere = 1.017;			/* Ionospheric altitude in Earth radii */

static int isInIonosphere(const real r[3]) {
	return sqr(r[0])+sqr(r[1])+sqr(r[2]) <= sqr(rIonosphere);
}

static int isCloseToIonosphere(const real r[3]) {
const real zAccuracy = 0.001;	/* tolerance of final eq. plane in Earth radii */
	return fabs(sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2])) - rIonosphere) < zAccuracy;
}

void i2m(real theta, real phi, real *x, real *y)
/* theta: colatitude (GSM) in deg, phi: longitude (GSM) in deg,
   on output: x,y (GSM) on equatorial plane */
{
const real eps = 1E-10;			/* Relative error, whatever it means? */
const real initialStep = 0.001;	/* from-ionosphere initial step in Earth radii */
int nok,nbad;
real x0,y0,z0;
real r[3];
	x0 = rIonosphere*sin(toRadians(theta))*cos(toRadians(phi));
	y0 = rIonosphere*sin(toRadians(theta))*sin(toRadians(phi));
	z0 = rIonosphere*cos(toRadians(theta));
	r[0] = x0;
	r[1] = y0;
	r[2] = z0;
#	ifdef DEBUG
	fprintf(stderr,"i2m: x=%G, y=%G, z=%G\n",x0,y0,z0);
#	endif
	odeint(r,3,0.,Infinity,eps,initialStep,0,Infinity/*2.0*/,&nok,&nbad,
		   totfield,isInSouthernHemisphere,isCloseToEqPlane,RKQC);
#	ifdef DEBUG
	fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G\n",
			nok,nbad,r[0],r[1],r[2]);
#	endif
	/* Now the point r is very near the equatorial plane. */
	*x = r[0];
	*y = r[1];
}

void MapToMagnetosphere
	(const real theta, const real phi, real *x, real *y, real *z,
	 int (*overnesstest)(const real [3]),
	 int (*closenesstest)(const real [3]))
/* theta: colatitude (GSM) in deg, phi: longitude (GSM) in deg,
   on output: x,y,z (GSM) on first encountered point for which endtest({x,y,z})
   is nonzero. */
{
const real eps = 1E-10;			/* Relative error, whatever it means? */
const real initialStep = 0.001;	/* from-ionosphere initial step in Earth radii */
int nok,nbad;
real x0,y0,z0;
real r[3];
	x0 = rIonosphere*sin(toRadians(theta))*cos(toRadians(phi));
	y0 = rIonosphere*sin(toRadians(theta))*sin(toRadians(phi));
	z0 = rIonosphere*cos(toRadians(theta));
	r[0] = x0;
	r[1] = y0;
	r[2] = z0;
#	ifdef DEBUG
	fprintf(stderr,"MapToMagnetosphere: x=%G, y=%G, z=%G\n",x0,y0,z0);
#	endif
	odeint(r,3,0.,Infinity,eps,initialStep,0,Infinity/*2.0*/,&nok,&nbad,
		   totfield,overnesstest,closenesstest,RKQC);
#	ifdef DEBUG
	fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G\n",
			nok,nbad,r[0],r[1],r[2]);
#	endif
	/* Now the point r should satisfy closenesstest(r). */
	*x = r[0];
	*y = r[1];
	*z = r[2];
} /* MapToMagnetosphere */

void m2i(real x, real y, real *theta, real *phi)
/* x,y: GSM coordinates on equatorial plane (z=0)
   theta: colatitude (GSM) in deg, phi: longitude (GSM) in deg */
{
const real eps = 1E-8;			/* Relative error, whatever it means? */
const real initialStep = 0.001;	/* from-eq.plane initial step in Earth radii */
int nok,nbad;
real r[3], R,Rho,Th,Ph;
	r[0] = x;
	r[1] = y;
	r[2] = 0;
#	ifdef DEBUG
	fprintf(stderr,"m2i: x=%G, y=%G\n",x,y);
#	endif
	odeint(r,3,0,-Infinity,eps,-initialStep,0,Infinity,
		   &nok,&nbad,
		   totfield,isInIonosphere,isCloseToIonosphere,RKQC);
#	ifdef DEBUG
	fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G\n",
			nok,nbad,r[0],r[1],r[2]);
#	endif
	/* Now the point r is at ionospheric altitudes. */
	R = sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2]));
	Rho = sqrt(sqr(r[0]) + sqr(r[1]));
	Th = asin(Rho/R);
	Ph = atan2(r[1],r[0]);
	if (Ph < 0) Ph+= 2*pi;
	*theta = toDegrees(Th);
	*phi = toDegrees(Ph);
}

void MapToIonosphere
	(const real x, const real y, const real z, real *theta, real *phi)
/* This is exactly similar to m2i except that the starting point is not necessarily
   at equatorial plane (z=0). x,y,z: GSM. theta: colatitude (GSM) in deg.
   phi: longitude (GSM) in deg. */
{
const real eps = 1E-8;			/* Relative error, whatever it means? */
const real initialStep = 0.001;	/* from-eq.plane initial step in Earth radii */
int nok,nbad;
real r[3], R,Rho,Th,Ph;
	r[0] = x;
	r[1] = y;
	r[2] = z;
#	ifdef DEBUG
	fprintf(stderr,"MapToIonosphere: x=%G, y=%G, z=%G\n",x,y,z);
#	endif
	odeint(r,3,0,-Infinity,eps,-initialStep,0,Infinity,
		   &nok,&nbad,
		   totfield,isInIonosphere,isCloseToIonosphere,RKQC);
#	ifdef DEBUG
	fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G\n",
			nok,nbad,r[0],r[1],r[2]);
#	endif
	/* Now the point r is at ionospheric altitudes. */
	R = sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2]));
	Rho = sqrt(sqr(r[0]) + sqr(r[1]));
	Th = asin(Rho/R);
	Ph = atan2(r[1],r[0]);
	if (Ph < 0) Ph+= 2*pi;
	*theta = toDegrees(Th);
	*phi = toDegrees(Ph);
} /* MapToIonosphere */

#if 0

static int isInNorthernHemisphere(const real r[3]) {
	return r[2] < 0;
}

static int isNotInIonosphere(const real r[3]) {
	return sqr(r[0])+sqr(r[1])+sqr(r[2]) > sqr(rIonosphere);
}

void i2m(real theta, real phi, real *x, real *y)
/* theta: colatitude (GSM) in deg, phi: longitude (GSM) in deg,
   on output: x,y (GSM) on equatorial plane */
{
const real eps = 1E-8;			/* Relative error, whatever it means? */
const real zAccuracy = 0.001;	/* tolerance of final eq. plane in Earth radii */
const real initialStep = 0.001;	/* from-ionosphere initial step in Earth radii */
const real minimumAllowedStep = 0.0001;	/* Earth radii */
int nok,nbad;
real x0,y0,z0;
real r[3], B[3], newstep, dtmin;
	x0 = rIonosphere*sin(toRadians(theta))*cos(toRadians(phi));
	y0 = rIonosphere*sin(toRadians(theta))*sin(toRadians(phi));
	z0 = rIonosphere*cos(toRadians(theta));
	r[0] = x0;
	r[1] = y0;
	r[2] = z0;
#	ifdef DEBUG
	fprintf(stderr,"i2m: x=%G, y=%G, z=%G\n",x0,y0,z0);
#	endif

	newstep = initialStep;

	do {

		dtmin = minimumAllowedStep;
		if (dtmin > 0.1*newstep) dtmin = 0.1*newstep;
		odeint(r,3,0.,Infinity,eps,newstep,dtmin,Infinity/*2.0*/,&nok,&nbad,
			   totfield,isInSouthernHemisphere,RKQC);
#		ifdef DEBUG
		fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G\n",
				nok,nbad,r[0],r[1],r[2]);
#		endif

		totfield(0.,r,B);
		newstep = fabs(r[2]);
		dtmin = minimumAllowedStep;
		if (dtmin > 0.1*newstep) dtmin = 0.1*newstep;
		odeint(r,3,0.,-Infinity,eps,-newstep,dtmin,Infinity,&nok,&nbad,
			   totfield,isInNorthernHemisphere,RKQC);

#		ifdef DEBUG
		fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G\n",
				nok,nbad,r[0],r[1],r[2]);
#		endif
		totfield(0.,r,B);
		newstep = fabs(r[2]);

	} while (fabs(r[2]) > zAccuracy);

	/* Now the point r is very near the equatorial plane. */

	*x = r[0];
	*y = r[1];
}

void m2i(real x, real y, real *theta, real *phi)
/* x,y: GSM coordinates on equatorial plane (z=0)
   theta: colatitude (GSM) in deg, phi: longitude (GSM) in deg */
{
const real eps = 1E-8;			/* Relative error, whatever it means? */
const real zAccuracy = 0.001;	/* tolerance of final eq. plane in Earth radii */
const real initialStep = 0.001;	/* from-eq.plane initial step in Earth radii */
const real minimumAllowedStep = 0.0001;	/* Earth radii */
int nok,nbad,FirstTime=1;
real r[3], B[3], newstep, dtmin, R,Rho,Th,Ph;
	r[0] = x;
	r[1] = y;
	r[2] = 0;
#	ifdef DEBUG
	fprintf(stderr,"m2i: x=%G, y=%G\n",x,y);
#	endif
	totfield(0.,r,B);
	newstep = initialStep;

	do {

		dtmin = minimumAllowedStep;
		if (dtmin > 0.1*newstep) dtmin = 0.1*newstep;
		odeint(r,3,0,-Infinity,eps,-newstep,dtmin,
			   FirstTime ? Infinity /*1.0*/ : 0.5*fabs(R-rIonosphere),
			   &nok,&nbad,
			   totfield,isInIonosphere,RKQC);
		R = sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2]));
#		ifdef DEBUG
		fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G, R=%G\n",
				nok,nbad,r[0],r[1],r[2], R);
#		endif

		totfield(0.,r,B);
		newstep = initialStep;

		dtmin = minimumAllowedStep;
		if (dtmin > 0.1*newstep) dtmin = 0.1*newstep;
		odeint(r,3,0,Infinity,eps,newstep,dtmin,1.1*fabs(R-rIonosphere),&nok,&nbad,
			   totfield,isNotInIonosphere,RKQC);
		R = sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2]));
		totfield(0.,r,B);
		newstep = initialStep;
#		ifdef DEBUG
		fprintf(stderr,"odeint finished: nok=%d, nbad=%d, x=%G, y=%G, z=%G, R=%G\n",
				nok,nbad,r[0],r[1],r[2], R);
#		endif
		FirstTime = 0;

	} while (fabs(R-rIonosphere) > zAccuracy);

	/* Now the point r is at ionospheric altitudes. */

	Rho = sqrt(sqr(r[0]) + sqr(r[1]));
	Th = asin(Rho/R);
	Ph = atan2(r[1],r[0]);
	if (Ph < 0) Ph+= 2*pi;
	*theta = toDegrees(Th);
	*phi = toDegrees(Ph);
}

#endif

