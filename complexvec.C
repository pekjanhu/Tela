/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
  Basic arithmetic (Add,Sub,Mul,Div) operations for complex vectors
  and other data types (scalars and int, real, complex vectors).
  Also some logical etc. operations are included in this file.

  This file is included by objarithm.C, not compiled as a separate unit.

  This file is currently included only if VECTOR_MACHINE is defined.
*/

#define FOR1(i) for (Tint i=0; i<2*N; i+=2)
#define FOR2(i,j) for (Tint i=0,j=0; j<N; i+=2,j++)

/* Notation:
   AddCC - add two complex vectors, producing complex vector
   AddCR - add complex and real vector, and so on.
   AddCCs - add complex vector and complex scalar etc.

   First argument is always output argument.
   */

/*------------------------------------------------------------------------------
  1. Add/Sub functions producing complex vectors.

  CV + CV -> CV				AddCC
  CV + RV -> CV				AddCR
  CV + IV -> CV				AddCI
  CV + C  -> CV				AddCCs
  CV + R  -> CV				 -"-
  RV + C  -> CV				AddRCs
  IV + C  -> CV				AddICs

  We get 6 static functions and 26 inline functions.
  -------------------------------------------------------------------------------*/

static void AddCC(Treal c[], const Treal a[], const Treal b[], Tint N, Treal bsign=1)
{
	VECTORIZED FOR1(i) {
		c[i] = a[i] + bsign*b[i];
		c[i+1] = a[i+1] + bsign*b[i+1];
	}
}

inline void Add(Tcomplex c[], const Tcomplex a[], const Tcomplex b[], Tint N) {
	AddCC((Treal*)c,(const Treal*)a,(const Treal*)b,N);
}

inline void Sub(Tcomplex c[], const Tcomplex a[], const Tcomplex b[], Tint N) {
	AddCC((Treal*)c,(const Treal*)a,(const Treal*)b,N,-1);
}


static void AddCR(Treal c[], const Treal a[], const Treal b[], Tint N, Treal asign=1, Treal bsign=1)
{
	VECTORIZED FOR2(i,j) {
		c[i] = asign*a[i] + bsign*b[j];
		c[i+1] = asign*a[i+1];
	}
}

inline void Add(Tcomplex c[], const Tcomplex a[], const Treal b[], Tint N) {
	AddCR((Treal*)c,(const Treal*)a,b,N);
}

inline void Add(Tcomplex c[], const Treal a[], const Tcomplex b[], Tint N) {
	AddCR((Treal*)c,(const Treal*)b,a,N);
}

inline void Sub(Tcomplex c[], const Tcomplex a[], const Treal b[], Tint N) {
	AddCR((Treal*)c,(const Treal*)a,b,N, 1,-1);
}

inline void Sub(Tcomplex c[], const Treal a[], const Tcomplex b[], Tint N) {
	AddCR((Treal*)c,(const Treal*)b,a,N, -1,1);
}


static void AddCI(Treal c[], const Treal a[], const Tint b[], Tint N, Treal asign=1, Treal bsign=1) {
	VECTORIZED FOR2(i,j) {
		c[i] = asign*a[i] + bsign*b[j];
		c[i+1] = asign*a[i+1];
	}
}

inline void Add(Tcomplex c[], const Tcomplex a[], const Tint b[], Tint N) {
	AddCI((Treal*)c,(const Treal*)a,b,N);
}

inline void Add(Tcomplex c[], const Tint a[], const Tcomplex b[], Tint N) {
	AddCI((Treal*)c,(const Treal*)b,a,N);
}

inline void Sub(Tcomplex c[], const Tcomplex a[], const Tint b[], Tint N) {
	AddCI((Treal*)c,(const Treal*)a,b,N,1,-1);
}

inline void Sub(Tcomplex c[], const Tint a[], const Tcomplex b[], Tint N) {
	AddCI((Treal*)c,(const Treal*)b,a,N,-1,1);
}

inline void Neg(Tcomplex c[], Tint N) {
	Treal *cr = (Treal*)c;
	VECTORIZED FOR1(i) {
		cr[i] = -cr[i];
		cr[i+1] = -cr[i+1];
	}
}

static void AddCCs(Treal c[], const Treal a[], Tcomplex b, Tint N, Treal bsign=1) {
	const Treal br = bsign*b.real();
	const Treal bi = bsign*b.imag();
	VECTORIZED FOR1(i) {
		c[i] = a[i] + br;
		c[i+1] = a[i+1] + bi;
	}
}

inline void Add(Tcomplex c[], const Tcomplex a[], Tcomplex b, Tint N) {
	AddCCs((Treal*)c,(const Treal*)a,b,N);
}

inline void Add(Tcomplex c[], Tcomplex a, const Tcomplex b[], Tint N) {
	AddCCs((Treal*)c,(const Treal*)b,a,N);
}

inline void Add(Tcomplex c[], const Tcomplex a[], Treal b, Tint N) {Add(c,a,Tcomplex(b),N);}
inline void Add(Tcomplex c[], Treal a, const Tcomplex b[], Tint N) {Add(c,Tcomplex(a),b,N);}

inline void Sub(Tcomplex c[], const Tcomplex a[], Tcomplex b, Tint N) {
	AddCCs((Treal*)c,(const Treal*)a,b,N,-1);
}

inline void Sub(Tcomplex c[], Tcomplex a, const Tcomplex b[], Tint N) {
	Sub(c,b,a,N);
	Neg(c,N);
}

inline void Sub(Tcomplex c[], const Tcomplex a[], Treal b, Tint N) {Sub(c,a,Tcomplex(b),N);}
inline void Sub(Tcomplex c[], Treal a, const Tcomplex b[], Tint N) {Sub(c,Tcomplex(a),b,N);}


static void AddRCs(Treal c[], const Treal a[], Tcomplex b, Tint N, Treal asign=1)
{
	const Treal br = b.real();
	const Treal bi = b.imag();
	VECTORIZED FOR2(i,j) {
		c[i] = asign*a[j] + br;
		c[i+1] = bi;
	}
}

inline void Add(Tcomplex c[], const Treal a[], Tcomplex b, Tint N) {
	AddRCs((Treal*)c,a,b,N);
}

inline void Add(Tcomplex c[], Tcomplex a, const Treal b[], Tint N) {Add(c,b,a,N);}

inline void Sub(Tcomplex c[], const Treal a[], Tcomplex b, Tint N) {
	AddRCs((Treal*)c,a,-b,N);
}

inline void Sub(Tcomplex c[], Tcomplex a, const Treal b[], Tint N) {
	AddRCs((Treal*)c,b,a,N,-1);
}


static void AddICs(Treal c[], const Tint a[], Tcomplex b, Tint N, Treal asign=1)
{
	const Treal br = b.real();
	const Treal bi = b.imag();
	VECTORIZED FOR2(i,j) {
		c[i] = asign*a[j] + br;
		c[i+1] = bi;
	}
}

inline void Add(Tcomplex c[], const Tint a[], Tcomplex b, Tint N) {
	AddICs((Treal*)c,a,b,N);
}

inline void Add(Tcomplex c[], Tcomplex a, const Tint b[], Tint N) {Add(c,b,a,N);}

inline void Sub(Tcomplex c[], const Tint a[], Tcomplex b, Tint N) {
	AddICs((Treal*)c,a,-b,N);
}

inline void Sub(Tcomplex c[], Tcomplex a, const Tint b[], Tint N) {
	AddICs((Treal*)c,b,a,N,-1);
}







/*------------------------------------------------------------------------------
  2. Mul functions producing complex vectors.

  CV * CV -> CV				MulCC
  CV * RV -> CV				MulCR
  CV * IV -> CV				MulCI
  CV * C  -> CV				MulCCs
  CV * R  -> CV				AddCR(asign=b,bsign=0)
  RV * C  -> CV				MulRCs
  IV * C  -> CV				MulICs

  We get 6 new static functions and 13 new inline functions.
  -------------------------------------------------------------------------------*/

static void MulCC(Treal c[], const Treal a[], const Treal b[], Tint N)
{
	Treal ci;			// c may be the same vector as a or b !
	VECTORIZED FOR1(i) {
		ci = a[i]*b[i] - a[i+1]*b[i+1];
		c[i+1] = a[i]*b[i+1] + a[i+1]*b[i];
		c[i] = ci;
	}
}

inline void Mul(Tcomplex c[], const Tcomplex a[], const Tcomplex b[], Tint N) {
	MulCC((Treal*)c,(const Treal*)a,(const Treal*)b,N);
}

static void MulCR(Treal c[], const Treal a[], const Treal b[], Tint N)
{
	VECTORIZED FOR2(i,j) {
		c[i] = a[i]*b[j];
		c[i+1] = a[i+1]*b[j];
	}
}

inline void Mul(Tcomplex c[], const Tcomplex a[], const Treal b[], Tint N) {
	MulCR((Treal*)c,(const Treal*)a,b,N);
}

inline void Mul(Tcomplex c[], const Treal a[], const Tcomplex b[], Tint N) {Mul(c,b,a,N);}


static void MulCI(Treal c[], const Treal a[], const Tint b[], Tint N)
{
	VECTORIZED FOR2(i,j) {
		c[i] = a[i]*b[j];
		c[i+1] = a[i+1]*b[j];
	}
}

inline void Mul(Tcomplex c[], const Tcomplex a[], const Tint b[], Tint N) {
	MulCI((Treal*)c,(const Treal*)a,b,N);
}

inline void Mul(Tcomplex c[], const Tint a[], const Tcomplex b[], Tint N) {Mul(c,b,a,N);}


static void MulCCs(Treal c[], const Treal a[], Tcomplex b, Tint N)
{
	Treal ci;			// c and a may be the same vector!
	const Treal br = b.real();
	const Treal bi = b.imag();
	VECTORIZED FOR1(i) {
		ci = a[i]*br - a[i+1]*bi;
		c[i+1] = a[i]*bi + a[i+1]*br;
		c[i] = ci;
	}
}

inline void Mul(Tcomplex c[], const Tcomplex a[], Tcomplex b, Tint N) {
	MulCCs((Treal*)c,(const Treal*)a,b,N);
}

inline void Mul(Tcomplex c[], Tcomplex a, const Tcomplex b[], Tint N) {Mul(c,b,a,N);}

inline void Mul(Tcomplex c[], const Tcomplex a[], Treal b, Tint N) {
	AddCR((Treal*)c,(const Treal*)a,(const Treal*)a,N,b,0);
}

inline void Mul(Tcomplex c[], Treal a, const Tcomplex b[], Tint N) {Mul(c,b,a,N);}


static void MulRCs(Treal c[], const Treal a[], Tcomplex b, Tint N)
{
	const Treal br = b.real();
	const Treal bi = b.imag();
	VECTORIZED FOR2(i,j) {
		c[i] = a[j]*br;
		c[i+1] = a[j]*bi;
	}
}

inline void Mul(Tcomplex c[], const Treal a[], Tcomplex b, Tint N) {
	MulRCs((Treal*)c,a,b,N);
}

inline void Mul(Tcomplex c[], Tcomplex a, const Treal b[], Tint N) {Mul(c,b,a,N);}


static void MulICs(Treal c[], const Tint a[], Tcomplex b, Tint N)
{
	const Treal br = b.real();
	const Treal bi = b.imag();
	VECTORIZED FOR2(i,j) {
		c[i] = a[j]*br;
		c[i+1] = a[j]*bi;
	}
}

inline void Mul(Tcomplex c[], const Tint a[], Tcomplex b, Tint N) {
	MulICs((Treal*)c,a,b,N);
}

inline void Mul(Tcomplex c[], Tcomplex a, const Tint b[], Tint N) {Mul(c,b,a,N);}









/*------------------------------------------------------------------------------
  3. Inv and Div functions producing complex vectors.

  CV / CV -> CV
  CV / RV -> CV
  RV / CV -> CV
  CV / IV -> CV
  IV / CV -> CV
  CV / C  -> CV
  C / CV  -> CV
  CV / R  -> CV
  R / CV  -> CV
  RV / C  -> CV
  C / RV  -> CV
  IV / C  -> CV
  C / IV  -> CV

  We get 5 new static functions and 13 new inline functions.
  -------------------------------------------------------------------------------*/

static void InvC(Treal c[], const Treal a[], Tint N)
{
	Treal C;
	VECTORIZED FOR1(i) {
		C = 1/(a[i]*a[i] + a[i+1]*a[i+1]);
		c[i] = C*a[i];
		c[i+1]= -C*a[i+1];
	}
}

inline void Inv(Tcomplex c[], const Tcomplex a[], int N) {InvC((Treal*)c,(const Treal*)a,N);}

inline void Div(Tcomplex c[], const Tcomplex a[], const Tcomplex b[], Tint N) {
	Inv(c,b,N);
	Mul(c,c,a,N);
}

static void DivCR(Treal c[], const Treal a[], const Treal b[], Tint N)
{
	VECTORIZED FOR2(i,j) {
		c[i] = a[i]/b[j];
		c[i+1] = a[i+1]/b[j];
	}
}

inline void Div(Tcomplex c[], const Tcomplex a[], const Treal b[], Tint N) {
	DivCR((Treal*)c,(const Treal*)a,b,N);
}

inline void Div(Tcomplex c[], const Treal a[], const Tcomplex b[], Tint N) {
	Inv(c,b,N);
	Mul(c,c,a,N);
}

static void DivCI(Treal c[], const Treal a[], const Tint b[], Tint N)
{
	VECTORIZED FOR2(i,j) {
		c[i] = a[i]/b[j];
		c[i+1] = a[i+1]/b[j];
	}
}

inline void Div(Tcomplex c[], const Tcomplex a[], const Tint b[], Tint N) {
	DivCI((Treal*)c,(const Treal*)a,b,N);
}

inline void Div(Tcomplex c[], const Tint a[], const Tcomplex b[], Tint N) {
	Inv(c,b,N);
	Mul(c,c,a,N);
}

inline void Div(Tcomplex c[], const Tcomplex a[], Tcomplex b, Tint N) {
	const Tcomplex invb = 1/b;
	Mul(c,a,invb,N);
}

inline void Div(Tcomplex c[], Tcomplex a, const Tcomplex b[], Tint N) {
	Inv(c,b,N);
	Mul(c,c,a,N);
}

inline void Div(Tcomplex c[], const Tcomplex a[], Treal b, Tint N) {Mul(c,a,1/b,N);}

inline void Div(Tcomplex c[], Treal a, const Tcomplex b[], Tint N) {
	Inv(c,b,N);
	Mul(c,c,a,N);
}

inline void Div(Tcomplex c[], const Treal a[], Tcomplex b, Tint N) {Mul(c,a,1/b,N);}

static void DivCsR(Treal c[], Tcomplex a, const Treal b[], Tint N)
{
	const Treal ar = a.real();
	const Treal ai = a.imag();
	VECTORIZED FOR2(i,j) {
		c[i] = ar/b[j];
		c[i+1] = ai/b[j];
	}
}

inline void Div(Tcomplex c[], Tcomplex a, const Treal b[], Tint N) {
	DivCsR((Treal*)c,a,b,N);
}

inline void Div(Tcomplex c[], const Tint a[], Tcomplex b, Tint N) {Mul(c,a,1/b,N);}

static void DivCsI(Treal c[], Tcomplex a, const Tint b[], Tint N)
{
	const Treal ar = a.real();
	const Treal ai = a.imag();
	VECTORIZED FOR2(i,j) {
		c[i] = ar/b[j];
		c[i+1] = ai/b[j];
	}
}

inline void Div(Tcomplex c[], Tcomplex a, const Tint b[], Tint N) {
	DivCsI((Treal*)c,a,b,N);
}






/*------------------------------------------------------------------------------
  4. Some Eq, Ne stuff
  ------------------------------------------------------------------------------*/

inline void Eq(Tint c[], const Tint a[], const Tcomplex b[], Tint N) {
	const Treal *br = (const Treal*)b;
	VECTORIZED FOR2(i,j)
		c[j] = (a[j] == br[i]) && (0 == br[i+1]);
}
	
inline void Ne(Tint c[], const Tint a[], const Tcomplex b[], Tint N) {
	const Treal *br = (const Treal*)b;
	VECTORIZED FOR2(i,j)
		c[j] = (a[j] != br[i]) || (0 != br[i+1]);
}

static void Eq(Tint c[], const Treal a[], Tcomplex b, Tint N)
{
	if (b.imag()!=0) {
		VECTORIZED FOR2(i,j) c[j] = 0;
	} else {
		const Treal br = b.real();
		VECTORIZED FOR2(i,j)
			c[j] = (a[j] == br);
	}
}

static void Ne(Tint c[], const Treal a[], Tcomplex b, Tint N)
{
	if (b.imag()!=0) {
		VECTORIZED FOR2(i,j) c[j] = 1;
	} else {
		const Treal br = b.real();
		VECTORIZED FOR2(i,j)
			c[j] = (a[j] != br);
	}
}




/*------------------------------------------------------------------------------
  5. Neg for complex vectors
  ------------------------------------------------------------------------------*/

static void Neg(Tcomplex c[], const Tcomplex a[], Tint N)
{
	Treal *cr = (Treal *)c;
	const Treal *ar = (const Treal *)a;
	VECTORIZED for (Tint i=0; i<2*N; i++)
		c[i] = -a[i];
}




/*------------------------------------------------------------------------------
  6. Inc,Dec
  ------------------------------------------------------------------------------*/

inline void Inc(Tint a[], Tint N)
{
	VECTORIZED for (Tint i=0; i<N; i++) a[i]++;
}

inline void Dec(Tint a[], Tint N)
{
	VECTORIZED for (Tint i=0; i<N; i++) a[i]--;
}

inline void Inc(Treal a[], Tint N)
{
	VECTORIZED for (Tint i=0; i<N; i++) a[i]+= 1.0;
}

inline void Dec(Treal a[], Tint N)
{
	VECTORIZED for (Tint i=0; i<N; i++) a[i]-= 1.0;
}

inline void Inc(Tcomplex a[], Tint N)
{
	Treal *ar = (Treal *)a;
	VECTORIZED FOR1(i) a[i]+= 1.0;
}

inline void Dec(Tcomplex a[], Tint N)
{
	Treal *ar = (Treal *)a;
	VECTORIZED FOR1(i) a[i]-= 1.0;
}

#undef FOR1
#undef FOR2
