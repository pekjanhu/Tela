/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifndef MACHINE_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "symbol.H"
#include "gatscat.H"

enum Tcode {
	Ksym,	// Symbol (variable) reference: symbol's value is Tobject. Tsymbol* is stored in symptr.
	Kpar,	// stackoffset is local stack frame offset
	Kobj,	// Literal object: staticoffset is offset (in sizeof(Tobject)) from the start of data section
	Klit,	// Literal int: intvalue is the int value directly
	Kallrange,	// Denotes ':', ALLRANGE object, which is equivalent to VOID
	// The remaining codes are opcodes. In each case, noperands tells the number of operands to follow.
	Fadd,Finc,Fsub,Fdec,Fmul,Fdiv,Fpow,Fmod,
	Fneg,Fabs,
	Fmove,Fcall,Fscall,
	Fgt,Flt,Fge,Fle,Feq,Fne,
	Fjngt,Fjnlt,Fjnge,Fjnle,
	Fand,For,Fnot,
	Fmin,Fmax,
//	Fbra,Fbif,Fbifn,
	Fjmp,Fincjmp,Fjif,Fjifn,
	Fpri,
	Fizeros,Frzeros,Fczeros,Fozeros,
	Fgath,Fscat,Fmgath,Fmscat,
	Frange,Farray,Fappend,
	Fnin,Fnout,
	Fgetin,Fgetout,Fsetout,
	Fmmpos,
#if SUPPORT_LOOP_TILING
	Fvloop,Fendv,
#endif
	Fnop
};

struct Tinstrinfo {const char *mnemonic; int isflop; int arbitraryN;};

extern Tinstrinfo instrinfo[];	// opcode mnemonics  as char* -strings, plus info about isflop

struct Thpm {T_Ninstructions Ninstr; T_Noperations Nops;};

extern Thpm HPM[Fnop+1];

#define EncodeOpcode(code,Nops) (((code)<<2)+(Nops))

// TLiteralInt is wrapper class for Tint, needed to distinguish Toperand -> Klit constructor
class TLiteralInt {
 private:
	Tint i;
 public:
	TLiteralInt(Tint x) : i(x) {}
	operator Tint() const {return i;}
};

class Toperand {
 private:
	int encoded;
	union {
		Tsymbol* symptr;
		TPtrInt stackoffset;
		TPtrInt staticoffset;
		Tint intvalue;
		int noperands;
		TPtrInt general;	// TPtrInt type must be >= than Tint,int,Tsymbol*
	};
 public:
	static TObjectLL *objLLptr;
	Toperand() : encoded(EncodeOpcode(Fnop,0)), noperands(0) {}		// creates opcode NOP (no-operation)
	Toperand(Tcode c) : encoded(EncodeOpcode(c,0)), noperands(0) {}	// creates given code word with no operands
	//Toperand(Tcode c, TPtrInt gen) {encoded=EncodeOpcode(c,0); general=gen;}
	Toperand(Tcode c, int nops);
	Toperand(const TLiteralInt& li) : encoded(EncodeOpcode(Klit,0)), intvalue(li) {}
	Toperand(Tsymbol& sym) : encoded(EncodeOpcode(Ksym,0)), symptr(&sym) {}
	Toperand(Tobject& obj);
	Toperand(Tslot s) : encoded(EncodeOpcode(Kpar,0)), stackoffset(s) {}
	Toperand(Tallrange a) : encoded(EncodeOpcode(Kallrange,0)) {}
	Toperand(const Toperand& op) : encoded(op.encoded), general(op.general) {}
	Toperand& operator=(const Toperand& op) {encoded=op.encoded; general=op.general; return *this;}
	// --- inquiry functions
	Tcode kind() const {return Tcode(encoded>>2);}
	int IsOpcode() const {return encoded > EncodeOpcode(Klit,0);}
	Tcode opcode() const {return kind();}
	int rawcode() const {return encoded;}
	Tsymbol& symbol() const {return *symptr;}
	int offset() const {return stackoffset;}
	Tint literal() const {return intvalue;}
	TConstObjectPtr NthObjectPtr() const {return (*objLLptr)[staticoffset];}
	// --- inquiry/set functions
	int& Noperands() {return noperands;}
	int operator==(const Toperand& op) {return encoded==op.encoded && general==op.general;}
};

// --- I/O
ostream& operator<<(ostream& o, const Toperand& op);

#define MACHINE_H

#endif
