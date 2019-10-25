/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#include "object.H"

#define disp(x) cout << #x " = " << (x) << '\n'
#define disp2(w,x) cout << w " = " << (x) << '\n';

#if 0
static void test1()
{
	Tobject i(1),x(2.001),z(complex(2,3));
	Tint ilist[3] = {1,2,3};
	Tobject iv(ilist,3);
	Treal xlist[4] = {11.5,10.5,9.5,8.5};
	Tobject xv(xlist,4);
	Tcomplex zlist[4] = {20.6,21.6,22.6,23.6};
	Tobject zv(zlist,4);
	disp(i);
	disp(x);
	disp(z);
	disp(iv);
	disp(xv);
	disp(zv);
	Tobject a;
	int I=0;
	Add(a,i,i); disp((2,a));
	Mul(a,i,x); disp((2.001,a));
	Add(a,i,z); disp((3+3*I,a));
	Mul(a,z,z); disp((-5+12*I,a));
	Add(a,i,iv); disp2("i+iv",a);
	Add(a,i,xv); disp2("i+xv",a);
	Add(a,i,zv); disp2("i+zv",a);
	Add(a,z,iv); disp2("z+iv",a);
	Neg(a,a); disp2("-z-iv",a);
}
#endif

#if 0
static void test2()
{
	Tobject i(1),x(2.001),z(complex(2,3));
	Tint ilist[3] = {1,2,3};
	Tobject iv(ilist,3);
	Treal xlist[4] = {11.5,10.5,9.5,8.5};
	Tobject xv(xlist,4);
	Tcomplex zlist[4] = {20.6,21.6,22.6,23.6};
	Tobject zv(zlist,4);
	disp(i);
	disp(x);
	disp(z);
	disp(iv);
	disp(xv);
	disp(zv);
	Tobject a;
	a = x;
	Sin(a,a);
	disp2("sin(2.001)",a);
	a = 1;
	Sin(a,a);
	disp2("sin(1)",a);
	a = 1;
	Add(a,a,a);
	disp2("2",a);
	a = iv;
	Sin(a,a);
	disp2("sin(iv)",a);
	a = xv;
	Sin(a,a);
	disp2("sin(xv)",a);
	a = zv;
	Sin(a,a);
	disp2("sin(zv)",a);
	a = 1;
	Tobject b = 2.01;
	Add(a,a,b);
	disp2("3.01",a);
	a = iv;
	Add(a,z,a);
	disp2("z+iv",a);
}
#endif

static void fn_gaussian(Tobject& y, const Tobject& x)
// y = exp(-x.^2)/sqrt(pi)
{
	Tobject A1,A2;
	Pow(A1,x,2);
	Neg(A1);
	Exp(A1);
	A2 = 3.141592654;
	Sqrt(A2);
	Div(y,A1,A2);
}

int main()
{
	const Treal mm[4] = {1,2,3,4};
	Tobject x(mm,TDimPack(4/*2,2*/)),y;
	disp(x);
	fn_gaussian(y,x);
	disp(y);
	x = complex(2,3);
	Abs(x);
	disp(x);
	return 0;
}
