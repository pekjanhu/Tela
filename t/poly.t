package
global(polyval,polyadd,polymul,roots)
{

/*
  A polynomial p(x) = p[1]*x^(n-1) + p[2]*x^(n-2) + ... + p[n-1]*x + p[n]
  is represented by vector #(p[1],...,p[n]). That is, the first element
  of the vector is the higher order coefficient.

  If p(x) = sum_{i=0}^N a[i]*x^i, a[0]=p[N+1], a[1]=p[N], ..., a[N]=p[1],
  in general a[i]=p[N+1-i].
*/

function y = polyval(p,x)
/* polyval(p,x) evaluates polynomial p at argument x.
   How polynonials are represented, see roots. */
{
	n = length(p);
	y = p[1];
	for (i=2; i<=n; i++)
		y = y*x + p[i];
};

function y = polyadd(p,q)
/* polyadd(p,q) forms the sum of two polynomials (which are represented
   in vector form).
   How polynonials are represented, see roots. */
{
	Lp = length(p);
	Lq = length(q);
	L = max(Lp,Lq);
	y = izeros(L) + 0*p[1] + 0*q[1];
	y[L-Lp+1:L] = p;
	y[L-Lq+1:L] = y[L-Lq+1:L] + q;
};

function y = polymul(p,q)
/* polymul(p,q) forms the product of two polynomials (which are represented
   in vector form).
   How polynonials are represented, see roots. */
{
	n = length(p)-1;
	m = length(q)-1;
	L = n+m+1;
	y = izeros(L) + 0*p[1] + 0*q[1];
	for (k=1; k<=L; k++) {
		i = max(0,k-1-m):min(k-1,n);
//		a[i]=p[n+1-i];
//		b[j]=q[m+1-j]; b[k-i]=q[m+1-k+i];
		y[L+1-k] = p[n+1-i]**q[m+1-k+1+i];
	};
};

function y = roots(p)
/* roots(p) returns the vector of zeros of polynomial p.
   It uses the companion matrix method.
   
   A polynomial p(x) = p[1]*x^(n-1) + p[2]*x^(n-2) + ... + p[n]
   is represented by vector #(p[1],...,p[n]). That is, the first
   element of the vector is the higher order coefficient

   See also: fsolve*/
{
	n = length(p);
	n1 = n - 1;
	p1 = p[1];
	A = zeros(n1,n1) + 0*p1;
	if (n > 2)  {
		A[n:n:n1^2] = 1;
		A[1,:] = -p[2:n]/p1;
	} else if (n == 2) {
		A[1,1] = -p[2]/p1;
	} else {
		y = #();
		return;
	};
	y = eig(A);
};

}
