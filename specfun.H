/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifndef SPECFUN_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "def.H"

#define default_accuracy 1e-10

template <class T> extern Tcomplex logfact(const T& x);
template <class T> extern Tcomplex IncompleteGammaP(const T& a, const T& x);
template <class T> extern Tcomplex IncompleteGammaQ(const T& a, const T& x);

extern Tcomplex Gauss2F1(const Tcomplex& a, const Tcomplex& b, const Tcomplex& c,
						 const Tcomplex& z,
						 Treal eps_accuracy=default_accuracy);

extern Tcomplex Kummer1F1(const Tcomplex& a, const Tcomplex& c,
						  const Tcomplex& z,
						  Treal eps_accuracy=default_accuracy);

extern Tcomplex Kummer1F1Regularized(const Tcomplex& a, const Tcomplex& c,
									 const Tcomplex& z,
									 Treal eps_accuracy=default_accuracy);

extern Tcomplex LegendreP(const Tcomplex& n, const Tcomplex& x, Treal eps_accuracy=default_accuracy);
extern Tcomplex LegendreP(const Tcomplex& n, const Tcomplex& m, const Tcomplex& x, Treal eps_accuracy=default_accuracy);

extern Tcomplex ChebyshevT(const Tcomplex& n, const Tcomplex& x, Treal eps_accuracy=default_accuracy);

#define SPECFUN_H 1

#endif
