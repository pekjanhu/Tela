/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
 * tunop1.C - 'template' for a kind-preserving unary operator that is applied to one object.
 * Included separately for Inc,Dec.
 */

void OPFNAME(Tobject& a) {
	int i;
	switch (a.kind()) {
	case Kint:
		OP(a.IntRef());
		global::nops++;
		break;
	case Kreal:
		OP(a.RealRef());
		global::nops++;
		break;
	case Kcomplex:
		OP(a.ComplexRef());
		global::nops++;
		break;
	case KIntArray:
#		ifdef VECTOR_MACHINE
		OPFNAME(a.IntPtr(),a.length());
#		else
		for (i=0; i<a.length(); i++) OP(a.IntPtr()[i]);
#		endif
		global::nops+= a.length();
		break;
	case KRealArray:
#		ifdef VECTOR_MACHINE
		OPFNAME(a.RealPtr(),a.length());
#		else
		for (i=0; i<a.length(); i++) OP(a.RealPtr()[i]);
#		endif
		global::nops+= a.length();
		break;
	case KComplexArray:
#		ifdef VECTOR_MACHINE
		OPFNAME(a.ComplexPtr(),a.length());
#		else
		for (i=0; i<a.length(); i++) OP(a.ComplexPtr()[i]);
#		endif
		global::nops+= a.length();
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
