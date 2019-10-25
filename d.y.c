#define YY_Tparser_h_included

/*  A Bison parser, made from ./d.y  */


#line 1 "./pars/bison.cc"
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#ifndef _MSDOS
#ifdef MSDOS
#define _MSDOS
#endif
#endif
/* turboc */
#ifdef __MSDOS__
#ifndef _MSDOS
#define _MSDOS
#endif
#endif

/* alloca stuff - modified by PJ, borrowed from prg.C */
#ifdef __GNUC__
#  ifndef alloca
#    define alloca __builtin_alloca
#  endif
#  ifndef HAVE_ALLOCA
#    define HAVE_ALLOCA
#  endif
#elif HAVE_ALLOCA_H
#  include <alloca.h>
#  ifndef HAVE_ALLOCA
#    define HAVE_ALLOCA
#  endif
#elif C_ALLOCA
  extern "C" void *alloca(unsigned int);
#elif defined(CONVEX)
#  include <stdlib.h>
   extern "C" char *alloca(int);
#endif


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
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  

/* #line 69 "./pars/bison.cc" */
#define YY_Tparser_LEX_BODY  {return 0;}		/* otherwise flex++ doesn't seem to work */
#define YY_Tparser_MEMBERS  \
	Tnode*nodeptr;	 		/* we return the parsed tree via Tparser::nodeptr */ \
	const Tchar*filename;	/* the input file name for error messages */ \
	int oldlineno;			/* the old line number */ \
	int wasSyntaxError;
#line 10 "./d.y"


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

#line 69 "./pars/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_Tparser_BISON 1
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
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_Tparser_STYPE 
#define YY_Tparser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_Tparser_DEBUG
#define  YY_Tparser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_Tparser_STYPE
#ifndef yystype
#define yystype YY_Tparser_STYPE
#endif
#endif
#endif

#ifndef YY_Tparser_PURE

/* #line 104 "./pars/bison.cc" */

#line 104 "./pars/bison.cc"
/*  YY_Tparser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 108 "./pars/bison.cc" */

#line 108 "./pars/bison.cc"
/* prefix */
#ifndef YY_Tparser_DEBUG

/* #line 110 "./pars/bison.cc" */

#line 110 "./pars/bison.cc"
/* YY_Tparser_DEBUG */
#endif


#ifndef YY_Tparser_LSP_NEEDED

/* #line 115 "./pars/bison.cc" */

#line 115 "./pars/bison.cc"
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
      /* We used to use `unsigned long' as YY_Tparser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

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
#if YY_Tparser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_Tparser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_Tparser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_Tparser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_Tparser_PURE
#ifndef YYPURE
#define YYPURE YY_Tparser_PURE
#endif
#endif
#ifdef YY_Tparser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_Tparser_DEBUG 
#endif
#endif
#ifndef YY_Tparser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_Tparser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_Tparser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_Tparser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif


/* #line 222 "./pars/bison.cc" */
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


#line 222 "./pars/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
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
#ifndef YY_Tparser_CONSTRUCTOR_CODE
#define YY_Tparser_CONSTRUCTOR_CODE
#endif
#ifndef YY_Tparser_CONSTRUCTOR_INIT
#define YY_Tparser_CONSTRUCTOR_INIT
#endif

class YY_Tparser_CLASS YY_Tparser_INHERIT
{
public: /* static const int token ... */

/* #line 253 "./pars/bison.cc" */
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


#line 253 "./pars/bison.cc"
 /* decl const */
public:
 int YY_Tparser_PARSE (YY_Tparser_PARSE_PARAM);
 virtual void YY_Tparser_ERROR(char *msg) YY_Tparser_ERROR_BODY;
#ifdef YY_Tparser_PURE
#ifdef YY_Tparser_LSP_NEEDED
 virtual int  YY_Tparser_LEX (YY_Tparser_STYPE *YY_Tparser_LVAL,YY_Tparser_LTYPE *YY_Tparser_LLOC) YY_Tparser_LEX_BODY;
#else
 virtual int  YY_Tparser_LEX (YY_Tparser_STYPE *YY_Tparser_LVAL) YY_Tparser_LEX_BODY;
#endif
#else
 virtual int YY_Tparser_LEX() YY_Tparser_LEX_BODY;
 YY_Tparser_STYPE YY_Tparser_LVAL;
#ifdef YY_Tparser_LSP_NEEDED
 YY_Tparser_LTYPE YY_Tparser_LLOC;
#endif
 int   YY_Tparser_NERRS;
 int    YY_Tparser_CHAR;
#endif
#if YY_Tparser_DEBUG != 0
 int YY_Tparser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_Tparser_CLASS(YY_Tparser_CONSTRUCTOR_PARAM);
public:
 YY_Tparser_MEMBERS 
};
/* other declare folow */

/* #line 281 "./pars/bison.cc" */
const int YY_Tparser_CLASS::INTEGER=258;
const int YY_Tparser_CLASS::REAL=259;
const int YY_Tparser_CLASS::iREAL=260;
const int YY_Tparser_CLASS::IDENTIFIER=261;
const int YY_Tparser_CLASS::STRING=262;
const int YY_Tparser_CLASS::CHAR=263;
const int YY_Tparser_CLASS::MOD=264;
const int YY_Tparser_CLASS::DOT=265;
const int YY_Tparser_CLASS::INC=266;
const int YY_Tparser_CLASS::DEC=267;
const int YY_Tparser_CLASS::INCASSIGN=268;
const int YY_Tparser_CLASS::DECASSIGN=269;
const int YY_Tparser_CLASS::MULASSIGN=270;
const int YY_Tparser_CLASS::DIVASSIGN=271;
const int YY_Tparser_CLASS::IF=272;
const int YY_Tparser_CLASS::ELSE=273;
const int YY_Tparser_CLASS::WHILE=274;
const int YY_Tparser_CLASS::FOR=275;
const int YY_Tparser_CLASS::FOREACH=276;
const int YY_Tparser_CLASS::CONTINUE=277;
const int YY_Tparser_CLASS::BREAK=278;
const int YY_Tparser_CLASS::RETURN=279;
const int YY_Tparser_CLASS::REPEAT=280;
const int YY_Tparser_CLASS::UNTIL=281;
const int YY_Tparser_CLASS::GOTO=282;
const int YY_Tparser_CLASS::LABEL=283;
const int YY_Tparser_CLASS::FUNCTION=284;
const int YY_Tparser_CLASS::PACKAGE=285;
const int YY_Tparser_CLASS::LOCAL=286;
const int YY_Tparser_CLASS::GLOBAL=287;
const int YY_Tparser_CLASS::CALL=288;
const int YY_Tparser_CLASS::DISP=289;
const int YY_Tparser_CLASS::DOUBLELEFTBRACKET=290;
const int YY_Tparser_CLASS::DOUBLERIGHTBRACKET=291;
const int YY_Tparser_CLASS::LEFTCONSTRUCTOR=292;
const int YY_Tparser_CLASS::ELLIPSIS=293;
const int YY_Tparser_CLASS::HERMITIAN_TRANSPOSE=294;
const int YY_Tparser_CLASS::TRANSPOSE=295;
const int YY_Tparser_CLASS::ERROR_TOKEN=296;
const int YY_Tparser_CLASS::OR=297;
const int YY_Tparser_CLASS::AND=298;
const int YY_Tparser_CLASS::EQ=299;
const int YY_Tparser_CLASS::NE=300;
const int YY_Tparser_CLASS::GT=301;
const int YY_Tparser_CLASS::GE=302;
const int YY_Tparser_CLASS::LT=303;
const int YY_Tparser_CLASS::LE=304;
const int YY_Tparser_CLASS::UNARY_MINUS=305;
const int YY_Tparser_CLASS::UNARY_PLUS=306;
const int YY_Tparser_CLASS::NOT=307;


#line 281 "./pars/bison.cc"
 /* const YY_Tparser_CLASS::token */
/*apres const  */
YY_Tparser_CLASS::YY_Tparser_CLASS(YY_Tparser_CONSTRUCTOR_PARAM) YY_Tparser_CONSTRUCTOR_INIT
{
#if YY_Tparser_DEBUG != 0
YY_Tparser_DEBUG_FLAG=0;
#endif
YY_Tparser_CONSTRUCTOR_CODE;
};
#endif

/* #line 291 "./pars/bison.cc" */


#define	YYFINAL		239
#define	YYFLAG		-32768
#define	YYNTBASE	68

#define YYTRANSLATE(x) ((unsigned)(x) <= 307 ? yytranslate[x] : 102)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    65,
    66,    53,    51,    67,    52,     2,    54,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    42,    60,     2,
    63,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    61,     2,    62,    57,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    64,     2,    59,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    43,    44,    45,    46,
    47,    48,    49,    50,    55,    56,    58
};

#if YY_Tparser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     4,     6,    10,    11,    13,    15,    21,    23,
    25,    27,    29,    31,    33,    35,    37,    39,    41,    43,
    45,    47,    49,    51,    55,    58,    60,    64,    68,    72,
    76,    80,    90,   102,   116,   123,   129,   130,   132,   136,
   138,   142,   145,   150,   151,   153,   158,   160,   165,   171,
   179,   185,   190,   200,   208,   210,   212,   214,   217,   220,
   223,   226,   229,   231,   233,   237,   243,   247,   249,   253,
   256,   259,   263,   267,   271,   275,   279,   283,   287,   291,
   295,   299,   303,   307,   311,   315,   319,   322,   325,   328,
   330,   332,   334,   336,   338,   340,   342,   345,   346,   348,
   350,   354,   356,   360,   364,   366,   371,   376,   381,   386,
   388,   392,   393,   395,   397,   401,   402
};

#endif

static const short yyrhs[] = {    69,
    59,     0,     0,    70,     0,    69,    60,    70,     0,     0,
    71,     0,    72,     0,    61,   101,    62,    63,    97,     0,
    73,     0,    74,     0,    78,     0,    79,     0,    80,     0,
    81,     0,    82,     0,    83,     0,    84,     0,    85,     0,
    86,     0,    87,     0,    88,     0,    89,     0,    90,     0,
    64,    69,    59,     0,     1,    60,     0,    91,     0,    96,
    63,    91,     0,    96,    13,    91,     0,    96,    14,    91,
     0,    96,    15,    91,     0,    96,    16,    91,     0,    29,
     6,    65,    76,    66,    77,    64,    69,    59,     0,    29,
     6,    63,     6,    65,    76,    66,    77,    64,    69,    59,
     0,    29,    61,    76,    62,    63,     6,    65,    76,    66,
    77,    64,    69,    59,     0,    30,     7,    77,    64,    69,
    59,     0,    30,    77,    64,    69,    59,     0,     0,     6,
     0,    75,    67,     6,     0,    75,     0,    75,    60,    75,
     0,    75,    38,     0,    75,    60,    75,    38,     0,     0,
    31,     0,    31,    65,    75,    66,     0,    32,     0,    32,
    65,    75,    66,     0,    17,    65,    91,    66,    70,     0,
    17,    65,    91,    66,    70,    18,    70,     0,    19,    65,
    91,    66,    70,     0,    25,    69,    26,    91,     0,    20,
    65,    70,    60,    91,    60,    70,    66,    70,     0,    21,
    65,     6,    63,    91,    66,    70,     0,    23,     0,    22,
     0,    24,     0,    27,     6,     0,    28,     6,     0,    96,
    11,     0,    96,    12,     0,    34,    91,     0,    92,     0,
    42,     0,    92,    42,    92,     0,    92,    42,    92,    42,
    92,     0,    65,    91,    66,     0,    41,     0,    37,    93,
    66,     0,    92,    39,     0,    92,    40,     0,    92,    45,
    92,     0,    92,    46,    92,     0,    92,    47,    92,     0,
    92,    49,    92,     0,    92,    48,    92,     0,    92,    50,
    92,     0,    92,    44,    92,     0,    92,    43,    92,     0,
    92,    51,    92,     0,    92,    52,    92,     0,    92,    53,
    92,     0,    92,    10,    92,     0,    92,    54,    92,     0,
    92,     9,    92,     0,    92,    57,    92,     0,    52,    92,
     0,    51,    92,     0,    58,    92,     0,    97,     0,     3,
     0,     4,     0,     5,     0,     7,     0,     8,     0,    96,
     0,     1,    60,     0,     0,    91,     0,    95,     0,    93,
    60,    94,     0,    91,     0,    94,    67,    91,     0,    94,
    67,    91,     0,     6,     0,    96,    61,    98,    62,     0,
    96,    35,    98,    36,     0,     6,    65,    99,    66,     0,
    33,    65,    99,    66,     0,    91,     0,    98,    67,    91,
     0,     0,    98,     0,     6,     0,   100,    67,     6,     0,
     0,   100,     0
};

#if YY_Tparser_DEBUG != 0
static const short yyrline[] = { 0,
    83,    84,    87,    88,    91,    92,    95,    96,    97,    98,
    99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
   109,   110,   111,   112,   113,   116,   117,   118,   119,   120,
   121,   124,   127,   130,   135,   138,   143,   144,   145,   148,
   149,   150,   151,   154,   155,   156,   157,   158,   161,   162,
   164,   167,   170,   173,   208,   211,   214,   217,   220,   223,
   229,   235,   238,   239,   240,   241,   244,   245,   246,   247,
   248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
   258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
   268,   269,   270,   271,   272,   277,   278,   281,   287,   288,
   289,   292,   293,   296,   299,   300,   307,   316,   317,   320,
   321,   324,   325,   328,   329,   332,   333
};

static const char * const yytname[] = {   "$","error","$illegal.","INTEGER",
"REAL","iREAL","IDENTIFIER","STRING","CHAR","MOD","DOT","INC","DEC","INCASSIGN",
"DECASSIGN","MULASSIGN","DIVASSIGN","IF","ELSE","WHILE","FOR","FOREACH","CONTINUE",
"BREAK","RETURN","REPEAT","UNTIL","GOTO","LABEL","FUNCTION","PACKAGE","LOCAL",
"GLOBAL","CALL","DISP","DOUBLELEFTBRACKET","DOUBLERIGHTBRACKET","LEFTCONSTRUCTOR",
"ELLIPSIS","HERMITIAN_TRANSPOSE","TRANSPOSE","ERROR_TOKEN","':'","OR","AND",
"EQ","NE","GT","GE","LT","LE","'+'","'-'","'*'","'/'","UNARY_MINUS","UNARY_PLUS",
"'^'","NOT","'}'","';'","'['","']'","'='","'{'","'('","')'","','","topobj","stmtseq",
"stmt","stmt_ne","set_stmt","fdef","pkgdef","formal_simple_arglist","formal_arglist",
"localdecl","if_stmt","while_stmt","repeat_stmt","for_stmt","foreach_stmt","break_stmt",
"continue_stmt","return_stmt","goto_stmt","label_stmt","incr_stmt","decr_stmt",
"disp_stmt","expr","xpr","semicolon_list","comma_list","nonempty_comma_list",
"lvalue","function_call","nonempty_arglist","arglist","nonempty_symbol_arglist",
"symbol_arglist",""
};
#endif

static const short yyr1[] = {     0,
    68,    68,    69,    69,    70,    70,    71,    71,    71,    71,
    71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
    71,    71,    71,    71,    71,    72,    72,    72,    72,    72,
    72,    73,    73,    73,    74,    74,    75,    75,    75,    76,
    76,    76,    76,    77,    77,    77,    77,    77,    78,    78,
    79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
    89,    90,    91,    91,    91,    91,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
    92,    92,    92,    92,    92,    92,    92,    93,    93,    93,
    93,    94,    94,    95,    96,    96,    96,    97,    97,    98,
    98,    99,    99,   100,   100,   101,   101
};

static const short yyr2[] = {     0,
     2,     0,     1,     3,     0,     1,     1,     5,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     3,     2,     1,     3,     3,     3,     3,
     3,     9,    11,    13,     6,     5,     0,     1,     3,     1,
     3,     2,     4,     0,     1,     4,     1,     4,     5,     7,
     5,     4,     9,     7,     1,     1,     1,     2,     2,     2,
     2,     2,     1,     1,     3,     5,     3,     1,     3,     2,
     2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     2,     2,     2,     1,
     1,     1,     1,     1,     1,     1,     2,     0,     1,     1,
     3,     1,     3,     3,     1,     4,     4,     4,     4,     1,
     3,     0,     1,     1,     3,     0,     1
};

static const short yydefact[] = {     0,
     0,    91,    92,    93,   105,    94,    95,     0,     0,     0,
     0,    56,    55,    57,     0,     0,     0,     0,    44,     0,
     0,     0,    68,    64,     0,     0,     0,   116,     0,     0,
     0,     3,     6,     7,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,    22,    23,    26,
    63,    96,    90,    97,     0,     0,     0,     0,     0,     0,
    58,    59,     0,    37,    44,    45,    47,     0,     0,     0,
    62,    96,    99,     0,     0,   100,    88,    87,    89,   114,
   117,     0,     0,     0,     1,     0,     0,     0,    70,    71,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    60,    61,     0,     0,     0,     0,
     0,     0,     0,   110,   113,     0,     0,     0,     0,     0,
     0,     0,    37,    38,    40,     0,     0,    37,    37,     0,
     0,    97,     0,    69,     0,     0,     0,    24,    67,     4,
    85,    83,    65,    79,    78,    72,    73,    74,    76,    75,
    77,    80,    81,    82,    84,    86,    28,    29,    30,    31,
     0,     0,    27,     0,   108,     0,     0,     0,     0,    52,
     0,     0,    42,    37,     0,     0,     0,     0,     0,     0,
   109,   102,   101,   104,   115,     0,     0,   107,   106,   111,
    49,    51,     0,     0,    37,    44,    41,    39,     0,     0,
    46,    48,    36,     0,     0,     8,    66,     0,     0,     0,
     0,     0,    43,     0,    35,   103,    50,     0,    54,    44,
     0,    37,     0,     0,     0,     0,    53,     0,    32,    44,
     0,     0,    33,     0,     0,    34,     0,     0,     0
};

static const short yydefgoto[] = {   237,
    31,    32,    33,    34,    35,    36,   125,   126,    68,    37,
    38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    74,    75,    76,    72,    53,   115,
   116,    81,    82
};

static const short yypact[] = {   299,
   -44,-32768,-32768,-32768,   -37,-32768,-32768,   -34,    -4,     3,
     6,-32768,-32768,-32768,   527,    39,    68,     1,    41,    17,
   699,    18,-32768,-32768,   722,   722,   722,    79,   592,   699,
     5,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   751,    92,-32768,   -13,   406,   699,   699,   657,    94,   -18,
-32768,-32768,    14,   103,    49,    47,    51,    62,   406,    85,
-32768,    -5,    80,   -22,    87,-32768,    -7,    -7,    55,-32768,
    93,    60,    58,    86,-32768,   462,   722,   722,-32768,-32768,
   722,   722,   722,   722,   722,   722,   722,   722,   722,   722,
   722,   722,   722,   722,-32768,-32768,   699,   699,   699,   699,
   699,   699,   699,-32768,   112,   114,   115,   116,   118,    95,
   699,   177,   103,-32768,   -26,   125,   124,   103,   103,   592,
   137,-32768,   699,-32768,   699,   198,   142,-32768,-32768,-32768,
    -7,    -7,   767,   829,   845,    89,    89,   878,   878,   878,
   878,    57,    57,    -7,    -7,    -7,-32768,-32768,-32768,-32768,
   -27,   -10,-32768,   699,-32768,   232,   232,   699,   699,-32768,
   141,   145,-32768,   103,   201,   150,   592,    65,    67,    91,
-32768,-32768,   147,   148,-32768,    21,   722,-32768,-32768,-32768,
   199,-32768,   156,   152,   103,    49,   -24,-32768,   214,    97,
-32768,-32768,-32768,   699,   -37,-32768,   813,   232,   364,   232,
   158,   162,-32768,   157,-32768,-32768,-32768,   161,-32768,    49,
   592,   103,   232,   165,   105,   164,-32768,   592,-32768,    49,
   111,   167,-32768,   592,   113,-32768,   241,   242,-32768
};

static const short yypgoto[] = {-32768,
    -9,   -47,-32768,-32768,-32768,-32768,  -111,  -120,   -61,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   -20,    98,-32768,    99,-32768,     0,    59,    63,
   174,-32768,-32768
};


#define	YYLAST		935


static const short yytable[] = {    52,
    71,    73,   172,   127,   -25,    60,    63,   121,   188,    84,
   119,   173,   -25,   213,    52,    54,   178,   179,    70,    83,
     2,     3,     4,     5,     6,     7,   205,    55,    52,   111,
    56,    89,    90,   174,   114,   117,   118,   133,   140,   164,
   175,    86,   175,   134,    61,   -25,   -25,    65,   114,   104,
    20,   189,   -25,    20,    22,   112,   164,    52,    23,    24,
    57,    64,   197,    85,    86,    87,    88,    58,    25,    26,
    59,    66,    67,    62,   211,    27,   122,   -98,   123,    66,
    67,    69,    30,   -98,    80,    52,   157,   158,   159,   160,
   114,   114,   163,    89,    90,    89,    90,    87,    88,   120,
   170,   226,   105,   106,   107,   108,   109,   110,   124,   102,
   103,   128,   182,   104,   184,   129,   138,    86,   191,   192,
   180,   137,    77,    78,    79,   130,   111,    89,    90,    52,
   201,   175,   202,   175,   212,    96,    97,    98,    99,   100,
   101,   102,   103,   190,   132,   104,  -102,   193,   194,   203,
    86,   139,   112,   135,   113,   215,    86,   169,   224,   136,
   217,   218,   219,   229,    86,    52,    52,   200,   232,   233,
    86,   236,    86,   161,   162,   227,    52,   168,   164,   165,
   166,   167,   171,   216,   141,   142,   176,   177,   143,   144,
   145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
   155,   156,   181,   185,   186,   195,   198,    52,    52,    52,
   196,   225,   199,   204,  -103,   209,   208,   210,   231,   214,
    52,   222,    52,   220,   235,   221,   223,    52,   228,   230,
   234,   183,     1,    52,     2,     3,     4,     5,     6,     7,
   238,   239,   131,     0,   206,     0,     0,     0,     8,    -5,
     9,    10,    11,    12,    13,    14,    15,    -5,    16,    17,
    18,    19,     0,     0,    20,    21,     0,     0,    22,     0,
     0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
     0,     0,    25,    26,   207,     0,     0,     0,     0,    27,
    -5,    -5,    28,     0,     0,    29,    30,    -5,    -2,     1,
     0,     2,     3,     4,     5,     6,     7,     0,     0,     0,
     0,     0,     0,     0,     0,     8,     0,     9,    10,    11,
    12,    13,    14,    15,     0,    16,    17,    18,    19,     0,
     0,    20,    21,     0,     0,    22,     0,     0,     0,    23,
    24,     0,     0,     0,     0,     0,     0,     0,     0,    25,
    26,     0,     0,     0,     0,     0,    27,    -5,    -5,    28,
     0,     0,    29,    30,     1,     0,     2,     3,     4,     5,
     6,     7,     0,     0,     0,     0,     0,     0,     0,     0,
     8,     0,     9,    10,    11,    12,    13,    14,    15,     0,
    16,    17,    18,    19,     0,     0,    20,    21,     0,     0,
    22,     0,     0,     0,    23,    24,    70,     0,     2,     3,
     4,     5,     6,     7,    25,    26,     0,     0,     0,     0,
     0,    27,     0,     0,    28,     0,     0,    29,    30,    -5,
     0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
     0,     0,    22,     0,     0,     0,    23,    24,     0,     0,
     0,     0,     0,     0,     0,     0,    25,    26,     0,     0,
     0,     0,     1,    27,     2,     3,     4,     5,     6,     7,
    30,  -112,     0,     0,     0,     0,     0,     0,     8,     0,
     9,    10,    11,    12,    13,    14,    15,    -5,    16,    17,
    18,    19,     0,     0,    20,    21,     0,     0,    22,     0,
     0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
     0,     0,    25,    26,     0,     0,     0,     0,     0,    27,
    -5,    -5,    28,     0,     0,    29,    30,     1,     0,     2,
     3,     4,     5,     6,     7,     0,     0,     0,     0,     0,
     0,     0,     0,     8,     0,     9,    10,    11,    12,    13,
    14,    15,    -5,    16,    17,    18,    19,     0,     0,    20,
    21,     0,     0,    22,     0,     0,     0,    23,    24,     0,
     0,     0,     0,     0,     0,     0,     0,    25,    26,     0,
     0,     0,     0,     0,    27,     0,    -5,    28,     0,     0,
    29,    30,     1,     0,     2,     3,     4,     5,     6,     7,
     0,     0,     0,     0,     0,     0,     0,     0,     8,     0,
     9,    10,    11,    12,    13,    14,    15,     0,    16,    17,
    18,    19,     0,     0,    20,    21,     0,     0,    22,     0,
     0,     0,    23,    24,     0,     0,     0,     0,     0,     0,
     0,     0,    25,    26,     0,     0,     0,     0,     0,    27,
    -5,    -5,    28,     0,     0,    29,    30,     1,     0,     2,
     3,     4,     5,     6,     7,     0,     0,     0,     0,     0,
     0,     0,     0,     8,     0,     9,    10,    11,    12,    13,
    14,    15,     0,    16,    17,    18,    19,     0,     0,    20,
    21,     0,     0,    22,     0,     0,     0,    23,    24,    70,
     0,     2,     3,     4,     5,     6,     7,    25,    26,     0,
     0,     0,     0,     0,    27,     0,    -5,    28,     0,     0,
    29,    30,    70,     0,     2,     3,     4,     5,     6,     7,
     0,    20,     0,     0,     0,    22,     0,     0,     0,    23,
    24,     0,     0,     0,     0,     0,     0,     0,     0,    25,
    26,     0,     0,     0,    20,     0,    27,     0,    22,    87,
    88,     0,    23,    30,     0,     0,     0,     0,     0,     0,
     0,     0,    25,    26,     0,    87,    88,     0,     0,    27,
     0,     0,     0,     0,     0,     0,    30,     0,     0,    89,
    90,     0,    91,    92,    93,    94,    95,    96,    97,    98,
    99,   100,   101,   102,   103,    89,    90,   104,   187,    92,
    93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
   103,    87,    88,   104,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    87,    88,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    89,    90,    87,    88,    92,    93,    94,    95,    96,
    97,    98,    99,   100,   101,   102,   103,    89,    90,   104,
     0,     0,    93,    94,    95,    96,    97,    98,    99,   100,
   101,   102,   103,    89,    90,   104,    87,    88,     0,    94,
    95,    96,    97,    98,    99,   100,   101,   102,   103,     0,
     0,   104,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,    89,    90,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   100,   101,
   102,   103,     0,     0,   104
};

static const short yycheck[] = {     0,
    21,    22,   123,    65,    18,    15,     6,    26,    36,    30,
    58,    38,    26,    38,    15,    60,   128,   129,     1,    29,
     3,     4,     5,     6,     7,     8,     6,    65,    29,    35,
    65,    39,    40,    60,    55,    56,    57,    60,    86,    67,
    67,    60,    67,    66,     6,    59,    60,     7,    69,    57,
    33,    62,    66,    33,    37,    61,    67,    58,    41,    42,
    65,    61,   174,    59,    60,     9,    10,    65,    51,    52,
    65,    31,    32,     6,   195,    58,    63,    60,    65,    31,
    32,    65,    65,    66,     6,    86,   107,   108,   109,   110,
   111,   112,   113,    39,    40,    39,    40,     9,    10,     6,
   121,   222,    11,    12,    13,    14,    15,    16,     6,    53,
    54,    65,   133,    57,   135,    65,    59,    60,   166,   167,
   130,    62,    25,    26,    27,    64,    35,    39,    40,   130,
    66,    67,    66,    67,   196,    47,    48,    49,    50,    51,
    52,    53,    54,   164,    60,    57,    67,   168,   169,    59,
    60,    66,    61,    67,    63,    59,    60,    63,   220,    67,
   208,   209,   210,    59,    60,   166,   167,   177,   230,    59,
    60,    59,    60,   111,   112,   223,   177,    60,    67,    66,
    66,    66,     6,   204,    87,    88,    62,    64,    91,    92,
    93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
   103,   104,    66,     6,    63,    65,     6,   208,   209,   210,
    66,   221,    63,    67,    67,    60,    18,    66,   228,     6,
   221,    65,   223,    66,   234,    64,    66,   228,    64,    66,
    64,   133,     1,   234,     3,     4,     5,     6,     7,     8,
     0,     0,    69,    -1,   186,    -1,    -1,    -1,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    -1,    -1,    33,    34,    -1,    -1,    37,    -1,
    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    51,    52,   187,    -1,    -1,    -1,    -1,    58,
    59,    60,    61,    -1,    -1,    64,    65,    66,     0,     1,
    -1,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    17,    -1,    19,    20,    21,
    22,    23,    24,    25,    -1,    27,    28,    29,    30,    -1,
    -1,    33,    34,    -1,    -1,    37,    -1,    -1,    -1,    41,
    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
    52,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
    -1,    -1,    64,    65,     1,    -1,     3,     4,     5,     6,
     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    17,    -1,    19,    20,    21,    22,    23,    24,    25,    -1,
    27,    28,    29,    30,    -1,    -1,    33,    34,    -1,    -1,
    37,    -1,    -1,    -1,    41,    42,     1,    -1,     3,     4,
     5,     6,     7,     8,    51,    52,    -1,    -1,    -1,    -1,
    -1,    58,    -1,    -1,    61,    -1,    -1,    64,    65,    66,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    33,    -1,
    -1,    -1,    37,    -1,    -1,    -1,    41,    42,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,
    -1,    -1,     1,    58,     3,     4,     5,     6,     7,     8,
    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,    17,    -1,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    -1,    -1,    33,    34,    -1,    -1,    37,    -1,
    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    51,    52,    -1,    -1,    -1,    -1,    -1,    58,
    59,    60,    61,    -1,    -1,    64,    65,     1,    -1,     3,
     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    17,    -1,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    -1,    -1,    33,
    34,    -1,    -1,    37,    -1,    -1,    -1,    41,    42,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
    -1,    -1,    -1,    -1,    58,    -1,    60,    61,    -1,    -1,
    64,    65,     1,    -1,     3,     4,     5,     6,     7,     8,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    17,    -1,
    19,    20,    21,    22,    23,    24,    25,    -1,    27,    28,
    29,    30,    -1,    -1,    33,    34,    -1,    -1,    37,    -1,
    -1,    -1,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    51,    52,    -1,    -1,    -1,    -1,    -1,    58,
    59,    60,    61,    -1,    -1,    64,    65,     1,    -1,     3,
     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    17,    -1,    19,    20,    21,    22,    23,
    24,    25,    -1,    27,    28,    29,    30,    -1,    -1,    33,
    34,    -1,    -1,    37,    -1,    -1,    -1,    41,    42,     1,
    -1,     3,     4,     5,     6,     7,     8,    51,    52,    -1,
    -1,    -1,    -1,    -1,    58,    -1,    60,    61,    -1,    -1,
    64,    65,     1,    -1,     3,     4,     5,     6,     7,     8,
    -1,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,    41,
    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
    52,    -1,    -1,    -1,    33,    -1,    58,    -1,    37,     9,
    10,    -1,    41,    65,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    51,    52,    -1,     9,    10,    -1,    -1,    58,
    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,    39,
    40,    -1,    42,    43,    44,    45,    46,    47,    48,    49,
    50,    51,    52,    53,    54,    39,    40,    57,    42,    43,
    44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
    54,     9,    10,    57,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    39,    40,     9,    10,    43,    44,    45,    46,    47,
    48,    49,    50,    51,    52,    53,    54,    39,    40,    57,
    -1,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    39,    40,    57,     9,    10,    -1,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
    53,    54,    -1,    -1,    57
};

#line 291 "./pars/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_Tparser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(0)
#define YYABORT         return(1)
#define YYERROR         goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_Tparser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_Tparser_CHAR = (token), YY_Tparser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_Tparser_CHAR);                                \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    { YY_Tparser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_Tparser_PURE
/* UNPURE */
#define YYLEX           YY_Tparser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_Tparser_CHAR;                      /*  the lookahead symbol        */
YY_Tparser_STYPE      YY_Tparser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_Tparser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_Tparser_LSP_NEEDED
YY_Tparser_LTYPE YY_Tparser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_Tparser_LSP_NEEDED
#define YYLEX           YY_Tparser_LEX(&YY_Tparser_LVAL, &YY_Tparser_LLOC)
#else
#define YYLEX           YY_Tparser_LEX(&YY_Tparser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_Tparser_DEBUG != 0
int YY_Tparser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
#endif



/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_Tparser_CLASS::
#endif
     YY_Tparser_PARSE(YY_Tparser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_Tparser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_Tparser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_Tparser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_Tparser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_Tparser_LSP_NEEDED
  YY_Tparser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_Tparser_LTYPE *yyls = yylsa;
  YY_Tparser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_Tparser_PURE
  int YY_Tparser_CHAR;
  YY_Tparser_STYPE YY_Tparser_LVAL;
  int YY_Tparser_NERRS;
#ifdef YY_Tparser_LSP_NEEDED
  YY_Tparser_LTYPE YY_Tparser_LLOC;
#endif
#endif

  YY_Tparser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  YY_Tparser_NERRS = 0;
  YY_Tparser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_Tparser_LSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_Tparser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_Tparser_LSP_NEEDED
      YY_Tparser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
#ifdef YY_Tparser_LSP_NEEDED
		 &yyls1, size * sizeof (*yylsp),
#endif
		 &yystacksize);

      yyss = yyss1; yyvs = yyvs1;
#ifdef YY_Tparser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_Tparser_ERROR("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_Tparser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_Tparser_LSP_NEEDED
      yyls = (YY_Tparser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_Tparser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_Tparser_DEBUG != 0
      if (YY_Tparser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (YY_Tparser_CHAR == YYEMPTY)
    {
#if YY_Tparser_DEBUG != 0
      if (YY_Tparser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_Tparser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_Tparser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_Tparser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_Tparser_DEBUG != 0
      if (YY_Tparser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_Tparser_CHAR);

#if YY_Tparser_DEBUG != 0
      if (YY_Tparser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_Tparser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_Tparser_CHAR, YY_Tparser_LVAL);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_Tparser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_Tparser_CHAR != YYEOF)
    YY_Tparser_CHAR = YYEMPTY;

  *++yyvsp = YY_Tparser_LVAL;
#ifdef YY_Tparser_LSP_NEEDED
  *++yylsp = YY_Tparser_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


/* #line 696 "./pars/bison.cc" */

  switch (yyn) {

case 1:
#line 83 "./d.y"
{ nodeptr = yyval.node_val = yyvsp[-1].node_val; YYACCEPT; ;
    break;}
case 2:
#line 84 "./d.y"
{ YYERROR; ;
    break;}
case 3:
#line 87 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FBLOCK,0); ;
    break;}
case 4:
#line 88 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,yyvsp[0].node_val,FBLOCK); ;
    break;}
case 5:
#line 91 "./d.y"
{ yyval.node_val = new Tnode(0,FNOP,0); ;
    break;}
case 6:
#line 92 "./d.y"
{ yyval.node_val = yyvsp[0].node_val; ;
    break;}
case 8:
#line 96 "./d.y"
{ yyval.node_val = NList(FSET,yyvsp[-3].node_val,yyvsp[0].node_val); ;
    break;}
case 24:
#line 112 "./d.y"
{ yyval.node_val = yyvsp[-1].node_val; ;
    break;}
case 25:
#line 113 "./d.y"
{ yyval.node_val = 0; ;
    break;}
case 26:
#line 116 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FSET,0); ;
    break;}
case 27:
#line 117 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FSET,0); ;
    break;}
case 28:
#line 118 "./d.y"
{ Tnode*n1 = new Tnode(*yyvsp[-2].node_val); yyval.node_val = NList(FSET,n1,NList(FPLUS,yyvsp[-2].node_val,yyvsp[0].node_val)); ;
    break;}
case 29:
#line 119 "./d.y"
{ Tnode*n1 = new Tnode(*yyvsp[-2].node_val); yyval.node_val = NList(FSET,n1,NList(FDIFFERENCE,yyvsp[-2].node_val,yyvsp[0].node_val)); ;
    break;}
case 30:
#line 120 "./d.y"
{ Tnode*n1 = new Tnode(*yyvsp[-2].node_val); yyval.node_val = NList(FSET,n1,NList(FTIMES,yyvsp[-2].node_val,yyvsp[0].node_val)); ;
    break;}
case 31:
#line 121 "./d.y"
{ Tnode*n1 = new Tnode(*yyvsp[-2].node_val); yyval.node_val = NList(FSET,n1,NList(FQUOTIENT,yyvsp[-2].node_val,yyvsp[0].node_val)); ;
    break;}
case 32:
#line 124 "./d.y"
{
			nodeptr = yyval.node_val = NList(FDEFUN,new Tnode(0,yyvsp[-7].sym_val,0),new Tnode(),yyvsp[-5].node_val,yyvsp[-3].node_val,yyvsp[-1].node_val);
		;
    break;}
case 33:
#line 127 "./d.y"
{
			nodeptr = yyval.node_val = NList(FDEFUN,new Tnode(0,yyvsp[-7].sym_val,0),new Tnode(0,yyvsp[-9].sym_val,0),yyvsp[-5].node_val,yyvsp[-3].node_val,yyvsp[-1].node_val);
		;
    break;}
case 34:
#line 130 "./d.y"
{
			nodeptr = yyval.node_val = NList(FDEFUN,new Tnode(0,yyvsp[-7].sym_val,0),yyvsp[-10].node_val,yyvsp[-5].node_val,yyvsp[-3].node_val,yyvsp[-1].node_val);
		;
    break;}
case 35:
#line 135 "./d.y"
{
			yyval.node_val = NList(FPACKAGE,new Tnode(0,tmp(new Tobject(yyvsp[-4].str_val)),0),yyvsp[-3].node_val,yyvsp[-1].node_val);
		;
    break;}
case 36:
#line 138 "./d.y"
{
			yyval.node_val = NList(FPACKAGE,new Tnode(0,tmp(new Tobject(filename)),0),yyvsp[-3].node_val,yyvsp[-1].node_val);
		;
    break;}
case 37:
#line 143 "./d.y"
{ yyval.node_val = new Tnode(0,FLIST,0); ;
    break;}
case 38:
#line 144 "./d.y"
{ yyval.node_val = NList(FLIST,new Tnode(0,yyvsp[0].sym_val,0)); ;
    break;}
case 39:
#line 145 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,new Tnode(0,yyvsp[0].sym_val,0),FLIST); ;
    break;}
case 41:
#line 149 "./d.y"
{ yyval.node_val = NList(FFORMAL,yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 42:
#line 150 "./d.y"
{ yyval.node_val = NList(FFORMAL_ELLIPSIS,yyvsp[-1].node_val); ;
    break;}
case 43:
#line 151 "./d.y"
{ yyval.node_val = NList(FFORMAL_ELLIPSIS,yyvsp[-3].node_val,yyvsp[-1].node_val); ;
    break;}
case 44:
#line 154 "./d.y"
{ yyval.node_val = new Tnode(); *yyval.node_val = FLOCAL; ;
    break;}
case 45:
#line 155 "./d.y"
{ yyval.node_val = new Tnode(0,FLOCAL,0); ;
    break;}
case 46:
#line 156 "./d.y"
{ yyval.node_val = yyvsp[-1].node_val; *yyval.node_val = FLOCAL; ;
    break;}
case 47:
#line 157 "./d.y"
{ yyval.node_val = new Tnode(0,FGLOBAL,0); ;
    break;}
case 48:
#line 158 "./d.y"
{ yyval.node_val = yyvsp[-1].node_val; *yyval.node_val = FGLOBAL; ;
    break;}
case 49:
#line 161 "./d.y"
{ yyval.node_val = NList(FIF,yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 50:
#line 162 "./d.y"
{ yyval.node_val = NList(FIF,yyvsp[-4].node_val,yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 51:
#line 164 "./d.y"
{ yyval.node_val = NList(FWHILE,yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 52:
#line 167 "./d.y"
{ yyval.node_val = NList(FREPEAT,yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 53:
#line 170 "./d.y"
{ yyval.node_val = NList(FFOR,yyvsp[-6].node_val,yyvsp[-4].node_val,yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 54:
#line 173 "./d.y"
{
	/* Notice that NList destroys the bindings of its arguments
       (that's why it's NList and not List). Thus we have to recompute
       ii and other Tnodes after every use. */
				Tsymbol *iiptr = UniqueSymbol((const Tchar*)"iiii");
				Tsymbol *vvptr = UniqueSymbol((const Tchar*)"vvvv");
				Tnode *ii = new Tnode(0,iiptr,0);
				Tnode *i = new Tnode(0,yyvsp[-4].sym_val,0);
				Tnode *arraybase = new Tnode(0,tmp(new Tobject(ArrayBase)),0);
				Tnode *setii = NList(FSET,ii, arraybase);
				ii = new Tnode(0,iiptr,0);
				Tnode *seti = NList(FSET, i, NList(FREF,new Tnode(0,vvptr,0),ii));
				Tnode *stmt = NList(FBLOCK,seti,yyvsp[0].node_val);
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
				Tnode *initstmt = NList(FSET,new Tnode(0,vvptr,0),new Tnode(*yyvsp[-2].node_val));
				Tnode *resetstmt1 = NList(FSET,new Tnode(0,iiptr,0),new Tnode(0,Tallrange(),0));
				Tnode *resetstmt2 = NList(FSET,new Tnode(0,vvptr,0),new Tnode(0,Tallrange(),0));
				yyval.node_val = NList(FBLOCK,initstmt,forstmt,resetstmt1,resetstmt2);
			;
    break;}
case 55:
#line 208 "./d.y"
{ yyval.node_val = new Tnode(0,FBREAK,0); ;
    break;}
case 56:
#line 211 "./d.y"
{ yyval.node_val = new Tnode(0,FCONTINUE,0); ;
    break;}
case 57:
#line 214 "./d.y"
{ yyval.node_val = new Tnode(0,FRETURN,0); ;
    break;}
case 58:
#line 217 "./d.y"
{ yyval.node_val = NList(FGOTO,new Tnode(0,yyvsp[0].sym_val,0)); ;
    break;}
case 59:
#line 220 "./d.y"
{ yyval.node_val = NList(FLABEL,new Tnode(0,yyvsp[0].sym_val,0)); ;
    break;}
case 60:
#line 223 "./d.y"
{
				Tnode*n1 = new Tnode(*yyvsp[-1].node_val);
				yyval.node_val = NList(FSET,n1,NList(FPLUS,yyvsp[-1].node_val,new Tnode(0,tmp(new Tobject(1)),0)));
			  ;
    break;}
case 61:
#line 229 "./d.y"
{
				Tnode*n1 = new Tnode(*yyvsp[-1].node_val);
				yyval.node_val = NList(FSET,n1,NList(FDIFFERENCE,yyvsp[-1].node_val,new Tnode(0,tmp(new Tobject(1)),0)));
			  ;
    break;}
case 62:
#line 235 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FDISP,0); ;
    break;}
case 64:
#line 239 "./d.y"
{ yyval.node_val = new Tnode(0,Tallrange(),0); ;
    break;}
case 65:
#line 240 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FRANGE,0); ;
    break;}
case 66:
#line 241 "./d.y"
{ yyvsp[-4].node_val->next = yyvsp[-2].node_val; yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-4].node_val,FRANGE,0); ;
    break;}
case 67:
#line 244 "./d.y"
{ yyval.node_val = yyvsp[-1].node_val; ;
    break;}
case 68:
#line 245 "./d.y"
{ YYERROR; ;
    break;}
case 69:
#line 246 "./d.y"
{ yyval.node_val = yyvsp[-1].node_val; ;
    break;}
case 70:
#line 247 "./d.y"
{ yyval.node_val = NList(FCALL,new Tnode(0,theHT.add((const Tchar*)"herm"),0),yyvsp[-1].node_val); ;
    break;}
case 71:
#line 248 "./d.y"
{ yyval.node_val = NList(FCALL,new Tnode(0,theHT.add((const Tchar*)"transpose"),0),yyvsp[-1].node_val); ;
    break;}
case 72:
#line 249 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FEQ,0); ;
    break;}
case 73:
#line 250 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FNE,0); ;
    break;}
case 74:
#line 251 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FGT,0); ;
    break;}
case 75:
#line 252 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FLT,0); ;
    break;}
case 76:
#line 253 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FGE,0); ;
    break;}
case 77:
#line 254 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FLE,0); ;
    break;}
case 78:
#line 255 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FAND,0); ;
    break;}
case 79:
#line 256 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,F_OR,0); ;
    break;}
case 80:
#line 257 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FPLUS,0); ;
    break;}
case 81:
#line 258 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FDIFFERENCE,0); ;
    break;}
case 82:
#line 259 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FTIMES,0); ;
    break;}
case 83:
#line 260 "./d.y"
{ yyval.node_val = DotProduct(yyvsp[-2].node_val,yyvsp[0].node_val); ;
    break;}
case 84:
#line 261 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FQUOTIENT,0); ;
    break;}
case 85:
#line 262 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FMOD,0); ;
    break;}
case 86:
#line 263 "./d.y"
{ yyvsp[-2].node_val->next = yyvsp[0].node_val; yyval.node_val = new Tnode(yyvsp[-2].node_val,FPOWER,0); ;
    break;}
case 87:
#line 264 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FMINUS,0); ;
    break;}
case 88:
#line 265 "./d.y"
{ yyval.node_val = yyvsp[0].node_val; ;
    break;}
case 89:
#line 266 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FNOT,0); ;
    break;}
case 91:
#line 268 "./d.y"
{ yyval.node_val = new Tnode(0,tmp(new Tobject(yyvsp[0].int_val)),0); ;
    break;}
case 92:
#line 269 "./d.y"
{ yyval.node_val = new Tnode(0,tmp(new Tobject(yyvsp[0].real_val)),0); ;
    break;}
case 93:
#line 270 "./d.y"
{ yyval.node_val = new Tnode(0,tmp(new Tobject(Tcomplex(0,yyvsp[0].real_val))),0); ;
    break;}
case 94:
#line 271 "./d.y"
{ yyval.node_val = new Tnode(0,tmp(new Tobject(yyvsp[0].str_val)),0); ;
    break;}
case 95:
#line 272 "./d.y"
{
			TObjectPtr objptr = new Tobject(Tint(yyvsp[0].ch_val));
			objptr->SetCharFlag();
			yyval.node_val = new Tnode(0,tmp(objptr),0);
		;
    break;}
case 97:
#line 278 "./d.y"
{ yyval.node_val = 0; ;
    break;}
case 98:
#line 281 "./d.y"
{
					yyval.node_val = NList(FCALL,
                               new Tnode(0,theHT.add((const Tchar*)"izeros"),0),
                               new Tnode(0,tmp(new Tobject(0)),0))
                         /*new Tnode(0,FEMPTY_ARRAY,0)*/;
                ;
    break;}
case 99:
#line 287 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FARRAY,0); ;
    break;}
case 101:
#line 289 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,yyvsp[0].node_val,FARRAY); ;
    break;}
case 103:
#line 293 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,yyvsp[0].node_val,FAPP); ;
    break;}
case 104:
#line 296 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,yyvsp[0].node_val,FAPP); ;
    break;}
case 105:
#line 299 "./d.y"
{ yyval.node_val = new Tnode(0,yyvsp[0].sym_val,0); ;
    break;}
case 106:
#line 300 "./d.y"
{
			yyval.node_val = Append(NList(FREF,yyvsp[-3].node_val/*new Tnode(0,$1,0)*/),yyvsp[-1].node_val);
			if (Length(yyvsp[-1].node_val) > MAXRANK) {
				cerr << "*** Too many (" << Length(yyvsp[-1].node_val) << ") indices in [..]. Max is " << MAXRANK << ".\n";
				yyerror("");
			}
		;
    break;}
case 107:
#line 307 "./d.y"
{
			yyval.node_val = Append(NList(FMREF,yyvsp[-3].node_val/*new Tnode(0,$1,0)*/),yyvsp[-1].node_val);
			if (Length(yyvsp[-1].node_val) > MAXRANK) {
				cerr << "*** Too many (" << Length(yyvsp[-1].node_val) << ") indices in [[..]]. Max is " << MAXRANK << ".\n";
				yyerror("");
			}
		;
    break;}
case 108:
#line 316 "./d.y"
{ yyval.node_val = Append(NList(FCALL,new Tnode(0,yyvsp[-3].sym_val,0)),yyvsp[-1].node_val); ;
    break;}
case 109:
#line 317 "./d.y"
{ *yyvsp[-1].node_val = FCALL; yyval.node_val = yyvsp[-1].node_val; ;
    break;}
case 110:
#line 320 "./d.y"
{ yyval.node_val = new Tnode(yyvsp[0].node_val,FLIST,0); ;
    break;}
case 111:
#line 321 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,yyvsp[0].node_val,FLIST); ;
    break;}
case 112:
#line 324 "./d.y"
{ yyval.node_val = new Tnode(0,FLIST,0); ;
    break;}
case 114:
#line 328 "./d.y"
{ yyval.node_val = new Tnode(new Tnode(0,yyvsp[0].sym_val,0),FLIST,0); ;
    break;}
case 115:
#line 329 "./d.y"
{ yyval.node_val = FlatOperator(yyvsp[-2].node_val,new Tnode(0,yyvsp[0].sym_val,0),FLIST); ;
    break;}
case 116:
#line 332 "./d.y"
{ yyval.node_val = new Tnode(0,FLIST,0); ;
    break;}
}

#line 696 "./pars/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_Tparser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_Tparser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_Tparser_LLOC.first_line;
      yylsp->first_column = YY_Tparser_LLOC.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_Tparser_NERRS;

#ifdef YY_Tparser_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_Tparser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_Tparser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_Tparser_ERROR_VERBOSE */
	YY_Tparser_ERROR("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_Tparser_CHAR == YYEOF)
	YYABORT;

#if YY_Tparser_DEBUG != 0
      if (YY_Tparser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_Tparser_CHAR, yytname[yychar1]);
#endif

      YY_Tparser_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_Tparser_LSP_NEEDED
  yylsp--;
#endif

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_Tparser_DEBUG != 0
  if (YY_Tparser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_Tparser_LVAL;
#ifdef YY_Tparser_LSP_NEEDED
  *++yylsp = YY_Tparser_LLOC;
#endif

  yystate = yyn;
  goto yynewstate;
}

/* END */

/* #line 890 "./pars/bison.cc" */
#line 336 "./d.y"
	/* auxiliary functions section */

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
