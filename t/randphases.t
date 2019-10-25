// Generate a power-law spectrum using random phases

kmax = 4;
SpectralExponent = 5/3;
SpectralExponent = 3.2;
Nx = 64;
Ny = 64;

[kx,ky] = grid((1:Nx)*(kmax/Nx), (1:Ny)*(kmax/Ny));
[nx,ny] = size(kx);
kxphys = (kx+0.5*kmax) mod kmax - 0.5*kmax;
kyphys = (ky+0.5*kmax) mod kmax - 0.5*kmax;
Fk = sqrt(0.1+kxphys^2+kyphys^2)^(-SpectralExponent)*exp(2i*pi*rand(nx,ny));
F = Re(invFFT2(Fk));
pcolor(F,"xmin",0,"xmax",1,"ymin",0,"ymax",1);
