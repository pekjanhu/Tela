%name Tlexer

%define INHERIT :public Tparser
%define MEMBERS \
	int seekback(Tchar[],int); \
	int AtEOF(); \
	int QuoteMeansChar;\
	istream* inputstreamptr; \
	int EofSeen; \
	virtual ~Tlexer();

%define CONSTRUCTOR_PARAM istream& is
%define CONSTRUCTOR_CODE QuoteMeansChar=1; inputstreamptr = &is; EofSeen = 0

%{

#include <string.h>
#include "tree.H"
#include "d.y.h"
/*#undef YY_READ_BUF_SIZE*/
/*#define YY_READ_BUF_SIZE 512*/

static Tchar *strdup_Stripped(const Tchar[]);
static void rm_escape(Tchar *);

#define YY_USER_ACTION QuoteMeansChar++;

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)\
{\
	if (EofSeen) {\
		(result) = 0;\
	} else if (inputstreamptr->good()) {\
		inputstreamptr->read((char*)(buf),(max_size));\
		(result) = inputstreamptr->gcount();\
	} else {\
		*(char*)buf = '}'; (result) = 1; EofSeen = 1;\
    }\
}

%}

DIGIT [0-9]

%%	/* beginning of rules section */

 /* skip blanks and tabs */
[ \t]+	{ QuoteMeansChar--; }

 /* skip also newlines, but keep track of line numbers */
\n		{ QuoteMeansChar--; global::lineno++; }

 /* recognize keywords */
"if"            return IF;
"else"          return ELSE;
"while"         return WHILE;
"for"           return FOR;
"foreach"		return FOREACH;
"break"         return BREAK;
"continue"      return CONTINUE;
"return"		return RETURN;
"repeat"		return REPEAT;
"until"			return UNTIL;
"goto"			return GOTO;
"label"			return LABEL;
"function"		return FUNCTION;
"package"		return PACKAGE;
"local"			return LOCAL;
"global"		return GLOBAL;
"call"			return CALL;
"disp"			return DISP;
"mod"			return MOD;

 /* recognize C style comments */
"/*" {
	int c,d,si=1;
	QuoteMeansChar--;
	for(;;) {
		while ( ((c=input()) != '*') && (c != '/') && (c != EOF)) if (c=='\n') global::lineno++;
		if (c == '/') {
			if ((d=input()) == '*') si++; else if (d=='\n') global::lineno++;
		} else if (c == '*') {
			d = input();
			if (d == '/') si--; else unput(d);
		} else break;	/* now c must be EOF */
		if (si == 0) break;
	}
}

 /* recognize C++ style comments */
"//" {
	int c;
	QuoteMeansChar--;
	/* eat up the comment, until a newline */
	while ( ((c=input()) != '\n') && (c != EOF)) ;
	global::lineno++;
}

 /* read literal integers */
{DIGIT}+ {
	double x = atof((const char*)yytext);
	QuoteMeansChar = -1;
	if (x > MAXTINT-1) {	/* if too large, handle it as real */
		yylval.real_val = x;
		return REAL;
	} else {
		yylval.int_val = Tint(x+0.5);
		return INTEGER;
	}
}

 /* read literal real numbers */
{DIGIT}+"."			|
{DIGIT}*"."{DIGIT}+ |
{DIGIT}+[eE][+-]?{DIGIT}+ {
	QuoteMeansChar = -1;
	yylval.real_val = atof((const char*)yytext);
	return REAL;
}

{DIGIT}*"."{DIGIT}*[eE][+-]?{DIGIT}+ { 
	QuoteMeansChar = -1;
	yylval.real_val = atof((const char*)yytext);
	return REAL;
}

 /* read imaginary constants */
({DIGIT}+"."?)+(i|j)        |
({DIGIT}*"."{DIGIT}+)+(i|j) |
({DIGIT}+[eE][+-]?{DIGIT}+)+(i|j) { 
	QuoteMeansChar = -1;
	yylval.real_val = atof((const char*)yytext);
	return iREAL;
}

({DIGIT}*"."{DIGIT}*[eE][+-]?{DIGIT}+)+(i|j) { 
	QuoteMeansChar = -1;
	yylval.real_val = atof((const char*)yytext);
	return iREAL;
}

 /* read identifiers */
 /* an identifier begins with a letter or dollar sign, the rest can be */
 /* also underscores and digits */
[a-zA-Z\$äÄöÖåÅ][a-zA-Z0-9\_\$äÄöÖåÅ]* {
	QuoteMeansChar = -1;
	Tsymbol* symptr = theHT.add(yytext);
	yylval.sym_val = symptr;
	return(IDENTIFIER);
}

 /* get character strings enclosed in " " */
\"([^\\\"]|\\(.|\n))*\"  {
	yylval.str_val = strdup_Stripped(yytext);	// Strip means: leave surrounding quotes off
	for (int i=0; yylval.str_val[i]; i++)
		if (yylval.str_val[i] == '\n') global::lineno++;
	rm_escape(yylval.str_val);	// do escape sequences in place
	return(STRING);
}

".'"	return(TRANSPOSE);

 /* A single quote starts a character constant if QuoteMeansChar is nonzero.
    If QuoteMeansChar is zero, it is the hermitian trasnpose. */
\' {
	if (QuoteMeansChar) {
		Tchar buff[20];
		int L=0;
		buff[L++] = '\'';
		buff[L++] = input();
		if (buff[L-1]=='\\') {
			buff[L++] = input();
			do {
				buff[L++] = input();
				if (L>=19) return(ERROR_TOKEN);
			} while (buff[L-1]!='\'');
		} else {
			buff[L++] = input();
			if (buff[L-1]!='\'') return(ERROR_TOKEN);
		}
		buff[L] = '\0';
		Tchar *ptr = strdup_Stripped(yytext);
		rm_escape(ptr);
		yylval.ch_val = *ptr;
		delete [] ptr;
		return(CHAR);
	} else {		/* Else Quote Means Hermitian Transpose */
		return(HERMITIAN_TRANSPOSE);
	}
}		

"++"	return INC;
"--"	return DEC;
"=="	return EQ;
">="	return GE;
">"		return GT;
"<="	return LE;
"<"		return LT;
"!="	return NE;
"!"		return NOT;
"&&"	return AND;
"||"	return OR;
"<["	return DOUBLELEFTBRACKET;
"]>"	{ QuoteMeansChar = -1; return DOUBLERIGHTBRACKET; }
"#("	return LEFTCONSTRUCTOR;
"..."	return ELLIPSIS;
"**"	return DOT;
"+="	return INCASSIGN;
"-="	return DECASSIGN;
"*="	return MULASSIGN;
"/="	return DIVASSIGN;

")"		{ QuoteMeansChar = -1; return yytext[0]; }
"]"		{ QuoteMeansChar = -1; return yytext[0]; }

.		{ return yytext[0]; }

%%

int Tlexer::seekback(Tchar s[], int N) {
	// s must be a pointer to N chars. This function tries to fetch N most recent characters
	// from the input stream and place them to s. If the stream does not contain that many
	// characters, the routine does not fetch them all --- in any case the function returns
	// the number of characters actually fetched and placed in s.
	Tchar *startptr = yy_current_buffer->yy_ch_buf;
	int L = strlen(startptr);
	Tchar *endptr = startptr + L;
	L--;
	if (N<L) L=N;
	s[L] = '\0';
	for (int i=1; i<=L; i++) s[L-i] = endptr[-i];
	return L;
}

int Tlexer::AtEOF() {
	if (yy_current_buffer)
		return !inputstreamptr->good();
	else	// if no buffer yet allocated, EOF is false:
		return 0;
}

Tlexer::~Tlexer() {YY_Tlexer_DELETE_BUFFER(yy_current_buffer);}

static Tchar *strdup_Stripped(const Tchar s[]) {
	int L = strlen(s);
	Tchar *result = new Tchar [L-1];
	for (int i=1; i<L-1; i++) result[i-1] = s[i];
	result[L-2] = '\0';
	return result;
}

// The following code fragment taken from RLaB by Ian Searle,
// who in turn obtained it from Mike Brennan...

/* **************************************************************
 * Process STRINGS before giving them to yyparse(). This piece of
 * code obtained from MAWK (GNU copylefted) by Mike Brennan.
 * However I have modified it so any bugs are my fault.
 * ************************************************************** */

// process the escape characters in a string, in place

static void rm_escape(Tchar *s)
{ 
	Tchar *p=s, *q=s;
	int i;
	const int ET_END = 10;
	struct Tescape_test { 
		Tchar in;
		Tchar out;
	};
	static Tescape_test escape_test[ET_END+1] = { 
		{ 'n' , '\n' },
		{ 't' , '\t' },
		{ 'f' , '\f' },
		{ 'b' , '\b' },
		{ 'r' , '\r' },
		{ 'a' , '\07' },
		{ 'v' , '\013' },
		{ '\\', '\\' },
		{ '\"', '\"' },
		{ '\n', '\0' },
		{ 0 , 0 }
	};
	while (*p)
		if ( *p == '\\' ) { 
			escape_test[ET_END].in = *++p;	// sentinal
			i = 0;
			while (escape_test[i].in != *p) i++;
			if (i != ET_END) {		// in table
				p++;
				if (escape_test[i].out) *q++ = escape_test[i].out;
			} else if (*p == '\0')	// not sure when this can happen yet
				*q++ = '\\' ;
			else  {					// not an escape sequence
				*q++ = '\\';
				*q++ = *p++;
			}
		} else
			*q++ = *p++;
	*q = '\0';
}
