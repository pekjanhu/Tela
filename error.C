/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "error.H"
#endif
#include "def.H"
#include "error.H"
#include "common.H"

TIntStack thePCstack;

extern int FindLineNumber();	// defined in tela.C
extern const char *FindFileName();	// also defined in tela.C
extern FILE *FindAndOpenFile(const Tchar* fn);	// this one too

#if HAVE_RUSAGE
struct rusage struct_ru;
#elif HAVE_SYS_PROCFS_H
prstatus_t struct_pr;
#endif

#if USE_STRINGSTREAM
stringstream err;
#else
strstream err;
#endif

TJumpBuffer jmp1,jmp2;

static void DisplayFromFile(const char *fn, int linenum) {
	int ch;
	const int PreLines = 2;		// display PreLines lines before the error line
	const int LINELEN = 70;		// truncate lines longer than this
	FILE *fp = FindAndOpenFile((const Tchar*)fn);
	if (!fp) return;
        int i;
	for (i=0; i<linenum-PreLines-1; i++)
		for (ch=fgetc(fp); ch!='\n'; ch=fgetc(fp)) if (feof(fp)) {fclose(fp); return;}
	for (i=0; i<min(PreLines+1,linenum); i++) {
		char s[LINELEN+2];
		if (i==min(PreLines,linenum-1)) cerr << "=> "; else cerr << "   ";
		int j=0;
		int truncated=0;
		for (ch=fgetc(fp); ch!='\n'; ch=fgetc(fp)) {
			if (feof(fp)) {fclose(fp); return;}
			if (j<LINELEN) s[j++] = ch; else truncated=1;
		}
		s[j] = '\0';
		cerr << s;
		if (truncated) cerr << "..";
		cerr << '\n';
	}
	fclose(fp);
}	

void error()
{
#	if SUPPORT_LOOP_TILING
	global::tile_in_use = 0;
#	endif
	int linenumber = FindLineNumber();
	const char *filename = FindFileName();
	cerr << "*** ";
	if (filename && *filename) {
		cerr << "Runtime error at line " << linenumber << " in \"" << filename << "\".\n";
		if (linenumber > 0) DisplayFromFile(filename,linenumber);
	}
	err << ends;
	err.seekg(0);
	int ch;
	while ((ch=err.get())) cerr.put(char(ch));
	err.clear();
	err.seekp(0);
	if (global::debugging) {
		LongJmp(jmp2,1);
	} else {
		LongJmp(jmp1,1);
	}
}

void ConsistencyChecks()
{
	if (sizeof(Treal) < sizeof(Tint) || sizeof(Tcomplex) < sizeof(Tint)) {
		cerr << "*** sizeof(Treal) = " << sizeof(Treal) << " is smaller than sizeof(Tint) = "
			 << sizeof(Tint) << '\n';
		exit(1);
	}
	if (sizeof(Tint) > sizeof(TPtrInt) || sizeof(void*) > sizeof(TPtrInt) || sizeof(int) > sizeof(TPtrInt)) {
		cerr << "*** sizeof(TPtrInt) = " << sizeof(TPtrInt) << " is too small.\n";
		exit(1);
	}
	if (sizeof(Treal*) != sizeof(Tcomplex*)) {
		cerr << "*** sizeof(Treal*) = " << sizeof(Treal*) << " != sizeof(Tcomplex*) = " << sizeof(Tcomplex*) << ".\n";
	    exit(1);
    }
	if (2*sizeof(Treal) != sizeof(Tcomplex)) {
		cerr << "*** 2*sizeof(Treal) = " << 2*sizeof(Treal) << " != sizeof(Tcomplex) = " << sizeof(Tcomplex) << ".\n";
		exit(1);
	}
}

