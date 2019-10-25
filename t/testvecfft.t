/*
   testvecfft.t - test various Tela FFT routines.
   The results of FFT functions are compared with explicit summed series.
   You should run this test to validate the correct working of your
   Tela installation.
   PJ 9.1.1995
*/

function testvecfft1(funcname,expr,n;imagflag)
global
{
	if (isdefined(imagflag))
		x = rand(n)-0.5 + 1i*(rand(n)-0.5)
	else
		x = rand(n)-0.5;
	xx = #(x; x; x; x);
	eval(#("ff1 = ",funcname,"(xx,2);"));
	eval(#("ff1_trans = transpose(",funcname,"(transpose(xx),1));"));
	tperr = abs(ff1 - ff1_trans);
	if (any(tperr>0)) {format("*** n=``: transposed `` differed by ``\n",n,funcname,max(tperr))};
	f1 = ff1[1,:];
	if (any(ff1[2,:] != f1)) {format("*** row 2 mismatch with ``\n",funcname)};
	if (any(ff1[3,:] != f1)) {format("*** row 3 mismatch with ``\n",funcname)};
	if (any(ff1[4,:] != f1)) {format("*** row 4 mismatch with ``\n",funcname)};
	k = 0:n-1;
	f2 = czeros(n);
	s = #("for (j=1; j<=n; j++)\n",
		  "f2[j] = ",expr, ";");
	eval(s);
	err = max(abs(f1-f2))*n/sum(abs(f1));
	if (isdefined(imagflag))
		format("Testing complex ``(n=``) with 4 identical rows...",funcname,n)
	else
		format("Testing ``(n=``) with 4 identical rows...",funcname,n);
	if (err > errlimit) {
		format("*** relative error = ``\n",err);
		relative_errors = #(relative_errors,err);
	} else
		format("ok.\n");
};

function testfft(funcname,expr,n) {testvecfft1(funcname,expr,n); testvecfft1(funcname,expr,n,1)};

ns = #(2,3,4,5,7,8,16,17,30,31,100);
relative_errors = #();

format("Testing vectorized FFT functions using random input data of several lengths.\n");
for (p=1; p<=length(ns); p++) {

	n = ns[p];
	errlimit = 6*n*eps;

	testfft("FFT","sum(x*exp(-(j-1)*(0:n-1)*2i*pi/n))",n);
	testfft("invFFT","(1/n)*sum(x*exp((j-1)*(0:n-1)*2i*pi/n))",n);

	testfft("sinFFT","2*sum(x*sin((1:n)*j*pi/(n+1)))",n);
	testfft("invsinFFT","(2/(2*n+2))*sum(x*sin((1:n)*j*pi/(n+1)))",n);

	testfft("cosFFT","x[1]-(-1)^j*x[n]+2*sum(x[2:n-1]*cos((1:n-2)*(j-1)*pi/(n-1)))",n);
	testfft("invcosFFT","(x[1]-(-1)^j*x[n]+2*sum(x[2:n-1]*cos((1:n-2)*(j-1)*pi/(n-1))))/(2*n-2)",n);
	errlimit*= 3;
	
	testfft("sinqFFT","(-1)^(j-1)*x[n]+sum(2*x[1:n-1]*sin((2*j-1)*(1:n-1)*pi/(2*n)))",n);
	testfft("invsinqFFT","(1/n)*sum(x*sin((2*(1:n)-1)*j*pi/(2*n)))",n);
	testfft("cosqFFT","x[1]+sum(2*x[2:n]*cos((2*j-1)*(1:n-1)*pi/(2*n)))",n);
	testfft("invcosqFFT","(1/n)*sum(x*cos((2*(1:n)-1)*(j-1)*pi/(2*n)))",n);

};

if (length(relative_errors)>0) {
	format("Relative error was reported in `` cases, but this is not serious\n",length(relative_errors));
	format("if the error displayed was small, e.g. less than 1E-10. If all tests reported\n");
	format("the relative error, however, there may be something wrong with your floating\n");
	format("point hardware or with the accuracy of the C library trigonometric functions.\n");
	format("The maximum relative error was ``.\n",max(relative_errors));
};
