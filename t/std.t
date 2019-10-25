package
global(input,int2str,compile,mean,stddev,Tr,FFT2,invFFT2,fib,
	   docview,kron,fix,isfile,nrange,strstr,export_ASCII,interp)
{

/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */
	
function y = input(;s)
// y = input() returns a value entered by user.
// y = input("prompt") writes a prompt before asking.
local(str)
{
	if (isstr(s)) format(s);
	str = input_string();
	if (streq(str,"")) y = : else y = evalexpr(str)
};

function s = int2str(x)
// int2str(x) converts an integer x to a string.
{
	s = sformat("``",x)
};

function [] = compile(...)
// compile("file1.t",...) compiles given t-file(s) to object files.
{
	for (i=1; i<=Nargin(); i++) {
		tfile = argin(i);
		L = length(tfile);
		if (tfile[L]=='t' && tfile[L-1]=='.')
			ctfile = #(tfile[1:L-1],"ct")
		else
			ctfile = #(tfile,".ct");
		t2ct(tfile);
		system(#("telakka -c -O1 ",ctfile));
	}
};

function y = mean(x; d)
// mean(x) computes the arithmetic mean of a numeric array x.
// mean(x,d) maps the operation only along d'th dimension.
{
	if (isdefined(d)) {
		s = size(x);
		y = sum(x,d)/s[d];
	} else
		y = sum(x)/length(x)
};

function y = stddev(x;d)
// stddev(x) computes the standard deviation of a numeric array x.
// It divides by N, not by N-1 as some other implementations.
// stddev(x,d) maps the operation only along d'th dimension.
// See also: mean.
{
	if (isdefined(d))
		y = sqrt( mean(x^2,d) - mean(x,d)^2 )
	else
		y = sqrt(mean( (x-mean(x))^2 ))
};

function y = Tr(x)
// Tr(x) returns the trace (diagonal sum) of a matrix.
{
	y = sum(diag(x))
};

function f = FFT2(u)
// FFT2(u) gives a 2D discrete Fourier transform of 2D array u.
// See also: invFFT2, FFT.
{
	f=FFT(FFT(u,1),2)
};

function f = invFFT2(u)
// invFFT2(u) gives the inverse 2D Fourier transform of u.
// See also: FFT2.
{
	f=invFFT(invFFT(u,1),2)
};

function f = fib(n)
// fib(n) returns the nth Fibonacci number.
// Recursive (==> very slow) implementation, useful for
// benchmarking only!
{
	if (n<2) f=n else f=fib(n-1)+fib(n-2)
};

function docview()
// docview() is an interactive program for viewing various
// kinds of Tela documentation.
local(directory,choicViewing,choiceDoc,editor,env)
{
	directory = "/usr/local/lib/tela";
	editor = "emacs";
	pager = "less -Ms";
	env = getenv("EDITOR");
	if (isstr(env)) editor = env;
	env = getenv("PAGER");
	if (isstr(env)) pager = env;
	
	format("Viewing Tela documentation:\n");
	choiceViewing = smenu("Choose viewing method",
						 "ASCII",
						 "HTML/Xmosaic (requires X)",
						 "HTML/Lynx",
						 "Xdvi (requires X)",
						 "Xemacs (requires X)",
						 "Cancel");
	if (strstarteq(choiceViewing,"Cancel")) return;
	choiceDoc = smenu("Choose document to view",
					 "Basic help file",
					 "Builtin functions alphabetically sorted",
					 "Builtin functions sorted by sections",
					 "Cancel");
	if (strstarteq(choiceDoc,"Cancel")) return;
	
	if (strstarteq(choiceDoc,"Basic help"))
		file = "telahelp"
	else if (strstarteq(choiceDoc,"Builtin functions alpha"))
		file = "telafuncs"
	else if (strstarteq(choiceDoc,"Builtin functions sorted"))
		file = "telafuncsSectioned";

	if (strstarteq(choiceViewing,"ASCII"))
		system(#(pager," ",directory,"/",file,".txt"))
	else if (strstarteq(choiceViewing,"HTML/X"))
		system(#("cd ",directory,"/html; xmosaic ",file,".html&"))
	else if (strstarteq(choiceViewing,"HTML/Lynx"))
		system(#("cd ",directory,"/html; lynx ",file,".html"))
	else if (strstarteq(choiceViewing,"Xdvi"))
		system(#("xdvi -geometry 847x570+0+0 -s 3 ",directory,"/",file,".dvi&"))
	else if (strstarteq(choiceViewing,"Xemacs"))
		system(#(editor," ",directory,"/",file,".txt&"));
};

function K = kron(A,B)
//  kron(X,Y) is the Kronecker tensor product of X and Y.
//	The result is a large matrix formed by taking all possible
//	products between the elements of X and those of Y.
//	For example, if X is 2 by 3, then KRON(X,Y) is
//
//	  #( X[1,1]*Y,  X[1,2]*Y,  X[1,3]*Y;
//	     X[2,1]*Y,  X[2,2]*Y,  X[2,3]*Y )
//local(ma,na, mb,nb,i,j,jk,ik)
{

	if (rank(A)!=2 || rank(B)!=2) {K=0; return};
	
	[ma,na] = size(A);
	[mb,nb] = size(B);
	La = length(A);
	Lb = length(B);
	m = ma*mb;
	n = na*nb;

	if (isint(A) && isint(B))
		K = izeros(m,n)
	else if (iscomplex(A) || iscomplex(B))
		K = czeros(m,n)
	else
		K = zeros(m,n);

	if (La <= Lb) {
		for (i=1; i<=ma; i++) {
			ik = 1+(i-1)*mb:i*mb;
			for (j=1; j<=na; j++) {
				jk = 1+(j-1)*nb:j*nb;
				K[ik,jk] = A[i,j]*B
			}
		}
	} else {
		for (i=1; i<=mb; i++) {
			ik=i:mb:(ma-1)*mb+i;
			for (j=1; j<=nb; j++) {
				jk = j:nb:(na-1)*nb+j;
				K[ik,jk] = A*B[i,j];
			}
		}
	}

};

/*function y = where(c,a,b)
// where(c,a,b) returns a where c is nonzero and b where c is zero.
// Usually the arguments are arrays, and if they are, they must be
// of similar dimensions.
{
	if (isarray(c)) {
		y = c + 0*a + 0*b;
		yes = find(c);
		no = find(!c);
		if (isarray(a))
			y[yes] = a[yes]
		else
			y[yes] = a;
		if (isarray(b))
			y[no] = b[no]
		else
			y[no] = b;
	} else {
		if (c) y=a else y=b
	}
};*/

function y = fix(x)
// fix(x) truncates x toward zero.
{
	y = sign(x)*floor(abs(x))
};

function y = isfile(f)
// isfile("file") tests whether "file" exists in the current directory.
{
	if (!isstring(f) || length(f)==0) {y=0; return};
	y = length(run(sformat("if test -f ``; then echo 1; fi",f)))
};

//function y = limit(x,a,b)
// THIS IS NOW BUILTIN FUNCTION
// limit(x,a,b) limits x to the range [a,b].
//{
//	y = min(b,max(x,a))
//};

function y = nrange(a,b,n)
/* y=nrange(a,b,n) returns a range of values from a to b of length n.
   The first y value is equal to a and the last y is equal to b.
   If you want to ensure that the generated range has exactly n components,
   it is safer to use nrange than the builtin a:s:b construct. */
{
	y = a + (0:n-1)*(b-a)/(n-1)
};

function y = strstr(haystack,needle)
/* strstr(haystack,needle) find the first occurrence of string needle
   in the string haystack. It returns the index of the first character
   in haystack, or 0 if not found.*/
{
	if (!isstring(haystack) || !isstring(needle)) {y=0; return};
	Lhaystack = length(haystack);
	Lneedle = length(needle);
	if (Lneedle == 0) {y=0; return};
	L = Lhaystack - Lneedle + 1;
	for (i=1; i<=L; i++) {
		if (streq(needle,haystack[i:i+Lneedle-1])) {
			y = i;
			return;
		}
	};
	y = 0;
};

function export_ASCII(fn,x)
/* export_ASCII("fn",x) writes variable x to an ASCII file "fn".
   It works for arrays and scalars. If the rank is 1, the components
   are written one by line, if the rank is 2, they are written in
   matrix layout. For higher ranks, the D-format is used (see import1).
   For scalars, the value is simply written to the file, followed
   by a newline. For array objects, import1 can read back the result
   saved by export_ASCII.*/
{
	fp = fopen(fn,"w");
	if (isarray(x)) {
		if (rank(x) == 1 && isreal(x)) {
			n = length(x);
			for (i=1; i<=n; i++) fformat(fp," ``\n",x[i]);
		} else if (rank(x) == 2 && isreal(x)) {
			[nx,ny] = size(x);
			for (i=1; i<=nx; i++) {
				for (j=1; j<=ny; j++) fformat(fp," ``",x[i,j]);
				fformat(fp,"\n");
			};
		} else {
			fformat(fp,"D=``",rank(x));
			if (!isreal(x)) fformat(fp,"c") else if (isint(x)) fformat(fp,"i");
			s = size(x);
			for (d=1; d<=rank(x); d++) fformat(fp," ``",s[d]);
			fformat(fp,"\n");
			n = length(x);
			for (i=1; i<=n; i++) fformat(fp,"``\n",x[i]);
		};
	} else {
		fformat(fp,"``\n",x);
	};
	fclose(fp);
};

function Y = interp(x,y,X; order)
/* Y = interp(x,y,X) does linear interpolation at X on data specified by
   vectors x and y (x must be monotonic). The third argument X can be
   either scalar or array, and the result Y has the same size as X.
   NaN (0/0) is set for points which are out of range.

   Y = interp(x,y,X,3) uses cubic interpolation. The call form
   interp(x,y,X,1) for linear interpolation is also supported
   and is the same as interp(x,y,X).

   Also the case where y is a matrix is supported, but with the
   following restrictions: X must be vector or scalar and y must
   not be complex. Y will then be a NxM matrix where N=length(X)
   and M the second dimension of y.
*/
{
	i0 = bsearch(x,X);
	i = max(1,i0);
	if (isint(order)) ordr = order else ordr = 1;
	if (ordr != 1 && ordr != 3) {format("interp: order given as `` but only 1 or 3 supported, order=1 assumed\n",ordr); ordr=1};
	if (ordr == 1) {
		x2 = x[i+1];
		x1 = x[i];
		norm = 1/(x2-x1);
		a = (x2-X)*norm;
		b = (X-x1)*norm;
//		Y = ((x2-X)*y<[i]> + (X-x1)*y<[i+1]>)/(x2-x1);
		if (rank(y) == 2) {
			[dum,S] = size(y);
			Y = zeros(length(a),S);
			for (s=1; s<=S; s++) Y[:,s] = a*y[i,s] + b*y[i+1,s];
		} else {
			Y = a*y[i] + b*y[i+1];
		};
	} else {
		ii = limit(i,2,length(x)-2);
		x1 = x[ii-1];
		x2 = x[ii];
		x3 = x[ii+1];
		x4 = x[ii+2];
		aux = (X-x3)*(X-x4);
		C1 = (X-x2)*aux/((x1-x2)*(x1-x3)*(x1-x4));
		C2 = (X-x1)*aux/((x2-x1)*(x2-x3)*(x2-x4));
		aux = (X-x1)*(X-x2);
		C3 = aux*(X-x4)/((x3-x1)*(x3-x2)*(x3-x4));
		C4 = aux*(X-x3)/((x1-x4)*(x2-x4)*(x4-x3));
		if (rank(y) == 2) {
			[dum,S] = size(y);
			Y = zeros(length(aux),S);
			for (s=1; s<=S; s++)
				Y[:,s] = C1*y[ii-1,s] + C2*y[ii,s] + C3*y[ii+1,s] + C4*y[ii+2,s];
		} else {
			Y = C1*y[ii-1] + C2*y[ii] + C3*y[ii+1] + C4*y[ii+2];
		};
	};
	if (isarray(X)) {
		badind = find(i0 == 0);
		if (rank(y) == 2) {
			Y[badind,:] = 0/0;
		} else {
			Y[badind] = 0/0;
		};
	} else {
		if (i0 == 0) Y = 0/0;
	};
};

};
