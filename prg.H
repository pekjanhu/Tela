/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma interface
#endif
#include "machine.H"
#include "common.H"

#define COMPTYPE Toperand
#define DEFAULT_COMPVALUE Toperand()
#define LINEARLIST TInstructionList1

#include "templ/tLL.H"

#undef LINEARLIST
#undef DEFAULT_COMPVALUE
#undef COMPTYPE

class TInstructionList : public TInstructionList1 {
 private:
	int maxNops;			// Maximum number of operands in any instruction
 public:
	TInstructionList() : TInstructionList1(0), maxNops(0) {}
	void add(Tcode code) {append(Toperand(code));}
	void add(Tcode code, const Toperand& op1);
	void add(Tcode code, const Toperand& op1, const Toperand& op2);
	void add(Tcode code, const Toperand& op1, const Toperand& op2, const Toperand& op3);
	void add(Tcode code, const Toperand& op1, const Toperand& op2, const Toperand& op3, const Toperand& op4);
	void add(Tcode code, int Noperands);
	int MaxNops() const {return maxNops;}
	void set_maxNops(int i) {maxNops = i;}
	void clear() {TInstructionList1::operator=(0);}
	friend ostream& operator<<(ostream& o, const TInstructionList& il);
};

/*#define MAX_NOPERANDS 200*/
#define MAX_NOPERANDS 250	// 9 Nov 2004

class TObjectStack {	// Runtime stack, common for all functions
 private:
	TObjectLL LL;		// Implemented as a linear list of TObjectPtrs
 public:
	TObjectStack() : LL(0) {}							// Initially, the stack is empty
	void push(TObjectPtr ptr) {LL.append(ptr);}			// stack.push(&obj) works
	void push(Tobject& obj) {LL.append(&obj);}			// stack.push(obj) also works
	void push(int N) {int L=LL.length(); LL=L+N;}		// push N null pointers
	Tobject& top() const {return *LL[LL.length()-1];}	// stack.top() gives ref to last object pushed
	void pop(int N);									// shorten the stack by N objects
	int length() const {return LL.length();}			// stack.length() == number of objects in the stack
	TObjectPtr& operator[](int i) {return LL[LL.length()-1-i];}	// stack[0] == &stack.top(), etc.
	friend inline ostream& operator<<(ostream& o, const TObjectStack& stack);
	~TObjectStack();
};

inline ostream& operator<<(ostream& o, const TObjectStack& stack) {return o << stack.LL;}

extern TObjectStack theStack;		// General runtime stack

extern void optimize(TInstructionList& il, TIntLL& linenums, const Tsymbol *symptr);

// uncommented 20 Nov 2007:
extern void Call(const Tobject& fn,
		 const TConstObjectPtr inputs[], const int Ninputs,
		 const TObjectPtr outputs[], const int Noutputs,
		 const Tsymbol* symptr=0, bool check_that_first_output_not_obligatory=false);

class Tprg {	// flatcode sequence with static objects
 private:
	TInstructionList il;	// the sequence of flatcode words
	TObjectLL ol;			// list of static objects
	TObjectLL* old_olptr;	// ptr to previous list of static objects
	TIntLL linenums;		// source line number information
	int nargin;				// (Maximum) number of input objects
	int nargout;			// (Maximum) number of output objects
	int nargin_min;			// Minimum number of input objects
	int nargout_min;		// Minimum number of output objects
	int nlocal;				// Number of local non-temporary objects
	int ntemp;				// Number of local temporary objects (introduced by codegen)
	int nstack;				// Stack frame size
	int inputellipsis;		// Nonzero if input arglist ends with ellipsis (...)
	int outputellipsis;		// Nonzero if output arglist ends with ellipsis
	Tchar *filename;		// source file name from this Tprg was compiled
	const Tsymbol* symptr;	// pointer to symbol whose value this Tprg is
	void CalcStack();
	void executeInstructionSequence();
	void MakeCurrent()
		{old_olptr=Toperand::objLLptr; Toperand::objLLptr = &ol;}	// Activate this Tprg to receive static objects into its ol
	void checkreadonlyoperand(const Toperand&, int) const;
	Tprg(const Tprg&);            // do not implement
	Tprg& operator=(const Tprg&); // do not implement
	friend void Call(const Tobject& fn,
					 const TConstObjectPtr inputs[], const int Ninputs,
					 const TObjectPtr outputs[], const int Noutputs,
					 const Tsymbol* symptr/*=0*/, bool check_that_first_output_not_obligatory/*=false*/);
	// 20 Nov 2007: commented out default args here, because extern prototype about 20 lines above had to be enabled
 public:
	Tprg(const Tchar *fn);
	Tprg(const Tchar *fn, int in, int out);
	Tprg(const Tchar *fn, int in, int out, int loc);
	Tprg(const Tchar *fn, int in, int out, int minin, int minout, int loc, const Tsymbol* sympointer=0);
	void SetInputEllipsisFlag() {inputellipsis=1; CalcStack();}
	void SetOutputEllipsisFlag() {outputellipsis=1; CalcStack();}
	int HasInputEllipsis() const {return inputellipsis;}
	int HasOutputEllipsis() const {return outputellipsis;}
	void JustMakeCurrent()
	    {Toperand::objLLptr = &ol;}	// Same as MakeCurrent but do not modify old_oldptr
	int FindLineNumber(int startPC=0) const;		// find line number after error
	// --- add functions, add new instructions to program
	void add(Tcode code) {il.add(code); linenums.append(global::lineno);}
	void add(Tcode code, const Toperand& op1) {il.add(code,op1); linenums.append(global::lineno);}
	void add(Tcode code, const Toperand& op1, const Toperand& op2) {il.add(code,op1,op2); linenums.append(global::lineno);}
	void add(Tcode code, const Toperand& op1, const Toperand& op2, const Toperand& op3)
		{il.add(code,op1,op2,op3); linenums.append(global::lineno);}
	void add(Tcode code, const Toperand& op1, const Toperand& op2, const Toperand& op3, const Toperand& op4)
		{il.add(code,op1,op2,op3,op4); linenums.append(global::lineno);}
	// --- another form of add function: first use add(code,N), then call append() exactly N times
	// --- this can be used to form instructions with arbitrary number of operands
	void add(Tcode code, int Noperands) {il.add(code,Noperands); linenums.append(global::lineno);}
	void append(const Toperand& op) {il.append(op);}
	// --- execution functions
	void execute(const TConstObjectPtr[],int, const TObjectPtr[],int, const Tsymbol*fnsymptr=0);
	// --- function to add (possibly) more temporaries
	void AllocTemporaries(int N) {if (N>ntemp) {ntemp=N; CalcStack();}}
	// --- inquiry functions
	int length() const {return il.length();}			// prg.length() returns number of flatcode words in prg
	Toperand& operator[](int i) const {return il[i];}	// prg[i] returns the ith flatcode word in prg
	int Nstatic() const {return ol.length();}			// prg.Nstatic() is the number of static objects
	TObjectPtr nthobject(int n) {return ol[n];}			// prg.nthobject(n) returns the nth static object in prg
	int Nargin() const {return nargin;}					// prg.Nargin() returns the maximum number of input objects for prg
	int Nargout() const {return nargout;}				// prg.Nargout() returns the maximum number of output objects for prg
	int NMinArgin() const {return nargin_min;}			// prg.NMinArgin() returns the minimum number of input objects for prg
	int NMinArgout() const {return nargout_min;}		// prg.NMinArgout() returns the minimum number of output objects for prg
	int Nlocal() const {return nlocal;}					// prg.Nlocal() returns the number of local objects in prg
	int Ntemp() const {return ntemp;}					// prg.Ntemp() returns the number of temporary local objects
	int StackFrameSize() const {return nstack;}			// prg.StackFrameSize() returns the number of objects in stack frame
	int PermanentFrameSize() const {return 1+nargin+nargout+nlocal+(inputellipsis!=0)+(outputellipsis!=0);}	// same as StackFrameSize but without temporaries
	const Tchar *FileName() const {return filename;}
	const Tsymbol& Symbol() const {return *symptr;}
	int MaxNops() const {return il.MaxNops();}
	Tcode LastOpcode() const;							// prg.LastOpcode() is the last opcode appended, or Klit if none is found
	// --- function to check that input args are not modified
	void checkreadonlyvars() const;
	// --- function to optimize flatcode
	void optimize() {::optimize(il,linenums,symptr);}
	// --- outputting
	friend ostream& operator<<(ostream& o, Tprg& prg);
	// --- destructor
	~Tprg();
};

class TCodeStack {
 private:
	struct TCodeStackNode {
		Tprg *prgptr;
		TCodeStackNode *next;
	};
	TCodeStackNode *p;

        TCodeStack(const TCodeStack&);            // do not implement
        TCodeStack& operator=(const TCodeStack&); // do not implement
 public:
	TCodeStack() : p(0) {}
	int isempty() const {return p==0;}
	void push(Tprg* ptr);
	Tprg* top() const;
	Tprg* pop();
};

extern TCodeStack theCodeStack;		// General stack of pointer to Tprgs

extern void PerformanceReport(int longflag=0);
