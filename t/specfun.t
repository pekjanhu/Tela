/*
   Some special functions for Tela. Started: PJ 23.12.1994.
   Last modified 2.7.2002 to partly use faster functions in
   specfun.C, specfun_ct.ct.

   The functions:
   
   The Euler Gamma function and its relatives:
   -------------------------------------------
   - Gamma(z)		(Verified with Mma)
   - logGamma(z) := log(Gamma(z))
   - fact(z) := z!
   - logfact(z) := log(z!) (Now in specfun.C)
   - Beta(x,y) := Gamma(x)*Gamma(y)/Gamma(x+y)
   - Binomial(n,k) := (n over k)		(Verified with Mma)
   - poch(a,n) := a*(a+1)* ... *(a+n-1)	(Pochhammer symbol)

   The incomplete gamma function and its relatives:
   ------------------------------------------------
   - IncompleteGammaP(a,x):= IncompleteGammaPQ(a,x,0) (Now in specfun.C)
   - IncompleteGammaQ(a,x):= 1 - IncompleteGammaP(a,x) (Now in specfun.C)
   - IncompleteGamma(a,x):= IncompleteGammaQ(a,x)*Gamma(a) (Verified with Mma for Re(a) > 0)
   - erf(z), erfc(z)	(Verified with Mma)

   The incomplete beta function:
   -----------------------------
   - BetaI(x,a,b) (Verified with Mma)
   - BetaB(x,a,b):= BetaI(x,a,b)*Beta(a,b)

   All the above functions are computed with arbitrary, possibly complex,
   argument. The functions are applied componentwise on array arguments.

   The "verified with Mma" means that it was verified for some automatically
   generated random values in the complex plane, usually 100 values in the
   range -3-3i .. 3+3i, but in some cases larger, and up to 7 digits no
   differences with Mathematica were found among these values. Many functions
   become highly oscillatory or of very large amplitude in some regions of
   the complex plane far away from the origin.

   The hypergeometric functions:
   -----------------------------
   - KummerU(a,c,z) is simply related to Kummer1F1(a,c,z) [U is sometimes called Psi(a,c,z)]

   Functions defined in terms of hypergeometric functions:
   -------------------------------------------------------
   - BesselJ(n,z), BesselI(n,z)		(Verified with Mma)
   - NeumannN(n,z), HankelH1(n,z), HankelH2(n,z)	(*** Do not work on integral indices ***)
   - BesselK(n,z)		(*** Also does not work on integral index ***)
   - LaguerreL(n,z), LaguerreL(n,a,z)	(Verified with Mma)

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
	Gamma,logGamma,fact,Beta,Binomial,poch,
	IncompleteGamma,erf,erfc,
    BetaB,BetaI,
	KummerU,
	BesselI,BesselJ,NeumannN,HankelH1,HankelH2,BesselK,LaguerreL
)
{

epsilon = 1E-10;		// general accuracy, used by various functions

/* -----------------------------------------------------------------
   The Euler Gamma function and associates
   ----------------------------------------------------------------- */
 
function w=fact(z)
/* fact(n) returns the factorial function n!. The argument
   can be any numerical values, also complex and array. For
   arrays, the operation is applied componentwise.
   See also: logfact, Gamma, logGamma
*/
{
	w = exp(logfact(z))
};

function w=logGamma(z)
/* logGamma(z) returns the logarithm of the Euler Gamma function
   log(Gamma(z)). The argument can be any numerical values, also
   complex and array. For arrays, the operation is applied
   componentwise.
   See also: Gamma, fact, logfact
*/
{
	w = logfact(z-1)
};

function w=Gamma(z)
/* Gamma(z) returns the Euler Gamma function.
   The argument can be any numerical values, also complex and
   array. For arrays, the operation is applied componentwise.
   See also: logGamma, fact, logfact, erf, erfc.
*/
{
	w = exp(logfact(z-1))
};

function w=Beta(x,y)
/* Beta(x,y) computes the Euler Beta function, which is simply
   defined in terms of the Euler Gamma function as
   Beta(x,y) := Gamma(x)*Gamma(y)/Gamma(x+y).
   The arguments can be any numerical values, also complex and
   arrays. For arrays, the operation is applied componentwise.
   See also: Gamma, fact, logfact, logGamma, erf, erfc
*/
{
	w = exp(logGamma(x) + logGamma(y) - logGamma(x+y))
};

function w=Binomial(n,k)
/* Binomial(n,k) returns the binomial coefficient (n over k),
   which is defined in terms of factorials as n!/(k!*(n-k)!).
   The arguments can be any numerical values, also complex and
   arrays. For arrays, the operation is applied componentwise.
   See also: fact, logfact, Gamma, logGamma
*/
{
	w = exp(logfact(n)-logfact(k)-logfact(n-k))
};

function w=poch(a,n)
/* poch(a,n) computes the Pochhammer symbol (a)_n, defined as
   (a)_n := a*(a+1)* ... *(a+n-1). The arguments can be any
   numerical values, also complex and arrays. For arrays, the
   operation is applied componentwise.
   See also: fact, logfact, Gamma, logGamma
*/
{
	a1 = a-1;
	w = exp(logfact(a1+n) - logfact(a1))
};

/* -----------------------------------------------------------
   The incomplete Gamma function IncompleteGamma and erf, erfc
   ----------------------------------------------------------- */

function w=IncompleteGamma(a,x)
/* IncompleteGamma(a,x) is the incomplete gamma function
   IncompleteGamma(a,x) := integrate(exp(-t)*t^(a-1),t=x..infinity),
   which is also equal to IncompleteGammaQ(a,x)*Gamma(a).
   The evaluation is reliable for Re(a) > 0 only, although a result
   is returned also for Re(a) <= 0.
   IncompleteGamma(a,x) is the same as Gamma[a,x] in Mathematica.
   See also: IncompleteGammaP, IncompleteGammaQ, erf, erfc
*/
{
	w = IncompleteGammaQ(a,x)*Gamma(a);
};

function w=erf(z)
/* erf(z) is the error function,
   erf(z):= (2/sqrt(pi))*integrate(exp(-t^2),t=0..x).
   See also: erfc, IncompleteGammaP, IncompleteGammaQ, IncompleteGamma
*/
{
	//w=(2/sqrt(pi))*z*hypgeoM(0.5,1.5,-z^2)
	w = where(Re(z) < 0, -IncompleteGammaP(0.5,(-z)^2), IncompleteGammaP(0.5,z^2));
};

function w=erfc(z)
/* erfc(z) computes the complementary error function,
   erfc(z):= 1 - erf(z).
   See also: erf
*/
{
	w = where(Re(z) < 0, 1+IncompleteGammaP(0.5,z^2), IncompleteGammaQ(0.5,z^2));
};

function w=KummerU(a,c,z)
/* KummerU(a,c,z) is the confluent hypergeometric function U(a,c; z),
   denoted by Psi in the Gradshteyn-Ryzhik table.
   The evaluation works for non-integral c only.
   See also: Kummer1F1, Gauss2F1
*/
{
//	w = (Gamma(c)*Gamma(1-c)/Gamma(a-c+1))*Kummer1F1Regularized(a,c,z)
//	+ (Gamma(c-1)*Gamma(2-c)/Gamma(a))*z^(1-c)*Kummer1F1Regularized(a-c+1,2-c,z);
	w = (pi/sin(pi*c))*((1/Gamma(1+a-c))*Kummer1F1Regularized(a,c,z)
		 - (1/Gamma(a))*z^(1-c)*Kummer1F1Regularized(a-c+1,2-c,z));
};

/* ----------------------------------------------------------------
   Functions defined in terms of confluent hypergeometric functions
   ---------------------------------------------------------------- */

function w=BetaB(z,a,b)
/* BetaB(z,a,b) is the incomplete Euler Beta function,
   BetaB(z,a,b) = integrate(t^(a-1)*(1-t)^(b-1),t=0..z)
   which is also equal to BetaI(x,a,b)*Beta(a,b).
   BetaB(x,a,b) is the same as Mathematica's Beta[x,a,b].
   See also: Gauss2F1, IncompleteGamma
*/
{
	w = (z^a/a)*Gauss2F1(a,1-b,a+1,z);
};

function w=BetaI(z,a,b)
/* BetaI(z,a,b) is the regularized incomplete Euler Beta function,
   BetaI(z,a,b) = (1/Beta(a,b))*BetaB(z,a,b).
   See also: Beta, BetaB
*/
{
	w = BetaB(z,a,b)/Beta(a,b);
};

function w=BesselJ(n,z)
/* BesselJ(n,z) is the Bessel function of the first kind
   with index n and argument z.
   See also: BesselI, NeumannN, HankelH1, HankelH2, Kummer1F1
*/
{
//	w = exp(-1i*z)*(0.5*z)^n*Kummer1F1(n+0.5,2*n+1,2i*z)/fact(n);
	w = exp(-1i*z)*(0.5*z)^n*Kummer1F1Regularized(n+0.5,2*n+1,2i*z)*poch(n+1,n);
};

function w=NeumannN(n,z)
/* NeumannN(n,z) is the Bessel function of the second kind,
   also denoted BesselY(n,z). The evaluation works for non-integral
   indices n only.
   See also: BesselJ, HankelH1, HankelH2
*/
{
	w = (BesselJ(n,z)*cos(n*pi) - BesselJ(-n,z))/sin(pi*n)
};

function w=HankelH1(n,z)
/* HankelH1(n,z) is the Hankel function of the first kind.
   The evaluation works for non-integral indices only.
   See also: HankelH2, BesselJ, NeumannN
*/
{
	w = (2/sqrt(pi))*exp(1i*(z - pi*(n+0.5)))*(2*z)^n*KummerU(n+0.5,2*n+1,-2i*z)
};

function w=HankelH2(n,z)
/* HankelH2(n,z) is the Hankel function of the second kind.
   The evaluation works for non-integral indices only.
   See also: HankelH1, BesselJ, NeumannN, KummerU
*/
{
	w = (2/sqrt(pi))*exp(1i*(pi*(n+0.5) - z))*(2*z)^n*KummerU(n+0.5,2*n+1,2i*z)
};

function w=BesselI(n,z)
/* BesselI(n,z) computes the modified Bessel function of the first kind
   with index n and argument z. Both input arguments may be complex.
   See also: BesselK, BesselJ, Kummer1F1
*/
{
//	w = exp(-z)*(0.5*z)^n*Kummer1F1(n+0.5,2*n+1,2*z)/fact(n);
	w = exp(-z)*(0.5*z)^n*Kummer1F1Regularized(n+0.5,2*n+1,2*z)*poch(n+1,n);
};

function w=BesselK(n,z)
/* BesselK(n,z) is the modified Bessel function of the second kind.
   The evaluation works for non-integral indices only.
   See also: BesselI, BesselJ, KummerU
*/
{
	w=sqrt(pi)*exp(-z)*(2*z)^n*KummerU(0.5+n,2*n+1,2*z);
};

function w=LaguerreL_3args(n,a,x) {
//	w=Binomial(n+a,n)*Gamma(a+1)*Kummer1F1Regularized(-n,a+1,x);
	w=exp(logfact(n+a)-logfact(n))*Kummer1F1Regularized(-n,a+1,x);
};

function w=LaguerreL(a1,a2; a3)
/* LaguerreL(n,m,x) returns the Laguerre polynomial L_n^m(x)
     (that is, with lower index n and upper index m).
   LaguerreL(n,x) is the same as LaguerreL(n,0,x)
     (that is, the upper index is effectively zero).
   See also: LegendreP
*/
{
	if (isdefined(a3))
		w = LaguerreL_3args(a1,a2,a3)
	else
		w = LaguerreL_3args(a1,0,a2)
};

};
