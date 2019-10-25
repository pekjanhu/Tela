/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2000 Pekka Janhunen
 */

#include "prg.H"

static int avoid_ctors = 0;
static char *star = "";			// This MUST be set to "*" if avoid_ctors is nonzero!!!

//#define UNIQ "__tela_"
#define UNIQ ""
#define VARBODY UNIQ "var"
#define CONSTBODY UNIQ "cnst"
#define GLOBBODY UNIQ "glob"
#define INPUTS UNIQ "inputs"
#define OUTPUTS UNIQ "outputs"
#define INDICES UNIQ "aux_index_vector"
#define FNAME_APPENDIX "function" /* Must be same as in ctpp.C */
#define MAXSYMBOLS 1000	/* maximum number of different symbols referenced in a function */
//#ifdef AVOID_CTORS
//#  define STAR "*"
//#else
//#  define STAR ""
//#endif

extern char *VersionString;

extern void SetAvoidCtorsFlag(int flag)
// Call this function from the outside to alter the avoid_ctors flag
{
	if (flag) {
		avoid_ctors = 1;
		star = "*";
	} else {
		avoid_ctors = 0;
		star = "";
	}
}

static char*barename(const Tstring& S) {
	// Remove a leading package selector from symbol name
	char *ptr = strchr((char*)S,':');
	return ptr ? ptr+1 : (char*)S;
}

static char*barename(const Tchar*s) {
	char *ptr = strchr((char*)s,':');
	return ptr ? ptr+1 : (char*)s;
}

struct VarRef {Toperand op; VarRef(Toperand op1) : op(op1) {}};

static Tprg *ThePrgptr;

ostream& operator<<(ostream& o, const VarRef& v) {
	const Toperand& op = v.op;
	switch (op.kind()) {	
	case Ksym:
		o << "(*(" << barename(op.symbol().name()) << GLOBBODY << "->value()))";
		break;
	case Kpar:
		o << '(' << (0<op.offset() && op.offset() <= ThePrgptr->Nargin()+ThePrgptr->Nargout() ? "" : star) << VARBODY << op.offset() << ')';
		break;
	case Kobj:
		o << '(' << star << CONSTBODY << op.offset() << ')';
		break;
	case Klit:
		if (op.literal()==1)
			o << '(' << star << UNIQ << "UnitObject" << ')';
		else {
			cerr << "Ctgen warning: Literal which is not equal to 1\n";
			o << op.literal();
		}
		break;
	case Kallrange:
		o << '(' << star << UNIQ << "VoidObject" << ')';
		break;
	default:
		o << instrinfo[op.opcode()].mnemonic;
		break;
	}
	return o;
}

// Class TCForm is a wrapper class, when outputted, it prints an object in C-compilable form

class TCForm {
 private:
	const Tobject* ptr;

//        TCForm(const TCForm&); // do not implement			// COMMENTED OUT 29 Sep 2004, g++-3.4.0 does not accept!
//        TCForm& operator=(const TCForm&); // do not implement	// SAME

 public:
	TCForm(const Tobject& obj) : ptr(&obj) {}
	friend ostream& operator<<(ostream& o, const TCForm& cf);
};

ostream& operator<<(ostream& o, const TCForm& cf) {
	const Tobject& obj = *cf.ptr;
	switch (obj.kind()) {
	case Kcomplex:
		o << "Tcomplex(" << real(obj.ComplexValue()) << ',' << imag(obj.ComplexValue()) << ')';
		break;
	default:
		o << obj;
	}
	return o;
}

static void F1arb(ostream& o, const char*opname, const Toperand& arg1) {
	o << opname << "(" << VarRef(arg1) << ");\n";
}

static void F2arb(ostream& o, const char*opname, const Toperand& arg1, const Toperand& arg2) {
	o << opname << "(" << VarRef(arg1) << "," << VarRef(arg2) << ");\n";
}

static void F3arb(ostream& o, const char*opname, const Toperand& arg1, const Toperand& arg2, const Toperand& arg3) {
	o << opname << "(" << VarRef(arg1) << "," << VarRef(arg2) << "," << VarRef(arg3) << ");\n";
}

#define F1(opname) F1arb(o,opname,d[0])
#define F2(opname) F2arb(o,opname,d[0],d[1])
#define F3(opname) F3arb(o,opname,d[0],d[1],d[2])

static void AddToSet(TIntLL& L, Tint x) {
	for (int i=0; i<L.length(); i++) if (L[i]==x) return;
	L.append(x);
}

static int MemberQ(const TIntLL& L, Tint x) {
	for (int i=0; i<L.length(); i++) if (L[i]==x) return 1;
	return 0;
}

static void DoExternalDeclarations(ostream& o, Tprg& prg)
{
	int Nop;
	for (int PC=0; PC<prg.length(); ) {
		Toperand *f = &prg[PC++];
		if (!f->IsOpcode()) {err << "Internal in DoExternalDeclarations: " << *f << " is not an opcode\n"; error();}
		Nop = f->Noperands();
		Toperand *d = &prg[PC];
		PC+= Nop;
		if ((f->rawcode() == EncodeOpcode(Fcall,0) || f->rawcode() == EncodeOpcode(Fcall,0)) && d[0].kind()==Ksym) {
			o << "extern \"C\" int " << barename(d[0].symbol().name()) << FNAME_APPENDIX
			  << "(const TConstObjectPtr[], const int, const TObjectPtr[], const int);\n";
		}
	}
	o << '\n';
}

static void DoSymbolReferences(ostream& o, Tprg& prg, int InInitialization=0)
{
	Tsymbol *syms[MAXSYMBOLS];
	Tint Nsyms = 0;
	for (int PC=0; PC<prg.length(); PC++) {
		Toperand *f = &prg[PC];
		if (f->kind() == Ksym && prg[PC-1].opcode() != Fcall && prg[PC-1].opcode() != Fscall) {
			int found = 0;
			for (int i=0; i<Nsyms; i++)
				if (syms[i] == &(f->symbol())) {found=1; break;}
			if (!found) {
				if (Nsyms >= MAXSYMBOLS) {
					cerr << "*** Too many (more than " << MAXSYMBOLS << ") global symbols in one function.\n";
					cerr << "    You can increase MAXSYMBOLS in Ctgen.C.\n";
					return;
				}
				syms[Nsyms++] = &(f->symbol());
				if (InInitialization)
					o << "\t\t" << barename(f->symbol().name()) << GLOBBODY << " = theHT.add((Tchar*)\"" << f->symbol().name() << "\");\n";
				else
					o << "\tstatic Tsymbol *" << barename(f->symbol().name()) << GLOBBODY << ";\n";
			}
		}
	}
}

static void FindJumpAddresses(Tprg& prg, TIntLL& L)
{
	int Nop,pcaddr;
	for (int PC=0; PC<prg.length(); ) {
		Toperand *f = &prg[PC++];
		if (!f->IsOpcode()) {err << "Internal in FindJumpAddresses: " << *f << " is not an opcode\n"; error();}
		Nop = f->Noperands();
		Toperand *d = &prg[PC];
		PC+= Nop;
#		define pair(opcode,Nargs) EncodeOpcode(opcode,Nargs)
		int c = f->rawcode();
//		if (c==pair(Fbra,1)) {
//			pcaddr = PC + d[0].literal() - 1;
//			AddToSet(L,pcaddr);
//		} else
		if (c==pair(Fjmp,1)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
//		} else if (c==pair(Fbif,2)) {
//			pcaddr = PC + d[0].literal()-1;
//			AddToSet(L,pcaddr);
//		} else if (c==pair(Fbifn,2)) {
//			pcaddr = PC + d[0].literal()-1;
//			AddToSet(L,pcaddr);
		} else if (c==pair(Fjif,2)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		} else if (c==pair(Fjifn,2)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		} else if (c==pair(Fjngt,3)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		} else if (c==pair(Fjnlt,3)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		} else if (c==pair(Fjnge,3)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		} else if (c==pair(Fjnle,3)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		} else if (c==pair(Fincjmp,2)) {
			pcaddr = d[0].literal();
			AddToSet(L,pcaddr);
		}
#		undef pair
	}
}

static int LastLabelUsed = 0;

static ostream& GotoInstruction(ostream& o, int pcaddr, const Tprg& prg) {
	o << "goto L";
	if (pcaddr < prg.length())
		o << pcaddr;
	else {
		o << "AST_LABEL";
		LastLabelUsed = 1;
	}
	return o << ";\n";
}
	
void Ctgen(ostream& o, Tprg& prg)
{
	ThePrgptr = &prg;
	TObjectLL *old_objLLptr = Toperand::objLLptr;
	prg.JustMakeCurrent();
	Tchar *FunctionName = &(prg.Symbol()) ? (Tchar*)(prg.Symbol().name()) : (Tchar*)"main";
	Tchar *ptr = strchr(FunctionName,':');
	int IsStatic = 0;
	if (ptr) {FunctionName=ptr+1; IsStatic=1;}

	LastLabelUsed = 0;
	
	//cerr << "compiling " << FunctionName << "...\n";

	// We need to access gather/scatter functions
	o << "\n// ct-translation of t-function " << FunctionName << ".\n";
	o << "// Produced by Tela " << VersionString << ".\n",
//	o << "\n#include \"gatscat.H\"\n";

	// Find out external functions and declare them
	//o << "// External functions for " << FunctionName << "\n";
	DoExternalDeclarations(o,prg);
	
	// Output function header
//	if (IsStatic) o << "static\n";
	o << '[';
    int argc;
	for (argc=prg.Nargin()+1; argc<=prg.Nargin()+prg.Nargout(); argc++) {
		o << VARBODY << argc;
		if (argc < prg.Nargin()+prg.Nargout()) o << ',';
	}
	o << "] = " << barename(FunctionName) << "(";
	for (argc=1; argc<=prg.Nargin(); argc++) {
		o << VARBODY << argc;
		if (argc<prg.Nargin()) o << ',';
	}
	o << ")\n{\n";

	// Standard arrays for argument passing etc. The maximum length is obtained from prg.MaxNops()
	if (!avoid_ctors) {
		o << '\t' << "Tobject " << UNIQ << "DummyObject;\n\n";
	}
	o << "\t// Standard arrays for argument passing\n";
	o << "\tTConstObjectPtr " << INPUTS << '[' << max(1,prg.MaxNops()) << "];\n";
	o << "\tTObjectPtr " << OUTPUTS << '[' << max(20/*4*/,prg.MaxNops()) << "];\n";	// Range uses 4 args
	if (!avoid_ctors) {
		o << "\tfor (int " << UNIQ << "_i=0; " << UNIQ << "_i<20; " << UNIQ << "_i++) " << OUTPUTS << '[' << UNIQ << "_i] = &" << UNIQ << "DummyObject;\n";
	}
	o << "\tTint " INDICES << '[' << MAXRANK << "];\n";
	
	// Output declarations of local objects
	o << "\t// Local objects\n";
	if (avoid_ctors) {
		o << "\tTobject *" << VARBODY << "0 = new Tobject;\n";
		for (argc=prg.Nargin()+prg.Nargout()+1; argc<prg.StackFrameSize(); argc++)
			o << "\tTobject *" << VARBODY << argc << " = new Tobject;\n";
	} else {
		o << "\tTobject " << VARBODY << "0;\n";
		for (argc=prg.Nargin()+prg.Nargout()+1; argc<prg.StackFrameSize(); argc++)
			o << "\tTobject " << VARBODY << argc << ";\n";
	}
	// Initialize var0 to VOID. If we don't do this, it will remain UNDEFINED.
	// When other ct-functions that have no output args get called, PRI $0 instructions
	// will be added, and these will write "<Undefined>" on cout if $0 is UNDEFINED.
	if (avoid_ctors) {
		o << '\t' << VARBODY << "0->SetToVoid();\n";
	} else {
		o << '\t' << VARBODY << "0.SetToVoid();\n";
	}
	
	// Output definitions of constant objects
	o << "\n\t// \"Constant\" objects\n";
	for (argc=0; argc<prg.Nstatic(); argc++) {
		if (avoid_ctors) {
			o << "\tstatic Tobject *" << CONSTBODY << argc << ";\n";
		} else {
			o << "\tstatic Tobject " << CONSTBODY << argc << ";\n";
		}
	}
	// We may also need a unit object
	// And we may need a VOID object
	if (avoid_ctors) {
		o << "\tstatic Tobject *" << UNIQ << "UnitObject;\n";
		o << "\tstatic Tobject *" << UNIQ << "VoidObject;\n";
	} else {
		o << "\tstatic Tobject " << UNIQ << "UnitObject;\n";
		o << "\tstatic Tobject " << UNIQ << "VoidObject;\n";
	}
	/*
	  Notice the UnitObject is quite a hack. It is (probably) only needed in
	  RANGE, which generates literal objects with value unity as the default
	  step. It is passed as a modifiable object pointer to Range() so we cannot
	  make it const here, however Range() does not modify it.
    */

	// Initialize find symbol pointers to all symbols (global vars)
	o << "\n\t// Global variables\n";
	DoSymbolReferences(o,prg);

	// Generate code to do first-time-only initialization of 'const' etc objects
	// DLD apparently fails to do static initialization properly! therefore choose
	// zero as first value of the static flag (it presumably clears the memory space).
	o << "\n\tstatic int InitializationDone = 0;\n\tif (!InitializationDone) {\n";
	// Const object values
	if (avoid_ctors) {
		for (argc=0; argc<prg.Nstatic(); argc++)
			o << "\t\t" << CONSTBODY << argc << " = new Tobject(" << TCForm(*prg.nthobject(argc)) << ");\n";
	} else {
		for (argc=0; argc<prg.Nstatic(); argc++)
			o << "\t\t" << CONSTBODY << argc << " = " << TCForm(*prg.nthobject(argc)) << ";\n";
	}
	// UnitObject
	if (avoid_ctors) {
		o << "\t\t" << UNIQ << "UnitObject = new Tobject(1);\n";
		o << "\t\t" << UNIQ << "VoidObject = new Tobject;\n";
		o << "\t\t" << UNIQ << "VoidObject->SetToVoid();\n";
	} else {
		o << "\t\t" << star << UNIQ << "UnitObject = 1;\n";
		o << "\t\t" << UNIQ << "VoidObject.SetToVoid();\n";
	}
	DoSymbolReferences(o,prg,1);
	o << "\t\tInitializationDone = 1;\n\t}\n";
	
	o << "\n\t// Code\n";
	
	// Find set of possible jump targets
	TIntLL jumplines(0);
	FindJumpAddresses(prg,jumplines);
	
	int i,oldPC,Nop;
	Toperand *f;
	for (int PC=0; PC<prg.length(); ) {

		if (MemberQ(jumplines,PC)) o << 'L' << PC << ":\n";
		
		oldPC = PC;
		f = &prg[PC++];
		if (!f->IsOpcode()) {err << "Internal: " << *f << " is not an opcode\n"; error();}
		Nop = f->Noperands();
		Toperand *d = &prg[PC];
		PC+= Nop;

		o << '\t';
		
#		define pair(opcode,Nargs) EncodeOpcode(opcode,Nargs)
#		define CASE(opcode,Nargs) } else if (c==pair(opcode,Nargs)) {
#		define CASE2(opcode1,opcode2,Nargs) } else if (c==pair(opcode1,Nargs) || c==pair(opcode2,Nargs)) {
#		define CASE3(opcode1,opcode2,opcode3,Nargs) \
            } else if (c==pair(opcode1,Nargs) || c==pair(opcode2,Nargs) || c==pair(opcode3,Nargs)) {

	    int c = f->rawcode();

		if (c==pair(Fadd,2)) {
			F2("Add");
		CASE(Fadd,3)
			F3("Add");
			
		CASE(Finc,1)
			F1("Inc");
			
		CASE(Fsub,2)
			F2("Sub");
		CASE(Fsub,3)
			F3("Sub");
			
		CASE(Fdec,1)
			F1("Dec");
			
		CASE(Fmul,2)
			F2("Mul");
		CASE(Fmul,3)
			F3("Mul");
			
		CASE(Fdiv,2)
			F2("Div");
		CASE(Fdiv,3)
			F3("Div");

		CASE(Fmod,3)
			F3("Mod");
			
		CASE(Fpow,2)
			F2("Pow");
		CASE(Fpow,3)
			F3("Pow");
			
		CASE(Fneg,1)
			F1("Neg");
		CASE(Fneg,2)
			F2("Neg");
		CASE(Fabs,1)
			F1("Abs");
		CASE(Fabs,2)
			F2("Abs");
			
		CASE(Fmove,2)
			o << VarRef(d[0]) << " = " << VarRef(d[1]) << ";\n";
			
		CASE(Fgt,3)
			F3("Gt");
		CASE(Flt,3)
			F3("Lt");
		CASE(Fge,3)
			F3("Ge");
		CASE(Fle,3)
			F3("Le");
		CASE(Feq,3)
			F3("Eq");
		CASE(Fne,3)
			F3("Ne");
			
		CASE(Fand,3)
			F3("And");
		CASE(Fand,2)
			F3arb(o,"And",d[0],d[0],d[1]);
		CASE(For,3)
			F3("Or");
		CASE(For,2)
			F3arb(o,"Or",d[0],d[0],d[1]);
		CASE(Fnot,2)
			F2("Not");
		CASE(Fnot,1)
			F2arb(o,"Not",d[0],d[0]);

		CASE(Fmin,3)
			F3("Min");
		CASE(Fmax,3)
			F3("Max");
		CASE(Fmin,2)
			F2("Min");
		CASE(Fmax,2)
			F2("Max");

		CASE(Fnin,1)
			o << VarRef(d[0]) << " = Nargin;\n";
		CASE(Fnout,1)
			o << VarRef(d[0]) << " = Nargout;\n";
		CASE(Fmmpos,1)
			o << VarRef(d[0]) << " = MinMaxPosition();\n";
		CASE(Fgetin,2)
			o << VarRef(d[0]) << " = *(argin[" << VarRef(d[1]) << ".IntValue()]);\n";
		CASE(Fgetout,2)
			o << VarRef(d[0]) << " = *(argout[" << VarRef(d[1]) << ".IntValue()]);\n";
		CASE(Fsetout,2)
			o << "*(argout[" << VarRef(d[0]) << ".IntValue()]) = " << VarRef(d[1]) << ";\n";

		CASE(Fvloop,0)
			;
		CASE(Fendv,1)
			;
		
//		CASE(Fbra,1)
//			GotoInstruction(o,PC+d[0].literal()-1,prg);
		CASE(Fjmp,1)
			GotoInstruction(o,d[0].literal(),prg);
		CASE(Fincjmp,2)
			o << "Inc(" << VarRef(d[1]) << "); ";
			GotoInstruction(o,d[0].literal(),prg);
//		CASE(Fbif,2)
//			o << "if (" << VarRef(d[1]) << ".IsNonzero()) ";
//			GotoInstruction(o,PC+d[0].literal()-1,prg);
//		CASE(Fbifn,2)
//			o << "if (!(" << VarRef(d[1]) << ".IsNonzero())) ";
//			GotoInstruction(o,PC+d[0].literal()-1,prg);
		CASE(Fjif,2)
			o << "if (" << VarRef(d[1]) << ".IsNonzero()) ";
			GotoInstruction(o,d[0].literal(),prg);
		CASE(Fjifn,2)
			o << "if (!(" << VarRef(d[1]) << ".IsNonzero())) ";
			GotoInstruction(o,d[0].literal(),prg);

		CASE(Fjngt,3)
			o << "if (!TestGt(" << VarRef(d[1]) << "," << VarRef(d[2]) << ")) ";
			GotoInstruction(o,d[0].literal(),prg);
		CASE(Fjnlt,3)
			o << "if (!TestLt(" << VarRef(d[1]) << "," << VarRef(d[2]) << ")) ";
			GotoInstruction(o,d[0].literal(),prg);
		CASE(Fjnge,3)
			o << "if (!TestGe(" << VarRef(d[1]) << "," << VarRef(d[2]) << ")) ";
			GotoInstruction(o,d[0].literal(),prg);
		CASE(Fjnle,3)
			o << "if (!TestLe(" << VarRef(d[1]) << "," << VarRef(d[2]) << ")) ";
			GotoInstruction(o,d[0].literal(),prg);
			
		CASE(Fpri,1)
			o << "if (" << VarRef(d[0]) << ".kind()!=Kvoid) cout << " << VarRef(d[0]) << " << '\\n';\n";
		CASE(Fnop,0)
			;

        } else if (c==pair(Fcall,0) || c==pair(Fscall,0)) {		// CALL fn, Noutputs, out1,out2,...outN, in1,in2,...,inM
			int Noutputs = d[1].literal();
			int Ninputs = Nop - Noutputs - 2;
			for (i=0; i<Ninputs; i++)
				o << INPUTS << '[' << i << "] = &" << VarRef(d[2+Noutputs+i]) << ";\n\t";
			if (Noutputs == 1 && d[2].kind()==Kpar && d[2].offset()==0)
				if (avoid_ctors) {
					o << VARBODY << "0->SetToVoid();\n\t";
				} else {
					o << VARBODY << "0.SetToVoid();\n\t";
				}
			for (i=0; i<Noutputs; i++)
				o << OUTPUTS << '[' << i << "] = &" << VarRef(d[2+i]) << ";\n\t";
//			if (d[0].kind() != Ksym) cerr << "*** Ctgen: calling non-symbol, not yet implemented\n";
			if (d[0].kind() == Kpar) {
				o << "Call(" << VarRef(d[0])
				  << "," << INPUTS << "," << Ninputs << "," << OUTPUTS << "," << Noutputs << ",0);\n";
			} else if (d[0].kind() == Ksym) {
//				if (d[0].symbol().value()->kind()!=Kfunction)
//					cerr << "*** Ctgen: CALL instruction for non-function\n";
//				else {
//					for (i=Noutputs; i<10 /*d[0].symbol().value()->FunctionValue()->Nargout()*/; i++)
//						o << OUTPUTS << '[' << i << "] = &Dummy;\n\t";
					o << barename(d[0].symbol().name()) << FNAME_APPENDIX
						<< "(" << INPUTS << "," << Ninputs << "," << OUTPUTS << "," << Noutputs << ");\n";
//				}
			} else
				cerr << "*** Ctgen: CALL instruction for non-symbol, non-par\n";
		CASE2(Fgath,Fscat,0)
			for (i=2; i<Nop; i++)
				o << INPUTS << "[" << i-2 << "] = &" << VarRef(d[i]) << ";\n\t";
			o << (f->opcode()==Fgath ? "Gather" : "Scatter");
			o << "(" << VarRef(d[0]) << ',' << VarRef(d[1]) << "," << INPUTS << "," << Nop-2 << ");\n";
		CASE2(Farray,Fappend,0)		// ARRAY result,arg1,arg2,...,argN
			                        // APPEND result,arg1,arg2,...,argN
			for (i=1; i<Nop; i++)
				o << INPUTS << '[' << i-1 << "] = &" << VarRef(d[i]) << ";\n\t";
			o << (f->opcode()==Farray ? "BuildArray" : "PasteArrays");
			o << "(" << VarRef(d[0]) << "," << INPUTS << "," << Nop-1 << ");\n";
		CASE2(Fmgath,Fmscat,0)
			for (i=2; i<Nop; i++)
				o << INPUTS << "[" << i-2 << "] = &" << VarRef(d[i]) << ";\n\t";
			o << "MGatScat(" << VarRef(d[0]) << "," << VarRef(d[1])
			  << "," << INPUTS << "," << Nop-2 << "," << (f->opcode()==Fmscat) << ");\n";
		CASE(Frange,0)
			for (i=0; i<4; i++)
				o << OUTPUTS << "[" << i << "] = (Tobject*)&" << VarRef(d[i]) << ";\n\t";
			o << "Range(" << OUTPUTS << ");\n";
		CASE3(Fizeros,Frzeros,Fczeros,0)
			if (Nop <= 3) {
				o << VarRef(d[0]) << ".";
				if (f->opcode()==Fizeros) o << 'i'; else if (f->opcode()==Frzeros) o << 'r'; else o << 'c';
				o << "zeros(TDimPack(";
				for (i=1; i<Nop; i++) {
					o << VarRef(d[i]) << ".IntValue()";
					if (i < Nop-1) o << ',';
				}
				o << "));\n";
			} else {
				for (i=1; i<Nop; i++)
					o << INDICES << "[" << i-1 << "] = " << VarRef(d[i]) << ".IntValue();\n\t";
				o << VarRef(d[0]) << ".";
				if (f->opcode()==Fizeros) o << 'i'; else if (f->opcode()==Frzeros) o << 'r'; else o << 'c';
				o << "zeros(TDimPack(" << INDICES << "," << Nop-1 << "));\n";
			}

		} else {
			cerr << "Internal: illegal instruction " << instrinfo[f->kind()].mnemonic << "(" << Nop << ") in Ctgen()\n";
			o << "ILLEGAL INSTRUCTION\n";
			//error();
		}
		
#		undef pair	

	}	// end of the big for loop

	if (LastLabelUsed) o << "LAST_LABEL:\n";
	if (avoid_ctors) {
		o << "\t// Deallocate locally created objects\n";
		o << "\tdelete " << VARBODY << "0;\n";
		for (argc=prg.Nargin()+prg.Nargout()+1; argc<prg.StackFrameSize(); argc++)
			o << "\tdelete " << VARBODY << argc << ";\n";
	
	}
	o << "\treturn 0;\n}\t// " << FunctionName << "\n";

	Toperand::objLLptr = old_objLLptr;
	
}
