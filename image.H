#ifndef IMAGE_H

#ifdef __GNUC__
#  pragma interface
#endif

#if defined(__i386) || defined(__alpha)
/* Little-endian machine */
#  define RED_MASK    0x000000ff
#  define GREEN_MASK  0x0000ff00
#  define BLUE_MASK   0x00ff0000
#  define ALPHA_MASK  0xff000000
#  define RED_SHIFT   0
#  define GREEN_SHIFT 8
#  define BLUE_SHIFT  16
#  define ALPHA_SHIFT 24
#else
/* Big-endian machine */
#  define RED_MASK    0xff000000
#  define GREEN_MASK  0x00ff0000
#  define BLUE_MASK   0x0000ff00
#  define ALPHA_MASK  0x000000ff
#  define RED_SHIFT   24
#  define GREEN_SHIFT 16
#  define BLUE_SHIFT  8
#  define ALPHA_SHIFT 0
#endif

struct TImagePixel {
	unsigned short r,g,b,a;
};

class Timage {
private:
	int w,h;
	TImagePixel *ptr;		// rgba
	int flatindex(int i, int j) const {return i*w+j;}
	void free();
	void from(const Timage& X);
	void from(int W, int H);
//	void setwithmask(int i, int j, unsigned int mask, unsigned int val) {
//		const int p = flatindex(i,j);
//		ptr[p]|= (mask & val);
//		ptr[p]&= (~mask | val);
//	}
public:
	Timage() : w(0), h(0), ptr(0) {}
	Timage(int W, int H) {from(W,H);}
	Timage(const Timage& X) {from(X);}
	Timage& operator=(const Timage& X) {free(); from(X); return *this;}
	void alloc(int W, int H) {free(); from(W,H);}
//	unsigned int rgba(int i, int j) const {const int p=flatindex(i,j); return ptr[p];}
	void set_rgba(int i, int j, unsigned int rgba) {
		const int p = flatindex(i,j);
//		ptr[p] = rgba;
		ptr[p].r = (rgba & RED_MASK)   >> RED_SHIFT;
		ptr[p].g = (rgba & GREEN_MASK) >> GREEN_SHIFT;
		ptr[p].b = (rgba & BLUE_MASK)  >> BLUE_SHIFT;
		ptr[p].a = (rgba & ALPHA_MASK) >> ALPHA_SHIFT;
	}
	int width() const {return w;}
	int height() const {return h;}
//	unsigned int red(int i, int j) const {return (ptr[flatindex(i,j)] & RED_MASK) >> RED_SHIFT;}
//	unsigned int green(int i, int j) const {return (ptr[flatindex(i,j)] & GREEN_MASK) >> GREEN_SHIFT;}
//	unsigned int blue(int i, int j) const {return (ptr[flatindex(i,j)] & BLUE_MASK) >> BLUE_SHIFT;}
//	unsigned int alpha(int i, int j) const {return (ptr[flatindex(i,j)] & ALPHA_MASK) >> ALPHA_SHIFT;}
	unsigned int red(int i, int j) const {return ptr[flatindex(i,j)].r;}
	unsigned int green(int i, int j) const {return ptr[flatindex(i,j)].g;}
	unsigned int blue(int i, int j) const {return ptr[flatindex(i,j)].b;}
	unsigned int alpha(int i, int j) const {return ptr[flatindex(i,j)].a;}
//	void set_red(int i, int j, int red) {setwithmask(i,j,RED_MASK,red << RED_SHIFT);}
//	void set_green(int i, int j, int green) {setwithmask(i,j,GREEN_MASK,green << GREEN_SHIFT);}
//	void set_blue(int i, int j, int blue) {setwithmask(i,j,BLUE_MASK,blue << BLUE_SHIFT);}
//	void set_alpha(int i, int j, int alpha) {setwithmask(i,j,ALPHA_MASK,alpha << ALPHA_SHIFT);}
	void set_red(int i, int j, int red) {ptr[flatindex(i,j)].r = red;}
	void set_green(int i, int j, int green) {ptr[flatindex(i,j)].g = green;}
	void set_blue(int i, int j, int blue) {ptr[flatindex(i,j)].b = blue;}
	void set_alpha(int i, int j, int alpha) {ptr[flatindex(i,j)].a = alpha;}
//	unsigned int *rgbadata() {return ptr;}
	void xv() const;
	~Timage() {free();}
};

#define IMAGE_H

#endif
