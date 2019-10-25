#include <stdio.h>

/* class MPEG writes MPEG-2 animations
   Based on mpeg2encode v.1.2 96/07/19, C++ version by PJ 2000/04/16
   class MPEG has quite simple public interface, see bottom of file...
*/

class MPEG {
#if 1 /*def __sgi*/
public:
#else
private:
#endif
	// frame buffer to store a few recent frames, needed because the algorithm adds frames not in sequential order
	class TFrameBuffer {
	private:
		typedef unsigned char *TUcharPtr;
		unsigned char **buff;		// M x 3 matrix (M == maxframes)
		int *npixvec;				// vector storing npixels values
		int max_frames;				// maximum number of frames in the frame buffer
		int nframes_now;			// current number of frames in the buffer
		int topframe_index;			// index of the most recently added frame (-1 if no frame yet added)
	public:
		TFrameBuffer(int m) : max_frames(m), nframes_now(0), topframe_index(-1) {
			buff = new TUcharPtr [max_frames*3];
			npixvec = new int [max_frames];
		}
		void add(const unsigned char r[], const unsigned char g[], const unsigned char b[], int npixels);
		int Nframes() const {return nframes_now;}
		bool getframe(int frame, unsigned char *&r, unsigned char *&g, unsigned char *&b, int& npixels) const;
		~TFrameBuffer();
	};
	
    // macroblock information
	struct Tmbinfo {
		int mb_type; /* intra/forward/backward/interpolated */
		int motion_type; /* frame/field/16x8/dual_prime */
		int dct_type; /* field/frame DCT */
		int mquant; /* quantization parameter */
		int cbp; /* coded block pattern */
		int skipped; /* skipped macroblock */
		int MV[2][2][2]; /* motion vectors */
		int mv_field_sel[2][2]; /* motion vertical field select */
		int dmvector[2]; /* dual prime vectors */
		double act; /* activity measure */
		int var; /* for debugging */
	};

    // motion data
	struct Tmotion_data {
		int forw_hor_f_code,forw_vert_f_code; /* vector range */
		int sxf,syf; /* search range */
		int back_hor_f_code,back_vert_f_code;
		int sxb,syb;
	};
	
    // type definitions for variable length code table entries
	struct VLCtable {
		unsigned char code; /* right justified */
		char len;
	};

    // for codes longer than 8 bits (excluding leading zeroes)
	struct sVLCtable {
		unsigned short code; /* right justified */
		char len;
	};

	// conform.C
	void range_checks();
	void profile_and_level_checks();

	// fdctref.C
	void init_fdct(void);
	void fdct(short *block);

	// idct.C
	void idct(short *block);
	void init_idct();

	// motion.C
	void motion_estimation(unsigned char *oldorg, unsigned char *neworg,
						   unsigned char *oldref, unsigned char *newref, unsigned char *cur,
						   unsigned char *curref, int sxf, int syf, int sxb, int syb,
						   Tmbinfo *mbi, int secondfield, int ipflag);

	// mpegintf.C
	void error(char *text);

	// predict.C
	void predict(unsigned char *reff[], unsigned char *refb[],
				 unsigned char *cur[3], int secondfield, Tmbinfo *mbi);

	// putbits.C
	void initbits();
	void putbits(int val, int n);
	void alignbits();
	int bitcount();

	// puthdr.C
	void putseqhdr();
	void putseqext();
	void putseqdispext();
	void putuserdata(char *userdata);
	void putgophdr(int frame, int closed_gop);
	void putpicthdr();
	void putpictcodext();
	void putseqend();

	// putmpg.C
	void putintrablk(short *blk, int cc);
	void putnonintrablk(short *blk);
	void putmv(int dmv, int f_code);

	// putpic.C
	void putpict(unsigned char *frame);

	// putvlc.C
	void putDClum(int val);
	void putDCchrom(int val);
	void putACfirst(int run, int val);
	void putAC(int run, int signed_level, int vlcformat);
	void putaddrinc(int addrinc);
	void putmbtype(int pict_type, int mb_type);
	void putmotioncode(int motion_code);
	void putdmv(int dmv);
	void putcbp(int cbp);

	// quantize.C
	int quant_intra(short *src, short *dst, int dc_prec,
					unsigned char *quant_mat, int mquant);
	int quant_non_intra(short *src, short *dst,
						unsigned char *quant_mat, int mquant);
	void iquant_intra(short *src, short *dst, int dc_prec,
					  unsigned char *quant_mat, int mquant);
	void iquant_non_intra(short *src, short *dst,
						  unsigned char *quant_mat, int mquant);

	// ratectl.C
	void rc_init_seq();
	void rc_init_GOP(int np, int nb);
	void rc_init_pict(unsigned char *frame);
	void rc_update_pict();
	int rc_start_mb();
	int rc_calc_mquant(int j);
	void vbv_end_of_picture();
	void calc_vbv_delay();

	// stats.C
	void calcSNR(unsigned char *org[3], unsigned char *rec[3]);
	void stats();
	
	// transfrm.C
	void transform(unsigned char *pred[], unsigned char *cur[],
				   Tmbinfo *mbi, short blocks[][64]);
	void itransform(unsigned char *pred[], unsigned char *cur[],
					Tmbinfo *mbi, short blocks[][64]);
	void dct_type_estimation(unsigned char *pred, unsigned char *cur,
							 Tmbinfo *mbi);
	// writepic.C
	void writeframe(char *fname, unsigned char *frame[]);

	// global variables
	// ----------------

	TFrameBuffer FrameBuffer;
	
	static char version[];
	static char author[];

	// zig-zag scan
	static unsigned char zig_zag_scan[64];

	// alternate scan
	static unsigned char alternate_scan[64];

	// default intra quantization matrix
	static unsigned char default_intra_quantizer_matrix[64];

	// non-linear quantization coefficient table
	static unsigned char non_linear_mquant_table[32];

    /* non-linear mquant table for mapping from scale to code
	 * since reconstruction levels are not bijective with the index map,
	 * it is up to the designer to determine most of the quantization levels
	 */

	static unsigned char map_non_linear_mquant[113];

	// picture data arrays

	// reconstructed frames
	unsigned char *newrefframe[3], *oldrefframe[3], *auxframe[3];
	// original frames
	unsigned char *neworgframe[3], *oldorgframe[3], *auxorgframe[3];
	// prediction of current frame
	unsigned char *predframe[3];
    // 8*8 block data
	short (*blocks)[64];
    // intra / non_intra quantization matrices
	unsigned char intra_q[64], inter_q[64];
	unsigned char chrom_intra_q[64],chrom_inter_q[64];
    // prediction values for DCT coefficient (0,0)
	int dc_dct_pred[3];
    // macroblock side information array
	Tmbinfo *mbinfo;
    // motion estimation parameters
	Tmotion_data *motion_data;
    // clipping (=saturation) table
	unsigned char *clp;

    // name strings
	char id_string[256], tplref[256];
	char iqname[256], niqname[256];
	char statname[256];
	char errortext[256];

	FILE *outfile, *statfile; // file descriptors

	bool quiet; // suppress warnings

    // coding model parameters

	int N; // number of frames in Group of Pictures
	int M; // distance between I/P frames
	int P; // intra slice refresh interval
	int frame0, tc0; // number and timecode of first frame
	int currentframenumber;
	bool mpeg1; // ISO/IEC IS 11172-2 sequence
	bool fieldpic; // use field pictures

    // sequence specific data (sequence header)

	int horizontal_size, vertical_size; // frame size (pels)
	int width, height; // encoded frame size (pels) multiples of 16 or 32
	int chrom_width,chrom_height,block_count;
	int mb_width, mb_height; // frame size (macroblocks)
	int width2, height2, mb_height2, chrom_width2; // picture size
	int aspectratio; // aspect ratio information (pel or display)
	int frame_rate_code; // coded value of frame rate
	double frame_rate; // frames per second
	double bpf;	// bytes per frame
	double bit_rate; // bits per second
	int vbv_buffer_size; // size of VBV buffer (* 16 kbit)
	bool constrparms; // constrained parameters flag (MPEG-1 only)
	int load_iquant, load_niquant; // use non-default quant. matrices
	int load_ciquant,load_cniquant;

    // sequence specific data (sequence extension)

	int profile, level; // syntax / parameter constraints
	bool prog_seq; // progressive sequence
	int chroma_format;
	bool low_delay; // no B pictures, skipped pictures

    // sequence specific data (sequence display extension)
	int video_format; // component, PAL, NTSC, SECAM or MAC
	int color_primaries; // source primary chromaticity coordinates
	int transfer_characteristics; // opto-electronic transfer char. (gamma)
	int matrix_coefficients; // Eg,Eb,Er / Y,Cb,Cr matrix coefficients
	int display_horizontal_size, display_vertical_size; // display size

    // picture specific data (picture header)
	int temp_ref; // temporal reference
	int pict_type; // picture coding type (I, P or B)
	int vbv_delay; // video buffering verifier delay (1/90000 seconds)

    // picture specific data (picture coding extension)
	int forw_hor_f_code, forw_vert_f_code;
	int back_hor_f_code, back_vert_f_code; // motion vector ranges
	int dc_prec; // DC coefficient precision for intra coded blocks
	int pict_struct; // picture structure (frame, top / bottom field)
	bool topfirst; // display top field first
    // use only frame prediction and frame DCT (I,P,B,current)
	bool frame_pred_dct_tab[3];
	int frame_pred_dct;
	bool conceal_tab[3]; // use concealment motion vectors (I,P,B)
	bool qscale_tab[3];
	int q_scale_type; // linear/non-linear quantizaton table
	bool intravlc_tab[3];
	int intravlc; // intra vlc format (I,P,B,current)
	bool altscan_tab[3];
	int altscan; // alternate scan (I,P,B,current)
	bool repeatfirst; // repeat first field after second field
	bool prog_frame; // progressive frame

	// private auxiliary functions
	void init();
	void DefaultParameters();
	void CheckParameters(int h, int m, int s, int f);
	void readquantmat();
	void conv444to422(unsigned char *src, unsigned char *dst);
	void conv422to420(unsigned char *src, unsigned char *dst);
	void GetYUVFrame(const unsigned char red[], const unsigned char green[], const unsigned char blue[],
					 unsigned char *frame[]);
	// putpic.C
	void putmvs(int MV[2][2][2], int PMV[2][2][2],
				int mv_field_sel[2][2], int dmvector[2], int s, int motion_type,
				int hor_f_code, int vert_f_code);
	// puthdr.C
	int frametotc(int frame);
	// putvlc.C
	void putDC(sVLCtable *tab, int val);
	// motion.C
	void frame_ME(unsigned char *oldorg, unsigned char *neworg, unsigned char *oldref, unsigned char *newref,
				  unsigned char *cur,
				  int i, int j, int sxf, int syf, int sxb, int syb,
				  Tmbinfo *mbi);
	void field_ME(unsigned char *oldorg, unsigned char *neworg,
				  unsigned char *oldref, unsigned char *newref, unsigned char *cur,
				  unsigned char *curref, int i, int j, int sxf, int syf, int sxb, int syb,
				  Tmbinfo *mbi, int secondfield, int ipflag);
	void frame_estimate(unsigned char *org, unsigned char *ref, unsigned char *mb,
						int i, int j, int sx, int sy,
						int *iminp, int *jminp, int *imintp, int *jmintp,
						int *iminbp,int *jminbp,int *dframep,int *dfieldp,
						int *tselp, int *bselp,
						int imins[2][2], int jmins[2][2]);
	void field_estimate(unsigned char *toporg, unsigned char *topref,
						unsigned char *botorg, unsigned char *botref,
						unsigned char *mb,
						int i, int j, int sx, int sy,
						int ipflag,
						int *iminp, int *jminp, int *imin8up, int *jmin8up,
						int *imin8lp, int *jmin8lp,
						int *dfieldp, int *d8p,
						int *selp, int *sel8up, int *sel8lp,
						int *iminsp, int *jminsp, int *dsp);
	void dpframe_estimate(unsigned char *ref, unsigned char *mb,
						  int i, int j, int iminf[2][2], int jminf[2][2],
						  int *iminp, int *jminp,
						  int *imindmvp, int *jmindmvp,
						  int *dmcp, int *vmcp);
	void dpfield_estimate(unsigned char *topref, unsigned char *botref, unsigned char *mb,
						  int i, int j, int imins, int jmins,
						  int *imindmvp, int *jmindmvp, int *dmcp, int *vmcp);
	int fullsearch(unsigned char *org, unsigned char *ref, unsigned char *blk,
				   int lx, int i0, int j0, int sx, int sy, int h,
				   int xmax, int ymax,
				   int *iminp, int *jminp);
	// predict.C
	void predict_mb(unsigned char *oldref[], unsigned char *newref[], unsigned char *cur[],
					int lx, int bx, int by, int pict_type, int pict_struct,
					int mb_type, int motion_type,
					int secondfield, int PMV[2][2][2],
					int mv_field_sel[2][2],
					int dmvector[2]);
	void pred(unsigned char *src[], int sfield,
			  unsigned char *dst[], int dfield,
			  int lx, int w, int h, int x, int y,
			  int dx, int dy, int addflag);
	void calc_DMV(int DMV[][2], int *dmvector, int mvx, int mvy);
	void clearblock(unsigned char *cur[], int i0, int j0);
	// transfrm.C
	void add_pred(unsigned char *pred, unsigned char *cur,
				  int lx, short *blk);
	// ratectl.C
	void calc_actj(unsigned char *frame);

	// mpegintf.C
	bool try_to_add_frame();
public:
	// Public interface of class MPEG
	// ------------------------------

	// Construct a new MPEG stream attached to OutputFile, giving image dimensions (in units of pixel)
	MPEG(const char *OutputFile, int Width, int Height);

	void set_mpeg1(bool flag);
	void set_verbose(bool flag) {quiet=!flag;}
	void bytes_per_frame(double bpf1) {bpf = bpf1; bit_rate = frame_rate*8*bpf;}
	void frames_per_second(double fps) {frame_rate = fps; bit_rate = frame_rate*8*bpf;}
	void Iframes_only(bool flag);
	
	// Add one frame to the animation. The RGB pixel arrays must be given.
	// They must be matrices of type [Height][Width], i.e. width coordinate varies fastest
	void addframe(const unsigned char red[], const unsigned char green[], const unsigned char blue[]);

	// Finish writing the animation and close the MPEG stream
	~MPEG();

	// ------------------------------
};

