# Compile pod-files to man-pages

version:=0.2.1
tarfile:=tela-extradoc-$(version).tar.gz

pod:=$(wildcard *.pod)
man:=$(subst .pod,.man,$(pod))
html:=$(subst .pod,.html,$(pod))

POD_FLAGS:=--section="1" --release="Tela-1.31" --center="Tela Documentation"
TIDY_FLAGS:=


.phony: all
all: man

.phony: dist
dist: $(tarfile)

.phony: man
man: $(man)

.phony: html
html: $(html)

.phony: clean
clean:
	rm -f $(man) $(html) pod2html-*cache $(tarfile)

.phony: distclean
distclean: clean
	rm -f *~


%.man:%.pod
	pod2man $(POD_FLAGS) $< > $*.man

%.html:%.pod
	pod2html --infile=$< | tidy $(TIDY_FLAGS) 2>/dev/null > $*.html; true


$(tarfile): $(pod) Makefile
	tar czf $@ $^
