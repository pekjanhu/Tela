package
global(SphericalToCartesian_deg,CartesianToSpherical_deg,CartesianToSpherical_vec,
	   IGRF,T89c,T89,geotomag,geotomagXYZ,magtogeoXYZ,magtogeo,
	   gsmtogseXYZ,gsetogsmXYZ,geotogsmXYZ,gsmtogeoXYZ,m2i)
{

/*
 * This file is part of tela the Tensor Language.
 * Copyright (c) 1994-2003 Pekka Janhunen
 */

function [x,y,z] = SphericalToCartesian_deg(r,theta,phi)
/* [x,y,z] = SphericalToCartesian_deg(r,theta,phi)
   is a simple utility function to transform spherical
   coordinates to Cartesian. r is the radial distance,
   theta is the colatitude in degrees and phi is the longitude
   (azimuth angle), also in degrees.
   See also: CartesianToSpherica_deg.*/
{
	th = theta*(pi/180);
	ph = phi*(pi/180);
	sinth = sin(th);
	x = r*sinth*cos(ph);
	y = r*sinth*sin(ph);
	z = r*cos(th);
};

function [r,theta,phi] = CartesianToSpherical_deg(x,y,z)
/* [r,theta,phi] = CartesianToSpherical_deg(x,y,z)
   is a simple utility function to transform Cartesian
   coordinates to spherical. r is the radial distance,
   theta is the colatitude in degrees and phi is the longitude
   (azimuth angle), also in degrees.
   See also: SphericalToCartesian_deg.*/
{
	r = sqrt(x^2 + y^2 + z^2);
	th = acos(z/r);
	ph = atan2(y,x);
	theta = (180/pi)*th;
	phi = (180/pi)*ph;
};

function [Br,Btheta,Bphi] = CartesianToSpherical_vec(x,y,z,Bx,By,Bz)
{
	rho2 = x^2 + y^2;
	r = sqrt(rho2 + z^2);
	rho = sqrt(rho2);
	cphi = where(rho==0,1.0,x/rho);
	sphi = where(rho==0,0.0,y/rho);
	ct = z/r;
	st = rho/r;
	Br = (x*Bx + y*By + z*Bz)/r;
	Btheta = (Bx*cphi + By*sphi)*ct - Bz*st;
	Bphi = By*cphi - Bx*sphi;
};

function [Br,Btheta,Bphi] = IGRF(year,r,theta,phi)
/* [Br,Btheta,Bphi] = IGRF(year,r,theta,phi)
   computes the IGRF model magnetic field using 10 coefficients.
   Inputs:
     year   Integer Year number, from 1965 to 1990
     r      Radial coordinate of the point, in Earth radii
     theta  Geographic colatitude of the point, in degrees
     phi    Geographic longitude of the point, in degrees
   Outputs:
     Br,Btheta,Bphi: Magnetic field in geographic spherical coordinates
     in nanotesla.
   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.*/
{
	epoch = computeEPOCH(year,6,1,0,0,0,0);
	[xGEO,yGEO,zGEO] = SphericalToCartesian_deg(r,theta,phi);
	[xGSM,yGSM,zGSM] = geopack_coordtrans("GEOGSM",xGEO,yGEO,zGEO,epoch);
	[BxGSM,ByGSM,BzGSM] = geopack_model("IGRF",zeros(0),xGSM,yGSM,zGSM,epoch);
	[BxGEO,ByGEO,BzGEO] = geopack_coordtrans("GSMGEO",BxGSM,ByGSM,BzGSM,epoch);
	[Br,Btheta,Bphi] = CartesianToSpherical_vec(xGEO,yGEO,zGEO,BxGEO,ByGEO,BzGEO);
};

function [Bx,By,Bz] = T89c(Kp,x,y,z;psi)
/* [Bx,By,Bz] = T89c(Kp,x,y,z) computes the corrected/updated
   Tsyganenko-89 model magnetic field.
   Inputs: Kp, the Kp index; x,y,z, GSM coordinates in Earth radii.
   Outputs: total magnetic field in nanotesla in GSM coordinates.

   [Bx,By,Bz] = T89c(Kp,x,y,z,psi) defines the dipole tilt angle psi
   in degrees, default 0 (no tilt). Positive tilt corresponds to a situation
   where the magnetic north pole is closer to the Sun than the south pole.

   T89c is synonym for T89.

   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.

   See also: geopack_trace, m2i, geotomag, geotogsmXYZ.
*/
{
	[Bx,By,Bz] = geopack_model("T89",#(Kp),x,y,z,0.0,psi*pi/180);
};

function [Bx,By,Bz] = T89(Kp,x,y,z;psi)
/* [Bx,By,Bz] = T89(Kp,x,y,z) computes the corrected/updated
   Tsyganenko-89 model magnetic field.
   Inputs: Kp, the Kp index; x,y,z, GSM coordinates in Earth radii.
   Outputs: total magnetic field in nanotesla in GSM coordinates.

   [Bx,By,Bz] = T89(Kp,x,y,z,psi) defines the dipole tilt angle psi
   in degrees, default 0 (no tilt). Positive tilt corresponds to a situation
   where the magnetic north pole is closer to the Sun than the south pole.

   T89 is synonym for T89c.

   See also: geopack_model, geopack_trace, m2i, geotomag, geotogsmXYZ.
*/
{
	[Bx,By,Bz] = geopack_model("T89",#(Kp),x,y,z,0.0,psi*pi/180);
};

function [latmag,longmag] = geotomag(lat,longit)
/* [latmag,longmag] = geotomag(lat,long) transforms geographic
   latitude and longitude to geomagnetic dipole latitude and longitude.
   All angles in degrees.

   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.

   See also: geopack_coordtrans, geotomagXYZ, magtogeo, geotogsmXYZ, gsmtogeoXYZ.
*/
{
	[x,y,z] = SphericalToCartesian_deg(1.0,90-lat,longit);
	[xMAG,yMAG,zMAG] = geopack_coordtrans("GEOMAG",x,y,z,computeEPOCH(2000,1,1,0,0,0,0));
	[r,theta,phi] = CartesianToSpherical_deg(xMAG,yMAG,zMAG);
	latmag = 90 - theta;
	longmag = phi;
};

 function [x1,y1,z1] = geotomagXYZ(x,y,z)
 /* [x1,y1,z1] = geotomagXYZ(x,y,z) transforms geographic
	point (x,y,z) to geomagnetic dipole coordinates (x1,y1,z1).
    This function is obsolete and may be removed in further releases.
    Use the more general and powerful functions geopack_coordtrans and
    geopack_model.
	See also: geopack_coordtrans, geotomag, magtogeoXYZ.
*/
{
	[x1,y1,z1] = geopack_coordtrans("GEOMAG",x,y,z,computeEPOCH(2000,1,1,0,0,0,0));
};

function [x1,y1,z1] = magtogeoXYZ(x,y,z)
/* [x1,y1,z1] = magtogeoXYZ(x,y,z) transforms geomagnetic dipole
   point (x,y,z) to geographic coordinates (x1,y1,z1).
   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.
   See also: geopack_coordtrans, magtogeo, geotomagXYZ.
*/
{
	[x1,y1,z1] = geopack_coordtrans("MAGGEO",x,y,z,computeEPOCH(2000,1,1,0,0,0,0));
};

function [lat,longit] = magtogeo(latmag,longmag)
/* [lat,long] = magtogeo(latmag,longmag) transforms geomagnetic
   latitude and longitude to geographic latitude and longitude.
   All angles in degrees.
   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.
   See also: geopack_coordtrans, geotomag, magtogeoXYZ, geotogsmXYZ, gsmtogeoXYZ.
*/
{
	[x,y,z] = SphericalToCartesian_deg(1.0,90-latmag,longmag);
	[xGEO,yGEO,zGEO] = geopack_coordtrans("MAGGEO",x,y,z,computeEPOCH(2000,1,1,0,0,0,0));
	[r,theta,phi] = CartesianToSpherical_deg(xGEO,yGEO,zGEO);
	lat = 90 - theta;
	longit = phi;
};

function [x1,y1,z1] = gsmtogseXYZ(x,y,z,year,month,day,hour,min,sec)
/* [x1,y1,z1] = gsmtogse(x,y,z, year,month,day,hour,min,sec)
   transform given point (x,y,z) in GSM coordinates into
   the corresponding point (x1,y1,z1) in GSE coordinates.
   The year,month,day,hour,min,sec must be given as integers.
   Year must contain four digits; month,day start from 1.
   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.
   See also: geopack_coordtrans, gsetogsmXYZ.
*/
{
	[x1,y1,z1] = geopack_coordtrans("GSMGSE",x,y,z,computeEPOCH(year,month,day,hour,min,sec,0));
};

function [x1,y1,z1] = gsetogsmXYZ(x,y,z,year,month,day,hour,min,sec)
/* [x1,y1,z1] = gsetogsmXYZ(x,y,z, year,month,day,hour,min,sec)
   transform given point (x,y,z) in GSE coordinates into
   the corresponding point (x1,y1,z1) in GSM coordinates.
   The year,month,day,hour,min,sec must be given as integers.
   Year must contain four digits; month,day start from 1.

   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.

   See also: geopack_coordtrans, gsmtogseXYZ.
*/
{
	[x1,y1,z1] = geopack_coordtrans("GSEGSM",x,y,z,computeEPOCH(year,month,day,hour,min,sec,0));
};

function [x1,y1,z1] = geotogsmXYZ(x,y,z,year,month,day,hour,min,sec)
/* [x1,y1,z1] = geotogsmXYZ(x,y,z, year,month,day,hour,min,sec)
   transform given point (x,y,z) in Cartesian GEOgraphic coordinates into
   the corresponding point (x1,y1,z1) in GSM coordinates.
   The year,month,day,hour,min,sec must be given as integers.
   Year must contain four digits; month,day start from 1.

   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.

   See also: geopack_coordtrans, gsmtogeoXYZ.
*/
{
	[x1,y1,z1] = geopack_coordtrans("GEOGSM",x,y,z,computeEPOCH(year,month,day,hour,min,sec,0));
};

function [x1,y1,z1] = gsmtogeoXYZ(x,y,z,year,month,day,hour,min,sec)
/* [x1,y1,z1] = gsmtogeoXYZ(x,y,z, year,month,day,hour,min,sec)
   transform given point (x,y,z) in Cartesian GSM coordinates into
   the corresponding point (x1,y1,z1) in Cartesian GEOgraphic coordinates.
   The year,day,hour,min,sec must be given as integers.
   Year must contain four digits; month,day start from 1.

   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans and
   geopack_model.
   
   See also: geopack_coordtrans, geotogsmXYZ.
*/
{
	[x1,y1,z1] = geopack_coordtrans("GSMGEO",x,y,z,computeEPOCH(year,month,day,hour,min,sec,0));
};

function [theta,phi] = m2i(Kp,x,y)
/* [theta,phi] = m2i(Kp,x,y) maps the equatorial plane point (x,y,z=0)
   to the northern ionosphere. Inputs: Kp, the Kp index; x,y,z, GSM
   coordinates in Earth radii. Outputs: colatitude and longitude in
   magnetic coordinates in degrees. The T89 model (corrected version, T89C)
   is used in mapping, with zero dipole tilt angle.
   
   This function is obsolete and may be removed in further releases.
   Use the more general and powerful functions geopack_coordtrans
   geopack_model and geopack_trace.

   See also: geopack_coordtrans, geopack_trace, T89, geotomag.
*/
{
	epoch = computeEPOCH(2000,1,1,0,0,0,0);
	[xi,yi,zi] = geopack_trace("DIP,T89",#(Kp),"R=1.017",x,y,0,epoch,"tiltangle",0,"sign",1);
	[r,theta,phi] = CartesianToSpherical_deg(xi,yi,zi);
};

}
