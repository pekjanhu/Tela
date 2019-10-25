epoch = computeEPOCH(1999,3,10,6,0,0,0);
//model = "DIP,T89"; params = #(3);
model = "DIP,T96"; params = #(2,0,5,0);

[x,y,z] = geopack_trace(model,params,"R=1",-13,0,0,epoch,"tiltangle",0,"maxstep",0.001);
[r,theta,phi] = CartesianToSpherical_deg(x,y,z);
ILAT = 90-theta;
format("ILAT=``\n",ILAT);
return;

/*
        Results (2 nPa, zero IMF, zero Dst):
  defaults (maxstep=0.3):   ILAT=67.8288     err=0.0088 (1 km)
  maxstep=0.1:              ILAT=67.8312     err=0.0064 (700 m)
  maxstep=0.01:             ILAT=67.8372     err=0.0004 (44 m)
  maxstep=0.005:            ILAT=67.8375     err=0.0001 (11 m)
  maxstep=0.002:            ILAT=67.8375     err=0.0001 (11 m)
  maxstep=0.001:            ILAT=67.8376

       Results (2 nPa, IMF By=5, zero Dst):
  defaults (maxstep=0.3)    ILAT=67.3765     err=0.0084 (924 m)
  maxstep=0.1:              ILAT=67.3783     err=0.0066 (726 m)
  maxstep=0.01:             ILAT=67.3845     err=0.0004 (44 m)
  maxstep=0.005:            ILAT=67.3848     err=0.0001 (11 m)
  maxstep=0.002:            ILAT=67.3849     err=0.0    (0 m)
  maxstep=0.001:            ILAT=67.3849
*/

hold(on);
for (x0=-6; x0>=-12; x0--) for (s=-1; s<=1; s+=2) {
//	format("x0=``, s=``\n",x0,s);
	[x,y,z,X,Y,Z] = geopack_trace(model,params,"R=1,R=30",x0,0,0,epoch,"sign",s,"tiltangle",0);
	plot(X,Z,"xflip","true","markertype",13,"equalscale","true");
//	plot3(X,Y,Z,"xflip","true","markertype",13,"axisscale","false","-3d");
};
hold(off);
return;
//[Bx,By,Bz] = geopack_model("IGRF,T96",#(2,0,5.,0),-8,0,0,epoch);
//#(Bx,By,Bz);
p1=perf();
[x,y,z,X,Y,Z] = geopack_trace("IGRF,T89",#(3),"R>1",-8,0,0,epoch,1);
//[x,y,z] = geopack_trace("IGRF,T96",#(2,0,0,0),"R>1",-8,0,0,epoch,1);
p2=perf();
format("`` seconds, value=``\n",cputime(p2-p1),#(x,y,z));
plot(X,Z,"xflip","true");

