{
function result=myfunc(x,z)
//local
//local(y,a,b,c)
{
/*
	y = 0.9;
	a = 2i;
	b = -3.14;
	c = 4;
	d = 5/2;
	disp(d);
	y = a + 1;
	y = 2*a + b;
	y = a + b + c + d;
	disp(y);
	y = -((a+1)^2 - (a-1)^2 + b^4);
	y = (y+b)*(y-b);
	disp(y);

	if (0)
		disp(2222)
	else if (1)
		disp(6666)
	else
		disp(55);

	i = 0;
	while (i < 4) {
		disp(i);
		i = i+1;
	};

	for (i=0; i<10; i++) disp(i);
	for (i=9; i>=0; i--) disp(i);
*/

	i = 0;
	repeat
		disp(i);
		i++;
	until i>=10;

/*
	i=0; j=1;
	if (!(i == 0 && j == 0))
		disp(44);
*/
/*
	disp((-2)^(-2));
	disp((-2)^(-1));
	disp((-2)^(0));
	disp((-2)^1);
	disp((-2)^2);
	disp((-2)^3);
	disp((-2)^4);
	disp((-2)^9);
	disp(333);
*/

	$x = 1;
	b = -3;
	disp((-1:$x/2:b^2));

	imax=5; jmax=5;
	M = zeros(imax,jmax);
	V = zeros(imax);
	for (i=2; i<=imax+1; i++) for (j=1; j<=jmax; j++) {
		M[i-1,j] = 1.0/(1+i+j);
	};
	V[2] = 11+12+13;
	disp(M[2,3]);
};

r=myfunc(1,2);

}

