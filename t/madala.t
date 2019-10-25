package global
(
	DIRICHLET, NEUMANN, PERIODIC,
	Madala, Madala1, TestMadala,
	SetMadalaBCs, SetMadalaBlockSize, SetMadalaIterationCount, SetMadalaDebug,
	CheckResidual
) {

// madala.t - the Madala algorithm for 2D 5-point elliptic equations.
// This program can solve any elliptic equation in a rectangular domain.
// The boundary conditions can be Dirichlet, Neumann or periodic.
// The only LIMITATION is that the y-boundary conditions (the second
// dimension) may not be periodic.

// Reference: Rangarao Madala, Monthly Weather Review, 106, 1735-1741 (1978)

DIRICHLET = 1;
NEUMANN   = 2;
PERIODIC  = 3;

function SetMadalaBCs(x1,x2,y1,y2) global
{
	if (!isint(x1) || !isscalar(x1) || x1<1 || x1>3) {error("SetMadalaBCs: bad x1 argument"); return};
	if (!isint(x2) || !isscalar(x2) || x2<1 || x2>3) {error("SetMadalaBCs: bad x2 argument"); return};
	if (!isint(y1) || !isscalar(y1) || y1<1 || y1>3) {error("SetMadalaBCs: bad y1 argument"); return};
	if (!isint(y2) || !isscalar(y2) || y2<1 || y2>3) {error("SetMadalaBCs: bad y2 argument"); return};
	if (y1==3 || y2==3) {error("SetMadalaBCs: Y conditions may not be periodic"); return};
	xbc1 = x1;
	xbc2 = x2;
	ybc1 = y1;
	ybc2 = y2;
	if (xbc1==3 || xbc2==3) {xbc1=3; xbc2=3};
};

function SetMadalaBlockSize(nb) global
{
	if (!isint(nb) || !isscalar(nb) || nb<3) {error("SetMadalaBlockSize: bad argument"); return};
	NB = nb;
};

function SetMadalaIterationCount(nits) global
{
	if (!isint(nits) || !isscalar(nits) || nits<1) {error("SetMadalaIterationCount: bad argument"); return};
	Nits = nits;
};

function SetMadalaDebug(flag) global {debug=flag};

function error(str) {format("``\n",str)};
	
// y is the sweeping direction

NB = 7;		// The block length. Depends on machine precision (high prec ==> larger NB)
Nits = 6;	// The number of internal roundoff-error reducing iterations
debug = 0;	// the debug flag

xbc1 = DIRICHLET;
xbc2 = DIRICHLET;
ybc1 = DIRICHLET;
ybc2 = DIRICHLET;

function [u;] = xBC(u0,j) local(imax,jmax)
// Apply inhomogeneous X boundary conditions on u at j
// Of course, j may be integer vector or scalar
{
	[imax,jmax] = size(u);
	if (xbc1 == NEUMANN)
		u[1,j] = u[2,j] + u0[1,j] - u0[2,j]
	else if (xbc1 == PERIODIC)
		u[1,j] = u[imax-1,j] + u0[1,j] - u0[imax-1,j]
	else
		u[1,j] = u0[1,j];
	if (xbc2 == NEUMANN)
		u[imax,j] = u[imax-1,j] + u0[imax,j] - u0[imax-1,j]
	else if (xbc2 == PERIODIC)
		u[imax,j] = u[2,j] + u0[imax,j] - u0[2,j]
	else
		u[imax,j] = u0[imax,j];
};

function [u;] = xBC_0(j) local(imax,jmax)
// Apply homogeneous X boundary conditions on u at j
{
	[imax,jmax] = size(u);
	if (xbc1 == NEUMANN)
		u[1,j] = u[2,j]
	else if (xbc1 == PERIODIC)
		u[1,j] = u[imax-1,j]
	else
		u[1,j] = 0;
	if (xbc2 == NEUMANN)
		u[imax,j] = u[imax-1,j]
	else if (xbc2 == PERIODIC)
		u[imax,j] = u[2,j]
	else
		u[imax,j] = 0;
};

function [u;] = yBC1(u0,i) global
// Apply inhomogeneous Y1-boundary conditions (notice not PERIODIC)
{
	if (ybc1 == NEUMANN)
		u[i,1] = u[i,2] + u0[i,1] - u0[i,2]
	else
		u[i,1] = u0[i,1];
};

function [u;] = yBC1_0(i) global
// Apply homogeneous Y1-boundary conditions (notice not PERIODIC)
{
	if (ybc1 == NEUMANN)
		u[i,1] = u[i,2]
	else
		u[i,1] = 0;
};

function [u;] = sweep(ap0,am0,a0p,a0m,a00,s,u0,j1,j2)
// Inhomogeneous sweep from j1 to j2 on u.
// First written row is j1+1, last is j2.
local(imax,jmax,I,j)
{
	[imax,jmax] = size(ap0);
	I = 2:imax-1;
	[u] = xBC(u0,j1);
	for (j=j1; j<j2; j++) {
		u[I,j+1] =
			(s[I,j]
			 - ap0[I,j]*u[I+1,j] - am0[I,j]*u[I-1,j]
			 - a0m[I,j]*u[I,j-1] - a00[I,j]*u[I,j] ) / a0p[I,j];
		[u] = xBC(u0,j+1);
	};
};

function [u;] = Hsweep(ap0,am0,a0p,a0m,a00,j1,j2)
// Homogeneous sweep from j1 to j2 on u.
// First written row is j1+1, last is j2.
local(imax,jmax,I,j)
{
	[imax,jmax] = size(ap0);
	I = 2:imax-1;
	[u] = xBC_0(j1);
	[u] = xBC_0(j1-1);	// took long until I discovered that this line is needed!
	for (j=j1; j<j2; j++) {
		u[I,j+1] =
			-( ap0[I,j]*u[I+1,j] + am0[I,j]*u[I-1,j]
			 + a0m[I,j]*u[I,j-1] + a00[I,j]*u[I,j] ) / a0p[I,j];
		[u] = xBC_0(j+1);
	};
};

function [R1,R2] = InfluenceMatrices(ap0,am0,a0p,a0m,a00,j1,j2,ybc2,S)
local(imax,jmax,I,k,kmax)
{
	[imax,jmax] = size(ap0);
	I = 2:imax-1;
	kmax = imax-2;
	R1 = zeros(kmax,kmax);
	R2 = R1;
	u = zeros(imax,jmax);
	for (k=1; k<=kmax; k++) {
		u[:,j1] = zeros(imax);
		u[k+1,j1] = 1;
		if (j1<=2) [u] = yBC1_0(I) else u[I,j1-1] = S**u[I,j1];
		[u] = Hsweep(ap0,am0,a0p,a0m,a00,j1,j2);
		R1[k,:] = u[I,j2-1];
		if (ybc2 == NEUMANN)
			R2[k,:] = u[I,j2] - u[I,j2-1]
		else
			R2[k,:] = u[I,j2];
	};
};	

// ------------------------------------------------------------------
// Function 'Madala' is the main algorithmic function.
// ------------------------------------------------------------------

function [u;] = Madala(ap0,am0,a0p,a0m,a00,u0,s)
// e2 = R1 e1, e3 = R2 e1 ==> e2 = R1 R2^-1 e3 = S e3
local(imax,jmax,I,NBLK,invRR,SS,j,b,nb,R1,R2,invR2,S,j2ndlast,uH,uprev,e1,e2,ybc,counter)
{
	[imax,jmax] = size(ap0);
	I = 2:imax-1;
	NBLK = ceil((jmax-2)/(NB-2));		// Number of blocks
	invRR = zeros(NBLK,imax-2,imax-2);	// NBLK influence matrices {inv(R2)}
	SS = zeros(NBLK-1,imax-2,imax-2);	// NBLK-1 S-influence matrices (neighbour-influences)
	if (debug) format("Preprocess: ");
	for ({j=1; b=1}; b<=NBLK; {j=j+NB-2; b++}) {
		if (debug) format("`` ",b);
		nb = min(NB,jmax-(j-1));
		if (b == NBLK) ybc=ybc2 else ybc=DIRICHLET;
		[R1,R2] = InfluenceMatrices(ap0,am0,a0p,a0m,a00,j+1,j+nb-1,ybc,S);
		/*
		format("symmratio for R2 = ``\n",mean(abs(R2-R2.'))/mean(abs(R2)));
        eigens=eig(R2);
        if (all(Re(eigens)>0) && all(Im(eigens)==0)) format("R2 is positive definite.\n");
		*/
		invR2 = inv(R2);
		invRR[b,:,:] = invR2;
		if (b < NBLK) {
			S = R1**invR2;
			SS[b,:,:] = S;
		};
	};
	if (debug) format("\nForward: ");
	// invRR,SS ready, preprocessing phase complete
	//u = u0;
	for ({j=1; b=1}; b<=NBLK; {j=j+NB-2; b++}) {
		if (debug) format("`` ",b);
		nb = min(NB,jmax-(j-1));
		if (b==NBLK-1) j2ndlast = j;
		u[I,j+1] = u0[I,j+1];		// guess; use u0 as guess
		if (b==1) [u] = yBC1(u0,I);
		[u] = sweep(ap0,am0,a0p,a0m,a00,s,u0,j+1,j+nb-1);
		for (counter=1; counter<=Nits; counter++) {
			if (ybc2 == NEUMANN && b==NBLK)
				e2 = (u0[I,jmax]-u0[I,jmax-1]) - (u[I,jmax]-u[I,jmax-1])
			else
				e2 = u0[I,j+nb-1] - u[I,j+nb-1];	// error vector at j+nb-1
			// now find uH that satisfies homogeneous eqn. and uH[I,jmax]==e2
			e1 = invRR[b,:,:]**e2;					// error vector at j=1
			uH = zeros(imax,jmax);
			uH[I,j+1] = e1;
			if (b==1)
				[uH] = yBC1_0(I)
			else
				uH[I,j] = SS[b-1,:,:]**e1;
			[uH] = Hsweep(ap0,am0,a0p,a0m,a00,j+1,j+nb-1);
			u = u + uH;
		};
	};
	if (debug) format("\nBackward: ");
	// Backward correction phase
	for ({b=NBLK-1; j=j2ndlast}; b>=1; {b--;j=j-(NB-2)}) {
		if (debug) format("`` ",b);
		nb = min(NB,jmax-(j-1));
		uprev = u0[I,j+nb-1];
		for (counter=1; counter<=Nits; counter++) {
			uH = zeros(imax,jmax);
			e2 = u[I,j+nb-1] - uprev;	// error vector at j+nb-1
			e1 = invRR[b,:,:]**e2;
			uH[I,j+1] = e1;
			if (b==1)
				[uH] = yBC1_0(I)
			else
				uH[I,j] = SS[b-1,:,:]**e1;
			[uH] = Hsweep(ap0,am0,a0p,a0m,a00,j+1,j+nb-3);
			u = u + uH;
			uprev = u[I,j+nb-1];
		};
	};
	if (debug) format("\n");
};

// ------------------------------------------------------------------
// Function 'Madala1' is the direct error vector propagation
// algorithm without subblocking. It needs the aux. function
// 'InfluenceMatrix' also.
// ------------------------------------------------------------------

function R = InfluenceMatrix(ap0,am0,a0p,a0m,a00,j1,j2,ybc2)
local(imax,jmax,I,k)
{
	[imax,jmax] = size(ap0);
	I = 2:imax-1;
	R = zeros(imax-2,imax-2);
	for (k=1; k<=imax-2; k++) {
		u = zeros(imax,jmax);
		u[k+1,j1] = 1;
		if (j1<=2) [u] = yBC1_0(k+1);
		[u] = Hsweep(ap0,am0,a0p,a0m,a00,j1,j2);
		if (ybc2 == NEUMANN)
			R[k,:] = u[I,j2] - u[I,j2-1]
		else
			R[k,:] = u[I,j2];
	};
};	

function [u;] = Madala1(ap0,am0,a0p,a0m,a00,u0,s)
// ap0*u[i+1,j] + am0*u[i-1,j] + a0p*u[i,j+1] + a0m*u[i,j-1] + a00*u == s
// ==> u[i,j+1] == (s - ap0*u[i+1,j] - am0*u[i-1,j] - a0m*u[i,j-1] - a00*u)/a0p
local(imax,jmax,I,R,k,invR,resids,counter,e2,e1,uH,a,m,aB,mB)
{
	[imax,jmax] = size(ap0);
	I = 2:imax-1;
	R = InfluenceMatrix(ap0,am0,a0p,a0m,a00,2,jmax,ybc2);
	invR = inv(R);
	// Find particular solution u that satisfies all BCs except at jmax
	u = u0;
	u[I,2] = zeros(imax-2);	// guess; zero is best guess?
	[u] = yBC1(u0,I);
	[u] = sweep(ap0,am0,a0p,a0m,a00,s,u0,2,jmax);
	resids = zeros(4,Nits);
	for (counter=1; counter<=Nits; counter++) {
		if (ybc2 == NEUMANN)
			e2 = (u0[I,jmax]-u0[I,jmax-1]) - (u[I,jmax]-u[I,jmax-1])
		else
			e2 = u0[I,jmax] - u[I,jmax];	// error vector at j=jmax
		// now find uH that satisfies homogeneous eqn. and uH[I,jmax]==e2
		//e1 = linsolve(R,e2);			// error vector at j=1
		e1 = invR**e2;					// error vector at j=1
		uH = zeros(imax,jmax);
		uH[I,2] = e1;
		[uH] = yBC1_0(I);
		[uH] = Hsweep(ap0,am0,a0p,a0m,a00,2,jmax);
		u = u + uH;
		[a,m,aB,mB] = CheckResidual(u,ap0,am0,a0p,a0m,a00,u0,s,"false");
		resids[:,counter] = #(a,m,aB,mB);
	};
};

function [averesid,maxresid,aveBresid,maxBresid] = CheckResidual(u,ap0,am0,a0p,a0m,a00,u0,s; printflag)
local(imax,jmax,I,J,resid,absresid,aveBx1,aveBx2,aveBy1,aveBy2,maxBx1,maxBx2,maxBy1,maxBy2)
{
	[imax,jmax] = size(u);
	I = 2:imax-1; J = 2:jmax-1;
	// Compute averesid, maxresid for interior points
	averesid = 0.0; maxresid = 0.0;
	resid =
		ap0[I,J]*u[I+1,J] + am0[I,J]*u[I-1,J] +
		a0p[I,J]*u[I,J+1] + a0m[I,J]*u[I,J-1] +
		a00[I,J]*u[I,J] - s[I,J];
	RESID=zeros(imax,jmax);
	RESID[I,J]=resid;
	averesid = mean(abs(resid));
	maxresid = max(abs(resid));
	// Compute residuals for all 4 boundaries
	// Boundary x1
	if (xbc1 == NEUMANN)
		absresid = abs((u[1,J] - u[2,J]) - (u0[1,J] - u0[2,J]))
	else if (xbc1 == PERIODIC)
		absresid = abs((u[1,J] - u[imax-1,J]) - (u0[1,J] - u0[imax-1,J]))
	else
		absresid = abs(u[1,J] - u0[1,J]);
	aveBx1 = mean(absresid);
	maxBx1 = max(absresid);
	// Boundary x2
	if (xbc2 == NEUMANN)
		absresid = abs((u[imax,J] - u[imax-1,J]) - (u0[imax,J] - u0[imax-1,J]))
	else if (xbc2 == PERIODIC)
		absresid = abs((u[imax,J] - u[2,J]) - (u0[imax,J] - u0[2,J]))
	else
		absresid = abs(u[imax,J] - u0[imax,J]);
	aveBx2 = mean(absresid);
	maxBx2 = max(absresid);
	// Boundary y1
	if (ybc1 == NEUMANN)
		absresid = abs((u[I,1] - u[I,2]) - (u0[I,1] - u0[I,2]))
	else if (ybc1 == PERIODIC)
		absresid = abs((u[I,1] - u[I,jmax-1]) - (u0[I,1] - u0[I,jmax-1]))
	else
		absresid = abs(u[I,1] - u0[I,1]);
	aveBy1 = mean(absresid);
	maxBy1 = max(absresid);
	// Boundary y2
	if (ybc2 == NEUMANN)
		absresid = abs((u[I,jmax] - u[I,jmax-1]) - (u0[I,jmax] - u0[I,jmax-1]))
	else if (ybc2 == PERIODIC)
		absresid = abs((u[I,jmax] - u[I,2]) - (u0[I,jmax] - u0[I,2]))
	else
		absresid = abs(u[I,jmax] - u0[I,jmax]);
	aveBy2 = mean(absresid);
	maxBy2 = max(absresid);
	aveBresid = mean(#(aveBx1,aveBx2,aveBy1,aveBy2));
	maxBresid = max(#(maxBx1,maxBx2,maxBy1,maxBy2));
	/*
	disp(#(aveBx1,aveBx2,aveBy1,aveBy2));
	disp(#(maxBx1,maxBx2,maxBy1,maxBy2));
	*/
	if (isdefined(printflag))
		format("averesid=``, maxresid=``,\n  aveBresid = ``, maxBresid = ``\n",
			   averesid,maxresid,aveBresid,maxBresid);
};

function TestMadala()
{
	plotopt("-pbg gray");
	imax = 100;
	jmax = 80;
	g = zeros(imax,jmax);
	u = g;
	[x,y] = grid((0:imax-1)/imax,(0:jmax-1)/jmax);
	dx = 1/(imax-2); dy = 1/(jmax-2);
	u0 = 0.1*sin(2*pi*x) + cos(4*pi*y);
    //u0 = 0.05*rand(imax,jmax);
    //u0 = g;
	ap0 = g+1/dx^2;
	am0 = g+1/dx^2;
	a0p = g+1/dy^2;
	a0m = g+1/dy^2;
	a00 = g-2*(1/dx^2+1/dy^2);
	s = g;
	s[round(imax/2),round(jmax/2)] = -1;
	s[round(imax/2)+1,round(jmax/2)] = 0.75;
	s = s/dx^2;
    /*
	utrue = g;
    [utrue] = Madala1(ap0,am0,a0p,a0m,a00,u0,s);
	format("utrue residual:::\n");
	[] = CheckResidual(utrue,ap0,am0,a0p,a0m,a00,u0,s);
	*/
	[u] = Madala(ap0,am0,a0p,a0m,a00,u0,s);
	[] = CheckResidual(u,ap0,am0,a0p,a0m,a00,u0,s,"true");
	//mesh(u,"xmin",1,"xmax",imax,"ymin",1,"ymax",jmax,"zlabel","u");
};

}
