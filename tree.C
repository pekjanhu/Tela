/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "tree.H"
#endif
#include "tree.H"
#include <ctype.h>

typedef const Tchar *CP;

const Tchar *FuncData[] = {
	CP("PLUS"),CP("DIFFERENCE"),CP("MINUS"),
	CP("TIMES"),CP("QUOTIENT"),CP("POWER"),CP("MOD"),
	CP("EQ"),CP("NE"),CP("GT"),CP("LT"),CP("GE"),CP("LE"),
	CP("AND"),CP("OR"),CP("NOT"),
	CP("RANGE"),
	CP("REF"),CP("MREF"),CP("CALL"),CP("LIST"),
	CP("SET"),CP("BLOCK"),CP("DEFUN"),CP("PACKAGE"),
	CP("LOCAL"),CP("GLOBAL"),CP("FORMAL"),CP("FORMAL_ELLIPSIS"),CP("GOTO"),CP("LABEL"),
	CP("DISP"),
	CP("IF"),CP("WHILE"),CP("REPEAT"),CP("FOR"),CP("CONTINUE"),CP("BREAK"),CP("RETURN"),
	CP("ARRAY"),CP("EMPTY_ARRAY"),CP("APPEND"),
	CP("HARDNOP"),CP("NOP")
};

TNodePool theNodePool;

// --------- Tnode members and friends -----------

void* Tnode::operator new(size_t sz) {
	Tnode *r;
	if (theNodePool.Last().length < TNodeBlock::SIZE) {
		theNodePool.Last().length++;
		r = theNodePool.Last().pool +  theNodePool.Last().length-1;
	} else {
		//clog << "new: allocating TNodeBlock\n";
		TNodeBlock *p = new TNodeBlock;
		p->prev = &theNodePool.Last();
		theNodePool.Last().next = p;
		theNodePool.SetLast(p);
		theNodePool.nblocks++;
		p->length = 1;
		r = p->pool;
	}
	//r->next=r->list=0; r->kind=Kbuiltin; r->func=FLIST;
	return (void*)r;
}

ostream& operator<<(ostream& o, const Tnode& node) {
	if (node.list) {
		switch (node.kind) {
		case Kconstant:
			o << node.ObjectValue(); break;
		case Kbuiltin:
			o << (char*)FuncData[node.FunctionValue()] /*<< node.LineNumber()*/; break;
		case Kvariable:
			o << node.SymbolValue(); break;
		case Kslot:
			o << '$' << node.SlotValue(); break;
		case Kcolon:
			o << ':'; break;
		}
		o << '[';
		for (Tnode*p=node.list; p; p=p->next) {
			o << *p;
			if (p->next) o << ',';
		}
		o << ']';
	} else {
		switch (node.kind) {
		case Kconstant:
			o << node.ObjectValue(); break;
		case Kbuiltin:
			o << (char*)FuncData[node.FunctionValue()]; break;
		case Kvariable:
			o << node.SymbolValue(); break;
		case Kslot:
			o << '$' << node.SlotValue(); break;
		case Kcolon:
			o << ':'; break;
		}
	}
	return o;
}

static void DeleteObjects(TObjectListNode *objlist)
{
	TObjectListNode *p = objlist, *q;
//	int n = 0;
	while (p) {
//		cout << "Deleting object " << long(p->op) << "\n";
//		n++;
		delete p->op;
		q = p->nextobj;
		delete p;
		p = q;
	}
//	cout << "DeleteObjects: " << n << "\n";
};

void TNodePool::AddDeletedObjectsInList(TNodeBlock*p, int start, int stop, TObjectListNode *&objlist) {
	// Detect all nodes pointing to an object in node block p.
	// Add pointers to those objects in objlist.
	int i;
	for (i=start; i<stop; i++)
		if (p->pool[i].kind == Kconstant) {
			TObjectPtr op = &(p->pool[i].ObjectValue());
			if (op) {
				if (op->IsTemporary()) {
					op->UnflagTemporary();
					TObjectListNode *n = new TObjectListNode;
					n->op = op;
//					cout << "Adding object " << long(op) << " in objlist\n";
					n->nextobj = objlist;
					objlist = n;
				}
			}
		}
}

void TNodePool::release(TNodePoolState& s) {
	TNodeBlock *p,*q;
	TObjectListNode *objlist = 0;
	p = last;
	while (p != s.Ptr()) {
		q = p->prev;
		AddDeletedObjectsInList(p,0,p->length,objlist);
		delete p;
		nblocks--;
		p = q;
	}
	AddDeletedObjectsInList(s.Ptr(),s.Len(),s.Ptr()->length,objlist);
	DeleteObjects(objlist);
	last = s.Ptr();
	last->length = s.Len();
}

int TNodePool::NodesInUse() {
	return TNodeBlock::SIZE*(nblocks-1) + last->length;
}

#if 0

static void skipblanks(istream& i) {
	//clog<<"SKIPBLANKS\n";
	Tchar ch;
	for (;;) {
		i >> ch;
		if (!isspace(ch)) {i.putback(ch); break;}
	}
}

static Tnode* readatom(istream& i) {
	//clog << "READATOM\n";
	skipblanks(i);
	Tchar ch = i.peek();
	if (isalpha(ch)) {
		Tchar s[80];
		for (int j=0; ; j++) {
			i.get(ch);
			if (isalnum(ch)) s[j] = ch; else {i.putback(ch); break;}
		}
		s[j] = '\0';
		Tsymbol* ptr = theHT.add(s);
		return new Tnode(0,ptr,0);
	} else {
		Tobject* ptr = new Tobject;
		return new Tnode(0,ptr,0);
	}
}

Tnode* readtree(istream& i) {
	//clog<<"READTREE\n";
	Tchar ch;
	Tnode* nodeptr = readatom(i);
	skipblanks(i);
	if (i.peek()=='[') {
		i >> ch;
		int FirstIter = 1;
		Tnode*start=0,*prev=0,*newptr=0;
		for (;;) {
			skipblanks(i);
			if (i.peek()==']') {i>>ch; break;}
			newptr = readtree(i);
			if (FirstIter) {
				start = newptr;
				prev = newptr;
				FirstIter = 0;
			} else {
				prev->next = newptr;
				prev = newptr;
			}
			skipblanks(i);
			i >> ch;
			if (ch!=',' && ch!=']') {err<<"Syntax error in readtree()\n"; error();}
			if (ch==']') break;
		}
		nodeptr->list = start;
	}
	return nodeptr;
}

#endif


