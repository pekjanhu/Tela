/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
 * tunop.C - 'template' for a kind-preserving unary operator, included separately for -,...
 */

void OPFNAME(Tobject& c, const Tobject& a) {
	int i,i1,i2;
	switch (a.kind()) {
	case Kint:
		c = OP(a.IntValue());
		global::nops++;
		break;
	case Kreal:
		c = OP(a.RealValue());
		global::nops++;
		break;
	case Kcomplex:
		c = OP(a.ComplexValue());
		global::nops++;
		break;
	case KIntArray:
		{
			c.copydimsIntArray(a);
			Tint *cp=c.IntPtr(), *ap=a.IntPtr();
			const Tint N = a.length();
			GetLoopLimits(i1,i2);
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i]);
			global::nops+= i2-i1+1;
		}
		break;
	case KRealArray:
		{
			c.copydimsRealArray(a);
			Treal *cp=c.RealPtr(), *ap=a.RealPtr();
			const Tint N = a.length();
			GetLoopLimits(i1,i2);
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i]);
			global::nops+= i2-i1+1;
		}
		break;
	case KComplexArray:
		{
			c.copydimsComplexArray(a);
			Tcomplex *cp=c.ComplexPtr(), *ap=a.ComplexPtr();
			const Tint N = a.length();
			GetLoopLimits(i1,i2);
#			ifdef VECTOR_MACHINE
			OPFNAME(&cp[i1],&ap[i1],i2-i1+1);
#			else
			for (i=i1; i<=i2; i++) cp[i] = OP(ap[i]);
#			endif
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
		UnaryError(OPCHAR,a);
		break;
	}
}
