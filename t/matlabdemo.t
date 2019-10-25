function ev(x) {format("``\n",x); eval(x)};
source("matlabextra");

ev("
    // Starting Matlab, please wait ...
    matlab_start()
");

ev("
    pause();
    // Plot J0(x) using Matlab .. this takes some time also
    x=0:0.1:8;
    []=matlab_call(\"plot\",x,matlab_call(\"besselj\",0,x),\"--\")
");

ev("
    pause();
    // Set the plot title
    []=matlab_call(\"title\",\"This Is Bessel Function J0(x)\")
");

ev("
    pause();
    // Modify colors and line width
    set(get(gca(),\"Child\"),\"LineWidth\",20);
    set(gca(),\"Color\",\"Red\");
    set(gca(),\"XColor\",\"Green\");
    set(gca(),\"YColor\",\"Green\");
");

ev("
    pause();
    // Quit Matlab
    matlab_stop()
");



