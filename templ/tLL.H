/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

// LINEARLIST, COMPTYPE and DEFAULT_COMPVALUE must be #define'd before including this

class LINEARLIST {
 private:
	int len;
	int alloc_len;
	COMPTYPE *ptr;
	void grow();
	static int oversize;
	static int mingrowchunk;
	static int maxgrowchunk;
 public:
	LINEARLIST() {len=0; alloc_len=oversize; ptr=0;}
	LINEARLIST(int L);
	LINEARLIST(int L, const COMPTYPE table[]);
	LINEARLIST(const LINEARLIST& L);
	LINEARLIST& operator=(const LINEARLIST& L);
	LINEARLIST& operator=(int L);
	COMPTYPE& operator[](int i) const {return ptr[i];}
	COMPTYPE* Ptr() const {return ptr;}
	void append(const COMPTYPE& c);
	int length() const {return len;}
	void clear() {delete [] ptr; len=alloc_len=0; ptr=0;}
	friend ostream& operator<<(ostream& o, const LINEARLIST& LL);
	void rawasciioutput(ostream& o) const;
	void rawasciiinput(istream& i);
	~LINEARLIST() {delete [] ptr;}
};




