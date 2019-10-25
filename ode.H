/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifndef ODE_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "def.H"

extern void bsstep(Treal y[], Treal dydx[], int nv, Treal& xx, Treal htry, Treal eps,
				   Treal yscal[], Treal &hdid, Treal &hnext,
				   void (*derivs)(Treal, Treal[], Treal[]));

extern void odeint(Treal ystart[], int nvar, Treal x1, Treal x2, Treal eps, Treal h1, Treal hmin,
				   int& nok, int& nbad,
				   void (*derivs)(Treal, Treal[], Treal[]),
				   void (*rkqc)(Treal[],Treal[],int,Treal&,Treal,Treal,
								Treal[],Treal&,Treal&,void (*)(Treal, Treal[], Treal[])));

#define ODE_H 1

#endif
