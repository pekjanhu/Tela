// remdiv.t - test divergence remover
// Theory: Let B be a 2D vector field.
// Then B + r*grad(div B), where r is a small constant, will have
// the same curl as B but smaller divergence. This can be used
// to reduce the divergence of a numerical vector field that should
// be exactly divergence-free.

imax = 40;	// the grid size
jmax = 40;

dx = 0.1;	// the grid spacing
dy = 0.1;

function [u;] = BC()	// Impose homogeneous Neumann boundary conditions on grid u
{
	u[2:imax-1,1] = u[2:imax-1,2];
	u[2:imax-1,jmax] = u[2:imax-1,jmax-1];
	u[1,2:jmax-1] = u[2,2:jmax-1];
	u[imax,2:jmax-1] = u[imax-1,2:jmax-1];
};

function [Vx,Vy] = grad(f)	// Compute the gradient of a scalar field f
{
	Vx = zeros(imax,jmax);
	Vy = zeros(imax,jmax);
	Vx[2:imax-1,2:jmax-1] = (f[3:imax,2:jmax-1] - f[1:imax-2,2:jmax-1])/(2*dx);
	Vy[2:imax-1,2:jmax-1] = (f[2:imax-1,3:jmax] - f[2:imax-1,1:jmax-2])/(2*dy);
	[Vx] = BC();
	[Vy] = BC();
};

function d = div(fx,fy)		// Compute the divergence of a vector field (fx,fy)
{
	d = zeros(imax,jmax);
	d[2:imax-1,2:jmax-1] =
		(fx[3:imax,2:jmax-1] - fx[1:imax-2,2:jmax-1])/(2*dx) +
		(fy[2:imax-1,3:jmax] - fy[2:imax-1,1:jmax-2])/(2*dy);
	[d] = BC();
};

function d = curl(fx,fy)	// Compute the curl of a vector field fx,fy
                            // Since we are in 2D, the curl has only one (z) component.
{
	d = zeros(imax,jmax);
	d[2:imax-1,2:jmax-1] =
		(fy[3:imax,2:jmax-1] - fy[1:imax-2,2:jmax-1])/(2*dx) -
		(fx[2:imax-1,3:jmax] - fx[2:imax-1,1:jmax-2])/(2*dy);
	[d] = BC();
};

function [Bx,By] = remdiv()	// Do one divergence-reduction step
local(Bx1,By1,r)
{
	r = 0.5*0.125*dx^2;
	[Bx1,By1] = grad(r*div(Bx,By));
	Bx = Bx + Bx1;
	By = By + By1;
};

function m = norm(u) {m = sum(abs(u))/(imax*jmax)};		// The one-norm of a scalar field u

// Initialize a random vector field (Bx,By)
Bx = rand(imax,jmax) - 0.5;
By = rand(imax,jmax) - 0.5;
[Bx] = BC();
[By] = BC(); 	// Impose the Neumann boundary conditions on it

// Call the divergence reducer 100 times:
for (cnt=0; cnt<100; cnt++) {
	if (cnt mod 10 == 0) {
		format("Step ``, |div B| = ``, |curl B| = ``\n",cnt+1,norm(div(Bx,By)),norm(curl(Bx,By)));
	};
	[Bx,By] = remdiv();
};



