function bm(n,m,op)
global
{
	s = sformat("A=rand(``,``);
                 p0=perf();
                 for(i=1; i<=``; i++) ``;
                 p=perf()-p0;",
				n,n,m,op);
	eval(s);
	MF = Mflops(p);
	format("`15` : Mflops = ``\n",op,MF);
};

for (i=1; i<=72; i++) format("-"); format("\n");
format("Tela-`` running on ``",version(),run("uname -a"));
for (i=1; i<=72; i++) format("-"); format("\n");

format("First testing speed of B=A+1 for various vector lengths...");

lengths = #(10,25,50,100,250,500,750,1000,2000,5000,7500,10000,20000,50000,75000,100000,200000,500000);
//iters = round(8*20000/sqrt(lengths-8));
iters = round(20*20000/(where(lengths>10000,4,1)*sqrt(lengths-8)));
megaflops = zeros(size(lengths));
for (i=1; i<=length(lengths); i++) {
	s = sformat("A=rand(``);
                 p0=perf();
                 for(j=1; j<=``; j++) B=A+1;
                 p=perf()-p0;",
				lengths[i],iters[i]);
	eval(s);
	megaflops[i] = max(0.1,Mflops(p));
};

format("Done.\n");

// the old sumppu, 50 MHz R4000
megaflops_sumppu = #(0.402972, 1.32971, 2.42171, 3.16636, 6.11611, 6.37161, 5.6623, 5.12683, 5.19016, 5.25154, 5.29892, 5.14685, 5.11636, 3.86976, 3.58619, 3.4489, 3.1642, 2.90688);

// the new sumppu, 195 MHz R10000
megaflops_sumppu = #(2.76882, 6.52507, 12.0783, 21.5615, 40.1624, 55.5966, 64.2357, 69.5357, 70.8917, 42.6064, 43.9498, 43.9319, 43.0659, 29.8511, 12.6039, 7.28488, 8.12976, 8.03325);

// R4400
megaflops_nuotta = #(1.10795, 2.6275, 3.79184, 6.52933, 9.88344, 11.981, 10.3231, 8.32587, 7.24773, 7.60409, 7.09044, 6.97588, 7.58836, 6.80639, 5.57375, 4.16604, 3.62971, 3.5896);

// R8000
megaflops_cypress =
#(1.0676, 2.39748, 4.58629, 8.02046, 17.2219, 29.5331, 36.0292, 41.2659, 49.9164, 59.6166, 63.4406, 62.224, 66.544, 63.4039, 59.9074, 61.4652, 60.7853, 9.8101);

// IBM Power-2
megaflops_rutja = #(1.41558, 3.88353, 7.41925, 10.0053, 15.6913, 22.3553, 27.4533, 31.7553, 44.8053, 10.8854, 10.1918, 9.52431, 10.0718, 9.67414, 10.7355, 11.0528, 10.5884, 10.4478);

// Cray C90
megaflops_cray = #(0.476614, 1.19192, 2.38017, 4.72507, 11.5865, 22.458, 32.7396, 42.4113, 76.7803, 148.549, 188.37, 216.575, 283.673, 338.226, 354.365, 373.717, 390.228, 400.158);

// HP
megaflops_harley = #(2.68329, 6.21286, 12.1611, 19.4206, 32.9406, 51.3434, 51.2761, 54.2179, 48.3175, 48.3265, 34.8059, 26.9629, 23.3012, 21.7116, 22.6195, 21.8072, 14.0783, 7.01431);

// 400 MHz Pentium-II, 25 Nov 1998
megaflops_esim = #(7.54247, 16.1692, 30.8613, 41.7025, 85.7083, 120.217, 88.11, 72.5714, 32.5909, 34.3182, 29.5053, 34.8043, 29.7895, 19.8889, 19.2105, 18.9513, 18.9418, 18.8333);

s = run("echo `uname -s` `uname -r` on `uname -n`");
hold(on);
plot(lengths/1024,megaflops,
	 "toplabel","Tela benchmark B=A+1",
	 "subtitle","MFLOPS as function of vector length",
	 "comment",s[1:length(s)-1],
	 "xlog","true","ylog","true","grid","true",
	 "xlabel","Vector length (kWord)","ylabel","Mflops",
	 "linecolor",yellow,"linewidth",3,
	 lengths/1024,megaflops_sumppu,"linecolor",green,"linewidth",3,
	 lengths/1024,megaflops_esim,"linecolor",red,"linewidth",3,
	 lengths/1024,megaflops_cypress,"linecolor",pink,"linewidth",3,
	 lengths/1024,megaflops_harley,"linecolor",8,"linewidth",3,
	 lengths/1024,megaflops_cray,"linecolor",blue,"linewidth",3);
N = length(lengths);
annotate("point","x1",0.004,"y1",50,
		 "markertype",13,"markersize",4,"markercolor",green,"linelabel","SGI R10000/195");
annotate("point","x1",0.004,"y1",40,
		 "markertype",13,"markersize",4,"markercolor",8,"linelabel","DEC 2100 EV5/250");
annotate("point","x1",0.004,"y1",70,
		 "markertype",13,"markersize",4,"markercolor",red,"linelabel","Pentium-II/400");
annotate("point","x1",0.004,"y1",100,
		 "markertype",13,"markersize",4,"markercolor",pink,"linelabel","SGI R8000");
annotate("point","x1",0.004,"y1",150,
		 "markertype",13,"markersize",4,"markercolor",blue,"linelabel","Cray C90");
annotate("point","x1",0.004,"y1",30,
		 "markertype",13,"markersize",4,"markercolor",yellow,"linelabel","This machine");
hold(off);

format("Then going on with more operations without graphical output...\n");

format("O(N) operations (N=10):\n");
n = 3;
m = 10000;
bm(n,m,"B=A+A");
bm(n,m,"B=A*A");
bm(n,m,"B=A+1");
bm(n,m,"B=A/A");

format("O(N) operations (N=100):\n");
n = 10;
m = 10000;
bm(n,m,"B=A+A");
bm(n,m,"B=A*A");
bm(n,m,"B=A+1");
bm(n,m,"B=A/A");

format("O(N) operations (N=300):\n");
n = 17;
m = 5000;
bm(n,m,"B=A+A");
bm(n,m,"B=A*A");
bm(n,m,"B=A+1");
bm(n,m,"B=A/A");

format("O(N) operations (N=1000):\n");
n = 33;
m = 3000;
bm(n,m,"B=A+A");
bm(n,m,"B=A*A");
bm(n,m,"B=A+1");
bm(n,m,"B=A/A");

format("O(N) operations (N=3000):\n");
n = 55;
m = 2000;
bm(n,m,"B=A+A");
bm(n,m,"B=A*A");
bm(n,m,"B=A+1");
bm(n,m,"B=A/A");

format("O(N) operations (N=10000):\n");
n = 100;
m = 1000;
bm(n,m,"B=A+A");
bm(n,m,"B=A*A");
bm(n,m,"B=A+1");
bm(n,m,"B=A/A");

format("O(N^2) operations (N=100):\n");
n = 100;
m = 50;
x = rand(n);
z = x + 1i*x;
bm(n,m,"B=A**x");
//bm(n,m,"B=A**z");

format("O(N^2) operations (N=300):\n");
n = 300;
m = 50;
x = rand(n);
z = x + 1i*x;
bm(n,m,"B=A**x");
//bm(n,m,"B=A**z");

format("O(N^3) operations (N=100):\n");
n = 100;
m = 10;
bm(n,m,"B=inv(A)");
bm(n,m,"B=A**A");
bm(n,m,"[L,U,P]=LU(A)");

format("O(N^3) operations (N=300):\n");
n = 300;
m = 10;
bm(n,m,"B=inv(A)");
bm(n,m,"B=A**A");
bm(n,m,"[L,U,P]=LU(A)");

