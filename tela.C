/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#ifdef __GNUC__
#  pragma implementation "common.H"
#  if __GNUC__ >= 3
#    include <ext/stdio_filebuf.h>
#  endif
#endif
#include <fstream>
#include "tree.H"
#include "prg.H"
#include "common.H"
#include "d.y.h"
#include "d.l.h"
#include "version.H"
#include <values.h>
//#include <unistd.h>
#if HAVE_IEEE754_H == 1
#  include <ieee754.h>
#endif
//#ifdef HAVE_ISTREAM /* use STL */
//#  include "fdstream.H"
//#endif /* HAVE_ISTREAM */

#if defined(_AIX) || defined(__sgi)
   // Switch to ANSI to get a reasonable signal(3) from signal.h.  ANSI
   // also ensures that RETSIGTYPE is void, so we can safely ignore what
   // configure found.
#  define _ANSI_C_SOURCE
#  undef RETSIGTYPE
#  define RETSIGTYPE void
#endif

#include <signal.h>

extern "C" {
#include "readline/chardefs.h"
extern Tchar *readline(...);
extern Tchar **completion_matches(...);
extern int rl_add_defun(...);
extern Tchar* rl_readline_name;
extern Tchar* rl_line_buffer;
extern char* rl_basic_word_break_characters;
extern void add_history(...);
typedef int Function(...);
extern Function *rl_attempted_completion_function;
// On some Linux versions the libf77.so refers to external symbol
// MAIN__ which is supposed to be the Fortran main program. Since we
// do not have a Fortran main program here, we define it here.  Hope
// it doesn't conflict with anything else.
#ifdef LINUX
int MAIN__;
#endif
}

char *VersionString = VERSION;
char *CompiledString = WHEN_COMPILED;

typedef RETSIGTYPE (*sig_handler)(int);

// Static global members from common.H:
int flags::verbose = 0;
int flags::echo = 0;
int flags::batch = 0;
int flags::silent = 0;
int flags::debugquery = 0;
int flags::optimize = 0;
int flags::profiling = 0;
#if SLOW_FLOATING_POINT_MATH
int
#else
double
#endif
global::nops = 0;
int global::lineno = 1;
int global::NTprg = 0;
int global::NTobject = 0;
char *global::argv0 = "";
#if !HAVE_RUSAGE && HAVE_SYS_PROCFS_H
#include <fcntl.h>
int global::procfd = -1;
#endif
#if SUPPORT_LOOP_TILING
int global::tile_in_use = 0;
Tint global::tile_start = 0;
Tint global::tile_end = 0;
Tint global::tile_i = 0;
Tint global::tile_Ntiles = 0;
Tint global::tile_arraylength = 0;
#endif
int flags::checkRO = 1;
ostream* global::ctfile = 0;
int global::interrupted = 0;
int global::debugging = 0;
//Tnode *global::globals = 0;
size_t global::memuse = 0;
size_t global::memalloc = 0;
unsigned int global::Nalloc = 0;
unsigned int global::Nmem = 0;
unsigned int global::FLlen = 0;

const size_t MAX_TELAPATH_LENGTH = 100;
// maximum number of directory components in TELAPATH

Tchar *TelaPath[MAX_TELAPATH_LENGTH];
size_t TelaPathLength = 0;



void Tparser::yyerror(char *dummy) {
	const int Nseek = 80;
	const int Nnewlines = 2;
	Tchar s[Nseek+2];
	cerr << "*** Syntax error in file \"" << (const char*)filename
	     << "\" at line " << global::lineno << ":\n";
	int L = ((Tlexer*)this)->seekback(s,Nseek);
	int i,nnewlines=0;
	for (i=L-1; i>=0 && nnewlines<Nnewlines; i--)
	    if (s[i]=='\n') nnewlines++;
	for (; i>=0; i--) if (s[i]=='\n') break;
	cerr << s+i+1 << "~~~~~~~" << '\n';
	wasSyntaxError = 1;
	//exit(1);
}

int FindLineNumber() {
	return theCodeStack.isempty() ? -1 : theCodeStack.top()->FindLineNumber();
}

const Tchar *FindFileName() {
	return theCodeStack.isempty() ? (const Tchar*)"unknown" : theCodeStack.top()->FileName();
}

#if 0
void DumpStack() {
	cerr << "Stack dump:\n";
	for (int i=0; i<theStack.length(); i++)
		cerr << *theStack[i] << '\n';
}

static long MemInUse() {
	rusage usage;
	int code=getrusage(RUSAGE_SELF,&usage);
	if (code) {
		cerr << "*** warning: getrusage returned " << code << '\n';
	}
	return usage.ru_ixrss + usage.ru_idrss + usage.ru_isrss;
}
#endif

static void ResetStacks() {
	// Shorten all stacks to zero length
	theStack.pop(theStack.length());
	for (int i=0; i<thePCstack.length(); i++) thePCstack.pop();
	while (!theCodeStack.isempty()) theCodeStack.pop();
}

extern void GenerateCode(Tnode*x, Tprg& prg, int printsetflag=0);
extern void InstallIntrinsic();
extern void InstallStatic();
extern void ConsistencyChecks();
extern void Ctgen(ostream& o, Tprg& prg);

static Treal FindMachineInfinity() {
#	if defined(HUGE_VAL)
	return HUGE_VAL;
#	elif defined(HUGE)
	return HUGE;
#	elif defined(DBL_MAX)
	return DBL_MAX;
#	else
	return 1e38;
#	endif
}

Treal MachineEpsilon;	// this will be initialized by the following function

static Treal FindMachineEpsilon() {
	// We have to use the volatile variable a in order to prevent compiler
	// from assigning it in register. On 386 (Linux) at least, the accuracy
	// of FP registers is greater than the accuracy of double variables,
	// and this would lead to too small epsilon being found.
	Treal eps = 1;
	volatile Treal a;
	do {
		eps/= 2;
		a = 1 + eps;
	} while (a > 1);
	eps*= 2;
	return eps;
}

static Treal FindMachineNan() {
#if HAVE_IEEE754_H == 1
	if (sizeof(Treal) == sizeof(float)) {
		ieee754_float nan;
		nan.ieee_nan.negative  = 0;
		nan.ieee_nan.exponent  = 0xff;
		nan.ieee_nan.quiet_nan = 1;
		nan.ieee_nan.mantissa  = 0;
		return nan.f;
	} else if (sizeof(Treal) == sizeof(double)) {
		ieee754_double nan;
		nan.ieee_nan.negative  = 0;
		nan.ieee_nan.exponent  = 0x7ff;
		nan.ieee_nan.quiet_nan = 1;
		nan.ieee_nan.mantissa0 = 0;
		nan.ieee_nan.mantissa1 = 0;
		return nan.d;
	/*
	} else if (sizeof(Treal) == sizeof(long double)) {
		ieee854_long_double nan;
		nan.ieee_nan.negative  = 0;
		nan.ieee_nan.exponent  = 0x7fff;
		nan.ieee_nan.empty     = 0;
		nan.ieee_nan.one       = 1;
		nan.ieee_nan.quiet_nan = 1;
		nan.ieee_nan.mantissa0 = 0;
		nan.ieee_nan.mantissa1 = 0;
		return nan.d;
	*/
	} else {
		return 0.0;
	}
#else
	real zero = 0.0;
	real result = zero/zero;
	return result;
#endif
}

static Tsymbol* NewGlobal(const char *name) {
	Tsymbol *result = theHT.add((const Tchar*)name);
	result->SetGlobalFlag();
	result->SetHiddenFlag();
	return result;
}

static void InstallGlobalSymbols() {
	// Install symbols 'on' and 'off', with values 1 and 0, respectively
	Tsymbol* symptr = NewGlobal("on");
	*(symptr->value()) = 1;
	symptr = NewGlobal("off");
	*(symptr->value()) = 0;
	// Install constant symbol 'pi'
	symptr = NewGlobal("pi");
	*(symptr->value()) = 3.14159265358979323846264;
	// Install constant symbol 'Inf'
	symptr = NewGlobal("Inf");
	*(symptr->value()) = FindMachineInfinity();
    // Only install NaN if it is supported by the compiler:
	// IEEE754 requires that NaN != any_number and NaN != NaN.
	if (FindMachineNan() != FindMachineNan()) {
		symptr = NewGlobal("NaN");
		*(symptr->value()) = FindMachineNan();
	}
	// Install constant symbol 'eps'
	symptr = NewGlobal("eps");
	MachineEpsilon = FindMachineEpsilon();
	*(symptr->value()) = MachineEpsilon;
	// Install constants 'overlay', 'paging', 'stacking' (see plotmtv.ct)
	symptr = NewGlobal("overlay");
	*(symptr->value()) = 1;
	symptr = NewGlobal("paging");
	*(symptr->value()) = 2;
	symptr = NewGlobal("stacking");
	*(symptr->value()) = 3;
	// Install some integer constants for export_CDF (see CDF.ct)
	symptr = NewGlobal("CDF_FLOAT");
	*(symptr->value()) = 1;
	symptr = NewGlobal("CDF_DOUBLE");
	*(symptr->value()) = 2;
	symptr = NewGlobal("CDF_RECORD");
	*(symptr->value()) = 4;
	symptr = NewGlobal("CDF_NORECORD");
	*(symptr->value()) = 8;
	// Also put the reserved words to the symbol table.  This does
	// no harm since the user is unable to ever generate these
	// names syntactically. The benefit: GNU readline can complete
	// these words also.
	static const char*const reserved_words[] = {
		"if", "else", "while", "for", "foreach", "break", "continue",
		"return", "repeat", "until", "function", "package", "local",
		"global", "call", "disp", "mod", 0};
	for (int i=0; reserved_words[i]; i++)
	    theHT.add((const Tchar*)reserved_words[i]);
}

static int FastStart = 0;	// if nonzero, do not read telainit.t
static int UseReadlineLibrary = 1; // if zero, do not call readline but use fgets

static void ShowVersion() {
	cout << "Tela the tensor language, version " << VersionString << ".\n"
		    "Compiled using " << CompiledString << ".\n" << flush;
}

static void GiveHelp() {
	cout << "Usage: tela [-befhnpsvV?] [-O0] [-O1] [-O]\n"
	        "            [--batch] [--execute code] [--fast] [--help] [--noreadline]\n"
			"            [--profile] [--silent] [--verbose] [--version]\n"
	        "            [inputfiles.t]\n"
	        "Starts tela. Named input files are executed before entering interactive mode.\n"
	        "Options:\n"
	        "  -b, --batch          Quit after executing named input files (batch mode)\n"
			"  -e, --execute code   Execute given Tela code before doing anything else\n"
	        "  -f, --fast           Do not source telainit.t\n"
	        "  -h, -\\?, --help      Give this help message\n"
		    "  -n, --noreadline     Direct input without GNU readline, No prompts\n"
	        "  -O0                  Do not optimize flatcode (the default)\n"
	        "  -O1, -O              Optimize flatcode\n"
		    "  -p  --profile        Give execution time profile report upon exit\n"
	        "  -s, --silent         Suppress all unnecessary messages\n"
	        "  -v, --verbose        Set tela very verbose: list treecodes and flatcodes\n"
	        "  -V, --version        Print version number and exit\n"
		    "For example: tela -bs -e 'x=3; s=\"a\"' myfile.t sets x equal to 3 and\n"
		    "  s equal to string \"a\", then runs myfile.t and exits, doing it all silently.\n"
		 << flush;
	exit(0);
}

static Tchar *executelist[100] = {0};
static int Nexecutes = 0;

static void ParseArgs(int argc, char **& argv, int& arg)
// recognizes options -bevVshfp?n -O0 -O1 -O
// --batch, --execute, --verbose, --version, --silent, --help, --fast
// --noreadline --profile
{
	for (; arg<argc && argv[arg][0]=='-'; arg++) {
		if (argv[arg][1] == 'O') {
			if (argv[arg][2] == '0') {
				flags::optimize = 0;
			} else if (argv[arg][2] == '1' || argv[arg][2] == '\0') {
				flags::optimize = 1;
			} else {
				cerr << "%%% Unknown optimization option -O"
				     << argv[arg][2] << '\n';
			}
		} else if (argv[arg][1] == '-') {
			if (!strcmp(argv[arg]+2,"silent")) {
				flags::silent = 1;
				flags::verbose = 0;
			} else if (!strcmp(argv[arg]+2,"batch"))
				flags::batch = 1;
			else if (!strcmp(argv[arg]+2,"verbose")) {
				flags::verbose = 1;
				flags::silent = 0;
			} else if (!strcmp(argv[arg]+2,"version")) {
				ShowVersion();
				exit(0);
			} else if (!strcmp(argv[arg]+2,"help"))
				GiveHelp();
			else if (!strcmp(argv[arg]+2,"fast"))
				FastStart = 1;
			else if (!strcmp(argv[arg]+2,"execute"))
				executelist[Nexecutes++] = (Tchar*)argv[++arg];
			else if (!strcmp(argv[arg]+2,"noreadline"))
				UseReadlineLibrary = 0;
			else if (!strcmp(argv[arg]+2,"profile"))
				flags::profiling = 1;
			else
				cerr << "%%% Unknown option " << argv[arg]
				     << '\n';
		} else {
			for (int i=1; argv[arg][i]; i++) {
				Tchar ch = argv[arg][i];
				if (ch == 'b')
					flags::batch = 1;
				else if (ch == 'e') {
					executelist[Nexecutes++] = (Tchar*)argv[++arg];
					break;
				} else if (ch == 'v') {
					flags::verbose = 1;
					flags::silent = 0;
				} else if (ch == 's') {
					flags::silent = 1;
					flags::verbose = 0;
				} else if (ch == 'p') {
					flags::profiling = 1;
				} else if (ch == 'V') {
					ShowVersion();
					exit(0);
				} else if (ch == 'h' || ch == '?')
					GiveHelp();
				else if (ch == 'f')
					FastStart = 1;
				else if (ch == 'n')
					UseReadlineLibrary = 0;
				else
					cerr << "%%% Unknown option -" << ch
					     << '\n';
			}
		}
	}
}

// Strip whitespace from the start and end of a string.
void stripwhite (Tchar *s)
{
	int i = 0;
	while (whitespace (s[i])) i++;
	if (i) strcpy (s, s + i);
	i = strlen (s) - 1;
	while (i > 0 && whitespace (s[i])) i--;
  	s[++i] = '\0';
}

static void HelpMacros(Tchar **lineptr)
// Replaces ?anything         with        help("anything")
//          help anything     with        help("anything")
//          !anything         with        system("anything")
// Warns about quit, exit, help
{
	Tchar *line = *lineptr;
	int i; const int L = strlen(line);
	if (line[0] == '?') {
		Tchar *line2 = new Tchar [L+8];
		for (i=1; i<L; i++) line2[i+5] = line[i];
		line2[L+5] = '"';
		line2[L+6] = ')';
		line2[L+7] = '\0';
		line2[0] = 'h';
		line2[1] = 'e';
		line2[2] = 'l';
		line2[3] = 'p';
		line2[4] = '(';
		line2[5] = '"';
		*lineptr = line2;
	} else if (L >= 5 && line[0] == 'h') {
		if (!strncmp((char*)line,"help ",5)) {
			Tchar *line2 = new Tchar [L+4];
			for (i=5; i<L; i++) line2[i+1] = line[i];
			line2[L+1] = '"';
			line2[L+2] = ')';
			line2[L+3] = '\0';
			line2[0] = 'h';
			line2[1] = 'e';
			line2[2] = 'l';
			line2[3] = 'p';
			line2[4] = '(';
			line2[5] = '"';
			*lineptr = line2;
		}
	} else if (line[0] == '!') {
		Tchar *line2 = new Tchar [L+10];
		for (i=1; i<L; i++) line2[i+7] = line[i];
		line2[L+7] = '"';
		line2[L+8] = ')';
		line2[L+9] = '\0';
		line2[0] = 's';
		line2[1] = 'y';
		line2[2] = 's';
		line2[3] = 't';
		line2[4] = 'e';
		line2[5] = 'm';
		line2[6] = '(';
		line2[7] = '"';
		*lineptr = line2;
	} else if (!strcmp((char*)line,"quit") || !strcmp((char*)line,"exit")) {
		cout << "- If you want to quit, type quit() or Control-D\n"
		     << flush;
	} else if (!strcmp((char*)line,"help")) {
		cout << "- Type ?help for first help\n" << flush;
	}
}

static RETSIGTYPE SoftInterruptHandler(int signum) {
	if (!global::debugging) {
		global::interrupted = 1;
		if (flags::debugquery) {
			char s[80];
			cout << "Break without continue possibility [y] ? "
			     << flush;
			fgets(s,77,stdin);
			if (*s == 'n' || *s == 'N') global::debugging = 1;
		}
	}
}

static RETSIGTYPE ControlGHandler(int signum) {
	if (!global::debugging) {
		global::interrupted = 1;
		global::debugging = 1;
	}
}

static RETSIGTYPE HardInterruptHandler(int signum) {
	if (!global::debugging) {
		err << "User break.\n";
		signal(SIGINT,SIG_IGN);
		global::interrupted = 0;
		error();
	}
}

void SetHardInterruptHandler() {

	signal(SIGINT,HardInterruptHandler);
}

void SetSoftInterruptHandler() {
	signal(SIGINT,SoftInterruptHandler);
}

void SetControlGInterruptHandler() {
	signal(SIGUSR1,ControlGHandler);
}

void ExecuteLine(Tchar *line, int SetJump=0, int printsetflag=1)
// On topmost level, call with SetJump=1.
// Call with printsetflag=0 if you don't want to print expression values
// (this is used by evalexpr in std.ct).
{
	sig_handler oldsig = 0;
	if (!flags::batch) oldsig = signal(SIGINT,SIG_IGN);
	sig_handler old_usr1 = signal(SIGUSR1,SIG_IGN);
	if (SetJump) HelpMacros(&line);	// apply helper macros only if called from toplevel
#	if USE_STRINGSTREAM
	stringstream str;
#	else
	strstream str;
#	endif
//	str << (char*)line << ";\n" << ends;
	str << (char*)line << "}\n" << ends;
	Tlexer lexer(str);
	lexer.filename = (const Tchar*)"stdin";
	lexer.oldlineno = global::lineno;
	global::lineno = 1;
	lexer.wasSyntaxError = 0;
	Tprg prg((const Tchar*)"stdin");
	TNodePoolState S = theNodePool.mark();
	if (lexer.yyparse()==0 && !lexer.wasSyntaxError) {	// parsed succesfully
		if (flags::verbose) cout << *lexer.nodeptr << '\n';
		if (SetJump) {
			int val;
			SetJmp(val,jmp1);
			if (!val) {
				GenerateCode(lexer.nodeptr,prg,printsetflag);
				if (flags::verbose) cout << prg << '\n';
				if (!flags::batch)
					signal(SIGINT,SoftInterruptHandler);
				signal(SIGUSR1,ControlGHandler);
				prg.execute(0,0,0,0);
				if (global::interrupted) {
					global::interrupted = 0;
					cerr << "User break.\n";
				}
				if (!flags::batch) signal(SIGINT,SIG_IGN);
				signal(SIGUSR1,SIG_IGN);
			} else {
				// Back to top level
				//cerr << "Back to top level\n";
				ResetStacks();
			}
		} else {	// SetJump==0, don't have to set jump or signals
			GenerateCode(lexer.nodeptr,prg,printsetflag);
			if (flags::verbose) cout << prg << '\n';
			if (!flags::batch) signal(SIGINT,SoftInterruptHandler);
			signal(SIGUSR1,ControlGHandler);
			prg.execute(0,0,0,0);
		}
	}
	theNodePool.release(S);
	global::lineno = lexer.oldlineno;
	if (!flags::batch) signal(SIGINT,oldsig);
	signal(SIGUSR1,old_usr1);
}

//static void Compile(Tprg& prg) {Ctgen(cout,prg);}

static void SetTelaPath1(Tchar*& path, const char *envvar)
// Replace or append to path from environment variable envvar, if set
{
	Tchar *path1 = (Tchar*)getenv(envvar);
	if (path1) {
		if (*path1 == ':') {
			Tchar *newpath = new Tchar [strlen(path)+strlen(path1)+2];
			strcpy(newpath,path);
			strcat(newpath,path1);
			path = newpath;
		} else
			path = path1;
	}
}

static void InitTelaPath()
{
	Tchar *path = (Tchar*) DEFAULT_TELAPATH;
	SetTelaPath1(path, "TELAPATH_SYSTEM");
	SetTelaPath1(path, "TELAPATH");
	size_t p = 0;
	size_t j = 0;
	Tchar buff[MAXFILENAME];
	size_t i = 0;
	while (p < MAX_TELAPATH_LENGTH  &&  path[i] != '\0')
	{
		if (path[i] == ':')
		{
			buff[j] = '\0';
			TelaPath[p] = strdup(buff);
			p++;
			j = 0;
		}
		else
			buff[j++] = path[i];
		i++;
	}
	if (p == MAX_TELAPATH_LENGTH  &&  path[i] != '\0')
	{
		TelaPathLength = p;
		cerr << "*** Tela search path too long!\n"
		     << "*** Dropping all path elements after element number "
		     << MAX_TELAPATH_LENGTH << ".\n";
	}
	else
	{
		buff[j] = '\0';
		TelaPath[p] = strdup(buff);
		TelaPathLength = p + 1;
	}
}

FILE *FindAndOpenFile(const Tchar* fn)
// Tries to find "fn" along TELAPATH.
// If found, returns pointer to file opened for reading,
// null pointer if not found.
// If name starts with '/',
// or start with '.' and second char is either '.' or '/',
// then the name is absolute.
// If name starts with '!', it is assumed to be a pipe.
{
	Tchar totalfn[MAXFILENAME];
	if (fn[0]=='!') {
#		if HAVE_POPEN
		return popen((const char*)(fn+1),"r");
#		else
		cerr << "Warning: pipes do not work in this system.\n";
		return 0;
#		endif
	}
	if (fn[0]=='/' || fn[0]=='.' && (fn[1]=='.' || fn[1]=='/'))
		return fopen((const char*)fn,"r");
	for (size_t p=0; p<TelaPathLength; p++) {
		strcpy(totalfn,TelaPath[p]);
		strcat(totalfn,(const Tchar*)"/");
		strcat(totalfn,fn);
		FILE *fp = fopen((const char*)totalfn,"r");
		if (fp) return fp;
	}
	return 0;
}

int CompleteFileName(const Tchar *fn, Tchar totalfn[MAXFILENAME])
// Tries to find "fn" along TELAPATH. If found, returns the total pathname.
// If fn starts with '/' or ".." or "./", just checks it is readable and
// copies it to totalfn.
// Return code: 0 if not found, 1 if found.
{
	if (fn[0]=='/' || fn[0]=='.' && (fn[1]=='.' || fn[1]=='/')) {
		FILE *fp = fopen((const char*)fn,"r");
		if (fp) {
			fclose(fp);
			strcpy(totalfn,fn);
			return 1;
		} else {
			totalfn[0] = '\0';
			return 0;
		}
	}
	for (size_t p=0; p<TelaPathLength; p++) {
		strcpy(totalfn,TelaPath[p]);
		strcat(totalfn,(const Tchar*)"/");
		strcat(totalfn,fn);
		FILE *fp = fopen((const char*)totalfn,"r");
		if (fp) {fclose(fp); return 1;}
	}
	totalfn[0] = '\0';
	return 0;
}

#if 0
static void CheckUserID() {
	static int allowed[] = {405,5351,10118,/*PJ*/ 5172/*AV*/,5311/*JT*/,5221/*JS*/,-1};
	int uid = getuid();
	int found=0;
	for (int i=0; allowed[i]>=0; i++) if (uid==allowed[i]) found=1;
	if (!found) {
		cerr << rand() << flush;
		float a = 0.6;
		for (int i=0; i<500; i++)
		    for (int j=0; j<1000; j++)
			a = cos(a);
		cerr <<" error():: Not enough memory\n";
		exit(rand());
	}
}
#endif

extern void Ctgen(ostream& o, Tprg& prg);

int ExecuteFile(const Tchar fn[], int silent)
{
	sig_handler oldsig = 0;
	if (!flags::batch) oldsig = signal(SIGINT,SIG_IGN);
	FILE *fp = FindAndOpenFile(fn);
	const Tchar *UsedFn = fn;
	if (!fp) {
		Tchar *fn2 = new Tchar [strlen(fn)+3];
		strcpy(fn2,fn);
		strcat(fn2,(const Tchar*)".t");
		fp = FindAndOpenFile(fn2);
		UsedFn = fn2;
	}
	if (!fp) {
		if (!silent) cerr << "*** Could not open \""
				  << (const char*)fn << "\"\n";
		if (!flags::batch) signal(SIGINT,oldsig);
		return 1;
	}

//#ifdef HAVE_ISTREAM /* use STL */
//	boost::fdistream in(fileno(fp));
//#else  /* use extensions in old gcc 2.95 */

#	ifdef __GNUC__
#	if __GNUC__ >= 3
#	if __GNUC_MINOR__ < 4 && __GNUC__ == 3
	__gnu_cxx::stdio_filebuf<char> b(fileno(fp),std::ios_base::in,false,4096);
#	else
	__gnu_cxx::stdio_filebuf<char> b(fp,std::ios_base::in,4096);
#	endif
	istream in(&b);
#	else
	ifstream in(fileno(fp));
#	endif
#	else
	ifstream in(fileno(fp));
#	endif

//#endif /* HAVE_IOSTREAM */

	Tlexer lexer(in);
	lexer.oldlineno = global::lineno;
	global::lineno = 1;
	lexer.filename = UsedFn;
	for (;!lexer.AtEOF();) {	// Loop over objects in file
		Tprg prg(UsedFn);
		lexer.wasSyntaxError = 0;
		TNodePoolState S = theNodePool.mark();
		lexer.yyparse();
		if (lexer.wasSyntaxError) {
			fclose(fp);
			theNodePool.release(S);
			if (!flags::batch) signal(SIGINT,oldsig);
			return 1;
		}
		if (flags::verbose) cout << *lexer.nodeptr << '\n';
		GenerateCode(lexer.nodeptr,prg);
		theNodePool.release(S);
		if (flags::verbose) cout << "\nPROGRAM:" << prg << '\n';
		if (global::ctfile) Ctgen(*global::ctfile,prg);
		TConstObjectPtr argin[1]; TObjectPtr argout[1];
		if (!flags::batch) signal(SIGINT,SoftInterruptHandler);
		prg.execute(argin,0,argout,0);
		if (global::interrupted) {
			global::interrupted = 0;
			cerr << "User break.\n";
		}
		if (!flags::batch) signal(SIGINT,SIG_IGN);
	}
	global::lineno = lexer.oldlineno;
	fclose(fp);
	if (!flags::batch) signal(SIGINT,oldsig);
	return 0;
}

static Tchar *CommandGenerator(Tchar *text, int state)
// Generator function for command completion.  STATE lets us know whether
// to start from scratch; without any state (i.e. STATE == 0), then we
// start at the top of the list.
{
	static int len;
	// If this is a new word to complete, initialize now.  This includes
	// saving the length of TEXT for efficiency, and calling theHT.first().
	Tsymbol *symptr;
	if (!text) return 0;
	if (!state) {
		symptr = theHT.first();
		len = strlen(text);
    } else
		symptr = theHT.next();
	if (!symptr) return 0;
	// Return the next name which partially matches from the command list.
	do {	// Loop over possibly all symbols in hash table
		Tchar *str = (Tchar*)(*symptr);
		// Exclude internal package symbols (names containing ':') from search
		if (!strchr((char*)str,':') && strncmp((char*)str,(char*)text,len) == 0) {
			TObjectPtr objptr = symptr->value();
			// We don't know what stub symbols are going to be, but they are
			// more often functions than variables, hence append '(' for them also.
			if (symptr->StubInfo() ||
				objptr && (objptr->IsFunction() || objptr->IsCfunction() || objptr->IsIntrinsicFunction()))
			{
				int L = strlen(str);
				Tchar *s = (Tchar*)malloc(sizeof(char)*(L+2));
				strcpy(s,str);
				s[L] = '(';
				s[L+1] = '\0';
				return s;
			} else
				return strdup(str);
		}
		symptr = theHT.next();
	} while (symptr);
	// If no names matched, then return 0
	return 0;
}

static Tchar **TelaCompletion (Tchar *text, int start, int end)
// Attempt to complete on the contents of TEXT.  START and END show the
// region of TEXT that contains the word to complete.  We can use the
// entire line in case we want to do some simple parsing.  Return the
// array of matches, or NULL if there aren't any.
{
	// If the name is preceded by double quote, it is presumably filename
	// ==> let Readline try filename completion
	// Otherwise call completion_matches with CommandGenerator.
	if (start > 0 && (rl_line_buffer[start-1] == '"' || rl_line_buffer[0] == '!')) {
		rl_basic_word_break_characters = " !\t\n\"\\'`@$<>=";
		return 0;
	} else {
		rl_basic_word_break_characters = " ()[]{}+-*/^#\"'&|=<>!$.,:;\t\n\\";
		return completion_matches(text,CommandGenerator);
	}
}

static void InitializeReadline() {
	rl_readline_name = (Tchar*)"tela";
	// Tell the completer that we want to try completion first.
	rl_attempted_completion_function = (Function *)TelaCompletion;
}

static Tchar *noreadline()
// Returns NULL if EOF is encountered immediately. If first char is
// newline, returns a string with zero length.
{
#	if USE_STRINGSTREAM
	stringstream str;
#	else
	strstream str;
#	endif
	char ch;
	int cnt = 0;
	if (feof(stdin)) return 0;
	do {
		ch = getchar();
		if (ch == '\n' || feof(stdin)) break;
		str << ch;
		cnt++;
	} while (1);
	str << ends;
#	if USE_STRINGSTREAM
	string charptr = str.str();
	Tchar *result = (Tchar *)malloc((cnt+1)*sizeof(char));
	memcpy((char *)result,charptr.c_str(),cnt);
#	else
	char *charptr = str.str();
	Tchar *result = (Tchar *)malloc((cnt+1)*sizeof(char));
	memcpy((char *)result,charptr,cnt);
#	endif
	result[cnt] = '\0';
#	if !USE_STRINGSTREAM
	delete [] charptr;
#	endif
	return result;
}

void InterpreterLoop(int ContWillExit=0)
{
	Tchar *line;
	for (;;) {
		clog << flush; cout << flush;
		if (UseReadlineLibrary)
			line = readline(">");
		else
			line = noreadline();
		if (!line) break;	// readline encountered eof
		stripwhite(line);
		if (*line) {
			if (UseReadlineLibrary) add_history(line);
			if (ContWillExit) {
				if (!strcmp((char*)line,"cont")) {
					free(line);
					SetSoftInterruptHandler();
					global::debugging = 0;
					global::interrupted = 0;
					break;
				}
			}
			if (ContWillExit) {
				int val;
				SetJmp(val,jmp2);
				if (!val)
					ExecuteLine(line);
				else
					cerr << "Returning to debug level\n";
			} else
				ExecuteLine(line,1);
		}
		if (line) free(line);
	}
}

#ifdef __GNUC__
extern "C" int exitfunction(const TConstObjectPtr[], const int,
			    const TObjectPtr[], const int);	// std.ct
#endif

int main(int argc, char *argv[])
{
	global::argv0 = argv[0];
	//CheckUserID();
	ConsistencyChecks();
#if !HAVE_RUSAGE && HAVE_SYS_PROCFS_H && HAVE_UNISTD_H
    char name[30];
    sprintf(name,"/proc/%d",getpid());
    global::procfd = open(name,O_RDONLY);
#endif
	InitializeReadline();
	InitTelaPath();
	int arg=1;
	ParseArgs(argc,argv,arg);
	InstallIntrinsic();
	InstallStatic();
	signal(SIGFPE,SIG_IGN);	// Ignore floating point exceptions
	InstallGlobalSymbols();
	if (!flags::batch) signal(SIGINT,SIG_IGN);
	if (!flags::batch && !flags::silent) {
		cout << "This tela is a tensor language, Version "
		     << VersionString << ".\n";
		cout << "Type  ?help  for help.\n" << flush;
	}
	if (!FastStart) ExecuteFile((const Tchar*)"telainit.t",0);
	int val;
	SetJmp(val,jmp1);
	if (!val) {
		for (int i=0; i<Nexecutes; i++) {	// Loop over execute strings
			ExecuteLine(executelist[i]);
		}
		for (; arg<argc; arg++) {		// Loop over argument filenames
			// Form a string source("filename") and add it to history.
			// This is called service!!!!!!
			char *line = new char [strlen(argv[arg]) + 12];
			strcpy(line,"source(\"");
			strcat(line,argv[arg]);
			strcat(line,"\")");
			add_history(line);
			delete [] line;
			ExecuteFile((const Tchar*)argv[arg],0);
		}
	} else {
		cerr << "*** Error in argument file.\n";
		// return 1;	-- Commented this out. Why should we exit if there's error in arg file?
	}
	if (!flags::batch) InterpreterLoop();
#	ifndef __GNUC__
	if (!flags::silent) PerformanceReport();
#	endif
	cout << flush; clog << flush;
#	ifdef __GNUC__
	{
		TConstObjectPtr inputs[1];
		TObjectPtr outputs[1];
		exitfunction(inputs,0,outputs,0);	// Call exit() function defined in std.ct, added 25 Aug 1998
		// notice that for Linux, exit is defined to be _exit in def.H, but return 0; from main would call global destructors.
	}
#	endif
	return 0;
}
