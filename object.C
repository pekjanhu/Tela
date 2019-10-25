/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "object.H"
#endif
#include "object.H"
#include <cctype>
#if USE_STRINGSTREAM
#  include <sstream>
#else
#  include <strstream.h>
#endif

/*
  Notice: When we allocate a ComplexArray object complex table, we
  do it by allocating 2*N Treal values. This works providing Tcomplex
  consists of exactly two Treals. Correspondingly when we delete
  ComplexArray, we delete [] a.xptr, not a.zptr. This is because sometimes
  a complex array is allocated using setsize(), which does 'new Tint []'.
  Some implementations of C++ systems (some g++ at least) tend to
  crash if an array of objects is newed and deleted using different
  types, even if the sizes match. This problem does not occur if the
  deleted objects have no constructor/destructor, thus we can freely
  mix Tint/Treal new and delete.

  The original 'new Tcomplex' etc. appear commented-out.
*/

// TDimPack members

TDimPack::TDimPack(const Tint ds[], int n) : len(1), Ndims(n) {
	if (n>MAXRANK) {err<<"TDimPack constructor: n="<<n<<" (max=" << MAXRANK << ").\n"; error();}
	NOVECTOR for (int i=0; i<n; i++) {
		dims[i] = ds[i];
		len*= ds[i];
	}
}

TDimPack::TDimPack(const TDimPack& dimpack) : len(dimpack.len), Ndims(dimpack.Ndims) {
	NOVECTOR for (int i=0; i<Ndims; i++)
		dims[i] = dimpack.dims[i];
}

TDimPack& TDimPack::operator=(const TDimPack& dimpack) {
	Ndims = dimpack.Ndims;
	NOVECTOR for (int i=0; i<Ndims; i++)
		dims[i] = dimpack.dims[i];
	len = dimpack.len;
	return *this;
}

int TDimPack::operator==(const TDimPack& dimpack) const {	// test TDimPack equality
	if (Ndims!=dimpack.Ndims || len!=dimpack.len) return 0;
	NOVECTOR for (int i=0; i<Ndims; i++)
		if (dims[i]!=dimpack.dims[i]) return 0;
	return 1;	// all tests passed, return true
}

int TDimPack::iscompatible(const TDimPack& dimpack) const {
	if (len!=dimpack.len) return 0;					// if lengths are unequal, they are never compatible
	if (Ndims==1 || dimpack.Ndims==1) return 1;		// if at least one is vector, they are compatible
	if (Ndims!=dimpack.Ndims) return 0;				// otherwise the ranks must be equal
	NOVECTOR for (int i=0; i<Ndims; i++)
		if (dims[i]!=dimpack.dims[i]) return 0;		// and all dims must agree
	return 1;
}

ostream& operator<<(ostream& o, const TDimPack& dimpack) {
	o << '[';
	for (int d=0; d<dimpack.rank(); d++) {
		o << dimpack.dims[d];
		if (d<dimpack.rank()-1) o << ',';
	}
	return o << ']';
}

istream& operator>>(istream& i, TDimPack& dimpack) {
	Tchar ch;
	i >> ch;
	if (ch == '[') {
		dimpack.Ndims = 0;
		dimpack.len = 1;
		for(int d=0; d<MAXRANK; d++) {
			i >> dimpack.dims[d];
			i >> ch;
			if (ch!=',' && ch!=']') {err<<"Syntax error when reading DimPack\n"; error();}
			dimpack.Ndims++;
			dimpack.len*= dimpack.dims[d];
			if (ch==']') break;
		}
	} else {
		i.putback(ch);
	}
	return i;
}

// Tobject members
// ---------------

// Private 'init' functions

// Inline version of object copy initializer

INLINE void init_inline(Tobject& o, const Tobject& obj)
// Object 'o' corresponds to 'this'
{
#	if !USE_MEMCPY
	int j;
#	endif
	o.k = obj.k;
	switch (o.k) {
	case Kint:
		o.integer.i = obj.integer.i;
		o.integer.charflag = obj.integer.charflag;
		return;
	case Kreal:
		o.x = obj.x;
		return;
	case Kcomplex:
		o.z = obj.z;
		return;
	case KIntArray:
		o.a.iptr = new Tint [obj.length()];
#		if USE_MEMCPY
		memcpy(o.a.iptr,obj.a.iptr,obj.length()*sizeof(Tint));
#		else
		VECTORIZED for (j=0; j<obj.length(); j++) o.a.iptr[j] = obj.a.iptr[j];
#		endif
		o.dimpack = obj.dimpack;
		o.a.strflag = obj.a.strflag;
		return;
	case KRealArray:
		o.a.xptr = new Treal [obj.length()];
#		if USE_MEMCPY
		memcpy(o.a.xptr,obj.a.xptr,obj.length()*sizeof(Treal));
#		else
		VECTORIZED for (j=0; j<obj.length(); j++) o.a.xptr[j] = obj.a.xptr[j];
#		endif
		o.dimpack = obj.dimpack;
		return;
	case KComplexArray:
		//a.zptr = new Tcomplex [obj.length()];
		o.a.xptr = new Treal [2*obj.length()];
#		if USE_MEMCPY
		memcpy(o.a.zptr,obj.a.zptr,obj.length()*sizeof(Tcomplex));
#		else
		VECTORIZED for (j=0; j<obj.length(); j++) o.a.zptr[j] = obj.a.zptr[j];
#		endif
		o.dimpack = obj.dimpack;
		return;
	case KObjectArray:
		o.a.opptr = new TObjectPtr [obj.length()];
		// Changed 28.2.2001 to use deep rather than shallow copy /Pj

		// Deep copy version:
		{
			int j;
			for (j=0; j<obj.length(); j++) o.a.opptr[j] = new Tobject(*obj.a.opptr[j]);
		}
	
#		if 0
		// The old shallow-copy version:
#		if USE_MEMCPY
		memcpy(o.a.opptr,obj.a.opptr,obj.length()*sizeof(TObjectPtr));
#		else
		VECTORIZED for (j=0; j<obj.length(); j++) o.a.opptr[j] = obj.a.opptr[j];
#		endif
#		endif
		
		o.dimpack = obj.dimpack;
		return;
	case Kfunction:
		o.funcptr = obj.funcptr;
		return;
	case KCfunction:
		o.Cfunc.ptr = obj.Cfunc.ptr;
		o.Cfunc.infoptr = obj.Cfunc.infoptr;
		return;
	case KIntrinsicFunction:
		o.intr.ptr = obj.intr.ptr;
		o.intr.code = obj.intr.code;
		return;
	case Kvoid:
	case KShallowObjectArray:
	case Kundef:
		return;
	}
}

void Tobject::init(const Tobject& obj) {init_inline(*this,obj);}

void Tobject::init(const Tchar *str) {
	k = KIntArray;
	a.strflag = 1;
	int L = strlen(str);
	dimpack = L;
	a.iptr = new Tint [L];
	for (int i=0; i<L; i++) a.iptr[i] = str[i];
}

// Tobject constructors

Tobject::Tobject(Treal x, Treal y) {
	k = Kcomplex;
	z = Tcomplex(x,y);
	countit();
}

Tobject::Tobject(const Tint itab[], int N, int stringflag) : dimpack(N) {
	k = KIntArray;
	a.strflag = stringflag;
	a.iptr = new Tint [N];
	VECTORIZED for (int i=0; i<N; i++) a.iptr[i] = itab[i];
	countit();
}

Tobject::Tobject(const Treal xtab[], int N) : dimpack(N) {
	k = KRealArray;
	a.xptr = new Treal [N];
	VECTORIZED for (int i=0; i<N; i++) a.xptr[i] = xtab[i];
	countit();
}

Tobject::Tobject(const Tcomplex ztab[], int N) : dimpack(N) {
	k = KComplexArray;
	//a.zptr = new Tcomplex [N];
	a.xptr = new Treal [2*N];
	VECTORIZED for (int i=0; i<N; i++) a.zptr[i] = ztab[i];
	countit();
}

Tobject::Tobject(const TObjectPtr otab[], int N) : dimpack(N) {
	k = KObjectArray;
	a.opptr = new TObjectPtr [N];
	VECTORIZED for (int i=0; i<N; i++) a.opptr[i] = otab[i];
	countit();
}

Tobject::Tobject(const Tint itab[], const TDimPack& dp) : dimpack(dp) {
	k = KIntArray;
	a.strflag = 0;
	a.iptr = new Tint [dp.length()];
	VECTORIZED for (int i=0; i<dp.length(); i++) a.iptr[i] = itab[i];
	countit();
}

Tobject::Tobject(const Treal xtab[], const TDimPack& dp) : dimpack(dp) {
	k = KRealArray;
	a.xptr = new Treal [dp.length()];
	VECTORIZED for (int i=0; i<dp.length(); i++) a.xptr[i] = xtab[i];
	countit();
}

Tobject::Tobject(const Tcomplex ztab[], const TDimPack& dp) : dimpack(dp) {
	k = KComplexArray;
	//a.zptr = new Tcomplex [dp.length()];
	a.xptr = new Treal [2*dp.length()];
	VECTORIZED for (int i=0; i<dp.length(); i++) a.zptr[i] = ztab[i];
	countit();
}

Tobject::Tobject(const TObjectPtr otab[], const TDimPack& dp) : dimpack(dp) {
	k = KObjectArray;
	a.opptr = new TObjectPtr [dp.length()];
	VECTORIZED for (int i=0; i<dp.length(); i++) a.opptr[i] = otab[i];
	countit();
}

istream& operator>>(istream& i, TObjectPtr& ptr) {
	cerr<<"***This should be never called\n";
	ptr = 0;
	return i;
}

INLINE int Tobject::IntArrayAllNonzero() const {	// Object MUST be IntArray!
	Tint *iptr = IntPtr();
	if (length()==0) return 0;
	VECTORIZED for (int i=0; i<length(); i++) if (iptr[i]==0) return 0;
	global::nops+= length();
	return 1;
}

int Tobject::IsNonzero() const {
	// Tests if object is nonzero integer, or IntArray with all elements nonzero
	// If object is neither integer nor IntArray, returns 0.
	int ret;
	switch (kind()) {
	case Kint:
		ret = IntValue()!=0;
		break;
	case KIntArray:
		ret = IntArrayAllNonzero();
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}

void Tobject::izeros(const TDimPack& dp) {
	setsize(dp.length()*sizeof(Tint));
	k = KIntArray;
	a.strflag = 0;
#	if USE_MEMCPY
	memset(IntPtr(),0,sizeof(Tint)*dp.length());
#	else
	VECTORIZED for (int i=0; i<dp.length(); i++) a.iptr[i] = 0;
#	endif
	dimpack = dp;
	global::nops+= dp.length();
}

void Tobject::rzeros(const TDimPack& dp) {
	setsize(dp.length()*sizeof(Treal));
	k = KRealArray;
#	if USE_MEMCPY
	memset(RealPtr(),0,sizeof(Treal)*dp.length());
#	else
	VECTORIZED for (int i=0; i<dp.length(); i++) a.xptr[i] = 0;
#	endif
	dimpack = dp;
	global::nops+= dp.length();
}

void Tobject::czeros(const TDimPack& dp) {
	setsize(dp.length()*sizeof(Tcomplex));
	k = KComplexArray;
#	if USE_MEMCPY
	memset(ComplexPtr(),0,sizeof(Tcomplex)*dp.length());
#	else
	VECTORIZED for (int i=0; i<dp.length(); i++) a.zptr[i] = 0;
#	endif
	dimpack = dp;
	global::nops+= dp.length();
}

void Tobject::ozeros(int N) {
	setsize(N*sizeof(TObjectPtr));
	k = KObjectArray;
#	if USE_MEMCPY
	memset(ObjectPtrPtr(),0,sizeof(TObjectPtr)*N);
#	else
	VECTORIZED for (int i=0; i<N; i++) a.opptr[i] = 0;
#	endif
	dimpack = N;
	global::nops+= N;
}

void Tobject::ozeros_shallow(int N) {
	setsize(N*sizeof(TObjectPtr));
	k = KShallowObjectArray;
#	if USE_MEMCPY
	memset(ObjectPtrPtr(),0,sizeof(TObjectPtr)*N);
#	else
	VECTORIZED for (int i=0; i<N; i++) a.opptr[i] = 0;
#	endif
	dimpack = N;
	global::nops+= N;
}

void Tobject::voids(const TDimPack& dp) {
	const Tint N = dp.length();
	setsize(N*sizeof(TObjectPtr));
	k = KObjectArray;
	for (int i=0; i<N; i++) {TObjectPtr p = new Tobject; p->SetToVoid(); a.opptr[i] = p;}
	dimpack = dp;
	global::nops+= N;
}

void Tobject::ireserv(const TDimPack& dp) {
	setsize(dp.length()*sizeof(Tint));
	k = KIntArray;
	a.strflag = 0;
	dimpack = dp;
}

void Tobject::rreserv(const TDimPack& dp) {
	setsize(dp.length()*sizeof(Treal));
	k = KRealArray;
	dimpack = dp;
}

void Tobject::creserv(const TDimPack& dp) {
	setsize(dp.length()*sizeof(Tcomplex));
	k = KComplexArray;
	dimpack = dp;
}

int Tobject::operator==(const Tobject& obj) const {
	if (this == &obj) return 1;
	if (kind()!=obj.kind()) return 0;
	switch (kind()) {
	case Kint:
		return IntValue() == obj.IntValue();
	case Kreal:
		return RealValue() == obj.RealValue();
	case Kcomplex:
		return ComplexValue() == obj.ComplexValue();
	case KIntArray:
		if (dims()!=obj.dims()) return 0;
		if (a.strflag != obj.a.strflag) return 0;
		return !memcmp(IntPtr(),obj.IntPtr(),sizeof(Tint)*length());
	case KRealArray:
		if (dims()!=obj.dims()) return 0;
		return !memcmp(RealPtr(),obj.RealPtr(),sizeof(Treal)*length());
	case KComplexArray:
		if (dims()!=obj.dims()) return 0;
		return !memcmp(ComplexPtr(),obj.ComplexPtr(),sizeof(Treal)*length());
	case KObjectArray:
	case KShallowObjectArray:
		if (dims()!=obj.dims()) return 0;
		{
			TObjectPtr *p1=ObjectPtrPtr(), *p2=obj.ObjectPtrPtr();
			for (int i=0; i<length(); i++)
				if (!(*p1[i] == *p2[i])) return 0;
			return 1;
		}
	case Kfunction:
		return FunctionValue() == obj.FunctionValue();
	case KCfunction:
		return CFunctionPtr() == obj.CFunctionPtr();
	case KIntrinsicFunction:
		return IntrinsicCompilerPtr() == obj.IntrinsicCompilerPtr();
	case Kvoid:
	case Kundef:
		return 1;
	}
	return 0;	// this is unnecessary but will prevent g++ from yelling
}

#define COMPTYPE Tint
#define DEFAULT_COMPVALUE 0
#define LINEARLIST TIntLL
#  include "templ/tLL.C"
#undef LINEARLIST
#undef DEFAULT_COMPVALUE
#undef COMPTYPE

#define COMPTYPE TObjectPtr
#define DEFAULT_COMPVALUE 0
#define LINEARLIST TObjectLL
#  include "templ/tLL.C"
#undef LINEARLIST
#undef DEFAULT_COMPVALUE
#undef COMPTYPE


#if 0

  For some reason Cray compiler cannot inline this.
  Therefore we wrote the #define macro below.
  Do not delete this C++ version since it contains the
  original comments!

INLINE void clear_inline(Tobject& o)
// Object 'o' corresponds to 'this'
{
	// Unallocate object. After operation, object has VOID value.
	// If object is Kfunction or KCfunction, does not delete the function definitions.
	// Optimized so that is fast for non-array objects (because for arrays it is slow anyway)
	// Otherwise the IfArray if statement could be left out.
	if (o.IsArray())
		switch (o.k) {
		case KIntArray:
			delete [] o.a.iptr;
			break;
		case KRealArray:
			delete [] o.a.xptr;
			break;
		case KComplexArray:
			//delete [] o.a.zptr;
			delete [] o.a.xptr;
			break;
		case KObjectArray:
		    {int j; for (j=0; j<(o).dimpack.length(); j++) delete (o).a.opptr[j];}  /* deep clear */
			delete [] o.a.opptr;
			break;
		case KShallowObjectArray:
			delete [] o.a.opptr;
		case Kint:
		case Kreal:
		case Kcomplex:
		case Kfunction:
		case KCfunction:
		case KIntrinsicFunction:
		case Kvoid:
		case Kundef:
			break;
		}
	o.k = Kvoid;
}

#endif

#define clear_inline(o)		/* o must be of type Tobject, e.g. *this */\
{\
	if ((o).IsArray())\
		switch ((o).k) {\
		case KIntArray:\
			delete [] (o).a.iptr;\
			break;\
		case KRealArray:\
			delete [] (o).a.xptr;\
			break;\
		case KComplexArray:\
			delete [] (o).a.xptr;\
			break;\
		case KObjectArray:\
            {int j; for (j=0; j<(o).dimpack.length(); j++) delete (o).a.opptr[j];}  /* deep clear */ \
			delete [] (o).a.opptr;\
            break;\
		case KShallowObjectArray:\
			delete [] (o).a.opptr;\
		case Kint:\
		case Kreal:\
		case Kcomplex:\
		case Kfunction:\
		case KCfunction:\
		case KIntrinsicFunction:\
		case Kvoid:\
		case Kundef:\
			break;\
		}\
	(o).k = Kvoid;\
}

void Tobject::clear() {clear_inline(*this);}

Tobject& Tobject::operator=(const Tobject& obj) {
	clear_inline(*this);
	init_inline(*this,obj);
	return *this;
}

void Tobject::setsize(TPtrInt sz) {
	// After calling setsize(sz), the object pointer field points to a memory area
	// of sz bytes in size. The object kind is undefined and must be set afterwards.
	// All pointer fields of the Tobject must share the same memory location, as
	// they currently do.
	switch (kind()) {
	case KIntArray:
		if (TPtrInt(length()*sizeof(Tint)) == sz) return;
		delete [] a.iptr;
		break;
	case KRealArray:
		if (TPtrInt(length()*sizeof(Treal)) == sz) return;
		delete [] a.xptr;
		break;
	case KComplexArray:
		if (TPtrInt(length()*sizeof(Tcomplex)) == sz) return;
		//delete [] a.zptr;
		delete [] a.xptr;
		break;
	case KObjectArray:
	    {int j; for (j=0; j<dimpack.length(); j++) delete a.opptr[j];}  /* deep clear */
		delete [] a.opptr;
	default:
		break;
	}
	a.iptr = new Tint [sz/sizeof(Tint)];
	// We must use the smallest type of (Tint,Treal,Tcomplex) here.
	// We assume that on any system, Treal or Tcomplex is not smaller than Tint.
	// We also assume that the resulting pointer from 'new Tint[]' is properly
	// aligned to be used even with Tcomplex.
	// If any of these assumptions is invalid, the program will probably crash.
	// In such a case, a separate version of setsize should be defined for
	// Int,Real and Complex.
}

static void OutputComplexNumber(ostream& o, const Tcomplex& z)
{
#	if USE_STRINGSTREAM
	ostringstream oss;
#	else
	ostrstream oss;
#	endif
	oss << real(z);
	if (imag(z)!=0.0) oss << (imag(z)<0 ? '-' : '+') << fabs(imag(z)) << 'i';
#	if USE_STRINGSTREAM
	string ptr = oss.str();
#	else
	oss << ends;			// this must not be inside USE_STRINGSTREAM branch (fixed 31 Jan 2005)
	char *ptr = oss.str();
#	endif
	o << ptr;
#	if !USE_STRINGSTREAM
	delete [] ptr;
#	endif
}

static void OutputComplexArray(ostream& o, const Tcomplex Z[], const TDimPack& dims, int width)
// Auxiliary function for operator<<(Tobject&). Int and real arrays are transformed
// to complex arrays before outputting, this reduces the amount of code and will produce
// equivalent results.
{
	if (dims.rank() == 1) {
		o << "#(";
		for (Tint i=0; i<dims.length(); i++) {
			OutputComplexNumber(o,Z[i]);
			if (i<dims.length()-1) o << ", ";
		}
		o << ')';
	} else if (dims.rank() == 2) {
		std::ios::fmtflags oldflags = o.flags();
		int oldwidth = o.width();
		o.setf(ios::right);
		o << "#(";
		for (Tint i=0; i<dims[0]; i++) {
			for (Tint j=0; j<dims[1]; j++) {
				o.width(width);
				OutputComplexNumber(o,Z[i*dims[1]+j]);
				if (j<dims[1]-1) o << ",";
			}
			if (i<dims[0]-1) o << ";\n  "; else o << ')';
		}
		o.width(oldwidth);
		o.flags(oldflags);
	} else {
	    int d;
		for (d=0; d<dims.rank(); d++) o << "#(";
		Tint modulus[MAXRANK];
		modulus[dims.rank()-1] = dims[dims.rank()-1];
		for (d=dims.rank()-2; d>=0; d--)
			modulus[d] = modulus[d+1]*dims[d];
		for (Tint i=0; i<dims.length(); i++) {
			OutputComplexNumber(o,Z[i]);
			int numseparators=0;
			for (d=0; d<dims.rank(); d++) if (((i+1) % modulus[d]) == 0) numseparators++;
			for (d=0; d<numseparators; d++) o << ')';
			if (i<dims.length()-1) {
				o << "; ";
				for (d=0; d<numseparators; d++) o << "#(";
			}
		}
	}
}

static void OutputObjectArray(ostream& o, const TObjectPtr OP[], const TDimPack& dims, int width)
// Auxiliary function for operator<<(Tobject&).
{
    int d;
	for (d=0; d<dims.rank(); d++) o << "#{";
	Tint modulus[MAXRANK];
	modulus[dims.rank()-1] = dims[dims.rank()-1];
	for (d=dims.rank()-2; d>=0; d--)
		modulus[d] = modulus[d+1]*dims[d];
	for (Tint i=0; i<dims.length(); i++) {
		o << *OP[i];
		int numseparators=0;
		for (d=0; d<dims.rank(); d++) if (((i+1) % modulus[d]) == 0) numseparators++;
		for (d=0; d<numseparators; d++) o << '}';
		if (i<dims.length()-1) {
			o << "; ";
			for (d=0; d<numseparators; d++) o << "#{";
		}
	}
}

extern void Add(Tobject& c, const Tobject& a, const Tobject& b);	// from objarithm.H

static void OutputChar(ostream& o, Tchar ch) {
	// See lexer source file "d.l", function rm_escape
	switch (ch) {
	case '\n':  o << "\\n"; break;
	case '\t':  o << "\\t"; break;
	case '\f':  o << "\\f"; break;
	case '\b':  o << "\\b"; break;
	case '\r':  o << "\\r"; break;
	case '\07': o << "\\a"; break;
	case '\013':o << "\\v"; break;
	case '\\':  o << "\\\\"; break;
	case '\"':  o << "\\\""; break;
	default:    o << ch; break;
	}
}

ostream& operator<<(ostream& o, const Tobject& obj) {	// Outputting an object
	switch (obj.kind()) {
	case Kint:
		if (obj.integer.charflag) {
			o << "'";
			OutputChar(o,Tchar(obj.integer.i));
			o << "'";
		} else
			o << obj.integer.i;
		global::nops++;
		break;
	case Kreal:
		o << obj.x;
		global::nops++;
		break;
	case Kcomplex:
		o << real(obj.z) << (imag(obj.z)<0 ? '-' : '+') << fabs(imag(obj.z)) << 'i';
		global::nops++;
		break;
	case KIntArray:
		if (obj.IsString()) {
			o << "\"";
			for (int i=0; i<obj.length(); i++)
				OutputChar(o,Tchar(obj.a.iptr[i]));
			o << "\"";
		} else if (obj.rank()==2 && obj.HasStringFlag()) {
			o << "#(\"";
			const Tint imax=obj.dims()[0], jmax=obj.dims()[1];
			for (Tint i=0; i<imax; i++) {
				for (Tint j=0; j<jmax; j++)
					OutputChar(o,Tchar(obj.a.iptr[i*jmax+j]));
				if (i<imax-1) o << "\"; \"";
			}
			o << "\")";
		} else {
			Tobject Z;
			Add(Z,obj,Tobject(0.0,0.0));
			OutputComplexArray(o,Z.ComplexPtr(),Z.dims(),5);
		}
		global::nops+= obj.length();
		break;
	case KRealArray:
		{
			Tobject Z;
			Add(Z,obj,Tobject(0.0,0.0));
			OutputComplexArray(o,Z.ComplexPtr(),Z.dims(),4+o.precision());
			global::nops+= obj.length();
		}
		break;
	case KComplexArray:
		{
			OutputComplexArray(o,obj.ComplexPtr(),obj.dims(),9+2*o.precision());
			global::nops+= obj.length();
		}
		break;
	case KObjectArray:
	case KShallowObjectArray:
		{
			OutputObjectArray(o,obj.ObjectPtrPtr(),obj.dims(),9+2*o.precision());
			/*
			TObjectLL olist(obj.length(),obj.a.opptr);
			o << obj.dims() << olist;
			*/
			global::nops+= obj.length();
		}
		break;
	case Kfunction:
		o << "<Function>";
		break;
	case KCfunction:
		if (obj.CFunctionInfoPtr())
			o << (char*)obj.CFunctionInfoPtr()->Cfname;
		else
			o << "<C-tela function>";
		break;
	case KIntrinsicFunction:
		o << "<Intrinsic function>";
		break;
	case Kvoid:
		// Void value doesn't output anything
		break;
	case Kundef:
		o << "<Undefined>";
		break;
	}
	return o;
}

void Tobject::flatten() {
	if (IsArray()) dimpack = TDimPack(length());
}

void Tobject::copykind(const Tobject& obj) {
	// Copy obj's kind: copy kind and dimensions, if array object
	if (this == &obj) return;	// Important: obj.copykind(obj) does nothing
	if (k == obj.k)
		if (length() == obj.length())
			return;
	clear();
	k = obj.k;
	switch (k) {
	case Kint:
	case Kreal:
	case Kcomplex:
	case Kfunction:
	case KCfunction:
	case KIntrinsicFunction:
	case Kvoid:
	case Kundef:
		break;
	case KIntArray:
		//a.len = obj.a.len;
		a.iptr = new Tint [obj.length()];
		dimpack = obj.dimpack;
		a.strflag = obj.a.strflag;
		break;
	case KRealArray:
		//a.len = obj.a.len;
		a.xptr = new Treal [obj.length()];
		dimpack = obj.dimpack;
		break;
	case KComplexArray:
		//a.len = obj.a.len;
		//a.zptr = new Tcomplex [obj.length()];
		a.xptr = new Treal [2*obj.length()];
		dimpack = obj.dimpack;
		break;
	case KShallowObjectArray:
	case KObjectArray:
		//a.len = obj.a.len;
		a.opptr = new TObjectPtr [obj.length()];
		dimpack = obj.dimpack;
		break;
	}
}

inline void Tobject::copydims(const Tobject& obj) {
	dimpack = obj.dimpack;
}

void Tobject::copydimsIntArray(const Tobject& obj) {
	// obj must be some Array, after call 'this' becomes IntArray of the same dims as obj
	if (this == &obj) return;
	if (k == KIntArray)
		if (dims() == obj.dims() && a.strflag == obj.a.strflag) return;
	clear();
	k = KIntArray;
	//a.len = obj.length();
	a.iptr = new Tint [obj.length()];
	a.strflag = 0;
	copydims(obj);
}

void Tobject::copydimsRealArray(const Tobject& obj) {
	// see copydimsIntArray
	if (this == &obj) return;
	if (k == KRealArray)
		if (dims() == obj.dims()) return;
	clear();
	k = KRealArray;
	//a.len = obj.length();
	a.xptr = new Treal [obj.length()];
	copydims(obj);
}

void Tobject::copydimsComplexArray(const Tobject& obj) {
	// see copydimsIntArray
	if (this == &obj) return;
	if (k == KComplexArray)
		if (dims() == obj.dims()) return;
	clear();
	k = KComplexArray;
	//a.len = obj.length();
	//a.zptr = new Tcomplex [obj.length()];
	a.xptr = new Treal [2*obj.length()];
	copydims(obj);
}

//Tint Tobject::rank() const {return IsScalar() ? -1 : dimpack.rank();}

// Class Tshort friend outputter:

ostream& operator<<(ostream& o, const Tshort& sh) {
	const int shortlimit = 4;	// vectors of length <= shortlimit are outputted in full
	const Tobject& obj = *sh.ptr;
	Tkind k = obj.kind();
	if (obj.IsArray()) {
		if (obj.length() <= shortlimit && obj.rank() == 1) {
			o << obj;
			if (k==KIntArray) {
				if (!obj.IsString()) o << " (integer array)";
			} else if (k==KRealArray)
				o << " (real array)";
			else if (k==KComplexArray)
				o << " (complex array)";
			else
				o << " (object array)";
		} else {
			o << '<';
			if (k==KIntArray)
				o << (obj.IsString() ? "string" : "integer");
			else if (k==KRealArray)
				o << "real";
			else if (k==KComplexArray)
				o << "complex";
			else
				o << "object";
			o << obj.dims() << '>';
		}
	} else if (obj.kind() == Kvoid)
		o << "<Void>";
	else {
		o << obj;
		if (k==Kint) o << (obj.IsChar() ? " (char)" : " (integer)"); else if (k==Kreal) o << " (real)";
	}
	return o;
}
