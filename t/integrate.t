package global(integrate) {

/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

// Functions:  integrate
// Purpose:    Numerical integration
// Method:     Romberg method (trapezoidal method plus polynomial
//             extrapolation of the result to continuum limit).
// Limitation: Only one-dimensional integrals can be done
// Created:    17.3.1995
// Reference:  Numerical Recipes

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
	[dif,ns] = min(abs(x-xa));
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

function [s,TrapezIt;] = TrapezStep(f,a,b,n)
/* (Adapted from TRAPZD of Numerical Recipes 1st ed.)
   Computes the nth stage of refinement of extended trapezoidal
   rule. The first call with n==1 returns the crudest estimate
   in s. Subsequent calls with n=2,3,... (in that order) will
   improve the accuracy of s by adding 2^(n-1) additional interior
   points. s should not be modified between sequential calls. */
{
	//format("TrapezStep n=``, s=``\n",n,s);
	if (n == 1) {
		s = 0.5*(b-a)*(f(a) + f(b));
		TrapezIt = 1;
	} else {
		tnm = TrapezIt;
		delta = (b-a)/tnm;
		x = a + ((1:TrapezIt) - 0.5)*delta;
		s = 0.5*(s + (b-a)*sum(f(x))/tnm);
		TrapezIt*= 2;
	};
};

function [s,MidpointIt;] = MidpointStep(f,a,b,n)
{
	if (n == 1) {
		s = (b-a)*f(0.5*(a+b));
		MidpointIt = 1;
	} else {
		tnm = MidpointIt;
		delta = (b-a)/(3*tnm);
		x1 = a + 0.5*delta + (0:MidpointIt-1)*(3*delta);
		x2 = x1 + 2*delta;
		s = (s + (b-a)*(sum(f(x1)) + sum(f(x2)))/tnm)/3;
		MidpointIt*= 3;
	};
};

function ss = Romberg(f,a,b,epsilon)
// f(a) and f(b) must exist
{
	jmax = 20;
	k = 5;
	S = czeros(jmax+1);
	H = zeros(jmax+1);
	H[1] = 1;
	TrapezIt = 0;
	for (j=1; j<=jmax; j++) {
		[s,TrapezIt] = TrapezStep(f,a,b,j);
		S[j] = s;
		if (j >= k) {
			S1 = S[j-k+1:j];
			if (Im(S1)==0) S1=Re(S1);
			[ss,dss] = polint(H[j-k+1:j],S1,0);
			if (abs(dss) < epsilon*abs(ss)) return;
		};
		S[j+1] = S[j];
		H[j+1] = 0.25*H[j];
	};
	format("*** Too many (``) steps in Romberg integration\n",jmax+1);
};

function ss = OpenRomberg(f,a,b,epsilon)
// f(a) and f(b) need not exist
{
	jmax = 13;
	k = 4;
	S = czeros(jmax+1);
	H = zeros(jmax+1);
	H[1] = 1;
	MidpointIt = 0;
	for (j=1; j<=jmax; j++) {
		[s,MidpointIt] = MidpointStep(f,a,b,j);
		S[j] = s;
		if (j >= k) {
			S1 = S[j-k+1:j];
			if (Im(S1)==0) S1=Re(S1);
			[ss,dss] = polint(H[j-k+1:j],S1,0);
			if (abs(dss) < epsilon*abs(ss)) return;
		};
		S[j+1] = S[j];
		H[j+1] = H[j]/9;
	};
	format("*** Too many (``) steps in open Romberg integration\n",jmax+1);
};

function ss = integrate(f,a,b; arg1,arg2)
/* integrate(f,a,b) integrates function f numerically over
   the interval [a,b].
   integrate(f,a,b,epsilon) stops when fractional accuracy
   is lower than epsilon. The default is 1E-6.
   integrate(f,a,b,"method") specifies the method. Possible
   methods are "romberg" and "openromberg".
   integrate(f,a,b,"method",epsilon) also works.
   The openromberg method should be used if the integrand
   can not be evaluated and the endpoints.
   
   Warning: If the value of the integral is zero, the
   function may take very long time and a lot of memory
   unless you increase epsilon. */
global(Romberg,OpenRomberg)
{
	epsilon = 1E-6;
	method = "romberg";
	integrator = Romberg;
	if (isdefined(arg1) && isscalar(arg1)) epsilon=arg1;
	if (isdefined(arg2) && isscalar(arg2)) epsilon=arg2;
	if (isdefined(arg1) && isstring(arg1)) method=arg1;
	if (isdefined(arg2) && isstring(arg2)) method=arg2;
	if (method == "romberg")
		integrator = Romberg
	else if (method == "openromberg")
		integrator = OpenRomberg
	else
		format("*** Unknown method '``' in integrate, using romberg\n",method);
	ss = integrator(f,a,b,epsilon)
};

};	// end of package

/*

// Tests
// -----

fevals=0;
function y=ff(x) global(fevals) {y=exp(-0.5*x^2)/sqrt(2*pi); fevals+=length(x)};
format("Integral of ff = ``, `` evaluations.\n",integrate(ff,-10,10),fevals);

*/
