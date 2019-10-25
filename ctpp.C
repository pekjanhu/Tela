/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

/*
  This is ctpp, C-tela preprocessor.
  It reads standard input and writes to standard output.
  It copies input to output, unless it sees constructs like
  [x,y] = f(a,b)
  which are replaced by corresponding tela-compatible C++ code.
  If it sees a left bracket, and if the last non-whitespace, non-comment
  and non-preprocessor line character before the left bracket was either
  semicolon (;) or right brace (}), it assumes that the left bracket
  begins the above construct.

  Following the pattern [x,y...] = f(a,b,...) must be a single
  ordinary C comment (it may also be absent). It is extracted
  to the help file, or if no help file is generated, file pointers
  to it will be saved so that the .ct source file will serve as
  help file. Following the comment is the function body.
  The end of the function body is signaled by a line beginning
  with '}'. No other line inside the body may contain the right
  brace as the first character. This convention makes it easy
  for ctpp to detect the end of the function body, without doing
  any parsing at all. If you follow the normal C coding practice,
  you will not be bothered by this seemingly ad hoc rule.
*/

#define FNAME_APPENDIX "function"
// The string "function" is appended to the C-tela function names, eg.
// if you declare []=myfunc(...) it's internal link name becomes myfuncfunction.
// If you refer to your own functions, you must follow this convention.
// Also, you should not change the definition above or you may have to find
// all places where you have called C-tela functions.
#define EXTERN_DEFINITION "extern \"C\" "

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#if STDC_HEADERS || HAVE_STRING_H
#  include <string.h>
#  if !STDC_HEADERS && HAVE_MEMORY_H
#    include <memory.h>
#  endif
#else
#  include <strings.h>
#  define strchr index
#  define strrchr rindex
#  define memcpy((d),(s),(n)) bcopy(s,d,n)
#  define memcmp((s1),(s2),(n)) bcmp(s1,s2,n)
#  define memset((s),(towhat),(n)) bzero(s,n); if (towhat) {cerr<<"bzero can only zero things\n"; exit(1);}
#endif

#ifdef CONVEX
extern "C" int remove(const char*);
#define tolower(c) ('A'<=(c)&&(c)<='Z'?(c)-'A'+'a':(c))
#endif

#define MAX_IDENTIFIER 256
#define MAX_NARGS 100
#define FilenameSeparator '/'
#define OUTSUFFIX "-.c"
#define HELPSUFFIX ".hlp"

#define GETC(ch) ch=getc(in); if (ch=='\n') line++
#define UNGETC(ch) if (ch=='\n') line--; ungetc(ch,in)

static int line = 1;	// keeps track of input line numbers
static int HelpFileFlag = 0;	// don't create help file by default

static void usage(FILE *f)
{
	fprintf(f,"Usage: ctpp [-h] [file.ct].\n");
	fprintf(f,"C-tela preprocessor. Preprocesses each file.ct, creating file-.c.\n");
	fprintf(f,"Options: -h will create help file(s) file.hlp. Use -h if you don't want\n");
	fprintf(f,"  to give users access to the source file.\n");
}

inline int iselectric(int ch) {return ch == '}' || ch == ';';}
inline int isIdentifierChar(int ch) {return isalpha(ch) || ch == '_';}

static void EmitLineNumber(FILE *out)
{
	fprintf(out,"#line %d\n",line);
}

static void ReadString(FILE *in, FILE *out, int endch)
{
	int ch;
	int allowbreak=1;
	for (;;) {
		GETC(ch);
		if (feof(in)) break;
		if (ch == '\\')
			allowbreak = 0;
		else if (ch == endch && allowbreak) {
			putc(ch,out);
			break;
		} else
			allowbreak = 1;
		putc(ch,out);
	}
}

static void ReadEndlineComment(FILE *in, FILE *out) {
	int ch;
	for (;;) {
		GETC(ch);
		if (feof(in)) break;
		if (ch == '\n') {
			putc(ch,out);
			break;
		}
		putc(ch,out);
	}
}

static void ReadOrdinaryComment(FILE *in, FILE *out) {
	int ch;
	int lastWasStar=0;
	for (;;) {
		GETC(ch);
		if (feof(in)) break;
		if (ch == '/' && lastWasStar) {
			putc(ch,out);
			break;
		}
		lastWasStar = ch == '*';
		putc(ch,out);
	}
}

static void ReadPPLine(FILE *in, FILE *out)
{
	int ch;
	int lastWasEscape=0;
	for (;;) {
		GETC(ch);
		if (feof(in)) break;
		if (ch == '\n' && !lastWasEscape) {
			putc(ch,out);
			break;
		}
		lastWasEscape = ch == '\\';
		putc(ch,out);
	}
}


static void syntaxerror(const char msg[])
{
	fprintf(stderr,"*** ctpp: Syntax error at line %d: %s\n",line,msg);
	exit(1);
}

static int nextNonWhiteSpace(FILE *in)
{
	int ch;
	for (;;) {
		GETC(ch);
		if (feof(in)) break;
		if (!isspace(ch)) break;
	}
	return ch;
}

#define LEFTPAREN '('
#define RIGHTPAREN ')'
#define LEFTBRACKET '['
#define RIGHTBRACKET ']'
#define COMMA ','
#define SEMICOLON ';'
#define EQUAL '='
#define ELLIPSIS 257
#define IDENTIFIER 258

typedef char Tidentifier[MAX_IDENTIFIER+1];

static Tidentifier theIdentifier;

static int lex(FILE *in)
{
	int ch;
	ch = nextNonWhiteSpace(in);
	switch (ch) {
	case LEFTPAREN:
	case RIGHTPAREN:
	case LEFTBRACKET:
	case RIGHTBRACKET:
	case COMMA:
	case SEMICOLON:
	case EQUAL:
		return ch;
	case '.':
		GETC(ch); if (ch!='.') syntaxerror("Read one '.' and expected second in arglist");
		GETC(ch); if (ch!='.') syntaxerror("Read two '.'s and expected third in arglist");
		return ELLIPSIS;
	default:
		{
			if (!isIdentifierChar(ch)) syntaxerror("Illegal character in arglist");
                        int i;
			for (i=0; ; i++) {
				theIdentifier[i] = ch;
				GETC(ch);
				if (!isdigit(ch) && !isIdentifierChar(ch)) {UNGETC(ch); break;}
				if (i >= MAX_IDENTIFIER) syntaxerror("Identifier too long");
			}
			theIdentifier[i+1] = '\0';
			return IDENTIFIER;
		}
	}
}

static void ExpectToken(FILE *in, int token)
{
	if (lex(in) != token) {
		char s[80];
		if (token == ELLIPSIS)
			sprintf(s,"'...' expected");
		else if (token == IDENTIFIER)
			sprintf(s,"Identifier expected");
		else
			sprintf(s,"'%c' expected",token);
		syntaxerror(s);
	}
}

typedef Tidentifier Targlist[MAX_NARGS];

Targlist out_arglist, in_arglist;	/* global vars */

static void ReadArglist(FILE *in, Targlist arglist, int& FirstOptional, int& HasEllipsisAtTheEnd, int& HasSemicolon)
{
	int token,i;
	FirstOptional = -1; HasEllipsisAtTheEnd = HasSemicolon = 0;
 start:
	token = lex(in);
	if (token == IDENTIFIER) {
		strcpy(arglist[i=0],theIdentifier);
		for (i=1; ; i++) {
			if (i >= MAX_NARGS) {
				syntaxerror("Too many arguments");
			}
			token = lex(in);
			if (token == COMMA) {
				ExpectToken(in,IDENTIFIER);
				strcpy(arglist[i],theIdentifier);
			} else if (token == SEMICOLON) {
				if (FirstOptional >= 0) syntaxerror("Only one semicolon per arglist");
				HasSemicolon = 1;
				FirstOptional = i;
				ExpectToken(in,IDENTIFIER);
				strcpy(arglist[i],theIdentifier);
			} else if (token == ELLIPSIS) {
				arglist[i][0] = '\0';
				HasEllipsisAtTheEnd = 1;
				FirstOptional = i;
				return;
			} else {
				UNGETC(token);
				arglist[i][0] = '\0';
				return;
			}
		}
	} else if (token == SEMICOLON) {
		FirstOptional = 0;
		HasSemicolon = 1;
		goto start;
	} else if (token == ELLIPSIS) {
		HasEllipsisAtTheEnd = 1;
		arglist[0][0] = '\0';
		return;
	} else {
		UNGETC(token);
		arglist[0][0] = '\0';
	}
} // ReadArglist

struct Tinfo;

struct Tinfo {
	Tinfo* next;
	Tidentifier name;
	int minin,minout, maxin,maxout;
	long helpstart,helpend;
};

static Tinfo *infos = 0;

static long HELP1=0, HELP2=0;	// global vars that record comment start and end positions from RecordComment

static void RecordComment(FILE *in, FILE *help)
{
	int ch;
	int lastWasStar=0;
	HELP2 = HELP1;	// so that if no help comment is generated, none will be displayed
	ch = nextNonWhiteSpace(in);
	if (ch != '/') {UNGETC(ch); return;}
	GETC(ch);
	if (ch != '*') return;
	int FirstTime = 1;
	for (;;) {
		GETC(ch);
		if (feof(in)) break;
		if (ch == '/')
			if (lastWasStar)
				break;
			else {
				if (HelpFileFlag)
					putc(ch,help);
				else {
					if (FirstTime) {
						HELP1=ftell(in);
						FirstTime = 0;
					}
					HELP2=ftell(in);
				}
			}
		if (lastWasStar) {
			if (HelpFileFlag)
				putc('*',help);
			else {
				if (FirstTime) {
					HELP1=ftell(in);
					FirstTime = 0;
				}
				HELP2=ftell(in);
			}
		}
		lastWasStar = ch == '*';
		if (!lastWasStar)
			if (HelpFileFlag)
				putc(ch,help);
			else {
				if (FirstTime) {
					HELP1=ftell(in);
					FirstTime = 0;
				}
				HELP2=ftell(in);
			}
	}
}

static void ReadFunctionBody(FILE *in, FILE *out)
/* Read input file until a line begins with '}' in the first position.
   This is thought to correspond to the end of function body.
   Echo characters to file out. The line following '}' is also read,
   up to newline. */
{
	int ch, LastWasNewline=0;
	// Loop until the pair "\n}" appears in input
	for (;;) {
		GETC(ch);
		if (feof(in)) return;
		putc(ch,out);
		if (ch=='\n')
			LastWasNewline=1;
		else if (ch=='}' && LastWasNewline)
			break;
		else
			LastWasNewline=0;
	}
	// Scan to next newline
	for (;;) {
		GETC(ch);
		if (feof(in)) return;
		putc(ch,out);
		if (ch=='\n') break;
	}
}

static void ReadPattern(FILE *in, FILE *out, FILE *help)
/*
  Recognize patterns of the form
  
  <arglist> ']' '=' IDENTIFIER '(' <arglist> ')'
  
  Also read in the following single ordinary C comment.
  Then read the following function body. The end of the
  function body is assumed to be a line beginning with '}'.
  The comment and the function body is echoed to file out.
*/
{
	int FirstOptionalIn,HasEllipsisIn;
	int FirstOptionalOut,HasEllipsisOut;
	int HasSemicolonOut,HasSemicolonIn;
	int minin,maxin,minout,maxout;
	Tidentifier fname;
	ReadArglist(in,out_arglist,FirstOptionalOut,HasEllipsisOut,HasSemicolonOut);
	ExpectToken(in,RIGHTBRACKET);
	ExpectToken(in,EQUAL);
	ExpectToken(in,IDENTIFIER);
	strcpy(fname,theIdentifier);
	ExpectToken(in,LEFTPAREN);
	ReadArglist(in,in_arglist,FirstOptionalIn,HasEllipsisIn,HasSemicolonIn);
	ExpectToken(in,RIGHTPAREN);
	fprintf(out,"\n");
	maxout = 0;
        int i;
	for (i=0; out_arglist[i][0]; i++) {
		fprintf(out,"#define %s (*(argout[%d]))\n",out_arglist[i],i);
		maxout++;
	}
	if (HasSemicolonOut) {
		minout = (FirstOptionalOut>=0) ? FirstOptionalOut : maxout;
		if (HasEllipsisOut && (FirstOptionalOut < 0)) minout = 0;
	} else {
		minout = maxout;	// if output list has no semicolon, all args are obligatory
	}
	if (HasEllipsisOut) maxout = -1;
	maxin = 0;
	for (i=0; in_arglist[i][0]; i++) {
		fprintf(out,"#define %s (*(argin[%d]))\n",in_arglist[i],i);
		maxin++;
	}
	minin = (FirstOptionalIn>=0) ? FirstOptionalIn : maxin;
	if (HasEllipsisIn && (FirstOptionalIn < 0)) minin = 0;
	if (HasEllipsisIn) maxin = -1;
	fprintf(out,"%sint %s%s\n\t(const TConstObjectPtr argin[], const int Nargin, const TObjectPtr argout[], const int Nargout)\n",
			EXTERN_DEFINITION,fname,FNAME_APPENDIX);
	static Tinfo *lastinfo=0, *newinfo=0;
	if (infos) {
		newinfo = new Tinfo;
		lastinfo->next = newinfo;
		lastinfo = newinfo;
	} else
		infos = lastinfo = newinfo = new Tinfo;
	newinfo->next = 0;
	strcpy(newinfo->name,fname);
	newinfo->minin = minin;
	newinfo->maxin = maxin;
	newinfo->minout = minout;
	newinfo->maxout = maxout;
	if (HelpFileFlag) {
		newinfo->helpstart = ftell(help);
		RecordComment(in,help);
		newinfo->helpend = ftell(help);
	} else {
		RecordComment(in,help);
		newinfo->helpstart = HELP1;
		newinfo->helpend = HELP2;
	}
	EmitLineNumber(out);
	ReadFunctionBody(in,out);
	// Undefine the parameters after the function body has been read
	for (i=0; out_arglist[i][0]; i++)
		fprintf(out,"#undef %s\n",out_arglist[i]);
	for (i=0; in_arglist[i][0]; i++)
		fprintf(out,"#undef %s\n",in_arglist[i]);
	// The undef's are extra lines, correct line number
	EmitLineNumber(out);
}

static void WriteInfo(FILE *out, char name[])
{
	fprintf(out,"extern \"C\" {\nTCFunctionInfo fninfo_%s[] = {\n",name);
	for (Tinfo*p=infos; p; p=p->next) {
		fprintf(out,"{(Tchar*)\"%s\",(Tchar*)helpname,%s%s, %d,%d,%d,%d, %ld,%ld},\n",
				p->name,p->name,FNAME_APPENDIX,p->minin,p->minout,p->maxin,p->maxout,p->helpstart,p->helpend);
	}
	fprintf(out,"{0,0,0, 0,0,0,0, 0,0}\n};\n}\n\n");
}

static void parse(FILE *in, FILE *out, FILE *help)
{
	int ch;
	int lastWasElectric=1;
	int lastWasNewline=1;
	int lastWasSlash=0;
	
	for (;;) {
		
		GETC(ch);
		if (feof(in)) break;
		
		if (lastWasElectric && ch == '[') {
			ReadPattern(in,out,help);
			lastWasElectric = 1;
			    // ReadPattern reads the function body, which ends with 'electric' right brace
		} else if (ch == '#' && lastWasNewline) {
			putc(ch,out);
			ReadPPLine(in,out);
		} else if (ch == '*' && lastWasSlash) {
			putc(ch,out);
			ReadOrdinaryComment(in,out);
		} else if (ch == '/' && lastWasSlash) {
			putc(ch,out);
			ReadEndlineComment(in,out);
		} else if (ch == '"') {
			putc(ch,out);
			ReadString(in,out,'"');
			lastWasElectric = 0;
		} else if (ch == '\'') {
			putc(ch,out);
			ReadString(in,out,'\'');
			lastWasElectric = 0;
		} else {
			putc(ch,out);
			lastWasNewline = ch == '\n';
			lastWasSlash = ch == '/';
			if (iselectric(ch))
				lastWasElectric = 1;
			else if (!isspace(ch) && !lastWasSlash)
				lastWasElectric = 0;
		}
	}
}

static char *MakeOutname(char *inname, const char *suffix)
{
	int L = strlen(inname);
	if (tolower(inname[L-1])!='t' || tolower(inname[L-2])!='c' || tolower(inname[L-3])!='.') {
		fprintf(stderr,"*** ctpp: input files should end with .ct: '%s'\n",inname);
		exit(1);
	}
	char *outname = new char [L+1-3+strlen(suffix)];
	strcpy(outname,inname);
	strcpy(outname+L-3,suffix);
	return outname;
}

static char *ExtractFilenameBody(char *s)
{
	// s MUST end with ".ct"
	int L = strlen(s);
	int L1 = 0;
        int i;
	for (i=L-4; i>=0 && s[i]!=FilenameSeparator; i--) L1++;
	char *result = new char [L1+1];
	int j=0;
	for (i=L-3-L1; i<=L-4; i++,j++) result[j] = s[i];
	result[j] = '\0';
	return result;
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {usage(stderr); return 1;}
	for (int i=1; i<argc; i++) {
		static const char *outsuffix = OUTSUFFIX;
		static const char *helpsuffix = HELPSUFFIX;
		if (argv[i][0] == '-') {
			if (!strcmp(argv[i],"-h")) HelpFileFlag = 1;
			else {fprintf(stderr,"ctpp: unrecognized option '%s' ignored\n",argv[i]);}
		} else {
			char *outname = MakeOutname(argv[i],outsuffix);
			char *helpname = MakeOutname(argv[i],helpsuffix);
			FILE *in = fopen(argv[i],"r");
			if (!in) {fprintf(stderr,"*** ctpp: could not open '%s' for reading\n",argv[i]); return 1;}
			FILE *out = fopen(outname,"w");
			if (!out) {fprintf(stderr,"*** ctpp: could not open '%s' for writing\n",outname); return 1;}
			FILE *help=0;
			long help0;
			if (HelpFileFlag) {
				help = fopen(helpname,"w");
				if (!help) {fprintf(stderr,"*** ctpp: could not open '%s' for writing\n",helpname); return 1;}
				help0 = ftell(help);
			} else
				help0 = ftell(in);
			fprintf(out,"#include \"symbol.H\"\n#include \"objarithm.H\"\n\n");
			fprintf(out,"#line 1 \"%s\"\n",argv[i]);
			parse(in,out,help);
			fprintf(out,"\n#ifdef helpname\n#  undef helpname\n#endif\n");	// ensure that 'helpname' is not a macro
			fprintf(out,"\nstatic char helpname[] = \"%s\";\n\n",HelpFileFlag ? helpname : argv[i]);
			WriteInfo(out,ExtractFilenameBody(argv[i]));
			fclose(in);
			fclose(out);
			if (HelpFileFlag) {
				long help1 = ftell(help);
				if (help1 == help0) {
					fclose(help);
					remove(helpname);	// if helpfile is empty, remove it
				} else
					fclose(help);
			}
		}
	}
	return 0;
}
