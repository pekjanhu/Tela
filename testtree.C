/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-1999 Pekka Janhunen
 */

#include "tree.H"
#include <fstream.h>

main()
{
	Tnode* p;
	ifstream fp("tree.dat");
	p = readtree(fp);
	cout<<*p<<'\n';
	return 0;
}
