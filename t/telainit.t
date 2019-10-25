// The Tela initialization file. Read automatically on startup.
// Usually users won't modify this.

// Declare some "stub" symbols (autoload stuff)

autosource("std.t","input","mean","int2str","stddev","Tr","FFT2","invFFT2",
		   "fib","docview","kron","compile","fix","isfile","nrange","strstr","export_ASCII","interp");
/*
autosource("aref.t","SetArrayStarts",
		   "aref1","aset1","aref2","aset2","aref3","aset3","aref4","aset4");
*/

autosource("perf.t","Mflops","Mips");

autosource("poly.t","polyval","polyadd","polymul","roots");

autosource("matmenu.t","matmenu","smatmenu");

autosource("fmin.t","fmin");
autosource("fsolve.t","fsolve");
autosource("integrate.t","integrate");
autosource("fit.t","fitline","fitlinear","fitnl");
autosource("diff.t","diff");
autosource("xv.t","xv");
autosource("draw.t","DrawString","SetFont");
autosource("lpmin.t","LinearProgramming");
autosource("specfun.t","Gamma","logGamma","fact","Beta","Binomial","poch",
		   "IncompleteGamma","erf","erfc","BetaB","BetaI","KummerU",
		   "BesselI","BesselJ","NeumannN","HankelH1","HankelH2","BesselK","LaguerreL");
autosource("geopack.t","SphericalToCartesian_deg","CartesianToSpherical_deg","CartesianToSpherical_vec",
		   "IGRF","T89c","T89","geotomag","geotomagXYZ","magtogeoXYZ","magtogeo",
		   "gsmtogseXYZ","gsetogsmXYZ","geotogsmXYZ","gsmtogeoXYZ","m2i");

// Define some color and line style symbolic constants

autoglobal("yellow","blue","green","red","DarkBlue","orange","pink",
		   "LightPink","cyan","brown");
hide("yellow","blue","green","red","DarkBlue","orange","pink",
	 "LightPink","cyan","brown");

yellow = 1;
blue = 2;
green = 3;
red = 4;
DarkBlue = 5;
orange = 6;
pink = 7;
LightPink = 8;
cyan = 9;
brown = 10;

autoglobal("solid","dashed","dotted");
hide("solid","dashed","dotted");

solid = 1;
dashed = 2;
dotted = 3;

// Try to read ~/.telainit.t
if (length(getenv("HOME")) > 0) source_silent(#(getenv("HOME"),"/.telainit.t"));

if (!BatchMode() && !SilentMode()) {
	if (UsingReadline())
		format("->TAB completion works; try source(\"demo\")\n")
	else
		format("->No readline library.\n")
};

// Make some function name aliases
import_PBM = import_PNM;
export_PBM = export_PNM;

