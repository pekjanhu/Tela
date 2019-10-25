/* This is a first benchmark for tela.
   CPU seconds for Sumppu (17.1.1994):

   Compiler              CPU-sec.       MIPS     MFLOPS
   --------              -------        ----     ------
   g++ -g              : 11.5           0.07     0.034
   g++ -g -O6          : 2.7            0.3      0.15
   ----------------------------------------------------
   Matlab4.1           : 9.55
   RLaB0.97d -O2	   : 27
*/

imax=100;
jmax=100;
N=10;

A = zeros(imax,jmax);

format("A allocated\n");

for (n=0; n<N; n++)
   for (i=1; i<=imax; i++) for (j=1; j<=jmax; j++)
      A[i,j] = 1/(1+i+j);

format("Benchmark complete, A[``,``] = ``\n",imax-1,jmax-1,A[imax-1,jmax-1]);
