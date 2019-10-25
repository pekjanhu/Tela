function f = fact(n) {	// The factorial function
	//dump();
	if (n <= 1)
		f = 1.0		// Make result always real
	else
		f = n*fact(n-1)
};

function f = fib(n) {
	if (n < 2)
		f = n
	else
		f = fib(n-1) + fib(n-2)
};

for (i=0; i<13; i++)
	disp fact(i);
//disp fact(150);

disp fib(10);
disp fib(16);

/*
function z=G(y) {
	z = 34;
	//dump();
};

function y=F(x) {
	y=100;
	//dump();
	y=G(x);
};

F(3.14);
*/
