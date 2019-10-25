/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2000 Pekka Janhunen
 */

//#define GLOBAL_TEMPORARIES
// if GLOBAL_TEMPORARIES is defined, temporary variables needed in expression evaluation
// are global, not allocated from the stack. The resulting code may then be incorrect
// if a function calls another function.

#define MAXGOTO 100
// MAXGOTO is the maximum number of label statements
// and the maximum number of unsatisfied goto statements
// in one function. The temporary memory overhead is 8*MAXGOTO bytes
// per one Tcompilation.

#include "lisp.H"
#include "prg.H"
#include "common.H"

#define COMPTYPE Tstring
#define DEFAULT_COMPVALUE Tstring()
#define LINEARLIST TStringLL
#include "templ/tLL.H"
#include "templ/tLL.C"
#undef LINEARLIST
#undef DEFAULT_COMPVALUE
#undef COMPTYPE

static TStringLL thePackageNames = 0;
extern void Ctgen(ostream& o, Tprg& prg);

#ifndef MAXINT
#  define MAXINT (~(1 << (8*sizeof(int)-1)))
#endif

class TBreakPoint {
 private:
	Tint addr;
	Tint lev;
 public:
	TBreakPoint() : addr(-1), lev(-1) {}
	TBreakPoint(Tint addr1, Tint level1) : addr(addr1), lev(level1) {}
	Tint address() const {return addr;}
	Tint level() const {return lev;}
};

class TBreakPointList {
 private:
	TIntLL addrs;
	TIntLL levels;
 public:
	TBreakPointList() : addrs(0), levels(0) {}
	int length() const {return addrs.length();}
	TBreakPoint operator[](int i) {return TBreakPoint(addrs[i],levels[i]);}
	Tint& address(int i) {return addrs[i];}
	Tint& level(int i) {return levels[i];}
	void append(const TBreakPoint& bp);
};

void TBreakPointList::append(const TBreakPoint& bp) {
	addrs.append(bp.address());
	levels.append(bp.level());
};

class TGotoList {
private:
	Tsymbol *labels[MAXGOTO];
	Tint addrs[MAXGOTO];
	int N;
	void error();
public:
	TGotoList() : N(0) {}
	int length() const {return N;}
	void add(Tsymbol*label, Tint addr);
	Tint getaddress(int i) const {return addrs[i];}
	Tsymbol *getlabel(int i) const {return labels[i];}
	int find(Tsymbol*label) const;
	void remove(int i);
	friend ostream& operator<<(ostream& o, const TGotoList L);
	~TGotoList() {}
};

void TGotoList::error() {
	err << "Too many (>" << MAXGOTO << ") label and/or goto statements in a function\n";
	::error();
}

void TGotoList::add(Tsymbol*label, Tint addr) {
	if (N > MAXGOTO-1) error();
	labels[N] = label;
	addrs[N++] = addr;
}

int TGotoList::find(Tsymbol*label) const {
	for (int i=0; i<N; i++)
		if (labels[i] == label) return i;
	return -1;
}

void TGotoList::remove(int i) {
	// no error checking
	for (int j=i; j<N-1; j++) {
		labels[j] = labels[j+1];
		addrs[j] = addrs[j+1];
	}
	N--;
}

ostream& operator<<(ostream& o, const TGotoList L) {
	o << "*** outputting TGotoList, N=" << L.N << "\n" << flush;
	for (int i=0; i<L.N; i++) {
		o << L.labels[i]->name();
		if (i<L.N-1) o << ", ";
	}
	return o;
}

class Tcompilation {
 private:
	int unique_index;
	Tnode* AnsVariable;
	int LoopLevel;
	TBreakPointList ContinueList, BreakList;
	TGotoList LabelList;

        Tcompilation(const Tcompilation&); // do not implement
        Tcompilation& operator=(const Tcompilation&); // do not implement
 public:
	TGotoList GotoList;
	Tcompilation() :
		unique_index(0),
		AnsVariable(new Tnode(0,Tslot(0),0)),
		LoopLevel(0),
		ContinueList(), BreakList(),
		LabelList()
		{}
	Tnode* MakeUnique(Tprg& prg);
	Tnode* CompileExpr1(Tnode*y, Tnode*e, Tprg& prg);
	void CompileCall(const Toperand& fn,
					 Tnode*const inputs[], int Ninputs,
					 Tnode*const outputs[], int Noutputs, Tprg& prg, bool generate_scall);
	void codegenSimpleExpr(Tnode*y, Tnode*e, Tprg& prg);
	void CompileExpr(Tnode*to, Tnode*auxvar, Tnode*from, Tprg& prg);
	void FixJumps(Tprg& prg, TBreakPointList& jumplist);
	void codegen(Tnode*x, Tprg& prg, int printsetflag=0);
	~Tcompilation() {delete AnsVariable;}
};

Tnode *gl_inputs[MAX_NOPERANDS], *gl_outputs[MAX_NOPERANDS];

#ifdef GLOBAL_TEMPORARIES /* currently unnecessary since stdio.h is included anyway */
#  include <stdio.h> // for sprintf
#endif

Tnode* Tcompilation::MakeUnique(Tprg& prg) {
	unique_index++;
#ifdef GLOBAL_TEMPORARIES	
	char s[80];
	sprintf(s,"_t%d",unique_index);
	Tsymbol *sym = theHT.add(s);
	return new Tnode(0,sym,0);
#else
	prg.AllocTemporaries(unique_index);
	int first_temp_index = prg.PermanentFrameSize();
	return new Tnode(0,Tslot(unique_index-1+first_temp_index),0);
#endif
}

Tnode* Tcompilation::CompileExpr1(Tnode*y, Tnode*e, Tprg& prg) {
	//clog << "CompileExpr1(" << *y << "," << *e << ")\n";
	if (Atom(e)) return List(List(y,e));
	Tfunc f = Head(e);
	Tnode* a = First(e);
	int len = Length(e);
	//clog << "Length = " << len << '\n';
	if (len == 1) {
		if (Atom(a))
			return List(List(y,e));
		else
			return Append(CompileExpr1(y,a,prg),List(List(y,List(f,y))));
	} else if (len == 2) {
		Tnode* b = Second(e);
		if (Atom(a) && Atom(b))		// both args atoms
			return List(List(y,e));
		else if (Atom(a))			// a atom, b expr
			return Append(CompileExpr1(y,b,prg),List(List(y,List(f,a,y))));
		else if (Atom(b))			// a expr, b atom
			return Append(CompileExpr1(y,a,prg),List(List(y,List(f,y,b))));
		else {
			Tnode* taux = MakeUnique(prg);
			return Append(CompileExpr1(taux,a,prg),CompileExpr1(y,b,prg),List(List(y,List(f,taux,y))));
		}
	} else if (len == 3) {
		Tnode *b = Second(e), *c = Third(e);
		Tnode *a1,*b1,*c1,*result=0;
		if (Atom(a))
			a1 = a;
		else {
			a1 = MakeUnique(prg);
			result = Append(result,CompileExpr1(a1,a,prg));
		}
		if (Atom(b))
			b1 = b;
		else {
			b1 = MakeUnique(prg);
			result = Append(result,CompileExpr1(b1,b,prg));
		}
		if (Atom(c))
			c1 = c;
		else {
			c1 = MakeUnique(prg);
			result = Append(result,CompileExpr1(c1,c,prg));
		}
		return Append(result,List(List(y,List(f,a1,b1,c1))));
	} else {	// len >= 4
		Tnode *result=0, *arglist=0;
		for (Tnode*p=First(e); p; p=p->next) {
			if (Atom(p))
				arglist = Append(arglist,List(p));
			else {
				Tnode *p1 = MakeUnique(prg);
				result = Append(result,CompileExpr1(p1,p,prg));
				arglist = Append(arglist,List(p1));
			}
		}
		return Append(result,List(List(y,new Tnode(First(arglist),f,0))));
	}
	return 0;	// so that g++ doesn't yell
}

/*
(defun cc(y e &aux f a b taux)
  (if (atom e) (return-from cc (list (list y e))))
  (setq f (first e))
  (setq a (second e))
  (cond
     ((eq (length e) 2)
        (if (atom a)
            (list (list y e))
            (append (cc y a) (list (list y (list f y)))) ))
     ((eq (length e) 3)
        (setq b (third e))
        (cond ((and (atom a) (atom b)) ; both args atoms
                    (list (list y e)) )
              ((atom a) ; a atom, b expr
                    (append (cc y b) (list (list y (list f a y)))) )
              ((atom b) ; a expr, b atom
                    (append (cc y a) (list (list y (list f y b)))) )
              (t        ; a expr, b expr
                    (setq taux (make-unique))
                    (append (cc taux a) (cc y b) (list (list y (list f taux y)))) ) ) )
     (t (format t "~&Error: arg number is ~D" (- (length e) 1)))
) )
*/

Toperand convertToOperand(const Tnode*node) {
	Toperand result;
	switch (node->kind) {
	case Kvariable:
		result = Toperand(node->SymbolValue());
		break;
	case Kconstant:
		result = Toperand(node->ObjectValue());
		break;
	case Kslot:
		result = Toperand(node->SlotValue());
		break;
	case Kcolon:
		result = Toperand(Tallrange());
		break;
	case Kbuiltin:
		err << "Internal error in Tnode* --> Toperand conversion: type Kbuiltin\n";
		error();
	}
	return result;
}

INLINE int find(Tnode*x, Tnode *const nodes[], int N) {
	for (int i=0; i<N; i++)
		if (nodes[i]->GeneralValue() == x->GeneralValue()) return i;
	return -1;
}

static int locate(const Tstring& s, TStringLL& strings) {
	const int L = strings.length();
	for (int i=0; i<L; i++)
		if (s == strings[i]) return i;
	strings.append(s);
	return L;
}

void Tcompilation::CompileCall(const Toperand& fn,
							   Tnode*const inputs[], int Ninputs,
							   Tnode*const outputs[], int Noutputs,
							   Tprg& prg, bool generate_scall)
{
	// Produce CALL fn,Noutputs,inputs[0],inputs[1],...,inputs[M], outputs[0],outputs[1],..,outputs[Noutputs-1]
	// If input arg is also output args, introduce temporaries in their place, plus MOVE instructions
	// prior to CALL.
	// If generate_scall is true, produces SCALL
	if (fn.kind() == Ksym && fn.symbol().value()->IsIntrinsicFunction()) {
		TIntrinsicCompilerPtr fptr = fn.symbol().value()->IntrinsicCompilerPtr();
		int c = fn.symbol().value()->IntrinsicCode();
		if (!(*fptr)(inputs,Ninputs, outputs,Noutputs,c, prg)) {
			err << "Code generation error: bad calling sequence of intrinsic function " << fn.symbol() << ".\n";
			error();
		}
	} else {
		static int isOutput[MAX_NOPERANDS];
		static Tnode *inputs1[MAX_NOPERANDS];
        int i;
		for (i=0; i<Ninputs; i++) {
			isOutput[i] = find(inputs[i],outputs,Noutputs) >= 0;
			inputs1[i] = (isOutput[i]) ? MakeUnique(prg) : inputs[i];
		}
		for (i=0; i<Ninputs; i++)
			if (isOutput[i])
				prg.add(Fmove,convertToOperand(inputs1[i]),convertToOperand(inputs[i]));
		if (generate_scall)
			prg.add(Fscall,2+Ninputs+Noutputs);
		else
			prg.add(Fcall,2+Ninputs+Noutputs);
		prg.append(fn);
		prg.append(Toperand(Klit,Noutputs));
		for (i=0; i<Noutputs; i++) prg.append(convertToOperand(outputs[i]));
		for (i=0; i<Ninputs; i++) prg.append(convertToOperand(inputs1[i]));
	}
}

void Tcompilation::codegenSimpleExpr(Tnode*y, Tnode*e, Tprg& prg)
// e must be a simple expr, like e=PLUS[a,b].
// y must be a variable or slot.
// Will append ADD y,a,b to prg
{
	if (y->kind != Kvariable && y->kind != Kslot) {
		err<<"codegenSimpleExpr: y must be variable\n";
		error();
	}
	Tfunc h = e->FunctionValue();
	global::lineno = e->LineNumber();
	Tnode*arg1 = First(e);
	Tnode*arg2 = 0;
	Toperand s=convertToOperand(y),a,b;
	if (arg1) {
		a = convertToOperand(arg1);
		arg2 = Second(e);
		if (arg2) b = convertToOperand(arg2);
	}
	switch (h) {
	case FPLUS:
		if (s == a) {
			if (arg2->kind==Kconstant && arg2->ObjectValue().IntValue() == 1)
				prg.add(Finc,s);
			else
				prg.add(Fadd,s,b);
		} else if (s == b) {
			if (arg1->kind == Kconstant && arg1->ObjectValue().IntValue() == 1)
				prg.add(Finc,s);
			else
				prg.add(Fadd,s,a);
		} else
			prg.add(Fadd,s,a,b);
		break;
	case FDIFFERENCE:
		if (s == a) {
			if (arg2->kind==Kconstant && arg2->ObjectValue().IntValue() == 1)
				prg.add(Fdec,s);
			else
				prg.add(Fsub,s,b);
		} else
			prg.add(Fsub,s,a,b);
		break;
	case FMINUS:
		prg.add(Fneg,s,a);
		break;
	case FTIMES:
		if (s == a)
			prg.add(Fmul,s,b);
		else if (s == b)
			prg.add(Fmul,s,a);
		else
			prg.add(Fmul,s,a,b);
		break;
	case FQUOTIENT:
		if (s == a)
			prg.add(Fdiv,s,b);
		else
			prg.add(Fdiv,s,a,b);
		break;
	case FMOD:
		prg.add(Fmod,s,a,b);
		break;
	case FPOWER:
		if (b.kind() == Kobj) {
			if (prg.nthobject(b.offset())->kind() == Kint) {
				int exponent = prg.nthobject(b.offset())->IntValue();
				switch (exponent) {
				case 0:
					prg.add(Fmove,s,Toperand(TLiteralInt(1)));
					break;
				case 1:
					prg.add(Fmove,s,a);
					break;
				case 2:
					prg.add(Fmul,s,a,a);
					break;
				case 4:
					prg.add(Fmul,s,a,a);
					prg.add(Fmul,s,s);
					break;
				default:
					prg.add(Fpow,s,a,b);
				}
			} else
				prg.add(Fpow,s,a,b);
		} else
			prg.add(Fpow,s,a,b);
		break;
	case FEQ:
		prg.add(Feq,s,a,b);
		break;
	case FNE:
		prg.add(Fne,s,a,b);
		break;
	case FGT:
		prg.add(Fgt,s,a,b);
		break;
	case FLT:
		prg.add(Flt,s,a,b);
		break;
	case FGE:
		prg.add(Fge,s,a,b);
		break;
	case FLE:
		prg.add(Fle,s,a,b);
		break;
	case FAND:
		if (s == a)
			prg.add(Fand,s,b);
		else
			prg.add(Fand,s,a,b);
		break;
	case F_OR:
		if (s == a)
			prg.add(For,s,b);
		else
			prg.add(For,s,a,b);
		break;
	case FNOT:
		if (s == a)
			prg.add(Fnot,s);
		else
			prg.add(Fnot,s,a);
		break;
	/*
	case FMIN:
		if (arg2)
			prg.add(Fmin,s,a,b);
		else
			prg.add(Fmin,s,a);
		break;
	case FMAX:
		if (arg2)
			prg.add(Fmax,s,a,b);
		else
			prg.add(Fmax,s,a);
		break;
	*/
	case FRANGE:	// RANGE[A,Step,B] (a=A, b=Step, c=B)
		{
			Tnode* arg3 = arg2->next;
			if (arg3)
				prg.add(Frange,s,a,convertToOperand(arg3),b);
			else
				prg.add(Frange,s,a,b,Toperand(TLiteralInt(1)));
		}
		break;
	case FEMPTY_ARRAY:
		prg.add(Farray,s);
		break;
	case FARRAY:	// ARRAY[a1,a2,...] ==> ARRAY s,a1,a2,...
		prg.add(Farray,1+Length(e));
		prg.append(s); prg.append(a);
		if (arg2) {
			prg.append(b);
			for (Tnode* arg=arg2->next; arg; arg=arg->next)
				prg.append(convertToOperand(arg));
		}
		break;
	case FAPP:	// APP[a1,a2,...] ==> APPEND s,a1,a2,...
		prg.add(Fappend,1+Length(e));
		prg.append(s); prg.append(a);
		if (arg2) {
			prg.append(b);
			for (Tnode* arg=arg2->next; arg; arg=arg->next)
				prg.append(convertToOperand(arg));
		}
		break;
	case FREF:	// REF[a,i1,i2,...] ==> GATH s,a,i1,i2,...
		{
			prg.add(Fgath,1+Length(e));
			prg.append(s); prg.append(a); prg.append(b);
			for (Tnode* arg=arg2->next; arg; arg=arg->next)
				prg.append(convertToOperand(arg));
		}
		break;
	case FMREF:	// MREF[a,i1,i2,...] ==> MGATH s,a,i1,i2,...
		{
			prg.add(Fmgath,1+Length(e));
			prg.append(s); prg.append(a); prg.append(b);
			for (Tnode* arg=arg2->next; arg; arg=arg->next)
				prg.append(convertToOperand(arg));
		}
		break;
	/*
	case FIZEROS:	// IZEROS[dim1,dim2,...]
	case FRZEROS:
	case FCZEROS:
		{
			Tcode cod;
			if (h==FIZEROS) cod=Fizeros;
			else if (h==FRZEROS) cod=Frzeros;
			else cod=Fczeros;
			prg.add(cod,1+Length(e));
			prg.append(s);
			for (Tnode*arg = First(e); arg; arg=arg->next)
				prg.append(convertToOperand(arg));
		}
		break;
	*/
	/*	
	case FABS:
		if (s == a)
			prg.add(Fabs,s);
		else
			prg.add(Fabs,s,a);
		break;
	*/
	case FCALL:	// s = CALL[func,arg1,arg2,...,argN] ==> CALL func,1,s,arg1,arg2,...,argN
		{
			int Ninputs=0;
			for (Tnode*arg=arg2; arg; arg=arg->next,Ninputs++) gl_inputs[Ninputs] = arg;
			gl_outputs[0] = y;
			CompileCall(a,gl_inputs,Ninputs,gl_outputs,1,prg,true);
#if 0
			prg.add(Fcall,2+Length(e));
			prg.append(a);	// func
			prg.append(Toperand(Klit,1));	// this 1 means: one output arg
			prg.append(s);
			for (Tnode*arg=arg2; arg; arg=arg->next)
				prg.append(convertToOperand(arg));
#endif
		}
		break;
	case FNOP:
	case FHARDNOP:
		// NOP and HARDNOP generate no code in expr (HARDNOP does generate as a stmt)
		break;
	case FSET:
	case FBLOCK:
	case FDEFUN:
	case FPACKAGE:
	case FIF:
	case FWHILE:
	case FFOR:
	case FCONTINUE:
	case FBREAK:
	case FRETURN:
	case FGOTO:
	case FLABEL:
	default:
		cerr << "Warning: codegen found ugly stuff inside expr\n";
	}
}

void Tcompilation::CompileExpr(Tnode*to, Tnode*auxvar, Tnode*from, Tprg& prg)
// Generate code for to=from, accumulating 'to' to auxvar
// Only the last instruction will use 'to' as a destination, other use auxvar.
{
	TNodeKind k = to->kind;
	if (k != Kvariable && k != Kslot) {
		err<<"CompileExpr: Trying to set to non-variable " << *to << ".\n";
		error();
	}
	Tnode*setlist = CompileExpr1(auxvar,from,prg);	// Compile to auxvar
	//clog << "--CompileExpr1=" << *setlist << ".\n";
	for (Tnode *p=First(setlist); p; p=p->next) {
		Tnode*y = First(p);
		Tnode*e = Second(p);
		if (!p->next) y = to;	// but change destination to 'to' in the last instruction
		switch (e->kind) {
		case Kbuiltin:
			codegenSimpleExpr(y,e,prg);
			break;
		case Kvariable:
			prg.add(Fmove,convertToOperand(y),Toperand(e->SymbolValue()));
			break;
		case Kslot:
			prg.add(Fmove,convertToOperand(y),Toperand(e->SlotValue()));
			break;
		case Kconstant:
			prg.add(Fmove,convertToOperand(y),Toperand(e->ObjectValue()));
			break;
		case Kcolon:
			prg.add(Fmove,convertToOperand(y),Toperand(Tallrange()));
			break;
		}
	}
}

INLINE int find(Tsymbol*sym, Tsymbol *symtab[], int N) {
	for (int i=0; i<N; i++)
		if (symtab[i] == sym) return i;
	return -1;
}

static void fixlexicals(Tnode*body, Tsymbol *lexicals[], int Nlexicals) {
	// Replaces all occurrences of lexicals in body by SLOT[n],
	// where n is the corresponding index in table lexicals.
	for (Tnode*p=body; p; p=p->next) {
		if (p->kind == Kvariable) {
			int n = find(&(p->SymbolValue()),lexicals,Nlexicals);
			if (n >= 0) *p = Tslot(n);
		}
		if (p->list) fixlexicals(First(p),lexicals,Nlexicals);
	}
}

static void fixlexicals_for_package(Tnode*body, Tsymbol *lexicals[], int Nlexicals, const Tchar prefix[]) {
	// Replaces all occurrences of lexicals in body by new symbols that have
	// prefix prepended in their name
	for (Tnode*p=body; p; p=p->next) {
		if (p->kind == Kvariable) {
			int n = find(&(p->SymbolValue()),lexicals,Nlexicals);
			if (n >= 0) {
				Tchar *newsymname = new Tchar [strlen(prefix) + strlen((char*)(p->SymbolValue().name())) + 1];
				strcpy(newsymname,prefix);
				strcat(newsymname,(Tchar*)(p->SymbolValue().name()));
				Tsymbol *newsymptr = theHT.add(newsymname);
				*p = *newsymptr;	// set new symbol in place
				delete [] newsymname;	// theHT.add made a copy of it already
			}
		}
		if (p->list) fixlexicals_for_package(First(p),lexicals,Nlexicals,prefix);
	}
}

static int IsSimpleIncrementation(Tnode*f) {
	// Tests if f is of theform SET[i,PLUS[i,1]] for any i
	if (f->kind!=Kbuiltin) return 0;
	Tnode*i = First(f);
	if (!i) return 0;
	Tnode*subexpr = Second(f);
	if (!subexpr) return 0;
	if (subexpr->kind!=Kbuiltin || Head(subexpr)!=FPLUS) return 0;
	if (!First(subexpr)) return 0;
	if (First(subexpr)->GeneralValue()!=i->GeneralValue()) return 0;
	Tnode*second = Second(subexpr);
	if (!second) return 0;
	if (second->kind!=Kconstant) return 0;
	if (second->ObjectValue().kind()!=Kint) return 0;
	if (second->ObjectValue().IntValue()!=1) return 0;
	return 1;
}

static int ComparisonAccelerator(Tprg& prg) {
	Tcode op = prg.LastOpcode();
	int result = -1;
	if (op==Fgt || op==Flt || op==Fge || op==Fle) {
		// If the last instruction generated by CompileExpr was a comparison,
        // generate JNGT etc. instructions (avoid using AnsVariable altogether)
		Tcode oldop = prg[prg.length()-4].opcode();
		Tcode newop;
		if (oldop==Fgt)
			newop = Fjngt;
		else if (oldop==Flt)
			newop = Fjnlt;
		else if (oldop==Fge)
			newop = Fjnge;
		else if (oldop==Fle)
			newop = Fjnle;
		else {
			err << "Internal in ComparisonAccelerator\n";
			error();
		}
		prg[prg.length()-4] = Toperand(newop,3);
		result = prg.length()-3;
    }
	return result;
}

void Tcompilation::FixJumps(Tprg& prg, TBreakPointList& jumplist) {
	for (int i=0; i<jumplist.length(); i++)
		if (LoopLevel == jumplist.level(i)) {
			prg[jumplist.address(i)] = Toperand(Klit,prg.length());
			jumplist.address(i) = -1;
			jumplist.level(i) = -1;
		}
}

static Tnode *FindVarsVarList = 0;

static int MemberQ(Tsymbol*s, Tnode*list) {
	if (!list) return 0;
	Tnode *start = (list->kind == Kbuiltin) ? list->list : list;
	for (Tnode*q = start; q; q=q->next)
		if (&(q->SymbolValue()) == s) return 1;
	return 0;
}

static void FindVars(Tnode*x, int CallFlag, Tnode*excl1, Tnode*excl2, Tnode*excl3) {
	if (!x) return;
	for (Tnode*p=x; p; p=p->next) {
		//cerr<<"FindVars: " << *p << "\n";
		if (p->list) FindVars(p->list,p->kind==Kbuiltin && Head(p)==FCALL, excl1,excl2,excl3);	// recursion
		if (p->kind==Kvariable && !CallFlag) {
			// add p to FindVarsVarList if not already there,
			// and if it's not in lists excl1, excl2
			Tsymbol *s = &(p->SymbolValue());
			if (!MemberQ(s,FindVarsVarList) && !MemberQ(s,excl1) && !MemberQ(s,excl2) && !MemberQ(s,excl3) && !s->IsGlobal()) {
				Tnode *n = new Tnode(0,&(p->SymbolValue()),FindVarsVarList);
				FindVarsVarList = n;
			}
		}
		CallFlag = 0;	// only first arg of CALL is ignored
	}
}

static Tnode* FindVars(Tnode*x, Tnode*excl1=0, Tnode*excl2=0, Tnode*excl3=0) {
	FindVarsVarList = 0;
	FindVars(x,0, excl1,excl2,excl3);
	Tnode *n = new Tnode(FindVarsVarList,FLIST,0);
	FindVarsVarList = n;
/*
	if (flags::verbose) {
		cout << "FindVars: ";
		cout << *FindVarsVarList << "\n";
	}
*/
	return FindVarsVarList;
}

void GenerateCode(Tnode*x, Tprg& prg, int printsetflag=0);

void Tcompilation::codegen(Tnode*x, Tprg& prg, int printsetflag)
{
	Tnode *p;
	Tfunc h;
	if (x->kind != Kbuiltin) {err<<"codegen: head must be builtin\n"; error();}
	h = x->FunctionValue();
	global::lineno = x->LineNumber();
	//clog << "codegen " << FuncData[h] << '\n';
	switch (h) {
	case FPLUS:
	case FDIFFERENCE:
	case FMINUS:
	case FTIMES:
	case FQUOTIENT:
	case FMOD:
	case FPOWER:
		cerr << "Warning: codegen ignored arithmetic operation without assignment\n";
		break;
	case FEQ:
	case FNE:
	case FGT:
	case FLT:
	case FGE:
	case FLE:
		cerr << "Warning: codegen ignored relational operation without assignment\n";
		break;
	case FAND:
	case F_OR:
	case FNOT:
		cerr << "Warning: codegen ignored logical operation without assignment\n";
		break;
	case FRANGE:
		cerr << "Warning: codegen saw RANGE\n";
		break;
	case FCALL:	// FCALL[fn,arg1,arg2,...,argN] ==> CALL fn,0,arg1,arg2,...,argN
		{
			int i,ncompiles=0,nargs=0,iscompilable[MAX_NOPERANDS];
			Tnode *indices[MAX_NOPERANDS], *exprs[MAX_NOPERANDS];
			unique_index = 0;	/* Added 30.1.1995 */
			for (i=0,p=First(x); p; i++,p=p->next,nargs++) {
				if (p->kind == Kbuiltin) {
					exprs[i] = p;
					indices[i] = MakeUnique(prg);
					iscompilable[i] = 1;
					ncompiles++;
				} else {
					indices[i] = p;
					iscompilable[i] = 0;
				}
			}
			for (i=0; i<nargs; i++)
				if (iscompilable[i]) {
					unique_index = ncompiles;
					CompileExpr(indices[i],AnsVariable,exprs[i],prg);
				}
			CompileCall(convertToOperand(indices[0]),indices+1,nargs-1,gl_outputs,0,prg,true);
		}
		break;
	case FSET:
		{
			Tnode *y = First(x);
			if (Length(x)==1) {
				static Tsymbol *NopSymbol = 0;
				if (NopSymbol == 0) NopSymbol = theHT.add((const Tchar*)"nop");
				CompileExpr(AnsVariable,AnsVariable,y,prg);
				if (!(y->kind == Kbuiltin && Head(y)==FCALL
					  && First(y)->kind == Kvariable
					  && &(First(y)->SymbolValue()) == NopSymbol))
					prg.add(Fpri,Toperand(AnsVariable->SlotValue()));
				break;
			}
			Tnode *e = Second(x);
			if (y->kind == Kvariable || y->kind == Kslot) {
				/* Set to symbol or slot, like y = expr */
				unique_index = 0;
				if (y->kind == Kvariable) y->SymbolValue().ClearStubFlag();
				CompileExpr(First(x),AnsVariable,Second(x),prg);
				if (printsetflag) {				// printing of moved value desired
					CompileExpr(AnsVariable,AnsVariable,First(x),prg);
					prg.add(Fpri,Toperand(AnsVariable->SlotValue()));
				}
			} else if (y->kind == Kbuiltin && (y->FunctionValue() == FREF || y->FunctionValue() == FMREF)) {
				/* Set to array component, like y[i,j,...] = expr */
				int i,ncompiles=0,ndims=0,iscompilable[MAXRANK+1];
				Tnode *indices[MAXRANK+1], *exprs[MAXRANK+1];
				unique_index = 0;
				for (i=0,p=First(y); p; i++,p=p->next,ndims++) {
					if (p->kind == Kbuiltin) {
						exprs[i] = p;
						indices[i] = MakeUnique(prg);
						iscompilable[i] = 1;
						ncompiles++;
					} else {
						indices[i] = p;
						iscompilable[i] = 0;
					}
				}
				for (i=0; i<ndims; i++)
					if (iscompilable[i]) {
						unique_index = ncompiles;
						CompileExpr(indices[i],AnsVariable,exprs[i],prg);
					}
				CompileExpr(AnsVariable,AnsVariable,e,prg);
				prg.add((y->FunctionValue()==FMREF) ? Fmscat : Fscat, 1+ndims);		// (M)SCAT Ans,First(y),Second(y),...
				prg.append(Toperand(AnsVariable->SlotValue()));
				for (i=0; i<ndims; i++)
					prg.append(convertToOperand(indices[i]));
				if (printsetflag)
					prg.add(Fpri,Toperand(AnsVariable->SlotValue()));
			} else if (y->kind == Kbuiltin && y->FunctionValue() == FLIST) {
				/* Multiple output arg call, like [y1,y2,..] = f(x1,x2,...) */
				/* FSET[FLIST[y1,y2,..,yM],FCALL[fn,arg1,arg2,...,argN]] ==> CALL fn,M,y1,y2,...,yM,arg1,arg2,...,argN */
				unique_index = 0;	/* Added 30.1.1995 */
				if (e->kind != Kbuiltin || e->FunctionValue() != FCALL) {
					err << "In SET[LIST[...],f], f must be CALL[...].\n";
					error();
				}
				int i,ncompiles=0,nargs=0,iscompilable[MAX_NOPERANDS];
				Tnode *indices[MAX_NOPERANDS], *exprs[MAX_NOPERANDS];
				/* Output args (y): */
				for (i=0,p=First(y); p; i++,p=p->next,nargs++) {
					if (p->kind == Kbuiltin) {
						exprs[i] = p;
						indices[i] = MakeUnique(prg);
						iscompilable[i] = 1;
						ncompiles++;
					} else {
						indices[i] = p;
						iscompilable[i] = 0;
					}
				}
				/* Input args (e): */
				for (p=Second(e); p; i++,p=p->next,nargs++) {
					if (p->kind == Kbuiltin) {
						exprs[i] = p;
						indices[i] = MakeUnique(prg);
						iscompilable[i] = 1;
						ncompiles++;
					} else {
						indices[i] = p;
						iscompilable[i] = 0;
					}
				}
				for (i=0; i<nargs; i++)
					if (iscompilable[i]) {
						unique_index = ncompiles;
						CompileExpr(indices[i],AnsVariable,exprs[i],prg);
					}
				int Noutputs = Length(y);
				int Ninputs = Length(e)-1;
				CompileCall(convertToOperand(First(e)),indices+Noutputs,Ninputs, indices,Noutputs, prg,false);
			} else {
				err<<"Trying to SET non-variable or non-ref or non-list\n";
				error();
			}
		}
		break;
	case FBLOCK:
		for (p=First(x); p; p=p->next) {
			unique_index=0;		/* Added 30.1.1995 */
			codegen(p,prg);
		}
		break;
	case FDEFUN:	// DEFUN[funcname,outargs,inargs,localvars,body]
		{
			Tnode *funcname = First(x);
			Tnode *outargs = Second(x);
			Tnode *inargs = Third(x);
			Tnode *localdecl = Fourth(x);
			Tnode *body = Fifth(x);
			int Noutputs,Ninputs,Nlocals;
			int NMinOutputs, NMinInputs;
			int InputEllipsis=0, OutputEllipsis=0;
			// Determine Noutputs and NMinOutputs
			if (outargs->kind==Kvariable) {		// y = function f(....  ==> y is optional
				Noutputs = 1;
				NMinOutputs = 0;
			} else if (Head(outargs) == FLIST) {	// [y1,y2] = function f(.... ==> y1,y2 optional
				Noutputs = Length(outargs);
				NMinOutputs = 0;
			} else if (Head(outargs) == FFORMAL) {
				if (Length(outargs) == 1) {			// [y1,y2] = function f(.... ==> y1,y2 optional
					outargs = First(outargs);
					Noutputs = Length(outargs);
					NMinOutputs = Noutputs;
				} else if (Length(outargs) == 2) {	// [y1,y2;y3,y4] ==> y1,y2 obligatory, but y3,y4 optional
					NMinOutputs = Length(First(outargs));
					outargs = Append(First(outargs),Second(outargs));
					Noutputs = Length(outargs);
				} else {
					err << "Internal 1 in codegen:DEFUN\n";
					Noutputs=NMinOutputs=0;
					error();
				}
			} else if (Head(outargs) == FFORMAL_ELLIPSIS) {
				OutputEllipsis = 1;
				if (Length(outargs) == 1) {		// [y1,y2...] ==> y1,y2 optional
					outargs = First(outargs);
					Noutputs = Length(outargs);
					NMinOutputs = 0;
				} else if (Length(outargs) == 2) {	// [y1,y2; y3,y4...] ==> y1,y2 obligatory, y3,y4 optional
					NMinOutputs = Length(First(outargs));
					outargs = Append(First(outargs),Second(outargs));
					Noutputs = Length(outargs);
				} else {
					err << "Internal 2 in codegen::DEFUN\n";
					Noutputs=NMinOutputs=0;
					error();
				}
			} else {
				err << "Internal 2b in codegen::DEFUN\n";
				Noutputs=NMinOutputs=0;
				error();
			}
			//Noutputs = (outargs->kind==Kvariable) ? 1 : Length(outargs);
			// Determine Ninputs and NMinInputs
			if (inargs->kind==Kvariable) {		// (x1) ==> obligatory parameter symbol x1
				Ninputs = 1;
				NMinInputs = Ninputs;
			} else if (Head(inargs) == FLIST) {	// (x1,x2) ==> obligatory x1 and x2
				Ninputs = Length(inargs);
				NMinInputs = Ninputs;
			} else if (Head(inargs) == FFORMAL) {
				if (Length(inargs) == 1) {		// (x1,x2) ==> obligatory x1 and x2
					inargs = First(inargs);
					Ninputs = Length(inargs);
					NMinInputs = Ninputs;
				} else if (Length(inargs) == 2) {	// (x1,x2; x3,x4) ==> obligatory x1 and x2, optional x3,x4
					NMinInputs = Length(First(inargs));
					inargs = Append(First(inargs),Second(inargs));
					Ninputs = Length(inargs);
				} else {
					err << "Internal 3 in codegen:DEFUN\n";
					Ninputs=NMinInputs=0;
					error();
				}
			} else if (Head(inargs) == FFORMAL_ELLIPSIS) {
				InputEllipsis = 1;
				if (Length(inargs) == 1) {		// (x1,x2...) ==> obligatory x1 and x2
					inargs = First(inargs);
					Ninputs = Length(inargs);
					NMinInputs = Length(inargs);
				} else if (Length(inargs) == 2) {	// (x1,x2; x3,x4...) ==> obligatory x1,x2, optional x3,x4
					NMinInputs = Length(First(inargs));
					inargs = Append(First(inargs),Second(inargs));
					Ninputs = Length(inargs);
				} else {
					err << "Internal 4 in codegen::DEFUN\n";
					Ninputs=NMinInputs=0;
					error();
				}
			} else {
				err << "Internal 4b in codegen::DEFUN\n";
				Ninputs=NMinInputs=0;
				error();
			}
			if (localdecl->kind==Kbuiltin && Head(localdecl)==FLOCAL) {
				if (Atom(localdecl)) {
					// Auto-local case: function f(...) local {...}
					localdecl = FindVars(body,outargs,inargs);
				} else {
					// Explicit local case: function f(...) local(a,b,...) {...}
					// need do nothing here...
				}
			} else if (localdecl->kind==Kbuiltin && Head(localdecl)==FGLOBAL) {
				if (Atom(localdecl)) {
					// Auto-global case: function f(...) global {...}
					localdecl = 0;
				} else {
					// Explicit global case: function f(...) global(a,b,...) {...}
					localdecl = FindVars(body,outargs,inargs,localdecl);
				}
			} else {
				err << "Internal 4c in codegen::DEFUN\n";
				error();
			}
			Nlocals = Length(localdecl);

			if (funcname->kind != Kvariable) {err<<"Trying to define non-symbol " << *funcname << "\n"; error();}
#if 0
			if (funcname->kind != Kvariable && funcname->kind != Kslot) {
				err<<"Trying to define non-variable " << *funcname << "\n";
				error();
			}
#endif

			int Nlexicals = Ninputs + Noutputs + Nlocals + 1;	// this 1 comes from the AnsVariable
			if (InputEllipsis) Nlexicals++;
			if (OutputEllipsis) Nlexicals++;
			typedef Tsymbol* TSymbolPtr;
			Tsymbol **lexicals = new TSymbolPtr [Nlexicals];
			int i=0,j=1;	// this 1 comes from the AnsVariable
			if (inargs->kind==Kvariable)
				lexicals[j++] = &(inargs->SymbolValue());
			else
				for (i=0; i<Ninputs; i++,j++) lexicals[j] = &(Nth(inargs,i)->SymbolValue());
			if (InputEllipsis) j++;
			if (outargs->kind==Kvariable)
				lexicals[j++] = &(outargs->SymbolValue());
			else
				for (i=0; i<Noutputs; i++,j++) lexicals[j] = &(Nth(outargs,i)->SymbolValue());
			if (OutputEllipsis) j++;
			for (i=0; i<Nlocals; i++,j++) lexicals[j] = &(Nth(localdecl,i)->SymbolValue());
			fixlexicals(body,lexicals,Nlexicals);
			if (flags::verbose) cout << *body << '\n';
			delete [] lexicals;
			TObjectPtr objptr = funcname->SymbolValue().value();
#if 0			
			TObjectPtr objptr = (funcname->kind == Kvariable) ? funcname->SymbolValue().value() : theStack[funcname->SlotValue()];
#endif
			if (objptr->IsFunction())
				delete objptr->FunctionValue();
			Tprg *p = new Tprg(prg.FileName(),Ninputs,Noutputs,NMinInputs,NMinOutputs,Nlocals,
#if 0
							   (funcname->kind==Kvariable) ?
#endif
							   &(funcname->SymbolValue())
#if 0
							   : 0
#endif
				);
			    // this will make *p the current static object list
			if (InputEllipsis) p->SetInputEllipsisFlag();
			if (OutputEllipsis) p->SetOutputEllipsisFlag();
			Tcompilation comp;
			comp.codegen(body,*p);
			p->checkreadonlyvars();
			if (comp.GotoList.length())
			cerr << "Warning: Undefined label" << (comp.GotoList.length()==1 ? " " : "s ") << "seen.\n"
			     << "         Gotos to these labels will just exit the function.\n";
			if (flags::optimize) p->optimize();
			if (flags::verbose) cout << "\nFUNCTION " << *funcname << ":" << *p << '\n';
			if (global::ctfile) Ctgen(*global::ctfile,*p);
			*objptr = p;
#if 0
			if (funcname->kind==Kvariable)
#endif
				funcname->SymbolValue().ClearStubFlag();
			prg.JustMakeCurrent();		// restore prg as the current static object list
		}
		break;
	case FPACKAGE:	// PACKAGE[pkgname,localvars,body]
		{
			Tnode *pkgname = First(x);
			Tnode *localdecl = Second(x);
			Tnode *body = Third(x);
			if (localdecl->kind==Kbuiltin && Head(localdecl)==FLOCAL) {
				if (Atom(localdecl)) {
					// Auto-local case: package "name" local {...}
					localdecl = FindVars(body);
				} else {
					// Explicit local case: package "name" local(a,b,...) {...}
					// need do nothing here...
				}
			} else if (localdecl->kind==Kbuiltin && Head(localdecl)==FGLOBAL) {
				if (Atom(localdecl)) {
					// Auto-global case: package "name" global {...}
					localdecl = 0;
				} else {
					// Explicit global case: package "name" global(a,b,...) {...}
					localdecl = FindVars(body,localdecl);
				}
			} else {
				err << "Internal 4c in codegen::PACKAGE\n";
				error();
			}
			const int Nlocals = Length(localdecl);
			if (pkgname->kind != Kconstant || !pkgname->ObjectValue().IsString()) {
				err<<"Internal 4d in codegen::PACKAGE: non-string " << *pkgname << "\n";
				error();
			}
			const Tstring name = pkgname->ObjectValue();
			const int p = locate(name,thePackageNames);
			char prefix[30];
			sprintf(prefix,"%d:",p+1);
			typedef Tsymbol* TSymbolPtr;
			Tsymbol **locals = new TSymbolPtr [Nlocals];
			for (int i=0; i<Nlocals; i++) locals[i] = &(Nth(localdecl,i)->SymbolValue());
			fixlexicals_for_package(body,locals,Nlocals,(Tchar*)prefix);
			if (flags::verbose) cout << *body << '\n';
			Tcompilation comp;
			comp.codegen(body,prg);
			if (flags::verbose) cout << "\nPACKAGE " << *pkgname << ", ID=" << p+1 << ":\n";
			delete [] locals;
		}
		break;
	case FDISP:
		unique_index = 0;
		CompileExpr(AnsVariable,AnsVariable,First(x),prg);
		prg.add(Fpri,Toperand(AnsVariable->SlotValue()));
		break;
	case FIF:
		if (Length(x)==2) {
			/* IF[cond,then-clause] will yield:
			   <evaluate condition to ANS>
			   <if not ANS, then JUMP to LABEL>
			   <then-clause>
			   LABEL:
			   */
			unique_index = 0;
			CompileExpr(AnsVariable,AnsVariable,First(x),prg);	// compile the conditional expression into ANS
			int fixop;
			if ((fixop=ComparisonAccelerator(prg)) < 0) {
				prg.add(Fjifn,Toperand(Klit,0),Toperand(AnsVariable->SlotValue()));	// add jump instruction with zero addr
				fixop = prg.length()-2;	// fixop points to jump address operand
			}
			codegen(Second(x),prg);		// compile the THEN-clause
			prg[fixop] = Toperand(Klit,prg.length());	// fix the jump address, prg.length() is just after the THEN-clause
		} else {	// not 2 operands, then it must have 3 (believe it without checking)
			/* IF[cond,then-clause,else-clause] will yield:
			   <evaluate condition to ANS>
			   <if not ANS, then JUMP to LABEL1>
			   <then-clause>
			   <jump to LABEL2>
			   LABEL1:
			   <else-clause>
			   LABEL2:
			   */
			unique_index = 0;
			int fixop1;
			CompileExpr(AnsVariable,AnsVariable,First(x),prg);
			if ((fixop1=ComparisonAccelerator(prg)) < 0) {
				prg.add(Fjifn,Toperand(Klit,0),Toperand(AnsVariable->SlotValue()));
				fixop1 = prg.length()-2;
			}
			codegen(Second(x),prg);
			prg.add(Fjmp,Toperand(Klit,0));
			prg[fixop1] = Toperand(Klit,prg.length());
			int fixop2 = prg.length()-1;
			codegen(Third(x),prg);
			prg[fixop2] = Toperand(Klit,prg.length());
		}
		break;
	case FWHILE:
		/* WHILE[cond,stmt] yields:
		   LABEL1:
		   <evaluate condition to ANS>
		   <if not ANS, then JUMP to LABEL2>
		   <stmt body>
		   <jump to LABEL1>
		   LABEL2:
		   */
		{
			int label1 = prg.length();
			unique_index = 0;
			int fixop;
			CompileExpr(AnsVariable,AnsVariable,First(x),prg);	// compile the conditional expression into ANS
			if ((fixop=ComparisonAccelerator(prg)) < 0) {
				prg.add(Fjifn,Toperand(Klit,0),Toperand(AnsVariable->SlotValue()));	// add jump instruction with zero addr
				fixop = prg.length()-2;		// fixop points to jump address operand
			}
			LoopLevel++;
			codegen(Second(x),prg);		// compile the stmt body
			LoopLevel--;
			FixJumps(prg,ContinueList);
			prg.add(Fjmp,Toperand(Klit,label1));	// add the jump to label1 after stmt body
			prg[fixop] = Toperand(Klit,prg.length());	// fix the jump address
			FixJumps(prg,BreakList);
		}
		break;
	case FREPEAT:
		/* REPEAT[stmt,cond] yields:
		   LABEL:
		   <stmt>
		   <evaluate cond to ANS>
		   <if not ANS, then JUMP to LABEL>
		   */
		{
			int label1 = prg.length();
			LoopLevel++;
			codegen(First(x),prg);	// stmt
			LoopLevel--;
			unique_index = 0;
			FixJumps(prg,ContinueList);
			CompileExpr(AnsVariable,AnsVariable,Second(x),prg);	// cond
			int fixop = ComparisonAccelerator(prg);
			if (fixop >= 0)
				prg[fixop] = Toperand(Klit,label1);
			else
				prg.add(Fjifn,Toperand(Klit,label1),Toperand(AnsVariable->SlotValue()));
			FixJumps(prg,BreakList);
		}
		break;
	case FFOR:
		/* FOR[init, cond, incr, stmt]. init,incr,stmt are Statements, cond is Expr:
		   <init>
		   LABEL1:
		   <evaluate cond to ANS>
		   <if not ANS, then JUMP to LABEL>
		   <stmt>
		   <incr>
		   <jump to LABEL1>
		   LABEL2:
		   */
		{
			codegen(First(x),prg);	// compile init
			int label1 = prg.length();
			unique_index = 0;
			CompileExpr(AnsVariable,AnsVariable,Second(x),prg);	// compile the conditional expression into ANS
			//Tcode op = prg.LastOpcode();
			int fixop;
			if ((fixop = ComparisonAccelerator(prg)) < 0) {	// Try optimize to JNGT,JNLT ...
				prg.add(Fjifn,Toperand(Klit,0),Toperand(AnsVariable->SlotValue()));	// add jump instruction with zero addr
				fixop = prg.length()-2;	// fixop points to jump address operand
			}
			LoopLevel++;
			codegen(Fourth(x),prg);		// compile stmt
			LoopLevel--;
			FixJumps(prg,ContinueList);	// set all waiting continue-jumps pointing after stmt
			if (IsSimpleIncrementation(Third(x))) {		// Try to combine INC and JMP instructions to INCJMP
				prg.add(Fincjmp,Toperand(Klit,label1),convertToOperand(First(Third(x))));
			} else {	// Not succeeded INCJMP, do in ordinary way
				codegen(Third(x),prg);		// compile incr
				prg.add(Fjmp,Toperand(Klit,label1));	// add the jump to label1 after incr
			}
			prg[fixop] = Toperand(Klit,prg.length());	// fix the jump address
			FixJumps(prg,BreakList);	// set all waiting break-jumps pointing after loop
		}
		break;
	case FCONTINUE:
		if (LoopLevel-1 >= 0) {
			prg.add(Fjmp,Toperand(Klit,prg.length()+2));
			ContinueList.append(TBreakPoint(prg.length()-1,LoopLevel-1));
		} else {
			cerr << "*** Warning: continue statement in bad context, ignored.\n";
		}
		break;
	case FBREAK:
		if (LoopLevel-1 >= 0) {
			prg.add(Fjmp,Toperand(Klit,prg.length()+2));
			BreakList.append(TBreakPoint(prg.length()-1,LoopLevel-1));
		} else {
			cerr << "*** Warning: break statement in bad context, ignored.\n";
		}
		break;
	case FRETURN:
		prg.add(Fjmp,Toperand(Klit,MAXINT));
		break;
	case FGOTO:
		{
			Tsymbol * const label = &(First(x)->SymbolValue());
			const int i = LabelList.find(label);
			if (i >= 0) {	// label found, i.e. it has been defined already
				prg.add(Fjmp,Toperand(Klit,LabelList.getaddress(i)));
			} else {		// label not found, add to unsatisfied goto's list
				prg.add(Fjmp,Toperand(Klit,MAXINT));				// this GOTO will be patched later ...
				GotoList.add(label,prg.length()-1);		// prg.length()-1 points to patched jump address
			}
		}
		break;
	case FLABEL:
		{
			Tsymbol * const label = &(First(x)->SymbolValue());
			// Several goto's may point to one label, this taken into account 4.9.1995
			for (;;) {
				const int i = GotoList.find(label);
				if (i >= 0) {
					prg[GotoList.getaddress(i)] = Toperand(Klit,prg.length());	// fix old jump address
					GotoList.remove(i);		// remove from unsatisfied goto's list
				} else break;
			}
			const int j = LabelList.find(label);
			if (j >= 0)
				cerr << "Warning: label " << label->name() << " defined twice - new ignored\n";
			else
				LabelList.add(label,prg.length());
		}
		break;
	case FNOP:
		// NOP generates no code
		break;
	case FHARDNOP:
		prg.add(Fnop);
		break;
	default:
		cerr << "Warning: codegen saw unknown head\n";
	}
}

void GenerateCode(Tnode*x, Tprg& prg, int printsetflag) {
	Tcompilation comp;
	comp.codegen(x,prg,printsetflag);
	prg.checkreadonlyvars();
	if (comp.GotoList.length())
		cerr << "Warning: Undefined label" << (comp.GotoList.length()==1 ? " " : "s ") << comp.GotoList << ".\n"
			    "         Gotos to these labels will just exit the function.\n";
	prg.optimize();
}

