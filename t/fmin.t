package global(fmin,mean) {

/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

// Functions: fmin
// Purpose:   function minimization for N-dimensional problems
// Method:    Downhill simplex method ("amoeba method")
//            Does not use derivatives. Can also be used for 1D
//            problems, but is not very fast for them.
// Created:   9.3.1995
// Some bugs fixed 25.6.1996
// Reference: Numerical Recipes
	
function [P;y,iter]=amoeba(funk,tol)
// This is adapted from Numerical Recipes (1st ed.) amoeba function.
// The stopping criterion has been somewhat modified.
global(mean)
{
	alpha = 1;
	beta = 0.5;
	gamma = 2;
	[M,N] = size(P);
	itmax = 500*N;
	Pr = zeros(N); Prr=Pr; Pbar=Pr;
	y = map(funk,P,2);
	for (iter=0; iter<=itmax; iter++) {
		// i_lowest, i_highest and i_nexthighets are indices in 1..M
		[ymin,i_lowest] = min(y);
		[ymax,i_highest] = max(y);
		y1 = y;
		y1[i_highest] = ymin;	// at this moment dummy is min(y)
		[dummy,i_nexthighest] = max(y1);
		if (iter==0) inv_ytypical = 1/(abs(ymax) + abs(ymin));
		rtol = 2*abs(ymax-ymin)*inv_ytypical;
//		format("rtol=``, ymin=``, ymax=``, sz=``\n",rtol,ymin,ymax,max(mapmax(P,1)-mapmin(P,1)));
		if (rtol < tol) return;
		i = find((1:M)!=i_highest);
		Pbar = map(mean,P[i,:],1);
		Pr = (1+alpha)*Pbar - alpha*P[i_highest,:];
		yPr = funk(Pr);
		if (yPr <= ymin) {
			Prr = gamma*Pr + (1-gamma)*Pbar;
			yPrr = funk(Prr);
			if (yPrr < ymin) {
				P[i_highest,:] = Prr;
				y[i_highest] = yPrr;
			} else {
				P[i_highest,:] = Pr;
				y[i_highest] = yPr;
			};
		} else if (yPr >= y[i_nexthighest]) {
			if (yPr < ymax) {
				P[i_highest,:] = Pr;
				y[i_highest] = yPr;
			};
			Prr = beta*P[i_highest,:] + (1-beta)*Pbar;
			yPrr = funk(Prr);
			if (yPrr < y[i_highest]) {
				P[i_highest,:] = Prr;
				y[i_highest] = yPrr;
			} else {
				for (i=1; i<=M; i++) if (i!=i_lowest) {
					P[i,:] = 0.5*(P[i,:] + P[i_lowest,:]);
					y[i] = funk(P[i,:]);
				};
			};
		} else {
			P[i_highest,:] = Pr;
			y[i_highest] = yPr;
		};
	};
	format("*** fmin.t: Nelder-Mead polytope search exceeded maximum iteration count ``\n",itmax);
};

function [x,fm,iter] = fmin(f,x0; epsilon, scale)
/* x=fmin(f,x0) finds a local minimum x of function f near x0.
   [x,fm,iter] = fmin(f,x0) returns also the found minimum fm
   and the iteration count iter. The function f must return
   a real number when called with single vector argument.
   The vector length N must equal length(x0).

   fmin(f,x0,epsilon) uses epsilon as stop criterion, the
   default is the machine epsilon eps.
   fmin(f,x0,epsilon,scale) gives a problem-specific scale size,
   default is unity. */
{
	// The sum() operations are just in case the user supplies weird values
	// for epsilon or scale.
	if (!isfunction(f)) {format("*** fmin: f is not a function\n"); return};
	if (!isreal(x0)) {format("*** fmin: x0 not real\n"); return};
	if (rank(x0)!=1) {format("*** fmin: x0 not a vector\n"); return};
	N = length(x0);
	P = zeros(N+1,N);
	P[1,:] = x0;
	if (isdefined(scale) && isreal(scale)) lambda=sum(scale) else lambda=1.0;
	for (i=2; i<=N+1; i++) {
		P[i,:] = x0;
		P[i,i-1]-= lambda;
	};
	ff = zeros(N+1);
	if (isdefined(epsilon) && isreal(epsilon)) e=sum(epsilon) else e=eps;
	[P,ff,iter] = amoeba(f,e);
	x = P[1,:];
	fm = ff[1];
};

/*

// Tests
// -----
   
// 1D case
//function y=ff(x) {y=1-exp(-(x[1]^2))};
//[x,m,iter] = fmin(ff,#(2.0));

// 2D case
//function y=ff(x) {y=1-exp(-(x[1]^2+x[2]^2))};
//[x,m,iter] = fmin(ff,#(1.6,2.0));

// 3D case
//function y=ff(x) {y=5-exp(-x**x)};
//[x,m,iter] = fmin(ff,#(1.6,2.0,-0.9));

// 4D case
function y=ff(x) {y=1-exp(-x**x)};
[x,m,iter] = fmin(ff,#(1.6,2.0,-0.9,-1.1));

format("Minimum of ff at x=``, minimum value=``, `` iters.\n",x,m,iter);

*/

};
