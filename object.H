/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef OBJECT_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "def.H"
#include "error.H"
#include "common.H"
#include "ctinfo.H"

extern int TObjectNewCounter, TObjectDelCounter;

enum Tkind {				// Object kinds (types)
	Kint,					// Integer scalar
	Kreal,					// Real scalar
	Kcomplex,				// Complex scalar
	KIntArray,				// Integer array (n-dimensional)
	KRealArray,				// Real array
	KComplexArray,			// Complex array
	KObjectArray,			// Object (pointer) array (returned by function voids())
	KShallowObjectArray,	// Version of KObjectArray which is not recursively deleted, used only internally by Tprg::execute()
	Kfunction,				// User-defined function, written in Tela
	KCfunction,				// Compiled and linked C-Tela function
	KIntrinsicFunction,		// Special "functions" generating inline code: abs, min, max ..
	Kvoid,					// Empty value, when printed prints nothing
	Kundef					// Undefined value, the default for new symbols
};	// Kundef MUST remain the LAST here (otherwise pair() definition breaks down in objarithm.C)

class Tprg;
class Tnode;

typedef int (*TIntrinsicCompilerPtr)
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg);

#define COMPTYPE TObjectPtr
#define DEFAULT_COMPVALUE 0
#define LINEARLIST TObjectLL
#  include "templ/tLL.H"
#undef LINEARLIST
#undef DEFAULT_COMPVALUE
#undef COMPTYPE

class TDimPack {
 private:
	Tint len;				// len = the product  dims[0]*...*dims[Ndims-1]
	Tint dims[MAXRANK];		// The dimensions
	short Ndims;			// Number of dimensions, small positive integer
 public:
	// --- constructors
	TDimPack() : len(0), Ndims(0) {}
	TDimPack(Tint d1) : len(d1), Ndims(1) {dims[0]=d1;}
	TDimPack(Tint d1, Tint d2) : len(d1*d2), Ndims(2) {dims[0]=d1; dims[1]=d2;}
	TDimPack(const Tint ds[], int n);
	TDimPack(const TDimPack& dimpack);
	// --- assignment & comparison
	TDimPack& operator=(const TDimPack& dimpack);
	TDimPack& operator=(Tint N) {len=N; dims[0]=N; Ndims=1; return *this;}
	int operator==(const TDimPack& dimpack) const;
	int operator!=(const TDimPack& dimpack) const {return !(operator==)(dimpack);}
	int iscompatible(const TDimPack& dimpack) const;
	// --- indexing & inquiry functions
	Tint operator[](int d) const {return dims[d];}
	Tint length() const {return len;}
	int rank() const {return Ndims;}
	// --- I/O
	friend ostream& operator<<(ostream& o, const TDimPack& dimpack);
	friend istream& operator>>(istream& i, TDimPack& dimpack);
	// --- destructor
	~TDimPack() {}
};

class Tprg;
struct TCFunctionInfo;

class Tobject {
 private:
	union {
		struct {
			Tint i;				// scalar integer object's value
			int charflag;		// true if integer is to be displayed as a character
		} integer;
		Treal x;				// scalar real object's value
		Tprg* funcptr;			// pointer to function, if k==Kfunction
		struct {
			TCFunctionPtr ptr;			// pointer to C-function, if k==KCfunction
			TCFunctionInfo* infoptr;	// pointer back to C-functioninfo, where minin..maxout info is located
		} Cfunc;
		struct {
			TIntrinsicCompilerPtr ptr;		// pointer to intrisic 'compiler', if k==KIntrinsicFunction
			int code;						// opcode
		} intr;
		struct {
			union {
				Tint *iptr;				// pointer to Tint array, if k==KIntArray
				Treal *xptr;			// pointer to Treal array, if k==KRealArray
				Tcomplex *zptr;			// pointer to Tcomplex array, if k==KComplexArray
				TObjectPtr *opptr;		// pointer TobjectPtr array, if k==KObjectArray
			};
			int strflag;		// flag for string, meaningful if k==KIntArray
		} a;					// a as array-info
    };
	Tcomplex z;			// scalar complex object's value
	TDimPack dimpack;	// rank & dims of an array object
	Tkind k;			// tag for object's type
	int tempflag;		// flag for temporary object
	void clear();		// unallocate if an array object. After operation, object is VOID.
        void clear_fast() {if /*(IsArray())*/(k>=KIntArray && k<=KObjectArray) clear();}		// same as clear() but after operation, object is in undefined state.
	// Use clear_fast() if you assign k immediately afterwards. clear_fast() is completely inline,
	// unless the object is an array.
	void setsize(TPtrInt sz);	// make object an array of sz bytes. After operation object is undefined.
	void copydims(const Tobject& obj);
	void copykind(const Tobject& obj);
	int IntArrayAllNonzero() const;
	// Private 'init' functions, used in both constructors and assignment ops
	void init(const Tobject& obj);		// initialization with another object
	void init(const Tchar* str);		// initialization with a Tchar-string
	friend inline void init_inline(Tobject&,const Tobject&);
	//friend inline void clear_inline(Tobject&);
	// Private inline function that increments a global counter.
	// To remove object counting, just remove it from here and from the destructor.
	// Don't remove the tempflag assignment.
	void countit() {/*global::NTobject++;*/ tempflag=0;}
 public:
	// --- these were first private but are now public, thereyougo!
	void copydimsIntArray(const Tobject& obj);
	void copydimsRealArray(const Tobject& obj);
	void copydimsComplexArray(const Tobject& obj);

	// --- constructors
	Tobject() : k(Kundef) {countit();}
	Tobject(Tint i1) : k(Kint) {integer.i=i1; integer.charflag=0; countit();}
	Tobject(Tchar ch) : k(Kint) {integer.i=ch; integer.charflag=1; countit();}
	Tobject(Treal x1) : k(Kreal) {x=x1; countit();}
	Tobject(Tcomplex z1) : k(Kcomplex) {z=z1; countit();}
	Tobject(Tprg* ptr) : k(Kfunction) {funcptr=ptr; countit();}
	Tobject(TCFunctionPtr ptr) : k(KCfunction) {Cfunc.ptr=ptr; countit();}
	Tobject(TIntrinsicCompilerPtr ptr) : k(KIntrinsicFunction) {intr.ptr=ptr; countit();}
	Tobject(Treal x, Treal y);
	Tobject(const Tint itab[], int N, int stringflag=0);
	Tobject(const Treal xtab[], int N);
	Tobject(const Tcomplex ztab[], int N);
	Tobject(const TObjectPtr otab[], int N);
	Tobject(const Tint itab[], const TDimPack& dp);
	Tobject(const Treal xtab[], const TDimPack& dp);
	Tobject(const Tcomplex ztab[], const TDimPack& dp);
	Tobject(const TObjectPtr otab[], const TDimPack& dp);
	Tobject(const Tchar *str) {init(str); countit();}
#	ifdef TCHAR_NOTEQUAL_CHAR
	Tobject(const char *str) {init((const Tchar*)str); countit();}
#	endif
	Tobject(const Tobject& obj) {init(obj); countit();}

	// --- assignments
	Tobject& operator=(Tint i1) {clear_fast(); k=Kint; integer.i=i1; integer.charflag=0; return *this;}
	Tobject& operator=(Treal x1) {clear_fast(); k=Kreal; x=x1; return *this;}
	Tobject& operator=(Tprg* ptr) {clear_fast(); k=Kfunction; funcptr=ptr; return *this;}
	Tobject& operator=(TCFunctionPtr ptr) {clear_fast(); k=KCfunction; Cfunc.ptr=ptr; return *this;}
	Tobject& operator=(TIntrinsicCompilerPtr ptr) {clear_fast(); k=KIntrinsicFunction; intr.ptr=ptr; return *this;}
	Tobject& operator=(const Tcomplex& z1) {clear_fast(); k=Kcomplex; z=z1; return *this;}
	Tobject& operator=(Tchar ch) {clear_fast(); k=Kint; integer.i=Tint(ch); integer.charflag=1; return *this;}
	Tobject& operator=(const Tchar *str) {clear_fast(); init(str); return *this;}
#	ifdef TCHAR_NOTEQUAL_CHAR
	Tobject& operator=(const char *str) {clear_fast(); init((const Tchar*)str); return *this;}
#	endif
	Tobject& operator=(const Tobject& obj);
	void izeros(const TDimPack& dp);	// set to zeroed int array of given dims
	void rzeros(const TDimPack& dp);	// set to zeroed real array of given dims
	void czeros(const TDimPack& dp);	// set to zeroed complex array of given dims
	void ozeros(int N);					// set to nulled object vector of given length
	void ozeros_shallow(int N);			// set to nulled object vector of given length, using KShallowObjectArray
	void voids(const TDimPack& dp);		// set to array of objects, each with VOID value
	void ireserv(const TDimPack& dp);	// set to uninitialized int array of given dims
	void rreserv(const TDimPack& dp);	// set to uninitialized real array of given dims
	void creserv(const TDimPack& dp);	// set to uninitialized complex array of given dims
	void SetToVoid() {clear_fast(); k=Kvoid;}// set to void value
	void SetToUndefined() {clear_fast(); k=Kundef;}	// set to undefined value
	void SetStringFlag() {a.strflag=1;}	// set string flag (assuming it is IntArray already)
    void SetCharFlag() {integer.charflag=1;} // assume it is Kint already
	void ClearStringFlag() {a.strflag=0;}	// unset string flag (assuming it is IntArray already)
	void ClearCharFlag() {integer.charflag=0;}	// assume it is Kint already

	// --- comparison
	int operator==(const Tobject& obj) const;
	int operator!=(const Tobject& obj) const {return ! (operator==(obj));}

	// --- inquiry functions
	Tkind kind() const {return k;}
	int length() const {return dimpack.length();}
	Tint rank() const {return dimpack.rank();}
	Tint IntValue() const {return integer.i;}
	Treal RealValue() const {return x;}
	const Tcomplex& ComplexValue() const {return z;}
	Tint& IntRef() {return integer.i;}
	Treal& RealRef() {return x;}
	Tcomplex& ComplexRef() {return z;}
	Tprg* FunctionValue() const {return funcptr;}
	TCFunctionPtr& CFunctionRef() {return Cfunc.ptr;}
	TCFunctionPtr CFunctionPtr() const {return Cfunc.ptr;}
	const TCFunctionInfo* CFunctionInfoPtr() const {return Cfunc.infoptr;}
	TCFunctionInfo*& CFunctionInfoPtrRef() {return Cfunc.infoptr;}
	TIntrinsicCompilerPtr IntrinsicCompilerPtr() const {return intr.ptr;}
	TIntrinsicCompilerPtr& IntrinsicCompilerPtrRef() {return intr.ptr;}
	int IntrinsicCode() const {return intr.code;}
	int& IntrinsicCodeRef() {return intr.code;}
	Tint *IntPtr() const {return a.iptr;}
	Treal *RealPtr() const {return a.xptr;}
	Tcomplex *ComplexPtr() const {return a.zptr;}
	TObjectPtr *ObjectPtrPtr() const {return a.opptr;}
	int IsNumerical() const {return /*k>=Kint && */ k<=KComplexArray;}		// notice k>=Kint is always true
	int IsArray() const {return (k>=KIntArray && k<=KObjectArray);}
	int IsNumericalArray() const {return (k>=KIntArray && k<=KComplexArray);}
	int IsScalar() const {return (k<=Kcomplex);}
	int IsFunction() const {return k==Kfunction;}
	int IsCfunction() const {return k==KCfunction;}
	int IsIntrinsicFunction() const {return k==KIntrinsicFunction;}
	int IsString() const {return k==KIntArray && dimpack.rank()==1 && a.strflag;}
	int HasStringFlag() const {return k==KIntArray && a.strflag;}
	int IsChar() const {return k==Kint && integer.charflag;}
	int IsNonzero() const;
	int IsTemporary() const {return tempflag;}
	void FlagTemporary() {tempflag=1;}
	void UnflagTemporary() {tempflag=0;}
	const TDimPack& dims() const {return dimpack;}
	int iscompatible(const Tobject& obj) const {return dimpack.iscompatible(obj.dimpack);}

	// --- other operations
	friend ostream& operator<<(ostream& o, const Tobject& obj);
	void flatten();
	void bitwiseMoveFrom(Tobject& obj);
	void SetNewDimPack(const TDimPack& dp) {dimpack=dp;}		// DANGEROUS!!!!! Used by reshape only.

	// --- destructor
	~Tobject() {clear_fast();/* global::NTobject--;*/}
}
//#ifdef __GNUC__
//__attribute__((aligned(sizeof(Treal))))
//#endif
;

#if MAXRANK % 2 == 1
#ifdef __GNUC__
#  warn For g++/gcc, using odd MAXRANK in def.H may cause trouble (probably compiler bug)
#endif
#endif

// With bitwiseMoveFrom you can 'copy' an object quickly without
// moving the data. See usage examples in templ/tbinop.C.

inline void Tobject::bitwiseMoveFrom(Tobject& obj) {
	clear_fast();
	memcpy(this,&obj,sizeof(Tobject));
	obj.k = Kvoid;		// do not call SetToVoid() here !
}

// Class Tshort is a wrapper class, when outputted, it prints in short form

class Tshort {
 private:
	const Tobject* ptr;
//        Tshort(const Tshort&);            // do not implement		// COMMENTED OUT 29 Sep 2004, g++3.4.0 does not accept it!
//        Tshort& operator=(const Tshort&); // do not implement		// SAME
 public:
	Tshort(const Tobject& obj) : ptr(&obj) {}
	friend ostream& operator<<(ostream& o, const Tshort& sh);
};

#define OBJECT_H

#endif
