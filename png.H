#ifndef PNG_H

#ifdef __GNUC__
#  pragma interface
#endif

#include <iostream>
#include <fstream>
#if HAVE_STDINT_H	// added 20 Dec 2007: get uint32_t
#  include <stdint.h>
#endif
#include "image.H"

// added 20 Dec 2007, and replaced everywhere "unsigned long" by CRCINT below in this file and in png.C
#if HAVE_STDINT_H
#  define CRCINT uint32_t
#else
#  define CRCINT unsigned long
#endif

class Tchunk {
private:
	int len;			// number of bytes in chunk's data field
	char type[5];		// 4-character type (type[4]==0 always)
	unsigned char *data;	// the data field, if len==0, data==0 also
	CRCINT read_uint4(istream& in);
	bool write_uint4(ostream& out, CRCINT x) const;
	static CRCINT crc(unsigned char *buf, unsigned char *typecode, int len);
	static CRCINT update_crc(CRCINT crc, unsigned char *buf, int len);
	static CRCINT crc_table[256];
	static bool crc_table_computed;
	static void make_crc_table();
public:
	Tchunk();
	Tchunk(const char *typestring, int chunklen);
	bool read(istream& in);
	bool write(ostream& out) const;
	int length() const {return len;}
	const char *typestr() const {return type;}
	unsigned char dataref(int i) const {return data[i];}
	void dataset(int i, unsigned char x) {data[i] = x;}
	unsigned char *data_addr() {return data;}
	unsigned int uint4ref(int i) const {return (data[i] << 24) | (data[i+1] << 16) | (data[i+2] << 8) | data[i+3];}
	void uint4set(int i, CRCINT x) {
		data[i] = (x >> 24) & 0xFF;
		data[i+1] = (x >> 16) & 0xFF;
		data[i+2] = (x >> 8) & 0xFF;
		data[i+3] = x & 0xFF;
	}
	friend ostream& operator<<(ostream& o, const Tchunk& c);
	~Tchunk() {if (data) delete [] data;}
};

extern bool pngcheckheader(istream& in);		// read 8 bytes and check if PNG magic signature
extern bool mngcheckheader(istream& in);		// read 8 bytes and check if MNG magic signature
extern bool pngwriteheader(ostream& out);		// write PNG file header (eight magic bytes)
extern bool mngwriteheader(ostream& out);		// write MNG file header (eight magic bytes)
extern bool pngread(istream& in, Timage& image);	// read PNG chunks IHDR...IEND (file header must be already read)
extern bool pngwrite(ostream& out, const Timage& image, int colortype, int compression); // write gray PNG image (IHDR+IDAT+IEND)
extern bool mngreadfirst(istream& in, Timage& image, int& w, int& h);
extern bool mngwritefirst(ostream& out, const Timage& image);
extern bool mngreadnext(istream& in, int w, int h, Timage& image);
extern bool mngwritenext(ostream& out, const Timage& image);
extern bool mngwriteendmark(ostream& out);

inline bool pngwrite_g(ostream& out, const Timage& image, int compression)
{
	return pngwrite(out,image,0,compression);
}

inline bool pngwrite_rgb(ostream& out, const Timage& image, int compression)
{
	return pngwrite(out,image,2,compression);
}

inline bool pngwrite_rgba(ostream& out, const Timage& image, int compression)
{
	return pngwrite(out,image,6,compression);
}

inline bool pngwrite_ga(ostream& out, const Timage& image, int compression)
{
	return pngwrite(out,image,4,compression);
}

#define PNG_H

#endif
