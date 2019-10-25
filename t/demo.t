repeat

EndChoice="Quit";
choice = smenu("Choose a demo:",
			  "2D graphics",
			  "Benchmarks",
			  "Contour and density plot example",
			  "Some Matlab style benchmarks",
			  "Matlab interface demo (works only on some systems)",
			  "XV interface demo (must have xv installed)",
			  "Power-law spectrum",
			  "Bar charts",
			  "BLAS-type benchmark series",
			  "Run some correctness tests",
			  "Quit demo"
			  );

function ev(x) {format("``\n",x); eval(x)};
function msg(x) {format("``\n",x)};

if (strstarteq(choice,"2D")) {

	// 2D graphics
	msg("Basic 2D graphics.");
	ev("
        x = 0:0.05:2*pi;
        y = sin(x);
        plot(x,y);
    ");
    pause();

    // Line styles
	msg("\nLine styles.");
    ev("
    plot(x,sin(x),\"linewidth\",3, \"linecolor\",4,
         x,cos(x),\"linewidth\",3, \"linecolor\",5);
    ");

} else if (strstarteq(choice,"Bench")) {

	msg("\nRunning some benchmarks.");
	msg("This may take some time or hopefully not;-)");
	pause();
	source("bench.t");
	msg("Benchmarks ready.");
	
} else if (strstarteq(choice,"Contour")) {

	cd("/usr/local/lib/tela");
	msg("\nA 'real world' example of plotting capabilities.");
	msg("Generating");
	msg("- a colormap plot with annotations");
	msg("- a contour plot with overlaid vector plot and annotations");
	msg("- a contour plot with annotations (points and green circles)");
	pause();
	source("3windows.t");
} else if (strstarteq(choice,"Some Matlab")) {
	msg("\nSome Matlab style benchmarks.");
	source("matlabbench2.t");
} else if (strstarteq(choice,"Matlab interface")) {
	if (isfunction(matlab_start)) {
        msg("\nDemonstrating how to call Matlab and plot with it from Tela.");
		source("matlabdemo.t");
	} else {
		msg("\n*** This Tela has no matlabeng.ct linked in. Sorry.");
	};
} else if (strstarteq(choice,"XV interface")) {
	msg("\nThe whortleberry hummock image is generated using just");
	msg("random numbers, Fourier transforms and algebraic operations.");
	source("puolukkamatas.t");
} else if (strstarteq(choice,"Power-law")) {
	msg("\nVisualizing a 2D power-law spectrum.");
	msg("First some input quantities.");
	repeat
      format("Give the number of grid points Nx [64] ");
      Nx = input();
	  if (isvoid(Nx)) Nx=64;
	until isint(Nx);
	Nx = abs(Nx);
	ev("
       SpectralExponent = 5/3;	// Kolmogoroff ...
       Ny = Nx;
    ");
	msg("Then the grids..");
	ev("
       [kx,ky] = grid(1.:Nx,1.:Ny);
       kxphys = (kx+0.5*Nx) mod Nx - 0.5*Nx;
       kyphys = (ky+0.5*Ny) mod Ny - 0.5*Ny;
    ");
	msg("And finally the formulas themselves:");
	ev("
       // Create the random-phase spectrum in k-space:
       Fk = sqrt(0.001*Nx^2+kxphys^2+kyphys^2)^(-SpectralExponent)
             * exp(2i*pi*rand(Nx,Ny));
       // Transform it to x-space ..
       F = Re(invFFT2(Fk));
       // ..and plot it
    ");
	format("Want to use interpolation? [n] ");
	ans = input_string();
	if (ans=="y" || ans=="Y") interpolate=1 else interpolate=3;
	ev("
       pcolor(F/max(abs(F)),\"xmin\",0,\"xmax\",1,\"ymin\",0,\"ymax\",1,
              \"interpolate\",interpolate,\"contbitmap\",\"true\",
              \"toplabel\",\"Power-law spectrum\",
              \"cmin\",-1,\"cmax\",1,\"nsteps\",40,
              \"subtitle\",\"with random phases and spectral index 5/3\");
    ");
} else if (strstarteq(choice,"Bar")) {
	msg("\nTwo bar chart examples from Kenny Toh's PlotMTV manual.");
	msg("Press the arrow buttons on the bottom left to switch graph.\n");
	source("bardemo1.t");
} else if (strstarteq(choice,"BLAS")) {
	source("blasbench.t")
} else if (strstarteq(choice,"Run some correct")) {
	msg("\ntestla - the some linear algebra routines");
	source("testla.t");
	pause();
	msg("\ntestfft - test the FFT routines for 1D data");
	source("testfft.t");
	pause();
	msg("\ntestvecfft - test the FFT routines for multidimensional data");
	source("testvecfft.t");
}

until strstarteq(choice,EndChoice)



