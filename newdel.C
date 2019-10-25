/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
  This is a general-purpose replacement for the C++ new and delete operators.
  If WITHIN_TELA is not defined, this file should compile and link with any C++ program.

  This file was written in June 1996 to work around the buggy malloc/free implementation
  of SGI IRIX 5.3 and 6.2.

  If your C++ system has bug-free new and delete operators, then this file is unnecessary
  (unless you find it, e.g., faster, which is unlikely however).

  The Tela Makefile is set up so that if NEWDEL_O is defined to be newdel.o, then this
  file is compiled and linked in libtela.a, otherwise not. In addition, the -DTELASPECIFIC_NEWDELETE
  compiler flag must be given, as well as -DWITHIN_TELA. If the file is compiled and linked in,
  but TELASPECIFIC_NEWDELETE is not defined, then the file is empty and the system new/delete
  are still in use.

  Customization:
  - SORTED_FREELIST should always be 1
  - MERGE_ALWAYS may be 0 or 1; this setting may affect performance. If you want
    experiment with performance you can try both options.
	If MERGE_ALWAYS is 1, to consequent free blocks are always merged together in the Freelist,
	possibly resulting in shorter Freelist and thus faster operations but possibly also internal
	fragmentation and thus larger memory requirements. (It could also be possible to find
	counterexamples to this, however.) The default is 1.
    If MERGE_ALWAYS is 0, merging is done only if a block is freed when blocks both on the left and
    on the right are already free (or empty; i.e. in case of pool boundary block).
  - To use leak checking you can define LEAK_CHECKED as 1. This is relatively fast.
    In this way you can find any memory area overflow in Tela or your own linked-in code 
	which uses new/delete. The performance overhead is not very large, but it is off by default.
  - If you really suspect an error in this package you can define DEBUG as 1, or even 2 or 3.
    If DEBUG is 1, the checksum consistency checks are done. If DEBUG is 2 or 3 (or more),
	more checks are done, and more diagnostic output to stderr is written.
	DEBUG=1 clearly hurts performance but is tolerable, DEBUG>1 may be very slow because
	all allocated blocks and the Freelist are checked every time.
  - If you want to do some statistics with this package, you can get logging information in
    file "malloc.log" by defining LOGGING as 1. The file will contain the size of every malloc
	call as well as how many search steps were required in Freelist traversal before a suitable
	block was found. (Works only if SORTED_FREELIST is 1.). The performance penalty is not very large.
	The file is not flushed, however, so unless your compiler does the flusing and closing for you,
	you can miss one bufferfull at the end (probably 8K). Typically the file is so large, however,
	that this is not a problem.
*/

#ifndef WITHIN_TELA
#  define TELASPECIFIC_NEWDELETE
#endif

#ifdef TELASPECIFIC_NEWDELETE

#include <fstream.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#ifdef WITHIN_TELA
#  include "common.H"
#endif

#define SORTED_FREELIST 1	/* should be 1 */
#define FAST_FREELIST 1		/* usually should be 1 (very small memory overhead, makes Freelist traversal faster) */
#define MERGE_ALWAYS 1		/* see comment above */
#define LEAK_CHECKED 0		/* use 1 to have leak-checked versions (slows down little bit) */
#define DEBUG 0				/* 0: no checks, 1: all checks, 2: checks + output, 3: checks + more output. Slow! */
#define LOGGING 0			/* logging to file malloc.log */
#define ALIGN_TYPE double	/* the worst possible alignment on host machine, all known machines (1996) allow double here */
/* malloc.log contains lines with two numbers, separated by space: the size of malloced block
   and how many pointers in Freelist was followed before a match was found.*/

// mempadd is relevant only if LEAK_CHECKED is 1 (otherwise it is defined to be 0 below)
const int ptrsz = sizeof(ALIGN_TYPE);			// must be at least as large as sizeof(size_t)
const int mempadd = 8*sizeof(ALIGN_TYPE);		// this many bytes of intact memory above and below malloc'ed pool
const size_t basic_poolsize = 1048576;
//const size_t basic_poolsize = 524288;
//const size_t basic_poolsize = 80000;
//const size_t basic_poolsize = 20000;

#if !LEAK_CHECKED
#  define mempadd 0
#endif

#if DEBUG
// If DEBUGging, we may encounter an error, and then we exit.
// But this exit calls global destructors and may yield into more errors,
// maybe even recursively. Therefore we call _exit, which does not know
// about destructors, and just plainly exits.
   extern "C" void _exit(int);
#  define exit(x) _exit(x)
#  define INLINE		/* No heavy inlining when debugging, to simplify compiler's task */
#else
#  define INLINE inline
#endif

//class TFreqTableEntry;
struct Theader;
struct TPoolEntry;

static void errorout(const char *msg)
{
	write(2,msg,strlen(msg));
}

#if 0
class TFreqTableEntry {
private:
	size_t sz;
	size_t freq;
	size_t Nfree;
	size_t Nalloc;
	void *firstfree;
	TFreqTableEntry *nextlargest;
public:
};
#endif

struct Theader {
	Theader *left, *right;		// pointer to left block and right block
	Theader *next, *prev;		// pointer to next and previous free block, or 0 if this block is in use
#if FAST_FREELIST
	Theader *nextlargest;		// pointer to successor block which is greater in size, or 0
	Theader *prevsmallest;		// pointer to predecessor block which is smaller in size, or 0
#endif
	size_t sz;					// size (in bytes) of the current block (excluding header)
	size_t isfree;				// Whether this block is free (Boolean)
	// sizeof(Theader) should be a multiple of sizeof(size_t), that is why isfree is of type size_t also
	void check(int id);			// Check the checksum
	void checkptrs(int id);		// Check mutual consistency of various pointers
	void ComputeChecksum();		// (Re)compute the checksum. Call this after modifying public members
private:
#if DEBUG
	size_t checksum;
	size_t filler;				// make number of data members even so that sizeof(Theader) is divisible by 2*sizeof(size_t)
#endif
	size_t Checksum() const;
};

inline size_t Theader::Checksum() const {
	return size_t(next) + size_t(prev) + size_t(left) + size_t(right) + size_t(sz);
}

inline void Theader::ComputeChecksum()
{
#if DEBUG
	checksum = Checksum();
#endif
}

struct TPoolEntry {
	TPoolEntry *next;	// pointer to next malloced pool
	size_t sz;			// size of pool in bytes, including headering TPoolEntry
	size_t magic;
	size_t checksum;
};

#define POOLMAGIC 0xBACDBACD

class Tnewdel {
private:
	Theader *Freelist;
	TPoolEntry *pools;
	void *fromFreelist(Theader*p, int id=0);
	Theader *toFreelist(Theader*p, int nomerge=0);
	void *divideBlock(Theader*p, size_t sz1);
	void CheckAll();
public:
	// The constructor is empty, this object must be initialized by calling init() explicitly
	void init(int id=0);
	Tnewdel() {}
	void *malloc(size_t sz);
	void free(void *ptr);
	void *Align(char *ptr);
	void ShowFreelist(ostream& o);
	void DebugOutputFreelist(Theader *markptr=0);
#if LOGGING
	FILE *log;
#endif
	~Tnewdel() {}
};

static Tnewdel newdel;

INLINE void Theader::checkptrs(int id)
{
#if DEBUG && FAST_FREELIST
	char S[200];
	int errors = 0;
	if (next != 0 && next->prevsmallest == 0 && next->sz != sz) {
		sprintf(S,"*** newdel.C:Theader::checkptrs: next!=0, next->prevsmallest==0, (next_sz=%ld)!=(sz=%ld) at %d\n",
				(long int)next->sz,(long int)sz,id);
		errorout(S);
		errors++;
	}
	if (prevsmallest == 0 && prev != 0 && prev->sz != sz) {
		sprintf(S,"*** newdel.C:Theader::checkptrs: prev!=0, prevsmallest==0, (prev->sz=%ld)!=(sz=%ld) at %d\n",
				(long int)prev->sz,(long int)sz,id);
		errorout(S);
		errors++;
	}
	if (errors) {
		sprintf(S," next=%ld, prev=%ld, left=%d, right=%d\n",
				(long int)next, (long int)prev, (long int)left, (long int)right);
		errorout(S);
		sprintf(S,"  nextlargest=%ld, prevsmallest=%d\n",(long int)nextlargest,(long int)prevsmallest);
		errorout(S);
		sprintf(S," sz=%ld, isfree=%d\n",sz,isfree);
		errorout(S);
		newdel.DebugOutputFreelist(this);
		exit(1);
	}
#endif
}

INLINE void Theader::check(int id)
{
#if DEBUG
	checkptrs(id);
	char S[200];
	int errors = 0;
	if (size_t(this) % sizeof(ALIGN_TYPE)) {
		sprintf(S,"*** newdelC:Theader: Misaligned object at %d\n",id);
		errorout(S);
		errors++;
	}
	if (sz % sizeof(ALIGN_TYPE)) {
		sprintf(S,"*** newdel.C:Theader: Misaligned sz %ld at %d\n",(long int)sz,id);
		errorout(S);
		errors++;
	}
	size_t s = Checksum();
	if (s != checksum) {
		sprintf(S,"*** newdel.C:Theader: Checksum failure at %d\n",id);
		errorout(S);
		errors++;
	}
	if (errors) {
		sprintf(S," next=%ld, prev=%ld, left=%d, right=%d\n",
				(long int)next, (long int)prev, (long int)left, (long int)right);
		errorout(S);
		sprintf(S," sz=%ld, isfree=%d\n",sz,isfree);
		errorout(S);
		exit(1);
	}
#endif
}

void Tnewdel::init(int id)
{
	char s[80];
#if DEBUG
	sprintf(s,"Tnewdel::init, id=%d\n",id);
	errorout(s);
#endif
	Freelist = 0; pools = 0;
	if (sizeof(ALIGN_TYPE) % sizeof(size_t) != 0) {
		errorout("Tnewdel::init warning: sizeof(ALIGN_TYPE) is not divisible by sizeof(size_t)\n");
	}
	if (sizeof(Theader) % sizeof(ALIGN_TYPE) != 0) {
		errorout("Tnewdel::init warning: sizeof(Theader) is not divisible by sizeof(ALIGN_TYPE)\n");
	}
	if (sizeof(TPoolEntry) % sizeof(ALIGN_TYPE) != 0) {
		errorout("Tnewdel::init warning: sizeof(TPoolEntry) is not divisible by sizeof(ALIGN_TYPE)\n");
	}
}

void Tnewdel::ShowFreelist(ostream& o)
{
	Theader *p;
	size_t oldsz = 0;
	size_t sz=0, freq=0;
	int totcnt, cnt;
	for (totcnt=0,cnt=0,p=Freelist; p; p=p->next,totcnt++) {
		sz = p->sz;
		if (sz == oldsz) {
			freq++;
		} else {
			if (totcnt > 0) {
				o << ' ';
				if (freq != 1) o << freq << '*';
				o << oldsz;
				cnt++;
			}
			freq = 1;
		}
		oldsz = sz;
	}
	o << ' ';
	if (freq != 1) o << freq << '*';
	o << sz;
	o << "\n" << totcnt << " entries total, " << cnt << " different sizes.\n";
#if FAST_FREELIST
	o << "'Header' elements only: ";
	for (p=Freelist; p; p=p->nextlargest)
		o << ' ' << p->sz;
	o << '\n';
#endif
}

void Tnewdel::DebugOutputFreelist(Theader *markptr)
{
#if FAST_FREELIST
	Theader *p;
	for (p=Freelist; p; p=p->next) {
		char s[200];
		sprintf(s,"sz=%ld, prevsmallest=%ld, nextlargest=%ld",
				long(p->sz), long(p->prevsmallest ? p->prevsmallest->sz : 0),
				long(p->nextlargest ? p->nextlargest->sz : 0));
		errorout(s);
		if (p->prevsmallest) {
			if (!p->prevsmallest->isfree) errorout(" **PREV is not free");
		}
		if (p->nextlargest) {
			if (!p->nextlargest->isfree) errorout(" ** NEXT is not free");
		}
		if (markptr == p) errorout(" <--");
		errorout("\n");
	}
#endif
}

void Tnewdel::CheckAll()
{
#if DEBUG > 2
	errorout("- CheckAll\n");
	errorout("-- Checking pool list\n");
#endif
	TPoolEntry *pool;
	Theader *p;
	int Nfree = 0;
	char s[200];
	int poolnum = 0;
	for (pool=pools; pool; pool=pool->next) {
		poolnum++;
		if (pool->magic != POOLMAGIC) {
			sprintf(s,"*** Bad magic number %ud on pool %d\n",pool->magic,poolnum);
			errorout(s);
			exit(1);
		}
		if (pool->checksum != size_t(pool->next) + size_t(pool->sz)) {
			sprintf(s,"*** Bad checksum in pool %d\n",poolnum);
			errorout(s);
			exit(1);
		}
		p = (Theader *)(pool + 1);
		int N = 0;
		while (p) {
			p->check(3);
			if (p->isfree) Nfree++;
			p = p->right;
			N++;
		}
#if DEBUG > 2
		sprintf(s,"--- pool %d: %d blocks\n",poolnum,N);
		errorout(s);
#endif
	}
#if DEBUG > 2
	sprintf(s,"-- %d pools, Nfree=%d, Checking Freelist\n",poolnum,Nfree);
	errorout(s);
#endif
	Nfree = 0;
	for (p=Freelist; p; p=p->next) {
		if (p->next) {
			if (p->next->prev != p) {
				errorout("*** newdel.C:CheckAll inconsistency: Freelist next/prev pointers\n");
				exit(1);
			}
		}
		if (p->prev) {
			if (p->prev->next != p) {
				errorout("*** newdel.C:CheckAll inconsistency: Freelist prev/next pointers\n");
				exit(1);
			}
		}
#if FAST_FREELIST
		if (p->nextlargest) {
			if (p->nextlargest->prevsmallest != p) {
				errorout("*** newdel.C:CheckAll inconsistency: Freelist nextlargest/prevsmallest pointers\n");
				exit(1);
			}
		}
		if (p->prevsmallest) {
			if (p->prevsmallest->nextlargest != p) {
				errorout("*** newdel.C:CheckAll inconsistency: Freelist prevsmallest/nextlargest pointers\n");
				char s[180];
				sprintf(s,"next=%ld, prev=%ld, p=%ld, nextlargest=%ld, prevsmallest=%ld, prevsmallest->nextlargest=%ld, sz=%ld, isfree=%d\n",
						long(p->next),long(p->prev),long(p),long(p->nextlargest),long(p->prevsmallest),
						long(p->prevsmallest->nextlargest), long(p->sz), p->isfree);
				errorout(s);
				sprintf(s,"prevsmallest->sz=%ld, prevsmallest->isfree=%d\n",
						long(p->prevsmallest->sz),p->prevsmallest->isfree);
				errorout(s);
				DebugOutputFreelist();
				exit(1);
			}
		}
		if (p->prev) {
			if (p->prev->sz != p->sz) {
				if (p->prevsmallest == 0) {
					errorout("*** newdel.C: CheckAll inconsistency: Missing prevsmallest pointer\n");
					DebugOutputFreelist(p);
					exit(1);
				}
			}
		}
#endif
		p->check(4);
		Nfree++;
	}
#if DEBUG > 2
	sprintf(s,"-- Nfree=%d\n",Nfree);
	errorout(s);
#endif
}

INLINE void *Tnewdel::fromFreelist(Theader*p, int id)
// Take a block from Freelist and return the starting address of usable data
{
	//errorout("# fromFreelist\n");
	p->check(100+id);
	Theader *const prev = p->prev;
	Theader *const next = p->next;
#if FAST_FREELIST
	// Update nextlargest, prevsmallest pointers
	if (p->nextlargest != 0 || p->prevsmallest != 0) {
#if DEBUG
		if (p->prevsmallest == 0 && p->prev != 0) {
			errorout("*** fromFreelist: inconsistency 112\n");
		}
		if (p->nextlargest) {
			if (p->nextlargest->sz <= p->sz) {
				errorout("*** fromFreelist: nextlargest size is not larger\n");
			}
		}
		if (p->prevsmallest) {
			if (p->prevsmallest->sz >= p->sz) {
				errorout("*** fromFreelist: prevsmallest size is not smaller\n");
			}
		}
#endif
		if (next) {
			if (next->sz == p->sz) {
				next->nextlargest = p->nextlargest;
				next->prevsmallest = p->prevsmallest;
				if (p->nextlargest)
					p->nextlargest->prevsmallest = next;
				if (p->prevsmallest) {
					p->prevsmallest->nextlargest = next;
				}
			} else {
				next->prevsmallest = p->prevsmallest;
				if (p->prevsmallest) {
					p->prevsmallest->nextlargest = next;
				}
			}
		} else {
			if (p->prevsmallest) {
				p->prevsmallest->nextlargest = 0;
			}
		}
		p->nextlargest = p->prevsmallest = 0;
	}
#endif
#if DEBUG
	if (!p->isfree) {
		errorout("*** newdel.C: fromFreelist: Trying to take a non-free block from Freelist\n");
//		ShowFreelist(cerr);
		exit(1);
	}
#endif
	if (prev) {
		prev->next = next;
		prev->ComputeChecksum();
	} else
		Freelist = next;
	if (next) {
		next->prev = prev;
		next->ComputeChecksum();
	}
	p->isfree = 0;
	p->prev = 0;
	p->next = 0;
	p->ComputeChecksum();
#ifdef WITHIN_TELA
	global::FLlen--;
#endif
	return (void *)((char *)p + sizeof(Theader));
}

INLINE Theader *Tnewdel::toFreelist(Theader*p, int nomerge)
// Put a block to Freelist. Sort it to the proper position.
{
	//errorout("# toFreelist\n");
	Theader *const left = p->left;
	Theader *const right = p->right;
#	if MERGE_ALWAYS
	const int merge = !nomerge && ((left && left->isfree) || (right && right->isfree));
#	else
	int merge;
	if (left)
		merge = left->isfree && (!right || right && right->isfree);
	else
		merge = right && right->isfree;
#if DEBUG
	if (nomerge && merge) {
		errorout("*** Inconsistency in toFreelist: merge and nomerge are both true\n");
		exit(1);
	}
#endif
#	endif	/* MERGE_ALWAYS */
	//
	// These comments apply to the case MERGE_ALWAYS=0 only.
	// If merge is true, we have three possibilities:
	// (1) Left and Right both exist and are free,
	// (2) There is no Left, and Right exists and is free
	// (3) There is no Right, and Left exists and is free
	//
	if (merge) {
		if (left && right
#if MERGE_ALWAYS
			&& left->isfree && right->isfree
#endif
			) {
			// Possibility 1: Left and Right are both free
#			if DEBUG && !MERGE_ALWAYS
			if (!(left && left->isfree && right && right->isfree)) {
				errorout("*** Inconsistency 1 in toFreelist\n");
				char s[80];
				sprintf(s,"left = %ld, right = %ld\n",(long int)left, (long int)right);
				errorout(s);
				if (left) {
					sprintf(s,"left->isfree = %d\n",int(left->isfree));
					errorout(s);
				}
				if (right) {
					sprintf(s,"right->isfree = %d\n",int(right->isfree));
					errorout(s);
				}
				exit(1);
			}
#			endif
			fromFreelist(left,1);
			fromFreelist(right,2);
			left->right = right->right;
			left->sz += 2*sizeof(Theader) + p->sz + right->sz;
			left->ComputeChecksum();
			if (right->right) {
				right->right->left = left;
				right->right->ComputeChecksum();
			}
			p = left;
		} else if (!left
#if MERGE_ALWAYS
				   || !left->isfree
#endif
			) {
			// Possibility 2: No Left, Right exists and is free
//			errorout("Possibility 2\n");
#if DEBUG
			if (!((!left
#if MERGE_ALWAYS
				   || !left->isfree
#endif
				) && right && right->isfree)) {
				errorout("*** Inconsistency 2 in toFreelist\n");
				exit(1);
			}
#endif
			fromFreelist(right,3);
			p->right = right->right;
			p->sz += sizeof(Theader) + right->sz;
			p->ComputeChecksum();
			if (right->right) {
				right->right->left = p;
				right->right->ComputeChecksum();
			}
		} else {
			// Possibility 3: No Right, Left exists and is free
//			errorout("Possibility 3\n");
#if DEBUG
			if (!((!right
#if MERGE_ALWAYS
				   || !right->isfree
#endif
				) && left && left->isfree)) {
				errorout("*** Inconsistency 3 in toFreelist\n");
				exit(1);
			}
#endif
			fromFreelist(left,4);
			left->right = right;
			left->sz += sizeof(Theader) + p->sz;
			left->ComputeChecksum();
#if MERGE_ALWAYS
			if (right) {
				right->left = left;
				right->ComputeChecksum();
			}
#endif
			p = left;
			
		}
	}
#	if SORTED_FREELIST
	Theader *q,*qold=0;
#if FAST_FREELIST
	for (q=Freelist; q;
		 (q->next==0 && (qold=q)),q=(q->nextlargest ? q->nextlargest : q->next)
		)
		if (q->sz >= p->sz) break;
	if (q && !qold) qold = q->prev;
#else
	for (q=Freelist; q;
		 qold=q,q=q->next
		)
		if (q->sz >= p->sz) break;
#endif

	if (q) {
		// Insert before q
		//errorout("# Insert before q\n");
		p->next = q;
		p->prev = qold;
		q->prev = p;
		if (q == Freelist) Freelist = p;
		q->ComputeChecksum();
		if (qold) {
			qold->next = p;
			qold->ComputeChecksum();
		}
	} else {
		//errorout("# Insert as last item in Freelist\n");
		// Insert as last item in Freelist
		if (qold) {
			qold->next = p;
			qold->ComputeChecksum();
			p->prev = qold;
		} else {
			Freelist = p;
			p->prev = 0;
		}
		p->next = 0;
	}
#if FAST_FREELIST
	// Update nextlargest, prevsmallest pointers
#if DEBUG
	if (p->prev)
		if (p->prev->sz == p->sz) {
			errorout("*** toFreelist: prev size is equal to p size");
		}
#endif
	if (p->next) {
		if (p->next->prevsmallest) {
			p->next->prevsmallest->nextlargest = p;
		}
		if (p->next->sz == p->sz) {
			p->prevsmallest = p->next->prevsmallest;
			if (p->next->nextlargest)
				p->next->nextlargest->prevsmallest = p;
			p->nextlargest = p->next->nextlargest;
			p->next->prevsmallest = p->next->nextlargest = 0;
		} else {
#if DEBUG
			if (p->next->sz < p->sz) errorout("***** toFreelist: next size is smaller\n");
#endif
			p->prevsmallest = p->next->prevsmallest;
			p->nextlargest = p->next;
			p->next->prevsmallest = p;
		}
	} else {
		Theader* r = p->prev;
		while (r && r->prevsmallest==0 && r->prev!=0) r=r->prev;
		p->prevsmallest = r;
		p->nextlargest = 0;
		if (r) {
			r->nextlargest = p;
		}
	}
#endif	/* FAST_FREELIST */
#	else
	// Not sorted Freelist, insert at the beginning
	p->next = Freelist;
	if (Freelist) {
		Freelist->prev = p;
		Freelist->ComputeChecksum();
	}
	Freelist = p;
	p->prev = 0;
#	endif	/* SORTED_FREELIST */
	p->isfree = 1;
	p->ComputeChecksum();
#ifdef WITHIN_TELA
	global::FLlen++;
#endif
	return p;
}

inline void *Tnewdel::Align(char *ptr) {
	return (void*)(sizeof(ALIGN_TYPE)*((size_t(ptr) + sizeof(ALIGN_TYPE) - 1)/sizeof(ALIGN_TYPE)));
}

INLINE void *Tnewdel::divideBlock(Theader*p, size_t sz1)
// Take sz1 bytes usable data from block p (which must be free) and return the rest to Freelist.
// The sz1 is first rounded upwards to word boundary if required.
// If the returned part would be too small, do not return but instead reserve
// the whole block.
// The return value is a pointer to the usable data area of the reserved block
{
	p->check(6);
	if (!p->isfree) {
		errorout("*** newdel.C: divideBlock: Trying to divide a non-free block\n");
		exit(1);
	}
	size_t sz = size_t((char *)Align((char *)p + sz1) - (char *)p);
	size_t totallen = p->sz + sizeof(Theader);
	if (totallen < 3*sizeof(Theader) + sz) {
#if DEBUG > 2
		errorout("---- Not returning block because too small\n");
#endif
		// Too small, do not return to Freelist
		return fromFreelist(p,5);
	} else {
#if DEBUG > 2
		errorout("---- Returning residual block to Freelist\n");
#endif
		fromFreelist(p,6);
		Theader *const q = (Theader *)((char *)p + sizeof(Theader) + sz);
		const size_t newsz = totallen - 2*sizeof(Theader) - sz;
		q->right = p->right;
		q->left = p;
		q->sz = newsz;
		toFreelist(q,1);
		Theader *const right = p->right;
		if (right) {right->left = q; right->ComputeChecksum();}
		p->right = q;
		p->sz = sz;
		p->ComputeChecksum();
		return (void *)((char *)p + sizeof(Theader));
	}
}

// Cfront may not be able to inline the the following function because it contains loops.
// If this happens, just remove the inline. (Is someone still using Cfront?)

INLINE void *Tnewdel::malloc(size_t sz)
{
#if DEBUG
	char s[128];
#endif
#if DEBUG > 2
	sprintf(s,"Calling Tnewdel::malloc(%ld)\n",(long int)sz);
	errorout(s);
#endif
#if DEBUG > 1
	sprintf(s,"malloc(%ld), pools=%ld\n",(long int)sz,(long int)pools);
	errorout(s);
#endif
#if DEBUG > 1
	CheckAll();
#endif
	Theader *p, *q, *leftblock, *rightblock;
	// q is the predecessor
	long int badness, bestbadness = -1, residsize;
	Theader *best_p=0;
	void *residarea;
#if LOGGING
	int count = 0;
#endif
#	if SORTED_FREELIST
	for (p=Freelist; p;
#		if FAST_FREELIST
		 p=p->nextlargest
#		else
		 p=p->next
#		endif
		) {
		p->check(1);
#if LOGGING
		count++;
#endif
		if (p->sz >= sz) {
			bestbadness = p->sz - sz;
			if (bestbadness == 0) {
				// Exact match was found
#if DEBUG > 2
				sprintf(s,"malloc, exact match, sz=%ld\n",(long int)sz);
				errorout(s);
#endif
#if LOGGING
				fprintf(log,"%ld %ld\n",(long int)sz,(long int)count);
#endif
				return fromFreelist(p,7);
			}
			best_p = p;
#if LOGGING
			fprintf(log,"%ld %ld\n",(long int)sz,(long int)count);
#endif
			break;
		}
	}
#	else	/* Not SORTED_FREELIST */
	for (q=0,p=Freelist; p; q=p, p=p->next) {
		p->check(1);
		badness = p->sz - sz;
		if (badness > 0) {
			// Possible match was found
			if (badness < bestbadness || bestbadness < 0) {
				bestbadness = badness;
				best_p = p;
			}
		} else if (badness == 0) {
			// Exact match was found
#if DEBUG > 2
			sprintf(s,"malloc, exact match, sz=%ld\n",(long int)sz);
			errorout(s);
#endif
			return fromFreelist(p,8);
		}
	}
#	endif	/* SORTED_FREELIST */
	if (bestbadness > 0) {
#if DEBUG > 2
		sprintf(s,"malloc: best match, badness=%ld, sz=%ld\n",bestbadness,sz);
		errorout(s);
#endif
		return divideBlock(best_p, sz);
	}
	// Otherwise, there was no room in Freelist, We must allocate a new pool
	size_t poolsize = basic_poolsize;
	if (poolsize < sz + sizeof(TPoolEntry) + 2*sizeof(Theader)) {
		poolsize = sz + sizeof(TPoolEntry) + 2*sizeof(Theader);
	}
#if DEBUG
	sprintf(s,"malloc: allocating new block of size %ld\n",(long int)poolsize);
	errorout(s);
#endif
	poolsize = size_t(Align((char *)poolsize));
	TPoolEntry* pool = (TPoolEntry*)::malloc(poolsize);
#ifdef WITHIN_TELA
	global::memalloc+= poolsize;
#endif
	if (!pool) {
		errorout("*** tela operator new: Out of memory\n");
		exit(1);
	}
	pool->next = pools;
	pool->sz = poolsize;
	pool->magic =  POOLMAGIC;
	pool->checksum = size_t(pool->next) + size_t(pool->sz);
	pools = pool;
	void *const poolarea = (void*)(pool+1);
	if (poolsize < sz + sizeof(TPoolEntry)) {
		errorout("*** Internal error 1 in newdel.C\n");
		exit(1);
	}
	Theader *const result = (Theader*)poolarea;
	result->left = 0;
	result->right = 0;
	result->sz = poolsize - sizeof(Theader) - sizeof(TPoolEntry);
	if (result->sz % sizeof(ALIGN_TYPE)) {
		errorout("*** result->sz is not divisible by sizeof(ALIGN_TYPE)\n");
		exit(1);
	}
	return divideBlock(toFreelist(result),sz);
}

#ifndef _UNICOS
INLINE
#endif
void Tnewdel::free(void *p)
{
	char s[80];
#if DEBUG > 1
	sprintf(s,"Calling Tnewdel::free(%ld)",(long int)p);
	errorout(s);
#endif
	Theader*F = (Theader*)p - 1;
	if (F->isfree) {
		sprintf(s,"*** Trying to free already free pointer %ld\n",(long int)p);
		errorout(s);
		exit(1);
	}
	F->check(2);
#if DEBUG > 1
	sprintf(s,", sz=%ld\n",F->sz);
	errorout(s);
#endif
	toFreelist(F);
}

void *operator new(size_t sz1)
{
	static int FirstTime = 1;
	if (FirstTime) {
		newdel.init(1);
		FirstTime = 0;
#if LOGGING
		newdel.log = fopen("malloc.log","w");
#endif
	}
	const size_t sz = size_t(newdel.Align((char *)sz1));
//	if (sz1 == 0) cerr << "- Calling operator new with argument 0, sz = " << sz << "\n";
	char *ptr = (char*)newdel.malloc(sz+2*mempadd+ptrsz);
	if (!ptr) {
		char s[80];
		sprintf(s,"*** operator new(%ld): out of memory\n",(long int)sz);
		errorout(s);
		exit(1);
	}
	*((size_t*)ptr) = sz;
#if LEAK_CHECKED
	for (int i=0; i<mempadd; i++) ptr[ptrsz+i] = ptr[sz+ptrsz+mempadd+i] = i;
	memset(ptr+ptrsz+mempadd,0xFF,sz);
#endif
	// Anyway we clear the first size_t word of the malloced area.
	// This is not required, of course, but by doing this we apparently work
	// around some bugs in SGI C++ iostream library. However, we don't do this
	// if new was called with sz argument of 0.
	if (sz >= sizeof(size_t))
		*(size_t*)(ptr+ptrsz+mempadd) = 0;
#ifdef WITHIN_TELA
	global::memuse+= sz;
	global::Nalloc++;
	global::Nmem++;
#endif
	return (void*)(ptr+ptrsz+mempadd);
}

void operator delete(void *ptr)
{
	if (!ptr) return;
	char *ptr1 = ((char*)ptr) - mempadd-ptrsz;
	const size_t sz = *((size_t*)ptr1);
#if LEAK_CHECKED
	int begerr=0, enderr=0;
	for (int i=0; i<mempadd; i++) {
		if (ptr1[ptrsz+i]!=i) begerr++;
		if (ptr1[sz+ptrsz+mempadd+i]!=i) enderr++;
	}
	if (begerr) {
		char s[100];
		sprintf(s,"*** leak-checked operator delete: %d beg errors\n",begerr);
		errorout(s);
	}
	if (enderr) {
		char s[100];
		sprintf(s,"*** leak-checked operator delete: %d end errors\n",enderr);
		errorout(s);
	}
#endif
#ifdef WITHIN_TELA
	global::memuse-= sz;
	global::Nalloc--;
#endif
	newdel.free(ptr1);
}

void ShowFreelist(ostream& o) {newdel.ShowFreelist(o);}

#endif
