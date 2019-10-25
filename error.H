/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma interface
#endif
#if USE_STRINGSTREAM
#  include <sstream>
#else
#  include <strstream.h>
#endif
#include <setjmp.h>

#if USE_STRINGSTREAM
extern stringstream err;
#else
extern strstream err;
#endif

#define COMPTYPE Tint
#define DEFAULT_COMPVALUE 0
#define LINEARLIST TIntLL
#  include "templ/tLL.H"
#undef LINEARLIST
#undef DEFAULT_COMPVALUE
#undef COMPTYPE

class TIntStack {		// Program counter stack, common for all functions
 private:
	TIntLL LL;			// Implemented as a linear list of integers
 public:
	TIntStack() : LL(0) {}			// Initially, the stack is empty
	void push(Tint i) {LL.append(i);}
	Tint top() const {return LL[LL.length()-1];}			// stack.top() gives ref to last int pushed
	Tint top(int nth) const {return LL[LL.length()-nth];}	// stack.top(n) gives the nth-last int pushed
	Tint pop() {int L; LL=L=LL.length()-1; return LL[L];}	// stack.pop() gives stack.top() and removes it
	int length() const {return LL.length();}				// stack.length() == number of items in the stack
	Tint& operator[](int i) {return LL[LL.length()-1-i];}	// stack[0] == &stack.top(), etc.
};

extern TIntStack thePCstack;		// General program counter stack

/* TJumpBuffer could be a class, but some C++ compilers don't like
   setjmp inside inline function, or refuse to inline the function
   which is equally bad because then setjmp will record wrong context
   - therefore we just use #define macros here */

struct TJumpBuffer {
	jmp_buf jmp;
	int PCstacklen;
};

// SetJmp(jmp) will return in its first argument the value returned by setjmp()

#define SetJmp(val,jumpbuffer)\
    {(jumpbuffer).PCstacklen=thePCstack.length();\
	 (val)=setjmp((jumpbuffer).jmp);}

#define LongJmp(jumpbuffer,val)\
    {\
		while (thePCstack.length() > (jumpbuffer).PCstacklen) thePCstack.pop();\
        longjmp((jumpbuffer).jmp,val);\
	}

extern TJumpBuffer jmp1,jmp2;

extern void error();
