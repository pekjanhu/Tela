#ifndef GEOPACKINTF_H

#ifdef __GNUC__
#  pragma interface
#endif

enum TCoordSys {COORDSYS_GSM, COORDSYS_GSE, COORDSYS_GEO, COORDSYS_GEI, COORDSYS_MAG, COORDSYS_SM, COORDSYS_NONE};

#ifdef MAXRANK
#  include "def.H"
#else
typedef double Treal;
#endif

extern void Geopack_coordtrans(TCoordSys fromsys, TCoordSys tosys,
							   const Treal x[], const Treal y[], const Treal z[],
							   const Treal epoch[],
							   int n,
							   Treal X[], Treal Y[], Treal Z[]);

enum TFieldModel {MODEL_T89, MODEL_T96, MODEL_T96_NOFAC, MODEL_IGRF, MODEL_DIP, MODEL_NONE};

extern void Geopack_model(TFieldModel model, const float paramvec[11],
						  float x, float y, float z, Treal epoch, float psi, bool psi_given,
						  float& Bx, float& By, float& Bz);

enum TTraceEndConditionVariable {ENDCOND_R, ENDCOND_X, ENDCOND_Y, ENDCOND_Z};

struct TTraceEndCondition {
	TTraceEndConditionVariable var;	// r,x,y or z
	bool gt;		// if true, the sign is ">", if false, it is "<"
	double val;		// value compared with 
};

#define MAX_TRACE_POINTS 2000

extern void Geopack_trace(TFieldModel model1, TFieldModel model2,
						  const float paramvec[11],
						  int sign, Treal maxstep, int maxsteps,
						  const TTraceEndCondition[9], int Nendconds,
						  Treal x0, Treal y0, Treal z0, Treal epoch, Treal psi, bool psi_given,
						  float x[MAX_TRACE_POINTS], float y[MAX_TRACE_POINTS], float z[MAX_TRACE_POINTS],
						  int& Npts, bool& successflag);

#define GEOPACKINTF_H 1

#endif
