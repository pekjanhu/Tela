function f = fib(n) {
	//cnt++;
	if (n < 2)
		f = n
	else
		f = fib(n-1) + fib(n-2)
};

function f = g(n) local(a,b,c) {f=n};

cnt=0;
t1=cputime();
f=fib(20);
t2=cputime();
disp f;
disp t2-t1;
//disp cnt;

// Measure empty loop cputime
t1=cputime();
for (i=0; i<21891; i++);
t2=cputime();
emptytime=t2-t1;

t1=cputime();
for (i=0; i<21891; i++) y=g(i);
t2=cputime();
disp t2-t1 - emptytime;

