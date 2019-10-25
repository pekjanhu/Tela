% Generate a power-law spectrum using random phases

kmax = 4;
SpectralExponent = 5/3;
SpectralExponent = 3.2;
Nx = 64;
Ny = 64;

[kx,ky] = meshgrid((1:Nx)*(kmax/Nx), (1:Ny)*(kmax/Ny));
[nx,ny] = size(kx);
Fk = sqrt(0.1+(rem(kx+0.5*kmax,kmax)-0.5*kmax).^2+(rem(ky+0.5*kmax,kmax)-0.5*kmax).^2).^(-SpectralExponent).*exp(2i*pi*rand(nx,ny));
F = real(ifft2(Fk));
pcolor(F);
shading flat;
colormap gray;
axis('square');
