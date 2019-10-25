/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#include "error.H"

int main()
{
	err << "Error message 1\n!!\n";
	error();
	err << "Another error\n";
	error();
	return 0;
}
