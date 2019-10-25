package global(Theta,Phi,bzsaved) {

E0 = 0.3E-3;
Kp = 4;
CorotationPotential = 91E3;
polarcappotential = 45E3;
dx = 0.5;
dy = dx;
[x,y] = grid(-16:dx:-2.,-7:dy:7.);

function [u;] = BC()	// Impose homogeneous Neumann boundary conditions on grid u
global
{
	u[2:imax-1,1] = u[2:imax-1,2];
	u[2:imax-1,jmax] = u[2:imax-1,jmax-1];
	u[1,:] = u[2,:];
	u[imax,:] = u[imax-1,:];
};


function [Vx,Vy] = grad(f)	// Compute the gradient of a scalar field f
global
{
	Vx = zeros(imax,jmax);
	Vy = zeros(imax,jmax);
	Vx[2:imax-1,2:jmax-1] = (f[3:imax,2:jmax-1] - f[1:imax-2,2:jmax-1])/(2*RE*dx);
	Vy[2:imax-1,2:jmax-1] = (f[2:imax-1,3:jmax] - f[2:imax-1,1:jmax-2])/(2*RE*dy);
	[Vx] = BC();
	[Vy] = BC();
};

function f=Phii(theta,phi1)
local (deltaphi,phi,thetar,phir,fcorot,x,y,a0,a,phifactor,thetawidth,
	   thetafactor,mainpart,harang,correction)
{
	deltaphi = 0;	// this parameter should somehow depend on IMF By
	phi = (phi1 + 180 + deltaphi) mod 360 - 180;
	phir = phi*pi/180;
	thetar = theta*pi/180;
	// Compute corotation part
	fcorot = -CorotationPotential*sin(theta*pi/180)^2;
	// Compute main part
	x = theta*cos(phir);
	y = theta*sin(phir);
	a0 = 17;	// polar cap radius
	a = a0 - 5*cos(phir);
	phifactor = sin(phir);
	thetawidth = 0.5*a*HeavisideTheta(a-theta) + 3*HeavisideTheta(theta-a);
	thetafactor = exp(-0.5*((theta-a)/thetawidth)^2);
	mainpart = polarcappotential*phifactor*thetafactor;
	// Compute harang
	a = a0-2 + 13*sin((phi-40)*pi/360)^2;
	thetawidth = 2*HeavisideTheta(a-theta) + 1.5*HeavisideTheta(theta-a);
	thetafactor = exp(-0.5*((theta-a)/thetawidth)^2);
	harang = 25E3*abs(cos(0.5*(phi-120)*pi/180))^5*thetafactor;
	// Compute yet another correction
	a = a0;
	thetawidth = 7*HeavisideTheta(a-theta) + 3*HeavisideTheta(theta-a);
	thetafactor = exp(-0.5*((theta-a)/thetawidth)^2);
	correction = 25E3*abs(cos(0.5*(phi+130)*pi/180))^4*thetafactor;
	// Sum up pieces
	f = fcorot - mainpart - harang + correction;
};

//function y=th(x) {y=HeavisideTheta(x)};

RE = 6730E3;
[imax,jmax] = size(x);
//rr = max(1,r+1E-8);
//phim = -E0*y*RE - CorotationPotential/rr + 20E3*exp(-((x+8)/(3*th(x+8)+10*th(-8-x)))^2-((y-2)/(6*th(y-2)+8*th(2-y)))^2);

function mycontour(z,tit) global {
contour(transpose(z),"xflip","true","cstep",2.0,"xlabel","y", "ylabel","x",
		"xmin",min(y), "xmax",max(y), "ymin",min(x), "ymax",max(x),
		"linetypes",1,
		"toplabel",tit, "subtitle","In equatorial plane GSM coordinates")
};	

function ionocontour(z) global {
contour(transpose(z),"xflip","true","cstep",4.0,"xlabel","", "ylabel","",//"splinetype",5,
		"xmin",min(xvec), "xmax",max(xvec), "ymin",min(yvec), "ymax",max(yvec),
		"linetypes",1,
		"toplabel","phii", "subtitle","In ionospheric plane")
};	

function mypcolor(z,tit) global {
pcolor(transpose(z),"xflip","true","xlabel","y","ylabel","x",
		"xmin",min(y), "xmax",max(y), "ymin",min(x), "ymax",max(x),
		"toplabel",tit, "subtitle","In equatorial plane GSM coordinates")
};	

//Bz = 30000E-9/rr^3;
Bz = zeros(imax,jmax);

/*
bzsaved = zeros(imax,jmax);
for (i=1; i<=imax; i++) for (j=1; j<=jmax; j++) {
    [bx,by,bz] = T89(Kp,x[i,j],y[i,j],0);
    bzsaved[i,j] = bz;
};
save("bz.hdf","bzsaved");
return;
*/

load("bz.hdf");
for (i=1; i<=imax; i++) for (j=1; j<=jmax; j++) {
	Bz[i,j] = 1E-9*bzsaved[i,j] /* * (1 - 0.5*exp(-0.5*((x[i,j]+9)/3)^2)*exp(-0.5*(y[i,j]/3)^2))*/;
};
load("ThetaPhi.hdf");
phim = Phii(Theta,Phi);

[xvec,yvec] = grid((-35:1:35.)+10*eps,(-35:1:35.)+10*eps);
[nx,ny] = size(xvec);
theta = sqrt( xvec^2 + yvec^2 );
phi = zeros(nx,ny);
for (i=1; i<=nx; i++) for (j=1; j<=ny; j++)
	phi[i,j] = (180/pi)*atan2(yvec[i,j],xvec[i,j]);
phii = Phii(theta,phi);

/*
function ionopoint(x,y,lab) local(theta,phi,xx,yy) {
	// ionopoint(x,y,lab) maps the GSM equatorial plane point (x,y)
	// to ionosphere and draws a small marker there
	[theta,phi] = m2i(Kp,x,y);
	xx = theta*cos(phi*pi/180);
	yy = theta*sin(phi*pi/180);
	annotate("point","x1",yy,"y1",xx,"markertype",13,"markersize",3,"markercolor",4,"linelabel",lab);
};
*/

function magnetopoint(x,y,lab) {
	annotate("point","x1",y,"y1",x,"markertype",13,"markersize",3,"markercolor",4,"linelabel",lab);
};

function dopoints(fn) {
	call(fn,-11,0,"A");
	call(fn,-5,-1,"B");
	call(fn,-9,3,"C");
	call(fn,-5,-5,"D");
	call(fn,-7,0,"E");
	call(fn,-12,6,"O");
};

function circle(x,y,r)
local (phi)
{
	phi = 0:0.1:2*pi;
	plot(x+r*cos(phi),y+r*sin(phi),"linecolor",3);
};

// Produce ionospheric contour plot
hold(on);
ionocontour(phii*1E-3);
circle(0,0,10);
circle(0,0,20);
circle(0,0,30);
//dopoints(ionopoint);
hold(off);

//mycontour(1E9*Bz,"Bz (nT)");
[Ex,Ey] = grad(-phim);
vx = Ey/Bz;
vy = -Ex/Bz;

//pause();

// Produce magnetospheric contour plot
hold(on);
mycontour(1E-3*phim,"Potential (kV)");
dopoints(magnetopoint);
vplot(transpose(y),transpose(x),transpose(vy),transpose(vx),
	  "xlabel","y","ylabel","x",
	  "vscale",1/200000,"vhead","false","xflip","true");
hold(off);

vmagn = sqrt(vx^2+vy^2);
[Dxvx,Dyvx] = grad(vx);
[Dxvy,Dyvy] = grad(vy);
w = abs(Dxvy-Dyvx);	// abs(curl v)

//mycontour(1E-3*vmagn,"v (km/s)");
scalelength = max(dx*RE,min(100*RE,vmagn/(1E-11+w)));
    // scale length less than dx is not reasonable
    // too large scale length may cause numerical problems
[scalelength] = BC();
//mycontour(scalelength/RE,"Scale length (RE)");

a = 0.72;
gamma = 1.64;
mp = 1.67E-27;

r = sqrt(x^2+y^2);
n = 1E6;
SigmaP = 5;
K = 1E-9;
Bi = 50000E-9;
H = 7*RE - 0.1*r*RE;
//scalelength = 2*RE;

/*[bx,by,bz] = T89(Kp,-11,0,0);	// compute bz at x=-11*/
bz = 7.91811;
bz = 1E-9*bz;					// transform it to Tesla
rho = mp*n * (Bz/bz);			// assume rho ~ Bz
lambda = sqrt(Bi*SigmaP/(Bz*K));
theQuantity = (1/a)*(rho*vmagn/(SigmaP*Bz^2))*(H/lambda)*(lambda/scalelength)^gamma;
hold(on);
mypcolor(min(2,theQuantity),"Pseudocolor plot");
dopoints(magnetopoint);
hold(off);

};
