/*
	C style comments are recognized
		/* They are even nested */
*/

{
	x = 23;
	a = -2.01;
	b = 0;
	c = 1;
	(x-2.3i)^2;		// comment, C++ style
	y = -134;		// comment
	x = -a+b-c+4;
	y = 4;
	if ( 1 ) {
		z = 5;
		z2 = 25i;
	} else if (2)
		z = 6
	else
		z = 7;
	i = 1;
	j = 4;
	imax = 10;
	while (x <= y && i && j) z = 0;
	for (i=0; i<imax; i++)
		disp(a+1+b^2);
	//continue;
	//break;
}

