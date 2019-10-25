/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
 * teqop.C - 'template' for an equality operator, included separately for ==, !=
 */

void OPFNAME(Tobject& c, const Tobject& a, const Tobject& b) {
	int i,N;
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
		global::nops++;
		break;
	case pair(Kint,KIntArray):
		{
			I = a.IntValue();
			N = b.length();
			c.copydimsIntArray(b);
			Tint *cp=c.IntPtr(), *bp=b.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(I,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(Kint,KRealArray):
		{
			Tobject c1;
			X = Treal(a.IntValue());
			N = b.length();
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(X,bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(Kint,KComplexArray):
		{
			Tobject c1;
			Z = Tcomplex(Treal(a.IntValue()),0.0);
			N = b.length();
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Tcomplex *bp=b.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Z,bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
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
		global::nops++;
		break;
	case pair(Kreal,KIntArray):
		{
			N = b.length();
			X = a.RealValue();
			c.copydimsIntArray(b);	// make c an int array of the same dims as b
			Tint *cp=c.IntPtr(), *bp=b.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(X,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(Kreal,KRealArray):
		{
			Tobject c1;
			N = b.length();
			X = a.RealValue();
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(X,bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(Kreal,KComplexArray):
		{
			Tobject c1;
			Z = a.RealValue();
			N = b.length();
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Tcomplex *bp=b.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Z,bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(Kcomplex,Kint):
		c = OP(a.ComplexValue(),Tcomplex(b.IntValue()));
		global::nops++;
		break;
	case pair(Kcomplex,Kreal):
		c = OP(a.ComplexValue(),Tcomplex(b.RealValue()));
		global::nops++;
		break;
	case pair(Kcomplex,Kcomplex):
		c = OP(a.ComplexValue(),b.ComplexValue());
		global::nops++;
		break;
	case pair(Kcomplex,KIntArray):
		{
			N = b.length();
			Z = a.ComplexValue();
			c.copydimsIntArray(b);
			Tint *cp=c.IntPtr(), *bp=b.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(Z,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(Kcomplex,KRealArray):
		{
			N = b.length();
			Z = a.ComplexValue();
			Tobject c1;
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Z,bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(Kcomplex,KComplexArray):
		{
			Tobject c1;
			N = b.length();
			Z = a.ComplexValue();
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Tcomplex *bp=b.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Z,bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kint):
		{
			I = b.IntValue();
			N = a.length();
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],I);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kreal):
		{
			N = a.length();
			X = b.RealValue();
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],X);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kcomplex):
		{
			N = a.length();
			Z = b.ComplexValue();
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();
/*
  Replaced OP(ap[i],Z) by OP(Z,ap[i]) here.
  The latter form vectorizes better.
*/
			for (i=0; i<N; i++) cp[i] = OP(Z,ap[i]);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KIntArray):
		N = a.length();
		if (N == b.length()) {
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr(), *bp=b.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],bp[i]);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KIntArray,KRealArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Treal *bp=b.RealPtr(); Tint *ap=a.IntPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Treal(ap[i]),bp[i]);
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KIntArray,KComplexArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tint *c1p=c1.IntPtr(); Tcomplex *bp=b.ComplexPtr(); Tint *ap=a.IntPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(c1p,ap,bp,N);
#			else
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],bp[i]);
#			endif
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KRealArray,Kint):
		{
			N = a.length();
			X = Treal(b.IntValue());
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Treal *ap=a.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],X);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kreal):
		{
			N = a.length();
			X = b.RealValue();
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Treal *ap=a.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],X);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kcomplex):
		{
			N = a.length();
			Z = b.ComplexValue();
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Treal *ap=a.RealPtr();
#			ifdef VECTOR_MACHINE
			OPFNAME(c1p,ap,Z,N);
#			else
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],Z);
#			endif
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KIntArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Treal *ap=a.RealPtr(); Tint *bp=b.IntPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],Treal(bp[i]));
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KRealArray,KRealArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Treal *ap=a.RealPtr(), *bp=b.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],bp[i]);
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KRealArray,KComplexArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Tcomplex *bp=b.ComplexPtr(); Treal *ap=a.RealPtr();
/*
  Here also swapped OP(ap[i],bp[i]) -> OP(bp[i],ap[i])
*/
			for (i=0; i<N; i++) c1p[i] = OP(bp[i],ap[i]);
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KComplexArray,Kint):
		{
			N = a.length();
			Z = Tcomplex(b.IntValue());
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Tcomplex *ap=a.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],Z);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,Kreal):
		{
			N = a.length();
			Z = Tcomplex(b.RealValue());
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Tcomplex *ap=a.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],Z);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,Kcomplex):
		{
			N = a.length();
			Z = b.ComplexValue();
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Tcomplex *ap=a.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],Z);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KComplexArray,KIntArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Tcomplex *ap=a.ComplexPtr(); Tint *bp=b.IntPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KComplexArray,KRealArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(a);
			Tint *c1p=c1.IntPtr(); Tcomplex *ap=a.ComplexPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KComplexArray,KComplexArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr(); Tcomplex *bp=b.ComplexPtr(); Tcomplex *ap=a.ComplexPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(KObjectArray,KObjectArray):
		N = a.length();
		if (N == b.length()) {
			Tobject c1;
			c1.copydimsIntArray(b);
			Tint *c1p=c1.IntPtr();
			TObjectPtr *bp=b.ObjectPtrPtr(), *ap=a.ObjectPtrPtr();
			for (i=0; i<N; i++) c1p[i] = OP(*ap[i],*bp[i]);		/* Inserted stars here 1.3.2001 /Pj */
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		} else c = OP(0,1);
		break;
	case pair(Kfunction,Kfunction):
		c = OP(a.FunctionValue(),b.FunctionValue());
		break;
	case pair(KCfunction,KCfunction):
		c = OP(a.CFunctionPtr(),b.CFunctionPtr());
		break;
	case pair(KIntrinsicFunction,KIntrinsicFunction):
		c = OP(a.IntrinsicCompilerPtr(),b.IntrinsicCompilerPtr());
		break;
	case pair(Kvoid,Kvoid):
		c = OP(0,0);
		break;
	default:
		// The default for two arbitrary nonmatching structures is to
		// return OP(0,1), of course, unless one of the args is undefined,
		// which leads to error.
		if (a.kind()==Kundef || b.kind()==Kundef) BinError(OPCHAR,a,b);
		c = OP(0,1);
		break;
	}
}
