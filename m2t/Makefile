default:m2t

lex.yy.c:m2t.l
	../flex++-3.0.2/flex++ -p -S../pars/flexskel.cc -H../pars/flexskel.h m2t.l

m2t:lex.yy.c
	$(CXX) -o m2t lex.yy.c

clean:
	rm -f *.o core *~ lex.yy.c m2t
