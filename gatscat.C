/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
  Indexing (gather/scatter).
  Functions:

  Gather (a,b,indices[D],D)		:	a = b[indices[0],...,indices[D-1]]
  Scatter(a,b,indices[D],D)		:	b[indices[0],...,indices[D-1]] = a

  PasteArrays(a,args[N],N)		:	a = [args[0], args[1],..., args[N-1]]
  BuildArray(a,args[N],N)		:	a = [args[0]; args[1],..., args[N-1]]

  Range(args[4])				:	args[0] = args[1]:args[3]:args[2] (notice order! a[3] is increment)
*/

#ifdef __GNUC__
#  pragma implementation "gatscat.H"
#endif
#include "gatscat.H"

INLINE Tint checkrange(const Tint V[], Tint N, Tint a, Tint b) {
#	ifdef VECTOR_MACHINE
	int retval = -1;
	VECTORIZED for (Tint i=0; i<N; i++) {
		if (V[i]<a) {retval=i; break;}
	}
	if (retval >= 0) return retval;
	VECTORIZED for (i=0; i<N; i++)
		if (V[i]>b) {retval=i; break;}
	return retval;
#	else
	for (Tint i=0; i<N; i++)
		if (V[i]<a || V[i]>b) return i;
	return -1;
#	endif
}

static void Gather1D(Tobject& a, const Tobject& b, const Tobject& index, int MakeStr)
// Gather tailored for 1D case. No integer multiplication, no work arrays ==> fast
// a = b[index]
{
	Tkind k = index.kind();
	if (k == Kint) {
		Tint i = index.IntValue() - ArrayBase;
		if (i<0 || i>=b.length()) {
			err << "Scalar index " << i+ArrayBase << " out of range for " << Tshort(b) << ".\n";
			error();
		}
		switch (b.kind()) {
		case KIntArray:
			a = b.IntPtr()[i];
			if (MakeStr) a.SetCharFlag();
			break;
		case KRealArray:
			a = b.RealPtr()[i];
			break;
		case KComplexArray:
			a = b.ComplexPtr()[i];
			break;
		case KObjectArray:
			a = *b.ObjectPtrPtr()[i];
			break;
		default:
			err << "Internal (1) in Gather1D\n";
			error();
		}
	} else if (k == KIntArray) {
		Tint i;
		Tint * const ip = index.IntPtr();
		if (index.rank()!=1) {err << "Invalid index " << Tshort(index) << " in a=b[..].\n"; error();}
		if (checkrange(ip,index.length(),ArrayBase,b.length()-1+ArrayBase) >= 0) {
			err << "Index vector " << Tshort(index) << " out of range for " << Tshort(b) << ".\n";
			error();
		}
		Tobject a1;
		switch (b.kind()) {
		case KIntArray:
			a1.copydimsIntArray(index);
			if (MakeStr) a1.SetStringFlag();
			VECTORIZED for (i=0; i<index.length(); i++) a1.IntPtr()[i] = b.IntPtr()[ip[i]-ArrayBase];
			break;
		case KRealArray:
			a1.copydimsRealArray(index);
			VECTORIZED for (i=0; i<index.length(); i++) a1.RealPtr()[i] = b.RealPtr()[ip[i]-ArrayBase];
			break;
		case KComplexArray:
			a1.copydimsComplexArray(index);
			VECTORIZED for (i=0; i<index.length(); i++) a1.ComplexPtr()[i] = b.ComplexPtr()[ip[i]-ArrayBase];
			break;
		case KObjectArray:
			err << "Vector index " << Tshort(index) << " for object array, only scalars are allowed.\n";
			error();
		default:
			err << "Internal (2) in Gather1D\n";
			error();
		}
		a.bitwiseMoveFrom(a1);
	} else if (k == Kvoid) {	// Void index means ALLRANGE
		a = b;
	} else {
		err << "Invalid index " << Tshort(index) << " in a=b[..].\n";
		error();
	}
}	// Gather1D


const int MAX_STATIC_INDEX = 100;			// Avoid using operator new for small datasets.
static Tint static_itmp[MAX_STATIC_INDEX];	// Use this static buffer instead for these.

void Gather(Tobject& a, const Tobject& b, const TConstObjectPtr indices[], int D)
// a = b[indices]
// Each *indices[i], 0<=i<D, must be either integer or 1D IntArray or a VOID object (denoting all range)
// Scalar b is allowed. The result a is always equal to b.
//   In scalar b case Indices are checked: they can be 1, VOID, or #(1).
{
	if (!b.IsArray()) {
		if (!b.IsScalar()) {
			err << "In a=b[..], b = " << Tshort(b) << " is non-numeric\n";
			error();
		}
		a = b;		// result is always equal to b, below we just do consistency checks
		for (int d=0; d<D; d++) {
			const Tkind k = indices[d]->kind();
			if (!(k==Kvoid ||
				  k==Kint && indices[d]->IntValue()==1 ||
				  k==KIntArray && indices[d]->rank()==1 && indices[d]->length()==1 && indices[d]->IntPtr()[0]==1)) {
				err << "In a=b[..] b = " << Tshort(b) << " is scalar but " << d+ArrayBase << ". index is not 1, :, or #(1).\n";
				error();
			}
		}
		return;
	}
	int MakeStr = b.HasStringFlag();
	if (D == 1) {Gather1D(a,b,**indices,MakeStr); return;}
	//Tint scalarindices[MAXRANK];
	Tint *vectorindices[MAXRANK];
	int tobedeleted[MAXRANK];
	Tint veclengths[MAXRANK];
	Tint multipliers[MAXRANK];	// for vector indices only
	Tint /*Nscalars=0,*/ Nvectors=0,L=1;
	if (D != b.rank()) {
		err << "In a=b[..], b is " << b.rank() << "-dimensional while the number of indices is " << D << ".\n";
		error();
	}
	const TDimPack& bdims = b.dims();
	Tint scalaroffset = 0;
	Tint mult;
    int d;
	for (d=0; d<D; d++) {
		// Compute multiplier
		mult = 1;
		NOVECTOR for (int d1=d+1; d1<D; d1++) mult*= bdims[d1];
		if (indices[d]->kind() == Kint) {
			//scalarindices[Nscalars++] = indices[d]->IntValue();
			if (indices[d]->IntValue() < ArrayBase || indices[d]->IntValue() >= bdims[d]+ArrayBase) {
				err << "In a=b[..], " << d+1 << ". index " << indices[d]->IntValue() << " is out of range "
					<< ArrayBase << " .. " << bdims[d]-1+ArrayBase << ".\n";
				error();
			}
			scalaroffset+= mult*(indices[d]->IntValue() - ArrayBase);
		} else if (indices[d]->kind() == KIntArray) {
			if (indices[d]->rank() != 1) {
				err << "Invalid IntArray index " << Tshort(*indices[d]) << " in a=b[..]. Should be one-dimensional.\n";
				error();
			}
			vectorindices[Nvectors] = indices[d]->IntPtr();
			tobedeleted[Nvectors] = 0;
			veclengths[Nvectors] = indices[d]->length();
			multipliers[Nvectors++] = mult;
			L*= indices[d]->length();
			Tint found;
			if ((found=checkrange(indices[d]->IntPtr(),indices[d]->length(),ArrayBase,bdims[d]-1+ArrayBase)) >= 0) {
				err << "In a=b[..], " << found+1 << ". element of " << d+1 << ". vector index is "
					<< indices[d]->IntPtr()[found] << ",\n which is outside range "
					<< ArrayBase << " .. " << bdims[d]-1+ArrayBase << ".\n";
				error();
			}
		} else if (indices[d]->kind() == Kvoid) {	// Void index means ALLRANGE
			vectorindices[Nvectors] = new Tint [bdims[d]];
			tobedeleted[Nvectors] = 1;
			VECTORIZED for (Tint i=0; i<bdims[d]; i++) vectorindices[Nvectors][i] = i+ArrayBase;
			veclengths[Nvectors] = bdims[d];
			multipliers[Nvectors++] = mult;
			L*= bdims[d];
		} else {
			err << "Invalid index " << Tshort(*indices[d]) << " in a=b[..]. Should be integer or IntArray.\n";
			error();
		}
	}
	if (Nvectors == 0) {		// scalar indexing case
		switch (b.kind()) {
		case KIntArray:
			a = b.IntPtr()[scalaroffset];
			if (MakeStr) a.SetCharFlag();
			break;
		case KRealArray:
			a = b.RealPtr()[scalaroffset];
			break;
		case KComplexArray:
			a = b.ComplexPtr()[scalaroffset];
			break;
		case KObjectArray:
			a = *b.ObjectPtrPtr()[scalaroffset];
			break;
		default:
			err << "Internal (1) in Gather\n";
			error();
		}
		return;		// return from scalar indexing case
	}
	/* Allocate temporary index vector itmp */
	Tint *itmp;
	if (L <= MAX_STATIC_INDEX)
		itmp = &static_itmp[0];
	else
		itmp = new Tint [L];
	// Compute itmp and adims
	Tint i0,i1,i2,i3,i4,i5,i6,i7,offset,ioffset;
	Tint *vindex;
	mult = multipliers[Nvectors-1];
	vindex = vectorindices[Nvectors-1];
	switch (Nvectors) {
	case 1:
		offset = scalaroffset - mult*ArrayBase;
		VECTORIZED for (i0=0; i0<veclengths[0]; i0++)
			itmp[i0] = offset + mult*vindex[i0];
		break;
	case 2:
		for (i0=0; i0<veclengths[0]; i0++) {
			offset = scalaroffset + (vectorindices[0][i0]-ArrayBase)*multipliers[0] - ArrayBase*mult;
			ioffset = i0*veclengths[1];
			VECTORIZED for (i1=0; i1<veclengths[1]; i1++)
				itmp[ioffset+i1] = offset + mult*vindex[i1];
		}
		break;
	case 3:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++) {
			offset =
				scalaroffset + (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1] - ArrayBase*mult;
			ioffset = (i0*veclengths[1]+i1)*veclengths[2];
			VECTORIZED for (i2=0; i2<veclengths[2]; i2++)
				itmp[ioffset+i2] = offset + mult*vindex[i2];
		}
		break;
	case 4:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++) for (i2=0; i2<veclengths[2]; i2++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2] - ArrayBase*mult;
			ioffset = ((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3];
			VECTORIZED for (i3=0; i3<veclengths[3]; i3++)
				itmp[ioffset+i3] = offset + mult*vindex[i3];
		}
		break;
#	if MAXRANK > 4
	case 5:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3] - ArrayBase*mult;
			ioffset = (((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4];
			VECTORIZED for (i4=0; i4<veclengths[4]; i4++)
				itmp[ioffset+i4] = offset + mult*vindex[i4];
		}
		break;
#	endif
#	if MAXRANK > 5
	case 6:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++)
		for (i4=0; i4<veclengths[4]; i4++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3]
				+ (vectorindices[4][i4]-ArrayBase)*multipliers[4] - ArrayBase*mult;
			ioffset = ((((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4]+i4)*veclengths[5];
			VECTORIZED for (i5=0; i5<veclengths[5]; i5++)
				itmp[ioffset+i5] = offset + mult*vindex[i5];
		}
		break;
#	endif
#	if MAXRANK > 6
	case 7:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++)
		for (i4=0; i4<veclengths[4]; i4++) for (i5=0; i5<veclengths[5]; i5++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3]
				+ (vectorindices[4][i4]-ArrayBase)*multipliers[4]
				+ (vectorindices[5][i5]-ArrayBase)*multipliers[5] - ArrayBase*mult;
			ioffset = (((((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4]+i4)*veclengths[5]+i5)*veclengths[5];
			VECTORIZED for (i6=0; i6<veclengths[6]; i6++)
				itmp[ioffset+i6] = offset + mult*vindex[i6];
		}
		break;
#	endif
#	if MAXRANK > 7
	case 8:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++)
		for (i4=0; i4<veclengths[4]; i4++) for (i5=0; i5<veclengths[5]; i5++)
		for (i6=0; i6<veclengths[6]; i6++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3]
				+ (vectorindices[4][i4]-ArrayBase)*multipliers[4]
				+ (vectorindices[5][i5]-ArrayBase)*multipliers[5]
				+ (vectorindices[6][i6]-ArrayBase)*multipliers[6] - ArrayBase*mult;
			ioffset = ((((((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4]+i4)*veclengths[5]+i5)*veclengths[5]+i6)*veclengths[6];
			VECTORIZED for (i7=0; i7<veclengths[7]; i7++)
				itmp[ioffset+i7] = offset + mult*vindex[i7];
		}
		break;
#	endif
	default:
		err << Nvectors << "-dimensional Gather not yet implemented.\n";
		error();
	}
	// Delete possible automatically generated vectors (ALLRANGE case)
	for (d=0; d<Nvectors; d++) if (tobedeleted[d]) delete [] vectorindices[d];
	TDimPack adims(veclengths,Nvectors);
	// Create 'a'. Use temporary a1 because b can be physically same object as a.
	Tint k1;
	Tobject a1;
	switch (b.kind()) {
	case KIntArray:
		{
			a1.ireserv(adims);
			Tint * const ap=a1.IntPtr(), * const bp=b.IntPtr();
			VECTORIZED for (k1=0; k1<L; k1++) ap[k1] = bp[itmp[k1]];
		}
		break;
	case KRealArray:
		{
			a1.rreserv(adims);
			Treal * const ap=a1.RealPtr(), * const bp=b.RealPtr();
			VECTORIZED for (k1=0; k1<L; k1++) ap[k1] = bp[itmp[k1]];
		}
		break;
	case KComplexArray:
		{
			a1.creserv(adims);
			Tcomplex * const ap=a1.ComplexPtr(), * const bp=b.ComplexPtr();
			VECTORIZED for (k1=0; k1<L; k1++) ap[k1] = bp[itmp[k1]];
		}
		break;
	case KObjectArray:
		err << "Vector indices for object array, only scalars are allowed.\n";
		error();
	default:
		err << "Internal (2) in Gather\n";
		error();
	}
	a.bitwiseMoveFrom(a1);
	if (L > MAX_STATIC_INDEX) delete [] itmp;
	if (MakeStr) {
		if (a.kind()==Kint) a.SetCharFlag(); else if (a.kind()==KIntArray) a.SetStringFlag();
	}
} // Gather


static void Scatter1D(const Tobject& a, Tobject& b, const Tobject& index)
// Scatter tailored for 1D case. No integer multiplication, no work arrays ==> fast
// b[index] = a
{
	Tkind k = index.kind();
	if (k == Kint) {
		Tint i = index.IntValue() - ArrayBase;
		if (i<0 || i>=b.length()) {
			err << "Scalar index " << i+ArrayBase << " out of range for " << Tshort(b) << ".\n";
			error();
		}
		switch (b.kind()) {
		case KIntArray:
			if (a.kind()!=Kint) {
				err << "Trying to assign non-integer " << Tshort(a) << " into integer array " << Tshort(b) << ".\n";
				error();
			}
			b.IntPtr()[i] = a.IntValue();
			break;
		case KRealArray:
			if (a.kind()==Kreal)
				b.RealPtr()[i] = a.RealValue();
			else if (a.kind()==Kint)
				b.RealPtr()[i] = a.IntValue();
			else {
				err << "Trying to assign non-real " << Tshort(a) << " into real array " << Tshort(b) << ".\n";
				error();
			}
			break;
		case KComplexArray:
			if (a.kind()==Kcomplex)
				b.ComplexPtr()[i] = a.ComplexValue();
			else if (a.kind()==Kreal)
				b.ComplexPtr()[i] = Tcomplex(a.RealValue(),0.0);
			else if (a.kind()==Kint)
				b.ComplexPtr()[i] = Tcomplex(a.IntValue(),0.0);
			else {
				err << "Trying to assign non-scalar " << Tshort(a) << "into complex array " << Tshort(b) << ".\n";
				error();
			}
			break;
		case KObjectArray:
			*b.ObjectPtrPtr()[i] = a;
			break;
		default:
			err << "Internal (1) in Scatter1D\n";
			error();
		}
	} else if (k == KIntArray) {
		Tint i;
		Tint * const ip = index.IntPtr();
		if (index.rank()!=1) {err << "Invalid index " << Tshort(index) << " in b[..]=a.\n"; error();}
		if (checkrange(ip,index.length(),ArrayBase,b.length()-1+ArrayBase) >= 0) {
			err << "Index vector " << Tshort(index) << " out of range for " << Tshort(b) << ".\n";
			error();
		}
		// Check added 28.8.1995 PJ:
		if (a.IsArray() && (index.length() != a.length())) {
			err << "Index vector length " << index.length() << " disagrees with RHS length "
				<< a.length() << " in vector assignment.\n";
			error();
		}
		switch (b.kind()) {
		case KIntArray:
			if (a.kind() == KIntArray) {
				VECTORIZED for (i=0; i<index.length(); i++) b.IntPtr()[ip[i]-ArrayBase] = a.IntPtr()[i];
			} else if (a.kind() == Kint) {
				VECTORIZED for (i=0; i<index.length(); i++) b.IntPtr()[ip[i]-ArrayBase] = a.IntValue();
			} else {
				err << "Trying to assign non-int-array " << Tshort(a) << " into integer array " << Tshort(b) << ".\n";
				error();
			}
			break;
		case KRealArray:
			if (a.kind()==KRealArray) {
				VECTORIZED for (i=0; i<index.length(); i++) b.RealPtr()[ip[i]-ArrayBase] = a.RealPtr()[i];
			} else if (a.kind()==KIntArray) {
				VECTORIZED for (i=0; i<index.length(); i++) b.RealPtr()[ip[i]-ArrayBase] = a.IntPtr()[i];
			} else if (a.kind() == Kreal) {
				VECTORIZED for (i=0; i<index.length(); i++) b.RealPtr()[ip[i]-ArrayBase] = a.RealValue();
			} else if (a.kind() == Kint) {
				VECTORIZED for (i=0; i<index.length(); i++) b.RealPtr()[ip[i]-ArrayBase] = a.IntValue();
			} else {
				err << "Trying to assign non-real-array " << Tshort(a) << " into real array " << Tshort(b) << ".\n";
				error();
			}
			break;
		case KComplexArray:
			switch (a.kind()) {
			case KComplexArray:
				VECTORIZED for (i=0; i<index.length(); i++) b.ComplexPtr()[ip[i]-ArrayBase] = a.ComplexPtr()[i];
				break;
			case KRealArray:
				VECTORIZED for (i=0; i<index.length(); i++) b.ComplexPtr()[ip[i]-ArrayBase] = Tcomplex(a.RealPtr()[i],0.0);
				break;
			case KIntArray:
				VECTORIZED for (i=0; i<index.length(); i++) b.ComplexPtr()[ip[i]-ArrayBase] = Tcomplex(a.IntPtr()[i],0.0);
				break;
			case Kcomplex:
				VECTORIZED for (i=0; i<index.length(); i++) b.ComplexPtr()[ip[i]-ArrayBase] = a.ComplexValue();
				break;
			case Kreal:
				VECTORIZED for (i=0; i<index.length(); i++) b.ComplexPtr()[ip[i]-ArrayBase] = Tcomplex(a.RealValue(),0.0);
				break;
			case Kint:
				VECTORIZED for (i=0; i<index.length(); i++) b.ComplexPtr()[ip[i]-ArrayBase] = Tcomplex(a.IntValue(),0.0);
				break;
			default:
				err << "Trying to assign non-numeric-array " << Tshort(a) << " into complex array " << Tshort(b) << ".\n";
				error();
			}
			break;
		case KObjectArray:
			err << "Indexed assignment to object array with vector index " << Tshort(index) << ".\n";
			err << "Only scalar indices are allowed for object arrays.\n";
			error();
		default:
			err << "Internal (2) in Scatter1D\n";
			error();
		}
	} else if (k == Kvoid) {	// Void index means ALLRANGE
		b = a;
	} else {
		err << "Invalid index " << Tshort(index) << " in b[..]=a.\n";
		error();
	}
}	// Scatter1D

void Scatter(const Tobject& a, Tobject& b, const TConstObjectPtr indices[], int D) {
	// b[indices] = a
	// Each *indices[i], 0<=i<D, must be either integer or 1D IntArray, or VOID (meaning whole range)
	// Scalar b is allowed: a is then simply assigned to b.
	// In scalar b case a must also we scalar, and indices must be either 1, VOID, or #(1).
	if (!b.IsArray()) {
		if (!b.IsScalar()) {
			err << "In b[..]=a, b = " << Tshort(b) << " is non-numeric\n";
			error();
		}
		if (!a.IsScalar()) {
			err << "In b[..]=a, b = " << Tshort(b) << " is scalar but a = " << Tshort(a) << " is not.\n";
			error();
		}
		b = a;		// result is always equal to a, below we just do consistency checks
		for (int d=0; d<D; d++) {
			const Tkind k = indices[d]->kind();
			if (!(k==Kvoid ||
				  k==Kint && indices[d]->IntValue()==1 ||
				  k==KIntArray && indices[d]->rank()==1 && indices[d]->length()==1 && indices[d]->IntPtr()[0]==1)) {
				err << "In b[..]=a b is scalar but " << d+ArrayBase << ". index is not 1, :, or #(1).\n";
				error();
			}
		}
		return;
	}
	if (D == 1) {Scatter1D(a,b,**indices); return;}
	//Tint scalarindices[MAXRANK];
	Tint *vectorindices[MAXRANK];
	int tobedeleted[MAXRANK];
	Tint veclengths[MAXRANK];
	Tint multipliers[MAXRANK];	// for vector indices only
	Tint Nscalars=0,Nvectors=0,L=1;
	if (D != b.rank()) {
		err << "In b[..]=a, b is " << b.rank() << "-dimensional while the number of indices is " << D << ".\n";
		error();
	}
	const TDimPack& bdims = b.dims();
	Tint scalaroffset = 0;
	Tint mult;
    int d;
	for (d=0; d<D; d++) {
		// Compute multiplier
		mult = 1;
		NOVECTOR for (int d1=d+1; d1<D; d1++) mult*= bdims[d1];
		if (indices[d]->kind() == Kint) {
			//scalarindices[Nscalars++] = indices[d]->IntValue();
			if (indices[d]->IntValue() < ArrayBase || indices[d]->IntValue() >= bdims[d]+ArrayBase) {
				err << "In b[..]=a, " << d+1 << ". index " << indices[d]->IntValue() << " is out of range "
					<< ArrayBase << " .. " << bdims[d]-1+ArrayBase << ".\n";
				error();
			}
			scalaroffset+= mult*(indices[d]->IntValue() - ArrayBase);
		} else if (indices[d]->kind() == KIntArray) {
			if (indices[d]->rank() != 1) {
				err << "Invalid IntArray index " << Tshort(*indices[d]) << " in b[..]=a. Should be one-dimensional.\n";
				error();
			}
			vectorindices[Nvectors] = indices[d]->IntPtr();
			tobedeleted[Nvectors] = 0;
			veclengths[Nvectors] = indices[d]->length();
			multipliers[Nvectors++] = mult;
			L*= indices[d]->length();
			Tint found;
			if ((found=checkrange(indices[d]->IntPtr(),indices[d]->length(),ArrayBase,bdims[d]-1+ArrayBase)) >= 0) {
				err << "In b[..]=a, " << found+1 << ". element of " << d+1 << ". vector index is "
					<< indices[d]->IntPtr()[found] << ",\n which is outside range "
					<< ArrayBase << " .. " << bdims[d]-1+ArrayBase << ".\n";
				error();
			}
		} else if (indices[d]->kind() == Kvoid) {
			vectorindices[Nvectors] = new Tint [bdims[d]];
			tobedeleted[Nvectors] = 1;
			VECTORIZED for (Tint i=0; i<bdims[d]; i++) vectorindices[Nvectors][i] = i+ArrayBase;
			veclengths[Nvectors] = bdims[d];
			multipliers[Nvectors++] = mult;
			L*= bdims[d];
		} else {
			err << "Invalid index " << Tshort(*indices[d]) << " in b[..]=a. Should be integer or IntArray.\n";
			error();
		}
	}
	if (Nvectors == 0) {		// scalar indexing case
		switch (b.kind()) {
		case KIntArray:
			if (a.kind() == Kint)
				b.IntPtr()[scalaroffset] = a.IntValue();
			else {
				err << "Elementwise assigning non-integer " << Tshort(a) << " to integer array "
					<< Tshort(b) << ".\n";
				error();
			}
			break;
		case KRealArray:
			if (a.kind() == Kint)
				b.RealPtr()[scalaroffset] = a.IntValue();
			else if (a.kind() == Kreal)
				b.RealPtr()[scalaroffset] = a.RealValue();
			else {
				err << "Elementwise assigning non-real " << Tshort(a) << " to real array "
					<< Tshort(b) << ".\n";
				error();
			}
			break;
		case KComplexArray:
			if (a.kind() == Kint)
				b.ComplexPtr()[scalaroffset] = Tcomplex(a.IntValue(),0.0);
			else if (a.kind() == Kreal)
				b.ComplexPtr()[scalaroffset] = Tcomplex(a.RealValue(),0.0);
			else if (a.kind() == Kcomplex)
				b.ComplexPtr()[scalaroffset] = a.ComplexValue();
			else {
				err << "Elementwise assigning non-scalar " << Tshort(a) << " to complex array "
					<< Tshort(b) << ".\n";
				error();
			}
			break;
		case KObjectArray:
			*b.ObjectPtrPtr()[scalaroffset] = a;
			break;
		default:
			err << "Internal (1) in Scatter\n";
			error();
		}
		return;		// return from scalar indexing case
	}
	/* Allocate temporary index vector itmp */
	Tint *itmp;
	if (L <= MAX_STATIC_INDEX)
		itmp = &static_itmp[0];
	else
		itmp = new Tint [L];
	//TDimPack adims;
	// Compute itmp and adims
	Tint i0,i1,i2,i3,i4,i5,i6,i7,offset,ioffset;
	Tint *vindex;
	mult = multipliers[Nvectors-1];
	vindex = vectorindices[Nvectors-1];
	switch (Nvectors) {
	case 1:
		offset = scalaroffset - mult*ArrayBase;
		VECTORIZED for (i0=0; i0<veclengths[0]; i0++)
			itmp[i0] = offset + mult*vindex[i0];
		break;
	case 2:
		for (i0=0; i0<veclengths[0]; i0++) {
			offset = scalaroffset + (vectorindices[0][i0]-ArrayBase)*multipliers[0] - ArrayBase*mult;
			ioffset = i0*veclengths[1];
			VECTORIZED for (i1=0; i1<veclengths[1]; i1++)
				itmp[ioffset+i1] = offset + mult*vindex[i1];
		}
		break;
	case 3:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++) {
			offset =
				scalaroffset + (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1] - ArrayBase*mult;
			ioffset = (i0*veclengths[1]+i1)*veclengths[2];
			VECTORIZED for (i2=0; i2<veclengths[2]; i2++)
				itmp[ioffset+i2] = offset + mult*vindex[i2];
		}
		break;
	case 4:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++) for (i2=0; i2<veclengths[2]; i2++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2] - ArrayBase*mult;
			ioffset = ((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3];
			VECTORIZED for (i3=0; i3<veclengths[3]; i3++)
				itmp[ioffset+i3] = offset + mult*vindex[i3];
		}
		break;
#	if MAXRANK > 4
	case 5:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3] - ArrayBase*mult;
			ioffset = (((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4];
			VECTORIZED for (i4=0; i4<veclengths[4]; i4++)
				itmp[ioffset+i4] = offset + mult*vindex[i4];
		}
		break;
#	endif
#	if MAXRANK > 5
	case 6:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++)
		for (i4=0; i4<veclengths[4]; i4++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3]
				+ (vectorindices[4][i4]-ArrayBase)*multipliers[4] - ArrayBase*mult;
			ioffset = ((((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4]+i4)*veclengths[5];
			VECTORIZED for (i5=0; i5<veclengths[5]; i5++)
				itmp[ioffset+i5] = offset + mult*vindex[i5];
		}
		break;
#	endif
#	if MAXRANK > 6
	case 7:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++)
		for (i4=0; i4<veclengths[4]; i4++) for (i5=0; i5<veclengths[5]; i5++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3]
				+ (vectorindices[4][i4]-ArrayBase)*multipliers[4]
				+ (vectorindices[5][i5]-ArrayBase)*multipliers[5] - ArrayBase*mult;
			ioffset = (((((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4]+i4)*veclengths[5]+i5)*veclengths[6];
			VECTORIZED for (i6=0; i6<veclengths[6]; i6++)
				itmp[ioffset+i6] = offset + mult*vindex[i6];
		}
		break;
#	endif
#	if MAXRANK > 7
	case 8:
		for (i0=0; i0<veclengths[0]; i0++) for (i1=0; i1<veclengths[1]; i1++)
		for (i2=0; i2<veclengths[2]; i2++) for (i3=0; i3<veclengths[3]; i3++)
		for (i4=0; i4<veclengths[4]; i4++) for (i5=0; i5<veclengths[5]; i5++)
		for (i6=0; i6<veclengths[6]; i6++) {
			offset =
				scalaroffset
				+ (vectorindices[0][i0]-ArrayBase)*multipliers[0]
				+ (vectorindices[1][i1]-ArrayBase)*multipliers[1]
				+ (vectorindices[2][i2]-ArrayBase)*multipliers[2]
				+ (vectorindices[3][i3]-ArrayBase)*multipliers[3]
				+ (vectorindices[4][i4]-ArrayBase)*multipliers[4]
				+ (vectorindices[5][i5]-ArrayBase)*multipliers[5]
				+ (vectorindices[6][i6]-ArrayBase)*multipliers[6] - ArrayBase*mult;
			ioffset = ((((((i0*veclengths[1]+i1)*veclengths[2]+i2)*veclengths[3]+i3)*veclengths[4]+i4)*veclengths[5]+i5)*veclengths[6]+i6)*veclengths[7];
			VECTORIZED for (i7=0; i7<veclengths[7]; i7++)
				itmp[ioffset+i7] = offset + mult*vindex[i7];
		}
		break;
#	endif
	default:
		err << Nvectors << "-dimensional Scatter not yet implemented.\n";
		error();
	}
	// Delete possible automatically generated vectors (ALLRANGE case)
	for (d=0; d<Nvectors; d++) if (tobedeleted[d]) delete [] vectorindices[d];
	TDimPack adims(veclengths,Nvectors);
	if (a.IsArray()) {
		// Check that adims agrees with a's actual dims
		if (adims != a.dims()) {
			err << "In b[..]=a, the dimensionality of the index pack " << adims
				<< " does not agree with a=" << Tshort(a) << ".\n";
			error();
		}
	}
	// Now actually move the data
	Tint k1;
	switch (b.kind()) {
	case KIntArray:
		if (a.kind() == KIntArray) {
			VECTORIZED for (k1=0; k1<L; k1++) b.IntPtr()[itmp[k1]] = a.IntPtr()[k1];
		} else if (a.kind() == Kint) {
			VECTORIZED for (k1=0; k1<L; k1++) b.IntPtr()[itmp[k1]] = a.IntValue();
		} else {
			err << "In b[..]=a, b is " << Tshort(b) << " while a is " << Tshort(a) << ".\n";
			error();
		}
		break;
	case KRealArray:
		if (a.kind() == KRealArray) {
			VECTORIZED for (k1=0; k1<L; k1++) b.RealPtr()[itmp[k1]] = a.RealPtr()[k1];
		} else if (a.kind() == Kreal) {
			VECTORIZED for (k1=0; k1<L; k1++) b.RealPtr()[itmp[k1]] = a.RealValue();
		} else if (a.kind() == Kint) {
			VECTORIZED for (k1=0; k1<L; k1++) b.RealPtr()[itmp[k1]] = a.IntValue();
		} else if (a.kind() == KIntArray) {
			VECTORIZED for (k1=0; k1<L; k1++) b.RealPtr()[itmp[k1]] = a.IntPtr()[k1];
		} else {
			err << "In b[..]=a, b is " << Tshort(b) << " while a is " << Tshort(a) << ".\n";
			error();
		}
		break;
	case KComplexArray:
		switch (a.kind()) {
		case KComplexArray:
			VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = a.ComplexPtr()[k1];
			break;
		case Kcomplex:
			VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = a.ComplexValue();
			break;
		case Kreal:
			VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = Tcomplex(a.RealValue(),0.0);
			break;
		case Kint:
			VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = Tcomplex(a.IntValue(),0.0);
			break;
		case KRealArray:
			VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = Tcomplex(a.RealPtr()[k1],0.0);
			break;
		case KIntArray:
			VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = Tcomplex(a.IntPtr()[k1],0.0);
			break;
		default:
			err << "In b[..]=a, b is " << Tshort(b) << " while a is " << Tshort(a) << ".\n";
			error();
		}
		break;
	case KObjectArray:
		err << "Indexed assignment to object array with vector indices is not allowed.\n";
		error();
	default:
		err << "Internal (2) in Scatter\n";
		error();
	}
	if (L > MAX_STATIC_INDEX) delete [] itmp;
} // Scatter
		

void MGatScat(Tobject& a, Tobject& b, const TObjectPtr indices[], int D, int scatflag) {
	// Mgather: a = b[indices], Mscatter: b[indices] = a
	// indices must be array of object pointers of size D, D=dimensionality (rank) of b
	// all index objects must be IntArray's of the same size
	/* Various checks */
	int r = b.rank();
	if (D != r) {err << "MGatScat: index length " << D << " not equal to rank = " << r << '\n'; error();}
	Tkind k = indices[0]->kind();
	if (k!=KIntArray) {err << "MGatScat: indices[0] not IntArray\n"; error();}
	TDimPack dims = indices[0]->dims();
    int i;
	for (i=1; i<r; i++) {
		if (indices[i]->kind()!=k) {err << "MGatScat: indices of differing kinds\n"; error();}
		if (indices[i]->dims()!=dims) {err << "MGatScat: indices of different dims\n"; error();}
	}
	if (scatflag) {
		/* Check that a and indices[0] dimensions agree */
		if (dims!=a.dims()) {err << "MGatScat/SCAT: a and indices[0] dimensions do not agree\n"; error();}
	}
	/* Allocate temporary index vector itmp */
	int L = indices[0]->length();
	Tint *itmp;
	if (L <= MAX_STATIC_INDEX)
		itmp = &static_itmp[0];
	else
		itmp = new Tint [L];
	/* Compute itmp */
	VECTORIZED for (i=0; i<L; i++) itmp[i] = 0;
	TDimPack dp = b.dims();
	for (int d=0; d<r; d++) {
		int m = 1;
		NOVECTOR for (int d1=d+1; d1<r; d1++) m*= dp[d1];
		VECTORIZED for (int kk=0; kk<L; kk++) itmp[kk]+= (indices[d]->IntPtr()[kk]-ArrayBase)*m;
	}
	/* Check dimension overflows */
	int Nerrors = 0;
	VECTORIZED for (i=0; i<L; i++) if (itmp[i]<0 || itmp[i]>=b.length()) Nerrors++;
	if (Nerrors) {
		for (i=0; i<L; i++) if (itmp[i]<0 || itmp[i]>=b.length()) {
			err << "Indexing overflow in " << (scatflag ? "Mscat" : "Mgath" ) << ": "
				<< itmp[i] << " exceeds " << Tshort(b) << '\n';
			error();
		}
	}
	if (!scatflag) {
		/* Allocate result */
		switch (b.kind()) {
		case KIntArray:
			a.ireserv(dims);
			break;
		case KRealArray:
			a.rreserv(dims);
			break;
		case KComplexArray:
			a.creserv(dims);
			break;
		case KObjectArray:
			err << "Mapped indexing for object arrays not yet implemented.\n";
			error();
		default:
			err << "Internal: in Mgatscat (1)";
			error();
			break;
		}
	}
	/* The dirty work: a[k1] = b[itmp[k1]] */
	int k1;
	if (scatflag) {
		switch (b.kind()) {
		case KIntArray:
			if (a.kind()!=KIntArray) {
				err << "Mscat: Since " << Tshort(b) << " is IntArray, " << Tshort(a) << " must be, too\n";
				error();
			}
			VECTORIZED for (k1=0; k1<L; k1++) b.IntPtr()[itmp[k1]] = a.IntPtr()[k1];
			break;
		case KRealArray:
			if (a.kind() == KRealArray) {
				VECTORIZED for (k1=0; k1<L; k1++) b.RealPtr()[itmp[k1]] = a.RealPtr()[k1];
			} else if (a.kind() == KIntArray) {
				VECTORIZED for (k1=0; k1<L; k1++) b.RealPtr()[itmp[k1]] = a.IntPtr()[k1];
			} else {
				err << "Mscat: Since " << Tshort(b) << " is RealArray, " << Tshort(a) << " must be real array, too\n";
				error();
			}
			break;
		case KComplexArray:
			if (a.kind() == KComplexArray) {
				VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = a.ComplexPtr()[k1];
			} else if (a.kind() == KRealArray) {
				VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = Tcomplex(a.RealPtr()[k1],0.0);
			} else if (a.kind() == KIntArray) {
				VECTORIZED for (k1=0; k1<L; k1++) b.ComplexPtr()[itmp[k1]] = Tcomplex(a.IntPtr()[k1],0.0);
			} else {
				err << "Mscat: Since " << Tshort(b) << " is numeric array, " << Tshort(a) << " must be, too\n";
				error();
			}
			break;
		default:
			err << "Internal: in Mgatscar (2)";
			error();
			break;
		}
	} else {
		switch (a.kind()) {
		case KIntArray:
			VECTORIZED for (k1=0; k1<L; k1++) a.IntPtr()[k1] = b.IntPtr()[itmp[k1]];
			break;
		case KRealArray:
			VECTORIZED for (k1=0; k1<L; k1++) a.RealPtr()[k1] = b.RealPtr()[itmp[k1]];
			break;
		case KComplexArray:
			VECTORIZED for (k1=0; k1<L; k1++) a.ComplexPtr()[k1] = b.ComplexPtr()[itmp[k1]];
			break;
		case KObjectArray:
			err << "Mapped indexing for object arrays not yet implemented.\n";
			error();
		default:
			err << "Internal: in Mgatscar (2)";
			error();
			break;
		}
	}
	if (L > MAX_STATIC_INDEX) delete [] itmp;
} /* MGatScat */


static void MakeVector(TObjectPtr a, Tkind akind, const TConstObjectPtr args[], int N)
// args[N] must all be scalars, and akind must conform the highest type scalar
{
	int n;
	switch (akind) {
	case KIntArray:
		a->ireserv(TDimPack(N));
		for (n=0; n<N; n++) a->IntPtr()[n] = args[n]->IntValue();
		break;
	case KRealArray:
		a->rreserv(TDimPack(N));
		for (n=0; n<N; n++)
			a->RealPtr()[n] = (args[n]->kind()==Kint) ? Treal(args[n]->IntValue()) : args[n]->RealValue();
		break;
	case KComplexArray:
		a->creserv(TDimPack(N));
		for (n=0; n<N; n++)
			if (args[n]->kind()==Kint)
				a->ComplexPtr()[n] = Tcomplex(args[n]->IntValue(),0.0);
			else if (args[n]->kind()==Kreal)
				a->ComplexPtr()[n] = Tcomplex(args[n]->RealValue(),0.0);
			else
				a->ComplexPtr()[n] = args[n]->ComplexValue();
		break;
	default:;
	}
}

static void MakeArray(TObjectPtr a, Tkind akind, const TConstObjectPtr args[], int N, int maxrank, const Tint adims[MAXRANK])
{
	int n;
	Tint i;
	switch (akind) {
	case KIntArray:
		{
			a->ireserv(TDimPack(adims,maxrank));
			Tint *iptr = a->IntPtr();
			for (n=0; n<N; n++) {
#				if USE_MEMCPY
				memcpy(iptr,args[n]->IntPtr(),sizeof(Tint)*args[n]->length());
#				else
				VECTORIZED for (i=0; i<args[n]->length(); i++) iptr[i] = args[n]->IntPtr()[i];
#				endif
				iptr+= args[n]->length();
			}
		}
		break;
	case KRealArray:
		{
			a->rreserv(TDimPack(adims,maxrank));
			Treal *rptr = a->RealPtr();
			for (n=0; n<N; n++) {
				if (args[n]->kind()==KIntArray)
					for (i=0; i<args[n]->length(); i++) rptr[i] = Treal(args[n]->IntPtr()[i]);
				else {
#					if USE_MEMCPY
					memcpy(rptr,args[n]->RealPtr(),sizeof(Treal)*args[n]->length());
#					else
					VECTORIZED for (i=0; i<args[n]->length(); i++) rptr[i] = args[n]->RealPtr()[i];
#					endif
				}
				rptr+= args[n]->length();
			}
		}
		break;
	case KComplexArray:
		{
			a->creserv(TDimPack(adims,maxrank));
			Tcomplex *cptr = a->ComplexPtr();
			for (n=0; n<N; n++) {
				if (args[n]->kind()==KIntArray)
					for (i=0; i<args[n]->length(); i++) cptr[i] = Tcomplex(args[n]->IntPtr()[i],0.0);
				else if (args[n]->kind()==KRealArray)
					for (i=0; i<args[n]->length(); i++) cptr[i] = Tcomplex(args[n]->RealPtr()[i],0.0);
				else {
#					if USE_MEMCPY
					memcpy(cptr,args[n]->ComplexPtr(),sizeof(Tcomplex)*args[n]->length());
#					else
					VECTORIZED for (i=0; i<args[n]->length(); i++) cptr[i] = args[n]->ComplexPtr()[i];
#					endif
				}
				// The following line was added for Tela 1.21. In previous versions #(...; ...) worked
				// incorrectly if any of the slots was complex.
				cptr+= args[n]->length();
			}
		}
		break;
	default:;
	}
}

void PasteArrays(Tobject& result, const TConstObjectPtr args[], int N)
// result = #( args[0], args[1], ..., args[N-1] )
// restriction: N>0
// If all args are scalars, the result 'a' is a vector containing args as component.
// If some args are vectors and some are (possibly) scalars, the result is a vector.
// If all args are at least vectors, then the maximum and minimum ranks may differ
// by at most one. Then the args are appended along the leading dimension of the maximum
// rank. Args whose rank is one less than the maximum rank contribute to slices of
// unit thickness in the result.
{
	int minrank=MAXRANK+1, maxrank=0;
	Tkind k,akind=KIntArray;
	int MakeStr = 1;
    int n;
	for (n=0; n<N; n++) {
		k = args[n]->kind();
		if (k==KIntArray || k==KRealArray || k==KComplexArray) {
			minrank = min(args[n]->rank(),minrank);
			maxrank = max(args[n]->rank(),maxrank);
			akind = (Tkind)max(int(akind),int(k));
			if (!args[n]->HasStringFlag()) MakeStr = 0;
		} else if (k==Kint || k==Kreal || k==Kcomplex) {
			minrank = 0;
			if (k==Kreal) akind = (Tkind)max(int(akind),int(KRealArray));
			if (k==Kcomplex) akind = (Tkind)max(int(akind),int(KComplexArray));
			if (!args[n]->IsChar()) MakeStr = 0;
		} else {
			err << "Undefined or non-numeric object " << *args[n] << " in array pasting #(x,y,...)\n";
			error();
		}
	}
	if (minrank>=MAXRANK+1) minrank=0;
	if (maxrank-minrank!=0 && maxrank-minrank!=1) {
		err << "In array pasting with [a,b,...], the ranks of arrays a,b,.. differ by " << maxrank-minrank << ".\n";
		err << "The maximum rank is " << maxrank << " and minimum is " << minrank << ".\n";
		err << "They should differ by a most one.\n";
		error();
	}
	// Check whether any of the args is physically the same object as 'result'
	int PhysicallySame = 0;
	TObjectPtr a;
	for (n=0; n<N; n++) if (args[n] == &result) {PhysicallySame=1; break;}
	if (PhysicallySame)
		a = new Tobject;
	else
		a = &result;
	if (maxrank == 0) {			// all args were scalars, construct a vector
		MakeVector(a,akind,args,N);
		if (MakeStr && akind==KIntArray) a->SetStringFlag();
	} else if (maxrank == 1) {	// args were vectors and possibly scalars, construct a vector
		// compute length of a
		Tint lena = 0;
		for (n=0; n<N; n++) {
			if (args[n]->IsArray())
				lena+= args[n]->length();
			else
				lena++;
		}
		Tint i = 0, j;
		switch (akind) {
		case KIntArray:
			a->ireserv(TDimPack(lena));
			for (n=0; n<N; n++) {
				if (args[n]->IsArray()) {
					VECTORIZED for (j=0; j<args[n]->length(); j++) a->IntPtr()[i++] = args[n]->IntPtr()[j];
				} else
					a->IntPtr()[i++] = args[n]->IntValue();
			}
			break;
		case KRealArray:
			a->rreserv(TDimPack(lena));
			for (n=0; n<N; n++) {
				if (args[n]->IsArray()) {
					if (args[n]->kind()==KIntArray) {
						VECTORIZED for (j=0; j<args[n]->length(); j++) a->RealPtr()[i++] = Treal(args[n]->IntPtr()[j]);
					} else {
						VECTORIZED for (j=0; j<args[n]->length(); j++) a->RealPtr()[i++] = args[n]->RealPtr()[j];
					}
				} else {
					if (args[n]->kind()==Kint)
						a->RealPtr()[i++] = Treal(args[n]->IntValue());
					else
						a->RealPtr()[i++] = args[n]->RealValue();
				}
			}
			break;
		case KComplexArray:
			a->creserv(TDimPack(lena));
			for (n=0; n<N; n++) {
				if (args[n]->IsArray()) {
					if (args[n]->kind()==KIntArray) {
						VECTORIZED for (j=0; j<args[n]->length(); j++)
							a->ComplexPtr()[i++] = Tcomplex(args[n]->IntPtr()[j],0.0);
					} else if (args[n]->kind()==KRealArray) {
						VECTORIZED for (j=0; j<args[n]->length(); j++)
							a->ComplexPtr()[i++] = Tcomplex(args[n]->RealPtr()[j],0.0);
					} else {
						VECTORIZED for (j=0; j<args[n]->length(); j++)
							a->ComplexPtr()[i++] = args[n]->ComplexPtr()[j];
					}
				} else {
					if (args[n]->kind()==Kint)
						a->ComplexPtr()[i++] = Tcomplex(args[n]->IntValue(),0.0);
					else if (args[n]->kind()==Kreal)
						a->ComplexPtr()[i++] = Tcomplex(args[n]->RealValue(),0.0);
					else
						a->ComplexPtr()[i++] = args[n]->ComplexValue();
				}
			}
			break;
		default:;
		}
		if (MakeStr && akind==KIntArray) a->SetStringFlag();
	} else {		// maxrank>=2, all args were arrays
		// compute leading dimension of a and check that other dims agree
		int d;
		Tint lda;
		Tint otherdims[MAXRANK];
		if (args[0]->rank()==maxrank) {
			NOVECTOR for (d=1; d<maxrank; d++) otherdims[d-1] = args[0]->dims()[d];
			lda = args[0]->dims()[0];
		} else {
			NOVECTOR for (d=0; d<maxrank-1; d++) otherdims[d] = args[0]->dims()[d];
			lda = 1;
		}
		for (n=1; n<N; n++) {
			if (args[n]->rank()==maxrank) {
				for (d=1; d<maxrank; d++)
					if (otherdims[d-1] != args[n]->dims()[d]) {
						err << "In array pasting with [a,b,...], the non-leading dimensions of a,b,.. do not agree.\n";
						error();
					}
				lda+= args[n]->dims()[0];
			} else {
				for (d=0; d<maxrank-1; d++)
					if (otherdims[d] != args[n]->dims()[d]) {
						err << "In array pasting with [a,b,...], the non-leading dimensions of a,b,.. do not agree.\n";
						error();
					}
				lda++;
			}
		}
		Tint adims[MAXRANK];
		adims[0] = lda;
		NOVECTOR for (d=0; d<maxrank-1; d++) adims[d+1] = otherdims[d];
		MakeArray(a,akind,args,N,maxrank,adims);
	}
	if (PhysicallySame) {
		result.bitwiseMoveFrom(*a);
		delete a;
	}
}

static int AllScalars(const TConstObjectPtr args[], int N, Tkind& k, int& AllChars) {
	k = Kint;
	AllChars = 1;
	for (int n=0; n<N; n++) {
		if (!args[n]->IsScalar()) {AllChars=0; return 0;}
		k = (Tkind)max(int(k),int(args[n]->kind()));
		if (!args[n]->IsChar()) AllChars = 0;
	}
	return 1;
}

void BuildArray(Tobject& result, const TConstObjectPtr args[], int N)
{
	if (N<=0) {result.izeros(TDimPack(0)); return;}
	int PhysicallySame = 0;
	TObjectPtr a;
	for (int n=0; n<N; n++) if (args[n] == &result) {PhysicallySame=1; break;}
	if (PhysicallySame)
		a = new Tobject;
	else
		a = &result;
	Tkind akind;
	int AllChars=0;
	if (AllScalars(args,N,akind,AllChars)) {
		akind = (Tkind)(akind + KIntArray - Kint);
		MakeVector(a,akind,args,N);
		if (AllChars) a->SetStringFlag();
	} else {
		TDimPack dims;
		akind = KIntArray;
		int MakeStr = 1;
		if (N==0) MakeStr = 0;
		for (int n=0; n<N; n++) {
			Tkind k = args[n]->kind();
			if (k!=KIntArray || !args[n]->HasStringFlag()) MakeStr = 0;
			if (k!=KIntArray && k!=KRealArray && k!=KComplexArray) {
				err << "In array builder [a; b; ...], the objects a,b,.. must be numeric arrays (or all scalars).\n";
				err << n+1 << ". object is now " << Tshort(*(args[n])) << ".\n";
				error();
			}
			akind = (Tkind)max(int(akind),int(k));
			if (args[n]->rank() > MAXRANK-1) {
				err << "Too high rank (" << args[n]->rank() << ") object in array builder [a; b; ...].\n";
				error();
			}
			if (n==0)
				dims = args[0]->dims();
			else {
				if (args[n]->dims() != dims) {
					err << "In array builder [a; b; ...], the dimensions of objects disagree.\n";
					error();
				}
			}
		}
		Tint adims[MAXRANK];
		NOVECTOR for (int d=0; d<args[0]->rank(); d++) adims[d+1] = args[0]->dims()[d];
		adims[0] = N;
		MakeArray(a,akind,args,N,args[0]->rank()+1,adims);
		if (MakeStr && akind==KIntArray) a->SetStringFlag();
	}
	if (PhysicallySame) {
		result.bitwiseMoveFrom(*a);
		delete a;
	}
}


void Range(TObjectPtr d[4])
{
	int i, type = Kint;
	for (i=1; i<=3; i++) {
		if (d[i]->kind() == Kreal)
			type = Kreal;
		else if (d[i]->kind() != Kint) {
			err << "Operands of RANGE must be integers or reals, not " << Tshort(*d[i]) << "\n";
			error();
		}
	}
	if (type == Kint) {
		Tint a,b,s,N;
		a = d[1]->IntValue();
		b = d[2]->IntValue();
		s = d[3]->IntValue();
		if (s==0) {err<<"RANGE: zero step\n"; error();}
		N = max(0,(b-a)/s + 1);
		d[0]->ireserv(TDimPack(N));
		Tint x=a;
		Tint * const dp = d[0]->IntPtr();
		VECTORIZED for (i=0; i<N; i++) {
			dp[i] = x;
			x+= s;
		}
	} else {	// type == Kreal
		Treal a,b,s;
		Tint N;
		a = (d[1]->kind()==Kint) ? Treal(d[1]->IntValue()) : d[1]->RealValue();
		b = (d[2]->kind()==Kint) ? Treal(d[2]->IntValue()) : d[2]->RealValue();
		s = (d[3]->kind()==Kint) ? Treal(d[3]->IntValue()) : d[3]->RealValue();
		if (s==0) {err<<"RANGE: zero step\n"; error();}
		N = max(0,int(0.5 + (b-a)/s) + 1);
		d[0]->rreserv(TDimPack(N));
		Treal x=a;
		Treal * const dp = d[0]->RealPtr();
		VECTORIZED for (i=0; i<N; i++) {
			dp[i] = x;
			x+= s;
		}
	}
}
