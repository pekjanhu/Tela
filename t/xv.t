package
global(xv)
{

function xv(r;g,b,opts)
/* xv(x) displays integer matrix x as a grayscale image in
   XV window. If all matrix elements are 0 or 1, a black-
   and-white image is shown.
   xv(r,g,b) displays a 24-bit (true color) image.
   The matrices r,g,b must define the red,green and blue
   color components, respectively.
   The matrix row dimension grows downward, the column
   dimension grows to the left. A new XV process is forked. */
global(cnt)
{
	if (isundefined(cnt)) cnt=1;		// cnt makes tmpfile unique for every xv call
	tmpdir = getenv("TMPDIR");
	if (isvoid(tmpdir)) tmpdir = "/tmp";
	tmpfile = sformat("``/xvpic``.``.pbm",tmpdir,getpid(),cnt);
	options = "";
	if (isdefined(g) && isdefined(b)) {
		export_PBM(tmpfile,round(r),round(g),round(b));
		if (isdefined(opts)) options = opts;
	} else {
		export_PBM(tmpfile,round(r));
		if (isdefined(g) && isstring(g)) options = g;
	};
	system(sformat("(xv `` ``; rm ``)&", options,tmpfile,tmpfile));
	cnt++;
};
	
}
