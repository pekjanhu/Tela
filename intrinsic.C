/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#include "tree.H"
#include "prg.H"

extern Toperand convertToOperand(const Tnode*);		// defined in codegen.C

static void Install(const TIntrinsicCompilerPtr fptr, Tcode c, const char *name) {
	Tsymbol* symptr = theHT.add((const Tchar*)name);
	*(symptr->value()) = fptr;
	symptr->value()->IntrinsicCodeRef() = int(c);
	if (flags::verbose)
		cout << "  Installed intrinsic function '" << name << "'\n";
}

static int AbsCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	int ret=1;
	if (Nargin==1 && Nargout==1) {
		prg.add(Fabs,convertToOperand(out[0]),convertToOperand(in[0]));
	} else
		ret = 0;
	return ret;
}

static int MinMaxCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	int ret=1;
	if (Nargin==1 && Nargout==1) {
		prg.add(Tcode(c),convertToOperand(out[0]),convertToOperand(in[0]));
	} else if (Nargin==1 && Nargout==2) {
		prg.add(Tcode(c),convertToOperand(out[0]),convertToOperand(in[0]));
		prg.add(Fmmpos,convertToOperand(out[1]));
	} else if (Nargin>1 && Nargout==1) {
		const Toperand outop = convertToOperand(out[0]);
		prg.add(Tcode(c),outop,convertToOperand(in[0]),convertToOperand(in[1]));
		for (int i=2; i<Nargin; i++)
			prg.add(Tcode(c),outop,outop,convertToOperand(in[i]));
	} else
		ret = 0;
	return ret;
}

static int ZerosCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	if (Nargout==1) {
		if (Nargin > MAXRANK) return 1;
		prg.add(Tcode(c),1+Nargin);
		prg.append(convertToOperand(out[0]));
		for (int i=0; i<Nargin; i++)
			prg.append(convertToOperand(in[i]));
		return 1;
	} else return 0;
}

static int NargCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	int ret=1;
	if (Nargin==0 && Nargout==1) {
		prg.add(Tcode(c),convertToOperand(out[0]));
	} else
		ret = 0;
	return ret;
}

static int ArgInOutCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	int ret=1;
	if (Nargin==1 && Nargout==1) {
		prg.add(Tcode(c),convertToOperand(out[0]),convertToOperand(in[0]));
	} else
		ret = 0;
	return ret;
}

static int SetArgOutCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	int ret=1;
	if (Nargin==2 && Nargout==1) {
		prg.add(Fsetout,convertToOperand(in[0]),convertToOperand(in[1]));
		prg.add(Fmove,convertToOperand(out[0]),convertToOperand(new Tnode(0,Tallrange(),0)));
	} else
		ret = 0;
	return ret;
}

static int NopCompiler
    (Tnode*const in[], const int Nargin, Tnode*const out[], const int Nargout, const int c, Tprg& prg)
{
	int ret=1;
	if (Nargin==0 && (Nargout==1 || Nargout==0)) {
		prg.add(Fnop);
	} else
		ret = 0;
	return ret;
}

void InstallIntrinsic()
{
	Install(AbsCompiler,Fabs,"abs");
	Install(MinMaxCompiler,Fmin,"min");
	Install(MinMaxCompiler,Fmax,"max");
	Install(ZerosCompiler,Fizeros,"izeros");
	Install(ZerosCompiler,Frzeros,"rzeros");
	Install(ZerosCompiler,Fczeros,"czeros");
	Install(ZerosCompiler,Frzeros,"zeros");	// zeros is alias for rzeros
	Install(ZerosCompiler,Fozeros,"voids");
	Install(NargCompiler,Fnin,"Nargin");
	Install(NargCompiler,Fnout,"Nargout");
	Install(ArgInOutCompiler,Fgetin,"argin");
	Install(ArgInOutCompiler,Fgetout,"argout");
	Install(SetArgOutCompiler,Fsetout,"SetArgOut");
	Install(NopCompiler,Fnop,"nop");
}

