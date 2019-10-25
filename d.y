%name Tparser

%define LEX_BODY {return 0;}		/* otherwise flex++ doesn't seem to work */
%define MEMBERS \
	Tnode*nodeptr;	 		/* we return the parsed tree via Tparser::nodeptr */ \
	const Tchar*filename;	/* the input file name for error messages */ \
	int oldlineno;			/* the old line number */ \
	int wasSyntaxError;

%{

#include <stdio.h>
#include "lisp.H"
#include "machine.H"

static Tnode* NList(Tfunc h, Tnode*arg1);
static Tnode* NList(Tfunc h, Tnode*arg1,Tnode*arg2);
static Tnode* NList(Tfunc h, Tnode*arg1,Tnode*arg2, Tnode*arg3);
static Tnode* NList(Tfunc h, Tnode*arg1,Tnode*arg2, Tnode*arg3, Tnode*arg4);
static Tnode* NList(Tfunc h, Tnode*arg1,Tnode*arg2, Tnode*arg3, Tnode*arg4, Tnode*arg5);
static Tnode* FlatOperator(Tnode*left, Tnode*right, Tfunc func);

static TObjectPtr tmp(TObjectPtr);	// pass through TObjectPtr, flagging it temporary
/* EXPLANATION:
   All parsed objects are linked to the parse tree under the Temporary attribute
   (relevant methods: object->IsTemporary(), object->FlagTemporary(), object->UnflagTemporary()).
   The parse tree is deallocated by the mark/release approach. The node pools are scanned
   through and all Tobjects pointed to by the to-be-deleted Tnodes are deleted, if they
   carry the Temporary attribute. In code generation phase equal objects of the parse tree
   are stored only once. */

static Tnode* DotProduct(Tnode*U, Tnode*V);

%}

%start topobj

%union {
	Tnode	*node_val;
	Treal	real_val;
	Tint	int_val;
	Tsymbol *sym_val;
	Tchar    *str_val;
	Tchar	ch_val;
}

%token <int_val> INTEGER
%token <real_val> REAL iREAL
%token <sym_val> IDENTIFIER
%token <str_val> STRING
%token <ch_val> CHAR
%token MOD DOT INC DEC
%token INCASSIGN DECASSIGN MULASSIGN DIVASSIGN
%token IF ELSE WHILE FOR FOREACH CONTINUE BREAK RETURN REPEAT UNTIL GOTO LABEL
%token FUNCTION PACKAGE LOCAL GLOBAL CALL
%token DISP DOUBLELEFTBRACKET DOUBLERIGHTBRACKET
%token LEFTCONSTRUCTOR ELLIPSIS HERMITIAN_TRANSPOSE TRANSPOSE
%token ERROR_TOKEN

%type <node_val> expr semicolon_list comma_list nonempty_comma_list xpr
%type <node_val> lvalue function_call
%type <node_val> arglist nonempty_arglist symbol_arglist nonempty_symbol_arglist
%type <node_val> stmt if_stmt while_stmt for_stmt foreach_stmt continue_stmt
%type <node_val> break_stmt return_stmt goto_stmt label_stmt
%type <node_val> disp_stmt incr_stmt decr_stmt repeat_stmt set_stmt
%type <node_val> stmt_ne stmtseq formal_arglist formal_simple_arglist
%type <node_val> fdef pkgdef localdecl topobj

%nonassoc ':'		/* vector creation operator */
%left OR            /* || */
%left AND           /* && */
%left EQ NE         /* == != */
%left GT GE LT LE   /* > >= < <= */
%left '+' '-'
%left '*' DOT '/' MOD
%nonassoc UNARY_MINUS UNARY_PLUS
%right '^'
%nonassoc NOT
%nonassoc HERMITIAN_TRANSPOSE TRANSPOSE

%%      /* Beginning of rules section */

topobj	: stmtseq '}' { nodeptr = $$ = $1; YYACCEPT; }	/* statement sequence */
		| { YYERROR; }
		;

stmtseq	: stmt { $$ = new Tnode($1,FBLOCK,0); }
		| stmtseq ';' stmt { $$ = FlatOperator($1,$3,FBLOCK); }
		;

stmt	: /* empty */ { $$ = new Tnode(0,FNOP,0); }
		| stmt_ne /* non-empty */ { $$ = $1; }
		;

stmt_ne	: set_stmt
		| '[' symbol_arglist ']' '=' function_call { $$ = NList(FSET,$2,$5); }
		| fdef
		| pkgdef
		| if_stmt
		| while_stmt
		| repeat_stmt
		| for_stmt
		| foreach_stmt
		| break_stmt
		| continue_stmt
		| return_stmt
		| goto_stmt
		| label_stmt
		| incr_stmt
		| decr_stmt
		| disp_stmt
		| '{' stmtseq '}' { $$ = $2; }
		| error ';' { $$ = 0; }
		;

set_stmt	: expr { $$ = new Tnode($1,FSET,0); }
			| lvalue '=' expr { $1->next = $3; $$ = new Tnode($1,FSET,0); }
			| lvalue INCASSIGN expr { Tnode*n1 = new Tnode(*$1); $$ = NList(FSET,n1,NList(FPLUS,$1,$3)); }
			| lvalue DECASSIGN expr { Tnode*n1 = new Tnode(*$1); $$ = NList(FSET,n1,NList(FDIFFERENCE,$1,$3)); }
			| lvalue MULASSIGN expr { Tnode*n1 = new Tnode(*$1); $$ = NList(FSET,n1,NList(FTIMES,$1,$3)); }
			| lvalue DIVASSIGN expr { Tnode*n1 = new Tnode(*$1); $$ = NList(FSET,n1,NList(FQUOTIENT,$1,$3)); }
			;

fdef	: FUNCTION IDENTIFIER '(' formal_arglist ')' localdecl '{' stmtseq '}' {
			nodeptr = $$ = NList(FDEFUN,new Tnode(0,$2,0),new Tnode(),$4,$6,$8);
		}
		| FUNCTION IDENTIFIER '=' IDENTIFIER '(' formal_arglist ')' localdecl '{' stmtseq '}' {
			nodeptr = $$ = NList(FDEFUN,new Tnode(0,$4,0),new Tnode(0,$2,0),$6,$8,$10);
		}
		| FUNCTION '[' formal_arglist ']' '=' IDENTIFIER '(' formal_arglist ')' localdecl '{' stmtseq '}' {
			nodeptr = $$ = NList(FDEFUN,new Tnode(0,$6,0),$3,$8,$10,$12);
		}
		;

pkgdef	: PACKAGE STRING localdecl '{' stmtseq '}' {
			$$ = NList(FPACKAGE,new Tnode(0,tmp(new Tobject($2)),0),$3,$5);
		}
		| PACKAGE localdecl '{' stmtseq '}' {
			$$ = NList(FPACKAGE,new Tnode(0,tmp(new Tobject(filename)),0),$2,$4);
		}
		;

formal_simple_arglist	: /* empty */ { $$ = new Tnode(0,FLIST,0); }
						| IDENTIFIER { $$ = NList(FLIST,new Tnode(0,$1,0)); }
						| formal_simple_arglist ',' IDENTIFIER { $$ = FlatOperator($1,new Tnode(0,$3,0),FLIST); }
						;

formal_arglist	: formal_simple_arglist
				| formal_simple_arglist ';' formal_simple_arglist { $$ = NList(FFORMAL,$1,$3); }
				| formal_simple_arglist ELLIPSIS { $$ = NList(FFORMAL_ELLIPSIS,$1); }
				| formal_simple_arglist ';' formal_simple_arglist ELLIPSIS { $$ = NList(FFORMAL_ELLIPSIS,$1,$3); }
				;

localdecl	: /* empty */ { $$ = new Tnode(); *$$ = FLOCAL; }
			| LOCAL { $$ = new Tnode(0,FLOCAL,0); }
			| LOCAL '(' formal_simple_arglist ')' { $$ = $3; *$$ = FLOCAL; }
			| GLOBAL { $$ = new Tnode(0,FGLOBAL,0); }
			| GLOBAL '(' formal_simple_arglist ')' { $$ = $3; *$$ = FGLOBAL; }
			;

if_stmt	: IF '(' expr ')' stmt { $$ = NList(FIF,$3,$5); }
		| IF '(' expr ')' stmt ELSE stmt { $$ = NList(FIF,$3,$5,$7); }

while_stmt	: WHILE '(' expr ')' stmt { $$ = NList(FWHILE,$3,$5); }
			;

repeat_stmt	: REPEAT stmtseq UNTIL expr { $$ = NList(FREPEAT,$2,$4); }
			;

for_stmt	: FOR '(' stmt ';' expr ';' stmt ')' stmt { $$ = NList(FFOR,$3,$5,$7,$9); }
			;

foreach_stmt: FOREACH '(' IDENTIFIER '=' expr ')' stmt {
	/* Notice that NList destroys the bindings of its arguments
       (that's why it's NList and not List). Thus we have to recompute
       ii and other Tnodes after every use. */
				Tsymbol *iiptr = UniqueSymbol((const Tchar*)"iiii");
				Tsymbol *vvptr = UniqueSymbol((const Tchar*)"vvvv");
				Tnode *ii = new Tnode(0,iiptr,0);
				Tnode *i = new Tnode(0,$3,0);
				Tnode *arraybase = new Tnode(0,tmp(new Tobject(ArrayBase)),0);
				Tnode *setii = NList(FSET,ii, arraybase);
				ii = new Tnode(0,iiptr,0);
				Tnode *seti = NList(FSET, i, NList(FREF,new Tnode(0,vvptr,0),ii));
				Tnode *stmt = NList(FBLOCK,seti,$7);
				Tnode *length = new Tnode(0,theHT.add((const Tchar*)"length"),0);
				ii = new Tnode(0,iiptr,0);
				Tnode *contcond;
				if (ArrayBase==1) {
					contcond = NList(FLE,ii,NList(FCALL,length,new Tnode(0,vvptr,0)));
				} else {
					Tnode *arraybaseMinus1 = new Tnode(0,tmp(new Tobject(ArrayBase-1)),0);
					contcond = NList(FLT,ii,NList(FPLUS,NList(FCALL,length,new Tnode(0,vvptr,0)),arraybaseMinus1));
				}
				Tnode *unity = new Tnode(0,tmp(new Tobject(1)),0);
				ii = new Tnode(0,iiptr,0);
				Tnode *iiplus1 = NList(FPLUS,ii,unity);
				ii = new Tnode(0,iiptr,0);
				Tnode *iiplusplus = NList(FSET,ii,iiplus1);
				Tnode *forstmt = NList(FFOR,setii,contcond,iiplusplus,stmt);
				Tnode *initstmt = NList(FSET,new Tnode(0,vvptr,0),new Tnode(*$5));
				Tnode *resetstmt1 = NList(FSET,new Tnode(0,iiptr,0),new Tnode(0,Tallrange(),0));
				Tnode *resetstmt2 = NList(FSET,new Tnode(0,vvptr,0),new Tnode(0,Tallrange(),0));
				$$ = NList(FBLOCK,initstmt,forstmt,resetstmt1,resetstmt2);
			}
			;

break_stmt	: BREAK { $$ = new Tnode(0,FBREAK,0); }
			;

continue_stmt	: CONTINUE { $$ = new Tnode(0,FCONTINUE,0); }
				;

return_stmt		: RETURN  { $$ = new Tnode(0,FRETURN,0); }
				;

goto_stmt	: GOTO IDENTIFIER { $$ = NList(FGOTO,new Tnode(0,$2,0)); }
			;

label_stmt	: LABEL IDENTIFIER { $$ = NList(FLABEL,new Tnode(0,$2,0)); }
			;

incr_stmt	: lvalue INC {
				Tnode*n1 = new Tnode(*$1);
				$$ = NList(FSET,n1,NList(FPLUS,$1,new Tnode(0,tmp(new Tobject(1)),0)));
			  }
			;

decr_stmt	: lvalue DEC {
				Tnode*n1 = new Tnode(*$1);
				$$ = NList(FSET,n1,NList(FDIFFERENCE,$1,new Tnode(0,tmp(new Tobject(1)),0)));
			  }
			;

disp_stmt	: DISP expr { $$ = new Tnode($2,FDISP,0); }
			;

expr	: xpr
		| ':' { $$ = new Tnode(0,Tallrange(),0); }
		| xpr ':' xpr { $1->next = $3; $$ = new Tnode($1,FRANGE,0); }
		| xpr ':' xpr ':' xpr { $1->next = $3; $3->next = $5; $$ = new Tnode($1,FRANGE,0); }
		;

xpr		: '(' expr ')' { $$ = $2; }
		| ERROR_TOKEN { YYERROR; }
		| LEFTCONSTRUCTOR semicolon_list ')' { $$ = $2; }
		| xpr HERMITIAN_TRANSPOSE { $$ = NList(FCALL,new Tnode(0,theHT.add((const Tchar*)"herm"),0),$1); }
		| xpr TRANSPOSE { $$ = NList(FCALL,new Tnode(0,theHT.add((const Tchar*)"transpose"),0),$1); }
		| xpr EQ  xpr { $1->next = $3; $$ = new Tnode($1,FEQ,0); }
		| xpr NE  xpr { $1->next = $3; $$ = new Tnode($1,FNE,0); }
		| xpr GT  xpr { $1->next = $3; $$ = new Tnode($1,FGT,0); }
		| xpr LT  xpr { $1->next = $3; $$ = new Tnode($1,FLT,0); }
		| xpr GE  xpr { $1->next = $3; $$ = new Tnode($1,FGE,0); }
		| xpr LE  xpr { $1->next = $3; $$ = new Tnode($1,FLE,0); }
		| xpr AND xpr { $1->next = $3; $$ = new Tnode($1,FAND,0); }
		| xpr OR  xpr { $1->next = $3; $$ = new Tnode($1,F_OR,0); }
		| xpr '+' xpr { $1->next = $3; $$ = new Tnode($1,FPLUS,0); }
		| xpr '-' xpr { $1->next = $3; $$ = new Tnode($1,FDIFFERENCE,0); }
		| xpr '*' xpr { $1->next = $3; $$ = new Tnode($1,FTIMES,0); }
		| xpr DOT xpr { $$ = DotProduct($1,$3); }
		| xpr '/' xpr { $1->next = $3; $$ = new Tnode($1,FQUOTIENT,0); }
		| xpr MOD xpr { $1->next = $3; $$ = new Tnode($1,FMOD,0); }
		| xpr '^' xpr { $1->next = $3; $$ = new Tnode($1,FPOWER,0); }
		| '-' xpr %prec UNARY_MINUS { $$ = new Tnode($2,FMINUS,0); }
		| '+' xpr %prec UNARY_PLUS { $$ = $2; }
		| NOT xpr { $$ = new Tnode($2,FNOT,0); }
		| function_call
		| INTEGER { $$ = new Tnode(0,tmp(new Tobject($1)),0); }
		| REAL  { $$ = new Tnode(0,tmp(new Tobject($1)),0); }
		| iREAL { $$ = new Tnode(0,tmp(new Tobject(Tcomplex(0,$1))),0); }
		| STRING { $$ = new Tnode(0,tmp(new Tobject($1)),0); }
		| CHAR {
			TObjectPtr objptr = new Tobject(Tint($1));
			objptr->SetCharFlag();
			$$ = new Tnode(0,tmp(objptr),0);
		}
		| lvalue
		| error ';' { $$ = 0; }
		;

semicolon_list	: /* empty */ {
					$$ = NList(FCALL,
                               new Tnode(0,theHT.add((const Tchar*)"izeros"),0),
                               new Tnode(0,tmp(new Tobject(0)),0))
                         /*new Tnode(0,FEMPTY_ARRAY,0)*/;
                }
				| expr { $$ = new Tnode($1,FARRAY,0); }
				| nonempty_comma_list
				| semicolon_list ';' comma_list  { $$ = FlatOperator($1,$3,FARRAY); }
				;

comma_list	: expr
			| comma_list ',' expr { $$ = FlatOperator($1,$3,FAPP); }
			;

nonempty_comma_list	: comma_list ',' expr { $$ = FlatOperator($1,$3,FAPP); }
					;

lvalue	: IDENTIFIER { $$ = new Tnode(0,$1,0); }
		| lvalue /*IDENTIFIER*/ '[' nonempty_arglist ']' {
			$$ = Append(NList(FREF,$1/*new Tnode(0,$1,0)*/),$3);
			if (Length($3) > MAXRANK) {
				cerr << "*** Too many (" << Length($3) << ") indices in [..]. Max is " << MAXRANK << ".\n";
				yyerror("");
			}
		}
		| lvalue /*IDENTIFIER*/ DOUBLELEFTBRACKET nonempty_arglist DOUBLERIGHTBRACKET {
			$$ = Append(NList(FMREF,$1/*new Tnode(0,$1,0)*/),$3);
			if (Length($3) > MAXRANK) {
				cerr << "*** Too many (" << Length($3) << ") indices in [[..]]. Max is " << MAXRANK << ".\n";
				yyerror("");
			}
		}
		;

function_call	: IDENTIFIER '(' arglist ')' { $$ = Append(NList(FCALL,new Tnode(0,$1,0)),$3); }
				| CALL '(' arglist ')' { *$3 = FCALL; $$ = $3; }
				;

nonempty_arglist	: expr { $$ = new Tnode($1,FLIST,0); }
					| nonempty_arglist ',' expr { $$ = FlatOperator($1,$3,FLIST); }
					;

arglist	: /* empty */ { $$ = new Tnode(0,FLIST,0); }
		| nonempty_arglist
		;

nonempty_symbol_arglist	: IDENTIFIER { $$ = new Tnode(new Tnode(0,$1,0),FLIST,0); }
						| nonempty_symbol_arglist ',' IDENTIFIER { $$ = FlatOperator($1,new Tnode(0,$3,0),FLIST); }
						;

symbol_arglist	: /* empty */ { $$ = new Tnode(0,FLIST,0); }
				| nonempty_symbol_arglist
				;

%%	/* auxiliary functions section */

static Tnode* NList(Tfunc h, Tnode*arg1) {
	arg1->next = 0;
	return new Tnode(arg1,h,0);
}

static Tnode* NList(Tfunc h, Tnode*arg1, Tnode*arg2) {
	arg1->next = arg2;
	arg2->next = 0;
	return new Tnode(arg1,h,0);
}

static Tnode* NList(Tfunc h, Tnode*arg1, Tnode*arg2, Tnode*arg3) {
	arg1->next = arg2;
	arg2->next = arg3;
	arg3->next = 0;
	return new Tnode(arg1,h,0);
}

static Tnode* NList(Tfunc h, Tnode*arg1, Tnode*arg2, Tnode*arg3, Tnode*arg4) {
	arg1->next = arg2;
	arg2->next = arg3;
	arg3->next = arg4;
	arg4->next = 0;
	return new Tnode(arg1,h,0);
}

static Tnode* NList(Tfunc h, Tnode*arg1, Tnode*arg2, Tnode*arg3, Tnode*arg4, Tnode*arg5) {
	arg1->next = arg2;
	arg2->next = arg3;
	arg3->next = arg4;
	arg4->next = arg5;
	arg5->next = 0;
	return new Tnode(arg1,h,0);
}

static Tnode* FlatOperator(Tnode*left, Tnode*right, Tfunc func) {
	if (left->kind==Kbuiltin && left->FunctionValue()==func) {
        Tnode *p;
		for (p=left->list; p->next; p=p->next);
		if (right) right->next = 0;
		p->next = right;
		return left;
	} else {
		left->next = right;
		return new Tnode(left,func,0);
	}
}

static TObjectPtr tmp(TObjectPtr p) {	// pass through TObjectPtr, flagging it temporary
	p->FlagTemporary();
	return p;
}

static Tnode* DotProduct(Tnode*U, Tnode*V) {
	// DotProduct(CALL[f,x],y) -> matprod(x,y,'f')
	// DotProduct(x,CALL[f,y]) -> matprod(x,y,'n','f')
	// DotProduct(CALL[f,x],CALL[g,y]) -> matprod(x,y,'f','g')
	// Here f,g is either transpose or herm, and 'f','g' corresponds f,g
	// such that transpose->'t', herm->'h'.
	static int FirstTime = 1;
	static Tsymbol *matprod_symbol, *herm, *transpose;
	if (FirstTime) {
		matprod_symbol = theHT.add((const Tchar*)"matprod");
		herm = theHT.add((const Tchar*)"herm");
		transpose = theHT.add((const Tchar*)"transpose");
		FirstTime = 0;
	}
	Tnode* matprod = new Tnode(0, matprod_symbol, 0);
	Tnode *U1=U, *V1=V;
	char Uflag='n', Vflag='n';
	if (U->kind==Kbuiltin && Head(U)==FCALL && First(U) && Second(U) && !Third(U)
		&& First(U)->kind==Kvariable &&
		( &(First(U)->SymbolValue())==transpose || &(First(U)->SymbolValue())==herm ) ) {
			U1 = Second(U);
			Uflag = (&(First(U)->SymbolValue())==transpose) ? 't' : 'h';
	}
	if (V->kind==Kbuiltin && Head(V)==FCALL && First(V) && Second(V) && !Third(V)
		&& First(V)->kind==Kvariable &&
		( &(First(V)->SymbolValue())==transpose || &(First(V)->SymbolValue())==herm ) ) {
			V1 = Second(V);
			Vflag = (&(First(V)->SymbolValue())==transpose) ? 't' : 'h';
	}
	Tnode *result;
	if (Vflag=='n') {
		if (Uflag=='n')
			result = NList(FCALL,matprod,U1,V1);
		else
			result = NList(FCALL,matprod,U1,V1,
						   new Tnode(0,tmp(new Tobject(Uflag)),0));
	} else
		result = NList(FCALL,matprod,U1,V1,
					   new Tnode(0,tmp(new Tobject(Uflag)),0),
					   new Tnode(0,tmp(new Tobject(Vflag)),0));
	return result;
}
