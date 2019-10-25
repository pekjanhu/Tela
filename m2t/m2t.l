%name Tm2tlex

DIGIT [0-9]
IDENTIFIER [a-zA-Z\_][a-zA-Z\_0-9]*
OPTBLANKS [ \t]*
NOCOLON [^:,;]+
NUMBERORIDENTIFIER {OPTBLANKS}[a-zA-Z0-9\+\-\.\_]+{OPTBLANKS}
SIMPLEEXPR [a-zA-Z0-9\+\-\.\_\*/ \t]+
NUMBERORIDENTIFIERSEQUENCE [a-zA-Z0-9\ \t\+\-\.\_]+
%{

#define STMT_MAX 8196		/* maximum # of chars in one statement */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *fp,*infp;

int lineno = 1;
int OpenBraceMode = 0;
int MaxParenIndex = 0;
int ParenIndex = 0;
int InLeftHandSide = 1;
int QuoteMeansChar = 1;
int SymToBeTypifiedMatrix = -1;
int LastWasMatrix = 0;
int IdentifierWasThere = 0;
int LastWasRightBracket = 0;

int IgnoreScripts = 0;
int IgnoreFunctions = 0;
int SilentMode = 0;
int VerboseMode = 0;
int NoDivisionWarnings = 0;
int MultiFileMode = 0;

static void warning(const char *str);
static void warning(const char *str, const char *str1);
static void warning(const char *str, const char *str1, const char *str2);

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) { \
	int c = fgetc(infp); \
	(result) = feof(infp) ? YY_NULL : (((buf)[0] = c), 1); \
}

#define INIT_QUOTEMEANSCHAR 0

char stmtbuff[STMT_MAX];		/* buffer where statement is accumulated */
int si = 0;						/* index to stmtbuff */

static void ResetParams() {
	OpenBraceMode = 0;
	ParenIndex = 0;
	MaxParenIndex = 0;
	InLeftHandSide = 1;
	QuoteMeansChar = 1;
	SymToBeTypifiedMatrix = -1;
	LastWasMatrix = 0;
	LastWasRightBracket = 0;
	si = 0;
}

char OldChar = '\0';

static void PutString(const char*S) {
	if (!*S) return;
	int L = strlen(S);
	strcpy(stmtbuff+si,S);
	si+= L;
	OldChar = S[L-1];
	LastWasRightBracket = 0;
}

static void PutChar(char ch) {
	stmtbuff[si++] = ch;
	stmtbuff[si] = '\0';
	OldChar = ch;
	LastWasRightBracket = 0;
}

static int HandleIdentifierSequenceCase(const char*s, FILE *fp) {
	if (!IdentifierWasThere) return 0;
	int i;
	for (i=0; s[i] && s[i]!=';' && s[i]!='\n'; i++)
		if (!(isalnum(s[i]) || isspace(s[i]) || s[i]=='-' || s[i]=='.')) return 0;
	for (i=0; s[i] && s[i]!=';' && s[i]!='\n' && isspace(s[i]); i++) fputc(s[i],fp);
	for (; s[i] && s[i]!=';' && s[i]!='\n' && !isspace(s[i]); i++) fputc(s[i],fp);
	fputc('(',fp);
	for (; s[i] && s[i]!=';' && s[i]!='\n' && isspace(s[i]); i++);
	int FirstTime = 1;
	while (s[i] && s[i]!=';' && s[i]!='\n') {
		if (FirstTime) FirstTime=0; else fputc(',',fp);
		fputc('"',fp);
		for (; s[i] && s[i]!=';' && s[i]!='\n' && !isspace(s[i]); i++) fputc(s[i],fp);
		fputc('"',fp);
		for (; s[i] && s[i]!=';' && s[i]!='\n' && isspace(s[i]); i++);
	}
	fputc(')',fp);
	for (; s[i]; i++) fputc(s[i],fp);
	return 1;
}

static void Flush() {
	if (!HandleIdentifierSequenceCase(stmtbuff,fp))
		fputs(stmtbuff,fp);
	si = 0;
	SymToBeTypifiedMatrix = -1;
	IdentifierWasThere = 0;
}

#define MAXSYM 1000

enum Tkind {SURELY_FUNCTION, PROBABLY_FUNCTION, SURELY_MATRIX, PROBABLY_MATRIX, UNKNOWN};

struct Tsymbol {
	char *s;
	Tkind k;
};

Tsymbol symtab[MAXSYM];
int Nsym = 0;

static void SetSymbolKind(int index, Tkind k) {
	if (k==UNKNOWN) k=symtab[index].k;
	switch (symtab[index].k) {
	case UNKNOWN: break;
	case SURELY_FUNCTION:
		if (k!=PROBABLY_FUNCTION && k!=SURELY_FUNCTION)
			warning("type conflict for symbol '%s', function or matrix?",symtab[index].s);
		break;
	case PROBABLY_FUNCTION:
	case PROBABLY_MATRIX:
		break;
	case SURELY_MATRIX:
		if (k!=PROBABLY_MATRIX && k!=SURELY_MATRIX)
			warning("type conflict for symbol '%s', function or matrix?",symtab[index].s);
		break;
	}
	symtab[index].k = k;
}

static int PutSymbol(const char *name, Tkind kind=UNKNOWN) {
// Put name in symbol table and return the symbol table index
	// Look for symbol in the set of old symbols
	int found=0, index=0;
	for (int i=0; i<Nsym; i++)
		if (!strcmp(name,symtab[i].s)) {
			found = 1;
			index = i;
			break;
		}
	if (found) {
		SetSymbolKind(index,kind);
		return index;
	}
	if (Nsym+1 > MAXSYM) {
		fprintf(stderr,"*** Symbol table overflow, increase MAXSYM\n");
		exit(1);
	}
	symtab[Nsym].s = strdup(name);
	symtab[Nsym].k = kind;
	Nsym++;
	return Nsym-1;
}

static void ClearSymbolTable() {
	for (int i=0; i<Nsym; i++) free(symtab[i].s);
	Nsym = 0;
	PutSymbol("pi",SURELY_MATRIX);
	PutSymbol("eps",SURELY_MATRIX);
	PutSymbol("i",SURELY_MATRIX);
	PutSymbol("Inf",SURELY_MATRIX);
	ResetParams();
}

static void OutputSymbolTable(FILE *fp, char*filename) {
	fprintf(fp,"SYMBOL TABLE FOR FILE '%s':\n",filename);
	for (int i=0; i<Nsym; i++) {
		fprintf(fp,"%-30s",symtab[i].s);
		switch (symtab[i].k) {
		case SURELY_FUNCTION:   fprintf(fp,"surely function"); break;
		case SURELY_MATRIX:     fprintf(fp,"surely matrix"); break;
		case PROBABLY_FUNCTION: fprintf(fp,"probably function"); break;
		case PROBABLY_MATRIX:   fprintf(fp,"probably matrix"); break;
		case UNKNOWN:           fprintf(fp,"UNKNOWN"); break;
		}
		fprintf(fp,"\n");
	}
}

#define MAXPAREN 1000

char ParenStack[MAXPAREN];

static void PutParen(char paren) {
	if (paren!='(' && paren!='[') {
		fprintf(stderr,"*** Internal error: bad paren\n");
		exit(2);
	}
	ParenStack[ParenIndex++] = paren;
	if (ParenIndex > MaxParenIndex) MaxParenIndex = ParenIndex;
	PutChar(paren);
}

static void CloseParen() {
	char ch = ParenStack[--ParenIndex];
	if (ch == '(')
		PutChar(')');
	else {
		PutChar(']');
		LastWasRightBracket = 1;
	}
}

struct Tstringpair {
	char *str1;
	char *str2;
};

Tstringpair IDtranslations[] = {
	{"real","Re"},
	{"imag","Im"},
	{"finite","isfinite"},
	{"fft","FFT"},
	{"ifft","invFFT"},
	{"svd","SVD"},
	{"lu","LU"},
	{"std","stddev"},
	{0,0}
};

char *FunctionalVariables[] = {
	"cputime",
	"tic",
	"toc",
	0
};

%}

%%

 /* recognize forms [x y z ...] where x,y,z.. contain only literal numbers or identifiers */
\[{NUMBERORIDENTIFIERSEQUENCE}\] {
	ParenStack[ParenIndex] = '[';
	PutString((InLeftHandSide&&ParenIndex==0)?"[":"#(");
	LastWasMatrix = 0;
	int LastWasSpace=0;
	for (int i=1; i<yyleng-1; i++)
		if (yytext[i] == ' ') {
			if (!LastWasSpace) PutChar(',');
			LastWasSpace=1;
		} else {
			PutChar(yytext[i]);
			LastWasSpace=0;
		}
	QuoteMeansChar = INIT_QUOTEMEANSCHAR;
	PutChar((InLeftHandSide&&ParenIndex==0)?']':')');
	LastWasMatrix = 0;
	Flush();
}

 /* echo spaces and tabs */
[ \t]+		{ PutString(yytext); QuoteMeansChar++; }

 /* recognize comments */
"%" {
	int c;
	/* eat up the comment, until a newline. Echo comment preceded by '//' to fp */
	PutString("//"); Flush();
	while ( ((c=input()) != '\n') && (c != EOF)) fputc(c,fp);
	lineno++;
	PutChar('\n'); Flush();
}

 /* continuation lines */
"...\n"		{ PutChar('\n'); lineno++; }

 /* statement separators */
",\n"		{ lineno++;
			  if (ParenIndex==0) {
				PutString(OpenBraceMode ? ") {\n" : ";\n");
				OpenBraceMode=0;
				InLeftHandSide=1;
				MaxParenIndex=0;
				Flush();
			  } else {
				PutString(",\n");
				QuoteMeansChar++;
			  }
			  LastWasMatrix = 0;
			}

","			{ if (ParenIndex==0) {
				PutString(OpenBraceMode ? ") {" : ";");
				OpenBraceMode=0;
				InLeftHandSide=1;
				MaxParenIndex=0;
				Flush();
			  } else {
				PutChar(',');
				QuoteMeansChar++;
			  }
			  LastWasMatrix = 0;
			}

";\n"		{ lineno++;
			  if (ParenIndex==0) {
				if (OldChar!='\n')
					PutString(OpenBraceMode ? ") {\n" : ";\n");
				else
					PutString(OpenBraceMode ? ") {\n" : "");
				OpenBraceMode=0;
				InLeftHandSide=1;
				MaxParenIndex=0;
				Flush();
			  } else {
				PutString(";\n");
			  }
			  LastWasMatrix = 0;
			}

";"			{ if (ParenIndex==0) {
				if (OldChar!='\n')
					PutString(OpenBraceMode ? ") {" : ";");
				else
					PutString(OpenBraceMode ? ") {" : "");
				OpenBraceMode=0;
				InLeftHandSide=1;
				MaxParenIndex=0;
				Flush();
			  } else {
				PutChar(';');
			  }
			  LastWasMatrix = 0;
			}

"\n"		{ lineno++;
			  if (ParenIndex==0) {
				if (OldChar!='\n')
					PutString(OpenBraceMode ? ") {\n" : ";\n");
				else
					PutString(OpenBraceMode ? ") {\n" : "\n");
				OpenBraceMode=0;
				InLeftHandSide=1;
				MaxParenIndex=0;
				Flush();
			  } else
				PutChar('\n');
			  LastWasMatrix = 0;
			}

 /* empty array constructor */
"["[ ]*"]"	{ PutString("zeros(0)"); QuoteMeansChar=INIT_QUOTEMEANSCHAR; }

  /* recognize keywords */
"if"            { PutString("if ("); OpenBraceMode=1; InLeftHandSide=0; MaxParenIndex=0; QuoteMeansChar++; Flush(); }
"else"			{ PutString("} else {"); QuoteMeansChar++; Flush(); }
"elseif"        { PutString("} else if ("); OpenBraceMode=1; InLeftHandSide=MaxParenIndex=0; QuoteMeansChar++; Flush(); }
"while"         { PutString("while ("); OpenBraceMode=1; InLeftHandSide=MaxParenIndex=0; QuoteMeansChar++; Flush(); }
"for"           { PutString("foreach ("); OpenBraceMode=1; InLeftHandSide=MaxParenIndex=0; QuoteMeansChar++; Flush(); }
"break"         { PutString("break"); QuoteMeansChar++; Flush();}
"return"		{ PutString("return"); QuoteMeansChar++; Flush(); }
"end"			{ PutChar('}'); QuoteMeansChar++; Flush(); }
 /*"function"		return FUNCTION;*/

 /* some operators */

"=="		{ PutString("=="); QuoteMeansChar++; LastWasMatrix=0; }
"="			{
				PutChar('=');
				QuoteMeansChar++;
				if (ParenIndex==0) {
					InLeftHandSide=0;
					if (SymToBeTypifiedMatrix>=0) {
						SetSymbolKind(SymToBeTypifiedMatrix,SURELY_MATRIX);
						if (MaxParenIndex > 0)
							warning("Edit %s(...) = ...; to %s[...] = ...; in output. Sorry.",
									symtab[SymToBeTypifiedMatrix].s, symtab[SymToBeTypifiedMatrix].s);
					}
					MaxParenIndex = 0;
				}
				LastWasMatrix = 0;
			 }

"("			{ PutParen(LastWasMatrix ? '[' : '('); QuoteMeansChar++; LastWasMatrix=0; }
")"			{ CloseParen(); QuoteMeansChar=INIT_QUOTEMEANSCHAR; LastWasMatrix=0; }
"["			{ ParenIndex++;
			  ParenStack[ParenIndex-1] = '[';
			  PutString((InLeftHandSide&&ParenIndex==1)?"[":"#(");
			  QuoteMeansChar++;
			  LastWasMatrix = 0;
			}
"]"			{ ParenIndex--;
			  QuoteMeansChar = INIT_QUOTEMEANSCHAR;
			  PutChar((InLeftHandSide&&ParenIndex==0)?']':')');
			  LastWasMatrix = 0;
			}

".*"			{ PutChar('*'); QuoteMeansChar++; LastWasMatrix=0; }
"*"				{ PutString("**"); QuoteMeansChar++; LastWasMatrix=0; }
"./"			{ PutChar('/'); QuoteMeansChar++; LastWasMatrix=0; }
"/"				{ PutChar('/'); if (!NoDivisionWarnings) warning("Matrix division seen"); QuoteMeansChar++; LastWasMatrix=0; }
".^"			{ PutChar('^'); QuoteMeansChar++; LastWasMatrix=0; }
"^"				{ PutChar('^'); warning("Matrix power seen"); QuoteMeansChar++; LastWasMatrix=0; }
".'"			{ PutString(".'"); QuoteMeansChar++; LastWasMatrix=0; }
"~="			{ PutString("!="); QuoteMeansChar++; LastWasMatrix=0;}
"&"				{ PutString("&&"); QuoteMeansChar++; LastWasMatrix=0;}
"|"				{ PutString("||"); QuoteMeansChar++; LastWasMatrix=0;}
"~"				{ PutChar('!'); QuoteMeansChar++; LastWasMatrix=0;}
"+"				{ PutChar('+'); QuoteMeansChar++; LastWasMatrix=0;}
"-"				{ PutChar('-'); QuoteMeansChar++; LastWasMatrix=0;}
":"				{ PutChar(':'); QuoteMeansChar++; LastWasMatrix=0;}
">"				{ if (LastWasRightBracket) PutChar(' '); PutChar('>'); QuoteMeansChar++; LastWasMatrix=0; }

 /* some miscellaneous translations */
"hold on"			{ PutString("hold(on)"); QuoteMeansChar++; LastWasMatrix=0; }
"hold off"			{ PutString("hold(off)"); QuoteMeansChar++; LastWasMatrix=0; }

rem{OPTBLANKS}\({SIMPLEEXPR},{SIMPLEEXPR}\) {
	char *arg1 = strchr(yytext,'(') + 1;
	char *arg2 = strchr(yytext,',') + 1;
	*(arg2-1) = '\0';			/* set null to ',' */
	yytext[yyleng-1] = '\0';	/* set null to ')' */
	PutString("((");
	PutString(arg1);
	PutString(") mod (");
	PutString(arg2);
	PutString("))");
	QuoteMeansChar++;
	LastWasMatrix=0;
}

ones{OPTBLANKS}\({OPTBLANKS}1{OPTBLANKS},{SIMPLEEXPR}\) {
	PutString("ones(");
	PutString(strchr(yytext,',')+1);
	QuoteMeansChar++;
	LastWasMatrix=0;
}

zeros{OPTBLANKS}\({OPTBLANKS}1{OPTBLANKS},{SIMPLEEXPR}\) {
	PutString("zeros(");
	PutString(strchr(yytext,',')+1);
	QuoteMeansChar++;
	LastWasMatrix=0;
}

ones{OPTBLANKS}\({SIMPLEEXPR},{OPTBLANKS}1{OPTBLANKS}\)	{
	*strrchr(yytext,',') = '\0';	/* set null at the ',' */
	PutString(yytext);
	PutChar(')');
	QuoteMeansChar++;
	LastWasMatrix=0;
}

zeros{OPTBLANKS}\({SIMPLEEXPR},{OPTBLANKS}1{OPTBLANKS}\)	{
	*strrchr(yytext,',') = '\0';	/* set null at the ',' */
	PutString(yytext);
	PutChar(')');
	QuoteMeansChar++;
	LastWasMatrix=0;
}

ones\({SIMPLEEXPR}\)	{
	yytext[yyleng-1] = '\0';	/* set null at the ')' */
	PutString(yytext);
	PutChar(',');
	PutString(yytext+5);
	PutChar(')');
	QuoteMeansChar++;
	LastWasMatrix=0;
}

zeros\({SIMPLEEXPR}\)	{
	yytext[yyleng-1] = '\0';	/* set null at the ')' */
	PutString(yytext);
	PutChar(',');
	PutString(yytext+6);
	PutChar(')');
	QuoteMeansChar++;
	LastWasMatrix=0;
}

 /* Identifiers */
 /* an identifier begins with a letter, the rest can be */
 /* also underscores and digits */
{IDENTIFIER} {
	QuoteMeansChar = INIT_QUOTEMEANSCHAR;
	/*fprintf(stderr,"IDENTIFIER %s\n",yytext);*/
	char *s = yytext;
	int i;
	for (i=0; IDtranslations[i].str1; i++)
		if (!strcmp(IDtranslations[i].str1,yytext)) {
			s = IDtranslations[i].str2;
			break;
		}
	int IsFunctional = 0;
	for (i=0; FunctionalVariables[i]; i++) {
		if (!strcmp(yytext,FunctionalVariables[i])) {
			IsFunctional = 1;
			break;
		}
	}
	PutString(s);
	int symindex=PutSymbol(s);
	if (InLeftHandSide && ParenIndex==0)
		SymToBeTypifiedMatrix = symindex;
	if (symtab[symindex].k == SURELY_MATRIX) LastWasMatrix=1;
	if (IsFunctional) PutString("()");
	IdentifierWasThere = 1;
}

\'		{ if (QuoteMeansChar) {
			int ch;
			PutChar('"');
			for(;;) {
				ch = input();
				if (feof(infp)) break;
				if (ch=='\'') {
					int ch2 = input();
					if (ch2=='\'') {
						PutString("\\\"");
						continue;
					} else {
						unput(ch2);
						break;
					}
				}
				if (ch == '"') PutChar('\\');
				PutChar(ch);
			}
			PutChar('"');
		  } else PutChar('\'');
		}

.		PutChar(*yytext);

%%

static void warning(const char *str) {
	if (SilentMode) return;
	fprintf(stderr,"line %d warning: %s\n",lineno,str);
}

static void warning(const char *str, const char *str1) {
	if (SilentMode) return;
	fprintf(stderr,"line %d warning: ",lineno);
	fprintf(stderr,str,str1);
	fprintf(stderr,"\n");
}

static void warning(const char *str, const char *str1, const char *str2) {
	if (SilentMode) return;
	fprintf(stderr,"line %d warning: ",lineno);
	fprintf(stderr,str,str1,str2);
	fprintf(stderr,"\n");
}

static void Basename(char *s) {
	char *SlashPos = strrchr(s,'/');
	if (SlashPos) {
		int L = strlen(SlashPos);
		memmove(s,SlashPos+1,L+1);
	}
	char *DotPos = strrchr(s,'.');
	if (DotPos) *DotPos = '\0';
}

static void PutArgumentSymbolsToSymbolTable(const char *s, const char *fname) {
	char token[256];
	int t=0;
	for (int i=0; s[i]; i++) {
		if (isalnum(s[i]))
			token[t++] = s[i];
		else if (t>0) {
			token[t] = '\0';
			if (strcmp(token,"function")!=0) {
				PutSymbol(token,strcmp(fname,token) ? SURELY_MATRIX : SURELY_FUNCTION);
			}
			t = 0;
		}
	}
}

static void GiveHelp() {
	printf("m2t - Matlab to Tela translator.\n");
	printf(" -> Warnings: Use at your own risk. You must almost always do some hand\n");
	printf("    editing afterwards. This program is provided to make transition to\n");
	printf("    Tela easier, not to 'emulate' Matlab.\n");
    printf(" -> It may be possible to violate the law by translating copyrighted Matlab\n");
	printf("    code to Tela. The author of this program does not take any responsibility\n");
	printf("    for that kind of problems, and you as a user must find out under what\n");
	printf("    conditions you can use this program.\n");
	printf("Usage: m2t [-h?]                        (give help)\n");
	printf("   or: m2t [-sdv] <file.m >file.t       (basic usage)\n");
	printf("   or: m2t [-FSmsvd] [files.m] >file.t  (many files at once)\n");
	printf("Options:\n");
	printf("    -F        Ignore script files (Functions only)\n");
	printf("    -S        Ignore function files (Scripts only)\n");
	printf("    -m        Multi-file mode: generate .t files using .m file basenames\n");
	printf("    -s        Silent mode, suppress all warnings\n");
	printf("    -v        Verbose mode\n");
	printf("    -d        Suppress matrix division warnings\n");
	printf("    -h, -?    Produce this message\n");
	printf("Examples:\n");
	printf("    m2t -F *.m >funcs.t    Compile all function files into \"funcs.t\"\n");
	printf("    m2t -Sm *.m            Compile all scripts in separate t-files\n");
	exit(0);
}

static void ParseArgs(int argc, char *argv[], int& arg)
// Recognizes options -F -S -m -s -v -d -h -?
{
	for (; arg<argc && argv[arg][0]=='-'; arg++) {
		for (int i=1; argv[arg][i]; i++) {
			int ch = argv[arg][i];
			if (ch == 'F') {
				IgnoreScripts = 1; IgnoreFunctions = 0;
			} else if (ch == 'S') {
				IgnoreFunctions = 1; IgnoreScripts = 0;
			} else if (ch == 'm') {
				MultiFileMode = 1;
			} else if (ch == 's') {
				SilentMode = 1;
				VerboseMode = 0;
			} else if (ch == 'v') {
				VerboseMode = 1;
				SilentMode = 0;
			} else if (ch == 'd')
				NoDivisionWarnings = 1;
			else if (ch == 'h' || ch == '?')
				GiveHelp();
		}
	}
}

static int IsCommentLine(const char *s) {
	for (int i=0; s[i]; i++) {
		if (s[i] == '%') return 1;
		if (!isspace(s[i])) return 0;
	}
	return 1;
}

static int FirstIsFunction(char *s) {
	static const char Function[] = "function";
	const int L0 = strlen(Function);
	int i;
	for (i=0; s[i] && isspace(s[i]); i++);
	int ch = s[i+L0];
	if (!isspace(ch)) return 0;
	s[i+L0] = '\0';
	int result = !strcmp(s+i,Function);
	s[i+L0] = ch;
	return result;
}

static void GenerateTFileName(char tfile[FILENAME_MAX], const char fn[]) {
	int L = strlen(fn);
	strcpy(tfile,fn);
	if (fn[L-1] == 'm' && fn[L-2] == '.') {
		tfile[L-1] = 't';
	} else
		strcat(tfile,".t");
}

int main(int argc, char *argv[])
{
#	define maxlinebuff 512
	fp = stdout;
	infp = stdin;
	int Arg = 1;
	ParseArgs(argc,argv,Arg);
	if (argc <= Arg) {
		Tm2tlex a;
		ClearSymbolTable();
		a.yylex();
		if (VerboseMode) OutputSymbolTable(stderr,"standard input");
	} else {
		int arg;
		for (arg=Arg; arg<argc; arg++) {
			lineno = 1;
			char tfile[FILENAME_MAX];
			if (MultiFileMode) {
				GenerateTFileName(tfile,argv[arg]);
				fp = fopen(tfile,"w");
				if (!fp) {
					fprintf(stderr,"*** Could not open '%s', using stdout\n",tfile);
					fp = stdout;
				}
			}
			Tm2tlex a;
			YY_BUFFER_STATE bufstate;
			int L;
			char linebuff[maxlinebuff+1];
			infp = fopen(argv[arg],"r");
			if (!infp) {fprintf(stderr,"warning: could not open '%s'\n",argv[arg]); continue;}
			for (;;) {
				fgets(linebuff,maxlinebuff,infp);
				lineno++;
				if (!IsCommentLine(linebuff)) break;
				for (int i=0; linebuff[i]; i++)
					if (linebuff[i] == '%')
						fputs("//",fp);
					else
						fputc(linebuff[i],fp);
			}
			L = strlen(linebuff);
			if (linebuff[L-1]=='\n') linebuff[--L]='\0';
			if (linebuff[L-1]==';') linebuff[--L]='\0';
			a.yyin = infp;
			if (FirstIsFunction(linebuff)) {
				if (IgnoreFunctions) {
					fprintf(stderr,"Ignoring FUNCTION file '%s' (-S option)\n",argv[arg]);
				} else {
					fprintf(stderr,"Processing FUNCTION file '%s' ...\n",argv[arg]);
					ClearSymbolTable();
					char *fname = strdup(argv[arg]);
					Basename(fname);
					PutArgumentSymbolsToSymbolTable(linebuff,fname);
					free(fname);
					fprintf(fp,"\n\n");
					fputs(linebuff,fp);
					fprintf(fp," {\n");
					a.yylex();
					fprintf(fp,"\n};\n");
					if (VerboseMode) OutputSymbolTable(stderr,argv[arg]);
				}
			} else {
				if (IgnoreScripts) {
					fprintf(stderr,"Ignoring SCRIPT file '%s' (-F option)\n",argv[arg]);
				} else {
					rewind(infp);
					fprintf(stderr,"Processing SCRIPT file '%s' ...\n",argv[arg]);
					ClearSymbolTable();
					a.yylex();
					if (VerboseMode) OutputSymbolTable(stderr,argv[arg]);
				}
			}
			fclose(infp);
			if (MultiFileMode && fp!=stdout) {
				if (ftell(fp)==0) {
					fclose(fp);
					remove(tfile);
				} else
					fclose(fp);
			}
		}
	}
	return 0;
}
