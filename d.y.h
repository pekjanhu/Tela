#ifndef YY_Tparser_h_included
#define YY_Tparser_h_included

#line 1 "./pars/bison.h"
/* before anything */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#endif
#include <stdio.h>

/* #line 14 "./pars/bison.h" */
#define YY_Tparser_LEX_BODY  {return 0;}		/* otherwise flex++ doesn't seem to work */
#define YY_Tparser_MEMBERS  \
	Tnode*nodeptr;	 		/* we return the parsed tree via Tparser::nodeptr */ \
	const Tchar*filename;	/* the input file name for error messages */ \
	int oldlineno;			/* the old line number */ \
	int wasSyntaxError;

#line 38 "./d.y"
typedef union {
	Tnode	*node_val;
	Treal	real_val;
	Tint	int_val;
	Tsymbol *sym_val;
	Tchar    *str_val;
	Tchar	ch_val;
} yy_Tparser_stype;
#define YY_Tparser_STYPE yy_Tparser_stype

#line 14 "./pars/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_Tparser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_Tparser_COMPATIBILITY 1
#else
#define  YY_Tparser_COMPATIBILITY 0
#endif
#endif

#if YY_Tparser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_Tparser_LTYPE
#define YY_Tparser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_Tparser_STYPE 
#define YY_Tparser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_Tparser_DEBUG
#define  YY_Tparser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_Tparser_STYPE
#ifndef yystype
#define yystype YY_Tparser_STYPE
#endif
#endif
#endif

#ifndef YY_Tparser_PURE

/* #line 54 "./pars/bison.h" */

#line 54 "./pars/bison.h"
/* YY_Tparser_PURE */
#endif

/* #line 56 "./pars/bison.h" */

#line 56 "./pars/bison.h"
/* prefix */
#ifndef YY_Tparser_DEBUG

/* #line 58 "./pars/bison.h" */

#line 58 "./pars/bison.h"
/* YY_Tparser_DEBUG */
#endif
#ifndef YY_Tparser_LSP_NEEDED

/* #line 61 "./pars/bison.h" */

#line 61 "./pars/bison.h"
 /* YY_Tparser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_Tparser_LSP_NEEDED
#ifndef YY_Tparser_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_Tparser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_Tparser_STYPE
#define YY_Tparser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_Tparser_PARSE
#define YY_Tparser_PARSE yyparse
#endif
#ifndef YY_Tparser_LEX
#define YY_Tparser_LEX yylex
#endif
#ifndef YY_Tparser_LVAL
#define YY_Tparser_LVAL yylval
#endif
#ifndef YY_Tparser_LLOC
#define YY_Tparser_LLOC yylloc
#endif
#ifndef YY_Tparser_CHAR
#define YY_Tparser_CHAR yychar
#endif
#ifndef YY_Tparser_NERRS
#define YY_Tparser_NERRS yynerrs
#endif
#ifndef YY_Tparser_DEBUG_FLAG
#define YY_Tparser_DEBUG_FLAG yydebug
#endif
#ifndef YY_Tparser_ERROR
#define YY_Tparser_ERROR yyerror
#endif

#ifndef YY_Tparser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_Tparser_PARSE_PARAM
#ifndef YY_Tparser_PARSE_PARAM_DEF
#define YY_Tparser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_Tparser_PARSE_PARAM
#define YY_Tparser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_Tparser_PURE
extern YY_Tparser_STYPE YY_Tparser_LVAL;
#endif


/* #line 134 "./pars/bison.h" */
#define	INTEGER	258
#define	REAL	259
#define	iREAL	260
#define	IDENTIFIER	261
#define	STRING	262
#define	CHAR	263
#define	MOD	264
#define	DOT	265
#define	INC	266
#define	DEC	267
#define	INCASSIGN	268
#define	DECASSIGN	269
#define	MULASSIGN	270
#define	DIVASSIGN	271
#define	IF	272
#define	ELSE	273
#define	WHILE	274
#define	FOR	275
#define	FOREACH	276
#define	CONTINUE	277
#define	BREAK	278
#define	RETURN	279
#define	REPEAT	280
#define	UNTIL	281
#define	GOTO	282
#define	LABEL	283
#define	FUNCTION	284
#define	PACKAGE	285
#define	LOCAL	286
#define	GLOBAL	287
#define	CALL	288
#define	DISP	289
#define	DOUBLELEFTBRACKET	290
#define	DOUBLERIGHTBRACKET	291
#define	LEFTCONSTRUCTOR	292
#define	ELLIPSIS	293
#define	HERMITIAN_TRANSPOSE	294
#define	TRANSPOSE	295
#define	ERROR_TOKEN	296
#define	OR	297
#define	AND	298
#define	EQ	299
#define	NE	300
#define	GT	301
#define	GE	302
#define	LT	303
#define	LE	304
#define	UNARY_MINUS	305
#define	UNARY_PLUS	306
#define	NOT	307


#line 134 "./pars/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_Tparser_CLASS
#define YY_Tparser_CLASS Tparser
#endif

#ifndef YY_Tparser_INHERIT
#define YY_Tparser_INHERIT
#endif
#ifndef YY_Tparser_MEMBERS
#define YY_Tparser_MEMBERS 
#endif
#ifndef YY_Tparser_LEX_BODY
#define YY_Tparser_LEX_BODY  
#endif
#ifndef YY_Tparser_ERROR_BODY
#define YY_Tparser_ERROR_BODY  
#endif
#ifndef YY_Tparser_CONSTRUCTOR_PARAM
#define YY_Tparser_CONSTRUCTOR_PARAM
#endif

class YY_Tparser_CLASS YY_Tparser_INHERIT
{
public: /* static const int token ... */

/* #line 160 "./pars/bison.h" */
static const int INTEGER;
static const int REAL;
static const int iREAL;
static const int IDENTIFIER;
static const int STRING;
static const int CHAR;
static const int MOD;
static const int DOT;
static const int INC;
static const int DEC;
static const int INCASSIGN;
static const int DECASSIGN;
static const int MULASSIGN;
static const int DIVASSIGN;
static const int IF;
static const int ELSE;
static const int WHILE;
static const int FOR;
static const int FOREACH;
static const int CONTINUE;
static const int BREAK;
static const int RETURN;
static const int REPEAT;
static const int UNTIL;
static const int GOTO;
static const int LABEL;
static const int FUNCTION;
static const int PACKAGE;
static const int LOCAL;
static const int GLOBAL;
static const int CALL;
static const int DISP;
static const int DOUBLELEFTBRACKET;
static const int DOUBLERIGHTBRACKET;
static const int LEFTCONSTRUCTOR;
static const int ELLIPSIS;
static const int HERMITIAN_TRANSPOSE;
static const int TRANSPOSE;
static const int ERROR_TOKEN;
static const int OR;
static const int AND;
static const int EQ;
static const int NE;
static const int GT;
static const int GE;
static const int LT;
static const int LE;
static const int UNARY_MINUS;
static const int UNARY_PLUS;
static const int NOT;


#line 160 "./pars/bison.h"
 /* decl const */
public:
 int YY_Tparser_PARSE(YY_Tparser_PARSE_PARAM);
 virtual void YY_Tparser_ERROR(char *) YY_Tparser_ERROR_BODY;
#ifdef YY_Tparser_PURE
#ifdef YY_Tparser_LSP_NEEDED
 virtual int  YY_Tparser_LEX(YY_Tparser_STYPE *YY_Tparser_LVAL,YY_Tparser_LTYPE *YY_Tparser_LLOC) YY_Tparser_LEX_BODY;
#else
 virtual int  YY_Tparser_LEX(YY_Tparser_STYPE *YY_Tparser_LVAL) YY_Tparser_LEX_BODY;
#endif
#else
 virtual int YY_Tparser_LEX() YY_Tparser_LEX_BODY;
 YY_Tparser_STYPE YY_Tparser_LVAL;
#ifdef YY_Tparser_LSP_NEEDED
 YY_Tparser_LTYPE YY_Tparser_LLOC;
#endif
 int YY_Tparser_NERRS;
 int YY_Tparser_CHAR;
#endif
#if YY_Tparser_DEBUG != 0
public:
 int YY_Tparser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_Tparser_CLASS(YY_Tparser_CONSTRUCTOR_PARAM);
public:
 YY_Tparser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_Tparser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_Tparser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_Tparser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_Tparser_DEBUG 
#define YYDEBUG YY_Tparser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 209 "./pars/bison.h" */
#endif
