package global(fitline,fitlinear,fitnl) {

/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

// Functions: fitline, fitlinear,fitnl
// Purpose:   Fitting (regression) to straight line and to linear
//            combination of arbitrary basis functions.
//            Nonlinear fitting (fitnl) using Levenberg-Marquardt.
// Created:   20.3.1995
// Modified:  27.3.1995
// Reference: Numerical Recipes (1st. edition)

function [a,b,sa,sb,chi2] = fitline(x,y;s)
/* [a,b] = fitline(x,y) does a least-squares fit to data
   points (x,y) such that a+b*x is as close to y as possible.
   [a,b,sa,sb,chi2] = fitline(x,y,s) gives the standard
   deviations s of y as input and returns the standard deviations
   of a and b in sa and sb, and the chi-square in chi2:

   chi2 = sum(((y-a-b*x)/s)^2)

   The probability Q that such chi2 would result by the Gaussian
   fluctuations of y only is given by

   Q = 1 - GammaP((n-2)/2, chi2/2)

   where n=length(x) and GammaP(a,x) is the incomplete Gamma
   function. This Q is not computed by fitline however.
   See also: fitlinear, fitnl. */
{
	n = length(x);
	mwt = isdefined(s);
	if (mwt) {
		wt = 1/s^2;
		ss = sum(wt);
		sx = sum(x*wt);
		sy = sum(y*wt);
	}else {
		ss = n;
		sx = sum(x);
		sy = sum(y);
	};
	sxoverss = sx/ss;
	if (mwt) {
		t = (x - sxoverss)/s;
		b = sum(t*y/s);
	} else {
		t = x - sxoverss;
		b = sum(t*y);
	};
	st2 = sum(t^2);
	b/= st2;
	a = (sy - sx*b)/ss;
	sa = sqrt((1+sx^2/(ss*st2))/ss);
	sb = sqrt(1/st2);
	if (mwt) {
		chi2v = (y-a-b*x)/s;
		chi2 = chi2v**chi2v;
//		Q = GammaQ(0.5*(n-2),0.5*chi2);
	} else {
		chi2v = y-a-b*x;
		chi2 = chi2v**chi2v;
//		Q = 1;
		sdat = sqrt(chi2/(n-2));
		sa*= sdat;
		sb*= sdat;
	};
};


/* A = U**S**V' where U and V are unitary and S is diagonal.
   Want to solve A**x == b. Now U^(-1)==U', V^(-1)==V'.
   U**S**V'**x == b ==> S**V'**x == U'**b
   ==> V'**x == (1/S)**U'**b
   ==> x == V**(1/S)**U'**b

   X[j,i] = X_j(x[i])
   The fit function is f(x) = sum(a[j]*X_j(x),j), f(x[i]) = sum(a[j]*X[j,i],j) = (X.'**a)[i]
*/

function [a,chi2,C] = fitlinear(x,y,X;sigma)
/* [a] = fitlinear(x,y,X) performs general linear least-squares
   fit (linear regression) to data points (x,y). The matrix X
   must have been initialized so that if u_j is the jth basis
   function, then X[j,i] = u_j(x[i]). The output vector a contains
   the best-fit coefficients such that X.'**a approximates y
   in the least-squres sense.

   [a,chi2,C] = fitlinear(x,y,X,sigma) is the most complete call
   form. It specifies the standard deviations of the data points
   (sigma) and returns the chi-square scalar chi2 and the
   covariance matrix C. See also: fitnl, fitline.*/
{
	tol = 1E-5;
	if (isdefined(sigma)) invsigma=1/sigma else invsigma=1;
	n1 = length(x);
	[m,n] = size(X);
	if (n1!=n) {format("*** fitlinear: length(x) must equal the second dimension of X\n"); return};
	if (m>n) {format("*** fitlinear: number of basis function must not exceed number of data points\n"); return};
    b = y*invsigma;
	if (isdefined(sigma)) {
		invsigmamat = zeros(m,length(invsigma));
		for (i=1; i<=m; i++) invsigmamat[i,:] = invsigma;
		A = transpose(X*invsigmamat);
	} else {
		A = transpose(X);
	};
	[U,W,V] = SVD(A);
//	format("A: ``, U: ``, W: ``, V: ``\n",size(A), size(U),size(W),size(V));
	w = diag(W);
	thresh = tol*max(w);
	invw = zeros(m);
	ind = find(w>=thresh);
	invw[ind] = 1/w[ind];
	invW = zeros(size(W));
	invW<[1:m,1:m]> = invw;
	a = V**transpose(invW)**transpose(U)**b;
	chi2v = (y-transpose(X)**a)*invsigma;
	chi2 = chi2v**chi2v;
	C = V**diag(invw^2)**transpose(V);
};

function y = ddx(f,i,x,a)
// Numerical differentiation of f(x,a) at a with respect to ith component of a
{
	dai = sqrt(eps);
	a2 = a;
	a2[i]+= dai;
	a1 = a;
	a1[i]-= dai;
	y = (f(x,a2) - f(x,a1))/(2*dai);
};

function [a;chi2,C,stda] = fitnl(x,y,fn;sigma,ind1,tr)
/* [a] = fitnl(x,y,fn) fits nonlinearly the model function fn
   to data points (x,y). The fit parameter vector a, which must
   contain an initial guess on entry, is fed to the model as
   y=fn(x,a). The Levenberg-Marquardt method is used.

   [a,chi2,C,stda] = fitnl(x,y,fn,sigma,ind,tr) specifies the
   standard deviation vector sigma and an index vector of varied
   parameters. Defaults are sigma=1, ind=1:length(a). The ind
   vector is useful in freezing some of the parameter during
   experimentation. Informative messages are displayed if tr is
   given as 1. chi2 is the minimum chi-square scalar found,
   C is the covariance matrix and stda is an estimate for standard
   deviations of fitted parameters, computed as stda=sqrt(diag(C)).
   If the measurement errors are not normally distributed, stda
   is unreliable. See also: fitlinear. */
{
	if (isdefined(sigma)) invsigma=1/sigma else invsigma=1;
	chi2v = (y-fn(x,a))*invsigma;
	chi2 = chi2v**chi2v;
	lambda = 0.0001;
	if (isdefined(ind1)) ind=ind1 else ind=1:length(a);
	M = length(ind);
	alpha = zeros(M,M);
	beta = zeros(M);
	success = 0;
	repeat
		cnt = 0;
		repeat
			lambda*= 10;
	        for (k=1; k<=M; k++) beta[k] = (y-fn(x,a)*invsigma^2)**ddx(fn,ind[k],x,a);
	        for (k=1; k<=M; k++) for (l=1; l<=M; l++)
		    	alpha[k,l] = ddx(fn,ind[k],x,a)**(ddx(fn,ind[l],x,a)*invsigma^2);
	        alpha<[1:M,1:M]> *= 1 + lambda;
		    da = linsolve(alpha,beta);
	        anew = a;
	        anew[ind] += da;
		    chi2v = (y-fn(x,anew))*invsigma;
		    chi2new = chi2v**chi2v;
	        cnt++;
	        if (cnt > 18) {format("*** fitnl: failed to converge\n"); return};
	    until chi2new < chi2;
	    if (isdefined(tr)) if (tr) {if (cnt > 1) format("Backward `` steps\n",cnt-1)};
	    lambda*= 0.01;
	    a = anew;
		chi2old = chi2;
		chi2 = chi2new;
        if (isdefined(tr)) if (tr) format("chi2=``, a=``, lambda=``\n",chi2,a,lambda);
	    oldsuccess = success;
	    success = (chi2old - chi2 < 0.1 || (chi2old - chi2)/chi2 < 1E-3);
	until success && oldsuccess;
	lambda = 0;
	for (k=1; k<=M; k++) for (l=1; l<=M; l++)
    	alpha[k,l] = ddx(fn,ind[k],x,a)**(ddx(fn,ind[l],x,a)*invsigma^2);
	C = inv(alpha);
	stda = sqrt(diag(C));
};

};

/*

// Tests
// -----
/*
// Test fitlinear
x = 0:0.05:10;
y = 0.55*sin(x) + 0.96*cos(x) + (rand(length(x))-0.5);
X = zeros(6,length(x));
X[1,:] = sin(x);
X[2,:] = cos(x);
X[3,:] = sin(2*x);
X[4,:] = cos(2*x);
X[5,:] = sin(3*x);
X[6,:] = cos(3*x);

[a,chi2,C] = fitlinear(x,y,X);
a;
plot(x,y,x,X.'**a);
*/

// Test fitlinear
[x,y] = grid(0:0.05:1, 0:0.05:1);
[n,m] = size(x);
f = 0.55*sin(x) + 0.96*cos(y) + (rand(n,m)-0.5);
X = zeros(4,length(x));
X[1,:] = flatten(sin(x));
X[2,:] = flatten(cos(x));
X[3,:] = flatten(sin(y));
X[4,:] = flatten(cos(y));
[a,chi2,C] = fitlinear(flatten(x),flatten(f),X);
a;

// Test nlfit
x = 0:0.05:10;
function y = model(x,a) {
	y = a[1]*exp(-((x-a[2])/a[3])^2) + a[4]*exp(-((x-a[5])/a[6])^2);
};
a = #(1.5,4,1.5, 1.2,6.5,1.3);
y = model(x,a) + (rand(length(x))-0.5);
a+= 0.25*(rand(length(a))-0.5);
[a,chi2,C,stda] = fitnl(x,y,model,1.4,1:6,1);
format("a=``, stddev(a)=``\n",a,stda);
plot(x,y,x,model(x,a));

*/
