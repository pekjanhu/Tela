#ifdef UNICOS
   extern "C" double RANF();						/* from system library; scalar version */
   extern "C" void RANSET(int*);					/* from system library */
   extern "C" void VRANF(double a[], const int *n);	/* from vranf.f */
#  define Random() RANF()
#elif HAVE_DRAND48
#  include <math.h>
   extern "C" double drand48();
   extern "C" void srand48(long);
#  define Random() drand48()
#else
#  define Random() (rand()/(1.0+RAND_MAX))
#endif

