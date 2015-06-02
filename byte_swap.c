#if defined i386 || defined _BYTE_SWAP_C
/* avxpc.c and any other files that need this
 * code even if running on big-endian cpus,
 * should define _BYTE_SWAP_C and then include
 * this file.
 */

#include "p_lib.h"


#if defined i386 && defined __GNUC__
static __inline__ uint16 swap16 (uint16);
static __inline__ uint32 swap32 (uint32);

static __inline__ uint16 swap16 (uint16 __arg)
{
	register uint16 __result;

	__asm__ ("xchg%B0 %b0,%h0" : "=q" (__result) : "0" (__arg));
	return __result;
}

static __inline__ uint32 swap32 (uint32 __arg)
{
	register uint32 __result;

	__asm__ ("\n\
		xchg%B0 %b0,%h0\n\
		ror%L0 $16,%0\n\
		xchg%B0 %b0,%h0" : "=q" (__result) : "0" (__arg));
	return __result;
}

#else
#define	swap16(word)	((((word) << 8)  & 0xff00) | (((word) >> 8) & 0x00ff))
#define swap32(word)	((((word) >> 24) & 0x000000ff) |\
						 (((word) >> 8) & 0x0000ff00) |\
						 (((word) << 8) & 0x00ff0000) |\
						 (((word) << 24) & 0xff000000))
#endif


/* These are defined in txfile.h.
 * Instead of including it,
 * define them here too.
 */
#if !defined TX_EOL
#if defined i386
#define TX_EOL	0xfcff		/* -4 */
#else
#define TX_EOL	0xfffc		/* -4 */
#endif
#endif

#define SW_BYTE 1
#define SW_HALF 2
#define SW_LONG 4


typedef struct sw_array
{
	int type;
	int count;
}	sw_array;

typedef struct sw_block
{
	void (*func)(uchar*, int, sw_array*);
	sw_array *sw;
}	sw_block;


sw_array sw_header[] =
{
	{ SW_HALF,   4 },
	{ SW_LONG,   1 },
	{ SW_HALF,   3 },
	{ SW_LONG,   6 },
	{ SW_HALF,   1 },
	{ SW_LONG,   1 },
	{ SW_HALF,  44 },
	{ SW_LONG,   1 },
	{ SW_HALF,   2 },
	{ SW_LONG,  18 },
	{ SW_HALF,   2 },
	{ SW_LONG,   1 },
	{ SW_HALF,   1 },
	{ SW_LONG,   1 },
	{ SW_HALF, 107 },
	{ SW_LONG,   1 },
	{ SW_HALF,  32 },
	{ 0, 0 }
};

sw_array sw_texthblock[] =
{
	{ SW_HALF, 6 },
	{ 0, 0 }
};

sw_array sw_format[] =
{
	{ SW_BYTE,   62 },		/* Up to 62 text characters of the format   */
	{ SW_HALF,   1 },		/* int16 link to next format record */
	{ 0, 0 }
};

sw_array sw_gentag[] =
{
	{ SW_BYTE,   24 },		/* Up to 24 text chars of the gentag name */
	{ SW_HALF,   2 },		/* Format command, format number */
	{ 0, 0 }
};

sw_array sw_styletag[] =
{
	{ SW_BYTE,   500 },		/* Up to 50 text chars of the gentag name */
	{ SW_HALF,   2 },		/* Format command, format number */
	{ 0, 0 }
};

sw_array sw_pm_text[] =
{
	{ SW_HALF,   24 },		/* All misc info about the line.	*/
	{ SW_LONG,   1 },
	{ SW_HALF,   6 },
	{ SW_BYTE,   2 },
	{ SW_HALF,   1 },
	{ SW_LONG,   1 },		/* cmus of the line 	*/
	{ SW_HALF,   2 },
	{ 0, 0 }
};

sw_array sw_pm_insert[] =
{
	{ SW_HALF,   28 },		/* All misc info about the cut.	*/
	{ SW_LONG,   5 },		/* First/last pm rec, cmus, ref-cmus	*/
	{ 0, 0 }
};

sw_array sw_pm_string[] =
{
	{ SW_HALF,   2 },		/* Line-type & string length.	*/
	{ SW_BYTE,   72 },		/* String-name[11], data[61]	*/
	{ 0, 0 }
};

sw_array sw_pm_str2[] =
{
	{ SW_HALF,   1 },		/* Line-type	*/
	{ SW_BYTE,   74 },		/* continuation of string data[74]	*/
	{ 0, 0 }
};

sw_array sw_psd_setup[] =
{
	{ SW_BYTE,   528 },		/* 22 ascii fields of 24 bytes each */
	{ SW_LONG,   10 },		/* 10 BOOLEANs	*/
	{ 0, 0 }
};

sw_array sw_psd_print[] =
{
	{ SW_LONG,   20 },		/* 20 BOOLEANs	*/
	{ SW_BYTE,   120 },		/* 5 ascii fields of 24 bytes each */
	{ SW_LONG,   12 },		/* 12 BOOLEANs	*/
	{ 0, 0 }
};

sw_array sw_facs[] =
{
	{ SW_LONG,   7 },		/* 7 longs: pids, ids, etc. */
	{ SW_BYTE,   328 },		/* 3 ascii fields of 256, 64 & 8 bytes */
	{ 0, 0 }
};

sw_array sw_dict_index[] =	/* Dict's index rec is made of 341 2-memb sub-structs. */
{							/* Each sub-struct is:	*/
	{ SW_BYTE,   4 },		/* 4-uchar key string */
	{ SW_HALF,   1 },		/* int16 "page#" of data rec */
	{ 0, 0 }				/* (At null, routine will loop until reach buflen) */
};							/* 2 last bytes in 2048-byte rec are unprocessed, that's
								fine, they are pad-chars.	*/
sw_array sw_kerndata[] =
{
	{ SW_HALF,   33 },		/* 33 words of legal 1sts, 2nds, increment */
	{ SW_BYTE,   6078 },	/* digit_1 left/right, 1980+2048+2048 values.
								NOTE: Kerndata recs can be size 1X, 2X, or 3X.
								6078 is as large or larger than any actual kerndata
								rec, and will guarantee that the routine won't loop
								& convert twice, which would apply SW_HALFs to
								byte-length value data.  NOTE, THIS ONLY WORKS IF:
								-- This array ends with type SW_BYTE, which just
									advances pntr rather than moving chars, and
								-- Kern data never exceeds 3 recs per font. */
	{ 0, 0 }
};

sw_array sw_width[] =
{
	{ SW_HALF,   256 },		/* 255 widths and video for edit display */
	{ SW_BYTE,   18 },		/* Font name in Ascii chars */
	{ SW_HALF,   247 },		/* Other font data (see (MV)file_specs:width.doc) */
	{ 0, 0 }
};

sw_array sw_shape_spec_header[] =
{							/* shape kern shape header(see shape_spec_header */
	{ SW_HALF,   4 },
	{ SW_BYTE,   4 },
	{ SW_HALF,   2 },
	{ SW_BYTE,   16 },
	{ 0, 0 }
};

sw_array sw_px_header[] =
{
	{ SW_HALF,   4 },		/* Resolution #s	*/
	{ SW_LONG,   8 },		/* Bounding/crop box in 20ths	*/
	{ SW_HALF,   2 },		/* Trim pixels	*/
	{ SW_LONG,   2 },		/* (dummies)	*/
	{ SW_HALF,   2 },		/* Source, alignment	*/
	{ SW_BYTE,   2 },		/* Color mask & BPP	*/
	{ SW_HALF,   35 },		/* Various values	*/
	{ SW_BYTE,   380 },		/* (dummies)	*/
	{ SW_HALF,   2 },		/* Scale & format	*/
	{ 0, 0 }
};

sw_array sw_pm_newlayout[] =
{
	{ SW_HALF,   1 },		/* Line-type	*/
	{ SW_BYTE,   64 },		/* even_name[32], odd_name[32] */
	{ SW_HALF,   1 },		/* (dummies)	*/
	{ SW_LONG,   1 },		/* cmus			*/
	{ SW_HALF,   2 },		/* (dummies)	*/
	{ 0, 0 }
};

sw_array sw_pm_stylerec[] = 
{
	{SW_HALF,	1 },		/* Line type	*/
	{SW_BYTE,	32},		/* .ils file name	*/
	{SW_HALF,	17},		/* (dummies)		*/
	{SW_LONG,	1 },		/* cmus				*/
	{SW_HALF,	2 },		/* (dummies)		*/
	{0, 0 },
};

void sw_swap_text_in (uchar* buf, int buflen, sw_array* sw);
void sw_swap_text_out (uchar* buf, int buflen, sw_array* sw);
void sw_swap_default (uchar* buf, int buflen, sw_array* sw);
void sw_swap_pm (uchar* buf, int buflen, sw_array* sw);

sw_block sw_master[] =
{
	{ NULL,				NULL },				/* 0 No conversion needed */
	{ NULL,				NULL },				/* 1 All int16's */
	{ NULL,				NULL },				/* 2 */
	{ NULL,				NULL },				/* 3 */
	{ NULL,				NULL },				/* 4 */
	{ NULL,				NULL },				/* 5 */
	{ NULL,				NULL },				/* 6 */
	{ NULL,				NULL },				/* 7 */
	{ NULL,				NULL },				/* 8 */
	{ NULL,				NULL },				/* 9 */
	{ sw_swap_default,	sw_header },		/* 10 Textfile rec 0 */
	{ sw_swap_text_in,	NULL },				/* 11 SW_TEXTBLOCK_IN - Textfile text rec */
	{ sw_swap_text_out,	NULL },				/* 12 SW_TEXTBLOCK_OUT - Textfile text rec */
	{ sw_swap_default,	sw_texthblock },	/* 13 header shorts of Textfile text rec */
	{ sw_swap_default,	sw_format },		/* 14 H&J format rec */
	{ sw_swap_default,	sw_gentag },		/* 15 H&J gentag rec */
	{ sw_swap_pm,		NULL },				/* 16 .pm global entry point */
	{ sw_swap_default,	sw_pm_text },		/* 17 .pm text rec */
	{ sw_swap_default,	sw_pm_insert },		/* 18 .pm insert rec */
	{ sw_swap_default,	sw_pm_string },		/* 19 .pm xt-string rec */
	{ sw_swap_default,	sw_pm_str2 },		/* 20 .pm xt-continued rec */
	{ sw_swap_default,	sw_psd_setup },		/* 21 .psd printer descr. setup rec */
	{ sw_swap_default,	sw_psd_print },		/* 22 .psd printer descr. print rec */
	{ sw_swap_default,	sw_facs },			/* 23 Rec from .facs1.list */
	{ sw_swap_default,	sw_dict_index },	/* 24 Index rec from hyp dictionary */
	{ sw_swap_default,	sw_kerndata },		/* 25 (ex)kerndata rec */
	{ sw_swap_default,	sw_width },			/* 26 Width file rec */
	{ sw_swap_default,	sw_shape_spec_header }, /* 27 Shape kern shape header*/
	{ sw_swap_default,	sw_px_header },		/* 28 .PX-file header rec */
	{ sw_swap_default,	sw_styletag },		/* 29 H&J styletag rec */
	{ sw_swap_default,	sw_pm_newlayout },	/* 30 .pm new-layout rec */
	{ sw_swap_default,	sw_pm_stylerec },	/* 31 .pm ill-stylename rec */
};


/*
General-purpose byte-swapper routine.  Accepts data buffer to be converted, and
pointer to conversion-guide array for the particular record-type.
*/
void sw_swap_default (uchar* buf, int buflen, sw_array* sw)
{
	union { uchar *bp; uint16 *hp; } u;
	sw_array* swp;
	uchar *buf_save;
	int buf_tot, buf_pass;
	int jj;

	if( !sw || !buf)
		return;

	buf_tot = 0;					/* Track total # bytes converted	*/
	u.bp = buf;

	do
	{
		buf_save = u.bp;
		for ( swp = sw; swp->type; swp++)
			switch (swp->type) {
			case SW_BYTE:
				u.bp += swp->count;
				break;
			case SW_HALF:
				for (jj = swp->count; --jj >= 0; u.hp++)
					*u.hp = swap16 (*u.hp);
				break;
			case SW_LONG:
				/* The only sw_array that currently contains longs
				 * is sw_header. The longs in this structure
				 * are NOT necessarily aligned on 4-byte
				 * boundaries in memory,
				 * so we canot use swap32().
				 */
				for (jj = swp->count; --jj >= 0; u.bp += 4)
				{
					uchar cc;
					cc = u.bp[0];
					u.bp[0] = u.bp[3];
					u.bp[3] = cc;
					cc = u.bp[1];
					u.bp[1] = u.bp[2];
					u.bp[2] = cc;
				}
				break;
			}
		buf_pass = u.bp - buf_save;	/* Calc size just converted. */
		buf_tot += buf_pass;		/* Total converted so far.	*/

		/* If there are 1+ more recs (based on
		   length just converted) left in the
		   buf, loop and do again:	*/
	}	while (buf_tot+buf_pass <= buflen); 
}


/*
Text rec from textfile has a fluid word/byte structure, so convert dynamically.
*/
void sw_swap_text_in (uchar* buf, int buflen, sw_array* sw)
{
	uint16 *hp0, *hp, *lwib;
	int jj;
	
#if 0
	(void) buflen;
	(void) sw;
#endif

	hp0 = hp = (uint16*)buf;

	/* Swap text header shorts.
	 * See struct tx_block in txfile.h.
	 */
	for (jj = 6; --jj >= 0; hp++)
		*hp = swap16 (*hp);

	/* Get pointer to last word in block.
	 * Look at the LWIB macro in txfile.h
	 * to see how this is derived.
	 * On input, this must be calculated
	 * AFTER the text header shorts have been
	 * byte-swapped.
	 */
	lwib = &hp0[6] + hp0[3] - 6;

	/* Scan thru text buffer (i.e., txb_buff[500]).
	 * The buffer is all bytes with two exceptions:
	 * 1) Each line of text bytes is preceded by
	 *    a line start header (see struct tx_line in
	 *    in txfile.h) comprised of 8 shorts. The first
	 *    short (tx_line.txl_minus_4) always contains
	 *    the value for '-4' as stored on a big-endian cpu.
	 *    This first short, defined as TX_EOL / TX_BOL,
	 *    is never byte-swapped.
	 * 2) The end-of-text is marked with the value '-3'
	 *    as stored on a big-endian cpu. This short,
	 *    defined as TX_EOF, is never byte_swapped. This
	 *    end-of-text marker occurs only once, in the last
	 *    block of text.
	 * Both the TX_EOL and TX_EOF must start on an even
	 * byte in the text buffer.
	 */
	for (;;)
	{
		/* Skip past text stopping AFTER
		 * a TX_EOL is encountered. 
		 */
		while (*hp++ != TX_EOL)
			if (hp > lwib)
				return;
			
		/* Byte_swap the last 7 shorts
		 * of the line start header.
		 */
		for (jj = 7; --jj >= 0; hp++)
			*hp = swap16 (*hp);
	}
}


void sw_swap_text_out (uchar* buf, int buflen, sw_array* sw)
{
	uint16 *hp, *lwib;
	int jj;
	
#if 0
	(void) buflen;
	(void) sw;
#endif

	hp = (uint16*)buf;

	/* See sw_swap_text() above.
	 * This routine is identical to it
	 * except that on output, 'lwib' must
	 * be calculated BEFORE the text header
	 * shorts have been byte-swapped.
	 */
	lwib = &hp[6] + hp[3] - 6;

	for (jj = 6; --jj >= 0; hp++)
		*hp = swap16 (*hp);

	for (;;)
	{
		while (*hp++ != TX_EOL)
			if (hp > lwib)
				return;
			
		for (jj = 7; --jj >= 0; hp++)
			*hp = swap16 (*hp);
	}
}


/*
Read a .pm-file record (H&J and MasterPage).  Rec-type is unknown before we
read it in.  Determine rec-type, call appropriate conversion based on it.
*/
void sw_swap_pm (uchar* buf, int buflen, sw_array* sw)
{
	uint16 line_type;

	if (!buf) return;				/* (Return no-op on illegal call)	*/

									/* 1st 2 bytes of rec is int16 rec-type. */
									/* Determine whether reading or writing:
									MSB will always be 0 or 0377, and LSB will
									never be, except in the case or rec-type 0,
									where it doesn't matter.			*/
	if (buf[0] == 0 || buf[0] == 0377)	/* MSB is on left: writing.	*/
		line_type = (buf[0] << 8) + buf[1];
	else								/* MSB is on right: reading. */
		line_type = (buf[1] << 8) + buf[0];

	switch ((int16)line_type)		/* Branch on reliable record-type: */
	{
	case 43:						/* Illustration ref	*/
	case 44:						/* Table ref	*/
	case 45:						/* Footnote ref	*/
	case 61:						/* Sidenote ref	*/
	case -9:						/* Table boxhead to repeat	*/
		sw_swap_default (buf, buflen, sw_pm_insert);
		break;
	case -101:						/* [XT string	*/
		sw_swap_default (buf, buflen, sw_pm_string);
		break;
	case -102:						/* [XT string continuation	*/
		sw_swap_default (buf, buflen, sw_pm_str2);
		break;
	case 8:							/* [NL New-layout names record	*/
		sw_swap_default (buf, buflen, sw_pm_newlayout);
		break;
	case 10:						/* Illustration-stylename from [TM */
		sw_swap_default (buf, buflen, sw_pm_stylerec);
		break;
	default:						/* Text records (0 & -15)	*/
		sw_swap_default (buf, buflen, sw_pm_text);
		break;
	}
}


/*
This is the entry point into Byte-Swap World, called from p_read & p_write.
*/
void byte_swap (void* buf, int buflen, int sw_type)
{
	if (sw_type == SW_INT16S)
	{
		uint16 *hp;
		int jj;
			
		for (hp = (uint16*)buf, jj = buflen >> 1; --jj >= 0; hp++)
			*hp = swap16 (*hp);
	}
	else if (sw_type == SW_INT32S)
	{
		uint32 *wp;
		int jj;
			
		for (wp = (uint32*)buf, jj = buflen >> 2; --jj >= 0; wp++)
			*wp = swap32 (*wp);
	}
	else if (sw_type > 1 &&
			 sw_type < sizeof sw_master / sizeof (sw_block) &&
			 sw_master[sw_type].func)
		(*sw_master[sw_type].func)(buf, buflen, sw_master[sw_type].sw);
}


void* byte_swap_copy (void* buf, int buflen, int sw_type)
{
	static int mlen = 0;
	static void *mbuf = NULL;

	if (buflen > mlen)
	{
		mbuf = p_realloc (mbuf, buflen);
		mlen = buflen;
	}

	memcpy (mbuf, buf, buflen);
	buf = mbuf;
	
	if (sw_type == SW_INT16S)
	{
		uint16 *hp;
		int jj;
			
		for (hp = (uint16*)buf, jj = buflen >> 1; --jj >= 0; hp++)
			*hp = swap16 (*hp);
	}
	else if (sw_type == SW_INT32S)
	{
		uint32 *wp;
		int jj;
			
		for (wp = (uint32*)buf, jj = buflen >> 2; --jj >= 0; wp++)
			*wp = swap32 (*wp);
	}
	else if (sw_type > 1 &&
			 sw_type < sizeof sw_master / sizeof (sw_block) &&
			 sw_master[sw_type].func)
		(*sw_master[sw_type].func)(buf, buflen, sw_master[sw_type].sw);

	return mbuf;
}

#endif
