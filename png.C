#ifdef __GNUC__
#  pragma implementation
#endif

#include <cstring>
#include <cmath>
#include <zlib.h>
#include <cstdlib>
using namespace std;
#include "png.H"
#include "def.H"
#if USE_STRINGSTREAM
#  include <sstream>
#else
#  include <strstream.h>
#endif

CRCINT Tchunk::crc_table[256];
bool Tchunk::crc_table_computed = false;

//inline int min(int x, int y) {return (x < y) ? x : y;}

Tchunk::Tchunk()
	: len(0), data(0)
{
	int i; for (i=0; i<5; i++) type[i] = 0;
	if (!crc_table_computed) make_crc_table();
}

Tchunk::Tchunk(const char *typestring, int chunklen)
	: len(chunklen), data(0)
{
	strncpy(type,typestring,4);
	type[4] = '\0';
	if (len > 0) data = new unsigned char [len];
	if (!crc_table_computed) make_crc_table();
}

ostream& operator<<(ostream& o, const Tchunk& c)
{
	o << "'" << c.type << "', len=" << c.len;
	return o;
}

CRCINT Tchunk::read_uint4(istream& in)
{
	unsigned char ch1,ch2,ch3,ch4;
	ch1 = in.get();
	ch2 = in.get();
	ch3 = in.get();
	ch4 = in.get();
	return (ch1 << 24) | (ch2 << 16) | (ch3 << 8) | ch4;
}

bool Tchunk::write_uint4(ostream& out, CRCINT x) const
{
	unsigned char ch1,ch2,ch3,ch4;
	ch1 = (x >> 24) & 0xFF;
	ch2 = (x >> 16) & 0xFF;
	ch3 = (x >> 8) & 0xFF;
	ch4 = x & 0xFF;
	out.put(ch1);
	out.put(ch2);
	out.put(ch3);
	out.put(ch4);
	return out.good();
}

void Tchunk::make_crc_table()
{
	CRCINT c;
	int n,k;
	for (n=0; n<256; n++) {
		c = static_cast<CRCINT>(n);
		for (k=0; k<8; k++) {
			if (c & 1)
				c = 0xedb88320UL ^ (c >> 1);
			else
				c = (c >> 1);
		}
		crc_table[n] = c;
	}
	crc_table_computed = true;
}

CRCINT Tchunk::update_crc(CRCINT crc, unsigned char *buf, int len)
{
	CRCINT c = crc;
	int n;
	for (n=0; n<len; n++) {
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}

CRCINT Tchunk::crc(unsigned char *buf, unsigned char *typecode, int len)
{
	CRCINT c = 0xffffffffUL;
	c = update_crc(c,typecode,4);
	if (buf) c = update_crc(c,buf,len);
	return c ^ 0xffffffffUL;
}

bool Tchunk::read(istream& in)
{
	len = read_uint4(in);
	if (in.eof()) return false;
	int i;
	char ch;
	for (i=0; i<4; i++) {in.get(ch); type[i] = ch;}
	if (data) {delete [] data; data = 0;}
	if (len > 0) {
		data = new unsigned char [len];
		in.read( reinterpret_cast<char*>(data),len);
	}
	const CRCINT c = read_uint4(in);
	const CRCINT c_calc = crc(data,(unsigned char *)type,len);
	if (c_calc != c) cerr << "*** crc error, crcinfile=" << std::hex << c << ", crc=" << std::hex << c_calc << "\n";
	return true;
}

bool Tchunk::write(ostream& out) const
{
	if (!write_uint4(out,len)) return false;
	int i;
	char ch;
	for (i=0; i<4; i++) {ch = type[i]; if (!out.put(ch)) return false;}
	if (len > 0) {
		out.write( reinterpret_cast<char*>(data),len);
		if (!out.good()) return false;
	}
	const CRCINT crcvalue = crc(data,(unsigned char *)type,len);
	if (!write_uint4(out,crcvalue)) return false;
	return true;
}

static bool expectchunk(istream& in, const char *type, Tchunk& c, bool ignore_palette=false)
{
	while (1) {
		if (!c.read(in)) return false;
		if (!strcmp(c.typestr(),type)) {
			return true;
		} else if (c.typestr()[0] & 32) {
//			cerr << "expectchunk " << type << ": passed ancillary chunk " << c.typestr() << "\n";
		} else if (ignore_palette && !strcmp(c.typestr(),"PLTE")) {
//			cerr << "expectchunk " << type << ": passed ancillary palette chunk\n";
		}
	}
	return false;
}

static bool Uncompress(unsigned char *out, int& outlen, const char *in, int inlen)
// uncompresses in[0..inlen-1] into out[0..outlen-1]
// in png applications, you know outlen already beforehand from IHDR info
{
	uLongf destLen = outlen;
	const int zret = uncompress((Bytef *)out,&destLen,(const Bytef *)in,inlen);
	outlen = destLen;
	return (zret == Z_OK);
}

static bool Compress(unsigned char *out, int& outlen, const unsigned char *in, int inlen, int level)
{
	uLongf destLen = outlen;
//	const int level = Z_DEFAULT_COMPRESSION;	// Z_DEFAULT_COMPRESSION, or number 0..9 (0 gives no compression)
	const int zret = compress2((Bytef *)out, &destLen, (const Bytef *)in, inlen, level);
	outlen = destLen;
	if (zret == Z_MEM_ERROR) {
		cerr << "*** Out of memory in Compress\n";
	} else if (zret == Z_BUF_ERROR) {
		cerr << "*** Output buffer too short in Compress\n";
	} else if (zret == Z_STREAM_ERROR) {
		cerr << "*** Invalid compression level " << level << " in Compress\n";
	}
	return (zret == Z_OK);
}

typedef short int paethint;

inline paethint PaethPredictor(paethint a, paethint b, paethint c)
{
	// a=left, b=above, c=upper left
	const paethint p = a + b - c;		// initial estimate
	const paethint pa = abs(p-a);		// distances to a,b,c
	const paethint pb = abs(p-b);
	const paethint pc = abs(p-c);
	// return nearest of a,b,c
	// breaking ties in order a,b,c
	if (pa <= pb && pa <= pc)
		return a;
	else if (pb <= pc)
		return b;
	else
		return c;
}

static void DecodeFilter(const int filtertype, unsigned char u[], int N, int bpp, const unsigned char *prev)
// filtertype is one of 0,1,2,3,4 (None,Sub,Up,Average,Paeth),
// u is scanline to be decoded, N is number of bytes on scanline (NOTE not number of pixels!),
// bpp is bytes per pixel, prev is previous scanline.
// If prev is null pointer, assume all zeros on previous scanline
{
	int i;
	if (filtertype == 0) {
		// None
		;
	} else if (filtertype == 1) {
		// Sub
		for (i=bpp; i<N; i++) u[i]+= u[i-bpp];
	} else if (filtertype == 2) {
		// Up
		if (prev) {
			for (i=0; i<N; i++) u[i]+= prev[i];
		}
	} else if (filtertype == 3) {
		// Average
		if (prev) {
			for (i=0; i<min(bpp,N); i++) u[i]+= (prev[i] >> 1);
			for (i=bpp; i<N; i++) u[i]+= ((int(u[i-bpp]) + int(prev[i])) >> 1);
		} else {
			for (i=bpp; i<N; i++) u[i]+= (u[i-bpp] >> 1);
		}
	} else if (filtertype == 4) {
		// Paeth
		if (prev) {
			const int max_i = min(bpp,N);
			for (i=0; i<max_i; i++) u[i]+= PaethPredictor(0,prev[i],0);
			for (i=bpp; i<N; i++) u[i]+= PaethPredictor(u[i-bpp],prev[i],prev[i-bpp]);
		} else {
			for (i=bpp; i<N; i++) u[i]+= PaethPredictor(u[i-bpp],0,0);
		}
	}
}

static void EncodeFilter(const int filtertype, unsigned char out[], int N, int bpp,
						 const unsigned char *u, const unsigned char *prev)
// filtertype is one of 0,1,2,3,4 (None,Sub,Up,Average,Paeth),
// out is filtered scanline, N is number of bytes on scanline (NOTE not number of pixels!),
// bpp is bytes per pixel, u is scanline to be encoded (input),
// prev is previous scanline.
// If prev is null pointer, assume all zeros on previous scanline
{
	int i;
	if (filtertype == 0) {
		// None
		for (i=0; i<N; i++) out[i] = u[i];
	} else if (filtertype == 1) {
		// Sub
		const int max_i = min(bpp,N);
		for (i=0; i<max_i; i++) out[i] = u[i];
		for (i=bpp; i<N; i++) out[i] = u[i] - u[i-bpp];
	} else if (filtertype == 2) {
		// Up
		if (prev) {
			for (i=0; i<N; i++) out[i] = u[i] - prev[i];
		} else {
			for (i=0; i<N; i++) out[i] = u[i];
		}
	} else if (filtertype == 3) {
		// Average
		if (prev) {
			const int max_i = min(bpp,N);
			for (i=0; i<max_i; i++) out[i] = u[i] - (prev[i] >> 1);
			for (i=bpp; i<N; i++) out[i] = u[i] - ((int(u[i-bpp]) + int(prev[i])) >> 1);
		} else {
			for (i=bpp; i<N; i++) out[i] = u[i] - (u[i-bpp] >> 1);
		}
	} else if (filtertype == 4) {
		// Paeth
		if (prev) {
			const int max_i = min(bpp,N);
			for (i=0; i<max_i; i++) out[i] = u[i] - PaethPredictor(0,prev[i],0);
			for (i=bpp; i<N; i++) out[i] = u[i] - PaethPredictor(u[i-bpp],prev[i],prev[i-bpp]);
		} else {
			for (i=bpp; i<N; i++) out[i] = u[i] - PaethPredictor(u[i-bpp],0,0);
		}
	}
}

bool pngread(istream& in, Timage& image)
{
	Tchunk ihdr;
	if (!ihdr.read(in) || strcmp(ihdr.typestr(),"IHDR") || ihdr.length() != 13) {
		if (strcmp(ihdr.typestr(),"MEND"))
			cout << "pngread failed, ihdr.length() = " << ihdr.length() << ", str=" << ihdr.typestr() << "\n";
		return false;
	}
	const int w = ihdr.uint4ref(0);
	const int h = ihdr.uint4ref(4);
//	cout << "pngread: w=" << w << ", h=" << h << "\n";
	if (w == 0 || h == 0) return false;
	const int bitdepth = ihdr.dataref(8);
	if (bitdepth != 1 && bitdepth != 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16) return false;
	const int colortype = ihdr.dataref(9);
	if (colortype != 0 && colortype != 2 && colortype != 3 && colortype != 4 && colortype != 6) return false;
	int bufflen=0,bytesperline=0;
	bool musthavepalette=false;
	bool mayhavepalette=false;
	int bytesperpixel = 0;
	if (colortype == 0) {
		// each pixel a grayscale sample, possible bitdepths 1,2,4,8,16
		bytesperpixel = int(ceil(bitdepth/8.0));
		bytesperline = int(ceil((bitdepth*w)/8.0));
	} else if (colortype == 2) {
		// each pixel an rgb triple, possible bitdepths 8,16
		bytesperpixel = 3*(bitdepth/8);
		bytesperline = 3*((bitdepth*w)/8);
		mayhavepalette = true;
	} else if (colortype == 3) {
		// each pixel a palette index, possible bitdepths 1,2,4,8
		musthavepalette = true;
		bytesperpixel = int(ceil(bitdepth/8.0));
		bytesperline = int(ceil((bitdepth*w)/8.0));
	} else if (colortype == 4) {
		// each pixel a grayscale and alpha sample, possible bitdepths 8,16
		bytesperpixel = 2*(bitdepth/8);
		bytesperline = 2*((bitdepth*w)/8);
	} else if (colortype == 6) {
		// each pixel rgba quadruple, possible bitdepths 8,16
		bytesperpixel = 4*(bitdepth/8);
		bytesperline = 4*((bitdepth*w)/8);
		mayhavepalette = true;
	}
	bufflen = bytesperline*h;
	bufflen+= h;		// all scanlines contain filter type byte
	const int compressionmethod = ihdr.dataref(10);
	if (compressionmethod != 0) return false;
	const int filtermethod = ihdr.dataref(11);
	if (filtermethod != 0) return false;
	const int interlacemethod = ihdr.dataref(12);
	if (interlacemethod != 0 && interlacemethod != 1) return false;
	if (interlacemethod == 1) {cerr << "interlaced PNG images not supported\n"; return false;}
	image.alloc(w,h);
	unsigned char pal[256][3];
	int pal_len = 0;
	memset(pal,0,3*256);
	if (musthavepalette) {
		Tchunk plte;
		if (!expectchunk(in,"PLTE",plte)) return false;
		if (plte.length() % 3 != 0) return false;
		pal_len = plte.length()/3;
		if (pal_len < 1 || pal_len > 256) return false;
		memcpy(&pal[0][0],plte.data_addr(),plte.length());
	}
//	cout << "width=" << w << ", height=" << h << ", colortype=" << colortype << ", bitdepth=" << bitdepth
//		 << ", interlacemethod=" << interlacemethod << "\n";
	unsigned char *buff = new unsigned char [bufflen];
	Tchunk idat;
	if (!expectchunk(in,"IDAT",idat, true)) return false;
#	if USE_STRINGSTREAM
	stringstream S;
#	else
	strstream S;
#	endif
	int Slen = 0;
	S.write( reinterpret_cast<char*>(idat.data_addr()),idat.length());
	Slen+= idat.length();
//	cout << "Read IDAT of length " << idat.length() << "\n";
	while (1) {
		if (!idat.read(in)) return false;
		if (!strcmp(idat.typestr(),"IDAT")) {
			S.write(reinterpret_cast<char*>(idat.data_addr()),idat.length());
			Slen+= idat.length();
//			cout << "Read IDAT of length " << idat.length() << "\n";
		} else
			break;
	}
	if (strcmp(idat.typestr(),"IEND")) {
		if (!expectchunk(in,"IEND",idat)) return false;
	}
	S << ends;
#	if USE_STRINGSTREAM
	string charptr = S.str();
	if (!Uncompress(buff,bufflen,charptr.c_str(),Slen)) {
#	else
	char *charptr = S.str();
	if (!Uncompress(buff,bufflen,charptr,Slen)) {
#	endif
		cerr << "uncompress failed when reading PNG\n";
		return false;
	}
#	if !USE_STRINGSTREAM
	delete [] charptr;
#	endif
	// now buff contains the uncompressed raw image data with each scanline preceded by filter info byte
	unsigned char *filterinfos = new unsigned char [h];
	int i,j;
	const int W = bytesperline + 1;
	const int FullyOpaque1 = 1;
	const int FullyOpaque2 = 3;
	const int FullyOpaque4 = 15;
	const int FullyOpaque8 = 255;
	const int FullyOpaque16 = 65535;
//	cout << "bytesperline = " << bytesperline << "\n";
	for (i=0; i<h; i++) {
		filterinfos[i] = buff[W*i];
//		if (filterinfos[i] != 0) cerr << "warning: filterinfo=" << int(filterinfos[i]) << "\n";
		DecodeFilter(filterinfos[i],&buff[1+i*W],bytesperline,bytesperpixel, (i==0) ? 0 : &buff[1+(i-1)*W]);
		if (colortype == 0) {
			// each pixel a grayscale sample, possible bitdepths 1,2,4,8,16
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					const int u = buff[1+i*W+j];
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					image.set_alpha(i,j,FullyOpaque8);
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					const int u = int(buff[1+i*W+2*j])*256 + buff[1+i*W+2*j+1];
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					image.set_alpha(i,j,FullyOpaque16);
				}
			} else if (bitdepth == 4) {
				for (j=0; j<w; j++) {
					const int j1 = j/2;
					int u;
					if (j & 1) {
						u = buff[1+i*W+j1] & 0x0f;
					} else {
						u = (buff[1+i*W+j1] & 0xf0) >> 4;
					}
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					image.set_alpha(i,j,FullyOpaque4);
				}
			} else if (bitdepth == 2) {
				for (j=0; j<w; j++) {
					const int j1 = j/4;
					int u=0;
					switch (j & 3) {
					case 0:
						u = (buff[1+i*W+j1] & 0xc0) >> 6;
						break;
					case 1:
						u = (buff[1+i*W+j1] & 0x30) >> 4;
						break;
					case 2:
						u = (buff[1+i*W+j1] & 0x0c) >> 2;
						break;
					case 3:
						u = (buff[1+i*W+j1] & 0x03);
						break;
					}
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					image.set_alpha(i,j,FullyOpaque2);
				}
			} else if (bitdepth == 1) {
				for (j=0; j<w; j++) {
					const int j1 = j/8;
					const int u = (buff[1+i*W+j1] >> (7-(j % 8))) & 1;
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					image.set_alpha(i,j,FullyOpaque1);
				}
			} else {
				cerr << "unsupported PNG colortype=0, bitdepth=" << bitdepth << "\n";
				return false;
			}
		} else if (colortype == 2) {
			// each pixel an rgb triple, possible bitdepths 8,16
			if (bitdepth == 8) {
				const int iW1 = i*W + 1;
				for (j=0; j<w; j++) {
					const int p = iW1+3*j;
					image.set_rgba(i,j,
								   (buff[p] << RED_SHIFT) | (buff[p+1] << GREEN_SHIFT)
								   | (buff[p+2] << BLUE_SHIFT) | FullyOpaque8 << ALPHA_SHIFT);
					/*
					image.set_red(i,j,buff[1+i*W+3*j]);
					image.set_green(i,j,buff[1+i*W+3*j+1]);
					image.set_blue(i,j,buff[1+i*W+3*j+2]);
					image.set_alpha(i,j,FullyOpaque8);
					*/
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					image.set_red  (i,j,int(buff[1+i*W+6*j+0])*256 + buff[1+i*W+6*j+1]);
					image.set_green(i,j,int(buff[1+i*W+6*j+2])*256 + buff[1+i*W+6*j+3]);
					image.set_blue (i,j,int(buff[1+i*W+6*j+4])*256 + buff[1+i*W+6*j+5]);
					image.set_alpha(i,j,FullyOpaque16);
				}
			} else {
				cerr << "unsupported PNG colortype=2, bitdepth=" << bitdepth << "\n";
				return false;
			}
		} else if (colortype == 3) {
			// each pixel a palette index, possible bitdepths 1,2,4,8
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					const int k = buff[1+i*W+j];
					if (k < 0 || k >= pal_len) continue;
					image.set_red(i,j,pal[k][0]);
					image.set_green(i,j,pal[k][1]);
					image.set_blue(i,j,pal[k][2]);
					image.set_alpha(i,j,FullyOpaque8);
				}
			} else if (bitdepth == 4) {
				for (j=0; j<w; j++) {
					const int j1 = j/2;
					int k;
					if (j & 1) {
						k = buff[1+i*W+j1] & 0x0f;
					} else {
						k = (buff[1+i*W+j1] & 0xf0) >> 4;
					}
					if (k < 0 || k >= pal_len) continue;
					image.set_red(i,j,pal[k][0]);
					image.set_green(i,j,pal[k][1]);
					image.set_blue(i,j,pal[k][2]);
					image.set_alpha(i,j,FullyOpaque4);
				}
			} else if (bitdepth == 2) {
				for (j=0; j<w; j++) {
					const int j1 = j/4;
					int k=0;
					switch (j & 3) {
					case 0:
						k = (buff[1+i*W+j1] & 0xc0) >> 6;
						break;
					case 1:
						k = (buff[1+i*W+j1] & 0x30) >> 4;
						break;
					case 2:
						k = (buff[1+i*W+j1] & 0x0c) >> 2;
						break;
					case 3:
						k = (buff[1+i*W+j1] & 0x03);
						break;
					}
					if (k < 0 || k >= pal_len) continue;
					image.set_red(i,j,pal[k][0]);
					image.set_green(i,j,pal[k][1]);
					image.set_blue(i,j,pal[k][2]);
					image.set_alpha(i,j,FullyOpaque2);
				}
			} else if (bitdepth == 1) {
				for (j=0; j<w; j++) {
					const int j1 = j/8;
					const int k = (buff[1+i*W+j1] >> (7-(j % 8))) & 1;
					if (k < 0 || k >= pal_len) continue;
					image.set_red(i,j,pal[k][0]);
					image.set_green(i,j,pal[k][1]);
					image.set_blue(i,j,pal[k][2]);
					image.set_alpha(i,j,FullyOpaque1);
				}
			} else {
				// this shouldn't happen, but check anyway
				cerr << "unsupported PNG colortype=3, bitdepth=" << bitdepth << "\n";
				return false;
			}
		} else if (colortype == 4) {
			// each pixel a grayscale and alpha sample, possible bitdepths 8,16
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					const int u = buff[1+i*W+2*j];
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					image.set_alpha(i,j,buff[1+i*W+2*j+1]);
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					const int u = int(buff[1+i*W+4*j])*256 + buff[1+i*W+4*j+1];
					image.set_red(i,j,u);
					image.set_green(i,j,u);
					image.set_blue(i,j,u);
					const int alp = int(buff[1+i*W+4*j+2])*256 + buff[1+i*W+4*j+3];
					image.set_alpha(i,j,alp);
				}
			} else {
				cerr << "unsupported PNG colortype=4, bitdepth=" << bitdepth << "\n";
			}
		} else if (colortype == 6) {
			// each pixel rgba quadruple, possible bitdepths 8,16
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					image.set_red(i,j,buff[1+i*W+4*j]);
					image.set_green(i,j,buff[1+i*W+4*j+1]);
					image.set_blue(i,j,buff[1+i*W+4*j+2]);
					image.set_alpha(i,j,buff[1+i*W+4*j+3]);
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					image.set_red  (i,j,int(buff[1+i*W+8*j+0])*256 + buff[1+i*W+8*j+1]);
					image.set_green(i,j,int(buff[1+i*W+8*j+2])*256 + buff[1+i*W+8*j+3]);
					image.set_blue (i,j,int(buff[1+i*W+8*j+4])*256 + buff[1+i*W+8*j+5]);
					image.set_alpha(i,j,int(buff[1+i*W+8*j+6])*256 + buff[1+i*W+8*j+7]);
				}
			} else {
				cerr << "unsupported PNG colortype=6, bitdepth=" << bitdepth << "\n";
			}
		}
	}
	delete [] filterinfos;
	delete [] buff;
	return true;
}

bool pngwrite(ostream& out, const Timage& image, int colortype, int compression)
{
	// write IHDR chunk
	Tchunk ihdr("IHDR",13);
	const int w = image.width();
	const int h = image.height();
	ihdr.uint4set(0,w);
	ihdr.uint4set(4,h);
	// colortype=0: gray, 2:rgb, 4:gray-alpha, 6:rgba
	int i,j;
	unsigned int maxval = 0;
	int valuesperpixel = 1;
	if (colortype==0) {
		// colortype=0 is grayscale image
		valuesperpixel = 1;
		for (i=0; i<h; i++) for (j=0; j<w; j++)
			if (image.red(i,j) > maxval) maxval = image.red(i,j);
	} else if (colortype == 2) {
		// colortype=2 is RGB image
		valuesperpixel = 3;
		for (i=0; i<h; i++) for (j=0; j<w; j++) {
			if (image.red(i,j)   > maxval) maxval = image.red(i,j);
			if (image.green(i,j) > maxval) maxval = image.green(i,j);
			if (image.blue(i,j)  > maxval) maxval = image.blue(i,j);
		}
	} else if (colortype == 4) {
		// colortype==4 is gray-alpha image
		valuesperpixel = 2;
		for (i=0; i<h; i++) for (j=0; j<w; j++) {
			if (image.red(i,j)   > maxval) maxval = image.red(i,j);
			if (image.alpha(i,j) > maxval) maxval = image.alpha(i,j);
		}
	} else if (colortype == 6) {
		// colortype=6 is RGBA image
		valuesperpixel = 4;
		for (i=0; i<h; i++) for (j=0; j<w; j++) {
			if (image.red(i,j)   > maxval) maxval = image.red(i,j);
			if (image.green(i,j) > maxval) maxval = image.green(i,j);
			if (image.blue(i,j)  > maxval) maxval = image.blue(i,j);
			if (image.alpha(i,j) > maxval) maxval = image.alpha(i,j);
		}
	}
	const int bitdepth = (maxval > 255) ? 16 : 8;
	ihdr.dataset(8,bitdepth);
	ihdr.dataset(9,colortype);
	ihdr.dataset(10,0);		// compressionmethod must always be zero
	ihdr.dataset(11,0);		// filtermethod must always be zero
	ihdr.dataset(12,0);		// interlacemethod==0: no interlace
	if (!ihdr.write(out)) return false;
	// header chunk written
	const int bytesperpixel = valuesperpixel*(bitdepth/8);
	const int bytesperline = valuesperpixel*((bitdepth*w)/8);
	const int inputbufflen = (bytesperline+1)*h;
	int outputbufflen = int(ceil(inputbufflen*1.002)) + 14;
	// prepare filtered data in inputbuff
	unsigned char *inputbuff = new unsigned char [inputbufflen];
	const int W = bytesperline + 1;
	unsigned char *scanline = new unsigned char [7*bytesperline];
	unsigned char *prev_scanline = scanline + bytesperline;
	unsigned char *filtered[5];
	int f;
	int prev_best_filter = 0;
	for (f=0; f<5; f++) filtered[f] = scanline + (2+f)*bytesperline;
	memset(prev_scanline,0,bytesperline);
	for (i=0; i<h; i++) {
		const int iW1 = i*W + 1;
		if (colortype == 0) {
			// each pixel grayscale value
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					int val = image.red(i,j);
					if (val > 255) val = 255;
					scanline[j] = val;
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					const int p = 2*j;
					scanline[p  ] = (image.red(i,j) & 0xff00) >> 8;
					scanline[p+1] = image.red(i,j) & 0x00ff;
				}
			}
		} else if (colortype == 2) {
			// each pixel an rgb triple, possible bitdepths 8,16
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					const int p = 3*j;
					int val;
					val = image.red(i,j);
					if (val > 255) val = 255;
					scanline[p] = val;
					val = image.green(i,j);
					if (val > 255) val = 255;
					scanline[p+1] = val;
					val = image.blue(i,j);
					if (val > 255) val = 255;
					scanline[p+2] = val;
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					const int p = 6*j;
					scanline[p  ] = (image.red(i,j) & 0xff00) >> 8;
					scanline[p+1] = image.red(i,j) & 0x00ff;
					scanline[p+2] = (image.green(i,j) & 0xff00) >> 8;
					scanline[p+3] = image.green(i,j) & 0x00ff;
					scanline[p+4] = (image.blue(i,j) & 0xff00) >> 8;
					scanline[p+5] = image.blue(i,j) & 0x00ff;
				}
			}
		} else if (colortype == 4) {
			// each pixel grayscale-alpha value
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					const int p = 2*j;
					int val;
					val = image.red(i,j);
					if (val > 255) val = 255;
					scanline[p] = val;
					val = image.alpha(i,j);
					if (val > 255) val = 255;
					scanline[p+1] = val;
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					const int p = 4*j;
					scanline[p  ] = (image.red(i,j) & 0xff00) >> 8;
					scanline[p+1] = image.red(i,j) & 0x00ff;
					scanline[p+2] = (image.alpha(i,j) & 0xff00) >> 8;
					scanline[p+3] = image.alpha(i,j) & 0x00ff;
				}
			}
		} else if (colortype == 6) {
			// each pixel an rgba quadruple, possible bitdepths 8,16
			if (bitdepth == 8) {
				for (j=0; j<w; j++) {
					const int p = 4*j;
					scanline[p  ] = image.red(i,j);
					scanline[p+1] = image.green(i,j);
					scanline[p+2] = image.blue(i,j);
					scanline[p+3] = image.alpha(i,j);
				}
			} else if (bitdepth == 16) {
				for (j=0; j<w; j++) {
					const int p = 8*j;
					scanline[p  ] = (image.red(i,j) & 0xff00) >> 8;
					scanline[p+1] = image.red(i,j) & 0x00ff;
					scanline[p+2] = (image.green(i,j) & 0xff00) >> 8;
					scanline[p+3] = image.green(i,j) & 0x00ff;
					scanline[p+4] = (image.blue(i,j) & 0xff00) >> 8;
					scanline[p+5] = image.blue(i,j) & 0x00ff;
					scanline[p+6] = (image.alpha(i,j) & 0xff00) >> 8;
					scanline[p+7] = image.alpha(i,j) & 0x00ff;
				}
			}
		}
		for (f=0; f<5; f++) {
			EncodeFilter(f,filtered[f],bytesperline,bytesperpixel,scanline,prev_scanline);
		}
		unsigned int smallest_abssum = (1 << 30);
		int best_filter = 0;
		for (f=0; f<5; f++) {
			unsigned int M1 = 0, M2 = 0;
			for (j=0; j<bytesperline; j++) {
				M1+= abs((signed char)filtered[f][j]);
			}
			for (j=1; j<bytesperline; j++) {
				M2+= abs((signed char)filtered[f][j] - (signed char)filtered[f][j-1]);
			}
			unsigned int M = (M1 < M2) ? M1 : M2;
//			if (f == 2 && prev_best_filter == 2) M = (unsigned int)(M*0.1);
			if (M < smallest_abssum) {smallest_abssum = M; best_filter = f;}
		}
//		cout << " " << best_filter << "/" << smallest_abssum;
		memcpy(&inputbuff[W*i+1],filtered[best_filter],bytesperline);
		inputbuff[W*i] = best_filter;
		memcpy(prev_scanline,scanline,bytesperline);
		prev_best_filter = best_filter;
	}
	delete [] scanline;
	// inputbuff ready
	// compress inputbuff to outputbuff
	unsigned char *outputbuff = new unsigned char [outputbufflen];
	if (!Compress(outputbuff,outputbufflen, inputbuff,inputbufflen, compression)) {
		cerr << "PNG: compress failed\n";
		delete [] outputbuff;
		delete [] inputbuff;
		return false;
	}
    // write IDAT chunk
	Tchunk idat("IDAT",outputbufflen);
	for (i=0; i<outputbufflen; i++) idat.dataset(i,outputbuff[i]);
	delete [] inputbuff;
	if (!idat.write(out)) {
		delete [] outputbuff;
		return false;
	}
	delete [] outputbuff;
	// write IEND chunk
	Tchunk iend("IEND",0);
	if (!iend.write(out)) return false;
	return true;
}

bool pngwriteheader(ostream& out)
{
	out.put((unsigned char)137);
	out.put((unsigned char)80);
	out.put((unsigned char)78);
	out.put((unsigned char)71);
	out.put((unsigned char)13);
	out.put((unsigned char)10);
	out.put((unsigned char)26);
	out.put((unsigned char)10);
	return out.good();
}

bool mngwriteheader(ostream& out)
{
	out.put((unsigned char)138);
	out.put((unsigned char)77);
	out.put((unsigned char)78);
	out.put((unsigned char)71);
	out.put((unsigned char)13);
	out.put((unsigned char)10);
	out.put((unsigned char)26);
	out.put((unsigned char)10);
	return out.good();
}

bool pngcheckheader(istream& in)
// reads 8 bytes from in, returns true if they match PNG header, false otherwise
{
	unsigned char chs[8];
	int i;
	for (i=0; i<8; i++) chs[i] = in.get();
	if (!in.good()) return false;
	if (chs[0] != 137 || chs[1] != 80 || chs[2] != 78 || chs[3] != 71 ||
		chs[4] != 13 || chs[5] != 10 || chs[6] != 26 || chs[7] != 10) return false;
	return true;
}

bool mngcheckheader(istream& in)
// reads 8 bytes from in, returns true if they match MNG header, false otherwise
{
	unsigned char chs[8];
	int i;
	for (i=0; i<8; i++) chs[i] = in.get();
	if (!in.good()) return false;
	if (chs[0] != 138 || chs[1] != 77 || chs[2] != 78 || chs[3] != 71 ||
		chs[4] != 13 || chs[5] != 10 || chs[6] != 26 || chs[7] != 10) return false;
	return true;
}

bool mngreadfirst(istream& in, Timage& image, int& w, int& h)
{
	Tchunk mhdr;
	if (!mhdr.read(in) || strcmp(mhdr.typestr(),"MHDR") || mhdr.length() != 28) return false;
	w = mhdr.uint4ref(0);
	h = mhdr.uint4ref(4);
//	cout << "mngreadfirst: w=" << w << ", h=" << h << "\n";
	if (w == 0 || h == 0) return false;
	const unsigned int tickspersecond = mhdr.uint4ref(8);
	const unsigned int nominal_layercount = mhdr.uint4ref(12);
	const unsigned int nominal_framecount = mhdr.uint4ref(16);
	const unsigned int nominal_playtime = mhdr.uint4ref(20);
	const unsigned int simplicityprofile = mhdr.uint4ref(24);
	while (1) {
		Tchunk c;
		const streampos i = in.tellg();
		c.read(in);
		// exit loop if chunk c is not one of BACK,bGRD,bkGD
		if (strcmp(c.typestr(),"BACK") && strcmp(c.typestr(),"bGRD") && strcmp(c.typestr(),"bKGD")) {
			in.seekg(i);
			break;
		}
	}
	return pngread(in,image);
}

bool mngwritefirst(ostream& out, const Timage& image)
{
	const int compression = 6;
	Tchunk mhdr("MHDR",28);
//	cout << "mngwritefirst: width=" << image.width() << ", height=" << image.height() << "\n";
	mhdr.uint4set(0,image.width());
	mhdr.uint4set(4,image.height());
	mhdr.uint4set(8,1);					// ticks per second (1)
	mhdr.uint4set(12,0);				// nominal layer count (0=unspecified)
	mhdr.uint4set(16,0);				// nominal frame count (0=unspecified)
	mhdr.uint4set(20,0);				// nominal play time (0=unspecified)
	mhdr.uint4set(24,1);				// simplicity profile
	if (!mhdr.write(out)) return false;
	return pngwrite_rgb(out,image,compression);
}

bool mngreadnext(istream& in, int w, int h, Timage& image)
{
	return pngread(in,image);
}

bool mngwritenext(ostream& out, const Timage& image)
{
	const int compression = 6;
	return pngwrite_rgb(out,image,compression);
}

bool mngwriteendmark(ostream& out)
{
	Tchunk mend("MEND",0);
	return mend.write(out);
}
