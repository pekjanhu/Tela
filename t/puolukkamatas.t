source("turbgen.t");
N = 128;
u=round(255*turbgen(N,N));
v=round(255*turbgen(N,N));
xv(round(0.8*u),v,0*u,"-expand 2 -name 'whortleberry hummock'");

