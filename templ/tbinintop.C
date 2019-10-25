/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
 * tbinintop.C - 'template' for a binary int operator, included separately for (logical) And, Or
 */

void OPFNAME(Tobject& c, const Tobject& a, const Tobject& b) {
	int i,N;
	Tint I;
	switch (pair(a.kind(),b.kind())) {
	case pair(Kint,Kint):
		c = OP(a.IntValue(),b.IntValue());
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
	default:
		BinError(OPCHAR,a,b);
		break;
	}
}
