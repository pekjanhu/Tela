#!/bin/sh
#
# This file is part of tela the Tensor Language.
# Copyright (c) 1994 Pekka Janhunen
#
# telahelp.sh
# This script is called by the Tela kernel when help on t-function
# is requested by the user.
# Usage: telahelp.sh myfn fyle.t
#        scans for the definition of function myfn in fyle.t
#        trying to extract the comment immediately following the header.
# Both /* ... */ and sequential // comments are recognized.
#
# This script should work in almost any Unix system.
# It only needs /bin/sh, egrep, tail and basic old awk.
#
# Fixed Dec 11 2008: Syntax tail +N no longer supported, now use tail -n +N

fn=$1
file=$2

if [ -f "$file" ]; then

	lines=`egrep -n "^[ \t]*function[^{]*[ \=\\t]${fn}[ \\t]*\\(" "$file" | cut -f1 -d':'`
	if [ -n "$lines" ]; then
		for line in $lines; do
			tail -n +`expr $line + 1` "$file" | awk '
BEGIN {incomment=0; printed=0}
/\/\*.*/ {
	printed = 1
	if (NR == 1) {
		incomment = 1
		L = length();
		if ($0 ~ /\*\//) {
			print substr($0,3,L-4)
			incomment = 0
		} else
			print substr($0,3,L-2)
	} else
		exit
	next
}
/.*\*\// {
	print substr($0,1,length()-2)
	exit
}
/\/\/.*/ {
	printed = 1;
	if (substr($0,3,1) == " ")
		print substr($0,4,length()-3)
	else
		print substr($0,3,length()-2)
	next
}
{
	if (incomment) {
		printed = 1
		print
	} else
		exit
}
END {if (!printed) print "No comment string."}
'
		done
	else
		echo "Function pattern \"${fn}\" not found in \"${file}\"."
		exit 1
	fi

else
	echo '*** telahelp.sh: File '"$file"' not found.'
	exit 1
fi
