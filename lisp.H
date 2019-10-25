/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef LISP_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "tree.H"

inline int Atom(Tnode*x) {return !(x->list);}

Tnode* List(Tfunc h, Tnode*a);
Tnode* List(Tfunc h, Tnode*a, Tnode*b);
Tnode* List(Tfunc h, Tnode*a, Tnode*b, Tnode*c);

inline Tnode* List(Tnode*a) {return List(FLIST,a);}
inline Tnode* List(Tnode*a, Tnode*b) {return List(FLIST,a,b);}
inline Tnode* List(Tnode*a, Tnode*b, Tnode*c) {return List(FLIST,a,b,c);}

inline Tfunc Head(Tnode*L) {return L->FunctionValue();}
inline Tnode* First(Tnode*L) {return L->list;}
inline Tnode* Second(Tnode*L) {return L->list->next;}
inline Tnode* Third(Tnode*L) {return L->list->next->next;}
inline Tnode* Fourth(Tnode*L) {return L->list->next->next->next;}
inline Tnode* Fifth(Tnode*L) {return L->list->next->next->next->next;}

Tnode* Nth(Tnode*L, int N);

int Length(Tnode*L);

Tnode* CopyList(Tnode*L);

Tnode* Append(Tnode*L1, Tnode*L2);
Tnode* Append(Tnode*L1, Tnode*L2, Tnode*L3);

#define LISP_H

#endif
