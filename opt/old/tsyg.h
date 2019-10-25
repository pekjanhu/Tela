
typedef struct {
	double x;
	double y;
	double z;
} vector3;

extern void tsyg89(const vector3 r, vector3 *B);
extern void mainfield(const vector3 r, vector3 *B);
extern void m2i(double x, double y, double *theta, double *phi);
extern void i2m(double theta, double phi, double *x, double *y);
extern void setIOPT(double ioptNew);

extern void MapToIonosphere
	(const double x, const double y, const double z, double *theta, double *phi);

extern void MapToMagnetosphere
	(const double theta, const double phi, double *x, double *y, double *z,
	 int (*overnesstest)(const double [3]),
	 int (*closenesstest)(const double [3]));
