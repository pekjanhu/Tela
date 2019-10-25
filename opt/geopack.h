#ifdef __cplusplus
extern "C" {
#endif

/*
  Coordinate systems: GSM,GSE,GEO,GEI,MAG,SM (6)
  Available transformations: GEOMAG,GEIGEO,MAGSM,GSMGSE,SMGSM,GEOGSM (6)

  GSM/GSE: directly, GSM/GEO: directly, GSM/GEI: via GEO, GSM/MAG: via GEO, GSM/SM: directly
  GSE/GEO: via GSM, GSE/GEI: via GSM and GEO, GSE/MAG: via GSM and SM, GSE/SM: via GSM
  GEO/GEI: directly, GEO/MAG: directly, GEO/SM: via GSM
  GEI/MAG: via GEO, GEI/SM: via GEO and GSM
  MAG/SM: directly

     GEI --- GEO --- GSM --- GSE
	          |      |
			 MAG --- SM
 */

#define IGRF_GSM igrf_gsm_
#define IGRF_GEO igrf_geo_
#define DIP dip_
#define RECALC recalc_
#define GEOMAG geomag_
#define GEIGEO geigeo_
#define MAGSM magsm_
#define GSMGSE gsmgse_
#define SMGSM smgsm_
#define GEOGSM geogsm_
#define SETPSI setpsi_
	
extern void IGRF_GSM(const float& xgsm, const float& ygsm, const float& zgsm,
					 float& Bxgsm, float& Bygsm, float& Bzgsm);
/*
  Calculates components of the main (internal) geomagnetic field in the geocentric solar
  magnetospheric coordinate system, using IAGA international geomagnetic reference model
  coefficients (e.g., http://www.ngdc.noaa.gov/IAGA/wg8/igrf2000.html).

  Before the first call of this subroutine, or if the date/time (iyear,iday,ihour,min,isec)
  was changed, the model coefficients and GEO-GSM rotation matrix elements should be updated
  by calling the subroutine RECALC.

-----Input parameters:
     xgsm,ygsm,zgsm - Cartesian GSM coordinates (in units RE=6371.2 km)
-----Output parameters:
     Bxgsm,Bygsm,Bzgsm - Cartesian GSM components of the main geomagnetic field in nanotesla
*/

extern void IGRF_GEO(const float& R, const float& theta, const float& phi,
					 float& Br, float& Btheta, float& Bphi);
/*
  Calculates components of the main (internal) geomagnetic field in the spherical geographic
  (geocentric) coordinate system, using IAGA international geomagnetic reference model
  coefficients (e.g., http://www.ngdc.noaa.gov/iaga/wg8/igrf2000.html)

  Before the first call of this subroutine, or if the date (iyear and iday) was changed,
  the model coefficients should be updated by calling the subroutine RECALC.

Input parameters:
   R, theta, phi - spherical geographic (geocentric) coordinates:
   radial distance r in units re=6371.2 km, colatitude theta and longitude phi in radians
Output parameters:
     Br, Btheta, Bphi - spherical components of the main geomagnetic field in nanotesla
     (positive br outward, btheta southward, bphi eastward)
*/

extern void DIP(const float& xgsm, const float& ygsm, const float& zgsm,
				float& Bxgsm, float& Bygsm, float& Bzgsm);
/*
  Calculates GSM components of a geodipole field with the dipole moment
  corresponding to the epoch, specified by calling subroutine RECALC (should be
  invoked before the first use of this one and in case the date/time was changed).

Input parameters: xgsm,ygsm,zgsm - gsm coordinates in RE (1 RE = 6371.2 km)
Output parameters: bxgsm,bygsm,bzgsm - field components in GSM system, in nanotesla.
*/

extern void RECALC(const int& yyyy, const int& ddd, const int& hh, const int& minute, const int& ss,
				   const float& vgsex, const float& vgsey, const float& vgsez);
/*
  1. Prepares elements of rotation matrices for transformations of vectors between
     several coordinate systems, most frequently used in space physics.
  2. Prepares coefficients used in the calculation of the main geomagnetic field
     (IGRF model).
  This subroutine should be invoked before using the following subroutines:
  igrf_geo, igrf_gsm, dip, geomag, geogsm, magsm, smgsm, gsmgse, geigeo.

  There is no need to repeatedly invoke recalc, if multiple calculations are made
  for the same date and time.
Input parameters:
     yyyy   -  year number (four digits)
     ddd    -  day of year (day 1 = jan 1)
     hh     -  hour of day (00 to 23)
     minute -  minute of hour (00 to 59)
     ss     -  seconds of minute (00 to 59)
	 vgsex  -  GSE X component of solar wind velocity vector
	 vgsey  -  GSE Y component of solar wind velocity vector
	 vgsez  -  GSE Z component of solar wind velocity vector
*/

extern void GEOMAG(float& xgeo, float& ygeo, float& zgeo,
				   float& xmag, float& ymag, float& zmag, const int& j);
/*
    Converts geographic (GEO) to dipole (MAG) coordinates or vica versa.
             j>0                       j<0
Input:   j,xgeo,ygeo,zgeo         j,xmag,ymag,zmag
Output:  xmag,ymag,zmag           xgeo,ygeo,zgeo

  Attention:  Subroutine  RECALC  must be invoked before GEOMAG in two cases:
     /a/  Before the first transformation of coordinates.
     /b/  If the values of yyyy and/or ddd have been changed.
*/

extern void GEIGEO(float& xgei, float& ygei, float& zgei, float& xgeo, float& ygeo, float& zgeo, const int& j);
/*
   Converts equatorial inertial (GEI) to geographical (GEO) coords
   or vica versa.
                j>0               j<0
Input:   j,xgei,ygei,zgei    j,xgeo,ygeo,zgeo
Output:  xgeo,ygeo,zgeo      xgei,ygei,zgei

   Attention:  Subroutine RECALC must be invoked before geigeo in two cases:
     /a/  Before the first transformation of coordinates.
     /b/  If the current values of iyear,iday,ihour,min,isec have been changed.
*/

extern void MAGSM(float& xmag, float& ymag, float& zmag, float& xsm, float& ysm, float& zsm, const int& j);
/*
  Converts dipole (MAG) to solar magnetic (SM) coordinates or vica versa.
                j>0              j<0
Input: j,xmag,ymag,zmag     j,xsm,ysm,zsm
Output:    xsm,ysm,zsm       xmag,ymag,zmag

  Attention:  Subroutine RECALC must be invoked before magsm in two cases:
     /a/  Before the first transformation of coordinates.
     /b/  If the values of iyear,iday,ihour,min,isec have been changed.
*/

extern void GSMGSE(float& xgsm, float& ygsm, float& zgsm, float& xgse, float& ygse, float& zgse, const int& j);
/*
 Converts geocentric solar magnetospheric (GSM) coords to solar ecliptic (GSE) ones
 or vica versa.
                j>0                j<0
Input:    j,xgsm,ygsm,zgsm    j,xgse,ygse,zgse
Output:   xgse,ygse,zgse      xgsm,ygsm,zgsm

  Attention: Subroutine RECALC must be invoked before gsmgse in two cases:
     /a/  Before the first transformation of coordinates.
     /b/  If the values of iyear,iday,ihour,min,isec have been changed.
*/

extern void SMGSM(float& xsm, float& ysm, float& zsm, float& xgsm, float& ygsm, float& zgsm, const int& j);
/*
 Converts solar magnetic (SM) to geocentric solar magnetospheric
 (GSM) coordinates or vica versa.
             j>0                 j<0
Input:   j,xsm,ysm,zsm        j,xgsm,ygsm,zgsm
Output:  xgsm,ygsm,zgsm       xsm,ysm,zsm

  Attention: Subroutine RECALC must be invoked before SMGSM in two cases:
     /a/  Before the first transformation of coordinates.
     /b/  If the values of iyear,iday,ihour,min,isec have been changed.
*/

extern void GEOGSM(float& xgeo, float& ygeo, float& zgeo,
				   float& xgsm, float& ygsm, float& zgsm, const int& j);
/*
 Converts geographic (GEO) to geocentric solar magnetospheric (GSM) coordinates
 or vica versa.

               j>0                   j<0
Input:   j,xgeo,ygeo,zgeo    j,xgsm,ygsm,zgsm
Output:  xgsm,ygsm,zgsm      xgeo,ygeo,zgeo

  Attention: Subroutine RECALC must be invoked before geogsm in two cases:
     /a/  Before the first transformation of coordinates.
     /b/  If the values of iyear,iday,ihour,min,isec  have been changed.
*/

extern void SETPSI(const float& ps);
	
#ifdef __cplusplus
}
#endif
