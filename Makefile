# Makefile for Tela. For usage, type 'make what'.
# Usually you won't edit this file any more! Instead, edit file makeinc.
# ----------------------------------------------------------------------
#
# Includes makeinc, which is created by ./configure script
# from makeinc.in
# makeinc is included after library etc. definitions, so you
# can override them in makeinc (useful if you are e.g. compiling
# with two different compilers on the same system).
#
# Tela quick installation instructions:
# -------------------------------------
# 1) Type ./configure. This will create makeinc.
#    with choices guessed by ./configure.
# 2) Edit makeinc. Set the paths to various libraries and wanted
#    compile flags. At minimum, check points marked with "unknown".
# 3) Type make. If needed, you can re-edit the makeinc several times
#    and try it with make. Usually you need to run ./configure only once.
#
# IF YOUR make DOESN'T SUPPORT include:
# -------------------------------------
# Then use a make program that does, such as GNU make.
# Or, include makeinc manually after typing ./configure.
# Again, usually you have to do this only once.

SHELL = /bin/sh

GNUZIP = gzip -9

READLINEINCLUDE = -I/usr/local/include
SGML2PS = sgml2latex --output=ps
#HDFLIBS = /usr/local/lib/libdf.a
#DLDINCLUDE =
FLEX++ = $(srcdir)/flex++-3.0.2/flex++

# --------------- Start of platform configuration part -----------------
# --- NOTICE: Everything is commented out in this section now,
# --- and the definitions are in makeinc. Don't edit this section,
# --- edit makeinc instead!
# ----------------------------------------------------------------------

# LINUX
#HDFLIBS = /usr/local/lib/libdf.a
#HDFINCLUDE = -I/usr/local/include -DSUN386

# IRIS4 OS version 5.2
#HDFLIBS = /usr/local/lib/libdf3.1.a
#HDFINCLUDE = -I/usr/local/include/hdf3.1 -DIRIS4 -D__STDC__

# Sun-OS 4.1.3 with gcc
#FLIBS = -L/usr/local/lang/SC2.0.1patch /usr/local/lang/SC2.0.1patch/values-Xs.o -lM77 -lF77 -lansi
#HDFLIBS = /usr/local/lib/libdf.a
#HDFINCLUDE = -I/usr/local/include/hdf -DSUN -DCONVEX -D__STDC__

# SUN Solaris 2.3 with gcc
#FLIBS = -L/opt/SUNWspro/SC2.0.1 -lM77 -lF77 -ldl
#HDFLIBS = ../lib/libdf.a
#HDFINCLUDE = -I../include/hdf -DSUN -D__STDC__
#BLASLIB = ../lib/libblas.a

# SUN Solaris 2.3 with native CC
#CFLAGS=-fast -O3  -DBROKEN_UCHAR_OUTPUTTER=1
#FLIBS = -L/opt/SUNWspro/SC2.0.1 -lM77 -lF77 -ldl -lmp
#HDFLIBS = ../lib/libdf.a
#HDFINCLUDE = -I../include/hdf -DSUN -D__STDC__
#BLASLIB = ../lib/libblas.a

# CONVEX with system supplied cc/CC
#FLIBS = -lF77 -lI77 -lU77 -lmathC2 -fn
#CFLAGS = -O1 -DCONVEX -fn
#UNROLL_FLAGS = -O2
#HDFLIBS = /usr/local/lib/libdf.a
#HDFINCLUDE = -I/usr/local/include/hdf -DCONVEX -D__STDC__
#BLASLIB = -lveclib

# AIX
#HDFLIBS = /home/pjanhune/HDF3.1r5/src/libdf.a
#HDFINCLUDE = -I/home/pjanhune/HDF3.1r5/src -DIBM6000

# --------------- End of platform configuration part -----------------

default : tela

include makeinc

INSTALL_BINDIR = $(ARCHDEP_INSTALLATION_PREFIX)
INSTALL_LIBDIR = $(INSTALLATION_PREFIX)
TELAKKA_OPTS = -v

# --------------- End of platform configuration part -----------------

# ------ List the wanted statically linked C-tela modules here:-------
#To include Tsyganenko routines, uncomment the following two lines
TSYGANENKO_O = geopack_ct.o
TSYGLIB = libtsyg.a
MPEG2ENCODELIB = libmpeg2encode.a
CTELA_MODULES = std.o files.o plotmtv.o sppc.o la.o fftw.o fft.o dld.o fileio.o \
				numerics.o ani.o specfun_ct.o $(MATLABENG_O) $(CDF_O) $(TSYGANENKO_O)
NEEDED_BY_CTELA_MODULES = $(MATLABLIB) $(VECFFTPACK_O) fftpack.o $(TSYGLIB) $(MPEG2ENCODELIB) \
	png.o image.o ode.o specfun.o
#---------------------------------------------------------------------

DEFAULT_TELAPATH = $(INSTALL_LIBDIR)/t:$(INSTALL_LIBDIR)/ct:$(INSTALL_LIBDIR):.

CC_OPTIONS = $(CPPFLAGS) $(DEFS) -I$(srcdir) -I$(srcdir)/t $(CFLAGS)
# Had to add -I$(srcdir)/t because if compiling from a subdir,
# bison inserts include ../d.l.h in d.l.c, which cannot be found
# if the same d.l.c is later compiled from the main dir.
F77_OPTIONS = $(FFLAGS)

LIBRARIES = $(READLINELIB) $(HDFLIBS) $(JPEGLIB) $(CDFLIB) $(LAPACKLIB) $(BLASLIB) \
			$(RFFTWLIB) $(FFTWLIB) \
			$(FLIBS) $(X_LIBS) $(LIBS) $(ZLIB) $(DLDLIB) -lm $(EXTRALIBS)

OBJECTDEPS = objarithm.H object.H def.H deftyp.h Tcomplex.H error.H ctinfo.H common.H $(srcdir)/templ/tLL.H
EXECUTABLES = testobject testil testerror testtree tela ctpp

all : tela docs

what help :
	@echo 'make                - same as make tela';\
	echo 'make what           - give this message';\
	echo 'make tela           - make the program';\
	echo 'make docs           - make the DVI, TXT, HTML, PS documentation';\
	echo 'make install        - install the program and support files';\
	echo 'make installdocs    - install online documentation';\
	echo 'make binary         - make binary distribution tarfile';\
	echo 'make dist           - make source distribution tarfile';\
	echo 'make clean          - remove object files etc.';\
	echo 'make docclean       - remove .html, .dvi etc. files in doc/';\
	echo 'make veryclean      - == make clean docclean';\

# -----------------------------------------------------------------
# ---- General rule to make object files from C++ files

.C.o:
	$(C++) -c $(CC_OPTIONS) $<

# ----------------------------------------------------------------
# ---- Parser modules

$(FLEX++) :
	cd $(srcdir)/flex++-3.0.2; make flex++

d.y.c d.y.h : d.y lisp.H tree.H machine.H $(OBJECTDEPS)
	$(BISON++) -d -S$(srcdir)/pars/bison.cc -H$(srcdir)/pars/bison.h \
		-o $(srcdir)/d.y.c $(srcdir)/d.y

d.l.c d.l.h : d.l tree.H $(OBJECTDEPS)
	$(FLEX++) -8 -p -S$(srcdir)/pars/flexskel.cc -H$(srcdir)/pars/flexskel.h \
		-o$(srcdir)/d.l.c -h$(srcdir)/d.l.h $(srcdir)/d.l

d.y.o : d.y.c
	$(C++) -c $(CC_OPTIONS) $(srcdir)/d.y.c

d.l.o : d.l.c d.y.h
	$(C++) -c $(CC_OPTIONS) $(srcdir)/d.l.c

# ----------------------------------------------------------------
# ---- Basic modules

error.o : error.C error.H common.H def.H deftyp.h

object.o : object.C $(OBJECTDEPS) templ/tLL.C

objarithm.o : objarithm.C objarithm.H complexvec.C $(OBJECTDEPS) \
			$(srcdir)/templ/tbinop.C $(srcdir)/templ/tbinintop.C \
			$(srcdir)/templ/tunop.C $(srcdir)/templ/tunop1.C \
			$(srcdir)/templ/tcmpop.C $(srcdir)/templ/tminmaxop.C \
			$(srcdir)/templ/teqop.C $(srcdir)/templ/tLL.C \
			$(srcdir)/templ/ttestcmpop.C \
			$(OBJECTDEPS)
	$(C++) -c $(CC_OPTIONS) $(UNROLL_FLAGS) $(NOVECTORALIASES) $(srcdir)/objarithm.C

# CONVEX:
#	$(C++) -F $(CC_OPTIONS) $(srcdir)/objarithm.C | \
#	sed 's/for(/@#pragma _CNX no_recurrence@for(/g' | tr '@' '\012' >$(srcdir)/objarithm.c;\
#	$(CC) -c $(CC_OPTIONS) $(UNROLL_FLAGS) $(srcdir)/object.c

Tcomplex.o : Tcomplex.C Tcomplex.H def.H deftyp.h

machine.o : machine.C machine.H symbol.H gatscat.H $(OBJECTDEPS)

tree.o : tree.C tree.H symbol.H $(OBJECTDEPS)

lisp.o : lisp.C lisp.H tree.H symbol.H $(OBJECTDEPS)

gatscat.o : gatscat.C gatscat.H $(OBJECTDEPS)

symbol.o : symbol.C symbol.H gatscat.H $(OBJECTDEPS)

prg.o : prg.C prg.H $(srcdir)/templ/tLL.C $(srcdir)/templ/tLL.H \
		machine.H symbol.H gatscat.H objarithm.H $(OBJECTDEPS)

optimize.o : optimize.C prg.H $(srcdir)/templ/tLL.H $(srcdir)/templ/tLL.C \
			machine.H common.H symbol.H gatscat.H $(OBJECTDEPS)

codegen.o : codegen.C prg.H machine.H symbol.H gatscat.H lisp.H \
			tree.H templ/tLL.H templ/tLL.C $(OBJECTDEPS)

intrinsic.o : intrinsic.C prg.H machine.H symbol.H gatscat.H tree.H $(OBJECTDEPS)

Ctgen.o : Ctgen.C prg.H machine.H symbol.H gatscat.H lisp.H tree.H $(OBJECTDEPS)

png.o: png.C png.H image.H def.H
image.o: image.C image.H def.H
ode.o: ode.H ode.C def.H deftyp.h Tcomplex.H
specfun.o: specfun.H specfun.C ode.H def.H deftyp.h Tcomplex.H

version.H : VERSION makeinc
	echo '// version.H - generated from environment vars and file VERSION by Tela Makefile - do not edit' >$(srcdir)/version.H
	@echo '' >>$(srcdir)/version.H
	@echo '#define VERSION "'`cat $(srcdir)/VERSION | tr -d '\012'`'"' >>$(srcdir)/version.H
	@echo '#define WHEN_COMPILED "''$(C++)'`if [ \"$(C++)\" = \"c++\" -o \"$(C++)\" = \"g++\" ]; then $(C++) --version; fi`' on '`uname -s`' '`uname -r`' ('`uname -m`'), '`date '+%b %d, %Y'`'"' >>$(srcdir)/version.H

tela.o : tela.C common.H prg.H machine.H symbol.H gatscat.H tree.H d.y.h d.l.h $(OBJECTDEPS) version.H
	$(C++) -c $(CC_OPTIONS) $(READLINEINCLUDE) "-DDEFAULT_TELAPATH=\"$(DEFAULT_TELAPATH)\"" $(srcdir)/tela.C

newdel.o : newdel.C common.H
	$(C++) -c $(CC_OPTIONS) -DWITHIN_TELA $(srcdir)/newdel.C

# alloca.c is needed if system does not have alloca

alloca.o : alloca.c
	$(CC) -c $(DEFS) $(srcdir)/alloca.c

# noreadline.c is needed if you can't get GNU Readline to work
# no DEFS are needed here

noreadline.o : noreadline.c
	$(CC) -c $(srcdir)/noreadline.c

# vranf.f is used only on Cray

vranf.o : vranf.f
	cf77 -c vranf.f

# ----------------------------------------------------------------
# ---- Kernel archive

libtela.a : lisp.o error.o objarithm.o object.o gatscat.o tree.o machine.o symbol.o \
			$(TCOMPLEX_O) prg.o optimize.o codegen.o Ctgen.o intrinsic.o d.y.o d.l.o tela.o \
			$(ALLOCA_O) $(VRANF_O) $(NEWDEL_O)
	@rm -f libtela.a
	$(AR) cr libtela.a codegen.o Ctgen.o intrinsic.o d.y.o d.l.o prg.o optimize.o symbol.o \
		 $(TCOMPLEX_O) machine.o tree.o gatscat.o objarithm.o object.o lisp.o \
		 error.o tela.o $(ALLOCA_O) $(VRANF_O) $(NEWDEL_O)
	$(RANLIB) libtela.a

# ----------------------------------------------------------------
# ---- FFTPACK library

fftpack.o : fftpack.c deftyp.h
	$(CC) -c $(CC_OPTIONS) $(srcdir)/fftpack.c

vecfftpack.o : vecfftpack.c deftyp.h
	$(CC) -c $(CC_OPTIONS) $(AGGRESS) $(srcdir)/vecfftpack.c

# ----------------------------------------------------------------
# ---- telakka (script to compile and link C-tela files)

telakka : telakka.in Makefile makeinc
	echo '#!/bin/sh' >telakka.scratch
	@echo '#' >>telakka.scratch
	@echo "CPLUSPLUS=\"$(C++)\"" >>telakka.scratch
	@echo "CC_OPTIONS=\"$(CC_OPTIONS)\"" >>telakka.scratch
	@echo "LDFLAGS=\"$(LDFLAGS)\"" >>telakka.scratch
	@echo "BINDIR=`pwd`" >>telakka.scratch
	@currdir=`pwd`; cd $(srcdir); echo "INCLUDEDIR=`pwd`" >>$$currdir/telakka.scratch
	@echo "VERSION=\"`cat $(srcdir)/VERSION | tr -d '\012'`\"" >>telakka.scratch
	@echo "WHEN_COMPILED=\"$(C++) on `uname -a | tr -d '\012'`, `date | tr -d '\012'`\"" >>telakka.scratch
	@echo "STD_CT_OBJECTS=\"$(CTELA_MODULES) $(LOCAL_CTELA_MODULES) $(NEEDED_BY_CTELA_MODULES) $(NEEDED_BY_LOCAL_CTELA_MODULES)\"" >>telakka.scratch
	@echo "LIBRARIES=\"$(LIBRARIES)\"" >>telakka.scratch
	cat telakka.scratch $(srcdir)/telakka.in >telakka
	rm telakka.scratch
	chmod +x telakka

# ----------------------------------------------------------------
# ---- C-tela modules

CTDEPS = ctpp symbol.H objarithm.H common.H object.H ctinfo.H def.H deftyp.h error.H

std.o : std.ct $(CTDEPS) randef.H
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/std.ct

plotmtv.o : plotmtv.ct $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(X_CFLAGS) $(srcdir)/plotmtv.ct

sppc.o : sppc.ct $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(X_CFLAGS) $(srcdir)/sppc.ct

files.o : files.ct $(CTDEPS) $(srcdir)/png.H $(srcdir)/image.H
	./telakka $(TELAKKA_OPTS) $(STRONGVECTOR) -c $(HDFINCLUDE) $(srcdir)/files.ct

la.o : la.ct lapack.H $(CTDEPS)
	./telakka $(TELAKKA_OPTS) $(NOVECTORALIASES) -c $(srcdir)/la.ct

fft.o : fft.ct fftpack.H vecfftpack.H $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/fft.ct

fftw.o : fftw.ct $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/fftw.ct

dld.o : dld.ct symbol.H ctinfo.H $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(DLDINCLUDE) $(srcdir)/dld.ct

fileio.o : fileio.ct $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/fileio.ct

numerics.o : numerics.ct $(CTDEPS) randef.H
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/numerics.ct

specfun_ct.o : specfun_ct.ct $(CTDEPS) ode.H
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/specfun_ct.ct

CDF.o : CDF.ct $(CTDEPS)
	./telakka $(CDFINCLUDE) $(TELAKKA_OPTS) -c $(srcdir)/CDF.ct

ani.o : ani.ct $(CTDEPS) $(srcdir)/mpeg2encode/mpegintf.H $(srcdir)/png.H
	./telakka $(CDFINCLUDE) $(TELAKKA_OPTS) -c $(srcdir)/ani.ct

tests.o : tests.ct $(CTDEPS)
	./telakka $(TELAKKA_OPTS) -c $(srcdir)/tests.ct

# ----------------------------------------------------------------
# ---- Optional (Tsyganenko magnetic field model) routines

optdir = $(srcdir)/opt

geopack.o : $(srcdir)/opt/geopack.f
	$(F77) -c $(F77_OPTIONS) $(optdir)/geopack.f

T89c.o : $(srcdir)/opt/T89c.f
	$(F77) -c $(F77_OPTIONS) $(optdir)/T89c.f

T96.o : $(srcdir)/opt/T96.f
	$(F77) -c $(F77_OPTIONS) $(optdir)/T96.f

#tsyg.o : $(optdir)/tsyg.c
#	$(CC) -c $(CC_OPTIONS) $(optdir)/$*.c

geopackintf.o : $(optdir)/geopackintf.C $(optdir)/geopackintf.H $(optdir)/T96.h $(optdir)/T89c.h $(CTDEPS)
	$(C++) -c $(CC_OPTIONS) $(CDFINCLUDE) $(optdir)/geopackintf.C

geopack_ct.o : $(optdir)/geopack_ct.ct $(optdir)/geopackintf.H $(CTDEPS) $(TSYGLIB)
	./telakka $(TELAKKA_OPTS) -c $(optdir)/$*.ct

$(TSYGLIB) : geopackintf.o geopack.o T96.o T89c.o
	@rm -f $(TSYGLIB)
	$(AR) cr $(TSYGLIB) geopackintf.o geopack.o T96.o T89c.o
	$(RANLIB) $(TSYGLIB)

# ----------------------------------------------------------------
# ---- MPEG2 encoder routines

mpegdir = $(srcdir)/mpeg2encode
mpegdep = $(mpegdir)/mpegintf.H $(mpegdir)/global.h

mpegintf.o : $(mpegdir)/mpegintf.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/mpegintf.C

conform.o : $(mpegdir)/conform.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/conform.C

putpic.o : $(mpegdir)/putpic.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/putpic.C

puthdr.o : $(mpegdir)/puthdr.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/puthdr.C

putmpg.o : $(mpegdir)/putmpg.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/putmpg.C

putvlc.o : $(mpegdir)/putvlc.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/putvlc.C

putbits.o : $(mpegdir)/putbits.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/putbits.C

motion.o : $(mpegdir)/motion.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/motion.C

predict.o : $(mpegdir)/predict.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/predict.C

writepic.o : $(mpegdir)/writepic.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/writepic.C

transfrm.o : $(mpegdir)/transfrm.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/transfrm.C

fdctref.o : $(mpegdir)/fdctref.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/fdctref.C

idct.o : $(mpegdir)/idct.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/idct.C

quantize.o : $(mpegdir)/quantize.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/quantize.C

ratectl.o : $(mpegdir)/ratectl.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/ratectl.C

stats.o : $(mpegdir)/stats.C $(mpegdep)
	$(C++) -c $(CFLAGS) $(mpegdir)/stats.C

libmpeg2encode.a : mpegintf.o conform.o putpic.o puthdr.o putmpg.o putvlc.o putbits.o motion.o predict.o writepic.o transfrm.o fdctref.o idct.o quantize.o ratectl.o stats.o
	@rm -f libmpeg2encode.a
	$(AR) cr libmpeg2encode.a mpegintf.o conform.o putpic.o puthdr.o putmpg.o putvlc.o putbits.o motion.o predict.o writepic.o transfrm.o fdctref.o idct.o quantize.o ratectl.o stats.o
	$(RANLIB) libmpeg2encode.a

# ----------------------------------------------------------------
# ---- Test modules

testil.o : testil.C prg.H machine.H $(OBJECTDEPS)

testobject.o : testobject.C $(OBJECTDEPS)

# ----------------------------------------------------------------
# ---- Executables

testobject : testobject.o object.o error.o
	$(C++) -o testobject testobject.o object.o error.o -lm

testil : testil.o prg.o machine.o symbol.o object.o gatscat.o error.o
	$(C++) -o testil testil.o prg.o machine.o symbol.o object.o gatscat.o error.o -lm

testerror : testerror.C error.o
	$(C++) -o testerror testerror.C error.o

testtree : testtree.o tree.o symbol.o object.o gatscat.o error.o
	$(C++) -o testtree testtree.o tree.o symbol.o object.o gatscat.o error.o -lm

tela : $(FLEX++) telakka $(CTDEPS) libtela.a \
		$(CTELA_MODULES) $(NEEDED_BY_CTELA_MODULES) \
		$(LOCAL_CTELA_MODULES) $(NEEDED_BY_LOCAL_CTELA_MODULES) $(EXTRA_TARGETS)
	./telakka $(TELAKKA_OPTS) -o tela

# ----------------------------------------------------------------
# ---- Preprocessor ctpp

ctpp : ctpp.C
	$(C++) -o ctpp $(CC_OPTIONS) $(srcdir)/ctpp.C

# ----------------------------------------------------------------
# ---- .L compilation listing files on Cray

complisting : libtela.a mkcclisting
	@echo 'Some warning messages may now appear...'
	-./mkcclisting $(srcdir)/*.V
	@echo 'See *.L files (also in templ/) for compilation listings.'
	@echo 'The utility mkcclisting might be generally useful for you.'

mkcclisting : mkcclisting.c
	$(CC) -o mkcclisting $(srcdir)/mkcclisting.c

# ----------------------------------------------------------------
# ---- Document files

docdir = $(srcdir)/doc

docs1: $(docdir)/telahelp.dvi $(docdir)/telafuncs.dvi $(docdir)/telafuncsSectioned.dvi $(docdir)/usrguide.dvi \
       $(docdir)/telahelp.txt $(docdir)/telafuncs.txt $(docdir)/telafuncsSectioned.txt $(docdir)/usrguide.txt \
       $(docdir)/telahelp.html $(docdir)/telafuncs.html $(docdir)/telafuncsSectioned.html $(docdir)/usrguide.html

ps : $(docdir)/telahelp.ps $(docdir)/telafuncs.ps $(docdir)/telafuncsSectioned.ps $(docdir)/usrguide.ps

docs : docs1 ps

$(docdir)/telahelp.dvi : $(docdir)/telahelp.sgml
	cd $(docdir); $(SGML2PS) --output=dvi telahelp.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --dvi telahelp.sgml

$(docdir)/telahelp.html : $(docdir)/telahelp.sgml
	cd $(docdir); sgml2html telahelp.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --html telahelp.sgml

$(docdir)/telahelp.txt : $(docdir)/telahelp.sgml
	cd $(docdir); sgml2txt telahelp.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --txt telahelp.sgml

$(docdir)/telahelp.ps : $(docdir)/telahelp.dvi
	cd $(docdir); dvips telahelp.dvi -o telahelp.ps


$(docdir)/telafuncs.sgml : $(CTELA_MODULES) $(LOCAL_CTELA_MODULES) $(srcdir)/VERSION
	cd $(srcdir);\
	./mkdocfile `echo $(CTELA_MODULES) $(LOCAL_CTELA_MODULES) | sed -e 's/\.o/\.ct/g'`;\
	mv telafuncs.sgml doc

$(docdir)/telafuncs.dvi : $(docdir)/telafuncs.sgml
	cd $(docdir); $(SGML2PS) --output=dvi telafuncs.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --dvi telafuncs.sgml

$(docdir)/telafuncs.ps : $(docdir)/telafuncs.dvi
	cd $(docdir);\
	dvips telafuncs.dvi -o telafuncs.ps

$(docdir)/telafuncs.html : $(docdir)/telafuncs.sgml
	cd $(docdir); sgml2html telafuncs.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --html telafuncs.sgml

$(docdir)/telafuncs.txt : $(docdir)/telafuncs.sgml
	cd $(docdir); sgml2txt telafuncs.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --txt telafuncs.sgml


$(docdir)/telafuncsSectioned.sgml : $(CTELA_MODULES) $(LOCAL_CTELA_MODULES) $(srcdir)/VERSION
	cd $(srcdir);\
	./mkdocfile -S `echo $(CTELA_MODULES) $(LOCAL_CTELA_MODULES) | sed -e 's/\.o/\.ct/g'`;\
	mv telafuncsSectioned.sgml doc

$(docdir)/telafuncsSectioned.dvi : $(docdir)/telafuncsSectioned.sgml
	cd $(docdir); $(SGML2PS) --output=dvi telafuncsSectioned.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --dvi telafuncsSectioned.sgml

$(docdir)/telafuncsSectioned.ps : $(docdir)/telafuncsSectioned.dvi
	cd $(docdir);\
	dvips telafuncsSectioned.dvi -o telafuncsSectioned.ps

$(docdir)/telafuncsSectioned.html : $(docdir)/telafuncsSectioned.sgml
	cd $(docdir); sgml2html telafuncsSectioned.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --html telafuncsSectioned.sgml

$(docdir)/telafuncsSectioned.txt : $(docdir)/telafuncsSectioned.sgml
	cd $(docdir); sgml2txt telafuncsSectioned.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --txt telafuncsSectioned.sgml


$(docdir)/usrguide.dvi : $(docdir)/usrguide.sgml
	cd $(docdir); $(SGML2PS) --output=dvi usrguide.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --dvi usrguide.sgml

$(docdir)/usrguide.html : $(docdir)/usrguide.sgml
	cd $(docdir); sgml2html usrguide.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --html usrguide.sgml

$(docdir)/usrguide.txt : $(docdir)/usrguide.sgml
	cd $(docdir); sgml2txt usrguide.sgml

#	cd $(docdir); $(srcdir)/sgmlconv --txt usrguide.sgml

$(docdir)/usrguide.ps : $(docdir)/usrguide.dvi
	cd $(docdir); dvips usrguide.dvi -o usrguide.ps


# ----------------------------------------------------------------
# ---- Installation

install: ctpp tela telakka installdocs
	if [ ! -d $(INSTALL_BINDIR) ]; then mkdir -p $(INSTALL_BINDIR); chmod 755 $(INSTALL_BINDIR); fi
	if [ ! -d $(INSTALL_LIBDIR) ]; then mkdir -p $(INSTALL_LIBDIR); chmod 755 $(INSTALL_LIBDIR); fi
	cd $(INSTALL_LIBDIR); if [ ! -d t ]; then mkdir t; chmod 755 t; fi;\
	if [ ! -d ct ]; then mkdir ct; chmod 755 ct; fi;\
	if [ ! -d include ]; then mkdir include; chmod 755 include; fi;\
	if [ ! -d include/templ ]; then mkdir include/templ; chmod 755 include/templ; fi
	$(INSTALL_PROGRAM) ctpp $(INSTALL_BINDIR)
	$(INSTALL_PROGRAM) tela $(INSTALL_BINDIR)
	sed -e 's@^BINDIR=.*$$@BINDIR=$(INSTALL_BINDIR)@' \
		-e 's@^INCLUDEDIR=.*$$@INCLUDEDIR=$(INSTALL_LIBDIR)/include@' \
		<telakka >telakka.1;\
	chmod +x telakka.1
	$(INSTALL_PROGRAM) telakka.1 $(INSTALL_BINDIR)/telakka;\
	rm telakka.1
	cd $(srcdir); for i in *.ct local/*.ct; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR)/ct; done
	cd $(srcdir)/opt; for i in *.ct; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR)/ct; done
	cd $(srcdir)/t; for i in *.t *.hdf; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR)/t; done
	cd $(srcdir); for i in *.H *.h; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR)/include; done
	cd $(srcdir)/templ; for i in *.H; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR)/include/templ; done
	cd $(srcdir); $(INSTALL_PROGRAM) telahelp.sh $(INSTALL_LIBDIR)/t
	for i in libtela.a $(CTELA_MODULES) $(NEEDED_BY_CTELA_MODULES) \
		$(LOCAL_CTELA_MODULES) $(NEEDED_BY_LOCAL_CTELA_MODULES); \
		do $(INSTALL_DATA) $$i $(INSTALL_BINDIR); done
#	if [ -n "$(FFTPACKLIB)" ]; then $(INSTALL_DATA) $(srcdir)/$(FFTPACKLIB) $(INSTALL_BINDIR)/libfftpack.a; fi

installdocs:
	if [ ! -d $(INSTALL_LIBDIR) ]; then mkdir -p $(INSTALL_LIBDIR); chmod 755 $(INSTALL_LIBDIR); fi
	cd $(docdir); for i in *.txt *.sgml *.dvi; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR); done
	if [ ! -d $(INSTALL_LIBDIR)/html ]; then mkdir $(INSTALL_LIBDIR)/html; chmod 755 $(INSTALL_LIBDIR)/html; fi
	cd $(docdir); for i in *.html; do $(INSTALL_DATA) $$i $(INSTALL_LIBDIR)/html; done

# ----------------------------------------------------------------
# ---- Distribution tarfile

dist:
	cd $(srcdir)/local;\
	cp makeinc makeinc.orig;\
	egrep -v '^include ' <makeinc.orig >makeinc
	version=`cat $(srcdir)/VERSION | tr -d '\012'`;\
	cd $(srcdir); tar cf - *.C *.H *.y *.l d.*.c *.h *.ct flex++-3.0.2 \
		alloca.c noreadline.c fftpack.c vecfftpack.c vranf.f \
		VERSION INSTALL README WARRANTY COPYING CREDITS CHANGES COPYRIGHT_HEADER \
		local/makeinc local/matlabeng.ct \
		templ t doc pars opt tela.el \
		m2t/*.l m2t/*.m m2t/Makefile \
		Makefile mkdocfile mkbinary telahelp.sh configure *.in install.sh \
		mpeg2encode/*.C mpeg2encode/*.H mpeg2encode/*.h mpeg2encode/Makefile \
		cray_fix.sh mkcclisting.c \
		>$${TMPDIR:=/tmp}/teladist$$$$.tar;\
	currdir=`pwd`;\
	cd $${TMPDIR}; mkdir tela-$${version};\
	cd tela-$${version};\
	tar xf ../teladist$$$$.tar;\
	rm -rf doc/graphex;\
	rm ../teladist$$$$.tar;\
	cd ..;\
	tar cf - tela-$${version} | $(GNUZIP) >$${currdir}/tela-$${version}.tar.gz;\
	rm -r tela-$${version}
	cd $(srcdir)/local; mv makeinc.orig makeinc

binary:
	$(srcdir)/mkbinary

# ----------------------------------------------------------------
# ---- Cleanup etc

clean :
	-rm -f *~ *.o *.V *.L config.cache config.status \
		core core.* libtela.a $(TSYGLIB) libmpeg2encode.a $(EXECUTABLES) *-.c *.log
	(cd flex++-3.0.2; rm -f *.o; rm -f flex++)

docclean :
	-cd $(srcdir)/doc;\
	rm -f telafuncs*.html telafuncs*.dvi telafuncs*.txt telahelp.dvi telahelp.txt \
		  telahelp.dvi telahelp*.html telafuncs.sgml telafuncsSectioned.sgml

veryclean : clean docclean
	(cd flex++-3.0.2; make clean)


