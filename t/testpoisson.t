/*
   poissontest.t - demonstrate solving of 2D Poisson equation
   in a square domain using FFTs with various boundary conditions.
   PJ 9.1.1995

   Parameters:
   - nx, ny define the number of grid points in X and Y.
   - Lx, Ly is the box length in X and Y
   - Xfft, Yfft are set to relevant FFT variant function names. Key:
     Periodic BCs      : "realFFT" ("FFT" for X if Y is "realFFT")
	 Dirichlet         : "sinFFT"
	 Neumann           : "cosFFT"
	 D at 0, N at 1    : "sinqFFT"      (The quarter-wave expansions)
	 N at 0, D at 1    : "cosqFFT"
   - Most efficient nx,ny are products of small primes (2,3,5),
     possibly offset by one. See "help FFT" for when you need to offset.
*/

nx = 256;
ny = 256;

Lx = 1;
Ly = 2;

Xfft = "realFFT";
Yfft = "cosqFFT";

// Generate source term grid
s = zeros(nx,ny);
s[round(nx/2),round(ny/2)] = 1;
s[round(nx/2),round(ny/2)+1] = -0.6;
s[round(4*nx/5),round(ny/2)] = -1;
s[round(4*nx/5),round(ny/2)-1] = 0.6;

// Xlen,Ylen are 2, 1 or 0.5 for periodic, sin/cos and
// quarter-wave transforms, respectively.
xevenspec = ""; yevenspec = "";
xdivisor = 1; ydivisor = 1;
xstart = 1; ystart = 1;
MustFixConstant = 1;
if (Xfft=="FFT" || Xfft=="realFFT") {
    Xlen = 2/nx;
	xstart = 0;
	if (Xfft=="realFFT") {xevenspec = sformat(",``",nx); xdivisor = 2};
} else if (Xfft=="sinFFT") {
    Xlen = 1/(nx+1);
	MustFixConstant = 0;
} else if (Xfft=="cosFFT") {
	Xlen = 1/(nx-1);
	xstart = 0;
} else {
    Xlen = 1/nx;
	xstart = 0.5;
	MustFixConstant = 0;
};

if (Yfft=="FFT" || Yfft=="realFFT") {
    Ylen = 2/ny;
	ystart = 0;
	if (Yfft=="realFFT") {yevenspec = sformat(",``",ny); ydivisor = 2};
} else if (Yfft=="sinFFT") {
    Ylen = 1/(ny+1);
	MustFixConstant = 0;
} else if (Yfft=="cosFFT") {
	Ylen = 1/(ny-1);
	ystart = 0;
} else {
    Ylen = 1/ny;
	ystart = 0.5;
	MustFixConstant = 0;
};

// Generate k grids and evenspec. Evenspec is needed only for realFFT.
[kx,ky] = grid(Xlen*pi*(xstart:(nx+xstart-1)/xdivisor), Ylen*pi*(ystart:(ny+ystart-1)/ydivisor));

// Grid spacing in X and Y
dx = Lx/(nx-1);
dy = Ly/(ny-1);

// Compute the inverse kernel k^2
// The upper line gives the spectrally accurate alternative, which however
// can not be checked in the residual sense.
//k2 = (kx/dx)^2 + (ky/dy)^2; checkresid=1;
k2 = -2*((cos(kx) - 1)/dx^2 + (cos(ky) - 1)/dy^2); checkresid=1;

// The solution itself
tic();
eval(sformat("f = ``(``(s,2),1);",Xfft,Yfft));
if (MustFixConstant) {
	if (abs(f[1,1]) > 1E4*eps*max(abs(f)))
		format("*** Warning: Boundary conditions require setting <u>=0, but <s>!=0.\n");
	f[1,1] = 0;
	k2[1,1] = 1;		// Avoid dividing by zero
};
eval(sformat("u = inv``(inv``(f/k2,1``),2``);",Yfft,Xfft,xevenspec,yevenspec));
format("`` CPU seconds.\n",toc());

if (nx*ny < 2000) mesh(u,"hiddenline","true","xmin",0,"xmax",Lx,"ymin",0,"ymax",Ly);

if (checkresid) {
	//dx = 1; dy = 1;
	I = 2:nx-1; J = 2:ny-1;
	lhs = -(u[I+1,J] + u[I-1,J] - 2*u[I,J])/dx^2 - (u[I,J+1] + u[I,J-1] - 2*u[I,J])/dy^2;
	resid =  lhs - s[I,J];
	format("Normalized residual = ``\n",max(abs(resid))/mean(abs(s[I,J])));
};
