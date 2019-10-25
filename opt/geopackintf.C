#ifdef __GNUC__
#  pragma implementation "geopackintf.H"
#endif
#include <iostream>
#include <cmath>
#include <cstdlib>
using namespace std;
#include "geopackintf.H"
#include "geopack.h"
#include "T96.h"
#include "T89c.h"
#include "cdf.h"

typedef void (*TCoordTransFunc)(float&,float&,float&, float&,float&,float&, const int&);

static TFieldModel tracer_model1 = MODEL_IGRF;
static TFieldModel tracer_model2 = MODEL_T89;
static double tracer_epoch = -1;
static float tracer_paramvec[11] = {0};
static float tracer_psi = 0;
static bool tracer_psi_given = false;
static int tracer_sign = 1;
static TTraceEndCondition tracer_endconds[9] = {{ENDCOND_R,false,1e3}};
static int tracer_Nendconds = 1;
static Treal tracer_endaccuracy = 1e-4;

struct TCoordTransRule {
	TCoordTransFunc fn1;
	int dir1;
	TCoordTransFunc fn2;
	int dir2;
	TCoordTransFunc fn3;
	int dir3;
};

/*
  Coordinate systems: GSM,GSE,GEO,GEI,MAG,SM (6)
  Available transformations: GEOMAG,GEIGEO,MAGSM,GSMGSE,SMGSM,GEOGSM (6)
*/
static const TCoordTransRule rules[6][6] = {
	{{0,0,0,0,0,0},							// GSM->GSM
	 {&GSMGSE,1,0,0,0,0},					// GSM->GSE
	 {&GEOGSM,-1,0,0,0,0},					// GSM->GEO
	 {&GEOGSM,-1,&GEIGEO,-1,0,0},			// GSM->GEO->GEI
	 {&GEOGSM,-1,&GEOMAG,1,0,0},			// GSM->GEO->MAG
	 {&SMGSM,-1,0,0,0,0}},					// GSM->SM
	{{&GSMGSE,-1,0,0,0,0},					// GSE->GSM
	 {0,0,0,0,0,0},							// GSE->GSE
	 {&GSMGSE,-1,&GEOGSM,-1,0,0},			// GSE->GSM->GEO
	 {&GSMGSE,-1,&GEOGSM,-1,&GEIGEO,-1},	// GSE->GSM->GEO->GEI
	 {&GSMGSE,-1,&GEOGSM,-1,&GEOMAG,1},		// GSE->GSM->GEO->MAG
	 {&GSMGSE,-1,&SMGSM,-1,0,0}},			// GSE->GSM->SM
	{{&GEOGSM,1,0,0,0,0},					// GEO->GSM
	 {&GEOGSM,1,&GSMGSE,1,0,0},				// GEO->GSM->GSE
	 {0,0,0,0,0,0},							// GEO->GEO
	 {&GEIGEO,-1,0,0,0,0},					// GEO->GEI
	 {&GEOMAG,1,0,0,0,0},					// GEO->MAG
	 {&GEOMAG,1,&MAGSM,1,0,0}},				// GEO->MAG->SM
	{{&GEIGEO,1,&GEOGSM,1,0,0},				// GEI->GEO->GSM
	 {&GEIGEO,1,&GEOGSM,1,&GSMGSE,1},		// GEI->GEO->GSM->GSE
	 {&GEIGEO,1,0,0,0,0},					// GEI->GEO
	 {0,0,0,0,0,0},							// GEI->GEI
	 {&GEIGEO,1,&GEOMAG,1,0,0},				// GEI->GEO->MAG
	 {&GEIGEO,1,&GEOGSM,1,&SMGSM,-1}},		// GEI->GEO->GSM->SM
	{{&GEOMAG,-1,&GEOGSM,1,0,0},			// MAG->GEO->GSM
	 {&GEOMAG,-1,&GEOGSM,1,&GSMGSE,1},		// MAG->GEO->GSM->GSE
	 {&GEOMAG,-1,0,0,0,0},					// MAG->GEO
	 {&GEOMAG,-1,&GEIGEO,-1,0,0},			// MAG->GEO->GEI
	 {0,0,0,0,0,0},							// MAG->MAG
	 {&MAGSM,1,0,0,0,0}},					// MAG->SM
	{{&SMGSM,1,0,0,0,0},					// SM->GSM
	 {&SMGSM,1,&GSMGSE,1,0,0},				// SM->GSM->GSE
	 {&SMGSM,1,&GEOGSM,-1,0,0},				// SM->GSM->GEO
	 {&SMGSM,1,&GEOGSM,-1,&GEIGEO,-1},		// SM->GSM->GEO->GEI
	 {&MAGSM,-1,0,0,0,0},					// SM->MAG
	 {0,0,0,0,0,0}							// SM->SM
	}};

static double latest_epoch = -1;

inline void MMDDtoDDD(int mm, int dd, int& ddd, int yyyy)
{
	int leap;
	if (yyyy % 4 == 0 && (yyyy % 100 != 0 || yyyy % 400 == 0))
		leap = 1;
	else
		leap = 0;
	switch (mm) {
	case 1:
		ddd = dd;
		break;
	case 2:
		ddd = 31+dd;
		break;
	case 3:
		ddd = 31+28+dd+leap;
		break;
	case 4:
		ddd = 31+28+31+dd+leap;
		break;
	case 5:
		ddd = 31+28+31+30+dd+leap;
		break;
	case 6:
		ddd = 31+28+31+30+31+dd+leap;
		break;
	case 7:
		ddd = 31+28+31+30+31+30+dd+leap;
		break;
	case 8:
		ddd = 31+28+31+30+31+30+31+dd+leap;
		break;
	case 9:
		ddd = 31+28+31+30+31+30+31+31+dd+leap;
		break;
	case 10:
		ddd = 31+28+31+30+31+30+31+31+30+dd+leap;
		break;
	case 11:
		ddd = 31+28+31+30+31+30+31+31+30+31+dd+leap;
		break;
	case 12:
		ddd = 31+28+31+30+31+30+31+31+30+31+30+dd+leap;
		break;
	default:
		cerr << "*** MMDDtoDDD: bad mm=" << mm << "\n";
		exit(1);
	}
}

static void Recalc(double epoch)
{
	if (tracer_psi_given) return;
	if (epoch != latest_epoch) {
		latest_epoch = epoch;
		long yyyy,mm,dd,hh,minute,ss,ms;
		EPOCHbreakdown(latest_epoch,&yyyy,&mm,&dd,&hh,&minute,&ss,&ms);
		int ddd;
		MMDDtoDDD(mm,dd,ddd, yyyy);
		float vgsex=-1,vgsey=0,vgsez=0;
//		cout << "RECALC yyyy=" << yyyy << ",ddd=" << ddd << ",hh=" << hh << ",minute=" << minute << ",ss=" << ss << "\n";
		RECALC(yyyy,ddd,hh,minute,ss,vgsex,vgsey,vgsez);
	}
}

void Geopack_coordtrans(TCoordSys fromsys, TCoordSys tosys,
						const Treal x[], const Treal y[], const Treal z[],
						const Treal epoch[],
						int n,
						Treal X[], Treal Y[], Treal Z[])
{
	int i;
	const TCoordTransRule& R = rules[fromsys][tosys];
	int Nfn = 0;
	float xin,yin,zin,xout,yout,zout;
	if (R.fn1) {
		Nfn++;
		if (R.fn2) {
			Nfn++;
			if (R.fn3) Nfn++;
		}
	}
	if (Nfn == 0) {
		for (i=0; i<n; i++) {X[i]=x[i]; Y[i]=y[i]; Z[i]=z[i];}
	} else {
		// common stuff for Nfn=1,2,3: do fn1 from x,y,z to X,Y,Z:
		for (i=0; i<n; i++) {
			Recalc(epoch[i]);
			xin = x[i]; yin = y[i]; zin = z[i];
//			cout << "r_in = (" << xin << "," << yin << "," << zin << ")\n";
			if (R.dir1 > 0) {
				(*R.fn1)(xin,yin,zin,xout,yout,zout,1);
			} else {
				(*R.fn1)(xout,yout,zout,xin,yin,zin,-1);
			}
//			cout << "r_out = (" << xout << "," << yout << "," << zout << ")\n";
			if (Nfn >= 2) {
//				cout << "moi, lisaa 2\n";
				xin = xout; yin = yout; zin = zout;
				if (R.dir2 > 0) {
					(*R.fn2)(xin,yin,zin,xout,yout,zout,1);
				} else {
					(*R.fn2)(xout,yout,zout,xin,yin,zin,-1);
				}
				if (Nfn >= 3) {
					xin = xout; yin = yout; zin = zout;
					if (R.dir3 > 0) {
						(*R.fn3)(xin,yin,zin,xout,yout,zout,1);
					} else {
						(*R.fn3)(xout,yout,zout,xin,yin,zin,-1);
					}
				}
			}
			X[i] = xout; Y[i] = yout; Z[i] = zout;
		}
	}
}

static Treal ComputePsi()
// Computes the dipole tilt angle psi (radians) of epoch currently Recalc'ed.
// sin(psi) = (xGSM.zSM)
{
	float xGSM,yGSM,zGSM, xSM,ySM,zSM;
	xSM = ySM = 0; zSM = 1;
	SMGSM(xSM,ySM,zSM, xGSM,yGSM,zGSM, 1);
	return asin(xGSM);
}

void Geopack_model(TFieldModel model, const float paramvec[11],
				   float x, float y, float z, Treal epoch, float psi, bool psi_given,
				   float& Bx, float& By, float& Bz)
{
	Recalc(epoch);
	switch (model) {
	case MODEL_T89:
	{
		/*
		C   IOPT= 1       2        3        4        5        6      7
                  CORRESPOND TO:
              KP= 0,0+  1-,1,1+  2-,2,2+  3-,3,3+  4-,4,4+  5-,5,5+  >= 6-
		*/
		const Treal Kp = *paramvec;
		int iopt;
		if (Kp <= 1.0/3.0) {
			iopt = 1;
		} else if (Kp <= 1+1.0/3.0) {
			iopt = 2;
		} else if (Kp <= 2+1.0/3.0) {
			iopt = 3;
		} else if (Kp <= 3+1.0/3.0) {
			iopt = 4;
		} else if (Kp <= 4+1.0/3.0) {
			iopt = 5;
		} else if (Kp <= 5+1.0/3.0) {
			iopt = 6;
		} else {
			iopt = 7;
		}
		float PSI;
		if (psi_given)
			PSI = psi;
		else
			PSI = ComputePsi();
		T89C(iopt,paramvec,PSI,x,y,z,Bx,By,Bz);
	}
	break;
	case MODEL_T96:
	case MODEL_T96_NOFAC:
	{
		const int iopt = 0;
		float PSI;
		if (psi_given)
			PSI = psi;
		else
			PSI = ComputePsi();
		const int incfac = model==MODEL_T96;
		T96_01(iopt,paramvec,PSI,x,y,z,Bx,By,Bz,incfac);
	}
	break;
	case MODEL_IGRF:
		IGRF_GSM(x,y,z,Bx,By,Bz);
		break;
	case MODEL_DIP:
		DIP(x,y,z,Bx,By,Bz);
		break;
	case MODEL_NONE:
		Bx = By = Bz = 0;
		break;
	}
}

#ifndef sqr
#  define sqr(x) ((x)*(x))
#endif

static void tracer(Treal /*t*/, const Treal r[3], Treal drdt[3])
{
	Treal Bx=0, By=0, Bz=0;
	float Bx1,By1,Bz1;
	Geopack_model(tracer_model1, tracer_paramvec,
				  r[0],r[1],r[2], tracer_epoch,tracer_psi,tracer_psi_given,
				  Bx1,By1,Bz1);
	Bx+= Bx1; By+= By1; Bz+= Bz1;
	Geopack_model(tracer_model2, tracer_paramvec,
				  r[0],r[1],r[2], tracer_epoch,tracer_psi,tracer_psi_given,
				  Bx1,By1,Bz1);
	Bx+= Bx1; By+= By1; Bz+= Bz1;
	Treal norm;
	drdt[0] = tracer_sign*Bx;
	drdt[1] = tracer_sign*By;
	drdt[2] = tracer_sign*Bz;
	norm = 1.0/sqrt(sqr(drdt[0]) + sqr(drdt[1]) + sqr(drdt[2]));
	drdt[0]*= norm;
	drdt[1]*= norm;
	drdt[2]*= norm;
}

static bool isOutsideDomain(const Treal r[3], const Treal rold[3], Treal rintersect[3])
{
	int i;
	bool retval = false;
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
		bool cond;
		if (tracer_endconds[i].gt) {
			cond = (v > tracer_endconds[i].val);
		} else {
			cond = (v < tracer_endconds[i].val);
		}
		if (!cond) {
			retval = true;
			Treal X1=0,X2=v;
			switch (tracer_endconds[i].var) {
			case ENDCOND_R:
				X1 = sqrt(sqr(rold[0]) + sqr(rold[1]) + sqr(rold[2]));
				break;
			case ENDCOND_X:
				X1 = rold[0];
				break;
			case ENDCOND_Y:
				X1 = rold[1];
				break;
			case ENDCOND_Z:
				X1 = rold[2];
				break;
			}
			const Treal t = (tracer_endconds[i].val - X1)/(X2-X1);
			rintersect[0] = (1-t)*rold[0] + t*r[0];
			rintersect[1] = (1-t)*rold[1] + t*r[1];
			rintersect[2] = (1-t)*rold[2] + t*r[2];
			break;
		}
	}
	return retval;
}

static float *x_sav=0, *y_sav=0, *z_sav=0;
static int count_sav=0;

static void RecordPoint(const Treal r[3])
{
	if (x_sav && y_sav && z_sav && count_sav < MAX_TRACE_POINTS) {
		x_sav[count_sav] = r[0];
		y_sav[count_sav] = r[1];
		z_sav[count_sav] = r[2];
		count_sav++;
	}
}

static void odeint_midpoint
	(Treal r[3],
	 Treal maxstep, int maxsteps,
	 void (*derivs)(Treal, const Treal[3], Treal[3]),
	 bool (*stopnow)(const Treal[3], const Treal[3], Treal[3]), bool& successflag)
{
	// ------------------------------------------
	const Treal nearEarth_relstep = 0.03;
	const Treal max_bhat_diff = 0.03;
	const Treal minstep_rel = 0.1;
	const Treal minstep_max = 1e-3;
	// ------------------------------------------
	int i;
	Treal s=0,step;
	Treal bhat0[3],bhat[3],rmid[3],rnew[3],rfinal[3];
	Treal minstep = minstep_rel*maxstep;
	if (minstep > minstep_max) minstep = minstep_max;
	for (i=0; i<maxsteps; i++) {
		RecordPoint(r);
		step = maxstep;
		const Treal R = sqrt(sqr(r[0]) + sqr(r[1]) + sqr(r[2]));
		if (step > nearEarth_relstep*R) step = nearEarth_relstep*R;
		while (1) {
			const Treal halfstep = 0.5*step;
			(*derivs)(s,r,bhat0);
			rmid[0] = r[0] + halfstep*bhat0[0];
			rmid[1] = r[1] + halfstep*bhat0[1];
			rmid[2] = r[2] + halfstep*bhat0[2];
			(*derivs)(s,rmid,bhat);
			if (sqr(bhat[0]-bhat0[0]) + sqr(bhat[1]-bhat0[1]) + sqr(bhat[2]-bhat0[2]) > sqr(max_bhat_diff)) {
				step*= 0.5;
				if (step < minstep) {step = minstep; break;}
			} else {
				break;
			}
		}
		rnew[0] = r[0] + step*bhat[0];
		rnew[1] = r[1] + step*bhat[1];
		rnew[2] = r[2] + step*bhat[2];
		if ((*stopnow)(rnew, r, rfinal)) {
			r[0] = rfinal[0];
			r[1] = rfinal[1];
			r[2] = rfinal[2];
			RecordPoint(r);
			break;
		}
		r[0] = rnew[0];
		r[1] = rnew[1];
		r[2] = rnew[2];
	}
	successflag = true;
	if (i>=maxsteps) {
//		cerr << "geopack_trace: more than " << maxsteps << " steps, returning early\n";
		successflag = false;
	}
}

void Geopack_trace(TFieldModel model1, TFieldModel model2,
				   const float paramvec[11],
				   int sign, Treal maxstep, int maxsteps,
				   const TTraceEndCondition endconds[9], int Nendconds,
				   Treal x0, Treal y0, Treal z0, Treal epoch, Treal psi, bool psi_given,
				   float x[MAX_TRACE_POINTS], float y[MAX_TRACE_POINTS], float z[MAX_TRACE_POINTS],
				   int& Npts, bool& successflag)
{
	tracer_model1 = model1;
	tracer_model2 = model2;
	tracer_epoch = epoch;
	int i;
	for (i=0; i<11; i++) tracer_paramvec[i] = paramvec[i];
	for (i=0; i<8; i++) tracer_endconds[i] = endconds[i];
	tracer_Nendconds = Nendconds;
	tracer_psi = psi;
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
	if (psi_given) {
		Recalc(epoch);
		SETPSI(psi);
		tracer_psi_given = psi_given;
	}
	odeint_midpoint(r,maxstep,maxsteps,&tracer,&isOutsideDomain,successflag);
	tracer_psi_given = false;
	if (count_sav <= 0) count_sav = 1;
	x[count_sav-1] = r[0];
	y[count_sav-1] = r[1];
	z[count_sav-1] = r[2];
	Npts = count_sav;
}

