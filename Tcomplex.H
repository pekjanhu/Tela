/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef TCOMPLEX_H

#ifdef __GNUC__
#  pragma interface
#endif

// Treal must be defined before including this

class Tcomplex {
private:
	Treal re;
	Treal im;
public:
	Treal real() const {return re;}
	Treal imag() const {return im;}

	Tcomplex() : re(0.0), im(0.0) {}
	Tcomplex(const Tcomplex& y) :re(y.real()), im(y.imag()) {}
	Tcomplex(Treal r, Treal i=0) :re(r), im(i) {}

	Tcomplex& operator=(const Tcomplex& y) {re = y.real(); im = y.imag(); return *this;}
	Tcomplex& operator+=(const Tcomplex& y) {re+= y.real(); im+= y.imag(); return *this;}
	Tcomplex& operator+=(Treal y) {re+= y; return *this;}
	Tcomplex& operator-=(const Tcomplex& y) {re-=y.real(); im-=y.imag(); return *this;}
	Tcomplex& operator-=(Treal y) {re-= y; return *this;}
	Tcomplex& operator*=(const Tcomplex& y)	{  
		Treal r = re*y.real() - im*y.imag();
		im = re*y.imag() + im*y.real(); 
		re = r; 
		return *this; 
	}

	Tcomplex& operator*=(Treal y) {re*= y; im*= y; return *this;}
	Tcomplex& operator/=(const Tcomplex& y); 
	Tcomplex& operator/=(Treal y) {re/= y; im/= y; return *this;}
};

inline int operator==(const Tcomplex& x, const Tcomplex& y) {
	return x.real() == y.real() && x.imag() == y.imag();
}

inline int operator==(const Tcomplex& x, Treal y) {
	return x.imag() == 0.0 && x.real() == y;
}

inline int operator!=(const Tcomplex& x, const Tcomplex& y) {
  return x.real() != y.real() || x.imag() != y.imag();
}

inline int operator!=(const Tcomplex& x, Treal y) {
	return x.imag() != 0.0 || x.real() != y;
}

inline Tcomplex operator-(const Tcomplex& x) {
	return Tcomplex(-x.real(), -x.imag());
}

inline Tcomplex conj(const Tcomplex& x) {
	return Tcomplex(x.real(), -x.imag());
}

inline Tcomplex operator+(const Tcomplex& x, const Tcomplex& y) {
	return Tcomplex(x.real() + y.real(), x.imag() + y.imag());
}

inline Tcomplex operator+(const Tcomplex& x, Treal y) {
	return Tcomplex(x.real() + y, x.imag());
}

inline Tcomplex operator+(Treal x, const Tcomplex& y) {
	return Tcomplex(x + y.real(), y.imag());
}

inline Tcomplex operator-(const Tcomplex& x, const Tcomplex& y) {
	return Tcomplex(x.real() - y.real(), x.imag() - y.imag());
}

inline Tcomplex operator-(const Tcomplex& x, Treal y) {
	return Tcomplex(x.real() - y, x.imag());
}

inline Tcomplex operator-(Treal x, const Tcomplex& y) {
	return Tcomplex(x - y.real(), -y.imag());
}

inline Tcomplex operator*(const Tcomplex& x, const Tcomplex& y) {
	return Tcomplex(x.real()*y.real() - x.imag()*y.imag(), 
					x.real()*y.imag() + x.imag()*y.real());
}

inline Tcomplex operator*(const Tcomplex& x, Treal y) {
	return Tcomplex(x.real()*y, x.imag()*y);
}

inline Tcomplex operator*(Treal x, const Tcomplex& y) {
	return Tcomplex(x*y.real(), x*y.imag());
}

inline Tcomplex operator/(const Tcomplex& x, const Tcomplex& y) {
	const Treal invden = 1.0/(fabs(y.real()) + fabs(y.imag()));
	const Treal xrden = x.real()*invden;
	const Treal xiden = x.imag()*invden;
	const Treal yrden = y.real()*invden;
	const Treal yiden = y.imag()*invden;
	const Treal invnrm = 1.0/(yrden*yrden + yiden*yiden);
	return Tcomplex((xrden*yrden + xiden*yiden)*invnrm,
					(xiden*yrden - xrden*yiden)*invnrm);
}

#if 0
inline Tcomplex operator/(const Tcomplex& x, const Tcomplex& y) {
	const Treal coeff = 1.0/(y.real()*y.real() + y.imag()*y.imag());
	return Tcomplex(
		(x.real()*y.real() + x.imag()*y.imag())*coeff,
		(x.imag()*y.real() - x.real()*y.imag())*coeff );
}
#endif

#if 0
inline TComplex operator/(const Treal x, const Tcomplex& y) {
	const Treal invden = 1.0/(fabs(y.real()) + fabs(y.imag()));
	const Treal xrden = x*invden;
	const Treal yrden = y.real()*invden;
	const Treal yiden = y.imag()*invden;
	const Treal coeff = xrden/(yrden*yrden + yiden*yiden);
	return Tcomplex(yrden*coeff,-yiden*coeff);
}
#endif

inline Tcomplex operator/(Treal x, const Tcomplex& y) {
	const Treal coeff = x/(y.real()*y.real() + y.imag()*y.imag());
	return Tcomplex(y.real()*coeff,-y.imag()*coeff);
}

inline Tcomplex operator/(const Tcomplex& x, Treal y) {
	const Treal invy = 1.0/y;
	return Tcomplex(x.real()*invy, x.imag()*invy);
}

inline Treal real(const Tcomplex& x) {return x.real();}

inline Treal imag(const Tcomplex& x) {return x.imag();}

inline Treal abs(const Tcomplex& x) {
	return hypot(x.real(), x.imag());
}

inline Treal arg(const Tcomplex& x) {
	return atan2(x.imag(), x.real());
}

inline Tcomplex exp(const Tcomplex& x) {
	const Treal r = exp(x.real());
	return Tcomplex(r*cos(x.imag()), 
					r*sin(x.imag()));
}

inline Tcomplex log(const Tcomplex& x) {
	Treal h = hypot(x.real(), x.imag());
	return Tcomplex(log(h), atan2(x.imag(), x.real()));
}

inline Tcomplex cosh(const Tcomplex& x) {
	return Tcomplex(cos(x.imag()) * cosh(x.real()), 
					sin(x.imag()) * sinh(x.real()));
}

inline Tcomplex sinh(const Tcomplex& x) {
	return Tcomplex(cos(x.imag()) * sinh(x.real()), 
					sin(x.imag()) * cosh(x.real()));
}

inline Tcomplex tanh(const Tcomplex& x) {
	// tanh(x) = (exp(x) - exp(-x))/(exp(x) + exp(-x))
	// = (1 - exp(-2*x))/(1 + exp(-2*x))
	const Tcomplex e = exp(-2*x);
	return (1-e)/(1+e);
}

inline Tcomplex cos(const Tcomplex& x) {
	return Tcomplex(cos(x.real()) * cosh(x.imag()), 
					-sin(x.real()) * sinh(x.imag()));
}

inline Tcomplex sin(const Tcomplex& x) {
	return Tcomplex(sin(x.real()) * cosh(x.imag()), 
					cos(x.real()) * sinh(x.imag()));
}

inline Tcomplex pow(const Tcomplex& x, Treal p) {
	const Treal h = hypot(x.real(), x.imag());
	const Treal lr = pow(h, p);
	const Treal a = atan2(x.imag(), x.real());
	const Treal li = p * a;
	return Tcomplex(lr*cos(li), lr*sin(li));
}

// Non-inline functions

Tcomplex pow(const Tcomplex& x, int p);
Tcomplex pow(const Tcomplex& x, const Tcomplex& p);
Tcomplex sqrt(const Tcomplex& x);
Tcomplex asin(const Tcomplex& x);
Tcomplex acos(const Tcomplex& x);
Tcomplex atan(const Tcomplex& x);

#define TCOMPLEX_H

#endif
