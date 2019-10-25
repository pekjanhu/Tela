#!/bin/sh
#
# cray_fix.sh - fix Tela source files to work around
# some UNICOS compiler bugs (scc4.0, C++1.0).
# (and actually the _Pragma keyword is unimplemented in C++2.0 also
#  so this fix is needed for C++2.0, too).
#
# It will rewrite all VECTORIZED macros as pragmas. This will
# work around some compiler bugs.
#
# The transforms are done in place. They should not cause any problems
# on other machines, but why push your luck by trying...
#
if uname -a | fgrep -i cray >/dev/null; then :; else
	echo '*** This is not Cray Research. Run cray_fix.sh only on Cray.'
	exit 1
fi

if [ -f objarithm.C ]; then :; else
	if [ -f ../objarithm.C ]; then cd ..; else
		echo '*** You must run cray_fix.sh from Tela source directory'
		exit 2
	fi
fi

echo 'This script will modify your Tela sources.'
echo 'Press return to continue, Control-C to quit now.'
read ans

#echo 'Inserting VECTORIZED in every for-loop'
#for f in objarithm.C templ/*op*.C
#do
#	echo "in $f ..."
#	sed -e 's/for[ \t]*\(.*\)/VECTORIZED &/g' <$f >tmp.$$
#	mv tmp.$$ $f
#done

echo 'Compiling filtKEYWORD C program...'

cat <<! >filtKEYWORD.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
/* call form: filtKEYWORD 'VECTORIZED ' 'ivdep'
   or filtKEYWORD 'NOVECTOR ' 'novector' */
{
	int i;
	char *pos,*modpos;
	char s[MAXLINE+1];
/*
	char *item = "VECTORIZED ";
	char *pragma = "ivdep";
*/
	char *item = argv[1];
	char *pragma = argv[2];
	int L = strlen(item);
	while (1) {
		modpos = 0;
		fgets(s,MAXLINE,stdin);
		if (feof(stdin)) break;
		pos = strstr(s,"//");
		if (pos) {*pos = '\n'; *(pos+1) = '\0'; modpos = pos;}
		pos = strstr(s,item);
		if (pos) {
			int allspace = 1;
			for (i=0; s[i] && (&s[i])!=pos; i++)
				if (!isspace(s[i])) {allspace=0; break;}
			if (allspace) {
				printf("#pragma _CRI %s\n",pragma);
				for (i=0; s[i] && (&s[i])!=pos; i++)
					putchar(s[i]);
			} else {
				for (i=0; s[i] && (&s[i])!=pos; i++)
					putchar(s[i]);
				printf("\n#pragma _CRI %s\n",pragma);
			}
			if (s[i]) {
				i+= L;
				for (; s[i]; i++) putchar(s[i]);
			}
		} else {
			if (modpos) {*modpos='/'; *(modpos+1)='/';}
			fputs(s,stdout);
		}
	}
	return 0;
}
!

cc -o filtKEYWORD filtKEYWORD.c

echo 'Transforming VECTORIZED to #pragma _CRI ivdep'

for f in *.C object.H templ/*op*.C *.ct
do
	if fgrep VECTORIZED $f >/dev/null; then
		echo "in $f ..."
		./filtKEYWORD 'VECTORIZED ' 'ivdep' <$f >tmp.$$
		mv tmp.$$ $f
	fi
done

echo 'Transforming NOVECTOR to #pragma _CRI novector'

for f in *.C templ/*op*.C *.ct
do
	if fgrep NOVECTOR $f >/dev/null; then
		echo "in $f ..."
		./filtKEYWORD 'NOVECTOR ' 'novector' <$f >tmp.$$
		mv tmp.$$ $f
	fi
done

echo 'Cleaning up..'
rm -f filtKEYWORD filtKEYWORD.c





