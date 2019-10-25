/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2002 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "prg.H"
#endif

#define CHECK_INTERNAL_ERRORS 0
// This was defined as 1 for over half a year and never we got any internal error.
// Maybe it's now safe to supress these checks, which may slow the thing down
// a little bit.

/*
  TO DO:
  ------
  Fix getin, getout, nin, nout, setout.
  They should work similar to ctfiles, count also named args.
  */

#include "prg.H"
#include "objarithm.H"
#include <cctype>
#include <fstream>

#ifdef __GNUC__
#  ifndef alloca
#    define alloca __builtin_alloca
#  endif
#  ifndef HAVE_ALLOCA
#    define HAVE_ALLOCA
#  endif
#elif HAVE_ALLOCA_H
#  include <alloca.h>
#  ifndef HAVE_ALLOCA
#    define HAVE_ALLOCA
#  endif
#elif C_ALLOCA
   extern "C" void *alloca(unsigned int);
#elif defined(CONVEX)
#  include <stdlib.h>
   extern "C" char *alloca(int);
#endif

// These stacks are declared extern in prg.H and defined here:
TObjectStack theStack;
TCodeStack theCodeStack;

static int PC=0;

#define COMPTYPE Toperand
#define DEFAULT_COMPVALUE Toperand()
#define LINEARLIST TInstructionList1

#include "templ/tLL.C"

// TInstructionList members

void TInstructionList::add(Tcode code, const Toperand& op1) {
	append(Toperand(code,1));
	append(op1);
	maxNops = max(maxNops,1);
}

void TInstructionList::add(Tcode code, const Toperand& op1, const Toperand& op2) {
	append(Toperand(code,2));
	append(op1);
	append(op2);
	maxNops = max(maxNops,2);
}

void TInstructionList::add(Tcode code, const Toperand& op1, const Toperand& op2, const Toperand& op3) {
	append(Toperand(code,3));
	append(op1);
	append(op2);
	append(op3);
	maxNops = max(maxNops,3);
}

void TInstructionList::add(Tcode code, const Toperand& op1, const Toperand& op2, const Toperand& op3, const Toperand& op4) {
	append(Toperand(code,4));
	append(op1);
	append(op2);
	append(op3);
	append(op4);
	maxNops = max(maxNops,4);
}

void TInstructionList::add(Tcode code, int Noperands) {
	if (Noperands > MAX_NOPERANDS) {
		err << "Internal: TInstructionList::add, Noperands=" << Noperands << " > " << MAX_NOPERANDS << "\n";
		error();
	}
	append(Toperand(code,Noperands));
 	maxNops = max(maxNops,Noperands);
}

ostream& operator<<(ostream& o, const TInstructionList& il) {
	o << '\n';
	for (int i=0; i<il.length(); i++) {
		if (il[i].IsOpcode()) {
			int N = il[i].Noperands();
			o << i << '\t' << il[i] << '\t';
			for (int j=0; j<N; j++) {
				o << il[i+1+j];
				if (j<N-1) o << ',';
			}
			i+= N;
			if (i<il.length()-1) o << '\n';
		} else {
			err << "Internal error in disassembly: syntax error in instruction list\n";
			error();
		}
	}
	return o;
}	

// TObjectStack members

void TObjectStack::pop(int N) {
	// Shorten the stack by N objects
	int L = LL.length();
	LL = L - N;
}

TObjectStack::~TObjectStack() {
	/*
	TObjectPtr *p=LL.Ptr();
	for (int i=0; i<LL.length(); i++) delete p[i];
	// Commented this out since it would be called only at exit,
	// and it seems to cause core dumps sometimes ...
	// If you ever declare TObjectStacks other than theStack, you
	// should rethink this destructor.
	*/
}

// Tprg members

Tprg::Tprg(const Tchar *fn) : ol(0), linenums(0), nargin(0), nargout(0), nargin_min(0), nargout_min(0), nlocal(0), ntemp(0), inputellipsis(0), outputellipsis(0), symptr(0) {
	global::NTprg++;
	CalcStack();
	MakeCurrent();
	filename = strdup(fn);
}

Tprg::Tprg(const Tchar *fn, int in, int out) : ol(0), linenums(0), nargin(in), nargout(out), nargin_min(0), nargout_min(0), nlocal(0), ntemp(0), inputellipsis(0), outputellipsis(0), symptr(0) {
	global::NTprg++;
	CalcStack();
	MakeCurrent();
	filename = strdup(fn);
}

Tprg::Tprg(const Tchar *fn, int in, int out, int loc) : ol(0), linenums(0), nargin(in), nargout(out), nargin_min(0), nargout_min(0), nlocal(loc), ntemp(0), inputellipsis(0), outputellipsis(0), symptr(0) {
	global::NTprg++;
	CalcStack();
	MakeCurrent();
	filename = strdup(fn);
}

Tprg::Tprg(const Tchar *fn, int in, int out, int minin, int minout, int loc, const Tsymbol* sympointer) : ol(0), linenums(0), nargin(in), nargout(out), nargin_min(minin), nargout_min(minout), nlocal(loc), ntemp(0), inputellipsis(0), outputellipsis(0), symptr(sympointer) {
	global::NTprg++;
	CalcStack();
	MakeCurrent();
	filename = strdup(fn);
}

Tprg::~Tprg() {
	global::NTprg--;
	Toperand::objLLptr=old_olptr;				// recover previous static object list
	// Unallocate static objects in static object list 'ol'
	TObjectPtr *p=ol.Ptr();
	for (int i=0; i<ol.length(); i++) {
		/*
		clog << "Deleting ";
		if (p[i])
			clog << *p[i];
		else
			clog << "(null)";
		clog << '\n';
		*/
		delete p[i];
	}
	if (filename) free(filename);	// filename was created with strdup
}

void Tprg::CalcStack() {
	nstack = nargin+nargout+nlocal+ntemp+(inputellipsis!=0)+(outputellipsis!=0) + 1;
}	// the +1 is the AnsVariable space

int Tprg::FindLineNumber(int startPC) const {
	int instruction_count = 0;
	int startingPC = startPC ? startPC : PC;
    int pc;
	for (pc=startingPC-1; pc>=0; pc--) if (il[pc].IsOpcode()) break;
    int i;
	for (i=0; i<il.length(); i++) {
		if (i >= pc) {
			if (instruction_count < linenums.length())
				return linenums[instruction_count];
			else
				return linenums[0];
		}
		if (il[i].IsOpcode()) {
			int N = il[i].Noperands();
			i+= N;
			instruction_count++;
		} else {
			cerr << "Internal error in FindLineNumber: Syntax error in instruction list\n";
			exit(1);
		}
	}
	return linenums[0];
}	

Tcode Tprg::LastOpcode() const {
	for (int i=length()-1; i>=0; i--)
		if (operator[](i).IsOpcode()) return operator[](i).opcode();
	return Klit;
}

ostream& operator<<(ostream& o, Tprg& prg) {
	//o << '\n';
	if (prg.symptr)
		o << "Disassembly of '" << *prg.symptr << "', ";
	o << "Source file: \"" << (char*)prg.filename << "\"\n";
	if (prg.HasInputEllipsis()) {
		o << prg.Nargin() << "+ input args, ";
	} else {
		if (prg.Nargin()==0)
			o << "no input args, ";
		else if (prg.Nargin()==1)
			o << "1 input arg, ";
		else
			o << prg.Nargin() << " input args, ";
	}
	if (prg.Nargout()==0)
		o << "no output args, ";
	else if (prg.Nargout()==1)
		o << "1 output arg, ";
	else
		o << prg.Nargout() << " output args, ";
	if (prg.Nlocal()==0)
		o << "no locals, ";
	else if (prg.Nlocal()==1)
		o << "1 local, ";
	else
		o << prg.Nlocal() << " locals, ";
	o << "stack frame size " << prg.StackFrameSize() << ".\n";
	o << "Maximum number of operands is " << prg.MaxNops() << ".";
	Toperand::objLLptr = &prg.ol;
	/*
	o << "[";
	for (int i=0; i<prg.ol.length(); i++) {
		o << *prg.ol[i];
		if (i<prg.ol.length()-1) o << ", ";
	}
	o << ']';
	*/
	//o << '\n' << prg.linenums;
	return o << prg.il;
}

static int ErrorMessageLineMatch(Tchar *s, int ret) {
	Tchar pattern[20];
	sprintf((char*)pattern,"%d:",ret);
	Tchar *startptr=strstr(s,pattern);
	if (!startptr) return 0;
	for (Tchar*p=s; p<startptr; p++) if (!isspace(*p)) return 0;
	return 1;
}

extern FILE *FindAndOpenFile(const Tchar* fn);   // defined in tela.C
extern void GenerateInterrupt();				// defined in tela.C

static void GiveErrorMessage(ostream& o, const TCFunctionInfo*p, int ret) {
	long help1 = p->helpstart;
	long help2 = p->helpend;
	if (help1 >= help2) {
		o << "Return code " << ret << " (no error message file).\n";
	} else {
		FILE *fp = FindAndOpenFile(p->helpfile);
		if (fp) {
			fseek(fp,help1,SEEK_SET);
			int StartLooking=0, found=0;
			while (ftell(fp) < help2) {
				Tchar s1[512+1];
				fgets((char*)s1,512,fp);
				if (StartLooking) {
					if (ErrorMessageLineMatch(s1,ret)) {
						Tchar *sptr = s1;
						for (int i=0; s1[i]; i++) if (s1[i]==':') sptr=s1+i+2;
						int L = strlen(s1);
						if (s1[L-1]=='\n') s1[L-1] = '\0';
						if (s1[L-3]=='*' && s1[L-2]=='/') s1[L-3] = '\0';
						o << sptr;
						found=1;
						break;
					}
				}
				if (!StartLooking) {if (strstr(s1,(const Tchar*)"Error codes:")) StartLooking=1;}
			}
			if (!found) o << "Return code " << ret << " (no verbal message for this code).\n";
			fclose(fp);
		} else {
			o << "Return code " << ret << " (cannot open error message file '" << p->helpfile << "').\n";
		}
	}
}

void Call(const Tobject& fn,
		  const TConstObjectPtr inputs[], const int Ninputs,
		  const TObjectPtr outputs[], const int Noutputs,
		  const Tsymbol* symptr,
		  bool check_that_first_output_not_obligatory)
// General call interface. Fn may be either T- or C-function.
// symptr should point to the Fn symbol (used for error messages).
{
	const Tkind k = fn.kind();
	if (k == Kfunction) {
		Tprg* p = fn.FunctionValue();
		if (check_that_first_output_not_obligatory) {
			if (p->NMinArgout() > 0) {
				cerr << "*** Warning: Calling function with obligatory output arg(s) with non-bracketed call form on line "
					 <<  p->FindLineNumber();
				if (p->symptr) cerr << " in " << *p->symptr;
				if (p->filename) cerr << " (" << (char*)p->filename << ")";
				cerr << ".\n";
			}
		}
		p->execute(inputs,Ninputs,outputs,Noutputs,symptr);
	} else if (k == KCfunction) {
		TCFunctionPtr p = fn.CFunctionPtr();
		const TCFunctionInfo* infop = fn.CFunctionInfoPtr();
		if (Ninputs < infop->minin) {
			err << "Too few (" << Ninputs << ") input args to " << infop->Cfname
				<< " (min=" << infop->minin << ")\n";
			error();
		}
		if (infop->maxin>=0) {
			if (Ninputs > infop->maxin) {
				err << "Too many input args ("<< Ninputs <<") to " << infop->Cfname
					<< " (max="<<infop->maxin<<")\n";
				error();
			}
		}
		if (Noutputs < infop->minout) {
			err << "Too few output args ("<< Noutputs <<") to " << infop->Cfname
				<< " (min="<<infop->minout<<")\n";
			error();
		}
		int NoutputsToPass = Noutputs;
		int make_outputs0_void = 0;
		if (infop->maxout>=0) {
			if (Noutputs == 1 && infop->maxout == 0) {
				NoutputsToPass = 0;
				make_outputs0_void = 1;
			} else if (Noutputs > infop->maxout) {
				err << "Too many output args ("<< Noutputs <<") to " << infop->Cfname
					<< " (max="<<infop->maxout<<")\n";
				error();
			}
		}
		const int ret = (*p)(inputs,Ninputs,outputs,NoutputsToPass);
		if (make_outputs0_void) outputs[0]->SetToVoid();
		if (ret > 0) {
			clog << "Warning from C-function '" << infop->Cfname << "':\n  ";
			GiveErrorMessage(clog,infop,ret);
			clog << ".\n" << flush;
		} else if (ret < 0) {
			err << "Fatal error from C-function '" << infop->Cfname << "':\n  ";
			GiveErrorMessage(err,infop,ret);
			err << ".\n";
			error();
		}
	} else {
		err << Tshort(fn) << " is not a function\n";
		error();
	}
}

#if SUPPORT_LOOP_TILING
INLINE int StudyTileArrays(const TObjectPtr d[], int N, int NIOs, Tint& array_size)
// If all objects d[0..N-1] are either numerical arrays or scalars
// and if all those that are arrays are of the same length and if
// that length is at least 1.5*TILE_BLOCKSIZE, return 1,
// and return array length in array_size. Otherwise return 0,
// and array_size remains unmodified.
// Also, if any of the first NIOs arguments is not-a-numerical-array, return 0.
{
	int i;
	int got_arraysize = 0;
	for (i=0; i<N; i++) {
		if (d[i]->IsNumericalArray()) {
			if (got_arraysize) {
				if (d[i]->length() != array_size) {
//					cout << "not tiling, array_size=" << array_size << ", L=" << d[i]->length() << "\n";
					return 0;
				}
			} else {
				array_size = d[i]->length();
				if (array_size < (3*TILE_BLOCKSIZE)/2) {
//					cout << "not tiling, too small\n";
					return 0;
				}
				got_arraysize = 1;
			}
		} else {
			if (i < NIOs) {
//				cout << "not tiling, non-array " << *d[i] << " gets modified\n";
				return 0;
			}
		}
	}
	return got_arraysize;
}
#endif

static TFixedHash *profhashptr = 0;

static void ProfilingRecord(const char *name, Treal cputime, Treal flops, Treal Ncalls)
{
	static int FirstTime = 1;
	if (FirstTime) {
		profhashptr = new TFixedHash(1113);
		FirstTime = 0;
	}
	const THashEntryPtr hashentryptr = profhashptr->add((const Tchar*)name);
	Tobject& obj = *hashentryptr->value();
	if (obj.kind() != KRealArray) obj.rzeros(3);		// components: cputime, flops, Ncalls
	obj.RealPtr()[0]+= cputime;
	obj.RealPtr()[1]+= flops;
	obj.RealPtr()[2]+= Ncalls;
}

static void ProfilingReport(ostream& o)
{
	const Treal dt = 30e-3;
	const int strwidth = 35;
	const int numwidth = 12;
	if (!profhashptr) return;
	THashEntryPtr p;
	int old_width = o.width();
	std::ios::fmtflags old_flags = o.flags();
	o << "             Detailed profiling report:\n";
	o.width(strwidth); o.setf(ios::left); o << " " << ' ';
	o.width(numwidth); o << "sec";
	o.width(numwidth); o << "ms/call";
	o << "Mflops\n";
	static char tmp_fn[MAXFILENAME] = "";
	tmpnam(tmp_fn);
	ofstream fp(tmp_fn);
	Treal totaltime=0, totalflops=0, totalcalls=0;
	for (p=profhashptr->first(); p; p=profhashptr->next()) {
		const Tobject& obj = *p->value();
		Treal t = obj.RealPtr()[0], f = obj.RealPtr()[1], c = obj.RealPtr()[2];
		totalflops+= f;
		totaltime+= t;
		totalcalls+= c;
		Treal Mflops = 1e-6*f/max(dt,t);
		if (t < dt) continue;
		fp.width(strwidth);
		fp.setf(ios::left);
		char name[80];
		strcpy(name,p->name());
		int i,slashpos = -1;
		const int L = strlen(name);
		for (i=L-1; i>=0; i--) if (name[i] == '/') {slashpos=i; break;}
		if (slashpos >= 0) {
			int colonpos = -1;
			for (i=slashpos+1; i<L; i++) if (name[i] == ':') {colonpos=i; break;}
			if (colonpos > slashpos) {
				memmove(&name[slashpos+1],&name[colonpos+1],L-colonpos);
			}
			name[slashpos] = ':';
		}
		fp << name;
		fp << ' ';
		fp.width(numwidth);
		fp << t;
		fp.width(numwidth);
		fp << 1e3*t/max(1.0,c);
		fp.width(numwidth);
		if (Mflops >= 0) fp << Mflops; else fp << "*";
		fp << "\n";
	}
	fp.close();
	char *s = new char [strlen(tmp_fn) + 80];
	sprintf(s,"sort -g -r -k 2 %s",tmp_fn);
	system(s);
	remove(tmp_fn);
	delete [] s;
	o.width(strwidth); o.setf(ios::left); o << "   Total :" << ' ';
	o.width(numwidth); o << totaltime;
	o.width(numwidth); o << "  ";
	o.width(numwidth); o << 1e-6*totalflops/max(dt,totaltime) << "\n";
	o.flags(old_flags);
	o.width(old_width);
}

static Treal CurrentFlops()
{
	const int L = int(Fnop) - int(Fadd) + 2;
	int i,c;
	Treal result = 0.0;
	for (i=1,c=Fadd; i<L; i++,c++) {
		result+= HPM[c].Nops;
	}
	return result;
}

INLINE void Tprg::executeInstructionSequence() {
	int i,oldPC,Nop;
	Toperand *f,*arg;
	TObjectPtr *d = new TObjectPtr [MaxNops()];
	static Tobject TheObject(0);
	static Tobject TheVoidObject;
	static int FirstTime = 1;
	if (FirstTime) {TheVoidObject.SetToVoid(); FirstTime=0;}
	Treal stack_cputime1,stack_cputime1cum,
		stack_flops1,stack_flops1cum,
		stack_Ncalls1,stack_Ncalls1cum;		// g++ can warn here about uninitialized variable, but it is safe, ignore it
	static Treal static_cputime1=0,static_cputime1cum=0;	// cum=cumulative, called func time included
	static Treal static_flops1=0,static_flops1cum=0;
	static Treal static_Ncalls1=0,static_Ncalls1cum=0;
	static Treal call_counter = 0;
	if (flags::profiling) {
		stack_cputime1 = static_cputime1;
		stack_cputime1cum = static_cputime1cum;
		stack_flops1 = static_flops1;
		stack_flops1cum = static_flops1cum;
		stack_Ncalls1 = static_Ncalls1;
		stack_Ncalls1cum = static_Ncalls1cum;
		static_cputime1 = static_cputime1cum = CPUSeconds();
		static_flops1 = static_flops1cum = CurrentFlops();
		static_Ncalls1 = static_Ncalls1cum = call_counter++;
	}
	for (PC=0; PC<il.length(); ) {
		oldPC = PC;
		f = &il[PC++];
#		if CHECK_INTERNAL_ERRORS
		if (!f->IsOpcode()) {err << "Internal: " << *f << " is not an opcode\n"; error();}
#		endif
		Nop = f->Noperands();
		// Load flatcode words (operands) beginning at PC to array d.
		// Nop is the number of operands to convert.
#		ifdef UNICOS
#		pragma _CRI align
#		endif
		for (i=0; i<Nop; i++) {
			arg = &il[PC++];
			// Replaced switch statement by successive if's /Pj 18.3.2001. Seems to be faster...
			// Notice that we test first those types that are likely to occur most often.
			const Tcode argkind = arg->kind();
			if (argkind == Kobj) {			// Local variables
				d[i] = ol[arg->offset()];
			} else if (argkind == Kpar) {	// Function parameters
				d[i] = theStack[arg->offset()];
			} else if (argkind == Ksym) {	// Global variables
				d[i] = arg->symbol().value();
			} else if (argkind == Klit) {	// Literal constants
				TheObject = arg->literal();
				d[i] = &TheObject;
			} else /* if (argkind == Kallrange)*/ {	// VOID objects
				d[i] = &TheVoidObject;
			}
#if 0
			switch (arg->kind()) {
			    case Kpar:
					d[i] = theStack[arg->offset()];
					break;
			    case Kobj:
					d[i] = ol[arg->offset()];
					break;
			    case Ksym:
					d[i] = arg->symbol().value();
					break;
			    case Klit:
					TheObject = arg->literal();
					d[i] = &TheObject;
					break;
			    case Kallrange:
                                // Denote ':' object by just a void
                                // object. This is bad practice but we
                                // want to avoid introducing more and
                                // more Tobject kinds, because they
                                // grow the switch statements in
                                // object.C in both source and object
                                // code.
					d[i] = &TheVoidObject;
					break;
			    default:
					err << "Internal error in loadoperands: "
						<< *arg << " should not be operand\n";
					error();
			}
#endif
		}
		
		global::nops = 0;
		
#		define pair(opcode,Nargs) EncodeOpcode(opcode,Nargs)
		
		switch (f->rawcode()) {
			
		case pair(Fadd,2):
			Add(*d[0],*d[1]); break;
		case pair(Fadd,3):
			Add(*d[0],*d[1],*d[2]); break;
			
		case pair(Finc,1):
			Inc(*d[0]);
			break;
			
		case pair(Fsub,2):
			Sub(*d[0],*d[1]); break;
		case pair(Fsub,3):
			Sub(*d[0],*d[1],*d[2]); break;
			
		case pair(Fdec,1):
			Dec(*d[0]);
			break;
			
		case pair(Fmul,2):
			Mul(*d[0],*d[1]); break;
		case pair(Fmul,3):
			Mul(*d[0],*d[1],*d[2]); break;
			
		case pair(Fdiv,2):
			Div(*d[0],*d[1]); break;
		case pair(Fdiv,3):
			Div(*d[0],*d[1],*d[2]); break;

		case pair(Fmod,3):
			Mod(*d[0],*d[1],*d[2]); break;
			
		case pair(Fpow,2):
			Pow(*d[0],*d[1]); break;
		case pair(Fpow,3):
			Pow(*d[0],*d[1],*d[2]); break;
			
		case pair(Fneg,1):
			Neg(*d[0]); break;
		case pair(Fneg,2):
			Neg(*d[0],*d[1]); break;
		case pair(Fabs,1):
			Abs(*d[0]); break;
		case pair(Fabs,2):
			Abs(*d[0],*d[1]); break;
			
		case pair(Fmove,2):
			if (d[1]->kind()==Kundef) {
				err << "Assigning undefined value " << Tshort(*d[1]) << '\n';
				error();
			}
			if (d[0] != d[1])
				*d[0] = *d[1];
			break;
			
		case pair(Fgt,3):
			Gt(*d[0],*d[1],*d[2]); break;
		case pair(Flt,3):
			Lt(*d[0],*d[1],*d[2]); break;
		case pair(Fge,3):
			Ge(*d[0],*d[1],*d[2]); break;
		case pair(Fle,3):
			Le(*d[0],*d[1],*d[2]); break;
		case pair(Feq,3):
			Eq(*d[0],*d[1],*d[2]); break;
		case pair(Fne,3):
			Ne(*d[0],*d[1],*d[2]); break;

		case pair(Fjngt,3):
			if (!TestGt(*d[1],*d[2]))  {
#				if CHECK_INTERNAL_ERRORS
				if (d[0]->kind() != Kint) {
					err << "Internal: illegal JGT offset: " << Tshort(*d[0]) << '\n';
					error();
				}
#				endif
				PC = d[0]->IntValue();
			}
			break;
		case pair(Fjnlt,3):
			if (!TestLt(*d[1],*d[2]))  {
#				if CHECK_INTERNAL_ERRORS
				if (d[0]->kind() != Kint) {
					err << "Internal: illegal JLT offset: " << Tshort(*d[0]) << '\n';
					error();
				}
#				endif
				PC = d[0]->IntValue();
			}
			break;
		case pair(Fjnge,3):
			if (!TestGe(*d[1],*d[2]))  {
#				if CHECK_INTERNAL_ERRORS
				if (d[0]->kind() != Kint) {
					err << "Internal: illegal JGE offset: " << Tshort(*d[0]) << '\n';
					error();
				}
#				endif
				PC = d[0]->IntValue();
			}
			break;
		case pair(Fjnle,3):
			if (!TestLe(*d[1],*d[2]))  {
#				if CHECK_INTERNAL_ERRORS
				if (d[0]->kind() != Kint) {
					err << "Internal: illegal JLE offset: " << Tshort(*d[0]) << '\n';
					error();
				}
#				endif
				PC = d[0]->IntValue();
			}
			break;

		case pair(Fand,3):
			And(*d[0],*d[1],*d[2]); break;
		case pair(Fand,2):
			And(*d[0],*d[0],*d[1]); break;
		case pair(For,3):
			Or(*d[0],*d[1],*d[2]); break;
		case pair(For,2):
			Or(*d[0],*d[0],*d[1]); break;
		case pair(Fnot,2):
			Not(*d[0],*d[1]); break;
		case pair(Fnot,1):
			Not(*d[0],*d[0]); break;

		case pair(Fmin,3):
			Min(*d[0],*d[1],*d[2]); break;
		case pair(Fmax,3):
			Max(*d[0],*d[1],*d[2]); break;
		case pair(Fmin,2):
			Min(*d[0],*d[1]); break;
		case pair(Fmax,2):
			Max(*d[0],*d[1]); break;

		case pair(Fnin,1):
			*d[0] = thePCstack.top(3); break;
		case pair(Fnout,1):
			*d[0] = thePCstack.top(2); break;
		case pair(Fmmpos,1):
			*d[0] = MinMaxPosition(); break;

		case pair(Fgetin,2):
			{
				if (d[1]->kind() != Kint) {
					err << "In argin(n), n must be integer scalar.\n";
					error();
				}
				Tint n = d[1]->IntValue() - ArrayBase;
				if (n < 0) {
					err << "In argin(n), n must not be smaller than " << ArrayBase << ". Was " << n << ".\n";
					error();
				}
				if (!HasInputEllipsis()) {
					err << "argin() may only be used in functions with input ellipsis (...)\n";
					error();
				}
				Tint maxn = theStack[Nargin()+1]->ObjectPtrPtr()[0]->IntValue();
				if (n >= maxn) {
					err << "argin(" << n+ArrayBase << ") called, while the function was called with only "
						<< maxn << " optional arg";
					if (maxn>1) err << "s";
					err << ".\n";
					error();
				}
				*d[0] = *(theStack[Nargin()+1]->ObjectPtrPtr()[n+1]);
			}
			break;
		case pair(Fgetout,2):
			{
				int i = Nargin();
				if (HasInputEllipsis()) i++;
				i+= Nargout()+1;
				if (d[1]->kind() != Kint) {
					err << "In argout(n), n must be integer scalar.\n";
					error();
				}
				Tint n = d[1]->IntValue() - ArrayBase;
				if (n < 0) {
					err << "In argout(n), n must not be smaller than " << ArrayBase << ". Was " << n << ".\n";
					error();
				}
				if (!HasOutputEllipsis()) {
					err << "argout() may only be used in functions with output ellipsis [...]\n";
					error();
				}
				Tint maxn = theStack[i]->ObjectPtrPtr()[0]->IntValue();
				if (n >= maxn) {
					err << "argout(" << n+ArrayBase << ") called, while the function was called with only "
						<< maxn << " optional output arg";
					if (maxn>1) err << "s";
					err << ".\n";
					error();
				}
				*d[0] = *(theStack[i]->ObjectPtrPtr()[n+1]);
			}
			break;
		case pair(Fsetout,2):
			{
				int i = Nargin();
				if (HasInputEllipsis()) i++;
				i+= Nargout()+1;
				if (d[0]->kind() != Kint) {
					err << "In SetArgOut(n,x), n must be integer scalar.\n";
					error();
				}
				Tint n = d[0]->IntValue() - ArrayBase;
				if (n < 0) {
					err << "In SetArgOut(n,x), n must not be smaller than " << ArrayBase << ". Was " << n << ".\n";
					error();
				}
				if (!HasOutputEllipsis()) {
					err << "SetArgOut() may only be used in functions with output ellipsis [...]\n";
					error();
				}
				Tint maxn = theStack[i]->ObjectPtrPtr()[0]->IntValue();
				if (n >= maxn) {
					err << "SetArgOut(" << n+ArrayBase << ",x) called, while the function was called with only "
						<< maxn << " optional output arg";
					if (maxn>1) err << "s";
					err << ".\n";
					error();
				}
				*(theStack[i]->ObjectPtrPtr()[n+1]) = *d[1];
			}
			break;
//		case pair(Fbra,1):
//#			if CHECK_INTERNAL_ERRORS
//			if (d[0]->kind() != Kint) {
//				err << "Internal: illegal branch offset: " << Tshort(*d[0]) << '\n';
//				error();
//			}
//#			endif
//			PC+= d[0]->IntValue() - 1;
//			break;
		case pair(Fjmp,1):
#			if CHECK_INTERNAL_ERRORS
			if (d[0]->kind() != Kint) {
				err << "Internal: illegal jump offset: " << Tshort(*d[0]) << '\n';
				error();
			}
#			endif
			PC = d[0]->IntValue();
			break;
		case pair(Fincjmp,2):
#			if CHECK_INTERNAL_ERRORS
			if (d[0]->kind() != Kint) {
				err << "Internal: illegal incjump offset: " << Tshort(*d[0]) << '\n';
				error();
			}
#			endif
			Inc(*d[1]);
			PC = d[0]->IntValue();
			break;
//		case pair(Fbif,2):
//#			if CHECK_INTERNAL_ERRORS
//			if (d[0]->kind() != Kint) {
//				err << "Internal: illegal BIF offset: " << Tshort(*d[0]) << '\n';
//				error();
//			}
//#			endif
//			if (d[1]->IsNonzero()) PC+= d[0]->IntValue()-1;
//			break;
//		case pair(Fbifn,2):
//#			if CHECK_INTERNAL_ERRORS
//			if (d[0]->kind() != Kint) {
//				err << "Internal: illegal BIFN offset: " << Tshort(*d[0]) << '\n';
//				error();
//			}
//#			endif
//			if (!d[1]->IsNonzero()) PC+= d[0]->IntValue()-1;
//			break;
		case pair(Fjif,2):
#			if CHECK_INTERNAL_ERRORS
			if (d[0]->kind() != Kint) {
				err << "Internal: illegal JIF offset: " << Tshort(*d[0]) << '\n';
				error();
			}
#			endif
			if (d[1]->IsNonzero()) PC = d[0]->IntValue();
			break;
		case pair(Fjifn,2):
#			if CHECK_INTERNAL_ERRORS
			if (d[0]->kind() != Kint) {
				err << "Internal: illegal JIFN offset: " << Tshort(*d[0]) << '\n';
				error();
			}
#			endif
			if (!d[1]->IsNonzero()) PC = d[0]->IntValue();
			break;
			
		case pair(Fpri,1):
			if (d[0]->kind()!=Kvoid) cout << *d[0] << '\n';	// avoid printing empty line in case of void object
			break;
			
		case pair(Fnop,0):
			break;
			
		case pair(Fgath,0):
			{
				TConstObjectPtr indices[MAX_NOPERANDS];
				for (i=2; i<Nop; i++)
					indices[i-2] = d[i];
				Gather(*d[0],*d[1],indices,Nop-2);
			}
			break;
		case pair(Fscat,0):
			{
				TConstObjectPtr indices[MAX_NOPERANDS];
				for (i=2; i<Nop; i++)
					indices[i-2] = d[i];
				Scatter(*d[0],*d[1],indices,Nop-2);
			}
			break;

		case pair(Fmgath,0):
		case pair(Fmscat,0):
			MGatScat(*d[0],*d[1],d+2,Nop-2,f->opcode()==Fmscat);
			break;
		case pair(Frange,0):
			Range(d);
			break;
		case pair(Farray,0):	// ARRAY result,arg1,arg2,...,argN
			{
				TConstObjectPtr args[MAX_NOPERANDS];
				for (i=1; i<Nop; i++)
					args[i-1] = d[i];
				BuildArray(*d[0],args,Nop-1);
			}
			break;
		case pair(Fappend,0):	// APPEND result,arg1,arg2,...,argN
			{
				TConstObjectPtr args[MAX_NOPERANDS];
				for (i=1; i<Nop; i++)
					args[i-1] = d[i];
				PasteArrays(*d[0],args,Nop-1);
			}
			break;
		case pair(Fcall,0):		// CALL fn, Noutputs, out1,out2,...outN, in1,in2,...,inM
		case pair(Fscall,0):	// almost same as CALL but checks that first output arg is not obligatory
			{
				const Tkind k = d[0]->kind();
#if 0
				if (d[0]->IsNumerical()) {
					cout << *d[0] << '\n' << flush;
					break;
				}
#endif
#				if CHECK_INTERNAL_ERRORS
				if (d[1]->kind()!=Kint) {err<<"Internal: 2nd operand of CALL must be integer\n"; error();}
#				endif
				const int Noutputs = d[1]->IntValue();
				const int Ninputs = Nop - Noutputs - 2;
				const Tsymbol* symptr = 0;
				if (k == Kfunction) {
					if (il[oldPC+1].kind() == Ksym) symptr = &(il[oldPC+1].symbol());
				}
				Call(*d[0],d+2+Noutputs,Ninputs,d+2,Noutputs,symptr,f->rawcode() == pair(Fscall,0));
			}
			break;
		case pair(Fizeros,0):
		case pair(Frzeros,0):
		case pair(Fczeros,0):
		case pair(Fozeros,0):
			{
				Tint indices[MAX_NOPERANDS];
				int ndims;
				// Accept also one integer vector argument a'la zeros(size(a))
				if (Nop == 2 && d[1]->kind()==KIntArray) {
					if (d[1]->rank() != 1) {
						err << "Non-vector integer argument " << Tshort(*d[1]) << " to ZEROS\n";
						error();
					}
					if (d[1]->length() > MAXRANK) {
						err << "More than MAXRANK=" << MAXRANK << " elements in vector arg " << Tshort(*d[1]) << " to ZEROS\n";
						error();
					}
					for (i=0; i<d[1]->length(); i++) {
						indices[i] = d[1]->IntPtr()[i];
						if (indices[i] < 0) {
							err << "Negative dimension " << indices[i] << " in ZEROS\n";
							error();
						}
					}
					ndims = d[1]->length();
				} else {
					if (Nop-1 > MAXRANK) {
						err << "More than MAXRANK=" << MAXRANK << " arguments to ZEROS\n";
						error();
					}
					for (i=1; i<Nop; i++) {
						if (d[i]->kind()!=Kint || d[i]->IntValue() < 0) {
							err << "Operands of ZEROS must be nonnegative integers, not " << Tshort(*d[i]) << "\n";
							error();
						}
						indices[i-1] = d[i]->IntValue();
					}
					ndims = Nop-1;
				}
				void (Tobject::*fn)(const TDimPack&);
				switch (f->opcode()) {
				case Fizeros: fn = &Tobject::izeros; break;
				case Frzeros: fn = &Tobject::rzeros; break;
				case Fczeros: fn = &Tobject::czeros; break;
				default :     fn = &Tobject::voids; break;	// this could as well be case Fozeros
				}
				(d[0]->*fn)(TDimPack(indices,ndims));
			}
			break;
#		if SUPPORT_LOOP_TILING
		case pair(Fvloop,0):
		{
#			if CHECK_INTERNAL_ERRORS
			if (d[0]->kind()!=Kint) {err<<"Internal: 1st operand of VLOOP must be integer\n"; error();}
#			endif
			if (global::tile_in_use) {
				err << "Nested VLOOP instruction\n";
				error();
			}
			Tint array_size;
			const int all_arrays_of_same_size = StudyTileArrays(d+1,Nop-1,d[0]->IntValue(),array_size);
			if (all_arrays_of_same_size) {
				global::tile_in_use = 1;
				global::tile_start = 0;
				global::tile_end = ((TILE_BLOCKSIZE < array_size) ? TILE_BLOCKSIZE : array_size) - 1;
				global::tile_i = 0;
				global::tile_Ntiles = (array_size+TILE_BLOCKSIZE-1)/TILE_BLOCKSIZE;
				global::tile_arraylength = array_size;
//				cout << "Executing VLOOP, tile=" << global::tile_start << ".." << global::tile_end << "\n";
			} /*else
				cout << "Passing through VLOOP\n";*/
		}
		break;
		case pair(Fendv,1):
		{
			if (global::tile_in_use) {
				if (++global::tile_i < global::tile_Ntiles) {
					global::tile_start+= TILE_BLOCKSIZE;
					global::tile_end+= TILE_BLOCKSIZE;
					if (global::tile_end >= global::tile_arraylength) global::tile_end = global::tile_arraylength-1;
//					cout << "Executing ENDV, tile=" << global::tile_start << ".." << global::tile_end << "\n";
					// Branch to beginning of tile
#					if CHECK_INTERNAL_ERRORS
					if (d[0]->kind() != Kint) {
						err << "Internal: illegal branch offset in ENDV: " << Tshort(*d[0]) << '\n';
						error();
					}
#					endif
					PC+= d[0]->IntValue() - 2;
				} else {
					// Last tile executed, exit tile mode
					global::tile_in_use = 0;
				}
			} /*else
				cout << "Passing through ENDV\n";*/
		}
		break;
#		endif		/* SUPPORT_LOOP_TILING */
#		if CHECK_INTERNAL_ERRORS			
		default:
			err << "Internal: illegal instruction\n";
			error();
#		endif
		}
		
#		undef pair	

		Thpm& hpmref = HPM[f->opcode()];
		hpmref.Ninstr+= 1;				// record that we issued this-and-this instruction
		hpmref.Nops+= global::nops;		// record number of operations involved in instruction

		if (global::interrupted) {
			if (global::debugging) {
				SetControlGInterruptHandler();
     			global::interrupted = 0;
				cout << "User break, entering debug shell, type cont to continue:\n" << flush;
				extern void InterpreterLoop(int ContWillExit=0);	// from tela.C
				InterpreterLoop(1);
     			global::debugging = 0;
			} else
				break /*GenerateInterrupt()*/;
		}

	}	// end of the big for loop

	delete [] d;
	if (flags::profiling) {
		const Treal cputime2 = CPUSeconds();
		const Treal flops2 = CurrentFlops();
		const int maxfilename = 39;
		const int maxfuncname = 39;
		static char s[maxfilename+maxfuncname+2];
		const char *const funcname = (symptr ? (char*)*symptr : "TOPLEVEL");
		const int Lf = strlen((char*)filename);
		const int L = strlen(funcname) + 1 + Lf;
		memset(s,0,maxfilename+maxfuncname+2);
		int leaveout = 0;
		if (Lf > maxfilename) leaveout = Lf-maxfilename;
		strncpy(s,((char*)filename)+leaveout,maxfilename); s[maxfilename] = '\0';
		strcat(s,"/");
		strncat(s,funcname,maxfuncname);
		const Treal cputime = cputime2 - static_cputime1;
		const Treal flops = flops2 - static_flops1;
		const Treal Ncalls = call_counter - static_Ncalls1;
		ProfilingRecord(s,cputime,flops,Ncalls);
		const Treal cputime_cum = cputime2 - static_cputime1cum;
		const Treal flops_cum = flops2 - static_flops1cum;
		const Treal Ncalls_cum = call_counter - static_Ncalls1cum;
		static_cputime1 = stack_cputime1 + cputime_cum;
		static_cputime1cum = stack_cputime1cum;
		static_flops1 = stack_flops1 + flops_cum;
		static_flops1cum = stack_flops1cum;
		static_Ncalls1 = stack_Ncalls1 + Ncalls_cum;
		static_Ncalls1cum = stack_Ncalls1cum;
	}
} // executeInstructionSequence


void Tprg::execute
	(const TConstObjectPtr argin[], int Nin, const TObjectPtr argout[], int Nout, const Tsymbol *fnsymptr)
{
	// Execute function with given input and output arguments lists.
	// Algorithm: Check that Nin,Nout do not exceed Nargin(), Nargout().
	// Check that Nin,Nout are no less than NMinArgin(), NMinArgout().
	// Set optional output args to Undefined (Added 24 Apr 1999) (REMOVED, CAUSES PROBLEMS)
	// Set up stack frame, and put argument lists on theStack.
	// Push Nin, Nout to thePCstack.
	// Push PC on thePCstack.
	// fnsymptr must point to the function symbol; it is used in error message.
	// Execute the instructions. Finally deallocate theStack and thePCstack.

	int i,j,k;
	int Nargin1 = Nargin();
	int Nargout1 = Nargout();
	if (HasInputEllipsis()) {
		Nargin1++;
	} else {
		// Check that there are not too many input arguments
		if (Nin > Nargin()) {
			err << "Too many (" << Nin << ") input arguments ";
			if (fnsymptr) err << "for " << *fnsymptr;
			err << ", max is " << Nargin() << '\n';
			error();
		}
	}
	// Check that there are not too few input arguments
	if (Nin < NMinArgin()) {
		err << "Too few (" << Nin << ") input arguments ";
		if (fnsymptr) err << "for " << *fnsymptr;
		err << ", min is " << NMinArgin() << '\n';
		error();
	}
	// Handle special case of one actual output arg and no formal output args
	// (this case is accepted, and the actual arg is set to void)
	int SingleVoidOutArg = 0;
	if (HasOutputEllipsis()) {
		Nargout1++;
	} else {
		if (Nout==1 && Nargout()==0) {
			SingleVoidOutArg = 1;
			Nout = 0;
		}
		// Check that there are not too many output arguments
		if (Nout > Nargout()) {
			err << "Too many (" << Nout << ") output arguments ";
			if (fnsymptr) err << "for " << *fnsymptr;
			err << ", max is " << Nargout() << '\n';
			error();
		}
	}
	// Check that there are not too few output arguments
	if (Nout < NMinArgout()) {
		err << "Too few (" << Nout << ") output arguments ";
		if (fnsymptr) err << "for " << *fnsymptr;
		err << ", min is " << NMinArgout() << '\n';
		error();
	}
	// Set optional output args to Undefined (Added 24 Apr 1999)
//	for (i=NMinArgout(); i<Nout; i++) argout[i]->SetToUndefined();
	// Reserve enough space from stack:
	theStack.push(StackFrameSize());
	// Allocate all new objects at once
	const int Nnewobjects = StackFrameSize()-min(Nout,Nargout())-min(Nin,Nargin());
	TObjectPtr newobjects = new Tobject [Nnewobjects];
	// Bottom of the stack contains the AnsVariable
	theStack[0] = newobjects;
	// Then follow input args:
	const int NRegularIn = min(Nin,Nargin());
	for (i=0; i<NRegularIn; i++) theStack[i+1] = TObjectPtr(argin[i]);
	// Then follow unused input args:
	for (i=NRegularIn,k=0; i<Nargin1; i++,k++) theStack[i+1] = newobjects+1+k;
	// If we have input ellipsis, the last input arg is a shallow object array.
	// Then we create the oarray and store its length as the first element
	// (which is 0 if no objects).
	TObjectPtr toBeDeleted1 = 0, toBeDeleted2 = 0;
	if (HasInputEllipsis()) {
		const int Nexcess = max(Nin-Nargin(),0);
		theStack[Nargin1]->ozeros_shallow(Nexcess+1);
		theStack[Nargin1]->ObjectPtrPtr()[0] = toBeDeleted1 = new Tobject(Nexcess);	// store length of oarray
		for (i=0; i<Nexcess; i++)
			theStack[Nargin1]->ObjectPtrPtr()[i+1] = TObjectPtr(argin[Nargin()+i]);
		thePCstack.push(Nexcess);
	} else
		thePCstack.push(0);		// No optional input args
	// Then, output args:
	const int NRegularOut = min(Nout,Nargout());
	for (i=Nargin1,j=0; i<Nargin1+NRegularOut; i++,j++) theStack[i+1] = argout[j];
	// Unused output args and local objects:
	for (i=Nargin1+NRegularOut; i<StackFrameSize()-1; i++,k++) theStack[i+1] = newobjects+1+k;
	// If we have output ellipsis, the last output arg is an object array.
	// Then we create the oarray and store its length as the first element
	// (which is 0 if no objects).
	if (HasOutputEllipsis()) {
		const int Nexcess = max(Nout-Nargout(),0);
		theStack[Nargin1+Nargout1]->ozeros_shallow(Nexcess+1);
		theStack[Nargin1+Nargout1]->ObjectPtrPtr()[0] = toBeDeleted2 = new Tobject(Nexcess);	// store length of oarray
		for (i=0; i<Nexcess; i++)
			theStack[Nargin1+Nargout1]->ObjectPtrPtr()[i+1] = argout[Nargout()+i];
		thePCstack.push(Nexcess);
	} else
		thePCstack.push(0);		// No optional output args
	// Push program counter on the PC stack
	thePCstack.push(PC);
	// Push pointer to this Tprg on its stack
	theCodeStack.push(this);
	executeInstructionSequence();
	// Pop codestack
	theCodeStack.pop();
	// Pop program counter
	PC = thePCstack.pop();
	// Pop Nin and Nout from the PC stack
	thePCstack.pop();
	thePCstack.pop();
	// Delete unused output args, unused input args, and local objects
	delete [] newobjects;
	// Delete other to-be-deleted objects created by ellipsis brances.
	// These can be NULL, but 'delete NULL' is harmless.
	delete toBeDeleted1;
	delete toBeDeleted2;
	// Correct stack:
	theStack.pop(StackFrameSize());
	// If there was one actual output arg but no formal ones, set it to void
	if (SingleVoidOutArg) argout[0]->SetToVoid();
	// Zero global operation counter to prevent last instruction ops to be counted twice
	global::nops = 0;
}

void Tprg::checkreadonlyoperand(const Toperand& op, int pc) const
{
	//cerr << "---checkreadonlyoperand " << op << "\n";
	if (op.kind() != Kpar) return;
	if (1 <= op.offset() && op.offset() <= nargin) {
		cerr << "*** Warning: Modifying input argument at line " << FindLineNumber(pc);
		if (symptr) cerr << " in " << *symptr;
		if (filename) cerr << " (" << (char*)filename << ")";
		cerr << ".\n";
	}
}

void Tprg::checkreadonlyvars() const
/* Check that input args (numbered 1..nargin) are not modified.
   Various flatcode instructions fall in four categories:
   1. Jump, branch, PRI and NOP instructions, except INCJMP.
      These do not modify any argument.
   2. INCJMP, SCAT and MSCAT. These modify only the second argument.
   3. CALL. This is special.
   4. All other instructions always modify their first argument.
 */
{
	if (!flags::checkRO) return;	// do nothing if the flag is off
	for (int i=0; i<il.length(); i++) {
		if (il[i].IsOpcode()) {
			const int N = il[i].Noperands();
			const Tcode c = il[i].opcode();
			//cerr << "--checking " << il[i] << "(" << N << " operands) ..\n";
			if (c==Fjngt || c==Fjnlt || c==Fjnge || c==Fjnle ||
				/*c==Fbra  ||*/ c==Fjmp  /*|| c==Fbif  || c==Fbifn*/ ||
				c==Fjif  || c==Fjifn || c==Fpri  || c==Fnop) {
				;	// do nothing, these instruction don't modify any operand
			} else if (c==Fincjmp || c==Fscat || c==Fmscat) {
				checkreadonlyoperand(il[i+2],i);
			} else if (c==Fcall || c==Fscall) {
#				if CHECK_INTERNAL_ERRORS
				if (il[i+2].kind()!=Klit) {
					err << "Internal error in checkreadonlyvars: 2nd arg of CALL is not literal int\n";
					error();
				}
#				endif
				const Tint noutputs = il[i+2].literal();
				for (int j=3; j<3+noutputs; j++) checkreadonlyoperand(il[i+j],i);
			} else {	// Else check that first arg is not read-only
				checkreadonlyoperand(il[i+1],i);
			}
			i+= N;
		} else {
			err << "Internal error in checkreadonlyvars: syntax error in instruction list\n";
			error();
		}
	}
}	

// TCodeStack members

void TCodeStack::push(Tprg* ptr) {
	TCodeStackNode* n = new TCodeStackNode;
	n->next = p;
	n->prgptr = ptr;
	p = n;
}

Tprg* TCodeStack::top() const {
	if (!p) {cerr << "Internal: TCodeStack underflow\n"; exit(1);}
	return p->prgptr;
}

Tprg* TCodeStack::pop() {
	if (!p) {cerr << "Internal:: TCodeStack underflow\n"; exit(1);}
	Tprg *result = p->prgptr;
	TCodeStackNode* n = p;
	p = p->next;
	delete n;
	return result;
}

void PerformanceReport(int longflag)
{
	int k;
	Treal t=CPUSeconds();
	Treal totalinstr=0,totalops=0;
	if (longflag) {
		cout << "INSTR\tNissued\tNops\n";
		for (k=Fadd; k<=Fnop; k++) {
			cout << instrinfo[k].mnemonic << '\t' << HPM[k].Ninstr << '\t' << HPM[k].Nops << '\n';
		}
	}
	for (k=Fadd; k<=Fnop; k++) {
		totalinstr+= HPM[k].Ninstr;
		if (instrinfo[k].isflop) totalops+= HPM[k].Nops;
	}
	cout << totalinstr << " instructions, " << totalops << " arithmetic ops.\n";
	if (t > 0) cout << 1E-6*totalinstr/t << " MIPS, " << 1E-6*totalops/t << " MFLOPS.\n";
	if (flags::profiling) ProfilingReport(cout);
}

