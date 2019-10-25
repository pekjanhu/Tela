# Makefile for mpeg2encode
# - This Makefile not used by Tela (taken care by Tela's Makefile) (/Pj 16 Apr 2000)
# Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

#
# Disclaimer of Warranty
#
# These software programs are available to the user without any license fee or
# royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
# any and all warranties, whether express, implied, or statuary, including any
# implied warranties or merchantability or of fitness for a particular
# purpose.  In no event shall the copyright-holder be liable for any
# incidental, punitive, or consequential damages of any kind whatsoever
# arising from the use of these programs.
#
# This disclaimer of warranty extends to the user of these programs and user's
# customers, employees, agents, transferees, successors, and assigns.
#
# The MPEG Software Simulation Group does not represent or warrant that the
# programs furnished hereunder are free of infringement of any third-party
# patents.
#
# Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
# are subject to royalty fees to patent holders.  Many of these patents are
# general enough such that they are unavoidable regardless of implementation
# design.
#
#

#CXXFLAGS = -O1 -Wall
#CFLAGS = -g -Wall

OBJ = mpegintf.o conform.o putpic.o puthdr.o putmpg.o putvlc.o putbits.o motion.o predict.o writepic.o transfrm.o fdctref.o idct.o quantize.o ratectl.o stats.o

all: libmpeg2encode.a
#all: libmpeg2encode.a jpeg2mpeg

clean:
	rm -f *.o *~ core jpeg2mpeg libmpeg2encode.a

libmpeg2encode.a: $(OBJ)
	-rm -f libmpeg2encode.a
	ar cr libmpeg2encode.a $(OBJ)

jpeg2mpeg : jpeg2mpeg.o libmpeg2encode.a
	$(CXX) $(CXXFLAGS) -o jpeg2mpeg jpeg2mpeg.o libmpeg2encode.a

jpeg2mpeg.o : jpeg2mpeg.C mpegintf.H
mpegintf.o : mpegintf.C mpegintf.H global.h
conform.o : conform.C mpegintf.H global.h 
fdctref.o : fdctref.C 
idct.o : idct.C 
motion.o : motion.C mpegintf.H global.h 
predict.o : predict.C mpegintf.H global.h 
putbits.o : putbits.C 
puthdr.o : puthdr.C mpegintf.H global.h 
putmpg.o : putmpg.C mpegintf.H global.h 
putpic.o : putpic.C mpegintf.H global.h 
putvlc.o : putvlc.C mpegintf.H global.h
quantize.o : quantize.C mpegintf.H global.h 
ratectl.o : ratectl.C mpegintf.H global.h 
stats.o : stats.C mpegintf.H global.h 
transfrm.o : transfrm.C mpegintf.H global.h 
writepic.o : writepic.C mpegintf.H global.h 

