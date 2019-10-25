x=-4:0.2:4;

/*
plot(x,sin(x),
	    "linewidth",3, "linecolor",6,
	 x,cos(x),
	    "linewidth",6,
	 "ylabel","y", "xlabel","x", "toplabel","Sin and Cos curves",
	 "subtitle","By PlotMTV/Tela");
*/
y=x;
[X,Y] = grid(x,y);
function z=f(x,y) {
	z = exp(-0.35*(x^2+y^2)) + 0.6*(x-1.5)*exp(-0.5*((x-1.5)^2+(y+0.7)^2))
};
/*
contour(f(X,Y),
	"interpolate",3,
	"nsteps",50,
	"meshplot","False",
	"contstyle",2,
	"xlabel","x", "ylabel","y",
	"splinetype",1,
	"toplabel","More than one Gaussian!",
	"subtitle","By PlotMTV/Tela", "comment","Using the contour() command");
*/
contour(f(X,Y));
mesh(f(X,Y),"toplabel","Tela/PlotMTV","subtitle","More than one Gaussian");
pcolor(f(X,Y));
