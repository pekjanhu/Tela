/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
 * ttestcmpop.C - 'template' for a binary compare operator, included separately for >,<,>=,<=
 * This is similar to tcmpop.C except that the current template returns int code, does not assign
 * to a result object.
 */

int OPFNAME(const Tobject& a, const Tobject& b) {
	int i,N,result;
	Tint I; Treal X;
	switch (pair(a.kind(),b.kind())) {
	case pair(Kint,Kint):
		result = OP(a.IntValue(),b.IntValue());
		global::nops++;
		break;
	case pair(Kint,Kreal):
		result = OP(Treal(a.IntValue()),b.RealValue());
		global::nops++;
		break;
	case pair(Kint,KIntArray):
		{
			I = a.IntValue();
			N = b.length();
			Tint *bp=b.IntPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(I,bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(Kint,KRealArray):
		{
			N = b.length();
			X = Treal(a.IntValue());
			Treal *bp=b.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(X,bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(Kreal,Kint):
		result = OP(a.RealValue(),Treal(b.IntValue()));
		global::nops++;
		break;
	case pair(Kreal,Kreal):
		result = OP(a.RealValue(),b.RealValue());
		global::nops++;
		break;
	case pair(Kreal,KIntArray):
		{
			X = a.RealValue();
			N = b.length();
			Tint *bp=b.IntPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(X,bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(Kreal,KRealArray):
		{
			N = b.length();
			X = a.RealValue();
			Treal *bp=b.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(X,bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kint):
		{
			N = a.length();
			I = b.IntValue();
			Tint *ap=a.IntPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],I)) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,Kreal):
		{
			N = a.length();
			X = b.RealValue();
			Tint *ap=a.IntPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],X)) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KIntArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tint *ap=a.IntPtr(), *bp=b.IntPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KIntArray,KRealArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tint *ap=a.IntPtr(); Treal *bp=b.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(Treal(ap[i]),bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kint):
		{
			N = a.length();
			X = Treal(b.IntValue());
			Treal *ap=a.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],X)) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,Kreal):
		{
			N = a.length();
			X = b.RealValue();
			Treal *ap=a.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],X)) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KIntArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Tint *bp=b.IntPtr(); Treal *ap=a.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],Treal(bp[i]))) {result=0; break;}
			global::nops+= N;
		}
		break;
	case pair(KRealArray,KRealArray):
		{
			N = a.length();
			//if (N != b.length()) LengthError(OPCHAR,a,b);
			if (!a.iscompatible(b)) LengthError(OPCHAR,a,b);
			Treal *ap=a.RealPtr(), *bp=b.RealPtr();
			result = 1;
			for (i=0; i<N; i++) if (!OP(ap[i],bp[i])) {result=0; break;}
			global::nops+= N;
		}
		break;
	default:
		result = 0;
		BinError(OPCHAR,a,b);
		break;
	}
	return result;
}
