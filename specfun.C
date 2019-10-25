/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "specfun.H"
#endif

#include "specfun.H"
#include "ode.H"

#define sqr(x) ((x)*(x))

inline Treal abs2(Tcomplex z) {return sqr(z.real()) + sqr(z.imag());}

// Notice that using abs() for overloaded functions for real values produces wrong results,
// because abs() comes from traditional C and operates only on integers. Applying abs() to real
// value silently truncates it.
// Thus we always use Abs() in real/complex template functions below.
inline Treal Abs(Treal x) {return fabs(x);}
inline Treal Abs(const Tcomplex& z) {return abs(z);}

ostream& operator<<(ostream& o, const Tcomplex& z)
{
	o << z.real();
	if (z.imag() >= 0) o << '+'; else o << '-';
	o << fabs(z.imag()) << 'i';
	return o;
}

/*
  TO DO: Find more accurate Gamma coefficients
*/

inline Treal Re(const Tcomplex& x) {return x.real();}
inline Treal Re(Treal x) {return x;}
inline Treal Im(const Tcomplex& x) {return x.imag();}
inline Treal Im(Treal x) {return 0;}

#define pi M_PI
#define epsilon default_accuracy		/* general accuracy, used by various functions */
// Setting epsilon smaller than 1e-11 may cause "no convergence" warnings from IncompleteGamma_contfract.

/* -----------------------------------------------------------------
   The Euler Gamma function and associates
   ----------------------------------------------------------------- */

/*
   Our primary function is logfactPositive, which evaluates log(z!) for Re(z)>=0.
   The relative error (in some meaningful sense) is less than 1E-10 everywhere.
   References:
   The Numerical Recipes book by Press et al. (1986)
   Lanczos, C., J. SIAM Num. Anal. B, 1, 86 (1964)
*/

// Coefficients for the rational approximation used in logfactPositive
static Treal logfactPositive_c[6] = {
	76.18009173,-86.50532033,24.01409822,
	-1.231739516,0.120858003e-2,-0.536382e-5};

template <class T>
static T logfactPositive(const T& z)
// Returns log(z!) for Re(z) >= 0. Applies componentwise on arrays.
{
	T z1 = z + 5.5;
	T series,x;
	int i;
	for (series=1,x=z+1,i=0; i<6; i++,x+=1) series+= logfactPositive_c[i]/x;
	return (z+0.5)*log(z1) - z1 + log(2.50662827465*series);	// the constant here is = sqrt(2*pi)
}

template Treal logfactPositive(const Treal&);
template Tcomplex logfactPositive(const Tcomplex&);

/*
   The factorial function reflection formula:
       z! = pi*z/((-z)!*sin(pi*z))

    Logarithmic form:
	   log(z!) = log(pi*z/sin(pi*z)) - log((-z)!)

    These formulas are used to compute z! for Re(z) < 0.
*/

template <class T>
Tcomplex logfact(const T& z)
// logfact(z) returns log(z!) for any numerical z.
{
	if (Re(z) < 0) {
		return log(Tcomplex(pi*z/sin(pi*z))) - logfactPositive(-z);
	} else
		return logfactPositive(z);
}

template Tcomplex logfact(const Treal&);
template Tcomplex logfact(const Tcomplex&);

/* ----------------------------------------------------------------------------------
   The incomplete Gamma functions IncompleteGammaP, IncompleteGammaQ, IncompleteGamma
   ---------------------------------------------------------------------------------- */

template <class T>
static Tcomplex IncompleteGamma_series(const T& a, const T& x)
/* Returns incomplete gamma function P(a,x) evaluated by its
   series representation (reference: Numerical Recipes).*/
{
	T ap = a;
	T del = 1/a;
	T summe = 1/a;
	int itmax = 200+10*int(ceil(sqrt(Abs(a))));
	if (itmax > 2000) itmax = 2000;
	int n;
	for (n=0; n<itmax; n++) {
		ap+= 1;
		del*= x/ap;
		summe+= del;
//		cout << "- n=" << n << ", ap=" << ap << ", del=" << del << ", summe=" << summe << "\n";
//		cout << "  Abs(del)=" << Abs(del) << ", Abs(summe)=" << Abs(summe) << "\n";
		if (Abs(del) <= epsilon*Abs(summe)) break;
	}
	if (n>=itmax) {
		cerr << "warning: too many iterations in IncompleteGamma_series, maxits=" << itmax << "\n";
	}
	const Tcomplex result = (Re(x)==0 && Im(x)==0) ? 0.0 : summe*exp(-x+a*log(Tcomplex(x))-logfact(a-1));
//	cout << "IncompleteGamma_series(" << a << "," << x << ") = " << result << ", n=" << n << "\n";
	return result;
}

template Tcomplex IncompleteGamma_series(const Treal&, const Treal&);
template Tcomplex IncompleteGamma_series(const Tcomplex&, const Tcomplex&);

template <class T>
inline T avoid_origin(const T& z, Treal tiny)
/* avoid_origin(z,tiny) returns z if |z| is not smaller than tiny.
   If it is, it returns tiny. The second argument tiny must be a positive
   real number.*/
{
	if (Abs(z) < tiny) return tiny; else return z;
}

template <class T>
static Tcomplex IncompleteGamma_contfrac(const T& a, const T& x)
/* Returns Q(a,x):= 1-P(a,x) evaluated by its continued fraction
   representation (reference: Numerical Recipes). */
{
	T b = x + 1 - a;
	const Treal fpmin = 1e-30;
	T c = 1/fpmin;
	T d = 1/b;
	T h = d;
	int itmax = 200+10*int(ceil(sqrt(Abs(a))));
	if (itmax > 2000) itmax = 2000;
	int i;
	for (i=0; i<itmax; i++) {
		T an = -(i+1)*(i+1-a);
		b+= 2;
		d = avoid_origin(an*d + b,fpmin);
		c = avoid_origin(b + an/c,fpmin);
		d = 1/d;
		T del = d*c;
		h*= del;
		if (Abs(del-1) < epsilon) break;
	};
	if (i >= itmax) {
		cerr << "warning: no convergence in IncompleteGamma_contfrac\n";
	}
	return exp(-x + a*log(Tcomplex(x)) - logfact(a-1))*h;
}

template Tcomplex IncompleteGamma_contfrac(const Treal&, const Treal&);
template Tcomplex IncompleteGamma_contfrac(const Tcomplex&, const Tcomplex&);

template <class T>
Tcomplex IncompleteGammaP(const T& a, const T& x)
/* IncompleteGammaP(a,x) is the regularized incomplete gamma function
   IncompleteGammaP(a,x) := integrate(exp(-t)*t^(a-1),t=0..x)/Gamma(a).
   The evaluation is reliable only for Re(a) > 0.
   This notation conforms with Numerical Recipes.*/
{
	if (Re(x) < Re(a)+1) {
		return IncompleteGamma_series(a,x);
	} else {
		return 1 - IncompleteGamma_contfrac(a,x);
	}
}

template Tcomplex IncompleteGammaP(const Treal&, const Treal&);
template Tcomplex IncompleteGammaP(const Tcomplex&, const Tcomplex&);

template <class T>
Tcomplex IncompleteGammaQ(const T& a, const T& x)
/* IncompleteGammaQ(a,x) is the complement regularized incomplete gamma function
   IncompleteGammaQ(a,x) := 1 - IncompleteGammaP(a,x).
   The evaluation is reliable only for Re(a) > 0.
   This notation conforms wich Numerical Recipes.
   IncompleteGammaQ(a,x) is the same as GammaRegularized[a,x]
   in Mathematica.*/
{
	Tcomplex result;
	if (Re(x) < Re(a)+1) {
//		cout << "using series\n";
		result = 1 - IncompleteGamma_series(a,x);
	} else {
		result = IncompleteGamma_contfrac(a,x);
	}
//	cout << "IncompleteGammaQ(" << a << "," << x << ") = " << result << "\n";
	return result;
}

template Tcomplex IncompleteGammaQ(const Treal&, const Treal&);
template Tcomplex IncompleteGammaQ(const Tcomplex&, const Tcomplex&);

/* ----------------------------------------------------------------------------
   Kummer's confluent hypergeometric function 1F1(a,c,z) (also called M(a,c,z))
   ---------------------------------------------------------------------------- */

static Tcomplex aa,bb,cc,z0,dz;		// communicates with driver functions
// (bb is not used in 1F1(a,c,z), only in 2F1(a,b,c,z) which is further below)

static Tcomplex Kummer1F1_series(const Tcomplex& a, const Tcomplex& c, const Tcomplex& z, Tcomplex& deriv, int maxN=-1)
// series computing 1F1 (Kummer's confluent hypergeometric function)
// 1F1(a,c,z) = sum(poch(a,n)*z^2/(n!*poch(c,n)),n=0..Inf)
// the parameter deriv returns the complex derivative of 1F1
// if maxN is >= 0, only up to maxN terms are calculated
{
	int n;
	Tcomplex aa,cc,fac,temp,series;
	deriv = Tcomplex(0.0,0.0);
	fac = Tcomplex(1.0,0.0);
	temp = series = fac;
	aa = a;
	cc = c;
	const int nmax = (maxN >= 0) ? min(1000,maxN) : 1000;
	for (n=1; n<=nmax; n++) {
		fac*= aa/cc;
		deriv+= fac;
		fac*= (1.0/n)*z;
		series = temp + fac;
		if (series == temp) return series;
		temp = series;
		aa+= 1;
		cc+= 1;
	}
	if (maxN < 0) cerr << "warning: convergence failure in Kummer1F1_series(" << a << "," << c << "," << z << ")\n";
	return series;
}

static Tcomplex direct_pochhammer(Tcomplex z, int n)
{
	Tcomplex result = 1;
	for (int i=0; i<n; i++) result*= z+i;
	return result;
}

static Treal direct_fact(int n)
{
	Treal result = 1;
	for (int i=1; i<=n; i++) result*= i;
	return result;
}

static Tcomplex Kummer1F1Regularized_series(const Tcomplex& a, const Tcomplex& c, const Tcomplex& z, Tcomplex& deriv)
// series computing 1F1(a,c,z)/Gamma(c)
{
	int n;
	const int N = int(floor(1.5-c.real()));
	const Tcomplex d = c + (N-1);
	if (N <= 0 || abs2(d) > 0.1) {
		Tcomplex ans = Kummer1F1_series(a,c,z,deriv);
		const Tcomplex invgam = exp(-logfact(c-1));
		ans*= invgam;
		deriv*= invgam;
		return ans;
	} else {
		Tcomplex aa,cc,fac,temp,series;
		deriv = Tcomplex(0.0,0.0);
		fac = exp(-logfact(d));
		temp = fac;
		aa = a+N;
		cc = N+1;
		for (n=1; n<=1000; n++) {
			fac*= aa/cc;
			deriv+= fac;
			fac*= (1.0/(n+d))*z;
			series = temp + fac;
			if (series == temp) break;
			temp = series;
			aa+= 1;
			cc+= 1;
		}
		if (n >= 1000)
			cerr << "warning: convergence failure in Kummer1F1Regularized_series(" << a << "," << c << "," << z << ")\n";
		const Tcomplex aN = direct_pochhammer(a,N);
		const Treal invfactN = 1.0/direct_fact(N);
		const Tcomplex prefactor = aN*pow(z,N-1)*invfactN;
		const Tcomplex factor = z*prefactor;
		deriv = deriv*factor + N*prefactor*series;
		series*= factor;
		// now series and deriv contain the infinite sum part
		Tcomplex derivF;
		Tcomplex seriesF = Kummer1F1_series(a,c,z,derivF,N-1);
		const Tcomplex invGammac = exp(-logfact(c-1));
		seriesF*= invGammac;
		derivF*= invGammac;
		deriv+= derivF;
		series+= seriesF;
		return series;
	}
}

#if 0
static Tcomplex Kummer1F1_asymptotic(const Tcomplex& a, const Tcomplex& c,
									 const Tcomplex& z,
									 Treal eps_accuracy)
{
	const Treal epsilon2 = sqr(eps_accuracy);		// epsilon squared
	Tcomplex a1,z1,C;
	if (Re(z) >= 0) {
		a1 = a;
		z1 = z;
		C = exp(z1);
	} else {
		a1 = c - a;
		z1 = -z;
		C = 1.0;
	}
	Tcomplex S = 1.0;
	Tcomplex term = 1.0;
	Tcomplex invz = 1/z1;
	int n;
	for (n=1; n<30; n++) {
		const Tcomplex coeff = (n-a1)*(c-a1+n-1)*invz/n;
		if (abs2(coeff) > 1) {
			cerr << "warning: Kummer1F1_asymptotic problem\n";
			break;
		}
		term*= coeff;
		if (abs2(term) < epsilon2*abs2(S)) break;
		S+= term;
	};
	return exp(logfact(c-1)-logfact(a1-1))*C*S*pow(z1,a1-c);
}
#endif

static void Kummer1F1_odedriver(Treal s, Treal yy[], Treal dyyds[])
// same ODEdriver goes for both 1F1(a,c,z) and 1F1Regularized(a,c,z) since they satisfy the same diff.eqn.
{
	Tcomplex z,y[2],dyds[2];
	y[0] = Tcomplex(yy[1],yy[2]);
	y[1] = Tcomplex(yy[3],yy[4]);
	z = z0 + s*dz;
	dyds[0] = y[1]*dz;
	dyds[1] = ((z-cc)*y[1] + aa*y[0])*(dz/z);
	dyyds[1] = dyds[0].real();
	dyyds[2] = dyds[0].imag();
	dyyds[3] = dyds[1].real();
	dyyds[4] = dyds[1].imag();
}

static Tcomplex Kummer1F1_odeint(const Tcomplex& a, const Tcomplex& c, const Tcomplex& z, bool regulflag, Treal eps_accuracy)
// Complex hypergeometric function 1F1(a,c,z) by integration of diff.equation in complex plane.
// If regulflag is true, computes the regularized function, i.e. 1F1(a,c,z)/Gamma(c),
// otherwise computes 1F1(a,c,z).
// Last parameter is the fractional accuracy wanted.
{
	int nbad,nok;
	Tcomplex y[2];
	Treal yy[5];
	if (z.real() < 0) {
		z0 = Tcomplex(-1.0,0.0);
	} else {
		z0 = Tcomplex(1.0,0.0);
	}
	aa = a;
	cc = c;
	dz = z - z0;
	if (regulflag) {
		y[0] = Kummer1F1Regularized_series(aa,cc,z0,y[1]);	// get starting function and derivative
	} else {
		y[0] = Kummer1F1_series(aa,cc,z0,y[1]);	// get starting function and derivative
	}
	yy[1] = y[0].real();
	yy[2] = y[0].imag();
	yy[3] = y[1].real();
	yy[4] = y[1].imag();
	odeint(yy,4,0.0,1.0,eps_accuracy,0.1,0.0001,nok,nbad,&Kummer1F1_odedriver,&bsstep);
	y[0] = Tcomplex(yy[1],yy[2]);
	return y[0];
}

Tcomplex Kummer1F1(const Tcomplex& a, const Tcomplex& c,
				   const Tcomplex& z,
				   Treal eps_accuracy)
/* Algorithm:
   For |z| < series_limit computes using the power series, currently
     series_limit=10.
   For |z| outside series limit uses the asymptotic (1/z) expansion
     if |Im z| <= |Re z|,
   otherwise integrate the defining diff.equation. */
{
	const Treal series_limit = 2.0;
	if (abs2(z) < sqr(series_limit)) {
		Tcomplex deriv;
		return Kummer1F1_series(a,c,z,deriv);
//	} else if (fabs(z.imag()) <= fabs(z.real())) {
//		return Kummer1F1_asymptotic(a,c,z,eps_accuracy);
	} else {
		return Kummer1F1_odeint(a,c,z,false,eps_accuracy);
	}
}

Tcomplex Kummer1F1Regularized(const Tcomplex& a, const Tcomplex& c,
							  const Tcomplex& z,
							  Treal eps_accuracy)
/* Algorithm:
   For |z| < series_limit computes using the power series, currently
     series_limit=10.
   For |z| outside series limit uses the asymptotic (1/z) expansion
     if |Im z| <= |Re z|,
   otherwise integrate the defining diff.equation. */
{
	const Treal series_limit = 2.0;
	if (abs2(z) < sqr(series_limit)) {
		Tcomplex deriv;
		return Kummer1F1Regularized_series(a,c,z,deriv);
	} else {
		return Kummer1F1_odeint(a,c,z,true,eps_accuracy);
	}
}

/* -------------------------------------------
   Gauss' hypergeometric function 2F1(a,b,c,z)
   ------------------------------------------- */

static Tcomplex Gauss2F1_series(const Tcomplex& a, const Tcomplex& b, const Tcomplex& c,
								const Tcomplex& z,
								Tcomplex& deriv)
// returns hypergeometric series 2F1 and its derivative, iterating to machine accuracy.
// for |z| <= 0.5 convergence is quite rapid
{
	int n;
	Tcomplex aa,bb,cc,fac,temp,series;
	deriv = Tcomplex(0.0,0.0);
	fac = Tcomplex(1.0,0.0);
	temp = fac;
	aa = a;
	bb = b;
	cc = c;
	for (n=1; n<=1000; n++) {
		fac*= (aa*bb)/cc;
		deriv+= fac;
		fac*= (1.0/n)*z;
		series = temp + fac;
		if (series == temp) return series;
		temp = series;
		aa+= 1;
		bb+= 1;
		cc+= 1;
	}
	cerr << "warning: convergence failure in Gauss2F1_series(" << a << "," << b << "," << c << "," << z << ")\n";
	return 0.0;
}

static Tcomplex Gauss2F1_transf(const Tcomplex& a, const Tcomplex& b, const Tcomplex& c,
								const Tcomplex& z)
{
	const Tcomplex logGamma_c = logfact(c-1);
	Tcomplex deriv;
	const Tcomplex invz = 1/z;
	return exp( logGamma_c + logfact(b-a-1) - logfact(b-1) - logfact(c-a-1) ) *
		pow(-invz,a) *
		Gauss2F1_series(a,a+1-c,a+1-b,invz,deriv) +
		exp( logGamma_c + logfact(a-b-1) - logfact(a-1) - logfact(c-b-1) ) *
		pow(-invz,b) *
		Gauss2F1_series(b,b+1-c,b+1-a,invz,deriv);
}

static void Gauss2F1_odedriver(Treal s, Treal yy[], Treal dyyds[])
{
	Tcomplex z,y[2],dyds[2];
	y[0] = Tcomplex(yy[1],yy[2]);
	y[1] = Tcomplex(yy[3],yy[4]);
	z = z0 + s*dz;
	dyds[0] = y[1]*dz;
	dyds[1] = (aa*bb*y[0] - (cc-(aa+bb+1)*z)*y[1])*(dz/(z*(1-z)));
	dyyds[1] = dyds[0].real();
	dyyds[2] = dyds[0].imag();
	dyyds[3] = dyds[1].real();
	dyyds[4] = dyds[1].imag();
}

static Tcomplex Gauss2F1_odeint(const Tcomplex& a, const Tcomplex& b, const Tcomplex& c, const Tcomplex& z, Treal eps_accuracy)
// Complex hypergeometric function 2F1(a,b;c;z) by integration of diff.equation in complex plane.
// Branch cut taken to lie along real axis, Re(z) > 1. Last parameter is the fractional accuracy wanted.
{
	int nbad,nok;
	Tcomplex y[2];
	Treal yy[5];
	if (z.real() < 0) {
		z0 = Tcomplex(-0.5,0.0);
	} else if (z.real() <= 1.0) {
		z0 = Tcomplex(0.5,0.0);
	} else {
		z0 = Tcomplex(0.0, z.imag() >= 0 ? 0.5 : -0.5);
	}
	aa = a;
	bb = b;
	cc = c;
	dz = z - z0;
	y[0] = Gauss2F1_series(aa,bb,cc,z0,y[1]);	// get starting function and derivative
	yy[1] = y[0].real();
	yy[2] = y[0].imag();
	yy[3] = y[1].real();
	yy[4] = y[1].imag();
	odeint(yy,4,0.0,1.0,eps_accuracy,0.1,0.0001,nok,nbad,&Gauss2F1_odedriver,&bsstep);
	y[0] = Tcomplex(yy[1],yy[2]);
	return y[0];
}

inline bool NearNonnegativeInt(Tcomplex z, Treal eps)
{
	if (fabs(z.imag()) > eps) {
		return false;
	} else {
		const int i = int(floor(z.real()+0.5));
		return (i <= 0) && abs2(z-i) < sqr(eps);
	}
}

Tcomplex Gauss2F1(const Tcomplex& a, const Tcomplex& b, const Tcomplex& c,
				  const Tcomplex& z,
				  Treal eps_accuracy)
{
	if (abs2(z) <= 0.25) {
		Tcomplex deriv;
		return Gauss2F1_series(a,b,c,z,deriv);
	} else if (abs2(z) >= 4.0 && !NearNonnegativeInt(a+1-b,0.1) && !NearNonnegativeInt(b+1-a,0.1)) {
		return Gauss2F1_transf(a,b,c,z);
	} else {
		return Gauss2F1_odeint(a,b,c,z,eps_accuracy);
	}
}

/* -------------------------------------------------------------
   Functions defined in terms of hypergeometric function F (2F1)
   ------------------------------------------------------------- */

/* LegendreP(n,x) is the nth Legendre polynomial P_n(x)
   evaluated at x.
   LegendreP(n,m,x) is the associated Legendre function
   P_n^m(x).
   The Arfken definition is in use, which is different from
   e.g. what Mathematica uses.*/

Tcomplex LegendreP(const Tcomplex& n, const Tcomplex& m, const Tcomplex& x, Treal eps_accuracy)
{
	return exp(logfact(n+m) - logfact(n-m) - logfact(m)) *
		pow(0.5*sqrt(1-x*x),m) *
		Gauss2F1(m-n,m+n+1,m+1,0.5*(1-x),eps_accuracy);
}

Tcomplex LegendreP(const Tcomplex& n, const Tcomplex& x, Treal eps_accuracy)
{
	return Gauss2F1(-n,n+1,1,0.5*(1-x),eps_accuracy);
}

Tcomplex ChebyshevT(const Tcomplex& n, const Tcomplex& x, Treal eps_accuracy)
// ChebyshevT(n,x) is the nth Chebyshev (Tshebyshev)
// polynomial evaluated at x.
{
	return Gauss2F1(-n,n,0.5,0.5*(1-x),eps_accuracy);
}
