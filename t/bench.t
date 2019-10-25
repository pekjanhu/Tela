package {

function ForBench()
local (imax,jmax,N,A,n,i,j)
{
	imax = 100;
	jmax = 100;
	N = 50;

	A = zeros(imax,jmax);

	for (n=0; n<N; n++)
		for (i=1; i<=imax; i++) for (j=1; j<=jmax; j++)
			A[i,j] = 1/(1+i+j);
};

function MipsBench()
local (nits,b,i)
{
	nits = 1000000;
	b = 0;
	for (i=0; i<nits; i++) {
		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();
		nop(); nop(); nop(); nop(); nop();
	}
};

function VCopyBench()
local (N,nits,a,i,b)
{
	N = 1000;
	nits = 100000;
	a = 1:N;
	for (i=0; i<nits; i++)
		b = a;
};

function VAddBench()
local (N,nits,a,i,b)
{
	N = 1000;
	nits = 100000;
	a = 1.0:N;
	for (i=0; i<nits; i++)
		b = a+11.0;
};

function VIAddBench()
local (N,nits,a,i,b)
{
	N = 1000;
	nits = 100000;
	a = 1:N;
	for (i=0; i<nits; i++)
		b = a+11;
};

function VMulBench()
local (N,nits,a,i,b)
{
	N = 1000;
	nits = 100000;
	a = 1.0:N;
	for (i=0; i<nits; i++)
		b = 11.0*a;
};

format("forbench:     scalar for-loops with 1/(i+j+1) as innermost computation ..\n");
t1=cputime();
ForBench();
for_time = cputime()-t1;

format("mipsbench:    30 NOP instructions inside for-loop ..\n");
t1=cputime();
MipsBench();
mips_time = cputime()-t1;

format("vcopybench:   vector copy ..\n");
t1=cputime();
VCopyBench();
vcopy_time = cputime()-t1;

format("vaddbench:    vector+scalar real addition ..\n");
t1=cputime();
VAddBench();
vadd_time = cputime()-t1;

format("viaddbench:   vector+scalar integer addition ..\n");
t1=cputime();
VIAddBench();
viadd_time = cputime()-t1;

format("vmulbench:    scalar*vector real multiplication ..\n");
t1=cputime();
VMulBench();
vmul_time = cputime()-t1;

fmt = "`20` `10.3` `10.3`\n";
format(fmt,"BENCHMARK",  "MIPS",            "MOPS");
format(fmt,"forbench",   "-",               1E-6*1500000/for_time);
format(fmt,"mipsbench",  1E-6*3E7/mips_time,0);
format(fmt,"vcopybench", "-",               1E-6*1E8/vcopy_time);
format(fmt,"vaddbench",  "-",               1E-6*1E8/vadd_time);
format(fmt,"viaddbench", "-",               1E-6*1E8/viadd_time);
format(fmt,"vmulbench",  "-",               1E-6*1E8/vmul_time);

// Graphical display and data gathered from other machines
tests = strmat("100*For","10*Mips","Vcopy","Vadd","Viadd","Vmul");
systems = strmat("Pentium II/400","SGI R10k/195", "IBM Power-2","HP 755","Alpha 240","SGI R8k/75","This machine");
for_mops =      #(1.17,            0.41,           0.191,       0.226,       0.161,   0.148);
mips_mips =     #(7.9,             3,              1.51,        1.65,        1.03,    1.29);
vcopy_mops =    #(240,             88,             28.3,        18.1,        27.1,    41.6);
vadd_mops =     #(73,              67,             30,          21.4,        13.6,    42.5);
viadd_mops =    #(117,             72,             20.3,        17.6,        16.9,    22.6);
vmul_mops =     #(64,              67,             27.3,        21.4,        13.4,    44.1);
/*
systems = strmat("Linux 486/33","SGI R4400/150","IBM Power-2","HP 755","Alpha 240","SGI R8000","This machine");
for_mops =     #(0.042,         0.202,          0.191,       0.226,       0.161,   0.148);
mips_mips =    #(0.36,          1.32,           1.51,        1.65,        1.03,    1.29);
vcopy_mops =   #(3.14,          15.5,           28.3,        18.1,        27.1,    41.6);
vadd_mops =    #(1.15,          9.95,           30,          21.4,        13.6,    42.5);
viadd_mops =   #(3.81,          19,             20.3,        17.6,        16.9,    22.6);
vmul_mops =    #(1.09,          5.83,           27.3,        21.4,        13.4,    44.1);
*/

bar(tests,
    #(100*for_mops, 100*1E-6*1500000/for_time;
	  10*mips_mips, 10*1E-6*3E7/mips_time;
	  vcopy_mops,1E-6*1E8/vcopy_time;
	  vadd_mops, 1E-6*1E8/vadd_time;
	  viadd_mops,1E-6*1E8/viadd_time;
	  vmul_mops, 1E-6*1E8/vmul_time),
	systems,
    "xlabel","Benchmarks","ylabel","Million ops/sec",
    "toplabel",sformat("Tela benchmarks, version ``",version()),"comment","");

};

