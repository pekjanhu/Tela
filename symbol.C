/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "symbol.H"
#endif
#include "symbol.H"
#include "common.H"

#define DBG(x)

// Global hash table
TFixedHash theHT(7919 /*1113*/);

// -------------------- Tstring methods --------------------------

// Constructor from string object
Tstring::Tstring(const Tobject& obj) {
	if (obj.kind()==KIntArray && obj.rank()==1) {
		L = obj.length();
		s = new Tchar [L+1];
		for (int i=0; i<L; i++) s[i] = Tchar(obj.IntPtr()[i]);
		s[L] = '\0';
	} else if (obj.kind()==Kint) {
		L = 1;
		s = new Tchar [2];
		s[0] = Tchar(obj.IntValue());
		s[1] = '\0';
	} else {
		cerr << "*** Constructing Tstring from non-string, non-char Tobject\n";
		L=0; s=0;
	}
}

Tstring& Tstring::operator=(const Tstring& s1) {
	if(&s1==this) return *this;
	DBG(clog<<"/del/"<<s<<'\n');
	delete [] s;
	int L=s1.length();
	DBG(clog<<"/new/"<<s1.s<<'\n');
	s=new Tchar[L+1];
	strcpy(s,s1.s);
	return *this;
}

// -------------------- TFixedHash methods -------------------------

TFixedHash::TFixedHash(int N) : n(N), counter(0), ptr(new THashEntryPtr [n]), curr_i(0), curr_ptr(0), dirty(0) {
	for (int i=0; i<int(n); i++) ptr[i] = 0;
}

INLINE int TFixedHash::HashFunction(const Tstring& name) {
	Tchar *s = name;
	unsigned int sum = 0;
	for (int i=0; s[i]; i++) sum+= s[i];
	return sum % n;
}

extern "C" int sourcefunction(const TConstObjectPtr argin[], int Nargin, const TObjectPtr argout[], int Nargout);

INLINE void HandleStubCase(Tsymbol*p) {
	const Tchar *stub = p->StubString();
	if (stub) {
		if (!flags::silent)
			cout << "[Autosourcing \"" << (char*)stub << "\"...";
		TConstObjectPtr argin[1]; TObjectPtr argout[1];
		Tobject fn(stub);
		*argin = &fn;
		void **stubinfo = (void**)p->StubInfo();
		for (int i=1; stubinfo[i]; i++) {
			//cerr << "Preparing to load " << *(Tsymbol*)(stubinfo[i]) << "...\n";
			((Tsymbol*)(stubinfo[i]))->ClearStubFlag();
		}
		int ret = sourcefunction(argin,1,argout,0);
		p->ClearStubFlag();
		if (ret) {
			cerr << "*** Sourcing did not succeed, error code " << ret << ".\n";
		} else {
			if (!flags::silent) cout << "done]\n" << flush;
			delete [] stubinfo;
		}
	}
}

THashEntry* TFixedHash::add(const Tstring& name) {
	int k = HashFunction(name);
	int found = 0;
    THashEntryPtr p;
	for (p=ptr[k]; p; p=p->next)
		if (p->name() == name) {found=1; break;}
	if (found) {
		HandleStubCase(p);
		return p;
	} else {
		THashEntryPtr q = new THashEntry(name);
		q->next = ptr[k];
		ptr[k] = q;
		counter++;
		dirty = 1;
		return q;
	}
}

THashEntry* TFixedHash::assoc(const Tstring& name) {
	int k = HashFunction(name);
	int found = 0;
    THashEntryPtr p;
	for (p=ptr[k]; p; p=p->next)
		if (p->name() == name) {found=1; break;}
	//if (found) HandleStubCase(p);
	return found ? p : 0;
}

TFixedHash& TFixedHash::operator+=(TFixedHash& h1) {
	for (THashEntryPtr p=h1.first(); p; p=h1.next()) this->add(p->name());
	return *this;
}

THashEntryPtr TFixedHash::first() {	// Return ptr to first element, or 0
	dirty = 0;
	for (curr_i=0; curr_i<int(n); curr_i++) if (ptr[curr_i]) return (curr_ptr=ptr[curr_i]);
	return (curr_ptr=0);
}

THashEntryPtr TFixedHash::next() {		// Return ptr to next element, or 0 when element returned by first() is reached
	if (dirty) return this->first();
//	if (!curr_ptr) return 0;
	for (int flag=0; curr_i<int(n); curr_i++, curr_ptr=ptr[curr_i], flag=1)
		if (flag) {
			if (curr_ptr) return curr_ptr;
		} else {
			if (curr_ptr && curr_ptr->next)
				return (curr_ptr = curr_ptr->next);
		}
	return 0;
}

void TFixedHash::unalloc() {
	THashEntryPtr q;
	for (int i=0; i<int(n); i++) if (ptr[i])
		for (THashEntryPtr p=ptr[i]; p; q=p, p=p->next, delete q);
	delete [] ptr;
}

TFixedHash& TFixedHash::operator=(const TFixedHash& h1) {
	unalloc();
	n = h1.n;
	curr_i = 0;
	curr_ptr = 0;
	dirty = 0;
	counter = h1.counter;
	ptr = h1.ptr;
	return *this;
}

ostream& operator<<(ostream& o, TFixedHash& h) {
	for (int i=0; i<int(h.n); i++) if (h.ptr[i]) {
		o << i << ": ";
		for (THashEntryPtr p=h.ptr[i]; p; p=p->next) o << p->name() << ' ';
		o << '\n';
	}
#if 0
	int cnt=0;
	for (THashEntryPtr p=h.first(); p; p=h.next()) {
		o << *p << ' ';
		if (cnt++ > 10) break;
	}
#endif
	return o;
}

#if 0

// ---------------------- Thash methods ----------------------------

/*
 * NOTE: Rehashing doesn't work correctly since it changes the
 * physical addresses of the THashEntry classes.
 * The same cells should be just redistributed.
 * For now, just define initial_size large enough such
 * that rehashing never occurs!
 */

const int initial_size = 257;			// initial length of hash table
const float FillThreshold = 1.4;		// do rehashing if fill factor exceeds this value

Thash::Thash() : h(initial_size),name("noname") {addempty();}

Thash::Thash(const Tchar *namestr) : h(initial_size),name(namestr) {addempty();}
Thash::Thash(const Tstring& namestr) : h(initial_size),name(namestr) {addempty();}

void Thash::addempty() {/*Empty=add("Empty");*/}

int Thash::nextlength(int m) {return 2*m;}

THashEntry* Thash::add(const Tstring& name) {			// add element
	if (h.count() > FillThreshold*h.length()) rehash(nextlength(h.length()));
	return h.add(name);
}

THashEntry* Thash::assoc(const Tstring& name) {
	return h.assoc(name);
}

void Thash::rehash(int m) {
	clog << "Calling rehash from " << h.length() << " to " << m << "...\n";
	TFixedHash *h2ptr = new TFixedHash(m);
	for (THashEntryPtr p=h.first(); p; p=h.next()) h2ptr->add(p->name());
	h = *h2ptr;
	clog << "Ok.\n";
}

ostream& operator<<(ostream& o, Thash &h1) {
	o << "Hash table " << h1.name << " contents, " << h1.count() << " elements:\n";
	return o << h1.h;
}

#endif

Tsymbol* UniqueSymbol(const Tchar *body) {
	Tchar buff[80];
	const int maxtrials=10000;
	for (int i=1; ; i++) {
		sprintf((char*)buff,"%s%.4d",body,i);
		if (!theHT.assoc(buff)) break;
		if (i>maxtrials) {
			cerr << "Fatal internal error: UniqueSymbol had to do more than " << maxtrials << " trials!\n";
			exit(1);
		}
	}
	return theHT.add(buff);
}

// --------------- Install'ing C-functions ------------------

void Install(const TCFunctionInfo& info) {
	Tsymbol* symptr = theHT.add(info.Cfname);
	*(symptr->value()) = info.Cfptr;
	symptr->value()->CFunctionInfoPtrRef() = (TCFunctionInfo*)&info;
	if (flags::verbose)
		cout << "  Installed " << (const char*)info.Cfname << " ("<<info.minin<<","<<info.minout<<")..("<<info.maxin<<","<<info.maxout<<")\n";
}
	
void Install(const TCFunctionInfo infos[], const Tchar modulename[]) {
	if (flags::verbose) cout << "Installing " << (const char*)modulename << ":\n";
	for (int i=0; infos[i].Cfname; i++) Install(infos[i]);
	if (flags::verbose) cout << "OK.\n\n";
}

// -----------------------------------------------------------------------------
