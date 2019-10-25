package global(LinearProgramming)
{

EPS = 1e-12;
//testlp();

function [kp,bmax] = FindMaximum(a,mm,ll,nll,absflag)
// determines the maximum of those elements whose index is contained in list ll,
// either with or without taking the absolute value, as determined by absflag
{
	if (nll <= 0) {bmax = 0; return};	// no eligible columns
	k = 1:nll;
	if (absflag) {
		[dummy,maxpos] = max(abs(a[mm+1,ll[k]+1]));
		kp = ll[maxpos];
		bmax = a[mm+1,kp+1];
	} else {
		[bmax,maxpos] = max(a[mm+1,ll[k]+1]);
		kp = ll[maxpos];
	}
};

function ip = LocatePivot(a,m,n,kp)
global(EPS)
{
	ip = find(a[2:m+1,kp+1] < -EPS);
	if (length(ip) == 0) {ip = 0; return};
	ip = ip[1];
	/*
	ip = 0;
	for (i=1; i<=m; i++)
		if (a[i+1,kp+1] < -EPS) break;
	if (i > m) return;
	ip = i;
	*/
	q1 = -a[ip+1,1]/a[ip+1,kp+1];
	for (i=ip+1; i<=m; i++) {
		if (a[i+1,kp+1] < -EPS) {
			q = -a[i+1,1]/a[i+1,kp+1];
			if (q < q1) {
				ip = i;
				q1 = q;
			} else if (q == q1) {
				// degeneracy
				for (k=1; k<=n; k++) {
					qp = -a[ip+1,k+1]/a[ip+1,kp+1];
					q0 = -a[i+1, k+1]/a[i+1, kp+1];
					if (q0 != qp) break;
				};
				if (q0 < qp) ip = i;
			}
		}
	};
};

function [a;] = VarExchange(i1,k1,ip,kp)
{
	piv = 1.0/a[ip+1,kp+1];
	kk = 1:k1+1;
	kind = find(kk-1 != kp);
	ii = 1:i1+1;
	iind = find(ii-1 != ip);
	a[iind,kp+1]*= piv;
	a[iind,kind]-= a[ip+1+0*iind,kind]*a[iind,kp+1+0*kind];
	a[ip+1,kind]*= -piv;
	a[ip+1,kp+1] = piv;
};

function [a;icase,izrov,iposv] = SimplexAlgorithm(m,n,m1,m2,m3)
global(EPS)
{
	if (m != (m1+m2+m3)) {
		format("*** Bad input constraint counts in SimplexAlgorithm");
		return;
	};
	l1 = #(1:n,0);
	l2 = 1:m;
	l3 = izeros(m);
	l3[1:m2] = 1;
	iposv = izeros(m);
	nl1 = n;
	izrov = 1:n;
	nl2 = m;
	iposv = (1:m)+n;
	ir = 0;
	if (m2+m3) {
		ir = 1;
		for (k=1; k<=n+1; k++)
			a[m+2,k] = -sum(a[m1+2:m+1,k]);
		repeat
			[kp,bmax] = FindMaximum(a,m+1,l1,nl1,0);
			if (bmax <= EPS && a[m+2,1] < -EPS) {
				icase = -1;
				return;
			} else if (bmax <= EPS && a[m+2,1] <= EPS) {
				m12 = m1 + m2 + 1;
				if (m12 <= m) {
					for (ip=m12; ip<=m; ip++) {
						if (iposv[ip] == (ip+n)) {
							[kp,bmax] = FindMaximum(a,ip,l1,nl1,1);
							if (bmax > 0.0) goto one;
						}
					}
				};
				ir = 0;
				m12--;
				if (m1+1 <= m12)
					for (i=m1+1; i<=m12; i++)
						if (l3[i-m1] == 1)
							a[i+1,:] = -a[i+1,:];
				break;
			};
			[ip] = LocatePivot(a,m,n,kp);
			if (ip == 0) {
				icase = -1;
				return;
			};
label one;
		    [a] = VarExchange(m+1,n,ip,kp);
			if (iposv[ip] >= (n+m1+m2+1)) {
				for (k=1; k<=nl1; k++)
					if (l1[k] == kp) break;
				nl1--;
				l1[k:nl1] = l1[k+1:nl1+1];
				a[m+2,kp+1]++;
				a[:,kp+1] = -a[:,kp+1];
			} else {
				if (iposv[ip] >= n+m1+1) {
					kh = iposv[ip] - m1 - n;
					if (l3[kh]) {
						l3[kh]=0;
						a[m+2,kp+1]++;
						a[:,kp+1] = -a[:,kp+1];
					}
				}
			};
			is = izrov[kp];
			izrov[kp] = iposv[ip];
			iposv[ip] = is;
		until !ir;
	};
	while (1) {
		[kp,bmax] = FindMaximum(a,0,l1,nl1,0);
		if (bmax <= 0.0) {
			// successfull return
			icase = 0;
			return;
		};
		[ip] = LocatePivot(a,m,n,kp);
		if (ip == 0) {
			// unbounded objective function
			icase = 1;
			return;
		};
		[a] = VarExchange(m,n,ip,kp);
		is = izrov[kp];
		izrov[kp] = iposv[ip];
		iposv[ip] = is;
	}
};

function x = LinearProgramming(c,A,b;m123)
/* LinearProgramming(c,A,b) returns the vector x which maximizes
   c**x while x>=0 and A**x <= b. If no solution exists, returns void (:).
   If the solution is unbounded, returns a vector with all elements infinite.

   LinearProgramming(c,A,b,#(m1,m2,m3)) species that first m1 conditions are
   of type A**x <= b, the next m2 conditions of type A**x >= b and the last m3
   conditions A**x == b (the respective parts of A and b are considered in each
   case). The default thus corresponds to m1=M, m2=0,m3=0 where M=length(b).
   In all cases m1+m2+m3 must be equal to M.*/
{
	if (rank(c)!=1 || rank(b) != 1) {
		format("*** LinearProgramming: c or b not vector\n");
		return;
	};
	N = length(c);
	x = zeros(N);
	M = length(b);
	if (any(b < 0)) {format("*** LinearProgramming: b contains negative values\n"); return};
	[mm,nn] = size(A);
	if (M!=mm || N!=nn) {
		format("*** LinearProgramming: size(A)=`` does not match with lengths of c or b\n",size(A));
		return;
	};
	a = zeros(M+2,N+1);
	if (isdefined(m123)) {
		if (rank(m123) != 1 || length(m123)!=3 || !isint(m123)) {
			format("*** LinearProgramming: m123=`` is not int 3-vector\n",m123);
			return;
		};
		if (sum(m123) != M) {
			format("*** LinearProgramming: m1+m2+m3 does not match M\n");
			return;
		};
		m1 = m123[1];
		m2 = m123[2];
		m3 = m123[3];
	} else {
		m1 = M;
		m2 = 0;
		m3 = 0;
	};
	// Scalings
	alpha = mean(abs(A));
	beta = mean(abs(b));
	if (alpha <= 0 || beta <= 0) {
		alpha = 1;
		beta = 1;
	};
	xscale = beta/alpha;
	a[1,2:N+1] = c/mean(abs(c));
	for (j=1; j<=M; j++) {
		a[j+1,1] = b[j]/beta;
		a[j+1,2:N+1] = -A[j,1:N]/alpha;
	};
	[a,icase,izrov,iposv] = SimplexAlgorithm(M,N,m1,m2,m3);
	if (icase == 0) {
		// solution ok
		ind = find(iposv <= N);
		x[iposv[ind]] = a[ind+1,1];
		// scale back:
		x*= xscale;
	} else if (icase < 0) {
		// no solution exists
		x = :;
	} else {
		// unbounded solution
		x[1:N] = 1/0;		// Inf
	}
};

	
};		// end of package

/*

function testlp()
{
	
	A = #(1,0,2,0;
		  0,2,0,-7;
		  0,1,-1,2;
		  1,1,1,1);
	b = #(740,0,0.5,9);
	c = #(1,1,3,-0.5);
	m1 = 2;
	m2 = 1;
	m3 = 1;
	cnt = 1000;
	/*
	srand(123124);
	A = rand(60,60);
	b = rand(60);
	c = rand(60);
	m1 = 60; m2 = 0; m3 = 0;
	cnt = 100;
	*/
	p1 = perf();
	for (i=1; i<=cnt; i++) x = LinearProgramming(c,A,b,#(m1,m2,m3));
	p2 = perf();
	format("`` seconds, `` Mflops, solution = ``\n",cputime(p2-p1)/cnt,Mflops(p2-p1),x);
};

*/

