#if 0

static bool isCloseToDomainBoundary(const Treal r[3])
{
	int i;
	if (!finite(r[0]) || !finite(r[1]) || !finite(r[2])) return true;
	for (i=0; i<tracer_Nendconds; i++) {
		Treal v=0;
		switch (tracer_endconds[i].var) {
		case ENDCOND_R:
			v = sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2]));
			break;
		case ENDCOND_X:
			v = r[0];
			break;
		case ENDCOND_Y:
			v = r[1];
			break;
		case ENDCOND_Z:
			v = r[2];
			break;
		}
		const Treal diff = fabs(v - tracer_endconds[i].val);
		if (diff <= tracer_endaccuracy) return true;
	}
	return false;
}

static int nrerrors_issued = 0;;

static void nrerror(const char *msg) {
	if (nrerrors_issued <= 5) cerr << "*** geopack tracer error: " << msg << "\n";
	nrerrors_issued++;
}

/* note n must always be == VECLENGTH when rk4 is called */

static void rk4
	(const Treal y[VECLENGTH], Treal dydx[VECLENGTH], int n,
	 Treal x, Treal h,
	 Treal yout[VECLENGTH],
	 void (*derivs)(Treal, const Treal[VECLENGTH], Treal[VECLENGTH]) )
{
int i;
Treal xh,hh,h6;
Treal dym[VECLENGTH],dyt[VECLENGTH],yt[VECLENGTH];
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
	(Treal y[VECLENGTH], Treal dydx[VECLENGTH], const int n,
	 Treal *x, Treal htry, Treal eps, const Treal yscal[VECLENGTH],
	 Treal *hdid, Treal *hnext,
	 void (*derivs)(Treal, const Treal[VECLENGTH], Treal [VECLENGTH]))
{
int i;
Treal xsav,hh,h,temp,errmax;
Treal dysav[VECLENGTH],ysav[VECLENGTH],ytemp[VECLENGTH];
const Treal PGROW = -0.20;
const Treal PSHRNK = -0.25;
const Treal FCOR = 1.0/15.0;
const Treal SAFETY = 0.9;
const Treal ERRCON = 6E-4;
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
		if (*x == xsav) {nrerror("step size too small in routine RKQC"); break;}
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
	(Treal y[VECLENGTH], Treal dydx[VECLENGTH], int nvar, Treal xs,
	 Treal htot, int nstep, Treal yout[VECLENGTH],
	 void (*derivs)(Treal, const Treal[VECLENGTH], Treal [VECLENGTH]) )
{
int n,i;
Treal x,swap,h2,h,ym[VECLENGTH],yn[VECLENGTH];
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
	(int iest, Treal xest, Treal yest[VECLENGTH],
	 Treal yz[VECLENGTH], Treal dy[VECLENGTH], int nv,
	 Treal x[IMAX+1], Treal d[VECLENGTH+1][NUSE+1])
{
int m1,k,j;
Treal yy,v,ddy=0,c,b1,b,fx[NUSE+1];
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
	(Treal y[VECLENGTH], Treal dydx[VECLENGTH], int nv,
	 Treal *xx, Treal htry, Treal eps, const Treal yscal[VECLENGTH],
	 Treal *hdid, Treal *hnext,
	 void (*derivs)(Treal, const Treal[VECLENGTH], Treal [VECLENGTH]))
{
const Treal SHRINK = 0.95;
const Treal GROW = 1.2;
int i,j;
Treal x[IMAX+1], d[VECLENGTH+1][NUSE+1];
Treal xsav,xest,h,errmax,temp;
Treal ysav[VECLENGTH],dysav[VECLENGTH],yseq[VECLENGTH],yerr[VECLENGTH];
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
		if ((*xx+h) == (*xx)) {nrerror("step size underflow in BSSTEP"); break;}
	}
}

#undef IMAX
#undef NUSE

static void odeint
	(Treal ystart[VECLENGTH], int nvar,
	 Treal x1, Treal x2, Treal eps,
	 Treal h1, Treal hmin, Treal hmax, int *nok, int *nbad,
	 void (*derivs)(Treal, const Treal[VECLENGTH], Treal[VECLENGTH]),
	 bool (*stopnow)(const Treal[VECLENGTH]),
	 bool (*closeenough)(const Treal[VECLENGTH]),
	 void (*rkqc)(Treal y[VECLENGTH], Treal dydx[VECLENGTH], int,
				  Treal *, Treal, Treal,
				  const Treal yscal[VECLENGTH],
				  Treal *, Treal *,
				  void (*derivs)(Treal, const Treal[VECLENGTH], Treal[VECLENGTH])) )
{
int nstp,i;
const int MAXSTP = 1000 /*10000*/;
const Treal TINY = 1.0E-30;
Treal x,hnext,hdid,h,xold;
Treal yscal[VECLENGTH],y[VECLENGTH],dydx[VECLENGTH];
Treal yold[VECLENGTH],dydxold[VECLENGTH];
	x=x1;
	h=(x2 > x1) ? fabs(h1) : -fabs(h1);
	*nok = (*nbad) = 0;
	for (i=0; i<nvar; i++) y[i] = ystart[i];
	for (nstp=1; nstp<=MAXSTP; nstp++) {
		(*derivs)(x,y,dydx);
		for (i=0; i<nvar; i++)
			yscal[i] = fabs(y[i]) + fabs(dydx[i]*h) + TINY;
		// --- save results
		if (x_sav && y_sav && z_sav && count_sav < MAX_TRACE_POINTS) {
			x_sav[count_sav] = y[0];
			y_sav[count_sav] = y[1];
			z_sav[count_sav] = y[2];
			count_sav++;
		}
		// ---
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
		if (fabs(hnext) <= hmin) {
			hnext = (hnext >= 0 ? +1 : -1)*hmin*1.1;
			nrerror("step size too small in ODEINT");
		}
		h = hnext;
	}
	nrerror("too many steps in routine ODEINT");
}

void Geopack_trace_old(TFieldModel model1, TFieldModel model2,
				   const float paramvec[11],
				   int sign, Treal maxstep,
				   const TTraceEndCondition endconds[9], int Nendconds,
				   Treal x0, Treal y0, Treal z0, Treal epoch, Treal psi, bool psi_given,
				   float x[MAX_TRACE_POINTS], float y[MAX_TRACE_POINTS], float z[MAX_TRACE_POINTS],
				   int& Npts)
{
//	cout << "Geopack_trace: model1=" << model1 << ", model2=" << model2 << "\n";
	tracer_model1 = model1;
	tracer_model2 = model2;
	tracer_epoch = epoch;
	int i;
	for (i=0; i<11; i++) tracer_paramvec[i] = paramvec[i];
	for (i=0; i<8; i++) tracer_endconds[i] = endconds[i];
	tracer_Nendconds = Nendconds;
	tracer_psi = psi;
	tracer_psi_given = psi_given;
	tracer_sign = sign;
	const bool use_bs = true;
	tracer_endaccuracy = 1e-4;
	Treal r[3];
	r[0] = x0; r[1] = y0; r[2] = z0;
	const Treal eps = 0.1*tracer_endaccuracy;
	const Treal initialStep = 1e-1;
	const Treal Infinity = 1.23E30;
	int nok,nbad;
	x_sav = &x[0];
	y_sav = &y[0];
	z_sav = &z[0];
	count_sav = 0;
	nrerrors_issued = 0;
	odeint(r,3,0.,Infinity,eps,initialStep,use_bs ? 1e-4 : 1e-5,maxstep,&nok,&nbad,
		   &tracer,&isOutsideDomain,&isCloseToDomainBoundary,use_bs ? &bsstep : &rkqc);
	if (count_sav <= 0) count_sav = 1;
	x[count_sav-1] = r[0];
	y[count_sav-1] = r[1];
	z[count_sav-1] = r[2];
	Npts = count_sav;
}

#endif

