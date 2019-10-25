/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef OBJARITHM_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "object.H"

void Add(Tobject& c, const Tobject& a, const Tobject& b);
void Inc(Tobject& a);
void Sub(Tobject& c, const Tobject& a, const Tobject& b);
void Dec(Tobject& a);
void Mul(Tobject& c, const Tobject& a, const Tobject& b);
void Div(Tobject& c, const Tobject& a, const Tobject& b);
void Pow(Tobject& c, const Tobject& a, const Tobject& b);
void Mod(Tobject& c, const Tobject& a, const Tobject& b);
void Neg(Tobject& c, const Tobject& a);
void Abs(Tobject& c, const Tobject& a);
void Mgatscat(Tobject& a, Tobject& b, const Tobject& index);
void Gt(Tobject& c, const Tobject& a, const Tobject& b);
void Lt(Tobject& c, const Tobject& a, const Tobject& b);
void Ge(Tobject& c, const Tobject& a, const Tobject& b);
void Le(Tobject& c, const Tobject& a, const Tobject& b);
void Eq(Tobject& c, const Tobject& a, const Tobject& b);
void Ne(Tobject& c, const Tobject& a, const Tobject& b);
int TestGt(const Tobject& a, const Tobject& b);
int TestLt(const Tobject& a, const Tobject& b);
int TestGe(const Tobject& a, const Tobject& b);
int TestLe(const Tobject& a, const Tobject& b);
void And(Tobject& c, const Tobject& a, const Tobject& b);
void Or(Tobject& c, const Tobject& a, const Tobject& b);
void Not(Tobject& c, const Tobject& a);
void Min(Tobject& c, const Tobject& a, const Tobject& b);
void Max(Tobject& c, const Tobject& a, const Tobject& b);
void Min(Tobject& c, const Tobject& a);
void Max(Tobject& c, const Tobject& a);

Tint MinMaxPosition();

extern istream& operator>>(istream& i, TObjectPtr& ptr);

inline void Add(Tobject& c, const Tobject& a) {Add(c,c,a);}
inline void Sub(Tobject& c, const Tobject& a) {Sub(c,c,a);}
inline void Mul(Tobject& c, const Tobject& a) {Mul(c,c,a);}
inline void Div(Tobject& c, const Tobject& a) {Div(c,c,a);}
inline void Pow(Tobject& c, const Tobject& a) {Pow(c,c,a);}

inline void Neg(Tobject& x) {Neg(x,x);}
inline void Abs(Tobject& x) {Abs(x,x);}

// --- Transcendental function stuff

typedef Treal (*TRealScalarFunction) (Treal);
// TComplexScalarFunction is typedefed in def.H.

typedef void (*TRealVectorFunction) (Treal[],int N);
typedef void (*TComplexVectorFunction) (Tcomplex[],int N);

struct TTranscenFunctionPack {
	const char *name;
	TRealScalarFunction f;
	TComplexScalarFunction cf;
	TRealVectorFunction vf;
	TComplexVectorFunction cvf;
	// We need this constructor also for AT&T 2.1 translator
	TTranscenFunctionPack(const char* name1,
			      TRealScalarFunction f1,
			      TComplexScalarFunction cf1,
			      TRealVectorFunction vf1,
			      TComplexVectorFunction cvf1)
	{
		name=name1; f=f1; cf=cf1; vf=vf1; cvf=cvf1;
	}
};

extern void Transcen(const TTranscenFunctionPack& funcs, Tobject& c, const Tobject& a);

#define OBJARITHM_H

#endif
