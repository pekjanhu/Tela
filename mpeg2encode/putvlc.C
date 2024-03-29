/* putvlc.c, generation of variable length codes                            */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include <stdio.h>
#include "mpegintf.H"
#include "global.h"


/* data from ISO/IEC 13818-2 DIS, Annex B, variable length code tables */

/* Table B-1, variable length codes for macroblock_address_increment
 *
 * indexed by [macroblock_address_increment-1]
 * 'macroblock_escape' is treated elsewhere
 */

static MPEG::VLCtable addrinctab[33]=
{
  {0x01,1},  {0x03,3},  {0x02,3},  {0x03,4},
  {0x02,4},  {0x03,5},  {0x02,5},  {0x07,7},
  {0x06,7},  {0x0b,8},  {0x0a,8},  {0x09,8},
  {0x08,8},  {0x07,8},  {0x06,8},  {0x17,10},
  {0x16,10}, {0x15,10}, {0x14,10}, {0x13,10},
  {0x12,10}, {0x23,11}, {0x22,11}, {0x21,11},
  {0x20,11}, {0x1f,11}, {0x1e,11}, {0x1d,11},
  {0x1c,11}, {0x1b,11}, {0x1a,11}, {0x19,11},
  {0x18,11}
};


/* Table B-2, B-3, B-4 variable length codes for macroblock_type
 *
 * indexed by [macroblock_type]
 */

static MPEG::VLCtable mbtypetab[3][32]=
{
 /* I */
 {
  {0,0}, {1,1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {1,2}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
 },
 /* P */
 {
  {0,0}, {3,5}, {1,2}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {1,3}, {0,0}, {1,1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {1,6}, {1,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {2,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
 },
 /* B */
 {
  {0,0}, {3,5}, {0,0}, {0,0}, {2,3}, {0,0}, {3,3}, {0,0},
  {2,4}, {0,0}, {3,4}, {0,0}, {2,2}, {0,0}, {3,2}, {0,0},
  {0,0}, {1,6}, {0,0}, {0,0}, {0,0}, {0,0}, {2,6}, {0,0},
  {0,0}, {0,0}, {3,6}, {0,0}, {0,0}, {0,0}, {2,5}, {0,0}
 }
};


/* Table B-5 ... B-8 variable length codes for macroblock_type in
 *  scalable sequences
 *
 * not implemented
 */

/* Table B-9, variable length codes for coded_block_pattern
 *
 * indexed by [coded_block_pattern]
 */

static MPEG::VLCtable cbptable[64]=
{
  {0x01,9}, {0x0b,5}, {0x09,5}, {0x0d,6}, 
  {0x0d,4}, {0x17,7}, {0x13,7}, {0x1f,8}, 
  {0x0c,4}, {0x16,7}, {0x12,7}, {0x1e,8}, 
  {0x13,5}, {0x1b,8}, {0x17,8}, {0x13,8}, 
  {0x0b,4}, {0x15,7}, {0x11,7}, {0x1d,8}, 
  {0x11,5}, {0x19,8}, {0x15,8}, {0x11,8}, 
  {0x0f,6}, {0x0f,8}, {0x0d,8}, {0x03,9}, 
  {0x0f,5}, {0x0b,8}, {0x07,8}, {0x07,9}, 
  {0x0a,4}, {0x14,7}, {0x10,7}, {0x1c,8}, 
  {0x0e,6}, {0x0e,8}, {0x0c,8}, {0x02,9}, 
  {0x10,5}, {0x18,8}, {0x14,8}, {0x10,8}, 
  {0x0e,5}, {0x0a,8}, {0x06,8}, {0x06,9}, 
  {0x12,5}, {0x1a,8}, {0x16,8}, {0x12,8}, 
  {0x0d,5}, {0x09,8}, {0x05,8}, {0x05,9}, 
  {0x0c,5}, {0x08,8}, {0x04,8}, {0x04,9},
  {0x07,3}, {0x0a,5}, {0x08,5}, {0x0c,6}
};


/* Table B-10, variable length codes for motion_code
 *
 * indexed by [abs(motion_code)]
 * sign of motion_code is treated elsewhere
 */

static MPEG::VLCtable motionvectab[17]=
{
  {0x01,1},  {0x01,2},  {0x01,3},  {0x01,4},
  {0x03,6},  {0x05,7},  {0x04,7},  {0x03,7},
  {0x0b,9},  {0x0a,9},  {0x09,9},  {0x11,10},
  {0x10,10}, {0x0f,10}, {0x0e,10}, {0x0d,10},
  {0x0c,10}
};


/* Table B-11, variable length codes for dmvector
 *
 * treated elsewhere
 */

/* Table B-12, variable length codes for dct_dc_size_luminance
 *
 * indexed by [dct_dc_size_luminance]
 */

static MPEG::sVLCtable DClumtab[12]=
{
  {0x0004,3}, {0x0000,2}, {0x0001,2}, {0x0005,3}, {0x0006,3}, {0x000e,4},
  {0x001e,5}, {0x003e,6}, {0x007e,7}, {0x00fe,8}, {0x01fe,9}, {0x01ff,9}
};


/* Table B-13, variable length codes for dct_dc_size_chrominance
 *
 * indexed by [dct_dc_size_chrominance]
 */

static MPEG::sVLCtable DCchromtab[12]=
{
  {0x0000,2}, {0x0001,2}, {0x0002,2}, {0x0006,3}, {0x000e,4}, {0x001e,5},
  {0x003e,6}, {0x007e,7}, {0x00fe,8}, {0x01fe,9}, {0x03fe,10},{0x03ff,10}
};


/* Table B-14, DCT coefficients table zero
 *
 * indexed by [run][level-1]
 * split into two tables (dct_code_tab1, dct_code_tab2) to reduce size
 * 'first DCT coefficient' condition and 'End of Block' are treated elsewhere
 * codes do not include s (sign bit)
 */

static MPEG::VLCtable dct_code_tab1[2][40]=
{
 /* run = 0, level = 1...40 */
 {
  {0x03, 2}, {0x04, 4}, {0x05, 5}, {0x06, 7},
  {0x26, 8}, {0x21, 8}, {0x0a,10}, {0x1d,12},
  {0x18,12}, {0x13,12}, {0x10,12}, {0x1a,13},
  {0x19,13}, {0x18,13}, {0x17,13}, {0x1f,14},
  {0x1e,14}, {0x1d,14}, {0x1c,14}, {0x1b,14},
  {0x1a,14}, {0x19,14}, {0x18,14}, {0x17,14},
  {0x16,14}, {0x15,14}, {0x14,14}, {0x13,14},
  {0x12,14}, {0x11,14}, {0x10,14}, {0x18,15},
  {0x17,15}, {0x16,15}, {0x15,15}, {0x14,15},
  {0x13,15}, {0x12,15}, {0x11,15}, {0x10,15}
 },
 /* run = 1, level = 1...18 */
 {
  {0x03, 3}, {0x06, 6}, {0x25, 8}, {0x0c,10},
  {0x1b,12}, {0x16,13}, {0x15,13}, {0x1f,15},
  {0x1e,15}, {0x1d,15}, {0x1c,15}, {0x1b,15},
  {0x1a,15}, {0x19,15}, {0x13,16}, {0x12,16},
  {0x11,16}, {0x10,16}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}
 }
};

static MPEG::VLCtable dct_code_tab2[30][5]=
{
  /* run = 2...31, level = 1...5 */
  {{0x05, 4}, {0x04, 7}, {0x0b,10}, {0x14,12}, {0x14,13}},
  {{0x07, 5}, {0x24, 8}, {0x1c,12}, {0x13,13}, {0x00, 0}},
  {{0x06, 5}, {0x0f,10}, {0x12,12}, {0x00, 0}, {0x00, 0}},
  {{0x07, 6}, {0x09,10}, {0x12,13}, {0x00, 0}, {0x00, 0}},
  {{0x05, 6}, {0x1e,12}, {0x14,16}, {0x00, 0}, {0x00, 0}},
  {{0x04, 6}, {0x15,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x07, 7}, {0x11,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x05, 7}, {0x11,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x27, 8}, {0x10,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x23, 8}, {0x1a,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x22, 8}, {0x19,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x20, 8}, {0x18,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x0e,10}, {0x17,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x0d,10}, {0x16,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x08,10}, {0x15,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1a,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x19,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x17,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x16,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}}
};


/* Table B-15, DCT coefficients table one
 *
 * indexed by [run][level-1]
 * split into two tables (dct_code_tab1a, dct_code_tab2a) to reduce size
 * 'End of Block' is treated elsewhere
 * codes do not include s (sign bit)
 */

static MPEG::VLCtable dct_code_tab1a[2][40]=
{
 /* run = 0, level = 1...40 */
 {
  {0x02, 2}, {0x06, 3}, {0x07, 4}, {0x1c, 5},
  {0x1d, 5}, {0x05, 6}, {0x04, 6}, {0x7b, 7},
  {0x7c, 7}, {0x23, 8}, {0x22, 8}, {0xfa, 8},
  {0xfb, 8}, {0xfe, 8}, {0xff, 8}, {0x1f,14},
  {0x1e,14}, {0x1d,14}, {0x1c,14}, {0x1b,14},
  {0x1a,14}, {0x19,14}, {0x18,14}, {0x17,14},
  {0x16,14}, {0x15,14}, {0x14,14}, {0x13,14},
  {0x12,14}, {0x11,14}, {0x10,14}, {0x18,15},
  {0x17,15}, {0x16,15}, {0x15,15}, {0x14,15},
  {0x13,15}, {0x12,15}, {0x11,15}, {0x10,15}
 },
 /* run = 1, level = 1...18 */
 {
  {0x02, 3}, {0x06, 5}, {0x79, 7}, {0x27, 8},
  {0x20, 8}, {0x16,13}, {0x15,13}, {0x1f,15},
  {0x1e,15}, {0x1d,15}, {0x1c,15}, {0x1b,15},
  {0x1a,15}, {0x19,15}, {0x13,16}, {0x12,16},
  {0x11,16}, {0x10,16}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0},
  {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}
 }
};

static MPEG::VLCtable dct_code_tab2a[30][5]=
{
  /* run = 2...31, level = 1...5 */
  {{0x05, 5}, {0x07, 7}, {0xfc, 8}, {0x0c,10}, {0x14,13}},
  {{0x07, 5}, {0x26, 8}, {0x1c,12}, {0x13,13}, {0x00, 0}},
  {{0x06, 6}, {0xfd, 8}, {0x12,12}, {0x00, 0}, {0x00, 0}},
  {{0x07, 6}, {0x04, 9}, {0x12,13}, {0x00, 0}, {0x00, 0}},
  {{0x06, 7}, {0x1e,12}, {0x14,16}, {0x00, 0}, {0x00, 0}},
  {{0x04, 7}, {0x15,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x05, 7}, {0x11,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x78, 7}, {0x11,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x7a, 7}, {0x10,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x21, 8}, {0x1a,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x25, 8}, {0x19,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x24, 8}, {0x18,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x05, 9}, {0x17,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x07, 9}, {0x16,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x0d,10}, {0x15,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1a,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x19,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x17,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x16,12}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,13}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1f,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1e,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1d,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1c,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}},
  {{0x1b,16}, {0x00, 0}, {0x00, 0}, {0x00, 0}, {0x00, 0}}
};

/* generate variable length code for luminance DC coefficient */
void MPEG::putDClum(int val)
{
	putDC(DClumtab,val);
}

/* generate variable length code for chrominance DC coefficient */
void MPEG::putDCchrom(int val)
{
	putDC(DCchromtab,val);
}

/* generate variable length code for DC coefficient (7.2.1) */
void MPEG::putDC(sVLCtable *tab, int val)
{
	int absval, size;

	absval = (val<0) ? -val : val; /* abs(val) */

	if (absval>2047 || (mpeg1 && absval>255))
	{
		/* should never happen */
		sprintf(errortext,"DC value out of range (%d)\n",val);
		error(errortext);
	}

	/* compute dct_dc_size */
	size = 0;

	while (absval)
	{
		absval >>= 1;
		size++;
	}

	/* generate VLC for dct_dc_size (Table B-12 or B-13) */
	putbits(tab[size].code,tab[size].len);

	/* append fixed length code (dc_dct_differential) */
	if (size!=0)
	{
		if (val>=0)
			absval = val;
		else
			absval = val + (1<<size) - 1; /* val + (2 ^ size) - 1 */
		putbits(absval,size);
	}
}

/* generate variable length code for first coefficient
 * of a non-intra block (7.2.2.2) */
void MPEG::putACfirst(int run, int val)
{
	if (run==0 && (val==1 || val==-1)) /* these are treated differently */
		putbits(2|(val<0),2); /* generate '1s' (s=sign), (Table B-14, line 2) */
	else
		putAC(run,val,0); /* no difference for all others */
}

/* generate variable length code for other DCT coefficients (7.2.2) */
void MPEG::putAC(int run, int signed_level, int vlcformat)
{
	int level, len;
	VLCtable *ptab = 0;

	level = (signed_level<0) ? -signed_level : signed_level; /* abs(signed_level) */

	/* make sure run and level are valid */
	if (run<0 || run>63 || level==0 || level>2047 || (mpeg1 && level>255))
	{
		sprintf(errortext,"AC value out of range (run=%d, signed_level=%d)\n",
				run,signed_level);
		error(errortext);
	}

	len = 0;

	if (run<2 && level<41)
	{
		/* vlcformat selects either of Table B-14 / B-15 */
		if (vlcformat)
			ptab = &dct_code_tab1a[run][level-1];
		else
			ptab = &dct_code_tab1[run][level-1];

		len = ptab->len;
	}
	else if (run<32 && level<6)
	{
		/* vlcformat selects either of Table B-14 / B-15 */
		if (vlcformat)
			ptab = &dct_code_tab2a[run-2][level-1];
		else
			ptab = &dct_code_tab2[run-2][level-1];

		len = ptab->len;
	}

	if (len!=0) /* a VLC code exists */
	{
		putbits(ptab->code,len);
		putbits(signed_level<0,1); /* sign */
	}
	else
	{
		/* no VLC for this (run, level) combination: use escape coding (7.2.2.3) */
		putbits(1l,6); /* Escape */
		putbits(run,6); /* 6 bit code for run */
		if (mpeg1)
		{
			/* ISO/IEC 11172-2 uses a 8 or 16 bit code */
			if (signed_level>127)
				putbits(0,8);
			if (signed_level<-127)
				putbits(128,8);
			putbits(signed_level,8);
		}
		else
		{
			/* ISO/IEC 13818-2 uses a 12 bit code, Table B-16 */
			putbits(signed_level,12);
		}
	}
}

/* generate variable length code for macroblock_address_increment (6.3.16) */
void MPEG::putaddrinc(int addrinc)
{
	while (addrinc>33)
	{
		putbits(0x08,11); /* macroblock_escape */
		addrinc-= 33;
	}

	putbits(addrinctab[addrinc-1].code,addrinctab[addrinc-1].len);
}

/* generate variable length code for macroblock_type (6.3.16.1) */
void MPEG::putmbtype(int pict_type, int mb_type)
{
	putbits(mbtypetab[pict_type-1][mb_type].code,
			mbtypetab[pict_type-1][mb_type].len);
}

/* generate variable length code for motion_code (6.3.16.3) */
void MPEG::putmotioncode(int motion_code)
{
	int abscode;

	abscode = (motion_code>=0) ? motion_code : -motion_code; /* abs(motion_code) */
	putbits(motionvectab[abscode].code,motionvectab[abscode].len);
	if (motion_code!=0)
		putbits(motion_code<0,1); /* sign, 0=positive, 1=negative */
}

/* generate variable length code for dmvector[t] (6.3.16.3), Table B-11 */
void MPEG::putdmv(int dmv)
{
	if (dmv==0)
		putbits(0,1);
	else if (dmv>0)
		putbits(2,2);
	else
		putbits(3,2);
}

/* generate variable length code for coded_block_pattern (6.3.16.4)
 *
 * 4:2:2, 4:4:4 not implemented
 */
void MPEG::putcbp(int cbp)
{
	putbits(cbptable[cbp].code,cbptable[cbp].len);
}
