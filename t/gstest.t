// gstest - benchmark for gather/scatter operations
imax=40;
jmax=40;
nits=500;
dx=0.1;
A=zeros(imax,jmax);
B=A;
format("Starting benchmark for gather/scatter ...\n");
t1=cputime();
for (t=0; t<nits; t++)
  B[2:imax-1,2:jmax-1] = (A[3:imax,2:jmax-1]-A[1:imax-2,2:jmax-1])/(2*dx);
  //B = (A+A)*(2*dx);
t2=cputime();
flops = nits*2*(imax-2)*(jmax-2);
time = t2-t1;
format("time = `` sec\n",time);
mflops = 1E-6*flops/(t2-t1);
format("`` Mflops\n",mflops);

