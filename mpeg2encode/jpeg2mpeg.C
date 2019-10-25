#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpegintf.H"

/* Simple test program only, don't use for anything serious */

static void usage()
{
	cerr << "usage: jpeg2mpeg *.jpg out.mpg\n";
}

int main(int argc, char *argv[])
{
	if (argc < 4) {usage(); exit(1);}
	const char *outfile = argv[argc-1];
	char s[256];
	sprintf(s,"djpeg %s | head -2 | tail -1",argv[1]);
	FILE *pp = popen(s,"r");
	if (!pp) {cerr << "*** Cannot open pipe to '" << s << "'\n"; exit(1);}
	int width,height;
	if (fscanf(pp,"%d%d",&width,&height) != 2) {
		cerr << "*** cannot obtain first JPEG file dimensions\n";
		exit(4);
	}
	pclose(pp);
	cout << "width=" << width << ", height=" << height << "\n";
	MPEG mpeg(outfile,width,height);
//	mpeg.bytes_per_frame(1e3);
	unsigned char *r = new unsigned char [width*height*3];
	unsigned char *g = r + width*height;
	unsigned char *b = r + 2*width*height;
	int a;
	for (a=1; a<argc-1; a++) {
		sprintf(s,"djpeg %s",argv[a]);
		FILE *pp = popen(s,"r");
		if (!pp) {cerr << "*** jpeg2mpeg: cannot open pipe '" << s << "'\n"; exit(6);}
		fgets(s,255,pp); fgets(s,255,pp);
		fgets(s,255,pp);
		int i,j;
		for (i=0; i<height; i++) for (j=0; j<width; j++) {
			const unsigned char ch = fgetc(pp);
			const int ii = i*width+j;
			r[ii] = g[ii] = b[ii] = ch;
			b[ii] = ch/2;
		}
		pclose(pp);
		mpeg.addframe(r,g,b);
	}
	return 0;
}
