#ifdef __GNUC__
#  pragma implementation "image.H"
#endif

#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
using namespace std;
#include "image.H"
#include "def.H"
#if USE_STRINGSTREAM
#  include <sstream>
#else
#  include <strstream.h>
#endif

void Timage::from(int W, int H)
{
	w = W;
	h = H;
	const int n = w*h;
//	ptr = new unsigned int [n];
	ptr = new TImagePixel [n];
}

void Timage::from(const Timage& X)
{
	w = X.w;
	h = X.h;
	if (X.ptr) {
		const int n = w*h;
//		ptr = new unsigned int [n];
		ptr = new TImagePixel [n];
//		memcpy(ptr,X.ptr,sizeof(unsigned int)*n);
		memcpy(ptr,X.ptr,sizeof(TImagePixel)*n);
	} else {
		ptr = 0;
	}
}

void Timage::xv() const
{
	if (!ptr) return;
	char fn[FILENAME_MAX];
	tmpnam(fn);
	ofstream o(fn);
	cout << "Doing xv with h=" << h << ", w=" << w << "\n";
	o << "P6\n" << "# Created by image.C\n" << w << " " << h << " 255\n";
	int i,j;
	for (i=0; i<h; i++) for (j=0; j<w; j++) {
		const int p = flatindex(i,j);
		o.put(red(i,j));
		o.put(green(i,j));
		o.put(blue(i,j));
	}
	o.close();
#	if USE_STRINGSTREAM
	stringstream s;
#	else
	strstream s;
#	endif
	s << "(xv " << fn << "; rm -f " << fn << ")& " << ends;
#	if USE_STRINGSTREAM
	string charptr = s.str();
	system(charptr.c_str());
#	else
	char *charptr = s.str();
	system(charptr);
	delete [] charptr;
#	endif
}

void Timage::free()
{
	if (ptr) {
		delete [] ptr;
	}
	w = h = 0;
	ptr = 0;
}


