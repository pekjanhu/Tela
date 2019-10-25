/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "machine.H"
#endif
#include "machine.H"

Tinstrinfo instrinfo[] = {
	{"sym",0,0},{"par",0,0},{"obj",0,0},{"lit",0,0},{"allrange",0,0},
	{"ADD",1,0},{"INC",0,0},{"SUB",1,0},{"DEC",0,0},{"MUL",1,0},{"DIV",1,0},{"POW",1,0},{"MOD",1,0},
	{"NEG",0,0},{"ABS",0,0},
	{"MOVE",0,0},{"CALL",1,1},{"SCALL",1,1},
	{"GT",0,0},{"LT",0,0},{"GE",0,0},{"LE",0,0},
	{"EQ",0,0},{"NE",0,0},
	{"JNGT",0,0},{"JNLT",0,0},{"JNGE",0,0},{"JNLE",0,0},
	{"AND",0,0},{"OR",0,0},{"NOT",0,0},
	{"MIN",0,0},{"MAX",0,0},
//	{"BRA",0,0},{"BIF",0,0},{"BIFN",0,0},
	{"JMP",0,0},{"INCJMP",0,0},{"JIF",0,0},{"JIFN",0,0},
	{"PRI",0,0},
	{"IZEROS",0,1},{"RZEROS",0,1},{"CZEROS",0,1},{"OZEROS",0,1},
	{"GATH",0,1},{"SCAT",0,1},{"MGATH",0,1},{"MSCAT",0,1},
	{"RANGE",0,1},{"ARRAY",0,1},{"APPEND",0,1},
	{"NIN",0,0},{"NOUT",0,0},
	{"GETIN",0,0},{"GETOUT",0,0},{"SETOUT",0,0},
	{"MMPOS",0,0},
#if SUPPORT_LOOP_TILING
	{"VLOOP",0,1},{"ENDV",0,0},
#endif
	{"NOP",0,0}
};

Thpm HPM[Fnop+1] = {{0,0}};

TObjectLL *Toperand::objLLptr = 0;

Toperand::Toperand(Tcode c, int nops) {
	if (nops >= 4 || instrinfo[c].arbitraryN)
		encoded = EncodeOpcode(c,0);
	else
		encoded = EncodeOpcode(c,nops);
	noperands = nops;
}

static int FindObject(const Tobject& x, const TObjectLL& LL) {
	/* Search for occurrence of x in linear list of object pointers LL.
	   If found, return the index (non-negative), otherwise return -1. */
	for (int i=0; i<LL.length(); i++)
		if (*LL[i] == x)
			return i;
	return -1;
}

Toperand::Toperand(Tobject& obj) : encoded(EncodeOpcode(Kobj,0)) {
	int i = FindObject(obj,*objLLptr);
	if (i>=0) {
		staticoffset = i;
	} else {
		obj.UnflagTemporary();
		objLLptr->append(&obj);
		staticoffset = objLLptr->length()-1;
	}
}

ostream& operator<<(ostream& o, const Toperand& op) {
	switch (op.kind()) {
	case Ksym:
		o << op.symbol();
		break;
	case Kpar:
		o << '$' << op.offset();
		break;
	case Kobj:
		o << *(op.NthObjectPtr());
		break;
	case Klit:
		o << op.literal();
		break;
	case Kallrange:
		o << ':';
		break;
	default:
		o << instrinfo[op.opcode()].mnemonic;
		break;
	}
	return o;
}

