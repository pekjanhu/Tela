/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "lisp.H"
#endif
#include "lisp.H"

Tnode* List(Tfunc h, Tnode*a) {
	Tnode *a1 = new Tnode(*a);
	a1->next = 0;
	return new Tnode(a1,h,0);
}

Tnode* List(Tfunc h, Tnode*a, Tnode*b) {
	Tnode *a1 = new Tnode(*a);
	Tnode *b1 = new Tnode(*b);
	b1->next=0;
	a1->next=b1;
	return new Tnode(a1,h,0);
}

Tnode* List(Tfunc h, Tnode*a, Tnode*b, Tnode*c) {
	Tnode *a1 = new Tnode(*a);
	Tnode *b1 = new Tnode(*b);
	Tnode *c1 = new Tnode(*c);
	c1->next=0;
	b1->next=c1;
	a1->next=b1;
	return new Tnode(a1,h,0);
}

Tnode* Nth(Tnode*L, int N) {
	Tnode *p = First(L);
	for (int i=0; i<N; i++) p=p->next;
	return p;
}

int Length(Tnode*L) {
	int n=0;
	if (!L) return 0;
	for (Tnode*p=L->list; p; p=p->next) n++;
 	return n;
}

Tnode* CopyList(Tnode*L) {
	Tnode *n,*start=0,*old=0;
	int FirstTime=1;
	if (!L) return L;
	for (Tnode*p=L->list; p; p=p->next) {
		n = new Tnode(*p);
		n->next = 0;
		if (FirstTime) {
			start = n;
			FirstTime = 0;
		} else {
			old->next = n;
		}
		old = n;
	}
	Tnode *result = new Tnode(*L);
	result->list = start;
	result->next = 0;
	return result;
}

Tnode* Append(Tnode*L1, Tnode*L2) {
	//clog << "Append(" << *L1 << "," << *L2 << ") = ";
	if (!L1 || !L1->list) return L2;
	Tnode *newlist = CopyList(L1);
        Tnode *p;
	for (p=newlist->list; p->next; p=p->next);
	p->next = L2->list;
	//clog << *newlist << '\n';
	return newlist;
}

Tnode* Append(Tnode*L1, Tnode*L2, Tnode*L3) {
	return Append(L1,Append(L2,L3));
}

