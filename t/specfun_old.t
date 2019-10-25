/*
   Some special functions for Tela. Started: PJ 23.12.1994.
   Last modified 28.12.2000.
   The externally usable functions currently include:
   
   The Euler Gamma function and its relatives:
   -------------------------------------------
   - Gamma(z)		(Verified with Mma)
   - logGamma(z) := log(Gamma(z))
   - fact(z) := z!
   - logfact(z) := log(z!)
   - Beta(x,y) := Gamma(x)*Gamma(y)/Gamma(x+y)
   - Binomial(n,k) := (n over k)		(Verified with Mma)
   - poch(a,n) := a*(a+1)* ... *(a+n-1)	(Pochhammer symbol)

   The incomplete gamma function and its relatives:
   ------------------------------------------------
   - IncompleteGammaP(a,x)
   - IncompleteGammaQ(a,x):= 1 - IncompleteGammaP(a,x)
   - IncompleteGamma(a,x):= IncompleteGammaQ(a,x)*Gamma(a) (Verified with Mma)
   - erf(z), erfc(z)	(Verified with Mma)

   The incomplete beta function:
   -----------------------------
   - BetaI(x,a,b)
   - BetaB(x,a,b):= BetaI(x,a,b)*Beta(a,b) (Verified with Mma)

   All the above functions are computed with arbitrary, possibly complex,
   argument. The functions are applied componentwise on array arguments.

   The "verified with Mma" means that it was verified for some hand-selected
   values which however represented the whole complex plane for all arguments,
   and up to 7 digits no differences with Mathematica were found among these
   values.

   The hypergeometric functions:
   -----------------------------
   - hypgeoM(a,c,z) := M(a,c,z) [sometimes used notation: Phi(a,c,z)]
   - hypgeoU(a,c,z) is simply related to M(a,c,z) [U is sometimes called Psi(a,c,z)]
   - hypgeoF(a,b,c,z) := F(a,b;c;z) [sometimes called 2F1]

   The hypergeometric functions as implemented here do not work in some part(s)
   of the complex plane. In these parts practically the only way to evaluate it
   would be to integrate the defining ODE directly. Thus the functions defined
   in terms of the hypergeometric functions also suffer from this drawback:
   they do not work on some argument values. Sometimes, these values may be
   just where you want to evaluate them...

   Functions defined in terms of hypergeometric functions:
   -------------------------------------------------------
   - BesselJ(n,z), BesselI(n,z)		(Verified with Mma)
   - NeumannN(n,z), HankelH1(n,z), HankelH2(n,z)	(*** Do not work on integral indices ***)
   - BesselK(n,z)		(*** Also does not work on integral index ***)
   - LaguerreL(n,z), LaguerreL(n,a,z)	(Verified with Mma)
   - LegendreP(n,z)		(Verified with Mma)
   - LegendreP(n,m,z)	(*** Disagrees with Mma ***)
   - ChebyshevT(n,z)	(Verified with Mma)

   In all the above functions, the argument types are not restricted
   in any way, i.e. the functions are evaluated for complex arguments
   also if wanted. In all cases, the arguments may also be arrays,
   in which case the operation is applied componentwise. The arrays
   must be compatible for different arguments, or the other argument
   must be scalar. For example in case of two-argument function Beta(x,y)
   we have the following four possibilities for argument types:
   1. Beta(x,y) for scalar x and scalar y.
   2. Beta(x,y) for array x and scalar y, in which case y is promoted to array.
   3. Beta(x,y) for scalar x and array y, in which case x is promoted to array.
   4. Beta(x,y) for array x and array y. Must be size(x)==size(y).
*/

package
global(
	Gamma,logGamma,fact,logfact,Beta,Binomial,poch,
	IncompleteGamma,IncompleteGammaP,IncompleteGammaQ,erf,erfc,
    BetaB,BetaI,
	hypgeoM,hypgeoU,hypgeoF,
	BesselI,BesselJ,NeumannN,HankelH1,HankelH2,BesselK,LaguerreL,LegendreP,ChevyshevT
)
{

epsilon = 1E-10;		// general accuracy, used by various functions

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
logfactPositive_c = #(76.18009173,-86.50532033,24.01409822,
					  -1.231739516,0.120858003e-2,-0.536382e-5);

function w=logfactPositive(z)
// Returns log(z!) for Re(z) >= 0. Applies componentwise on arrays.
global(logfactPositive_c)
{
	z1 = z + 5.5;
	for ({series=1; x=z+1; i=1}; i<=6; {i++; x++}) series+= logfactPositive_c[i]/x;
	w = (z+0.5)*log(z1) - z1 + log(2.50662827465*series);	// the constant here is = sqrt(2*pi)
};

/*
   The factorial function reflection formula:
       z! = pi*z/((-z)!*sin(pi*z))

    Logarithmic form:
	   log(z!) = log(pi*z/sin(pi*z)) - log((-z)!)

    These formulas are used to compute z! for Re(z) < 0.
*/

function w=logfact(z)
/* logfact(z) returns log(z!) for any numerical z.
   Applies componentwise on arrays.
   If z is an array, the result will be complex array if any(Re(z)<0),
   otherwise the result is a real array.*/
{
	if (isarray(z)) {
		pos = find(Re(z) >= 0);
		neg = find(Re(z) < 0);
		if (any(neg)) w=z+0i else w=z+0.0;
		if (any(pos)) w[pos] = logfactPositive(z[pos]);
		if (any(neg)) w[neg] = log(pi*z[neg]/sin(pi*z[neg])) - logfactPositive(-z[neg]);
	} else {		// z is scalar
		if (Re(z) < 0)
			w = log(pi*z/sin(pi*z)) - logfactPositive(-z)
		else
			w = logfactPositive(z);
	}
};

function w=fact(z)
/* fact(n) returns the factorial function n!. The argument may be
   any complex number. */
{
	w = exp(logfact(z))
};

function w=logGamma(z)
/* logGamma(z) returns the logarithm of the Euler Gamma function
   log(Gamma(z)). The argument type is not restricted. */
{
	w = logfact(z-1)
};

function w=Gamma(z)
/* Gamma(z) returns the Euler Gamma function.
   The argument type is not restricted. */
{
	w = exp(logfact(z-1))
};

function w=Beta(x,y)
/* Beta(x,y) computes the Euler Beta function, which is simply
   defined in terms of the Euler Gamma function as
   Beta(x,y) := Gamma(x)*Gamma(y)/Gamma(x+y).
   No restrictions on argument types. */
{
	w = exp(logGamma(x) + logGamma(y) - logGamma(x+y))
};

function w=Binomial(n,k)
/* Binomial(n,k) returns the binomial coefficient (n over k),
   which is defined in terms of factorials as n!/(k!*(n-k)!).
   The arguments need not be integers, they may also be complex. */
{
	w = exp(logfact(n)-logfact(k)-logfact(n-k))
};

function w=poch(a,n)
/* poch(a,n) computes the Pochhammer symbol (a)_n, defined as
   (a)_n := a*(a+1)* ... *(a+n-1). */
{
	a1 = a-1;
	w = exp(logfact(a1+n) - logfact(a1))
};

/* ----------------------------------------------------------------------------------
   The incomplete Gamma functions IncompleteGammaP, IncompleteGammaQ, IncompleteGamma
   ---------------------------------------------------------------------------------- */

function w=IncompleteGamma_series(a,x)
/* Returns incomplete gamma function P(a,x) evaluated by its
   series representation (reference: Numerical Recipes).
   Works for both scalar and array arguments.*/
global(epsilon)
{
	ap = a;
	del = 1/a;
	summe = 1/a;
	itmax = 100+10*ceil(sqrt(max(abs(a))));
	for (n=1; n<=itmax; n++) {
		ap++;
		del*= x/ap;
		summe+= del;
		if (all(abs(del) <= abs(summe)*epsilon)) break;
	};
	if (n>itmax) format("warning: too many iterations in IncompleteGamma_series(``,``), maxits=``\n",a,x,itmax);
	w = where(x==0,0.0,summe*exp(-x+a*log(x)-logGamma(a)));
};

function y = avoid_origin(z,tiny)
/* avoid_origin(z,tiny) returns z if |z| is not smaller than tiny.
   If it is, it returns tiny. If z is array, the operations is
   applied componentwise. The second argument tiny must be a positive
   real number.*/
{
	if (isarray(z)) {
		y = z;
		ind = find(abs(z) < tiny);
		y[ind] = tiny;
	} else {
		if (abs(z) < tiny) y = tiny else y = z;
	};
};

function w=IncompleteGamma_contfrac(a,x)
/* Returns Q(a,x):= 1-P(a,x) evaluated by its continued fraction
   representation (reference: Numerical Recipes).
   Works for both scalar and array arguments.*/
global(epsilon)
{
	b = x + 1 - a;
	fpmin = 1e-30;
	c = 1/fpmin;
	d = 1/b;
	h = d;
	itmax = 100+10*ceil(sqrt(max(abs(a))));
	for (i=1; i<=itmax; i++) {
		an = -i*(i-a);
		b+= 2;
		d = avoid_origin(an*d + b,fpmin);
		c = avoid_origin(b + an/c,fpmin);
		d = 1/d;
		del = d*c;
		h*= del;
		if (all(abs(del-1) < epsilon)) break;
	};
	if (i > itmax) format("warning: no convergence in IncompleteGamma_contfrac\n");
	w = exp(-x + a*log(x) - logGamma(a))*h;
};

function w=IncompleteGammaP(a,x)
/* IncompleteGammaP(a,x) is the regularized incomplete gamma function
   IncompleteGammaP(a,x) := integrate(exp(-t)*t^(a-1),t=0..x)/Gamma(a).
   This notation conforms with Numerical Recipes.*/
{
	if (isarray(a) || isarray(x)) {
		xx = 0*a + x;
		aa = a + 0*x;
		w = 0*xx + 0.0i;
		series_ind = find(Re(xx) < Re(aa)+1);
		cf_ind = find(Re(xx) >= Re(aa)+1);
		w[series_ind] = IncompleteGamma_series(aa[series_ind],xx[series_ind]);
		w[cf_ind] = 1 - IncompleteGamma_contfrac(aa[cf_ind],xx[cf_ind]);
		if (all(Im(w)==0)) w = Re(w);
	} else {
		if (Re(x) < Re(a)+1) {
			w = IncompleteGamma_series(a,x);
		} else {
			w = 1 - IncompleteGamma_contfrac(a,x);
		};
	};
};

function w=IncompleteGammaQ(a,x)
/* IncompleteGammaQ(a,x) is the complement regularized incomplete gamma function
   IncompleteGammaQ(a,x) := 1 - IncompleteGammaP(a,x).
   This notation conforms wich Numerical Recipes.
   IncompleteGammaQ(a,x) is the same as GammaRegularized[a,x]
   in Mathematica.*/
{
	if (isarray(a) || isarray(x)) {
		xx = 0*a + x;
		aa = a + 0*x;
		w = 0*xx + 0.0i;
		series_ind = find(Re(xx) < Re(aa)+1);
		cf_ind = find(Re(xx) >= Re(aa)+1);
		w[series_ind] = 1 - IncompleteGamma_series(aa[series_ind],xx[series_ind]);
		w[cf_ind] = IncompleteGamma_contfrac(aa[cf_ind],xx[cf_ind]);
		if (all(Im(w)==0)) w = Re(w);
	} else {
		if (Re(x) < Re(a)+1) {
			w = 1 - IncompleteGamma_series(a,x);
		} else {
			w = IncompleteGamma_contfrac(a,x);
		};
	};
};

function w=IncompleteGamma(a,x)
/* IncompleteGamma(a,x) is the incomplete gamma function
   IncompleteGamma(a,x) := integrate(exp(-t)*t^(a-1),t=x..infinity),
   which is also equal to IncompleteGammaQ(a,x)*Gamma(a).
   IncompleteGamma(a,x) is the same as Gamma[a,x] in Mathematica.*/
{
	w = IncompleteGammaQ(a,x)*Gamma(a);
};

function w=erf(z)
/* erf(z) computes the error function,
   erf(z):= (2/sqrt(pi))*integrate(exp(-t^2),t=0..x) */
{
	//w=(2/sqrt(pi))*z*hypgeoM(0.5,1.5,-z^2)
	w = where(Re(z) < 0, -IncompleteGammaP(0.5,(-z)^2), IncompleteGammaP(0.5,z^2));
};

function w=erfc(z)
/* erfc(z) computes the complementary error function,
   erfc(z):= 1 - erf(z). */
{
	w = where(Re(z) < 0, 1+IncompleteGammaP(0.5,z^2), IncompleteGammaQ(0.5,z^2));
};

/* --------------------------------------------------------
   The incomplete Beta functions BetaI(x,a,b), BetaB(x,a,b)
   -------------------------------------------------------- */

function w=BetaI_contfrac(x,a,b)
/* BetaI_contfrac(x,a,b) is the same as Numerical Recipes routine
   betacf(a,b,x) (notice that arguments in different order!).*/
global(epsilon)
{
	fpmin = 1e-30;
	itmax = 100 + 5*sqrt(max(max(abs(a)),max(abs(b))));
	qab = a + b;
	qap = a + 1;
	qam = a - 1;
	c = 1.0;
	d = avoid_origin(1 - qab*x/qap,fpmin);
	d = 1/d;
	w = d;
	for (m=1; m<=itmax; m++) {
		m2 = 2*m;
		aa = m*(b-m)*x/((qam+m2)*(a+m2));
		d = avoid_origin(1+aa*d,fpmin);
		c = avoid_origin(1+aa/c,fpmin);
		d = 1/d;
		w*= d*c;
		aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
		d = avoid_origin(1+aa*d,fpmin);
		c = avoid_origin(1+aa/c,fpmin);
		d = 1/d;
		del = d*c;
		w*= del;
		if (all(abs(del-1) < epsilon)) break;
	};
	if (m > itmax) format("warning: too many iterations in BetaI_contfrac\n");
};

function w=BetaI(x,a,b)
/* BetaI(x,a,b) is the incomplete beta function I_x(a,b),
   BetaI(x,a,b) := integrate(t^(a-1)*(1-t)^(b-1),t=0..x)/Beta(a,b).
   BetaI(x,a,b) is the same as Mathematica's BetaRegularized[x,a,b].*/
{
	bt = where(x==0 || x==1,0.0,exp(logGamma(a+b) - logGamma(a) - logGamma(b) + a*log(x) + b*log(1-x)));
	if (isarray(bt)) {
		w = 0*bt;
		cond = Re(x) < Re((a+1)/(a+b+2));
		direct = find(cond);
		indirect = find(!cond);
		xx = x + 0*a + 0*b;
		aa = 0*x + a + 0*b;
		bb = 0*x + 0*a + b;
		w[direct] = bt[direct]*BetaI_contfrac(xx[direct],aa[direct],bb[direct])/aa[direct];
		w[indirect] = 1 - bt[indirect]*BetaI_contfrac(1-xx[indirect],bb[indirect],aa[indirect])/bb[indirect];
	} else {
		if (Re(x) < Re((a+1)/(a+b+2))) {
			w = bt*BetaI_contfrac(x,a,b)/a;
		} else {
			w = 1 - bt*BetaI_contfrac(1-x,b,a)/b;
		};
	};
};

function w=BetaB(x,a,b)
/* BetaB(x,a,b) is the incomplete beta function B_x(a,b),
   BetaB(x,a,b) := integrate(t^(a-1)*(1-t)^(b-1),t=0..x)
   which is also equal to BetaI(x,a,b)*Beta(a,b).
   BetaB(x,a,b) is the same as Mathematica's Beta[x,a,b].*/
{
	w = BetaI(x,a,b)*Beta(a,b);
};

/* -----------------------------------------------------------------
   The confluent hypergeometric function M(a,c;z)
   ----------------------------------------------------------------- */

function w=hypgeoM_Series(a,c,z) global(epsilon)
{
	w = 1.0;
	term = 1.0;
	for (n=1; n<500; n++) {
		n1 = n-1;
		term*= (a+n1)*z/((c+n1)*n);
		w+= term;
		if (abs(term) <= epsilon*abs(w)) break;
	};
	//format("hypgeoM_Series: `` iters, w=``\n",n,w);
};

function w=hypgeoM_Asympt(a,c,z)
{
	//format("hypgeoM_Asympt(``,``,``)\n",a,c,z);
	epsilon2 = 1E-20;		// epsilon squared
	cond = Re(z) >= 0;
	a1 = where(cond, a, c-a);
	if (isarray(a1)) {
		if (a1[1]==a1) a1=a1[1]
	};
	z1 = where(cond, z, -z);
	if (isarray(z1)) {
		if (z1[1]==z1) z1=z1[1]
	};
	C = where(cond, exp(z1), 1.0);
	if (isarray(C)) {
		if (C[1]==C) C=C[1]
	};
	//format("a1=``, z1=``, C=``\n",a1,z1,C);
	S = 1.0;
	term = 1.0;
	invz = 1/z1;
	for (n=1; n<30; n++) {
		coeff = (n-a1)*(c-a1+n-1)*invz/n;
		if (any(abs2(coeff) > 1)) {format("hypgeoM_Asympt: badbreak\n"); break;};
		term*= coeff;
		//format("term=``\n",term);
		if (abs2(term) < epsilon2*abs2(S)) break;
		S+= term;
	};
	w = (Gamma(c)/Gamma(a1))*C*S*z1^(a1-c);
	//format("hypgeoM_Asympt: `` iters, sum=``, w=``\n",n,S,w);
};

function [m,i] = findmin(v)
/* [m,i] = findmin(v) returns the minimum of v and its index. */
{
	m = min(v);
	is = find(v==m);
	i = is[1];
};

function [y,dy] = polint(xa,ya,x)
/* (Adapted from POLINT of Numerical Recipes 1st ed.)
   [y,dy] = polint(xa,ya,x).
   Polynomial interpolation/extrapolation through points
   (xa,ya). The polynomial P(x) satisfies P(xa[i]) = ya[i]
   for all i; it is evaluated at x to give y. Error estimate
   dy (always non-negative) is obtained by dropping one point.*/
{
	n = length(xa);
	c = ya;
	d = ya;
	ns = 1;
	[dif,ns] = findmin(abs(x-xa));
	y = ya[ns];
	ns--;
	for (m=1; m<=n-1; m++) {
		i = 1:n-m;
		h0 = xa[i] - x;
		hp = xa[i+m] - x;
		coeff = (c[i+1] - d[i])/(h0-hp);
		d[i] = hp*coeff;
		c[i] = h0*coeff;
		if (2*ns < n-m)
			dy = c[ns+1]
		else {
			dy = d[ns];
			ns--;
		};
		y+= dy;
	};
	dy = abs(dy);
};

function [I,dI]=vromberg(f,a,b)
/* vromberg(fa,a,b) estimates the definite integral
   of a function f from a to b.
   fa must contain tabulated values of the integrand
   at equally spaced abscissas such that fa[1] = f(a),
   fa[n] = f(b) where n=length(fa). The length n must
   be a power of two plus one. Error estimate is returned
   in the second output parameter if wanted.

   The Romberg method: estimate the integral using
   a sequence of finer grids and extrapolating to
   infinitely dense grid. Notice that vromberg does not
   compute function values on its own but uses ONLY the
   values already tabulated. Thus it can be used also to
   integrate data. */
{
	n = length(f);
	j = round(log(n-1)/log(2));
	if (2^j != n-1) {format("*** vromberg: vlen must be 2^j-1 not ``\n",n); return};
	k = min(j,5);
	H = 0.25^(0:k-1);
	steps = 2^(k-1:-1:0);
	correction = 0.5*(f[1] + f[n]);
	S = zeros(k)+0*correction;
	for (i=1; i<=k; i++)
		S[i] = (sum(f[1:steps[i]:n]) - correction);
	h = (b-a)/(n-1);
	[I,dI] = polint(H,h*steps*S,0.0);
};

// /*Now builtin*/ function y=nrange(a,b,n) {y=a+(0:n-1)*(b-a)/(n-1)};

function w=hypgeoM_Integr(a,c,z)
/* Computes M(a,c;z) using the integral representation. Arguments must be scalars.
   Relations Re(c) > Re(a) > 0 must hold. */ 
{
	if (any(Re(a) <= 0 || Re(c)-Re(a) <= 0)) {
		format("*** hypgeoM_Integr: sorry cannot evaluate M(a,c;z). Re(c)>Re(a)>0 does not hold.\n");
		w = 0.0;
		return;
	};
	x = nrange(0,1,65);
	a1 = 1/a;
	a2 = 1/(c-a);
	xtoa1 = x^a1;
	xtoa2 = x^a2;
	f = exp((c-a-1)*log(2-xtoa1) + 0.5*z*(xtoa1-1))*a1 + exp((a-1)*log(2-xtoa2) - 0.5*z*(xtoa2-1))*a2;
	w = (2^(1-c)/Beta(a,c-a))*exp(0.5*z)*vromberg(f,0,1);
};

function w=hypgeoM(a,c,z)
/* hypgeoM(a,c,z) computes the confluent hypergeometric function M(a,c; z).
   No restrictions on argument types.

   Algorithm:
   For |z| < series_limit computes using the power series, currently
     series_limit=10.
   For |z| outside series limit uses the asymptotic (1/z) expansion
     if |Im z| <= |Re z|,
   otherwise integrates numerically from a regularized integral representation. */
{
	series_limit = 12.0;
	if (isarray(z)) {
		series_cond = abs2(z) < series_limit^2;
		asympt_cond = abs2(z) >= series_limit^2 && (abs(Im(z)) < abs(Re(z)));
		integr_cond = !series_cond && !asympt_cond;
		series = find(series_cond);
		integr = find(integr_cond);
		asympt = find(asympt_cond);
		w = a + c + z;		// make w of correct size and type; the values are overwritten anyway
		if (any(series)) {
			//format("hypgeoM: `` series\n",length(series));
			if (isarray(a)) a1=a[series] else a1=a;
			if (isarray(c)) c1=c[series] else c1=c;
			w[series] = hypgeoM_Series(a1,c1,z[series]);
		};
		if (any(asympt)) {
			//format("hypgeoM: `` asympt\n",length(asympt));
			if (isarray(a)) a1=a[asympt] else a1=a;
			if (isarray(c)) c1=c[asympt] else c1=c;
			w[asympt] = hypgeoM_Asympt(a1,c1,z[asympt]);
		};
		if (any(integr)) {
			//format("hypgeoM: computing `` integrals\n",length(integr));
			for (i=1; i<=length(integr); i++) {
				if (isarray(a)) a1=a[i] else a1=a;
				if (isarray(c)) c1=c[i] else c1=c;
				w[integr[i]] = hypgeoM_Integr(a1,c1,z[integr[i]]);
			}
		};
	} else {		// z is scalar
		if (abs(z) < series_limit)
			w = hypgeoM_Series(a,c,z)
		else if (abs(Im(z)) < abs(Re(z))) {
			if (isarray(a) || isarray(c)) {
				w = a + c + z;
				for (i=1; i<=length(a); i++) {
					if (isarray(a)) a1=a[i] else a1=a;
					if (isarray(c)) c1=c[i] else c1=c;
					w[i] = hypgeoM_Integr(a1,c1,z);
				}
			} else
				w = hypgeoM_Integr(a,c,z)
		} else
			w = hypgeoM_Asympt(a,c,z);
	}
};

function w=hypgeoU(a,c,z)
/* hypgeoU(a,c,z) returns the confluent hypergeometric function U(a,c; z).
   It is defined simply in terms of the confluent hypergeometric function
   M(a,c;z), which is computed by hypgeoM(a,c,z).
   For definition of U, see Gradshteyn & Ryzhik, where U is called Psi,
   or the source code that follows.

   *** Warning: Does not work on integer c. This is not checked! */
{
	//format("hypgeoU(a=``, c=``)\n",a,c);
	w = (Gamma(1-c)/Gamma(a-c+1))*hypgeoM(a,c,z) + (Gamma(c-1)/Gamma(a))*z^(1-c)*hypgeoM(a-c+1,2-c,z)
};

/* -----------------------------------------------------------------
   The hypergeometric function F(a,b,c;z) (or 2F1)
   ----------------------------------------------------------------- */

function w=hypgeoF_Series(a,b,c,z)
// Computes F(a,b,c;z) using the power series centered at z=0
global(epsilon)
{
	w = 1.0;
	term = 1.0;
	for (n=1; n<500; n++) {
		n1 = n-1;
		term*= (a+n1)*(b+n1)*z/((c+n1)*n);
		w+= term;
		if (abs(term) < epsilon*abs(w)) break;
	};
	format("hypgeoF_Series: `` iters\n",n);
};

function w=F1(a,b,c) {
	w = exp(logGamma(c)+logGamma(c-a-b)-logGamma(c-a)-logGamma(c-b))
};

/*
function w=F(a,b,c,z)
{
	n = 0:50;
	/*
	S = ((z-1)^n/fact(n))*poch(a,n)*poch(b,n)*fact(c-a-b-n-1);
	abs(S[length(S)]/max(abs(S)));
	w = sum(S)*Gamma(c)/(Gamma(c-a)*Gamma(c-b));
	*/
	beta = (poch(a,n)/fact(n))*(poch(b,n)/poch(c,n))*F1(a+n,b+n,c+n);
	abs(beta[length(beta)])/max(abs(beta));
	w = sum(beta*(z-1)^n)
};
*/
function w=F(a,b,c,z)
{
	n = 0:50;
	z0 = -0.7;
	dnF_z0 = (poch(a,n)*poch(b,n)/poch(c,n))*hypgeoF_Series(a+n,b+n,c+n,z0);
	beta = dnF_z0/fact(n);
	terms = beta*(z-z0)^n;
	abs(terms[length(terms)])/max(abs(terms));
	w = sum(terms);
};

function w=hypgeoF_Series_1(a,b,c,z)
// Computes F(a,b,c;z) using the power series centered at z=1
global//(epsilon)
{
	z1 = z-1;
	term = 1/fact(a+b-c);
	S = term;
	for (n=1; n<500; n++) {
		n1 = n-1;
		term*= (a+n1)*(b+n1)*z1/((a+b-c+n)*n);
		S+= term*(-1)^n;
		if (abs(term) < epsilon*abs(S)) break;
	};
	w = pi*Gamma(c)*S/(sin(pi*(c-a-b))*Gamma(c-a)*Gamma(c-b));
//	format("hypgeoF_Series_1: `` iters\n",n);
};

function w=hypgeoF_Integr(a,b,c,z)
{
	if (any(Re(b) <= 0 || Re(c)-Re(b)<=0)) {
		format("*** hypgeoF_Integr: sorry can't evaluate F(a,b,c;z). Re(c)>Re(b)>0 doesn't hold.\n");
		w = 0.0;
		return;
	};
	n = 65;
	// compute I1
	x = nrange(0,0.5^b,65);
	f = (1 - x^(1/b))^(c-b-1) * (1 - z*x^(1/b))^(-a);
	I1 = vromberg(f,0,0.5^b)/b;
	// compute I2
	x = nrange(0,0.5^(c-b),65);
	f = (1 - x^(1/(c-b)))^(b-1) * (1 - z*(1 - x^(1/(c-b))))^(-a);
	I2 = vromberg(f,0,0.5^(c-b))/(c-b);
	w = (I1 + I2)/Beta(b,c-b);
};

//function w=hypgeoF(a,b,c,z) {w=hypgeoF_Series(a,b,c,z)};

function w=hypgeoF(a,b,c,z)
/* hypgeoF(a,b,c,z) is the hypergeometric function 2F1(a,b,c;z).*/
{
	series_limit = 0.9;
	if (isarray(z)) {
		series_cond = abs2(z) < series_limit^2;
		integr_cond = !series_cond && (abs2(z) < series_limit^(-2));
		series = find(series_cond);
		integr = find(integr_cond);
		transf = find(!series_cond && !integr_cond);
		if (any(transf) || any(integr))
			w = z + 0i
		else
			w = a + b + c + z;		// make w of correct size and type; the values are overwritten anyway
		if (any(series)) {
			//format("hypgeoF: `` series\n",length(series));
			if (isarray(a)) a1=a[series] else a1=a;
			if (isarray(b)) b1=b[series] else b1=b;
			if (isarray(c)) c1=c[series] else c1=c;
			w[series] = hypgeoF_Series(a1,b1,c1,z[series]);
		};
		if (any(transf)) {
			//format("hypgeoF: `` transf\n",length(transf));
			if (isarray(a)) a1=a[asympt] else a1=a;
			if (isarray(b)) b1=b[asympt] else b1=b;
			if (isarray(c)) c1=c[asympt] else c1=c;
			w[transf] = exp( logGamma(c1) + logGamma(b1-a1) - logGamma(b1) - logGamma(c1-a1) ) *
				        (-1/z[transf]+0i)^a1 *
						hypgeoF_Series(a1,a1+1-c1,a1+1-b1,1/z[transf]) +
						exp( logGamma(c1) + logGamma(a1-b1) - logGamma(a1) - logGamma(c1-b1) ) *
						(-1/z[transf]+0i)^b1 *
						hypgeoF_Series(b1,b1+1-c1,b1+1-a1,1/z[transf]);
		};
		if (any(integr)) {
			//format("hypgeoF: computing `` integrals\n",length(integr));
			for (i=1; i<=length(integr); i++) {
				if (isarray(a)) a1=a[i] else a1=a;
				if (isarray(b)) b1=b[i] else b1=b;
				if (isarray(c)) c1=c[i] else c1=c;
				w[integr[i]] = hypgeoF_Integr(a1,b1,c1,z[integr[i]]);
			}
		};
	} else {		// z is scalar
		az = abs(z);
		if (az < series_limit)
			w = hypgeoF_Series(a,b,c,z)
		else if (az < 1/series_limit) {
			if (isarray(a) || isarray(b) || isarray(c)) {
				w = a + b+ c + z;
				for (i=1; i<=length(a); i++) {
					if (isarray(a)) a1=a[i] else a1=a;
					if (isarray(b)) b1=b[i] else b1=b;
					if (isarray(c)) c1=c[i] else c1=c;
					w[i] = hypgeoF_Integr(a1,b1,c1,z);
				}
			} else
				w = hypgeoF_Integr(a,b,c,z)
		} else {
			w = exp( logGamma(c) + logGamma(b-a) - logGamma(b) - logGamma(c-a) ) *
				(-1/z+0i)^a *
				hypgeoF_Series(a,a+1-c,a+1-b,1/z) +
				exp( logGamma(c) + logGamma(a-b) - logGamma(a) - logGamma(c-b) ) *
				(-1/z+0i)^b *
				hypgeoF_Series(b,b+1-c,b+1-a,1/z);
		}
	}
};

/* ------------------------------------------------------------------------
   Functions defined in terms of confluent hypergeometric functions M and U
   ------------------------------------------------------------------------ */

//function w=BetaB(z,a,b)
///* BetaB(z,a,b) is the incomplete Euler Beta function,
//   BetaB(z,a,b) = integrate(t^(a-1)*(1-t)^(b-1),t=0..z).*/
//{
//	w = (z^a/a)*hypgeoF(a,1-b,a+1,z);
//};

//function w=BetaI(z,a,b)
///* BetaI(z,a,b) is the regularized incomplete Euler Beta function,
//   BetaI(z,a,b) = (1/Beta(a,b))*BetaB(z,a,b).*/
//{
//	w = BetaB(z,a,b)/Beta(a,b);
//};

function w=BesselJ(n,z)
/* BesselJ(n,z) computes the Bessel function of the first kind
   with index n and argument z. Both input arguments can be complex. */
{
	w = exp(-1i*z)*(0.5*z)^n*hypgeoM(n+0.5,2*n+1,2i*z)/fact(n);
	// Chop away any imaginary part for real input
	if (isreal(n) && isreal(z)) w = Re(w);
};

function w=NeumannN(n,z)
{
	w = (BesselJ(n,z)*cos(n*pi) - BesselJ(-n,z))/sin(pi*n)
};

function w=HankelH1(n,z)
{
	w = (2/sqrt(pi))*exp(1i*(z - pi*(n+0.5)))*(2*z)^n*hypgeoU(n+0.5,2*n+1,-2i*z)
};

function w=HankelH2(n,z)
{
	w = (2/sqrt(pi))*exp(1i*(pi*(n+0.5) - z))*(2*z)^n*hypgeoU(n+0.5,2*n+1,2i*z)
};

function w=BesselI(n,z)
/* BesselI(n,z) computes the modified Bessel function of the first kind
   with index n and argument z. Both input arguments may be complex. */
{
	w = exp(-z)*(0.5*z)^n*hypgeoM(n+0.5,2*n+1,2*z)/fact(n)
};

function w=BesselK(n,z)
{
	w=sqrt(pi)*exp(-z)*(2*z)^n*hypgeoU(0.5+n,2*n+1,2*z);
};

function w=LaguerreL_3args(n,a,x) {w=Binomial(n+a,n)*hypgeoM(-n,a+1,x)};

function w=LaguerreL(a1,a2; a3)
/* LaguerreL(n,m,x) returns the Laguerre polynomial L_n^m(x)
     (that is, with lower index n and upper index m).
   LaguerreL(n,x) is the same as LaguerreL(n,0,x)
     (that is, the upper index is effectively zero). */
{
	if (isdefined(a3))
		w = LaguerreL_3args(a1,a2,a3)
	else
		w = LaguerreL_3args(a1,0,a2)
};

/* -------------------------------------------------------------
   Functions defined in terms of hypergeometric function F (2F1)
   ------------------------------------------------------------- */

function w=LegendreP_3args(n,m,x)
{
	w = exp(logfact(n+m) - logfact(n-m) - logfact(m)) *
		(0.5*sqrt(1-x^2))^m *
		hypgeoF(m-n,m+n+1,m+1,(1-x)/2)
};

function w=LegendreP_2args(n,x)
{
	w = hypgeoF(-n,n+1,1,(1-x)/2)
};

function w=LegendreP(a1,a2;a3)
/* LegendreP(n,x) is the nth Legendre polynomial P_n(x)
   evaluated at x.
   LegendreP(n,m,x) is the associated Legendre function
   P_n^m(x).
   The Arfken definition is in use, which is different from
   e.g. what Mathematica uses.*/
{
	if (isdefined(a3))
		w = LegendreP_3args(a1,a2,a3)
	else
		w = LegendreP_2args(a1,a2)
};

function w=ChebyshevT(n,x)
/* ChebyshevT(n,x) is the nth Chebyshev (Tshebyshev)
   polynomial evaluated at x. */
{
	w = hypgeoF(-n,n,0.5,(1-x)/2)
};


};
