package
global(Mflops,Mips)
{

function y = Mflops(;p)
/* Mflops() returns the approximate rate of millions of floating
   point operations per second since this Tela session was started.
   Mflops(p) where p is performance data array obtained from perf()
   reads the information from p. To measure the MFLOPS rate of your
   code, do something like

   p0=perf(); mycode(); p=perf()-p0; Mflops(p)

   See also: Mips
   */
{
	if (isarray(p)) p1=p else p1=perf();
	[ninstr,nops] = GetInstructionData(p1,"flop");
	y = 1E-6*nops/p1[1,1];
};

function y = Mips(;p)
/* Mips() returns the approximate rate of millions of Flatcode
   instructions per second during this Tela session.
   Mips(p) where p is performance data array obtained from perf()
   reads the information from p. To measure the MIPS rate of your
   code, do something like

   p0=perf(); mycode(); p=perf()-p0; Mips(p)

   See also: Mflops
   */
{
	if (isarray(p)) p1=p else p1=perf();
	[ninstr,nops] = GetInstructionData(p1,"*");
	y = 1E-6*ninstr/p1[1,1];
};

}

