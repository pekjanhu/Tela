u = rand(9);
f=realFFT(u);
w=realFFTW(u);
max(abs(invrealFFTW(w)-u));
return;


nx = 8; ny = 16; nz = 32; nt = 8;
glmax = 0;
format("FFT/FFTW comparison (real data):\n");
dotest(FFT,FFTW,0);
format("FFT/FFTW comparison (complex data):\n");
dotest(FFT,FFTW,1);
format("invFFT/invFFTW comparison (real data):\n");
dotest(invFFT,invFFTW,0);
format("invFFT/invFFTW comparison (complex data):\n");
dotest(invFFT,invFFTW,1);
format("realFFT/realFFTW comparison:\n");
dotest(realFFT,realFFTW,0);

format("realFFT/realFFTW/invrealFFT/invrealFFTW comparison:\n");
format("1D:\n");
u = rand(nx);
f = realFFT(u);
w = realFFTW(u);
uf = invrealFFT(f);
uw = invrealFFTW(f);
format("diff(F/W):   ``\n",Max(abs(f-w)));
format("diff(iF/iW): ``\n",Max(abs(uf-uw)));
format("roundtrip:   ``\n",Max(abs(uw-u)));
format("2D:\n");
u = rand(nx,ny);
for (d=1; d<=2; d++) {
	f = realFFT(u,d);
	w = realFFTW(u,d);
	uf = invrealFFT(f,d);
	uw = invrealFFTW(f,d);
	format("d=`` diff(F/W):   ``\n",d,Max(abs(f-w)));
	format("d=`` diff(iF/iW): ``\n",d,Max(abs(uf-uw)));
	format("d=`` roundtrip:   ``\n",d,Max(abs(uw-u)));
}; 
format("3D:\n");
u = rand(nx,ny,nz);
for (d=1; d<=3; d++) {
	f = realFFT(u,d);
	w = realFFTW(u,d);
	uf = invrealFFT(f,d);
	uw = invrealFFTW(f,d);
	format("d=`` diff(F/W):   ``\n",d,Max(abs(f-w)));
	format("d=`` diff(iF/iW): ``\n",d,Max(abs(uf-uw)));
	format("d=`` roundtrip:   ``\n",d,Max(abs(uw-u)));
}; 
format("4D:\n");
u = rand(nx,ny,nz,nt);
for (d=1; d<=4; d++) {
	f = realFFT(u,d);
	w = realFFTW(u,d);
	uf = invrealFFT(f,d);
	uw = invrealFFTW(f,d);
	format("d=`` diff(F/W):   ``\n",d,Max(abs(f-w)));
	format("d=`` diff(iF/iW): ``\n",d,Max(abs(uf-uw)));
	format("d=`` roundtrip:   ``\n",d,Max(abs(uw-u)));
};
format("Global maximum error = ``\n",glmax);

// -----------------------------------------------------------------

function y = Max(x) global(glmax) {y=max(x); glmax=max(glmax,y)};

function dotest(FFT,FFTW,cmplxflag)
global(nx,ny,nz,nt)
{
	u=rand(nx);
	if (cmplxflag) u+= 1i*rand(nx);
	f=FFT(u);
	w=FFTW(u);
	format("1D: ``\n",Max(abs(f-w)));
	u=rand(nx,ny);
	if (cmplxflag) u+= 1i*rand(nx,ny);
	for (d=1; d<=2; d++) {
		f=FFT(u,d);
		w=FFTW(u,d);
		format("2D, d=``: ``\n",d,Max(abs(f-w)));
	};
	u=rand(nx,ny,nz);
	if (cmplxflag) u+= 1i*rand(nx,ny,nz);
	for (d=1; d<=3; d++) {
		f=FFT(u,d);
		w=FFTW(u,d);
		format("3D, d=``: ``\n",d,Max(abs(f-w)));
	};
	u=rand(nx,ny,nz,nt);
	if (cmplxflag) u+= 1i*rand(nx,ny,nz,nt);
	for (d=1; d<=4; d++) {
		f=FFT(u,d);
		w=FFTW(u,d);
		format("4D, d=``: ``\n",d,Max(abs(f-w)));
	};
};

	
