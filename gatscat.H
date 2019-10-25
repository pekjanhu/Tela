/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma interface
#endif
#include "object.H"

extern void Gather(Tobject& a, const Tobject& b, const TConstObjectPtr indices[], int D);
extern void Scatter(const Tobject& a, Tobject& b, const TConstObjectPtr indices[], int D);

extern void MGatScat(Tobject& a, Tobject& b, const TObjectPtr indices[], int D, int scatflag);

extern void PasteArrays(Tobject& a, const TConstObjectPtr args[], int N);
extern void BuildArray(Tobject& a, const TConstObjectPtr args[], int N);

extern void Range(TObjectPtr d[4]);
