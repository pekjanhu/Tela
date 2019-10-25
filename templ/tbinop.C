/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2000 Pekka Janhunen
 */

/*
 * tbinop.C - 'template' for a binary operator, included separately for +,-,*
 */

static void OPFNAME(Treal y[], const Treal x[], const Treal a, const int N) {
	for (int i=0; i<N; i++) y[i] = OP(x[i],a);
}

static void OPFNAME(Treal y[], const Treal a, const Treal x[], const int N) {
	for (int i=0; i<N; i++) y[i] = OP(a,x[i]);
}

static void OPFNAME(Treal z[], const Treal x[], const Treal y[], const int N) {
	for (int i=0; i<N; i++) z[i] = OP(x[i],y[i]);
}

/*
  Mflops recording policy:
  real,real is one op,
  real,complex is two ops (correct for both + and *)
  complex,complex is four ops (average of 2 (+) and 6 (*))
*/

//#define show(msg) cout << OPCHAR << " " << msg << "\n"
#define show(msg)

void OPFNAME(Tobject& c, const Tobject& a, const Tobject& b) {
	int i,N,i1,i2;
	Tint I; Treal X; Tcomplex Z;
	switch (pair(a.kind(),b.kind())) {
	case pair(Kint,Kint):
		c = OP(a.IntValue(),b.IntValue());
		global::nops++;
		break;
	case pair(Kint,Kreal):
		c = OP(Treal(a.IntValue()),b.RealValue());
		global::nops++;
		break;
	case pair(Kint,Kcomplex):
		c = OP(Tcomplex(a.IntValue()),b.ComplexValue());
		global::nops+= 2;
		break;
	case pair(Kint,KIntArray):
		{
			show("Kint,KIntArray");
			I = a.IntValue();
			N = b.length();
			c.copydimsIntArray(b);
			Tint * const cp=c.IntPtr(), * const bp=b.IntPtr();
			GetLoopLimits(i1,i2);
			for (i=i1; i<=i2; i++) cp[i] = OP(I,bp[i]);
			global::nops+= (i2-i1+1);
		}
		break;
	case pair(Kint,KRealArray):
		{
			show("Kint,KRealArray");
			X = Treal(a.IntValue());
			N = b.length();
			c.copydimsRealArray(b);
			Treal * const cp=c.RealPtr(), * const bp=b.RealPtr();
			GetLoopLimits(i1,i2);
			OPFNAME(&cp[i1],X,&bp[i1],i2-i1+1);
			global::nops+= (i2-i1+1);
		}
		break;
	case pair(Kint,KComplexArray):
		{
			Z = Tcomplex(Treal(a.IntValue()),0.0);
			N = b.length();
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr();
			GetLoopLimits(i1,i2);
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],Treal(a.IntValue()),&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(Z,bp[i]);
#			endif
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(Kreal,Kint):
		c = OP(a.RealValue(),Treal(b.IntValue()));
		global::nops++;
		break;
	case pair(Kreal,Kreal):
		c = OP(a.RealValue(),b.RealValue());
		global::nops++;
		break;
	case pair(Kreal,Kcomplex):
		c = OP(Tcomplex(a.RealValue()),b.ComplexValue());
		global::nops+= 2;
		break;
	case pair(Kreal,KIntArray):
		show("Kreal,KIntArray");
		N = b.length();
		X = a.RealValue();
		GetLoopLimits(i1,i2);
		if (&c == &b) {
			Tobject c1;
			c1.copydimsRealArray(b);
			Treal * const c1p=c1.RealPtr(); Tint * const bp=b.IntPtr();
			for (i=i1; i<=i2; i++) c1p[i] = OP(X,bp[i]);
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsRealArray(b);	// make c a real array of the same dims as b
			Treal * const cp=c.RealPtr(); Tint * const bp=b.IntPtr();
			for (i=i1; i<=i2; i++) cp[i] = OP(X,bp[i]);
		}
		global::nops+= i2-i1+1;
		break;
	case pair(Kreal,KRealArray):
		{
			show("Kreal,KRealArray");
			N = b.length();
			X = a.RealValue();
			c.copydimsRealArray(b);
			Treal * const cp=c.RealPtr(), * const bp=b.RealPtr();
			GetLoopLimits(i1,i2);
			OPFNAME(&cp[i1],X,&bp[i1],i2-i1+1);
			global::nops+= (i2-i1+1);
		}
		break;
	case pair(Kreal,KComplexArray):
		{
			Z = a.RealValue();
			N = b.length();
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr();
			GetLoopLimits(i1,i2);
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],a.RealValue(),&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(Z,bp[i]);
#			endif
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(Kcomplex,Kint):
		c = OP(a.ComplexValue(),Tcomplex(b.IntValue()));
		global::nops+= 2;
		break;
	case pair(Kcomplex,Kreal):
		c = OP(a.ComplexValue(),Tcomplex(b.RealValue()));
		global::nops+= 2;
		break;
	case pair(Kcomplex,Kcomplex):
		c = OP(a.ComplexValue(),b.ComplexValue());
		global::nops+= 4;
		break;
	case pair(Kcomplex,KIntArray):
		N = b.length();
		Z = a.ComplexValue();
		GetLoopLimits(i1,i2);
		if (&c == &b) {
			Tobject c1;
			c1.copydimsComplexArray(b);
			Tcomplex * const c1p=c1.ComplexPtr(); Tint * const bp=b.IntPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&c1p[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) c1p[i] = OP(Z,bp[i]);
#			endif
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsComplexArray(b);
			Tint * const bp=b.IntPtr();
			Tcomplex *cp=c.ComplexPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(Z,bp[i]);
#			endif
		}
		global::nops+= (i2-i1+1)*2;
		break;
	case pair(Kcomplex,KRealArray):
		N = b.length();
		Z = a.ComplexValue();
		GetLoopLimits(i1,i2);
		if (&c == &b) {
			Tobject c1;
			c1.copydimsComplexArray(b);
			Tcomplex * const c1p=c1.ComplexPtr(); Treal * const bp=b.RealPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&c1p[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) c1p[i] = OP(Z,bp[i]);
#			endif
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(); Treal * const bp=b.RealPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(Z,bp[i]);
#			endif
		}
		global::nops+= (i2-i1+1)*2;
		break;
	case pair(Kcomplex,KComplexArray):
		{
			N = b.length();
			Z = a.ComplexValue();
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(b);
			Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],Z,&bp[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(Z,bp[i]);
#			endif
			global::nops+= (i2-i1+1)*4;
		}
		break;
	case pair(KIntArray,Kint):
		{
			show("KIntArray,Kint");
			I = b.IntValue();
			N = a.length();
			GetLoopLimits(i1,i2);
			c.copydimsIntArray(a);
			Tint * const cp=c.IntPtr(), * const ap=a.IntPtr();
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],I);
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KIntArray,Kreal):
		show("KIntArray,Kreal");
		N = a.length();
		X = b.RealValue();
		GetLoopLimits(i1,i2);
		if (&c == &a) {
			Tobject c1;
			c1.copydimsRealArray(a);
			Treal * const c1p=c1.RealPtr(); Tint * const ap=a.IntPtr();
			for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],X);
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsRealArray(a);
			Treal * const cp=c.RealPtr(); Tint * const ap=a.IntPtr();
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],X);
		}
		global::nops+= i2-i1+1;
		break;
	case pair(KIntArray,Kcomplex):
		N = a.length();
		Z = b.ComplexValue();
		GetLoopLimits(i1,i2);
		if (&c == &a) {
			Tobject c1;
			c1.copydimsComplexArray(a);
			Tcomplex * const c1p=c1.ComplexPtr(); Tint * const ap=a.IntPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&c1p[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],Z);
#			endif
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(); Tint * const ap=a.IntPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],Z);
#			endif
		}
		global::nops+= (i2-i1+1)*2;
		break;
	case pair(KIntArray,KIntArray):
		{
			show("KIntArray,KIntArray");
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			GetLoopLimits(i1,i2);
			c.copydimsIntArray(a);
			Tint * const cp=c.IntPtr(), * const ap=a.IntPtr(), * const bp=b.IntPtr();
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KIntArray,KRealArray):
		{
			show("KIntArray,KRealArray");
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Treal * const bp=b.RealPtr(); Tint * const ap=a.IntPtr();
			GetLoopLimits(i1,i2);
			if (&a == &c) {
				Tobject c1;
				c1.copydimsRealArray(b);
				Treal * const c1p=c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = OP(Treal(ap[i]),bp[i]);
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(b);
				Treal * const cp=c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = OP(Treal(ap[i]),bp[i]);
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KIntArray,KComplexArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tcomplex * const bp=b.ComplexPtr(); Tint * const ap=a.IntPtr();
			GetLoopLimits(i1,i2);
			if (&a == &c) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],bp[i]);
#				endif
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tcomplex * const cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
#				endif
			}
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(KRealArray,Kint):
		{
			show("KRealArray,Kint");
			X = Treal(b.IntValue());
			N = a.length();
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(a);
			Treal * const cp=c.RealPtr(), * const ap=a.RealPtr();
			OPFNAME(&cp[i1],&ap[i1],X,i2-i1+1);
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,Kreal):
		{
			show("KRealArray,Kreal");
			N = a.length();
			X = b.RealValue();
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(a);
			Treal * const cp=c.RealPtr(), * const ap=a.RealPtr();
			OPFNAME(&cp[i1],&ap[i1],X,i2-i1+1);
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,Kcomplex):
		N = a.length();
		Z = b.ComplexValue();
		GetLoopLimits(i1,i2);
		if (&c == &a) {
			Tobject c1;
			c1.copydimsComplexArray(a);
			Tcomplex * const c1p=c1.ComplexPtr(); Treal * const ap=a.RealPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&c1p[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],Z);
#			endif
			c.bitwiseMoveFrom(c1);
		} else {
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(); Treal * const ap=a.RealPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],Z);
#			endif
		}
		global::nops+= (i2-i1+1)*2;
		break;
	case pair(KRealArray,KIntArray):
		{
			show("KRealArray,KIntArray");
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Treal * const ap=a.RealPtr(); Tint * const bp=b.IntPtr();
			GetLoopLimits(i1,i2);
			if (&c == &b) {
				Tobject c1;
				c1.copydimsRealArray(a);
				Treal * const c1p=c1.RealPtr();
				for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],Treal(bp[i]));
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsRealArray(a);
				Treal * const cp=c.RealPtr();
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],Treal(bp[i]));
			}
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,KRealArray):
		{
			show("KRealArray,KRealArray");
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			GetLoopLimits(i1,i2);
			c.copydimsRealArray(a);
			Treal * const cp=c.RealPtr(), * const ap=a.RealPtr(), * const bp=b.RealPtr();
			OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
			global::nops+= i2-i1+1;
		}
		break;
	case pair(KRealArray,KComplexArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tcomplex * const bp=b.ComplexPtr(); Treal * const ap=a.RealPtr();
			GetLoopLimits(i1,i2);
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],bp[i]);
#				endif
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tcomplex * const cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
#				endif
			}
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(KComplexArray,Kint):
		{
			N = a.length();
			Z = Tcomplex(b.IntValue());
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(), * const ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],Z);
#			endif
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(KComplexArray,Kreal):
		{
			N = a.length();
			Z = Tcomplex(b.RealValue());
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(), * const ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],Z);
#			endif
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(KComplexArray,Kcomplex):
		{
			N = a.length();
			Z = b.ComplexValue();
			GetLoopLimits(i1,i2);
			c.copydimsComplexArray(a);
			Tcomplex * const cp=c.ComplexPtr(), * const ap=a.ComplexPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],&ap[i1],Z,i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],Z);
#			endif
			global::nops+= (i2-i1+1)*4;
		}
		break;
	case pair(KComplexArray,KIntArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tcomplex * const ap=a.ComplexPtr(); Tint * const bp=b.IntPtr();
			GetLoopLimits(i1,i2);
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],bp[i]);
#				endif
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(a);
				Tcomplex * const cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
#				endif
			}
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(KComplexArray,KRealArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tcomplex * const ap=a.ComplexPtr(); Treal * const bp=b.RealPtr();
			GetLoopLimits(i1,i2);
			if (&c == &b) {
				Tobject c1;
				c1.copydimsComplexArray(a);
				Tcomplex * const c1p=c1.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&c1p[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) c1p[i] = OP(ap[i],bp[i]);
#				endif
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(a);
				Tcomplex * const cp=c.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
#				endif
			}
			global::nops+= (i2-i1+1)*2;
		}
		break;
	case pair(KComplexArray,KComplexArray):
		{
			N = a.length();
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			GetLoopLimits(i1,i2);
			/****
			   Added &c==&a test 9.1.1995.
			   OP x,x,y yielded error if x.iscompatible(y) was true but x.dims()!=y.dims().
			   This was possible e.g. if x=rand(3) and y=rand(1,3).
			   This should fix it here, but is the same bug lurking elsewhere?
			   Think about this!!!
			   *****/
			if (&c == &a) {
				Tobject c1;
				c1.copydimsComplexArray(b);
				Tcomplex * const cp=c1.ComplexPtr(), * const bp=b.ComplexPtr(), * const ap=a.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
#				endif
				c.bitwiseMoveFrom(c1);
			} else {
				c.copydimsComplexArray(b);
				Tcomplex * const cp=c.ComplexPtr(), * const bp=b.ComplexPtr(), * const ap=a.ComplexPtr();
#				ifdef VECTOR_MACHINE
				OPFNAME(&cp[i1],&ap[i1],&bp[i1],i2-i1+1);
#				else
				for (i=i1; i<=i2; i++) cp[i] = OP(ap[i],bp[i]);
#				endif
			}
			global::nops+= (i2-i1+1)*4;
		}
		break;
	default:
		BinError(OPCHAR,a,b);
		break;
	}
}
