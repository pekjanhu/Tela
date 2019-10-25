c/*
c * This file is part of tela the Tensor Language.
c * Copyright (c) 1994-1996 Pekka Janhunen
c */

c The Cray C compiler cannot vectorize RANF loops
c thus we need Fortran.

c This routine can be called from C++ as follows:
c extern "C" void VRANF(double a[], int *n)
c ...
      subroutine vranf(a,n)
      implicit none
      integer n
      real a(n)
      integer i
      do i=1,n
         a(i) = RANF()
      enddo
      return
      end
