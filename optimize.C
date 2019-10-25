/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2000 Pekka Janhunen
 */

#include "prg.H"

static const Tsymbol *symboli = 0;

// A flatcode program can be represented as a sequence of basic blocks (BBs).
// Each basic blocks ends with a jump or branch instruction.

inline int is_jump_instruction(Tcode c)
{
	return c==Fjngt || c==Fjnlt || c==Fjnge || c==Fjnle ||
		c==Fjmp || c==Fincjmp || c==Fjif || c==Fjifn;
}

inline int is_tilable_instruction(Tcode c)
{
	return c==Fabs || c==Fadd || c==Fsub || c==Fmul || c==Fdiv  || c==Fmod || c==Fneg || c==Finc || c==Fdec;
}

static int member(int x, const TIntLL& L)
{
	int i;
	for (i=0; i<L.length(); i++) if (x == L[i]) return 1;
	return 0;
}

static int member(Toperand x, const TInstructionList1& L)
{
	int i;
	for (i=0; i<L.length(); i++) if (x == L[i]) return 1;
	return 0;
}

class Tinstruction {
private:
	Toperand *ptr;
	int n;
	int linenum;
	void fromtab(const Toperand tab[]) {
		ptr = new Toperand [n];
		for (int i=0; i<n; i++) ptr[i] = tab[i];
	}		
public:
	Tinstruction(const Toperand tab[], int nn) : n(nn), linenum(-3) {
		fromtab(tab);
	}
	Tinstruction(const Tinstruction& instr) : n(instr.n), linenum(instr.linenum) {
		fromtab(instr.ptr);
	}
	Tinstruction& operator=(const Tinstruction& instr) {
		delete [] ptr;
		n = instr.n;
		linenum = instr.linenum;
		fromtab(instr.ptr);
		return *this;
	}
	void set_linenumber(int nn) {linenum=nn;}
	Toperand operator()(int i) const {return ptr[i];}
	Toperand& operator[](int i) {return ptr[i];}
	int linenumber() const {return linenum;}
	int length() const {return n;}
	~Tinstruction() {delete [] ptr;}
};

ostream& operator<<(ostream& o, const Tinstruction& i)
{
	o << "Tinstruction(" << i.linenumber() << ": " << i(0) << " ";
	for (int j=1; j<i.length(); j++) {
		o << i(j);
		if (j<i.length()-1) o << ',';
	}
	o << ')';
	return o;
}

class TBBlist;

class TBasicBlock {
private:
	struct TInstructionListNode {
		Tinstruction instr;
		TInstructionListNode *next, *prev;
		TInstructionListNode(const Tinstruction& i) : instr(i), next(0), prev(0) {}
	};
	TInstructionListNode *first, *last;		// pointer to first and last instruction in the BB
	int len;						// number of instructions
	Tinstruction *jumpinstr;		// pointer to terminating jump instruction, or 0 if no terminating jump
	TBasicBlock *jump_target;		// in case jumpinstr is not 0, jump_target points to some BB (or is 0 if jump exits function)
	int startaddr;					// used in BBlist construction phase: address of first instruction of this BB
	void copyfrom(const TBasicBlock& bb);
	int first_occurrence_is_input(const Toperand& op, TInstructionListNode *tilestart, TInstructionListNode *tileend);
public:
	TBasicBlock() : first(0),last(0),len(0),jumpinstr(0),jump_target(0),startaddr(0) {}
	TBasicBlock(const TBasicBlock& bb) {copyfrom(bb);}
	TBasicBlock& operator=(const TBasicBlock& bb) {clear(); copyfrom(bb); return *this;}
	void clear();					// delete everything, make basic block empty (same state as after default constructor)
	void add(const Tinstruction& instr);	// add new instruction to end of BB
	int length() const {return len;}		// number of instructions in BB, excluding possible terminating jump instr.
	int Ncodewords() const;					// number of codewords (> number of instructions)
	void set_startaddress(int i) {startaddr = i;}
	int startaddress() const {return startaddr;}
	void set_jumpinstr(const Tinstruction& j) {jumpinstr = new Tinstruction(j);}
	Tinstruction *jumpinstruction() const {return jumpinstr;}
	void to_flatcode(TInstructionList& il, TIntLL& linenums, int& maxNops) const;
	void loop_tiling();
	~TBasicBlock() {clear();}
	friend ostream& operator<<(ostream& o, const TBasicBlock& bb);
	friend void fix_jump_targets(TBBlist& bblist, int maxaddr);
};

void TBasicBlock::copyfrom(const TBasicBlock& bb)
{
	first = last = 0; jumpinstr = 0;
	jump_target = 0;
	len = 0;
	TInstructionListNode *p;
	for (p=bb.first; p; p=p->next) {
		add(p->instr);
	}
	if (bb.jumpinstr) jumpinstr = new Tinstruction(*bb.jumpinstr);
	startaddr = bb.startaddr;
}

ostream& operator<<(ostream& o, const TBasicBlock& bb)
{
	TBasicBlock::TInstructionListNode *p;
	o << "BasicBlock/startaddr=" << bb.startaddr << "[\n";
	for (p=bb.first; p; p=p->next) o << p->instr << '\n';
	if (bb.jumpinstr) o << '*' << *bb.jumpinstr << '\n';
	o << "]\n";
	return o;
}

void TBasicBlock::add(const Tinstruction& instr)
{
	TInstructionListNode *n = new TInstructionListNode(instr);
	n->next = 0;
	n->prev = last;
	if (last) {
		last->next = n;
	} else {
		first = n;
	}
	last = n;
	len++;
}

void TBasicBlock::clear()
{
	TInstructionListNode *p;
	while (first) {
		p = first;
		first = p->next;
		delete p;
	}
	if (jumpinstr) delete jumpinstr;
	first = last = 0; len = 0;
	jumpinstr = 0;
}

int TBasicBlock::Ncodewords() const
{
	TInstructionListNode *p;
	int result = 0;
	for (p=first; p; p=p->next) result+= p->instr.length();
	if (jumpinstr) result+= jumpinstr->length();
	return result;
}

INLINE int is_input_slot(Tcode c, int Noperands, int slot)
{
	if (c == Fadd || c == Fsub || c == Fmul || c == Fdiv || c == Fmod) {
		if (Noperands == 3)
			return (slot >= 2);
		else
			return 1;
	} else if (c == Fneg || c == Fabs || c == Finc || c == Fdec) {
		if (Noperands == 2)
			return (slot == 2);
		else
			return 1;
	} else {
		cerr << "optimize.C:is_input_slot: internal error\n";
		return 0;
	}
}

int TBasicBlock::first_occurrence_is_input(const Toperand& op, TInstructionListNode *tilestart, TInstructionListNode *tileend)
{
	TInstructionListNode *q;
	for (q=tilestart; q->prev!=tileend; q=q->next) {
		if (q->instr.length() <= 1) continue;
		int i;
		for (i=1; i<q->instr.length(); i++) if (q->instr(i) == op) {
			return is_input_slot(q->instr(0).opcode(),q->instr.length()-1,i);
		}
	}
	cerr << "optimize.C:first_occurrence_is_input: internal inconsistency\n";
	return 0;
}

void TBasicBlock::loop_tiling()
{
//	cout << "loop_tiling\n";
	TInstructionListNode *p,*tilestart=0,*tileend=0;
	int tilelen = 0;
	for (p=first; p; p=p->next) {
		if (is_tilable_instruction(p->instr(0).opcode())) {
			if (tilelen > 0) {
				tilelen++;
			} else {
				tilestart = p;
				tilelen = 1;
			}
		} else {
			if (tilelen > 0) {
				tileend = p->prev;
				if (tilelen > 3) {
//					cout << "got tile in ";
//					if (symboli) cout << *symboli; else cout << "<nosymbol>";
//					cout << ", length=" << tilelen << "\n";
					// Now we have tile whose first instruction is tilestart and last is tileend
					// Insert VLOOP NIOs,a1,...aN,b1,...,bM before tilestart where a1,...,aN are all
					// input/output objects appearing inside the tile, and b1,...,bM are all pureinput objects.
					// Insert ENDV (-x) at the end where x is
					// the number of codewords inside the tile.

					// (1) Gather all output slots in list Outputs
					TInstructionList1 Outputs(0);
					TInstructionListNode *q;
					for (q=tilestart; q->prev!=tileend; q=q->next) {
						if (q->instr.length() <= 1) continue;
						if ((q->instr(1).kind() == Ksym || q->instr(1).kind() == Kpar) && !member(q->instr(1),Outputs))
							Outputs.append(q->instr(1));
					}
					// (2) Pass through Outputs. Test for each, if its first occurrence in the tile is
					// input slot. If it is, add operand to list IOs. If it is not, add it to pureoutputs.
					TInstructionList1 IOs(0), pureoutputs(0);
					int i;
					for (i=0; i<Outputs.length(); i++) {
						if (first_occurrence_is_input(Outputs[i],tilestart,tileend))
							IOs.append(Outputs[i]);
						else
							pureoutputs.append(Outputs[i]);
					}
					// (3) Gather all non-output slots (i.e., s>=2) to list pureinputs, however excluding
					// those that are already in IOs or pureoutputs
					TInstructionList1 pureinputs(0);
					int ncodewords = 0;
					for (q=tilestart; q->prev!=tileend; q=q->next) {
						if (q->instr.length() <= 2) continue;
						for (i=2; i<q->instr.length(); i++)
							if ((q->instr(i).kind() == Ksym || q->instr(i).kind() == Kpar)
								&& !member(q->instr(i),IOs) && !member(q->instr(i),pureoutputs)
								&& !member(q->instr(i),pureinputs))
								pureinputs.append(q->instr(i));
						ncodewords+= q->instr.length();
					}
					const int Npureinput = pureinputs.length();
					const int NIOs = IOs.length();
					const int n = 1 + NIOs + Npureinput;
					if (n < MAX_NOPERANDS && NIOs == 0) {		// FOR NOW, WE ONLY OUTPUT VLOOP if NIOs==0 !!!
						// Insert VLOOP. It is of the form VLOOP NIOs,IOs1,...IOsN,pureinput1,...,pureinputM
						Toperand *operands = new Toperand [n+1];
						operands[0] = Toperand(Fvloop,n);
						operands[1] = Toperand(TLiteralInt(NIOs));
						for (i=0; i<NIOs; i++) operands[2+i] = IOs[i];
						for (i=0; i<Npureinput; i++) operands[2+NIOs+i] = pureinputs[i];
						const Tinstruction vloop_instr(operands,n+1);
						operands[0] = Toperand(Fendv,1);
						operands[1] = Toperand(TLiteralInt(-ncodewords));
						const Tinstruction endv_instr(operands,2);
						TInstructionListNode *vloopnode = new TInstructionListNode(vloop_instr);
						TInstructionListNode *endvnode = new TInstructionListNode(endv_instr);
						vloopnode->next = tilestart;
						vloopnode->prev = tilestart->prev;
						if (tilestart->prev) {
							tilestart->prev->next = vloopnode;
						} else {
							first = vloopnode;
						}
						tilestart->prev = vloopnode;
						endvnode->prev = tileend;
						endvnode->next = tileend->next;
						if (tileend->next) {
							tileend->next->prev = endvnode;
						} else {
							last = endvnode;
						}
						tileend->next = endvnode;
						len+= 2;		// because added 2 new instructions in this BB (VLOOP..ENDV)
					} /*else
						cout << "--- ignoring tile because operandlist too long (" << n << ")\n";*/
				}
				tilestart = tileend = 0;
				tilelen = 0;
			}
		}
	}
}

#ifndef MAXINT
#  define MAXINT (~(1 << (8*sizeof(int)-1)))
#endif

void TBasicBlock::to_flatcode(TInstructionList& il, TIntLL& linenums, int& maxNops) const
{
	TInstructionListNode *p;
	for (p=first; p; p=p->next) {
		for (int i=0; i<p->instr.length(); i++) il.append(p->instr(i));
		linenums.append(p->instr.linenumber());
		if (p->instr.length()-1 > maxNops) maxNops = p->instr.length()-1;
	}
	if (jumpinstr) {
		il.append((*jumpinstr)(0));
		if (jump_target) {
			il.append(Toperand(Klit,jump_target->startaddress()));
		} else {
			il.append(Toperand(Klit,MAXINT));
		}
		for (int i=2; i<jumpinstr->length(); i++) {
			il.append((*jumpinstr)(i));
		}
		if (jumpinstr->length()-1 > maxNops) maxNops = jumpinstr->length()-1;
		linenums.append(jumpinstr->linenumber());
	}
}

class TBBlist {
private:
	struct TBBListNode {
		TBasicBlock bb;
		TBBListNode *next;
		TBBListNode(const TBasicBlock& b) : bb(b), next(0) {}
	};
	TBBListNode *first,*last;
	int len;
public:
	TBBlist() : first(0),last(0) {}
	void add(const TBasicBlock& bb);
	int length() const {return len;}
	void to_flatcode(TInstructionList& il, TIntLL& linenums) const;
	void basic_block_optimization(void (TBasicBlock::*optimizer)());
	~TBBlist() {TBBListNode *p; while (first) {p=first; first=p->next; delete p;}}
	friend ostream& operator<<(ostream& o, const TBBlist& bblist);
	friend void fix_jump_targets(TBBlist& bblist, int maxaddr);
};

ostream& operator<<(ostream& o, const TBBlist& bblist)
{
	TBBlist::TBBListNode *p;
	for (p=bblist.first; p; p=p->next) o << p->bb;
	return o;
}

void TBBlist::add(const TBasicBlock& bb)
{
	TBBListNode *n;
	n = new TBBListNode(bb);
	n->next = 0;
	if (last) {
		last->next = n;
	} else {
		first = n;
	}
	last = n;
	len++;
}

void TBBlist::to_flatcode(TInstructionList& il, TIntLL& linenums) const
{
	TBBListNode *p;
	// (1) Fix startaddresses
	int sa = 0;
	for (p=first; p; p=p->next) {
//		if (sa != p->bb.startaddress()) cout << "sa=" << sa << ", startaddr=" << p->bb.startaddress() << "\n";
		p->bb.set_startaddress(sa);
		sa+= p->bb.Ncodewords();
	}
	// (2) Turn each basic block to flatcode
	int maxNops = 0;
	for (p=first; p; p=p->next) {
		p->bb.to_flatcode(il,linenums,maxNops);
	}
	il.set_maxNops(maxNops);
}

void TBBlist::basic_block_optimization(void (TBasicBlock::*optimizer)())
{
	TBBListNode *p;
	for (p=first; p; p=p->next) {
		(p->bb.*optimizer)();
	}
}

void fix_jump_targets(TBBlist& bblist, int maxaddr)
{
	// Set jump targets in basic blocks to point to proper BBs
	TBBlist::TBBListNode *p,*q;
//	cout << "fix_jump_targets: maxaddr=" << maxaddr << "\n";
	for (p=bblist.first; p; p=p->next) {
		if (p->bb.jumpinstr) {
			const int t = ((*p->bb.jumpinstr)(1)).literal();
//			cout << "- t = " << t << "\n";
			if (t >= maxaddr) continue;
			bool found = false;
			for (q=bblist.first; q; q=q->next) {
				if (q->bb.startaddress() == t) {
					p->bb.jump_target = &(q->bb);
					found = true;
					break;
				}
			}
			if (!found) cerr << "jump target " << t << " not found\n";
		}
	}

}

void flatcode_to_BBlist(const TInstructionList& il, const TIntLL& linenums, TBBlist& bblist)
// Takes as inputs flatcode instruction list il and associated line number information.
// Returns the corresponding basic block list.
// Algorithm: A basic block (BB) is a sequence of instructions which contains no jump instruction
// (except the last instruction can be a jump instruction) and to which no jump instruction jumps to.
// Thus, we know that every jump instruction ends a BB, and every instruction which is a target
// of some jump instruction starts a BB. We first make an integer list (TIntLL) of addresses of all
// flatcode words which are targets of some jump instruction. Then we pass through the flatcode
// once, knowing that each branch target starts a BB, as does each jump instruction.
{
	// (1) Find branch target addresses to 'targets'
	TIntLL targets(0);
	int i;
	for (i=0; i<il.length(); i++) {
		if (il[i].IsOpcode()) {
			const Tcode c = il[i].opcode();
			if (is_jump_instruction(c)) {
				const int t = il[i+1].literal();
				if (t>0 && t<il.length() && !member(t,targets)) targets.append(t);
			}
			i+= il[i].Noperands();
		} else {
			err << "Internal error in optimize/flatcode_to_BBList: syntax error in instruction list\n";
			error();
		}
	}
//	cout << "targets = " << targets << "\n";
	TBasicBlock bb;
	int instr_number = 0;
	for (i=0; i<il.length(); i++) {
		if (il[i].IsOpcode()) {
			const int Nop = il[i].Noperands();
			Tinstruction instr(&il[i],Nop+1);
			instr.set_linenumber(linenums[instr_number]);
			const Tcode c = il[i].opcode();
//			cout << "i = " << i << ", " << instr << "\n";

			// VIRHE: Jos JUMP kasky on myos jump target, BB:sta pitaisi tulla
			// nollan mittainen, eli tyhja + jump. Esimerkkibugi: ffttest.t
			// Alla oleva toimii vain jos BB ei ole tyhja.
			
			if (member(i,targets)) {
//				cout << "end BB with jump targ\n" << flush;
				// begin new basic block, without inserting jump instruction into it
				if (bb.length() > 0 || bb.jumpinstruction())
					bblist.add(bb);
				bb.clear();
				if (is_jump_instruction(c)) {
					bb.set_jumpinstr(instr);
					bb.set_startaddress(i);
					bblist.add(bb);
					bb.clear();
					bb.set_startaddress(i+Nop+1);
				} else {
					bb.add(instr);
					bb.set_startaddress(i);
				}
			} else if (is_jump_instruction(c)) {
				// end current basic block with jump instruction
//				cout << "end BB with jump instr\n" << flush;
				bb.set_jumpinstr(instr);
				bblist.add(bb);
				bb.clear();		// make bb empty list again (start new basic block)
				bb.set_startaddress(i+Nop+1);
			} else {
				// append instr to current basic block
				bb.add(instr);
			}
			i+= Nop;
			instr_number++;
		} else {
			err << "Internal error 2 in optimize/flatcode_to_BBList: syntax error in instruction list\n";
			error();
		}
	}
	if (bb.length() > 0 || bb.jumpinstruction())
		bblist.add(bb);
	fix_jump_targets(bblist,il.length());
//	cout << bblist;
}

void optimize(TInstructionList& il, TIntLL& linenums, const Tsymbol *symptr)
{
	symboli = symptr;
	/*
	cout << "calling optimize for ";
	if (symptr)
		cout << *symptr;
	else
		cout << "<nosymbol>";
	cout << ", " << il.length() << " codewords, "
		 << linenums.length() << " instructions\n";
	*/
	// (1) Turn flatcode into basic block list (bblist)
	TBBlist bblist;
	flatcode_to_BBlist(il,linenums,bblist);
	// (2) Optimize bblist
	if (flags::optimize) bblist.basic_block_optimization(&TBasicBlock::loop_tiling);
	// (3) Turn bblist back into flatcode and line number information
	il.clear();
	linenums = 0;
	bblist.to_flatcode(il,linenums);
	/*
	TInstructionList newil; TIntLL newlinenums(0);
	bblist.to_flatcode(newil,newlinenums);
	*/
}
