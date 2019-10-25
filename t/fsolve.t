package global(fsolve) {

/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

// Functions: fsolve
// Purpose:   Root finding
// Method:    Brent's method for 1D problems, Newtons' method
//            for multidimensional problems.
//            The root must be bracketed for 1D case, and the method
//            is guaranteed to succeed. Multidimensional case can fail
//            if the initial guess is not good enough.
// Created:   17.3.1995
// Reference: Numerical Recipes

function [x,fx,iter] = zbrent(f,x1,x2,tol)
// One-dimensional root finding of f by Brent's method.
// Root must be bracketed in [x1,x2], i.e. f(x1) and f(x2) must have different signs.
// The input tolerance tol is the accuracy on x-axis.
// Outputs: x is the found root, fx equals f(x) and iter is the number of
// iterations carried out. The number of function evaluations is one larger
// than iter.
{
	itmax = 100;
	a = x1;
	b = x2;
	fa = f(a);
	fb = f(b);
	if (fa*fb > 0) {
		format("*** fsolve: root must be bracketed in 1D case\n");
		return;
	};
	fc = fb;
	for (iter=1; iter<=itmax; iter++) {
		if (fb*fc > 0) {
			c = a;
			fc = fa;
			d = b-a;
			e = d;
		};
		if (abs(fc) < abs(fb)) {
			a = b;
			b = c;
			c = a;
			fa = fb;
			fb = fc;
			fc = fa;
		};
		tol1 = 2*eps*abs(b) + 0.5*tol;
		xm = 0.5*(c-b);
		if (abs(xm) <= tol1 || fb == 0) {
			x = b;
			fx = fb;
			return;
		};
		if (abs(e) >= tol1 && abs(fa) > abs(fb)) {
			s = fb/fa;
			if (a == c) {
				p = 2*xm*s;
				q = 1-s;
			} else {
				q = fa/fc;
				r = fb/fc;
				p = s*(2*xm*q*(q-r) - (b-a)*(r-1));
				q = (q-1)*(r-1)*(s-1);
			};
			if (p > 0) q = -q;
			p = abs(p);
			if (2*p < min(3*xm*q - abs(tol1*q), abs(e*q))) {
				e = d;
				d = p/q;
			} else {
				d = xm;
				e = d;
			}
		} else {
			d = xm;
			e = d;
		};
		a = b;
		fa = fb;
		if (abs(d) > tol1)
			b+= d
		else
			b+= tol1*sign(xm);
		fb = f(b);
	};
	format("*** Brent method exceeded maximum iteration count ``\n",itmax);
};

function y = ddx(f,i,x)
// Numerical differentiation of f at x with respect to ith variable
{
	dxi = sqrt(eps);
	x2 = x;
	x2[i]+= dxi;
	x1 = x;
	x1[i]-= dxi;
	y = (f(x2) - f(x1))/(2*dxi);
};

function [x,fx,k] = mnewt(f,x0,tolf)
// (Multidimensional) Newton's method for solving f(x)==0.
// x0 is the initial guess and iteration stops when |f(x)| is smaller
// than tolf. On exit, fx=f(x) and k is the number of iteration steps.
// The number of function evaluations is 2*k+1.
{
	itmax = 100;
	x = x0;
	n = length(x0);
	alpha = zeros(n,n);
	for (k=1; k<=itmax; k++) {
		fx = f(x);
		for (j=1; j<=n; j++) alpha[:,j] = ddx(f,j,x);
		errf = sum(abs(fx));
		if (k==1) initerrf = errf;
		if (errf <= tolf) return;
		if (errf > 1E6*initerrf) {
			format("*** Newton's method seems to diverge\n");
			return;
		};
		x += linsolve(alpha,-fx);
	};
	format("*** Newton's method did not converge in `` iterations\n",k);
};

function [x,fx,iter] = fsolve(f,tol,x1;x2)
/* fsolve(f,tol,a,b) tries to solve f(x)==0 for x
   in the interval [a,b].
   fsolve(f,tol,x0) is for multidimensional problems and
   it tries to improve the initial guess x0 by Newton's
   method.
   [x,fx,iter] = fsolve(...) also returns the function
   value at the reached solution and the number of iteration
   steps taken.
   See also: roots.*/
{
	if (length(x1)==1 || isscalar(x1)) {
		if (isundefined(x2)) {
			format("*** fsolve: both x1 and x2 must be given in 1D problems\n");
			return;
		};
		[x,fx,iter] = zbrent(f,x1,x2,tol);
	} else {
		if (isdefined(x2)) format("fsolve warning: x2 is ignored in multidimensional problems\n");
		[x,fx,iter] = mnewt(f,x1,tol);
	};
};

};	// end of package

/*
// Tests
// -----
fevals=0;

//function y=ff(x) global {y=tanh(x); fevals++};
function y=ff(x) global {z=x[1]+1i*x[2]; w=tanh(z); y=#(Re(w),Im(w)); fevals++};

//[x0,ftol,iter]=fsolve(ff,eps,-10,20);
[x0,ftol,iter]=fsolve(ff,eps,#(0.6,0.7));
format("root at ``, ff()=``, iter=``, fevals=``\n",x0,ftol,iter,fevals);
*/
