#ifdef __cplusplus
extern "C" {
#endif

#define T89C t89c_

extern void T89C(const int& iopt, const float parmod[10], const float& psi,
				 const float& x, const float& y, const float& z,
				 float& Bx, float& By, float& Bz);
	
/*
SUBROUTINE T89C (IOPT,PARMOD,PS,X,Y,Z,BX,BY,BZ)
*/

#ifdef __cplusplus
}
#endif
