package {

// Run some Matlab style benchmarks:
// 'loop' and 'LU decomposition'.

// The problem size, n, for each task has been chosen 
// so that the task takes about one second on a SPARC-2.

ntimes = 10;

// Loops

format("The 'Loops' benchmark...\n");
n = 375;
A = 0;
r = 1;
tic();
for (k=1; k<=ntimes; k++) {
	for (j=1; j<=n; j++) {
		A = 0;
		r = (pi*r) mod 1;
		m = floor(100*r);
		A = zeros(m,m);
	}
};
tLoops = toc();

format("The 'Loops' benchmark with integer array...\n");
n = 375;
A = 0;
r = 1;
tic();
for (k=1; k<=ntimes; k++) {
	for (j=1; j<=n; j++) {
		A = 0;
		r = (pi*r) mod 1;
		m = floor(100*r);
		A = izeros(m,m);
	}
};
tLoopsInteger = toc();

// LU

format("The 'LU' benchmark\n");
n = 167;
A = rand(n,n);
tic();
for (k=1; k<=ntimes; k++) B=LU(A);
tLU = toc();

format("'Loops' benchmark:         `` sec.\n",tLoops);
format("'Integer Loops' benchmark: `` sec.\n",tLoopsInteger);
format("'LU' benchmark:            `` sec.\n",tLU);

};

