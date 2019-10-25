/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

[B] = transpose(A; P)
/* ... */
{
	Tkind k = A.kind();
	if (k!=KIntArray && k!=KRealArray && k!=KComplexArray)
		B = A;
	else {
		int d,d1,pA,pB;
		TDimPack Adims = A.dims();
		int D = Adims.rank();
		TDimPack Bdims = Adims;
		TDimPack perm;
		for (d=0; d<D; d++) perm[d] = D-d-1;
		if (Nargin == 2) {	// P given ?
			if (P.kind()==KIntArray) {
				if (P.rank()==1 && P.length()==D)
					for (d=0; d<D; d++) perm[d] = P.IntPtr()[d]-1;	// note -1: P is eg. [2,1]
				else
					return -2;
			} else
				return -1;
		}
		for (int d=0; d<D; d++) Bdims[d] = Adims[perm[d]];
		// now Adims, Bdims, D are ready
		TDimPack modulus = Adims;
		TDimPack divisor;
		for (d=0; d<D; d++) {
			divisor[d] = 1;
			for (d1=d+1; d1<D; d1++) divisor[d]*= Adims[d1];
		}
		TDimPack mult;
		for (d=0; d<D; d++) {
			mult[d] = 1;
			for (d1=d+1; d1<D; d1++) mult[d]*= Bdims[d1];
		}
		// Allocate B
		switch (k) {
		case KIntArray:
			B.ireserv(Bdims); break;
		case KRealArray:
			B.rreserv(Bdims); break;
		case KComplexArray:
			B.creserv(Bdims); break;
		default:
		}
		// Now the dirty work
		TDimPack ind;
		for (pA=0; pA<A.length(); pA++) {
			for (d=0; d<D; d++)
				ind[d] = (pA / divisor[d]) % modulus[d];
			pB = 0;
			for (d=0; d<D; d++) pB+= ind[d]*mult[d];
			switch (k) {
			case KIntArray:
				B.IntPtr()[pB] = A.IntPtr()[pA]; break;
			case KRealArray:
				B.RealPtr()[pB] = A.RealPtr()[pA]; break;
			case KComplexArray:
				B.ComplexPtr()[pB] = A.ComplexPtr()[pA]; break;
			default:
			}
		}
	}
	return 0;
}


