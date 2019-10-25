dt = 0.1;
L = 10;
tmax = 50;
v = 0.5;
N = 50;
dx = L/(N-3);
x = dx*(0:N-1);
counter = 0;

disp x;
disp pi;
disp L;
disp 2.0*pi*x;
disp 2*pi*x/L;

u = sin(2*pi*x/L);
unew = u;

disp u;

/*
for (t=0; t<tmax; t=t+dt) {
	unew[[2:N-1]] = 0.5*(u[1:N-2]+u[3:N]) - v*dt*(u[3:N] - u[1:N-2])/(2*dx);
	unew[1] = unew[N-1];
	unew[N] = unew[2];
	u = unew;
	//if (counter mod 50 == 0) plot(x,u);
	counter = counter+1;
};
*/

