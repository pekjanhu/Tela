/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef CTINFO_H

// -------------- types and functions to install C-functions ---------------

class Tobject;

typedef Tobject *TObjectPtr;
typedef const Tobject *TConstObjectPtr;

typedef int (*TCFunctionPtr)(const TConstObjectPtr in[], const int Nargin, const TObjectPtr out[], const int Nargout);

struct TCFunctionInfo {
	unsigned char *Cfname;
	unsigned char *helpfile;
	TCFunctionPtr Cfptr;
	int minin;
	int minout;
	int maxin;
	int maxout;
	long helpstart;
	long helpend;
};

extern void Install(const TCFunctionInfo& info);
extern void Install(const TCFunctionInfo infos[], const unsigned char modulename[]);

#define CTINFO_H

#endif
