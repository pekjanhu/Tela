/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "Tcomplex.H"
#endif
#include "def.H"
#include "Tcomplex.H"

Tcomplex& Tcomplex::operator/=(const Tcomplex& y)
{
	// Treat special cases
	if (y.imag() == 0.0) {
		re /= y.real();
		im /= y.real();
		return *this;
	}
	if (y.real() == 0.0) {
		re /= y.imag();
		im /= -y.imag();
		return *this;
	}
        // Generic division algorithm
	if (fabs(y.real()) >= fabs(y.imag())) {
		const Treal r = y.imag()/y.real();
		const Treal d = y.real() + r*y.imag();
		const Treal re_tmp = (re + im*r)/d;
		im = (im - re*r)/d;
		re = re_tmp;
	} else {
		const Treal r = y.real()/y.imag();
		const Treal d = y.imag() + r*y.real();
		const Treal re_tmp = (re*r + im)/d;
		im = (im*r - re)/d;
		re = re_tmp;
	}
	return *this;
}

Tcomplex pow(const Tcomplex& x, const Tcomplex& p) {
	const Treal h = hypot(x.real(), x.imag());
	const Treal a = atan2(x.imag(), x.real());
	Treal lr = pow(h, p.real());
	Treal li = p.real()*a;
	if (p.imag() != 0.0) {
		lr/= exp(p.imag() * a);
		li+= p.imag() * log(h);
	}
	return Tcomplex(lr*cos(li), lr*sin(li));
}

Tcomplex pow(const Tcomplex& x, int p)
{
	if (p == 0)
		return Tcomplex(1.0, 0.0);
	else if (x == 0.0)
		return Tcomplex(0.0, 0.0);
	else if (abs(p) > 20)
		return pow(x,Treal(p));
	else {
		Tcomplex res(1.0, 0.0);
		Tcomplex b = x;
		if (p < 0) {
			p = -p;
			b = 1.0 / b;
		}
		for(;;) {
			if (p & 1)
				res *= b;
			if ((p >>= 1) == 0)
				return res;
			else
				b *= b;
		}
		return res;
	}
}

Tcomplex sqrt(const Tcomplex& x) {
	if (x.real() == 0.0 && x.imag() == 0.0)
		return Tcomplex(0.0, 0.0);
	else {
		const Treal s = sqrt((fabs(x.real()) + hypot(x.real(), x.imag()))*0.5);
		const Treal d = (x.imag()/s) * 0.5;
		if (x.real() > 0.0)
			return Tcomplex(s, d);
		else if (x.imag() >= 0.0)
			return Tcomplex(d, s);
		else
			return Tcomplex(-d, -s);
	}
}
 
Tcomplex asin(const Tcomplex& z) {
	const Tcomplex I(0,1);
	return -I*log(sqrt(1-z*z)+I*z);
}

Tcomplex acos(const Tcomplex& z) {
	const Tcomplex I(0,1);
	return -I*log(z+I*sqrt(1-z*z));
}

Tcomplex atan(const Tcomplex& z) {
	const Tcomplex I(0,1);
	return 0.5*I*log((1-I*z)/(1+I*z));
}


