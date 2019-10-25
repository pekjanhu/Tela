/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
 * tminmaxop.C - 'template' for a binary min/max operator, included separately for Min,Max
 */

void OPFNAME(Tobject& c, const Tobject& a, const Tobject& b) {
	int i,N;
	Tint I; Treal X;
	switch (pair(a.kind(),b.kind())) {
	case pair(Kint,Kint):
		c = OP(a.IntValue(),b.IntValue());
		global::nops++;
		break;
	case pair(Kint,Kreal):
		c = OP(Treal(a.IntValue()),b.RealValue());
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
			N = b.length();
			X = Treal(a.IntValue());
			c.copydimsRealArray(b);
			Treal *cp=c.RealPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) cp[i] = OP(X,bp[i]);
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
	case pair(Kreal,KIntArray):
		{
			Tobject c1;
			X = a.RealValue();
			N = b.length();
			c1.copydimsRealArray(b);	// make c1 a real array of the same dims as b
			Treal *c1p=c1.RealPtr(); Tint *bp=b.IntPtr();
			for (i=0; i<N; i++) c1p[i] = OP(X,Treal(bp[i]));
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(Kreal,KRealArray):
		{
			N = b.length();
			X = a.RealValue();
			c.copydimsRealArray(b);
			Treal *cp=c.RealPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) cp[i] = OP(X,bp[i]);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kint):
		{
			N = a.length();
			I = b.IntValue();
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],I);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kreal):
		{
			Tobject c1;
			N = a.length();
			X = b.RealValue();
			c1.copydimsRealArray(a);
			Treal *c1p=c1.RealPtr(); Tint *ap=a.IntPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Treal(ap[i]),X);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KIntArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr(), *bp=b.IntPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],bp[i]);
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KRealArray):
		{
			Tobject c1;
			c1.copydimsRealArray(b);
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Treal *c1p=c1.RealPtr(); Tint *ap=a.IntPtr(); Treal *bp=b.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(Treal(ap[i]),bp[i]);
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kint):
		{
			N = a.length();
			X = Treal(b.IntValue());
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(); Treal *ap=a.RealPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],X);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kreal):
		{
			N = a.length();
			X = b.RealValue();
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(); Treal *ap=a.RealPtr();
			for (i=0; i<N; i++) cp[i] = OP(ap[i],X);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KIntArray):
		{
			Tobject c1;
			c1.copydimsRealArray(a);
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Treal *c1p=c1.RealPtr(); Tint *bp=b.IntPtr(); Treal *ap=a.RealPtr();
			for (i=0; i<N; i++) c1p[i] = OP(ap[i],Treal(bp[i]));
			//c = c1;
			c.bitwiseMoveFrom(c1);
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KRealArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(); Treal *ap=a.RealPtr(), *bp=b.RealPtr();
			VECTORIZED for (i=0; i<N; i++) cp[i] = OP(ap[i],bp[i]);
			global::nops+= N;
		}
		break;
	default:
		BinError(OPCHAR,a,b);
		break;
	}
}
