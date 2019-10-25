function u = turbgen(Nx,Ny; gamma)
/* turbgen(Nx,Ny) generates Nx x Ny matrix of random
   phase turbulence obeying 5/3 power law spectrum.
   The result is normalized so that the values fall
   between zero and one, inclusive.
   turbgen(Nx,Ny,gamma) uses specified power instead of 5/3. */
{
    if (isdefined(gamma)) 
        SpectralExponent=abs(gamma)
    else
        SpectralExponent = 5/3;
    [kx,ky] = grid(1.:Nx,1.:Ny);
    kxphys = (kx+0.5*Nx) mod Nx - 0.5*Nx;
    kyphys = (ky+0.5*Ny) mod Ny - 0.5*Ny;
    uk = sqrt(0.001*Nx^2+kxphys^2+kyphys^2)^(-SpectralExponent)
           * exp(2i*pi*rand(Nx,Ny));
    u = Re(invFFT2(uk));
    u/= max(u) - min(u);
    u-= min(u);
};
