/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

// tLL.H must have been included before

int LINEARLIST::oversize = 10;
int LINEARLIST::mingrowchunk = 50;
int LINEARLIST::maxgrowchunk = 1000;


LINEARLIST::LINEARLIST(int L) {
	len = L;
	alloc_len = L + oversize;
	ptr = new COMPTYPE [alloc_len];
	for (int i=0; i<L; i++) ptr[i] = DEFAULT_COMPVALUE;
}

LINEARLIST::LINEARLIST(int L, const COMPTYPE table[]) {
	len = L;
	alloc_len = L + oversize;
	ptr = new COMPTYPE [alloc_len];
#	if USE_MEMCPY
	memcpy(ptr,table,L*sizeof(COMPTYPE));
#	else
	for (int i=0; i<L; i++) ptr[i] = table[i];
#	endif
}

LINEARLIST::LINEARLIST(const LINEARLIST& L) {
	len = L.len;
	alloc_len = len + oversize;
	ptr = new COMPTYPE [alloc_len];
#	if USE_MEMCPY
	memcpy(ptr,L.ptr,len*sizeof(COMPTYPE));
#	else
	for (int i=0; i<len; i++) ptr[i] = L.ptr[i];
#	endif
}

LINEARLIST& LINEARLIST::operator=(const LINEARLIST& L) {
	int i;
	if (len < L.len) {	// doesn't fit
		delete [] ptr;
		alloc_len = L.len + oversize;
		ptr = new COMPTYPE [alloc_len];
	}
#	if USE_MEMCPY
	memcpy(ptr,L.ptr,L.len*sizeof(COMPTYPE));
#	else
	for (i=0; i<L.len; i++) ptr[i] = L.ptr[i];
#	endif
	len = L.len;
	return *this;
}

LINEARLIST& LINEARLIST::operator=(int L) {
	if (len < L) {	// doesn't fit
		COMPTYPE* oldptr = ptr;
		alloc_len = L + oversize;
		ptr = new COMPTYPE [alloc_len];
#		if USE_MEMCPY
		memcpy(ptr,oldptr,len*sizeof(COMPTYPE));
#		else
		for (int i=0; i<len; i++) ptr[i] = oldptr[i];
#		endif
		delete [] oldptr;
	}
	len = L;
	return *this;
}

void LINEARLIST::grow() {
	int old_alloc_len = alloc_len;
	alloc_len = max(5*old_alloc_len/4,mingrowchunk);
	if (alloc_len - old_alloc_len > maxgrowchunk) alloc_len = old_alloc_len + maxgrowchunk;
	COMPTYPE *newptr = new COMPTYPE [alloc_len];
#	if USE_MEMCPY
	memcpy(newptr,ptr,old_alloc_len*sizeof(COMPTYPE));
#	else
	for (int i=0; i<old_alloc_len; i++) newptr[i] = ptr[i];
#	endif
	delete [] ptr;
	ptr = newptr;
}

void LINEARLIST::append(const COMPTYPE& c) {
	if (alloc_len <= len) grow();
	ptr[len++] = c;
}

ostream& operator<<(ostream& o, const LINEARLIST& LL) {
	o << '[';
	for (int i=0; i<LL.length()-1; i++) o << LL[i] << ',';
	o << LL[LL.length()-1] << ']';
	return o;
}
