/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "objarithm.H"
#endif
#include "objarithm.H"
#include <ctype.h>

#ifdef VECTOR_MACHINE
#  include "complexvec.C"
#endif

/* For megaflops recoding only: */
#define REAL_COMPLEX_DIV_OPS 7
#define COMPLEX_REAL_DIV_OPS 2
#define COMPLEX_COMPLEX_DIV_OPS 13

static void LengthError(const char *op, const Tobject& a, const Tobject& b) {
	err << "Dimension mismatch with " << Tshort(a) << ' ' << op << ' ' << Tshort(b) << '\n';
	error();
}

static void UnaryError(const char *op, const Tobject& a) {
	err << "Unary '" << op << "' applied on " << Tshort(a) << '\n';
	error();
}

static void BinError(const char *op, const Tobject& a, const Tobject& b) {
	err << "Bad binary operator arguments: " << Tshort(a) << ' ' << op << ' ' << Tshort(b) << '\n';
	error();
}

#define pair(ka,kb) ((ka)*(int(Kundef)+1)+(kb))		/* We trust the Kundef is the LAST one in enum Tkind (object.H) */

#if SUPPORT_LOOP_TILING
#  define GetLoopLimits(i1,i2) if (global::tile_in_use) {i1=global::tile_start; i2=global::tile_end;} else {i1=0; i2=N-1;}
#else
#  define GetLoopLimits(i1,i2) {i1=0; i2=N-1;}
#endif

// Binary operators +,-,*,/,Pow
// This is because '/' is not integer but real division.

#define OP(a,b) ((a)+(b))
#define OPCHAR "+"
#define OPFNAME Add
#include "templ/tbinop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)-(b))
#define OPCHAR "-"
#define OPFNAME Sub
#include "templ/tbinop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)*(b))
#define OPCHAR "*"
#define OPFNAME Mul
#include "templ/tbinop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// The Div operation does not fit the standard templates
// since the result is always at least real and because
// we want to replace division by inverse multiplication
// whenever possible.

void Div(Tobject& c, const Tobject& a, const Tobject& b) {
	int i,N,i1,i2;
	Treal X;
	static char OPCHAR[] = "/";
	switch (pair(a.kind(),b.kind())) {
	case pair(Kint,Kint):
		c = Treal(a.IntValue())/Treal(b.IntValue());
		global::nops++;
		break;
	case pair(Kint,Kreal):
		c = Treal(a.IntValue())/b.RealValue();
		global::nops++;
		break;
	case pair(Kint,Kcomplex):
		c = Tcomplex(a.IntValue(),0.0)/b.ComplexValue();
		global::nops+= REAL_COMPLEX_DIV_OPS;
		break;
	case pair(Kint,KIntArray):
		{
			X = Treal(a.IntValue());
			N = b.length();
			Tint *bp = b.IntPtr();
			GetLoopLimits(i1,i2);
			if (&b == &c) {
				Tobject c1;
				c1.copydimsRealArray(b);
				Treal *c1p = c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = X/Treal(bp[i]);
				c = c1;
			} else {
				c.copydimsRealArray(b);
				Treal *cp = c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = X/Treal(bp[i]);
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(Kint,KRealArray):
		{
			X = Treal(a.IntValue());
			N = b.length();
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(b);
			Treal *cp=c.RealPtr(), *bp=b.RealPtr();
			for (i=i1; i<=i2; i++) cp[i] = X/bp[i];
			global::nops+= i2-i1+1;
		}
		break;
	case pair(Kint,KComplexArray):
		{
			Tcomplex Z(a.IntValue(),0.0);
			N = b.length();
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(b);
			Tcomplex *cp=c.ComplexPtr(), *bp=b.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Div(cp,Z,bp,N);
#			else
			for (i=i1; i<=i2; i++) cp[i] = Z/bp[i];
#			endif
			global::nops+= (i2-i1+1)*REAL_COMPLEX_DIV_OPS;
		}
		break;
	case pair(Kreal,Kint):
		c = a.RealValue()/Treal(b.IntValue());
		global::nops++;
		break;
	case pair(Kreal,Kreal):
		c = a.RealValue()/b.RealValue();
		global::nops++;
		break;
	case pair(Kreal,Kcomplex):
		c = Tcomplex(a.RealValue(),0.0)/b.ComplexValue();
		global::nops+= REAL_COMPLEX_DIV_OPS;
		break;
	case pair(Kreal,KIntArray):
		{
			N = b.length();
			X = a.RealValue();
			Tint *bp=b.IntPtr();
			GetLoopLimits(i1,i2);
			if (&c == &b) {
				Tobject c1;
				c1.copydimsRealArray(b);
				Treal *c1p=c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = X/bp[i];
				c = c1;
			} else {
				c.copydimsRealArray(b);	// make c a real array of the same dims as b
				Treal *cp=c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = X/bp[i];
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(Kreal,KRealArray):
		{
			N = b.length();
			X = a.RealValue();
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(b);
			Treal *cp=c.RealPtr(), *bp=b.RealPtr();
			for (i=i1; i<=i2; i++) cp[i] = X/bp[i];
			global::nops+= i2-i1+1;
		}
		break;
	case pair(Kreal,KComplexArray):
		{
			Tcomplex Z(a.RealValue(),0.0);
			N = b.length();
			c.copydimsComplexArray(b);
			GetLoopLimits(i1,i2);
			Tcomplex *cp=c.ComplexPtr(), *bp=b.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Div(&cp[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = Z/bp[i];
#			endif
			global::nops+= (i2-i1+1)*REAL_COMPLEX_DIV_OPS;
		}
		break;
	case pair(Kcomplex,Kint):
		c = a.ComplexValue()/Tcomplex(b.IntValue(),0.0);
		global::nops+= COMPLEX_REAL_DIV_OPS;
		break;
	case pair(Kcomplex,Kreal):
		c = a.ComplexValue()/Tcomplex(b.RealValue(),0.0);
		global::nops+= COMPLEX_REAL_DIV_OPS;
		break;
	case pair(Kcomplex,Kcomplex):
		c = a.ComplexValue()/b.ComplexValue();
		global::nops+= COMPLEX_COMPLEX_DIV_OPS;
		break;
	case pair(Kcomplex,KIntArray):
		{
			N = b.length();
			Tcomplex Z(a.ComplexValue());
			Tint *bp=b.IntPtr();
			GetLoopLimits(i1,i2);
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&c1p[i1],Z,&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = Z/bp[i];
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(b);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&cp[i1],Z,&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = Z/bp[i];
#				endif
			}
			global::nops+= (i2-i1+1)*COMPLEX_REAL_DIV_OPS;
		}
		break;
	case pair(Kcomplex,KRealArray):
		{
			N = b.length();
			Tcomplex Z(a.ComplexValue());
			GetLoopLimits(i1,i2);
			Treal *bp=b.RealPtr();
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&c1p[i1],Z,&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = Z/bp[i];
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(b);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&cp[i1],Z,&bp[i2],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = Z/bp[i];
#				endif
			}
			global::nops+= (i2-i1+1)*COMPLEX_REAL_DIV_OPS;
		}
		break;
	case pair(Kcomplex,KComplexArray):
		{
			N = b.length();
			Tcomplex Z(a.ComplexValue());
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(b);
			Tcomplex *cp=c.ComplexPtr(), *bp=b.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Div(&cp[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = Z/bp[i];
#			endif
			global::nops+= (i2-i1+1)*COMPLEX_COMPLEX_DIV_OPS;
		}
		break;
	case pair(KIntArray,Kint):
		{
			X = 1.0/Treal(b.IntValue());
			N = a.length();
			Tint *ap=a.IntPtr();
			GetLoopLimits(i1,i2);
			if (&a == &c) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal *c1p=c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]*X;
				c = c1;
			} else {
				c.copydimsRealArray(a);
				Treal *cp=c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = ap[i]*X;
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KIntArray,Kreal):
		{
			N = a.length();
			X = 1.0/b.RealValue();
			Tint *ap=a.IntPtr();
			GetLoopLimits(i1,i2);
			if (&c == &a) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal *c1p=c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]*X;
				c = c1;
			} else {
				c.copydimsRealArray(a);
				Treal *cp=c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = ap[i]*X;
			}
			global::nops+= (i2-i1+1);
		}
		break;
	case pair(KIntArray,Kcomplex):
		{
			N = a.length();
			Tcomplex Z = 1.0/b.ComplexValue();
			Tint *ap=a.IntPtr();
			GetLoopLimits(i1,i2);
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Mul(&c1p[i1],&ap[i1],Z,i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]*Z;
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(a);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Mul(&cp[i1],&ap[i1],Z,i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = ap[i]*Z;
#				endif
			}
			global::nops+= (i2-i1+1)*REAL_COMPLEX_DIV_OPS;
		}
		break;
	case pair(KIntArray,KIntArray):
		N = a.length();
		//if (N != b.length()) LengthError(OPCHAR,a,b);
		if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
		GetLoopLimits(i1,i2);
		if (&c == &a || &c == &b) {
			Tobject c1;
			c1.copydimsRealArray(a);
			Treal *c1p=c1.RealPtr(); Tint *ap=a.IntPtr(), *bp=b.IntPtr();
			for (i=i1; i<=i2; i++) c1p[i] = Treal(ap[i])/Treal(bp[i]);
			c = c1;
		} else {
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(); Tint *ap=a.IntPtr(), *bp=b.IntPtr();
			for (i=i1; i<=i2; i++) cp[i] = Treal(ap[i])/Treal(bp[i]);
		}
		global::nops+= i2-i1+1;
		break;
	case pair(KIntArray,KRealArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			GetLoopLimits(i1,i2);
			if (&a == &c) {
				Tobject c1;
				c1.copydimsRealArray(b);
				Treal *cp=c1.RealPtr(), *bp=b.RealPtr(); Tint *ap=a.IntPtr();
				for (i=i1; i<=i2; i++) cp[i] = Treal(ap[i])/bp[i];
				c = c1;
			} else {
				c.copydimsRealArray(b);
				Treal *cp=c.RealPtr(), *bp=b.RealPtr(); Tint *ap=a.IntPtr();
				for (i=i1; i<=i2; i++) cp[i] = Treal(ap[i])/bp[i];
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KIntArray,KComplexArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tint *ap=a.IntPtr(); Tcomplex *bp=b.ComplexPtr();
			GetLoopLimits(i1,i2);
			if (&a == &c) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]/bp[i];
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(b);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = ap[i]/bp[i];
#				endif
			}
			global::nops+= (i2-i1+1)*REAL_COMPLEX_DIV_OPS;
		}
		break;
	case pair(KRealArray,Kint):
		{
			X = 1.0/Treal(b.IntValue());
			N = a.length();
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(), *ap=a.RealPtr();
			for (i=i1; i<=i2; i++) cp[i] = ap[i]*X;
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,Kreal):
		{
			N = a.length();
			X = 1.0/b.RealValue();
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(), *ap=a.RealPtr();
			for (i=i1; i<=i2; i++) cp[i] = ap[i]*X;
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,Kcomplex):
		{
			N = a.length();
			Treal *ap=a.RealPtr();
			Tcomplex Z = 1/b.ComplexValue();
			GetLoopLimits(i1,i2);
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Mul(&c1p[i1],&ap[i1],Z,i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]*Z;
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(a);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Mul(&cp[i1],&ap[i1],Z,i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = ap[i]*Z;
#				endif
			}
			global::nops+= (i2-i1+1)*REAL_COMPLEX_DIV_OPS;
		}
		break;
	case pair(KRealArray,KIntArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tint *bp=b.IntPtr();
			Treal *ap=a.RealPtr();
			GetLoopLimits(i1,i2);
			if (&b == &c) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal *c1p=c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]/Treal(bp[i]);
				c = c1;
			} else {
				c.copydimsRealArray(a);
				Treal *cp=c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = ap[i]/Treal(bp[i]);
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,KRealArray):
		{
			N = a.length();
			GetLoopLimits(i1,i2);
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(), *ap=a.RealPtr(), *bp=b.RealPtr();
			for (i=i1; i<=i2; i++) cp[i] = ap[i]/bp[i];
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,KComplexArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Treal *ap=a.RealPtr(); Tcomplex *bp=b.ComplexPtr();
			GetLoopLimits(i1,i2);
			if (&a == &c) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]/bp[i];
#				endif
			} else {
				c.copydimsComplexArray(b);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = ap[i]/bp[i];
#				endif
			}
			global::nops+= (i2-i1+1)*REAL_COMPLEX_DIV_OPS;
		}
		break;
	case pair(KComplexArray,Kint):
		{
			N = a.length();
			Tcomplex Z = 1/Tcomplex(b.IntValue(),0.0);
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(a);
			Tcomplex *cp=c.ComplexPtr(), *ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Mul(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = ap[i]*Z;
#			endif
			global::nops+= (i2-i1+1)*COMPLEX_REAL_DIV_OPS;
		}
		break;
	case pair(KComplexArray,Kreal):
		{
			N = a.length();
			X = 1.0/b.RealValue();
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(a);
			Tcomplex *cp=c.ComplexPtr(), *ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Mul(&cp[i1],&ap[i1],X,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = ap[i]*X;
#			endif
			global::nops+= (i2-i1+1)*COMPLEX_REAL_DIV_OPS;
		}
		break;
	case pair(KComplexArray,Kcomplex):
		{
			N = a.length();
			Tcomplex Z = 1/b.ComplexValue();
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(a);
			Tcomplex *cp=c.ComplexPtr(), *ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Mul(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = ap[i]*Z;
#			endif
			global::nops+= (i2-i1+1)*COMPLEX_COMPLEX_DIV_OPS;
		}
		break;
	case pair(KComplexArray,KIntArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			GetLoopLimits(i1,i2);
			Tcomplex *ap=a.ComplexPtr(); Tint *bp=b.IntPtr();
			if (&b == &c) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]/bp[i];
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(a);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = ap[i]/bp[i];
#				endif
			}
			global::nops+= (i2-i1+1)*COMPLEX_REAL_DIV_OPS;
		}
		break;
	case pair(KComplexArray,KRealArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tcomplex *ap=a.ComplexPtr(); Treal *bp=b.RealPtr();
			GetLoopLimits(i1,i2);
			if (&b == &c) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex *c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = ap[i]/bp[i];
#				endif
				c = c1;
			} else {
				c.copydimsComplexArray(a);
				Tcomplex *cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				Div(&cp[i1],&ap[i1],&bp[i1],i2-i2+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = ap[i]/bp[i];
#				endif
			}
			global::nops+= (i2-i1+1)*COMPLEX_REAL_DIV_OPS;
		}
		break;
	case pair(KComplexArray,KComplexArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(b);
			Tcomplex *cp=c.ComplexPtr(), *bp=b.ComplexPtr(), *ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			Div(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = ap[i]/bp[i];
#			endif
			global::nops+= (i2-i2+1)*COMPLEX_COMPLEX_DIV_OPS;
		}
		break;
	default:
		BinError(OPCHAR,a,b);
		break;
	}
} // Div

#if defined(HPUX) && !defined(__GNUC__)
  // HPUX apparently has prototype for pow(double,int) in the system
  inline Treal Pow(Treal x, Tint y) {return pow(x,y);}
  inline Tint Pow(Tint x, Tint y) {return iround(pow(Treal(x),y));}
#else
  // Other systems don't, so we use the ordinary (double,double) version
  inline Treal Pow(Treal x, Tint y) {return pow(x,Treal(y));}
  inline Tint Pow(Tint x, Tint y) {return iround(pow(Treal(x),Treal(y)));}
#endif
inline Treal Pow(Tint x, Treal y) {return pow(Treal(x),y);}
inline Treal Pow(Treal x, Treal y) {return pow(x,y);}
inline Tcomplex Pow(const Tcomplex& x, const Tcomplex& y) {return pow(x,y);}
inline Tcomplex Pow(const Tcomplex& x, Treal y) {return pow(x,y);}
inline Tcomplex Pow(const Tcomplex& x, Tint y) {return pow(x,y);}

static void Pow(Treal y[], const Treal x[], const Treal a, const int N) {
	for (int i=0; i<N; i++) y[i] = Pow(x[i],a);
}

static void Pow(Treal y[], const Treal a, const Treal x[], const int N) {
	for (int i=0; i<N; i++) y[i] = Pow(a,x[i]);
}

static void Pow(Treal z[], const Treal x[], const Treal y[], const int N) {
	for (int i=0; i<N; i++) z[i] = Pow(x[i],y[i]);
}

static int AnyNegative(const Tint x[], const int N) {
	for (int i=0; i<N; i++) if (x[i] < 0) return 1;
	return 0;
}

static int AnyNegative(const Treal x[], const int N) {
	for (int i=0; i<N; i++) if (x[i] < 0) return 1;
	return 0;
}

#ifndef pi
#  define pi 3.141592653589793
#endif

void Pow(Tobject& c, const Tobject& a, const Tobject& b) {
	int i,N;
	Tint I; Treal X;
	switch (pair(a.kind(),b.kind())) {
	case pair(Kint,Kint):
		// Bug fixed 5.6.1997. Take into account that if b is negative, c becomes non-integer (real)
		// and if also a is negative, the result is complex.
		// This is in four places: (Kint,Kint), (Kint,KIntArray), (KIntArray,Kint), (KIntArray,KIntArray)
		if (b.IntValue() < 0) {
			if (a.IntValue() < 0)
				c = Pow(Tcomplex(a.IntValue(),0.0),Tcomplex(b.IntValue(),0.0));
			else
				c = Pow(a.IntValue(),Treal(b.IntValue()));
		} else
			c = Pow(a.IntValue(),b.IntValue());
		global::nops++;
		break;
	case pair(Kint,Kreal):
		if (a.IntValue() < 0) {		// Negative base produces complex result!
			const Treal r = Pow(-a.IntValue(),b.RealValue());
			c = Tcomplex(cos(pi*b.RealValue())*r, sin(pi*b.RealValue())*r);
		} else
			c = Pow(a.IntValue(),b.RealValue());
		global::nops++;
		break;
	case pair(Kint,Kcomplex):
		c = Pow(Tcomplex(a.IntValue(),0.0),b.ComplexValue());
		global::nops++;
		break;
	case pair(Kint,KIntArray):
		{
			// Bug fixed 5.6.1997. Take into account that if b is negative, c becomes non-integer
			// and if also a is negative, c becomes complex.
			I = a.IntValue();
			N = b.length();
			if (AnyNegative(b.IntPtr(),N)) {
				Tint * const bp=b.IntPtr();
				if (I < 0) {
					if (&c == &b) {		// Bug fixed 2.11.2001: Forgot to test what happens if &c==&b
						Tobject c1;
						c1.copydimsComplexArray(b);
						Tcomplex * const cp=c1.ComplexPtr();
						for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(I,0.0),Tcomplex(bp[i],0.0));
						c.bitwiseMoveFrom(c1);
					} else {
						c.copydimsComplexArray(b);
						Tcomplex * const cp=c.ComplexPtr();
						for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(I,0.0),Tcomplex(bp[i],0.0));
					}
				} else {
					if (&c == &b) {		// Bug fixed 2.11.2001: Forgot to test what happens if &c==&b
						Tobject c1;
						c1.copydimsRealArray(b);
						Treal * const cp=c1.RealPtr();
						for (i=0; i<N; i++) cp[i] = Pow(I,Treal(bp[i]));
						c.bitwiseMoveFrom(c1);
					} else {
						c.copydimsRealArray(b);
						Treal * const cp=c.RealPtr();
						for (i=0; i<N; i++) cp[i] = Pow(I,Treal(bp[i]));
					}
				}
			} else {
				c.copydimsIntArray(b);
				Tint * const cp=c.IntPtr(), * const bp=b.IntPtr();
				for (i=0; i<N; i++) cp[i] = Pow(I,bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(Kint,KRealArray):
		{
			N = b.length();
			Treal * const bp=b.RealPtr();
			if (a.IntValue() < 0) {			// Negative base produces complex result!
				if (&c == &b) {		// Bug fixed 2.11.2001: Forgot to test what happens if &c==&b
					Tobject c1;
					c1.copydimsComplexArray(b);
					Tcomplex * const cp=c1.ComplexPtr();
					I = -a.IntValue();
					for (i=0; i<N; i++) {
						const Treal r = Pow(I,bp[i]);
						cp[i] = Tcomplex(cos(pi*bp[i])*r, sin(pi*bp[i])*r);
					}
					c.bitwiseMoveFrom(c1);
				} else {
					c.copydimsComplexArray(b);
					Tcomplex * const cp=c.ComplexPtr();
					I = -a.IntValue();
					for (i=0; i<N; i++) {
						const Treal r = Pow(I,bp[i]);
						cp[i] = Tcomplex(cos(pi*bp[i])*r, sin(pi*bp[i])*r);
					}
				}
			} else {
				c.copydimsRealArray(b);
				Treal * const cp=c.RealPtr();
				Pow(cp,Treal(a.IntValue()),bp,N);
			}
			global::nops+= N;
		}
		break;
	case pair(Kint,KComplexArray):
		{
			Tcomplex Z(a.IntValue(),0.0);
			N = b.length();
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(Z,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(Kreal,Kint):
		// We rely on that pow(<negative real>, <integer-valued real>) produces meaningful result, as it should
		c = Pow(a.RealValue(),Treal(b.IntValue()));
		global::nops++;
		break;
	case pair(Kreal,Kreal):
		if (a.RealValue() < 0) {		// Negative base produces complex result!
			const Treal r = pow(-a.RealValue(),b.RealValue());
			c = Tcomplex(cos(pi*b.RealValue())*r, sin(pi*b.RealValue())*r);
		} else
			c = Pow(a.RealValue(),b.RealValue());
		global::nops++;
		break;
	case pair(Kreal,Kcomplex):
		c = Pow(Tcomplex(a.RealValue(),0.0),b.ComplexValue());
		global::nops++;
		break;
	case pair(Kreal,KIntArray):
		// We rely on that pow(<negative real>, <integer-valued real>) produces meaningful result, as it should
		N = b.length();
		X = a.RealValue();
		if (&c == &b) {
			Tobject c1;
			c1.copydimsRealArray(b);
			Treal * const c1p=c1.RealPtr(); Tint * const bp=b.IntPtr();
			for (i=0; i<N; i++) c1p[i] = Pow(X,bp[i]);
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsRealArray(b);	// make c a real array of the same dims as b
			Treal * const cp=c.RealPtr(); Tint * const bp=b.IntPtr();
			for (i=0; i<N; i++) cp[i] = Pow(X,bp[i]);
		}
		global::nops+= N;
		break;
	case pair(Kreal,KRealArray):
		{
			N = b.length();
			X = a.RealValue();
			Treal * const bp=b.RealPtr();
			if (X < 0) {		// Negative base produces complex result!
				if (&c == &b) {		// Bug fixed 2.11.2001: Forgot to test what happens if &c==&b
					Tobject c1;
					c1.copydimsComplexArray(b);
					Tcomplex * const cp=c1.ComplexPtr();
					for (i=0; i<N; i++) {
						const Treal r = pow(-X,bp[i]);
						cp[i] = Tcomplex(cos(pi*bp[i])*r, sin(pi*bp[i])*r);
					}
					c.bitwiseMoveFrom(c1);
				} else {
					c.copydimsComplexArray(b);
					Tcomplex * const cp=c.ComplexPtr();
					for (i=0; i<N; i++) {
						const Treal r = pow(-X,bp[i]);
						cp[i] = Tcomplex(cos(pi*bp[i])*r, sin(pi*bp[i])*r);
					}
				}
			} else {
				c.copydimsRealArray(b);
				Treal * const cp=c.RealPtr();
				Pow(cp,X,bp,N);
			}
			global::nops+= N;
		}
		break;
	case pair(Kreal,KComplexArray):
		{
			Tcomplex Z(a.RealValue(),0.0);
			N = b.length();
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(Z,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(Kcomplex,Kint):
		c = Pow(a.ComplexValue(),b.IntValue());
		global::nops++;
		break;
	case pair(Kcomplex,Kreal):
		c = Pow(a.ComplexValue(),b.RealValue());
		global::nops++;
		break;
	case pair(Kcomplex,Kcomplex):
		c = Pow(a.ComplexValue(),b.ComplexValue());
		global::nops++;
		break;
	case pair(Kcomplex,KIntArray):
		{
			N = b.length();
			Tcomplex Z(a.ComplexValue());
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr(); Tint * const bp=b.IntPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Z,bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tint * const bp=b.IntPtr();
				Tcomplex *cp=c.ComplexPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Z,bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(Kcomplex,KRealArray):
		{
			N = b.length();
			Tcomplex Z(a.ComplexValue());
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr(); Treal * const bp=b.RealPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Z,bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tcomplex * const cp=c.ComplexPtr(); Treal * const bp=b.RealPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Z,bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(Kcomplex,KComplexArray):
		{
			N = b.length();
			Tcomplex Z(a.ComplexValue());
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(Z,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kint):
		{
			// Bug fixed 5.6.1997. Take into account that if b is negative, c becomes non-integer
			// and if also a is negative, c becomes complex.
			I = b.IntValue();
			N = a.length();
			if (I < 0) {
				Tint * const ap=a.IntPtr();
				if (AnyNegative(ap,N)) {
					if (&c == &a) {		// Bug fixed 2.11.2001: Forgot to test what happens if &c==&a
						Tobject c1;
						c1.copydimsComplexArray(a);
						Tcomplex * const cp=c1.ComplexPtr();
						for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(ap[i],0.0),Tcomplex(I,0.0));
						c.bitwiseMoveFrom(c1);
					} else {
						c.copydimsComplexArray(a);
						Tcomplex * const cp=c.ComplexPtr();
						for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(ap[i],0.0),Tcomplex(I,0.0));
					}
				} else {
					if (&c == &a) {		// Bug fixed 2.11.2001: Forgot to test what happens if &c==&a
						Tobject c1;
						c1.copydimsRealArray(a);
						Treal * const cp=c1.RealPtr();
						for (i=0; i<N; i++) cp[i] = Pow(ap[i],Treal(I));
						c.bitwiseMoveFrom(c1);
					} else {
						c.copydimsRealArray(a);
						Treal * const cp=c.RealPtr();
						for (i=0; i<N; i++) cp[i] = Pow(ap[i],Treal(I));
					}
				}
			} else {
				c.copydimsIntArray(a);
				Tint * const cp=c.IntPtr(), * const ap=a.IntPtr();
				for (i=0; i<N; i++) cp[i] = Pow(ap[i],I);
			}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kreal):
		N = a.length();
		X = b.RealValue();
		{
			Tint * const ap=a.IntPtr();
			if (AnyNegative(ap,N)) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p = c1.ComplexPtr();
				const Treal co = cos(pi*X), si = sin(pi*X);
				for (i=0; i<N; i++) {
					if (ap[i] < 0) {
						const Treal r = Pow(-ap[i],X);
						c1p[i] = Tcomplex(co*r, si*r);
					} else {
						c1p[i] = Tcomplex(Pow(ap[i],X),0.0);		// imaginary part becomes zero
					}
				}
				c.bitwiseMoveFrom(c1);
			} else if (&c == &a) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal * const c1p=c1.RealPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(ap[i],X);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(a);
				Treal * const cp=c.RealPtr();
				for (i=0; i<N; i++) cp[i] = Pow(ap[i],X);
			}
		}
		global::nops+= N;
		break;
	case pair(KIntArray,Kcomplex):
		{
			N = a.length();
			Tcomplex Z(b.ComplexValue());
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr(); Tint * const ap=a.IntPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Tcomplex(ap[i],0.0),Z);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(a);
				Tcomplex * const cp=c.ComplexPtr(); Tint * const ap=a.IntPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(ap[i],0.0),Z);
			}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KIntArray):
		{
			// Bug fixed 5.6.1997. Take into account that if b is negative, c becomes non-integer
			// and if also a is negative, c becomes complex.
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			if (AnyNegative(b.IntPtr(),N)) {
				Tint * const ap=a.IntPtr(), * const bp=b.IntPtr();
				if (AnyNegative(a.IntPtr(),N)) {
					Tobject c1;		// Fixed 2.11.2001: Safe also in case &c==&b
					c1.copydimsComplexArray(a);
					Tcomplex * const cp=c1.ComplexPtr();
					for (i=0; i<N; i++) cp[i] = Tcomplex(Pow(ap[i],Treal(bp[i])),0.0);
					c.bitwiseMoveFrom(c1);
				} else {
					Tobject c1;
					c1.copydimsRealArray(a);
					Treal * const cp=c1.RealPtr();
					for (i=0; i<N; i++) cp[i] = Pow(ap[i],Treal(bp[i]));
					c.bitwiseMoveFrom(c1);
				}
			} else {
				c.copydimsIntArray(a);
				Tint * const cp=c.IntPtr(), * const ap=a.IntPtr(), * const bp=b.IntPtr();
				for (i=0; i<N; i++) cp[i] = Pow(ap[i],bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KRealArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Treal * const bp=b.RealPtr(); Tint * const ap=a.IntPtr();
			if (AnyNegative(ap,N)) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr();
				for (i=0; i<N; i++) {
					if (ap[i] < 0) {
						const Treal r = Pow(Treal(-ap[i]),bp[i]);
						c1p[i] = Tcomplex(cos(pi*bp[i])*r, sin(pi*bp[i])*r);
					} else
						c1p[i] = Tcomplex(Pow(Treal(ap[i]),bp[i]),0.0);
				}
				c.bitwiseMoveFrom(c1);
			} else if (&a == &c) {
				Tobject c1;
				c1.copydimsRealArray(b);
				Treal * const c1p=c1.RealPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Treal(ap[i]),bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(b);
				Treal * const cp=c.RealPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Treal(ap[i]),bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KComplexArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Tcomplex * const bp=b.ComplexPtr(); Tint * const ap=a.IntPtr();
			if (&a == &c) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Tcomplex(ap[i],0.0),bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tcomplex * const cp=c.ComplexPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(ap[i],0.0),bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kint):
		{
			X = Treal(b.IntValue());
			N = a.length();
			c.copydimsRealArray(a);
			Treal * const cp=c.RealPtr(), * const ap=a.RealPtr();
			Pow(cp,ap,X,N);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kreal):
		{
			N = a.length();
			X = b.RealValue();
			Treal * const ap=a.RealPtr();
			if (AnyNegative(ap,N)) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr();
				const Treal co = cos(pi*X), si = sin(pi*X);
				for (i=0; i<N; i++) {
					if (ap[i] < 0) {
						const Treal r = Pow(-ap[i],X);
						c1p[i] = Tcomplex(co*r, si*r);
					} else
						c1p[i] = Tcomplex(Pow(ap[i],X),0.0);
				}
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(a);
				Treal * const cp=c.RealPtr();
				Pow(cp,ap,X,N);
			}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kcomplex):
		{
			N = a.length();
			Tcomplex Z(b.ComplexValue());
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr(); Treal * const ap=a.RealPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Tcomplex(ap[i],0.0),Z);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(a);
				Tcomplex * const cp=c.ComplexPtr(); Treal * const ap=a.RealPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(ap[i],0.0),Z);
			}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KIntArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Treal * const ap=a.RealPtr(); Tint * const bp=b.IntPtr();
			if (&c == &b) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal * const c1p=c1.RealPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(ap[i],Treal(bp[i]));
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(a);
				Treal * const cp=c.RealPtr();
				for (i=0; i<N; i++) cp[i] = Pow(ap[i],Treal(bp[i]));
			}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KRealArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Treal * const ap=a.RealPtr(), * const bp=b.RealPtr();
			if (AnyNegative(ap,N)) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr();
				for (i=0; i<N; i++) {
					if (ap[i] < 0) {
						const Treal r = Pow(-ap[i],bp[i]);
						c1p[i] = Tcomplex(cos(pi*bp[i])*r, sin(pi*bp[i])*r);
					} else
						c1p[i] = Tcomplex(Pow(ap[i],bp[i]),0.0);
				}
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(a);
				Treal * const cp=c.RealPtr();
				Pow(cp,ap,bp,N);
			}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KComplexArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Tcomplex * const bp=b.ComplexPtr(); Treal * const ap=a.RealPtr();
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(Tcomplex(ap[i],0.0),bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tcomplex * const cp=c.ComplexPtr();
				for (i=0; i<N; i++) cp[i] = Pow(Tcomplex(ap[i],0.0),bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,Kint):
		{
			N = a.length();
			Tcomplex Z(b.IntValue(),0.0);
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(), * const ap=a.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(ap[i],Z);
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,Kreal):
		{
			N = a.length();
			Tcomplex Z(b.RealValue(),0.0);
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(), * const ap=a.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(ap[i],Z);
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,Kcomplex):
		{
			N = a.length();
			Tcomplex Z(b.ComplexValue());
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(), * const ap=a.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(ap[i],Z);
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,KIntArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Tcomplex * const ap=a.ComplexPtr(); Tint * const bp=b.IntPtr();
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(ap[i],bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(a);
				Tcomplex * const cp=c.ComplexPtr();
				for (i=0; i<N; i++) cp[i] = Pow(ap[i],bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,KRealArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			Tcomplex * const ap=a.ComplexPtr(); Treal * const bp=b.RealPtr();
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr();
				for (i=0; i<N; i++) c1p[i] = Pow(ap[i],bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(a);
				Tcomplex * const cp=c.ComplexPtr();
				for (i=0; i<N; i++) cp[i] = Pow(ap[i],bp[i]);
			}
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,KComplexArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError("Pow",a,b);
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr(), * const ap=a.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = Pow(ap[i],bp[i]);
			global::nops+= N;
		}
		break;
	default:
		BinError("Pow",a,b);
		break;
	}
}

/*
 #define OP(a,b) pow(a,b)
 #define OPCHAR "Pow"
 #define OPFNAME Pow
 #include "templ/tbinop.C"
 #undef OPFNAME
 #undef OPCHAR
 #undef OP
*/
 
inline Tint Modulus(Tint x, Tint y) {return (y==0) ? 0 : x % y;}
inline Treal Modulus(Treal x, Tint y) {return (y==0) ? 0 : fmod(x,y);}
inline Treal Modulus(Tint x, Treal y) {return (y==0) ? 0 : fmod(x,y);}
inline Treal Modulus(Treal x, Treal y) {return (y==0) ? 0 : fmod(x,y);}
static Tcomplex Modulus(Tcomplex x, Tint y) {return Tcomplex((y==0) ? 0 : fmod(real(x),y),0.0);}
static Tcomplex Modulus(Tcomplex x, Treal y) {return Tcomplex((y==0) ? 0 : fmod(real(x),y),0.0);}
static Tcomplex Modulus(Tcomplex x, Tcomplex y) {
	return Tcomplex((real(y)==0) ? 0 : fmod(real(x),real(y)), (imag(y)==0) ? 0 : fmod(imag(x),imag(y)));
}
inline Tcomplex Modulus(Tint x, Tcomplex y) {return Modulus(Tcomplex(x,0.0),y);}
inline Tcomplex Modulus(Treal x, Tcomplex y) {return Modulus(Tcomplex(x,0.0),y);}

#ifdef VECTOR_MACHINE
#  define VECTOR_MACHINE_WAS_DEFINED 1
#else
#  define VECTOR_MACHINE_WAS_DEFINED 0
#endif

#define OP(a,b) Modulus(a,b)
#define OPCHAR "mod"
#define OPFNAME Mod
#undef VECTOR_MACHINE
#include "templ/tbinop.C"
#if VECTOR_MACHINE_WAS_DEFINED
#  define VECTOR_MACHINE
#endif
#undef OPFNAME
#undef OPCHAR
#undef OP

// Comparison operators >,<,>=,<=

#define OP(a,b) ((a)>(b))
#define OPCHAR ">"
#define OPFNAME Gt
#include "templ/tcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)<(b))
#define OPCHAR "<"
#define OPFNAME Lt
#include "templ/tcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)>=(b))
#define OPCHAR ">="
#define OPFNAME Ge
#include "templ/tcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)<=(b))
#define OPCHAR "<="
#define OPFNAME Le
#include "templ/tcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// Equality operators == and !=

#define OP(a,b) ((a)==(b))
#define OPCHAR "=="
#define OPFNAME Eq
#include "templ/teqop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)!=(b))
#define OPCHAR "!="
#define OPFNAME Ne
#include "templ/teqop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// Logical operators And, Or

#define OP(a,b) ((a)&&(b))
#define OPCHAR "&&"
#define OPFNAME And
#include "templ/tbinintop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)||(b))
#define OPCHAR "||"
#define OPFNAME Or
#include "templ/tbinintop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// Comparison tests >,<,>=,<= (these return int code, not assign to objects)

#define OP(a,b) ((a)>(b))
#define OPCHAR ">"
#define OPFNAME TestGt
#include "templ/ttestcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)<(b))
#define OPCHAR "<"
#define OPFNAME TestLt
#include "templ/ttestcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)>=(b))
#define OPCHAR ">="
#define OPFNAME TestGe
#include "templ/ttestcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) ((a)<=(b))
#define OPCHAR "<="
#define OPFNAME TestLe
#include "templ/ttestcmpop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// Binary Min/Max operators

#define OP(a,b) min(a,b)
#define OPCHAR "Min"
#define OPFNAME Min
#include "templ/tminmaxop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a,b) max(a,b)
#define OPCHAR "Max"
#define OPFNAME Max
#include "templ/tminmaxop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// Unary Min/Max operators

static Tint theMinMaxPosition = ArrayBase;
// theMinMaxPosition stores the position of the last unary Min or Max operation
// the first element corresponds to theMinMaxPosition==ArrayBase, not zero
// the function MinMaxPosition() inquires theMinMaxPosition.

void Min(Tobject& c, const Tobject& a)
/* Min(c,a) returns the minimum element of a in c.
   The object a must not be complex. If a is scalar,
   it is returned as such. If it is an array, the
   minimum element (scalar) is returned. */
{
    switch (a.kind()) {
    case Kint:
        c = a.IntValue();
		theMinMaxPosition = ArrayBase;
        break;
    case Kreal:
        c = a.RealValue();
		theMinMaxPosition = ArrayBase;
        break;
    case KIntArray:
	{
		if (a.length() == 0) {
			c = a;
		} else {
			Tint *const p = a.IntPtr();
			Tint minpos = 0;
			Tint minimum = p[minpos];
			for (int i=1; i<a.length(); i++)
				if (p[i] < minimum) {minimum=p[i]; minpos=i;}
			c = minimum;
			theMinMaxPosition = minpos + ArrayBase;
		}
	}
	break;
    case KRealArray:
	{
		Treal *const p = a.RealPtr();
		Tint minpos = 0;
		Treal minimum = p[minpos];
		for (int i=1; i<a.length(); i++)
			if (p[i] < minimum) {minimum=p[i]; minpos=i;}
		c = minimum;
		theMinMaxPosition = minpos + ArrayBase;
	}
	break;
	default:
		UnaryError("Min",a);
		break;
    }
}

void Max(Tobject& c, const Tobject& a)
/* Max(c,a) returns the maximum element of a in c.
   The object a must not be complex. If a is scalar,
   it is returned as such. If it is an array, the
   maximum element (scalar) is returned. */
{
    switch (a.kind()) {
    case Kint:
        c = a.IntValue();
		theMinMaxPosition = ArrayBase;
        break;
    case Kreal:
        c = a.RealValue();
		theMinMaxPosition = ArrayBase;
        break;
    case KIntArray:
		if (a.length() == 0) {
			c = a;
		} else {
            Tint *const p = a.IntPtr();
            Tint maxpos = 0;
			Tint maximum = p[maxpos];
            for (int i=1; i<a.length(); i++)
				if (p[i] > maximum) {maximum=p[i]; maxpos=i;}
            c = maximum;
			theMinMaxPosition = maxpos + ArrayBase;
        }
        break;
    case KRealArray:
        {
            Treal *const p = a.RealPtr();
            Tint maxpos = 0;
			Treal maximum = p[maxpos];
            for (int i=1; i<a.length(); i++)
				if (p[i] > maximum) {maximum=p[i]; maxpos=i;}
            c = maximum;
			theMinMaxPosition = maxpos + ArrayBase;
        }
        break;
	default:
		UnaryError("Max",a);
		break;
    }
}

Tint MinMaxPosition() {return theMinMaxPosition;}


// The Not operator does not fit any of the templates

void Not(Tobject& c, const Tobject& a) {
	int i;
	switch (a.kind()) {
	case Kint:
		c = !(a.IntValue());
		global::nops++;
		break;
	case KIntArray:
		{
			int N = a.length();
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();;
			for (i=0; i<N; i++) cp[i] = !(ap[i]);
			global::nops+= N;
		}
		break;
	default:
		UnaryError("!",a);
		break;
	}
}
		

// Unary operators -, Inc, Dec

#define OP(a) (-(a))
#define OPCHAR "-"
#define OPFNAME Neg
#include "templ/tunop.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

inline void Increment(Tint& i) {i++;}
inline void Increment(Treal& x) {x+=1;}
inline void Increment(Tcomplex& z) {z+=1;}
inline void Decrement(Tint& i) {i--;}
inline void Decrement(Treal& x) {x-=1;}
inline void Decrement(Tcomplex& z) {z-=1;}

#define OP(a) Increment(a)
#define OPCHAR "++"
#define OPFNAME Inc
#include "templ/tunop1.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

#define OP(a) Decrement(a)
#define OPCHAR "--"
#define OPFNAME Dec
#include "templ/tunop1.C"
#undef OPFNAME
#undef OPCHAR
#undef OP

// The Abs operator doesn't fit to the standard template schemes
// since abs(complex) -> real

void Abs(Tobject& c, const Tobject& a) {
	int i,i1,i2;
	switch (a.kind()) {
	case Kint:
		c = abs(a.IntValue());
		c.ClearCharFlag();
		global::nops++;
		break;
	case Kreal:
		c = fabs(a.RealValue());
		global::nops++;
		break;
	case Kcomplex:
		c = abs(a.ComplexValue());
		global::nops++;
		break;
	case KIntArray:
		{
			c.copydimsIntArray(a);
			int N = a.length();
			GetLoopLimits(i1,i2);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();
			for (i=i1; i<=i2; i++) cp[i] = abs(ap[i]);
			c.ClearStringFlag();
			global::nops+= i2-i1+1;
		}
		break;
	case KRealArray:
		{
			c.copydimsRealArray(a);
			int N = a.length();
			GetLoopLimits(i1,i2);
			Treal *cp=c.RealPtr(), *ap=a.RealPtr();
			for (i=i1; i<=i2; i++) cp[i] = fabs(ap[i]);
			global::nops+= i2-i1+1;
		}
		break;
	case KComplexArray:
		{
			int N = a.length();
			GetLoopLimits(i1,i2);
			if (&c == &a) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal *c1p=c1.RealPtr(); Tcomplex *ap=a.ComplexPtr();
				for (i=i1; i<=i2; i++) c1p[i] = abs(ap[i]);
				c = c1;
			} else {
				c.copydimsRealArray(a);
				Treal *cp=c.RealPtr(); Tcomplex *ap=a.ComplexPtr();
				for (i=i1; i<=i2; i++) cp[i] = abs(ap[i]);
			}
			global::nops+= i2-i1+1;
		}
		break;
	case KObjectArray:
	case KShallowObjectArray:
	case Kfunction:
	case KCfunction:
	case KIntrinsicFunction:
	case Kvoid:
	case Kundef:
		UnaryError("Abs",a);
		break;
	}
}
		

// A general one-argument transcendental function FN, which is defined
// at all complex values, and is real-valued on the real axis.
// Transcen(FNpack,c,a) performes c=FN(a) for arbitrary numeric object a.
// FNpack is a struct containing the name ("FN"), plus pointers to 4 evaluators:
// real scalar FN, complex scalar FN, real vector FN, complex vector FN.

void Transcen(const TTranscenFunctionPack& funcs, Tobject& c, const Tobject& a) {
	int i,N;
	switch (a.kind()) {
	case Kint:
		c = (*funcs.f)(Treal(a.IntValue()));
		global::nops++;
		break;
	case Kreal:
		c = (*funcs.f)(a.RealValue());
		global::nops++;
		break;
	case Kcomplex:
		c = (*funcs.cf)(a.ComplexValue());
		global::nops++;
		break;
	case KIntArray:
		N = a.length();
		if (&c==&a) {
			Tobject c1;
			c1.copydimsRealArray(a);
			Treal *c1p=c1.RealPtr(); Tint *ap=a.IntPtr();
			for (i=0; i<N; i++) c1p[i] = Treal(ap[i]);
			(*funcs.vf)(c1p,N);
			c = c1;
		} else {
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(); Tint *ap=a.IntPtr();
			for (i=0; i<N; i++) cp[i] = Treal(ap[i]);
			(*funcs.vf)(cp,N);
		}
		global::nops+= N;
		break;
	case KRealArray:
		{
			N = a.length();
			if (&c!=&a) c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(), *ap=a.RealPtr();
			for (i=0; i<N; i++) cp[i] = ap[i];
			(*funcs.vf)(cp,N);
			global::nops+= N;
		}
		break;
	case KComplexArray:
		{
			N = a.length();
			if (&c!=&a) c.copydimsComplexArray(a);
			Tcomplex *cp=c.ComplexPtr(), *ap=a.ComplexPtr();
			for (i=0; i<N; i++) cp[i] = ap[i];
			(*funcs.cvf)(cp,N);
			global::nops+= N;
		}
		break;
	case KObjectArray:
	case KShallowObjectArray:
	case Kfunction:
	case KCfunction:
	case KIntrinsicFunction:
	case Kvoid:
	case Kundef:
		err << "Transcendental function '" << funcs.name << "' applied to " << Tshort(a) << "\n";
		error();
		break;
	}
}


