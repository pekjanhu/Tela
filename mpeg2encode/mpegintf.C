#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpegintf.H"
#include "global.h"

char MPEG::version[] = "mpeg2encode V1.2, 96/07/19";
char MPEG::author[] = "(C) 1996, MPEG Software Simulation Group";
unsigned char MPEG::zig_zag_scan[64] = {
	0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
	12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
	35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63};

unsigned char MPEG::alternate_scan[64] = {
	0,8,16,24,1,9,2,10,17,25,32,40,48,56,57,49,
	41,33,26,18,3,11,4,12,19,27,34,42,50,58,35,43,
	51,59,20,28,5,13,6,14,21,29,36,44,52,60,37,45,
	53,61,22,30,7,15,23,31,38,46,54,62,39,47,55,63};

unsigned char MPEG::default_intra_quantizer_matrix[64] = {
	8, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83};

unsigned char MPEG::non_linear_mquant_table[32] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8,10,12,14,16,18,20,22,
	24,28,32,36,40,44,48,52,
	56,64,72,80,88,96,104,112};

unsigned char MPEG::map_non_linear_mquant[113] = {
	0,1,2,3,4,5,6,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,
	16,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,22,22,
	22,22,23,23,23,23,24,24,24,24,24,24,24,25,25,25,25,25,25,25,26,26,
	26,26,26,26,26,26,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,29,
	29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,31,31,31,31,31};


void MPEG::TFrameBuffer::add(const unsigned char r[], const unsigned char g[], const unsigned char b[], int npixels)
{
	int i,c,p;
	for (i=nframes_now-1; i>=0; i--) {
		if (i >= max_frames-1) {
			for (c=0; c<3; c++) delete [] buff[i*3+c];
		} else {
			for (c=0; c<3; c++) buff[(i+1)*3+c] = buff[i*3+c];
			npixvec[i+1] = npixvec[i];
		}
	}
	if (nframes_now < max_frames) nframes_now++;
	for (c=0; c<3; c++) buff[c] = new unsigned char [npixels];
	npixvec[0] = npixels;
	memcpy(buff[0],r,npixels*sizeof(unsigned char));
	memcpy(buff[1],g,npixels*sizeof(unsigned char));
	memcpy(buff[2],b,npixels*sizeof(unsigned char));
	topframe_index++;
}

bool MPEG::TFrameBuffer::getframe(int frame, unsigned char *&r, unsigned char *&g, unsigned char *&b, int& npixels) const
{
	if (topframe_index-nframes_now < frame && frame <= topframe_index) {
		const int i = topframe_index-frame;
		r = buff[i*3+0];
		g = buff[i*3+1];
		b = buff[i*3+2];
		npixels = npixvec[i];
		return true;
	} else
		return false;
}

MPEG::TFrameBuffer::~TFrameBuffer()
{
	int i;
	for (i=0; i<nframes_now; i++) {
		delete [] buff[i*3+0];		// r
		delete [] buff[i*3+1];		// g
		delete [] buff[i*3+2];		// b
	}
	delete [] buff;
	delete [] npixvec;
}

void MPEG::set_mpeg1(bool flag)
{
	mpeg1 = flag;
	if (mpeg1) {
		aspectratio = 8;
		prog_seq = true;
		prog_frame = true;
		dc_prec = 0;
		vbv_buffer_size = 20;
		qscale_tab[0] = false;
		qscale_tab[1] = false;
		qscale_tab[2] = false;
		intravlc_tab[0] = false;
		intravlc_tab[1] = false;
		intravlc_tab[2] = false;
		CheckParameters(0,0,0,0);
	}
}

void MPEG::Iframes_only(bool flag)
{
	if (flag) {
		N = M = 1;
		CheckParameters(0,0,0,0);
	} else {
		N = 12;
		M = 3;
		CheckParameters(0,0,0,0);
	}
}

void MPEG::init()
{
	int i, size;
	static int block_count_tab[3] = {6,8,12};

	initbits();
	init_fdct();
	init_idct();

	/* round picture dimensions to nearest multiple of 16 or 32 */
	mb_width = (horizontal_size+15)/16;
	mb_height = prog_seq ? (vertical_size+15)/16 : 2*((vertical_size+31)/32);
	mb_height2 = fieldpic ? mb_height>>1 : mb_height; /* for field pictures */
	width = 16*mb_width;
	height = 16*mb_height;

	chrom_width = (chroma_format==CHROMA444) ? width : width>>1;
	chrom_height = (chroma_format!=CHROMA420) ? height : height>>1;

	height2 = fieldpic ? height>>1 : height;
	width2 = fieldpic ? width<<1 : width;
	chrom_width2 = fieldpic ? chrom_width<<1 : chrom_width;
  
	block_count = block_count_tab[chroma_format-1];

	/* clip table */
	if (!(clp = (unsigned char *)malloc(1024)))
		error("malloc failed\n");
	clp+= 384;
	for (i=-384; i<640; i++)
		clp[i] = (i<0) ? 0 : ((i>255) ? 255 : i);

	for (i=0; i<3; i++) {
		size = (i==0) ? width*height : chrom_width*chrom_height;

		if (!(newrefframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
		if (!(oldrefframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
		if (!(auxframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
		if (!(neworgframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
		if (!(oldorgframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
		if (!(auxorgframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
		if (!(predframe[i] = (unsigned char *)malloc(size)))
			error("malloc failed\n");
	}

	mbinfo = (Tmbinfo *)malloc(mb_width*mb_height2*sizeof(Tmbinfo));

	if (!mbinfo)
		error("malloc failed\n");

	blocks =
		(short (*)[64])malloc(mb_width*mb_height2*block_count*sizeof(short [64]));

	if (!blocks)
		error("malloc failed\n");

	/* open statistics output file */
	if (!statname[0])
		statfile = 0;
	else if (statname[0]=='-')
		statfile = stdout;
	else if (!(statfile = fopen(statname,"w")))	{
		sprintf(errortext,"Couldn't create statistics output file %s",statname);
		error(errortext);
	}
}

void MPEG::error(char *text)
{
	fprintf(stderr,text);
	putc('\n',stderr);
	exit(1);
}

void MPEG::DefaultParameters()
{
	extern int r,Xi,Xb,Xp,d0i,d0p,d0b; /* rate control */
	extern double avg_act; /* rate control */

	horizontal_size = -1;
	vertical_size = -1;
	display_horizontal_size = -1;
	display_vertical_size = -1;
	strcpy(id_string,"MPEG2 C++ encoder sequence");
	strcpy(tplref,"-");
	strcpy(iqname,"-");
	strcpy(niqname,"-");
	strcpy(statname,"");		// no statistics
	quiet = true;
	frame0 = 0;
	currentframenumber = 0;
	N = 12;
	M = 3;
	mpeg1 = false;
	fieldpic = false;
	aspectratio = 1;
	frame_rate_code = 3;
	bpf = 12000.0;		// bytes per frame
//	bit_rate = 5000000.0;	// now set in CheckParameters below, computed from bpf and frame_rate
	vbv_buffer_size = 112;	// in units of 16 kbit
	low_delay = false;
	constrparms = false;
	profile = 4;		// Main Profile
	level = 4;			// highest possible level
	prog_seq = false;
	chroma_format = 1;
	video_format = 0;	// 0=comp.
	color_primaries = 5;
	transfer_characteristics = 5;
	matrix_coefficients = 5;
	dc_prec = 2;
	topfirst = true;
	frame_pred_dct_tab[0] = false;
	frame_pred_dct_tab[1] = false;
	frame_pred_dct_tab[2] = false;
	conceal_tab[0] = false;
	conceal_tab[1] = false;
	conceal_tab[2] = false;
	qscale_tab[0] = true;
	qscale_tab[1] = true;
	qscale_tab[2] = true;
	intravlc_tab[0] = true;
	intravlc_tab[1] = false;
	intravlc_tab[2] = false;
	altscan_tab[0] = false;
	altscan_tab[1] = false;
	altscan_tab[2] = false;
	repeatfirst = false;
	prog_frame = false;
/* intra slice interval refresh period */
	P = 0;
	r = 0;
	avg_act = 0;
	Xi = 0;
	Xp = 0;
	Xb = 0;
	d0i = 0;
	d0p = 0;
	d0b = 0;

	if (N<1)
		error("N must be positive");
	if (M<1)
		error("M must be positive");
	if (N%M != 0)
		error("N must be an integer multiple of M");

	if (M != 3) error("M must be 3");
	
	motion_data = (Tmotion_data *)malloc(M*sizeof(Tmotion_data));
	if (!motion_data)
		error("malloc failed\n");

	motion_data[0].forw_hor_f_code = 2;
	motion_data[0].forw_vert_f_code = 2;
	motion_data[0].sxf = 11;
	motion_data[0].syf = 11;

	motion_data[1].forw_hor_f_code = 1;
	motion_data[1].forw_vert_f_code = 1;
	motion_data[1].sxf = 3;
	motion_data[1].syf = 3;
	motion_data[1].back_hor_f_code = 1;
	motion_data[1].back_vert_f_code = 1;
	motion_data[1].sxb = 7;
	motion_data[1].syb = 7;

	motion_data[2].forw_hor_f_code = 1;
	motion_data[2].forw_vert_f_code = 1;
	motion_data[2].sxf = 7;
	motion_data[2].syf = 7;
	motion_data[2].back_hor_f_code = 1;
	motion_data[2].back_vert_f_code = 1;
	motion_data[2].sxb = 3;
	motion_data[2].syb = 3;
}

void MPEG::CheckParameters(int h, int m, int s, int f)
{
	int i;
	static double ratetab[8]=
    {24000.0/1001.0,24.0,25.0,30000.0/1001.0,30.0,50.0,60000.0/1001.0,60.0};
	
	/* make sure MPEG specific parameters are valid */
	bit_rate = 5e5;		// some valid setting so that range_checks() doesnt complain..
	range_checks();

	// now really set bit_rate
	frame_rate = ratetab[frame_rate_code-1];
	bit_rate = frame_rate*8*bpf;
	range_checks();		// possibly the bit_rate was too large, check it...

	/* timecode -> frame number */
	tc0 = h;
	tc0 = 60*tc0 + m;
	tc0 = 60*tc0 + s;
	tc0 = (int)(frame_rate+0.5)*tc0 + f;

	if (!mpeg1) {
		profile_and_level_checks();
	} else {
		/* MPEG-1 */
		if (constrparms) {
			if (horizontal_size>768
				|| vertical_size>576
				|| ((horizontal_size+15)/16)*((vertical_size+15)/16)>396
				|| ((horizontal_size+15)/16)*((vertical_size+15)/16)*frame_rate>396*25.0
				|| frame_rate>30.0)
			{
				if (!quiet)
					fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
				constrparms = 0;
			}
		}

		if (constrparms) {
			for (i=0; i<M; i++) {
				if (motion_data[i].forw_hor_f_code>4) {
					if (!quiet)
						fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
					constrparms = 0;
					break;
				}

				if (motion_data[i].forw_vert_f_code>4) {
					if (!quiet)
						fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
					constrparms = 0;
					break;
				}

				if (i!=0) {
					if (motion_data[i].back_hor_f_code>4) {
						if (!quiet)
							fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
						constrparms = 0;
						break;
					}

					if (motion_data[i].back_vert_f_code>4) {
						if (!quiet)
							fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
						constrparms = 0;
						break;
					}
				}
			}
		}
	}

	/* relational checks */

	if (mpeg1)
	{
		if (!prog_seq)
		{
			if (!quiet)
				fprintf(stderr,"Warning: setting progressive_sequence = 1\n");
			prog_seq = 1;
		}

		if (chroma_format!=CHROMA420)
		{
			if (!quiet)
				fprintf(stderr,"Warning: setting chroma_format = 1 (4:2:0)\n");
			chroma_format = CHROMA420;
		}

		if (dc_prec!=0)
		{
			if (!quiet)
				fprintf(stderr,"Warning: setting intra_dc_precision = 0\n");
			dc_prec = 0;
		}

		for (i=0; i<3; i++)
			if (qscale_tab[i])
			{
				if (!quiet)
					fprintf(stderr,"Warning: setting qscale_tab[%d] = 0\n",i);
				qscale_tab[i] = 0;
			}

		for (i=0; i<3; i++)
			if (intravlc_tab[i])
			{
				if (!quiet)
					fprintf(stderr,"Warning: setting intravlc_tab[%d] = 0\n",i);
				intravlc_tab[i] = 0;
			}

		for (i=0; i<3; i++)
			if (altscan_tab[i])
			{
				if (!quiet)
					fprintf(stderr,"Warning: setting altscan_tab[%d] = 0\n",i);
				altscan_tab[i] = 0;
			}
	}

	if (!mpeg1 && constrparms)
	{
		if (!quiet)
			fprintf(stderr,"Warning: setting constrained_parameters_flag = 0\n");
		constrparms = 0;
	}

	if (prog_seq && !prog_frame)
	{
		if (!quiet)
			fprintf(stderr,"Warning: setting progressive_frame = 1\n");
		prog_frame = 1;
	}

	if (prog_frame && fieldpic)
	{
		if (!quiet)
			fprintf(stderr,"Warning: setting field_pictures = 0\n");
		fieldpic = 0;
	}

	if (!prog_frame && repeatfirst)
	{
		if (!quiet)
			fprintf(stderr,"Warning: setting repeat_first_field = 0\n");
		repeatfirst = 0;
	}

	if (prog_frame)
	{
		for (i=0; i<3; i++)
			if (!frame_pred_dct_tab[i])
			{
				if (!quiet)
					fprintf(stderr,"Warning: setting frame_pred_frame_dct[%d] = 1\n",i);
				frame_pred_dct_tab[i] = 1;
			}
	}

	if (prog_seq && !repeatfirst && topfirst)
	{
		if (!quiet)
			fprintf(stderr,"Warning: setting top_field_first = 0\n");
		topfirst = 0;
	}

	/* search windows */
	for (i=0; i<M; i++)
	{
		if (motion_data[i].sxf > (4<<motion_data[i].forw_hor_f_code)-1)
		{
			if (!quiet)
				fprintf(stderr,
						"Warning: reducing forward horizontal search width to %d\n",
						(4<<motion_data[i].forw_hor_f_code)-1);
			motion_data[i].sxf = (4<<motion_data[i].forw_hor_f_code)-1;
		}

		if (motion_data[i].syf > (4<<motion_data[i].forw_vert_f_code)-1)
		{
			if (!quiet)
				fprintf(stderr,
						"Warning: reducing forward vertical search width to %d\n",
						(4<<motion_data[i].forw_vert_f_code)-1);
			motion_data[i].syf = (4<<motion_data[i].forw_vert_f_code)-1;
		}

		if (i!=0)
		{
			if (motion_data[i].sxb > (4<<motion_data[i].back_hor_f_code)-1)
			{
				if (!quiet)
					fprintf(stderr,
							"Warning: reducing backward horizontal search width to %d\n",
							(4<<motion_data[i].back_hor_f_code)-1);
				motion_data[i].sxb = (4<<motion_data[i].back_hor_f_code)-1;
			}

			if (motion_data[i].syb > (4<<motion_data[i].back_vert_f_code)-1)
			{
				if (!quiet)
					fprintf(stderr,
							"Warning: reducing backward vertical search width to %d\n",
							(4<<motion_data[i].back_vert_f_code)-1);
				motion_data[i].syb = (4<<motion_data[i].back_vert_f_code)-1;
			}
		}
	}
}

void MPEG::readquantmat()
{
	int i,v;
	FILE *fd;

	if (iqname[0]=='-')
	{
		/* use default intra matrix */
		load_iquant = 0;
		for (i=0; i<64; i++)
			intra_q[i] = default_intra_quantizer_matrix[i];
	}
	else
	{
		/* read customized intra matrix */
		load_iquant = 1;
		if (!(fd = fopen(iqname,"r")))
		{
			sprintf(errortext,"Couldn't open quant matrix file %s",iqname);
			error(errortext);
		}

		for (i=0; i<64; i++)
		{
			fscanf(fd,"%d",&v);
			if (v<1 || v>255)
				error("invalid value in quant matrix");
			intra_q[i] = v;
		}

		fclose(fd);
	}

	if (niqname[0]=='-')
	{
		/* use default non-intra matrix */
		load_niquant = 0;
		for (i=0; i<64; i++)
			inter_q[i] = 16;
	}
	else
	{
		/* read customized non-intra matrix */
		load_niquant = 1;
		if (!(fd = fopen(niqname,"r")))
		{
			sprintf(errortext,"Couldn't open quant matrix file %s",niqname);
			error(errortext);
		}

		for (i=0; i<64; i++)
		{
			fscanf(fd,"%d",&v);
			if (v<1 || v>255)
				error("invalid value in quant matrix");
			inter_q[i] = v;
		}

		fclose(fd);
	}
}

MPEG::MPEG(const char *OutputFile, int Width, int Height)
	: FrameBuffer(3)		// works for M=3
{
	DefaultParameters();
	// transfer arguments to globals
	horizontal_size = display_horizontal_size = Width;
	vertical_size = display_vertical_size = Height;
	CheckParameters(0,0,0,0);
	readquantmat();
	outfile = fopen(OutputFile,"w");
	if (!outfile) {
		fprintf(stderr,"*** MPEGopen: cannot open output file \"%s\"\n",OutputFile);
		return;
	}
}

static void border_extend(unsigned char *frame, int w1, int h1, int w2, int h2)
{
	int i, j;
	unsigned char *fp;

	/* horizontal pixel replication (right border) */

	for (j=0; j<h1; j++)
	{
		fp = frame + j*w2;
		for (i=w1; i<w2; i++)
			fp[i] = fp[i-1];
	}

	/* vertical pixel replication (bottom border) */

	for (j=h1; j<h2; j++)
	{
		fp = frame + j*w2;
		for (i=0; i<w2; i++)
			fp[i] = fp[i-w2];
	}
}

/* horizontal filter and 2:1 subsampling */
void MPEG::conv444to422(unsigned char *src, unsigned char *dst)
{
	int i, j, im5, im4, im3, im2, im1, ip1, ip2, ip3, ip4, ip5, ip6;

	if (mpeg1)
	{
		for (j=0; j<height; j++)
		{
			for (i=0; i<width; i+=2)
			{
				im5 = (i<5) ? 0 : i-5;
				im4 = (i<4) ? 0 : i-4;
				im3 = (i<3) ? 0 : i-3;
				im2 = (i<2) ? 0 : i-2;
				im1 = (i<1) ? 0 : i-1;
				ip1 = (i<width-1) ? i+1 : width-1;
				ip2 = (i<width-2) ? i+2 : width-1;
				ip3 = (i<width-3) ? i+3 : width-1;
				ip4 = (i<width-4) ? i+4 : width-1;
				ip5 = (i<width-5) ? i+5 : width-1;
				ip6 = (i<width-5) ? i+6 : width-1;

				/* FIR filter with 0.5 sample interval phase shift */
				dst[i>>1] = clp[(int)(228*(src[i]+src[ip1])
									  +70*(src[im1]+src[ip2])
									  -37*(src[im2]+src[ip3])
									  -21*(src[im3]+src[ip4])
									  +11*(src[im4]+src[ip5])
									  + 5*(src[im5]+src[ip6])+256)>>9];
			}
			src+= width;
			dst+= width>>1;
		}
	}
	else
	{
		/* MPEG-2 */
		for (j=0; j<height; j++)
		{
			for (i=0; i<width; i+=2)
			{
				im5 = (i<5) ? 0 : i-5;
				im3 = (i<3) ? 0 : i-3;
				im1 = (i<1) ? 0 : i-1;
				ip1 = (i<width-1) ? i+1 : width-1;
				ip3 = (i<width-3) ? i+3 : width-1;
				ip5 = (i<width-5) ? i+5 : width-1;

				/* FIR filter coefficients (*512): 22 0 -52 0 159 256 159 0 -52 0 22 */
				dst[i>>1] = clp[(int)(  22*(src[im5]+src[ip5])-52*(src[im3]+src[ip3])
										+159*(src[im1]+src[ip1])+256*src[i]+256)>>9];
			}
			src+= width;
			dst+= width>>1;
		}
	}
}

/* vertical filter and 2:1 subsampling */
void MPEG::conv422to420(unsigned char *src, unsigned char *dst)
{
	int w, i, j, jm6, jm5, jm4, jm3, jm2, jm1;
	int jp1, jp2, jp3, jp4, jp5, jp6;

	w = width>>1;

	if (prog_frame)
	{
		/* intra frame */
		for (i=0; i<w; i++)
		{
			for (j=0; j<height; j+=2)
			{
				jm5 = (j<5) ? 0 : j-5;
				jm4 = (j<4) ? 0 : j-4;
				jm3 = (j<3) ? 0 : j-3;
				jm2 = (j<2) ? 0 : j-2;
				jm1 = (j<1) ? 0 : j-1;
				jp1 = (j<height-1) ? j+1 : height-1;
				jp2 = (j<height-2) ? j+2 : height-1;
				jp3 = (j<height-3) ? j+3 : height-1;
				jp4 = (j<height-4) ? j+4 : height-1;
				jp5 = (j<height-5) ? j+5 : height-1;
				jp6 = (j<height-5) ? j+6 : height-1;

				/* FIR filter with 0.5 sample interval phase shift */
				dst[w*(j>>1)] = clp[(int)(228*(src[w*j]+src[w*jp1])
										  +70*(src[w*jm1]+src[w*jp2])
										  -37*(src[w*jm2]+src[w*jp3])
										  -21*(src[w*jm3]+src[w*jp4])
										  +11*(src[w*jm4]+src[w*jp5])
										  + 5*(src[w*jm5]+src[w*jp6])+256)>>9];
			}
			src++;
			dst++;
		}
	}
	else
	{
		/* intra field */
		for (i=0; i<w; i++)
		{
			for (j=0; j<height; j+=4)
			{
				/* top field */
				jm5 = (j<10) ? 0 : j-10;
				jm4 = (j<8) ? 0 : j-8;
				jm3 = (j<6) ? 0 : j-6;
				jm2 = (j<4) ? 0 : j-4;
				jm1 = (j<2) ? 0 : j-2;
				jp1 = (j<height-2) ? j+2 : height-2;
				jp2 = (j<height-4) ? j+4 : height-2;
				jp3 = (j<height-6) ? j+6 : height-2;
				jp4 = (j<height-8) ? j+8 : height-2;
				jp5 = (j<height-10) ? j+10 : height-2;
				jp6 = (j<height-12) ? j+12 : height-2;

				/* FIR filter with 0.25 sample interval phase shift */
				dst[w*(j>>1)] = clp[(int)(8*src[w*jm5]
										  +5*src[w*jm4]
										  -30*src[w*jm3]
										  -18*src[w*jm2]
										  +113*src[w*jm1]
										  +242*src[w*j]
										  +192*src[w*jp1]
										  +35*src[w*jp2]
										  -38*src[w*jp3]
										  -10*src[w*jp4]
										  +11*src[w*jp5]
										  +2*src[w*jp6]+256)>>9];

				/* bottom field */
				jm6 = (j<9) ? 1 : j-9;
				jm5 = (j<7) ? 1 : j-7;
				jm4 = (j<5) ? 1 : j-5;
				jm3 = (j<3) ? 1 : j-3;
				jm2 = (j<1) ? 1 : j-1;
				jm1 = (j<height-1) ? j+1 : height-1;
				jp1 = (j<height-3) ? j+3 : height-1;
				jp2 = (j<height-5) ? j+5 : height-1;
				jp3 = (j<height-7) ? j+7 : height-1;
				jp4 = (j<height-9) ? j+9 : height-1;
				jp5 = (j<height-11) ? j+11 : height-1;
				jp6 = (j<height-13) ? j+13 : height-1;

				/* FIR filter with 0.25 sample interval phase shift */
				dst[w*((j>>1)+1)] = clp[(int)(8*src[w*jp6]
											  +5*src[w*jp5]
											  -30*src[w*jp4]
											  -18*src[w*jp3]
											  +113*src[w*jp2]
											  +242*src[w*jp1]
											  +192*src[w*jm1]
											  +35*src[w*jm2]
											  -38*src[w*jm3]
											  -10*src[w*jm4]
											  +11*src[w*jm5]
											  +2*src[w*jm6]+256)>>9];
			}
			src++;
			dst++;
		}
	}
}

void MPEG::GetYUVFrame(const unsigned char red[], const unsigned char green[], const unsigned char blue[],
					   unsigned char *frame[])
{
	int i, j;
	int r, g, b;
	double y, u, v;
	double cr, cg, cb, cu, cv;
	unsigned char *yp, *up, *vp;
	static unsigned char *u444, *v444, *u422, *v422;
	static double coef[7][3] = {
		{0.2125,0.7154,0.0721}, /* ITU-R Rec. 709 (1990) */
		{0.299, 0.587, 0.114},  /* unspecified */
		{0.299, 0.587, 0.114},  /* reserved */
		{0.30,  0.59,  0.11},   /* FCC */
		{0.299, 0.587, 0.114},  /* ITU-R Rec. 624-4 System B, G */
		{0.299, 0.587, 0.114},  /* SMPTE 170M */
		{0.212, 0.701, 0.087}}; /* SMPTE 240M (1987) */

	i = matrix_coefficients;
	if (i>8)
		i = 3;

	cr = coef[i-1][0];
	cg = coef[i-1][1];
	cb = coef[i-1][2];
	cu = 0.5/(1.0-cb);
	cv = 0.5/(1.0-cr);

	if (chroma_format==CHROMA444)
	{
		u444 = frame[1];
		v444 = frame[2];
	}
	else
	{
		if (!u444)
		{
			if (!(u444 = (unsigned char *)malloc(width*height)))
				error("malloc failed");
			if (!(v444 = (unsigned char *)malloc(width*height)))
				error("malloc failed");
			if (chroma_format==CHROMA420)
			{
				if (!(u422 = (unsigned char *)malloc((width>>1)*height)))
					error("malloc failed");
				if (!(v422 = (unsigned char *)malloc((width>>1)*height)))
					error("malloc failed");
			}
		}
	}

	for (i=0; i<vertical_size; i++) {
		yp = frame[0] + i*width;
		up = u444 + i*width;
		vp = v444 + i*width;

		for (j=0; j<horizontal_size; j++)
		{
			r=red[i*horizontal_size+j]; g=green[i*horizontal_size+j]; b=blue[i*horizontal_size+j];
			/* convert to YUV */
			y = cr*r + cg*g + cb*b;
			u = cu*(b-y);
			v = cv*(r-y);
			yp[j] = (unsigned char)((219.0/256.0)*y + 16.5);  /* nominal range: 16..235 */
			up[j] = (unsigned char)((224.0/256.0)*u + 128.5); /* 16..240 */
			vp[j] = (unsigned char)((224.0/256.0)*v + 128.5); /* 16..240 */
		}
	}

	border_extend(frame[0],horizontal_size,vertical_size,width,height);
	border_extend(u444,horizontal_size,vertical_size,width,height);
	border_extend(v444,horizontal_size,vertical_size,width,height);

	if (chroma_format==CHROMA422)
	{
		conv444to422(u444,frame[1]);
		conv444to422(v444,frame[2]);
	}

	if (chroma_format==CHROMA420)
	{
		conv444to422(u444,u422);
		conv444to422(v444,v422);
		conv422to420(u422,frame[1]);
		conv422to420(v422,frame[2]);
	}
	
}

bool MPEG::try_to_add_frame()
// Tries to add a new frame to the animation by reading the necessary frame from FrameBuffer.
// If the needed frame is not yet in FrameBuffer, returns false, otherwise does the job and returns true.
{
	/* this routine assumes (N % M) == 0 */
	int j, k, f, f0, n, np, nb, sxf=0, syf=0, sxb=0, syb=0;
	int ipflag;
	char name[256];
	unsigned char *neworg[3], *newref[3];
	static char ipb[5] = {' ','I','P','B','D'};

	if (currentframenumber==0 || (currentframenumber-1)%M==0) {
		// I or P frame
		f = (currentframenumber==0) ? 0 : currentframenumber+M-1;
//		if (f>=nframes)
//			f = nframes - 1;
	} else {
		// B frame
		f = currentframenumber - 1;
	}
	const int frame_needed = f+frame0;
	unsigned char *r,*g,*b; int dummy;
	if (!FrameBuffer.getframe(frame_needed,r,g,b,dummy)) return false;
	
	if (!quiet) {
		fprintf(stderr,"Encoding frame %d ",currentframenumber);
		fflush(stderr);
	}

	/* f0: lowest frame number in current GOP
	 *
	 * first GOP contains N-(M-1) frames,
	 * all other GOPs contain N frames
	 */
	f0 = N*((currentframenumber+(M-1))/N) - (M-1);

	if (f0<0)
		f0=0;

	if (currentframenumber==0 || (currentframenumber-1)%M==0) {
		/* I or P frame */
		for (j=0; j<3; j++) {
			/* shuffle reference frames */
			neworg[j] = oldorgframe[j];
			newref[j] = oldrefframe[j];
			oldorgframe[j] = neworgframe[j];
			oldrefframe[j] = newrefframe[j];
			neworgframe[j] = neworg[j];
			newrefframe[j] = newref[j];
		}

		/* f: frame number in display order */
		f = (currentframenumber==0) ? 0 : currentframenumber+M-1;
//		if (f>=nframes)
//			f = nframes - 1;

		if (currentframenumber==f0) {	/* first displayed frame in GOP is I */
			/* I frame */
			pict_type = I_TYPE;
			forw_hor_f_code = forw_vert_f_code = 15;
			back_hor_f_code = back_vert_f_code = 15;

			/* n: number of frames in current GOP
			 *
			 * first GOP contains (M-1) less (B) frames
			 */
			n = (currentframenumber==0) ? N-(M-1) : N;

//			/* last GOP may contain less frames */
//			if (n > nframes-f0)
//				n = nframes-f0;

			/* number of P frames */
			if (currentframenumber==0)
				np = (n + 2*(M-1))/M - 1; /* first GOP */
			else
				np = (n + (M-1))/M - 1;

			/* number of B frames */
			nb = n - np - 1;

			rc_init_GOP(np,nb);

			putgophdr(f0,currentframenumber==0); /* set closed_GOP in first GOP only */
		} else {
			/* P frame */
			pict_type = P_TYPE;
			forw_hor_f_code = motion_data[0].forw_hor_f_code;
			forw_vert_f_code = motion_data[0].forw_vert_f_code;
			back_hor_f_code = back_vert_f_code = 15;
			sxf = motion_data[0].sxf;
			syf = motion_data[0].syf;
		}
	} else {
		/* B frame */
		for (j=0; j<3; j++) {
			neworg[j] = auxorgframe[j];
			newref[j] = auxframe[j];
		}

		/* f: frame number in display order */
		f = currentframenumber - 1;
		pict_type = B_TYPE;
		n = (currentframenumber-2)%M + 1; /* first B: n=1, second B: n=2, ... */
		forw_hor_f_code = motion_data[n].forw_hor_f_code;
		forw_vert_f_code = motion_data[n].forw_vert_f_code;
		back_hor_f_code = motion_data[n].back_hor_f_code;
		back_vert_f_code = motion_data[n].back_vert_f_code;
		sxf = motion_data[n].sxf;
		syf = motion_data[n].syf;
		sxb = motion_data[n].sxb;
		syb = motion_data[n].syb;
	}

	temp_ref = f - f0;
	frame_pred_dct = frame_pred_dct_tab[pict_type-1];
	q_scale_type = qscale_tab[pict_type-1];
	intravlc = intravlc_tab[pict_type-1];
	altscan = altscan_tab[pict_type-1];
	
	if (statfile) {
		fprintf(statfile,"\nFrame %d (#%d in display order):\n",currentframenumber,f);
		fprintf(statfile," picture_type=%c\n",ipb[pict_type]);
		fprintf(statfile," temporal_reference=%d\n",temp_ref);
		fprintf(statfile," frame_pred_frame_dct=%d\n",frame_pred_dct);
		fprintf(statfile," q_scale_type=%d\n",q_scale_type);
		fprintf(statfile," intra_vlc_format=%d\n",intravlc);
		fprintf(statfile," alternate_scan=%d\n",altscan);
		
		if (pict_type!=I_TYPE) {
			fprintf(statfile," forward search window: %d...%d / %d...%d\n",
					-sxf,sxf,-syf,syf);
				fprintf(statfile," forward vector range: %d...%d.5 / %d...%d.5\n",
						-(4<<forw_hor_f_code),(4<<forw_hor_f_code)-1,
						-(4<<forw_vert_f_code),(4<<forw_vert_f_code)-1);
		}

		if (pict_type==B_TYPE) {
			fprintf(statfile," backward search window: %d...%d / %d...%d\n",
					-sxb,sxb,-syb,syb);
			fprintf(statfile," backward vector range: %d...%d.5 / %d...%d.5\n",
					-(4<<back_hor_f_code),(4<<back_hor_f_code)-1,
					-(4<<back_vert_f_code),(4<<back_vert_f_code)-1);
		}

	}
//	if (!quiet) fprintf(stderr,"Now needing frame %d\n",f+frame0);
	unsigned char *red=0, *green=0, *blue=0;
	FrameBuffer.getframe(frame_needed,red,green,blue,dummy);		// it returns true, we know it
	GetYUVFrame(red,green,blue,neworg);

	if (fieldpic) {
		if (!quiet) {
			fprintf(stderr,"\nfirst field  (%s) ",topfirst ? "top" : "bot");
			fflush(stderr);
		}

		pict_struct = topfirst ? TOP_FIELD : BOTTOM_FIELD;

		motion_estimation(oldorgframe[0],neworgframe[0],
						  oldrefframe[0],newrefframe[0],
						  neworg[0],newref[0],
						  sxf,syf,sxb,syb,mbinfo,0,0);

		predict(oldrefframe,newrefframe,predframe,0,mbinfo);
		dct_type_estimation(predframe[0],neworg[0],mbinfo);
		transform(predframe,neworg,mbinfo,blocks);

		putpict(neworg[0]);

		for (k=0; k<mb_height2*mb_width; k++) {
			if (mbinfo[k].mb_type & MB_INTRA)
				for (j=0; j<block_count; j++)
					iquant_intra(blocks[k*block_count+j],blocks[k*block_count+j],
								 dc_prec,intra_q,mbinfo[k].mquant);
			else
				for (j=0;j<block_count;j++)
					iquant_non_intra(blocks[k*block_count+j],blocks[k*block_count+j],
									 inter_q,mbinfo[k].mquant);
		}

		itransform(predframe,newref,mbinfo,blocks);
		if (statfile) {
			calcSNR(neworg,newref);
			stats();
		}

		if (!quiet) {
			fprintf(stderr,"second field (%s) ",topfirst ? "bot" : "top");
			fflush(stderr);
		}

		pict_struct = topfirst ? BOTTOM_FIELD : TOP_FIELD;

		ipflag = (pict_type==I_TYPE);
		if (ipflag) {
			/* first field = I, second field = P */
			pict_type = P_TYPE;
			forw_hor_f_code = motion_data[0].forw_hor_f_code;
			forw_vert_f_code = motion_data[0].forw_vert_f_code;
			back_hor_f_code = back_vert_f_code = 15;
			sxf = motion_data[0].sxf;
			syf = motion_data[0].syf;
		}

		motion_estimation(oldorgframe[0],neworgframe[0],
						  oldrefframe[0],newrefframe[0],
						  neworg[0],newref[0],
						  sxf,syf,sxb,syb,mbinfo,1,ipflag);

		predict(oldrefframe,newrefframe,predframe,1,mbinfo);
		dct_type_estimation(predframe[0],neworg[0],mbinfo);
		transform(predframe,neworg,mbinfo,blocks);

		putpict(neworg[0]);

		for (k=0; k<mb_height2*mb_width; k++) {
			if (mbinfo[k].mb_type & MB_INTRA)
				for (j=0; j<block_count; j++)
					iquant_intra(blocks[k*block_count+j],blocks[k*block_count+j],
								 dc_prec,intra_q,mbinfo[k].mquant);
			else
				for (j=0;j<block_count;j++)
					iquant_non_intra(blocks[k*block_count+j],blocks[k*block_count+j],
									 inter_q,mbinfo[k].mquant);
		}

		itransform(predframe,newref,mbinfo,blocks);
		if (statfile) {
			calcSNR(neworg,newref);
			stats();
		}
	} else {
		pict_struct = FRAME_PICTURE;

		/* do motion_estimation
		 *
		 * uses source frames (...orgframe) for full pel search
		 * and reconstructed frames (...refframe) for half pel search
		 */

		motion_estimation(oldorgframe[0],neworgframe[0],
						  oldrefframe[0],newrefframe[0],
						  neworg[0],newref[0],
						  sxf,syf,sxb,syb,mbinfo,0,0);

		predict(oldrefframe,newrefframe,predframe,0,mbinfo);
		dct_type_estimation(predframe[0],neworg[0],mbinfo);
		transform(predframe,neworg,mbinfo,blocks);

		putpict(neworg[0]);

		for (k=0; k<mb_height*mb_width; k++)
		{
			if (mbinfo[k].mb_type & MB_INTRA)
				for (j=0; j<block_count; j++)
					iquant_intra(blocks[k*block_count+j],blocks[k*block_count+j],
								 dc_prec,intra_q,mbinfo[k].mquant);
			else
				for (j=0;j<block_count;j++)
					iquant_non_intra(blocks[k*block_count+j],blocks[k*block_count+j],
									 inter_q,mbinfo[k].mquant);
		}

		itransform(predframe,newref,mbinfo,blocks);
		if (statfile) {
			calcSNR(neworg,newref);
			stats();
		}
	}

	sprintf(name,tplref,f+frame0);
	writeframe(name,newref);

	currentframenumber++;
	return true;
}

void MPEG::addframe(const unsigned char red[], const unsigned char green[], const unsigned char blue[])
{
	if (FrameBuffer.Nframes() == 0) {
		// First frame, do some initialization
		init();
		rc_init_seq(); // initialize rate control
		// sequence header, sequence extension and sequence display extension
		putseqhdr();
		if (!mpeg1)
		{
			putseqext();
			putseqdispext();
		}

		// optionally output some text data (description, copyright or whatever)
		if (strlen(id_string) > 1)
			putuserdata(id_string);
	}
	FrameBuffer.add(red,green,blue,vertical_size*horizontal_size);
	bool success;
	do {
		success = try_to_add_frame();
	} while (success);
}

MPEG::~MPEG()
{
	bool success;
	do {
		success = try_to_add_frame();
	} while (success);
	putseqend();
	fclose(outfile);
	if (statfile) fclose(statfile);
}

