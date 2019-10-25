package global(diff) {

function y = diff(x;d1)
/* diff(x) returns the first forward finite difference
   with respect to the first dimension, e.g. if x is a vector,
   diff(x) gives x[2:length(x)] - x[1:length(x)-1].

   diff(x,d) takes the difference with respect to dth dimension.
   Only ranks up to 3 are currently supported. */
{
	r = rank(x);
	if (r < 1) {format("*** diff: x must be array\n"); return};
	if (isdefined(d1)) d=d1 else d=1;
	if (r == 1) {
		if (d != 1) {format("*** diff: x is vector and d!=1\n"); return};
		nx = length(x);
		y = x[2:nx] - x[1:nx-1];
	} else if (r == 2) {
		if (d != 1 && d != 2) {format("*** diff: x is matrix and d!=1,2\n"); return};
		[nx,ny] = size(x);
		if (d == 1)
			y = x[2:nx,:] - x[1:nx-1,:]
		else
			y = x[:,2:ny] - x[:,1:ny-1];
	} else if (r == 3) {
		if (d != 1 && d != 2 && d != 3) {format("*** diff: d outside range\n"); return};
		[nx,ny,nz] = size(x);
		if (d == 1)
			y = x[2:nx,:,:] - x[1:nx-1,:,:]
		else if (d == 2)
			y = x[:,2:ny,:] - x[:,1:ny-1,:]
		else
			y = x[:,:,2:nz] - x[:,:,1:nz-1];
	} else
		format("*** diff: rank > 3 not supported, sorry\n");
};

}
