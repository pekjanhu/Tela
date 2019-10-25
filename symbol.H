/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2001 Pekka Janhunen
 */

#ifndef SYMBOL_H

#ifdef __GNUC__
#  pragma interface
#endif

#include "object.H"
#include "ctinfo.H"

// Classes to be defined

class Tstring;					// Basic free-store strings
class Tsymbol;					// Derivative of Tstring
class THashEntry;				// Derivative of Tsymbol, contains the link field
class TFixedHash;				// Fixed length hash table for Tstring objects
class Thash;					// Automatically rehashing hash table for Tstrings
class Tslot;					// Wrapper class for int slot number

// ------------------- class Tstring ----------------------

class Tstring {
 protected:
	int L;
	Tchar *s;
 public:
	Tstring() : L(0), s(0) {}
	Tstring(const Tchar *ptr) : L(strlen(ptr)+1), s(new Tchar[L]) {strcpy(s,ptr);}
	Tstring(const Tstring& s1) : L(strlen(s1.s)+1), s(new Tchar[L]) {strcpy(s,s1.s);}
	Tstring(const Tobject& obj);
	int length() const {return L;}
	Tstring& operator=(const Tstring& s1);
	Tstring& operator=(const Tchar*ptr) {delete[]s; s=new Tchar[(L=strlen(ptr))+1]; strcpy(s,ptr); return *this;}
	int operator==(const Tstring& s1) const {return !strcmp(s,s1.s);}
	operator Tchar *() const {return s;}
#	ifdef TCHAR_NOTEQUAL_CHAR
	operator char*() const {return (char*)s;}
#	endif
	Tchar& operator[](int i) const {return s[i];}
	Tchar& operator()(int i) const {return s[i];}
	friend inline ostream& operator<<(ostream& o, const Tstring& s1);
	~Tstring() {delete [] s;}
};

inline ostream& operator<<(ostream& o, const Tstring& s1) {
	return o << (char*)s1.s;
}

// --------------------- class Tsymbol -----------------------

class Tsymbol : public Tstring {
 private:
	Tsymbol& operator=(const Tsymbol&);     // do not implement
	unsigned int isglobal:1;		// flag for autoglobal symbol (pi, on, off, ...)
	unsigned int ishidden:1;		// flag for symbol hidden from whos()
	const void **stub;	// usually NULL, if not NULL, tries to do source(stub) on usage
 protected:
	Tobject *objptr;
 public:
	Tsymbol(const Tsymbol& s1) : Tstring(s1), isglobal(0), ishidden(0), stub(0), objptr(s1.objptr) {}
	Tsymbol(const Tstring& s1) : Tstring(s1), isglobal(0), ishidden(0), stub(0), objptr(new Tobject()) {}
	Tsymbol(const Tchar *ptr) : Tstring(ptr), isglobal(0), ishidden(0), stub(0), objptr(new Tobject()) {}
	const Tstring& name() const {return *this;}
	Tobject*& value() {return objptr;}
	void SetGlobalFlag() {isglobal=1;}
	void SetHiddenFlag() {ishidden=1;}
	void ClearHiddenFlag() {ishidden=0;}
	void SetStubInfo(const void**stubinfo) {stub=stubinfo;}
	void ClearStubFlag() {stub=0;}
	int IsGlobal() const {return isglobal;}
	int IsHidden() const {return ishidden;}
	const Tchar*StubString() const {return stub ? (Tchar*)(*stub) : 0;}
	const void**StubInfo() const {return stub;}
	Tchar operator[](int i) const {return s[i];}
	Tchar operator()(int i) const {return s[i];}
	friend inline ostream& operator<<(ostream& o, const Tsymbol& s1);
	~Tsymbol() {delete objptr;}
};

inline ostream& operator<<(ostream& o, const Tsymbol& s1) {return o << (char*)s1.s;}

// ------------------ class THashEntry ----------------------

class THashEntry : public Tsymbol {
 private:
	THashEntry(const THashEntry&);            // do not implement
	THashEntry& operator=(const THashEntry&); // do not implement
 public:
	THashEntry *next;
	THashEntry(const Tstring& s1) : Tsymbol(s1), next(0) {}
};

typedef THashEntry* THashEntryPtr;

// -------------------- class TFixedHash ------------------------

class TFixedHash {
 private:
	unsigned int n;							// length of hash array
	unsigned int counter;					// number of elements stored
	THashEntryPtr *ptr;						// ptr[n]
	int curr_i;								// current index, used in iterator
	THashEntryPtr curr_ptr;					// current pointer, used in iterator
	int dirty;								// indicates whether table has been modified after last first() call
	int HashFunction(const Tstring& name);	// our secret and private hash function
	void unalloc();							// destructor
	TFixedHash& operator=(const TFixedHash& h1);	// called by Thash::rehash only, not for public use
	TFixedHash(const TFixedHash&); // do not implement
 public:
	TFixedHash(int N);						// constructor
	THashEntry* add(const Tstring& name);	// add element
	THashEntry* add(const Tchar *name)
		{return add(Tstring(name));}		// add element
	THashEntry* assoc(const Tstring& name);	// search element but do not add if doesn't exist
	THashEntry* assoc(const Tchar *name)
	    {return assoc(Tstring(name));}
	int count() {return counter;}			// return number of elements stored
	int length() {return n;}				// return length of hash array
	TFixedHash& operator+=(TFixedHash& h1);	// add all elements of h1 to *this
	friend ostream& operator<<(ostream& o, TFixedHash &h);	// print whole table
	THashEntryPtr first();					// return pointer to first element
	THashEntryPtr next();					// return pointer to next element, or 0
	//friend Thash::rehash(int m);			// needed to access the operator=
	friend class Thash;						// Cfront doesn't support the previous line
	~TFixedHash() {unalloc();}				// destructor
};

#if 0

// ----------------- class Thash ----------------------

class Thash {								// Hash table with automatic rehashing
 private:
	TFixedHash h;							// The underlying fixed-length hash table
	Tstring name;							// Hash table name, for identification purposes only
	void rehash(int m);						// The rehash procedure
	int nextlength(int m);					// Returns the length on which to rehash
	void addempty();						// every hash table contains the symbol named "Empty", this fn adds it
 public:
	Thash();								// constructor, no args
	Thash(const Tchar *namestr);				// constructor with HT name
	Thash(const Tstring& namestr);			// constructor with HT name
	THashEntry* add(const Tstring& name);	// add element
	THashEntry* add(const Tchar *name)
		{return add(Tstring(name));}		// add element
	THashEntry* assoc(const Tstring& name);	// search element but do not add if doesn't exist
	THashEntry* assoc(const Tchar *name)
	    {return assoc(Tstring(name));}
	int count() {return h.count();}			// return number of elements stored
	Thash& operator+=(Thash& h1)
		{h+=h1.h; return *this;}			// add all elements of h1 to *this
	Thash& operator+=(TFixedHash& h1)
		{h+=h1; return *this;}				// h1 can also be a fixed-length hash table
	friend ostream& operator<<(ostream& o, Thash &h1);	// print whole table
	THashEntryPtr first()
		{return h.first();}					// return pointer to first element
	THashEntryPtr next()
		{return h.next();}					// return pointer to next element, or 0
};

#endif

extern TFixedHash theHT;

extern Tsymbol* UniqueSymbol(const Tchar*body);

// ----------------- class Tslot ----------------------

class Tslot {
 private:
	TPtrInt n;
 public:
	Tslot() : n(0) {}
	Tslot(TPtrInt i) : n(i) {}
	operator TPtrInt() const {return n;}
	friend inline ostream& operator<<(ostream& o, const Tslot& s);
};

inline ostream& operator<<(ostream& o, const Tslot& s) {return o << s.n;}

class Tallrange {public:Tallrange(){}};		// Dummy class to denote ':' (ALLRANGE) objects

#define SYMBOL_H

#endif

