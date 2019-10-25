/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#include "prg.H"

#define disp(x) cout << #x " = " << (x) << '\n'
#define disp2(w,x) cout << w " = " << (x) << '\n';

/*
  First speed measurements (LINUX) 19.12.1993:
  
  11      ADD     sum,i
  14      ADD     i,#I(1)
  17      LT      tmp,i,#I(100000)
  21      JIF     11,tmp

  yielded 1223 clock cycles per iteration, i.e. about 300 cc per instruction,
  or about 0.1MIPS. -O6 was used. If no optimization, about 3 times slower.
*/


#if 0
int main()
{
	Tprg prg;
	Tsymbol sym1("AA");
	Tobject obj1(5.6,-2.1);
	sym1.value() = &obj1;
	Toperand op1(Klit,234);
	Toperand op2(sym1);
	Toperand op3(Kpar,0);
	Toperand op4(Kpar,1);
	Toperand op5(Tobject(3.14,2));
	Toperand op6(1111);
	Tcomplex ztab[6] = {1,2,3,4,5,6};
	Toperand op7(Tobject(ztab,TDimPack(2,3)));
	disp(op1);
	disp(op2);
	disp(op3);
	disp(op4);
	disp(op5);
	disp(op6);
	disp(op7);
	prg.add(Fneg,op1);
	prg.add(Fpri,op2);
	prg.add(Fdiv,op3,op4,op5);
	prg.add(Fmul,op3,op7,op6);
	disp(prg);
	prg.execute();
	return 0;
}
#endif

void gaussian()
{
	Tprg prg;

	Treal xtab[100];
	for (int i=0; i<100; i++) xtab[i] = sin(0.01*i);
	
	Tobject objPi(3.141592654);
	Tobject objUnit(1.0);
	Tobject objTwo(2);
	Tobject objX(xtab,100);
	
	Tsymbol reg0("reg0");
	Tsymbol reg1("reg1");
	Tsymbol reg2("reg2");
	Tsymbol reg3("reg3");

	prg.add(Fmove,Toperand(reg1),Toperand(objX));
	prg.add(Fmove,Toperand(reg2),Toperand(objPi));
	prg.add(Fsqrt,Toperand(reg2));
	prg.add(Fdiv,Toperand(reg2),Toperand(objUnit),Toperand(reg2));
	prg.add(Fpow,Toperand(reg3),Toperand(reg1),Toperand(objTwo));
	prg.add(Fneg,Toperand(reg3));
	//prg.add(Fbra,Toperand(Klit,3));
	prg.add(Fexp,Toperand(reg3));
	prg.add(Fmul,Toperand(reg0),Toperand(reg2),Toperand(reg3));
	prg.add(Fpri,Toperand(reg0));
	//prg.add(Fbra,Toperand(Klit,-29));
	
	disp(prg);
	prg.execute();
}

void looping()
{
	Tprg prg;

	Tobject objZero(0);
	Tobject objUnit(1);
	Tobject objTen(10);

	Tsymbol i("i");
	Tsymbol tmp("tmp");

	prg.add(Fmove,Toperand(i),Toperand(objZero));
	int pc = prg.length();
	prg.add(Fpri,Toperand(i));
	prg.add(Fadd,Toperand(i),Toperand(objUnit));
	prg.add(Flt,Toperand(tmp),Toperand(i),Toperand(objTen));
	//prg.add(Fbif,Toperand(Klit,-11),Toperand(tmp));
	prg.add(Fjif,Toperand(Klit,pc),Toperand(tmp));

	disp(prg);
	prg.execute();
}

void summing()
{
	const int N=100000;
	Tprg prg;

	Tobject objN(N);
	Tobject objZero(0);
	Tobject objRzero(0.0);
	Tobject objUnit(1);
	Tsymbol i("i");
	Tsymbol V("V");
	Tsymbol sum("sum");
	Tsymbol tmp("tmp");

	prg.add(Fpri,Toperand(objZero));
	prg.add(Fmove,Toperand(sum),Toperand(objRzero));
	prg.add(Frzeros,Toperand(V),Toperand(objN));
	prg.add(Fmove,Toperand(i),Toperand(objZero));
	int pc = prg.length();
	prg.add(Fadd,Toperand(sum),Toperand(i));
	prg.add(Fadd,Toperand(i),Toperand(objUnit));
	prg.add(Flt,Toperand(tmp),Toperand(i),Toperand(objN));
	prg.add(Fjif,Toperand(Klit,pc),Toperand(tmp));
	
	prg.add(Fpri,Toperand(sum));

	disp(prg);
	float t1=CPUSeconds();
	prg.execute();
	float t2=CPUSeconds();
	float cc=(t2-t1)*Hz/N;
	disp(cc);
}

void flopping()
{
	const int N=4000;
	const int M=1000;
	Tprg prg;

	Tobject objN(N);
	Tobject objM(M);
	Tobject objZero(0);
	Tobject objUnit(1);
	Tsymbol i("i");
	Tsymbol V("V");
	Tsymbol tmp("tmp");

	prg.add(Fizeros,Toperand(V),Toperand(objN));
	prg.add(Fmove,Toperand(i),Toperand(objZero));
	int pc = prg.length();
	prg.add(Fadd,Toperand(V),Toperand(V));
	prg.add(Fadd,Toperand(i),Toperand(objUnit));
	prg.add(Flt,Toperand(tmp),Toperand(i),Toperand(objM));
	prg.add(Fjif,Toperand(Klit,pc),Toperand(tmp));
	
	disp(prg);
	float t1=CPUSeconds();
	prg.execute();
	float t2=CPUSeconds();
	float cc=(t2-t1)*Hz/(N*M);
	disp(cc);
}

void indexing()
{
	Tprg prg;

	const int N=10;
	Treal xtab[N];
	for (int i=0; i<N; i++) xtab[i] = sin(i/Treal(N));
	
	Tobject objX(xtab,TDimPack(2,5));
	Tobject ii(1);
	Tobject jj(3);
	Tobject newval(-2.0);
	
	Tsymbol reg0("reg0");
	Tsymbol reg1("reg1");

	prg.add(Fmove,Toperand(reg1),Toperand(objX));
	prg.add(Fgath,Toperand(reg0),Toperand(reg1),Toperand(ii),Toperand(jj));
	prg.add(Fpri,Toperand(reg0));
	prg.add(Fscat,Toperand(newval),Toperand(reg1),Toperand(ii),Toperand(jj));
	prg.add(Fpri,Toperand(reg1));
	Tint itab[2],jtab[2];
	itab[0]=1; itab[1]=0; jtab[0]=2; jtab[1]=3;
	Tobject iitab(itab,2), jjtab(jtab,2);
	prg.add(Fmgath,Toperand(reg0),Toperand(reg1),Toperand(iitab),Toperand(jjtab));
	prg.add(Fpri,Toperand(reg0));
	prg.add(Fscat,Toperand(newval),Toperand(reg0),Toperand(ii));
	prg.add(Fpri,Toperand(reg0));
	prg.add(Fmscat,Toperand(reg0),Toperand(reg1),Toperand(iitab),Toperand(jjtab));
	prg.add(Fpri,Toperand(reg1));
	
	disp(prg);
	prg.execute();
}

int main()
{
	//gaussian();
	//looping();
	//summing();
	//flopping();
	indexing();
	PerformanceReport();
	return 0;
}
