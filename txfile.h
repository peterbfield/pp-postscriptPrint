#ifndef _TXFILE_H_
#define _TXFILE_H_

/* EDITORIAL SYSTEM TEXT FILE LAYOUT
 *
 * When text files are being edited on the terminals, text is
 * kept in a file that is very similar to a Pentaquick .TX file.
 * This file consists of a header 512 bytes long. The actual
 * text starts in the second disk block and is maintained in
 * a bi-directionally linked list of disk blocks.
 *
 * The following structures define the layouts of each of these
 * block types. The meanings of all the words in the line start
 * group and all passthru characters are also given.
 *
 * NOTE - XXXX marks changes made in this file when ported to
 * the Aviion from the MV.
 * The GetOdd32() and PutOdd32() macros and the odd32 typedef
 * are also porting changes.
 * These change comments were added July 94 by CWR.
 */


typedef struct Odd32
{
	uint16 hi16;	/* hi order 16 bit of 32 bit word */
	uint16 lo16;	/* lo order 16 bit of 32 bit word */
}	odd32;

#define GetOdd32(i)			((i.hi16 << 16) | i.lo16)
#define PutOdd32(i,j)		(void)(i.hi16 = ((j) >> 16), i.lo16 = ((j) & 0x0000ffff))
#define GetOdd32Ptr(i)		((i->hi16 << 16) | i->lo16)
#define PutOdd32Ptr(i,j)	(i->hi16 = (j >> 16), i->lo16 = (j & 0x0000ffff))

/* This structure defines the history data for the file. */
struct hist
{
		int16 s_rno;					/* revision number (not used) */
		int16 s_sno;					/* session number (not used) */

#define TX_EDIT_TIME(p)					(GetOdd32(p->h_hst.s_edtm))
#define PUT_TX_EDIT_TIME(p,v)			(PutOdd32(p->h_hst.s_edtm,v))
		odd32 s_edtm;					/* editing time in seconds */

		int16 s_date;					/* scalar date (not used) */
		int16 s_time;					/* scalar time (not used) */

#define TX_FILE_MOD(p)					(p->h_hst.s_mod)
		int16 s_mod;					/* file has been modified this session */

		odd32 s_nstrokes;				/* number of keystrokes (not used) */
		odd32 s_ninserts;				/* number of inserts (not used) */
		odd32 s_ndeletes;				/* number of deletes (not used) */
		odd32 s_ncommands;				/* number of commands (not used) */
		odd32 s_npchars;				/* number of printable characters (not used) */
		odd32 s_nachars;				/* number of ascii characters (not used) */

#define TX_TEXT_HILINE(p)				(p->h_hst.s_hiline)
		int16 s_hiline;					/* highest line number in file */

		odd32 s_depth;					/* depth of file in MRU's (not used) */
		int16 s_opno;					/* operator number for PVT's (not used) */
};


/* This structure defines the main header block of the file. */
struct tx_head
{
#define TX_FILE_REV(p)					(p->h_frev)
#define TX_TEXT_REV						-2				/* curr text rev number*/
		int16 h_frev;					/* File revision number */

#define TX_EDIT_TRACE(p)				(p->edit_trace)
		int16 edit_trace;				/* Edit Trace flags -- see below */

		/* Following is a description of the edit trace flags:
		 *
		 *    C        Edit-Off       Trace   U       Legal           Next
		 *    F   0  ---Action--  0  -Type--  K  ----Actions----  0  Action-
		 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
		 *  |   | 0 |   |   |   | 0 |   |   |   |   |   |   |   | 0 |   |   |
		 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
		 *    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
		 */

		#define TX_ET_CFMASK	0x8000	/* Old <-> New style conversion flag: 0=old  1=new */

		/* Old style:
		 *	left byte:
		 *		edit: 0=release  1=merge
		 *		comp: none (composition before Rev.16.20 keeps no edit trace)
		 *	rite byte:
		 *		edit: 0=release  1=merge
		 *		comp: has no meaning
		 *
		 * Revision numbering:
		 *		comp: LT 1; usu. 0 unless set to -1 by SATURN_CREATE (16.20).
		 *		edit: GE 1; always.
		 *
		 * NOTE as of Rev.16.20:
		 *		CASCADE replaces RELEASE in both comp and edit.
		 *		RELEASE is no longer used because of inconsistent meanings.
		 *
		 * (XXXX RED-LINING was PROOFTRACE)
		 * RED-LINING is defined as accumulate current edits
		 * (the UNDO key is deactivated) until either MERGE or
		 * CASCADE or DROP is executed through the EXEC key.
		 * When a file is created the initial edit trace
		 * surrounding the file is dropped.
		 * Next-Action is set to NO ACTION and
		 * the Edit-Off action is set to NO ACTION.
		 */

		#define TX_ET_EOMASK	0x3800		/* Edit-off action mask for bits 2 - 4. */
		#define TX_EO_NOACT		0x0000		/*   0x0000 = NO ACTION */
		#define TX_EO_DROP		0x0800		/*   0x0800 = DROP edit trace */
		#define TX_EO_CASCADE	0x1000		/*   0x1000 = CASCADE edit trace */
		#define TX_EO_MERGE		0x1800		/*   0x1800 = MERGE edit trace */

		#define TX_ET_TYMASK	0x0300		/* Edit-trace type mask for bits 6 - 7. */
		#define	TX_TY_NONE		0x0000		/*   0x0000 = NONE (drop edit trace) */
		#define TX_TY_CASCADE	0x0100		/*   0x0100 = CASCADE edit trace */
		#define TX_TY_MERGE		0x0200		/*   0x0200 = MERGE edit trace */
		#if 0	/* (XXXX) */
		#define TX_TY_PROOF		0x0300		/*   0x0300 = PROOFTRACE (accumulate current edits) */
		#else
		#define TX_TY_REDLINE	0x0300		/*   0x0300 = RED-LINE (accumulate current edits above) */
		#endif

		#define TX_ET_UNDOMASK	0x0080		/* UNDO key mask for bit 8: 1 = disable, 0 = enable. */

		#define TX_ET_LAMASK	0x0078		/* Legal-action mask for bits 9 - 12 (9 - C).
											 * Not presently used -- needs a "permissions" file
											 * to determine who can override legal actions. */
		#define TX_LA_NONE		0x0000		/*   0x0000 = NONE */
		#define TX_LA_DROP		0x0008		/*   0x0008 = DROP */
		#define TX_LA_CASCADE	0x0010		/*   0x0010 = CASCADE */
		#define TX_LA_MERGE		0x0020		/*   0x0020 = MERGE */
							 /* 0x0040 */   /*   undefined */

		#define TX_ET_NAMASK	0x0003		/* Next-action mask for bits 14 - 15 (E - F). */
		#define TX_NA_DROP		0x0000		/*   0x0000 = DROP edit trace */
		#define TX_NA_CASCADE	0x0001		/*   0x0001 = CASCADE edit trace */
		#define TX_NA_MERGE		0x0002		/*   0x0002 = MERGE edit trace */
		#define TX_NA_NOACT		0x0003		/*   0x0003 = NO ACTION (accumulate Level-1 ET) */


#define TX_HISTORY(p)					(p->h_hst)
		struct hist h_hst;				/* File history record (see above) */

		int16 h_u2[41];					/* Unused */

#define TX_ARCHIVE(p)					(p->h_archive)
		int16 h_archive;				/* file is archived */

#define TX_READ_ONLY(p)					(p->h_ronly)
		int16 h_ronly;					/* file is read only */

#define TX_MAX_DEPTH(p)					(GetOdd32(p->h_mdepth))
#define PUT_TX_MAX_DEPTH(p,val)			(PutOdd32(p->h_mdepth,val))
		odd32 h_mdepth;					/* max depth allowed */

#define TX_DEPTH_UNITS(p)				(p->h_units)
		int16 h_units;					/* units for reporting depth */

		#if 0	/* XXXX */
		int16 h_u3;						/* unused.
										 * on QualiType, this word is now h_version_flags:
										 * &1 = new treatment of ligs at EOL */
		#else
		int16 h_version_flags;			/* unused */
		#endif

#define TX_READABILITY(p)				(GetOdd32(p->h_rdby))
#define PUT_TX_READABILITY(p,val)		(PutOdd32(p->h_rdby,val))
		odd32 h_rdby[16];				/* readability levels in 100ths */

#define TX_BLK_STS(p)					(GetOdd32(p->bdsts))
#define PUT_TX_BLK_STS(p,val)			(PutOdd32(p->bdsts,val))
#define TX_BLK_BEG_STATUS(p)			(GetOdd32(p->bdsts))
#define PUT_TX_BLK_BEG_STATUS(p,val)	(PutOdd32(p->bdsts,val))
		odd32 bdsts;					/* status at start of block */

#define TX_BLK_START_POS(p)				(GetOdd32(p->bdpos))
#define PUT_TX_BLK_START_POS(p,val)		(PutOdd32(p->bdpos,val))
		odd32 bdpos;					/* starting Position for block */

#define TX_BLK_START_LINE(p)			(p->bdlno)
		int16 bdlno;					/* starting line # for block */

#define TX_BLK_DEF(p)					(p->blk_def)
		int16 blk_def;					/* block is currently defined */

#define TX_BLK_END_POS(p)				(GetOdd32(p->edpos))
#define PUT_TX_BLK_END_POS(p,val)		(PutOdd32(p->edpos,val))
		odd32 edpos;					/* ending position for block */

#define TX_BLK_END_LINE(p)				(p->edlno)
		int16 edlno;					/* ending line # for block */

#define TX_BLK_END_STATUS(p)			(GetOdd32(p->edsts))
#define PUT_TX_BLK_END_STATUS(p,val)	(PutOdd32(p->edsts,val))
		odd32 edsts;					/* status at end of block */

		int16 u4[35];					/* unused */

#define SYSCON							0		/* systems standards and configuration */
#define NFBLK							25
#define TX_SYS_CON(p)					(p->h_fblk[SYSCON])
		int16 h_fblk[NFBLK];			/* indexes (block numbers) to fixed record types */


#define FTEXT							0		/* first text sector */
#define NFCHAINS						15
#define TX_F_TEXT(p)					(p->h_fchains[FTEXT])
		int16 h_fchains[NFCHAINS];		/* indexes to chain-linked blocks
										 *	[0] = rev link		(XXXX mv)
										 *	[1] = unused		(XXXX mv)
										 *	[2] = fwd link		(XXXX mv)
										 *	[3] = size			(XXXX mv) */


#define TX_SCRATCH(p)					(p->h_scratch)
		int16 h_scratch[31];			/* scratch area */

#define TX_VISION(p)					(p->h_vision)
		int16 h_vision;					/* -1 = visionable, 0 = not */

		odd32 h_curtime;				/* (XXXX added) */
		int16 h_sgml_flag;				/* 0=regular 1=sgmlfile  2=sgmlfloatfile
										 *           3=XML file  4=XML floatfile */
		int16 h_no_hnj_lines;			/* 1=line start groups stripped; hnj will recreate */
		int16 h_U6[15];					/* unused (XXXX was 19) */
		int16 h_mag_mem;				/* non zero = member of magazine */
		int16 h_mag_errors;				/* number of magazine errors */
		int16 h_mag_long_short;			/* number of lines long or short */
		int16 h_fart;					/* first article line */
		int16 h_lart;					/* last article line */

#define TX_SYS_STAND(p)					(p->h_syss)
		int16 h_syss;					/* system standards number */

		int16 h_fllline;				/* flash H&J lowest line number */
		int16 h_fllrec;					/* flash H&J low record number */
		int16 h_flhiline;				/* flash H&J highest line number (XXXX was flhline) */

#define TX_FILE_IN_USE(p)				(p->h_inuse)
		int16 h_inuse;					/* file is in use */

		int16 h_flrev;					/* flash H&J revision number */

#define TX_HI_REC_CREATE(p)				(p->h_hicrec)
		int16 h_hicrec;					/* highest record used at creation */

#define TX_HI_LINE_CREATE(p)			(p->h_hicline)
		int16 h_hicline;				/* highest line number used at creation */

#define TX_HI_REC(p)					(p->h_hirec)
		uint16 h_hirec;					/* current highest record (XXXX was int16) */

#define TX_HI_REC_TEXT(p)				(p->h_hitrec)
		uint16 h_hitrec;				/* current highest text record (XXXX was int16) */
};


/* This structure defines FILE NAME BLOCK 1 (XXXX TX_ added to macros) */
struct f_blk1
{
#define TX_HOME_DESK(p)					(p->home_desk)
		char home_desk[128];

#define TX_PRINT_TABLE(p)				(p->print_table)
		char print_table[128];

#define TX_EWB_FORMAT(p)  				(p->ewb_format)
		char ewb_format[128];

#define TX_FILENAME(p)					(p->filename)
		char filename[32];

#define TX_COMP_FILENAME(p)				(p->comp_filename)
		char comp_filename[32];

		char blk1_unused[64];
};


/* This structure defines FILE NAME BLOCK 2  (XXXX TX_ added to macros) */
struct f_blk2
{
#define TX_ZIPS(p)           			(p->named_zip)    
		char named_zip[128];

#define TX_FORMATS(p)        			(p->format_file)
		char format_file[128];

#define TX_STYLE_INTERCHANGE(p)			(p->style_interchange)
		char style_interchange[128];

#define TX_GENCODE_FORMATS(p)  			(p->gencode_formats)   
		char gencode_formats[128];
};


/* This structure defines the auto route block (XXXX TX_ added to macros) */
struct auto_route
{
#define TX_AUTO_ROUTE(p)				(p->auto_route_name)
		char auto_route_name[32];

#define TX_AUTO_STEP(p)      			(p->autostep)
		int16 autostep;					/* current step in route */

#define TX_AUTO_REC(p)					(p->autorec)							
		uint16 autorec;					/* rec num in autoroute timefile */

		char ar_unused[476];
};


/* This structure defines the keyword blocks */
struct key_blk
{
#define KEY_REV_LNK(p)					(p->key_rev)
		int16 key_rev;					/* reverse link to previous keywords block */

#define KEY_CNT_BLK(p)					(p->key_count)
		int16 key_count;				/* number of keywords in block */

#define KEY_FWD_LNK(p)					(p->key_fwd)
		int16 key_fwd;					/* forward link to next keywords block */

#define KEY_WORDS(p)					(p->keywords)
		char keywords[506];				/* keywords and phrases, delimited by NULL */
};


/* This structure defines the XY chain of blocks in the file */
struct tx_xys
{
		int16 txb_rev;			/* reverse link sector number. Must be
								 * zero in the first text block. */
		uint16 txb_hln;			/* Highest line # to start this block */
		int16 txb_fwd;			/* forward link sector number. Must be
								 * zero in the last logical (as opposed
								 * to physical) text block. */
		int16 txb_size;			/* Number of 16-bit words used in this
								 * block, including all header words */
		char xynames[4][126];	/* four xy file names. */
};


/* page edit blocks */
struct pe_record
{
		int16 slin_start;		/* source starting line.(tape) (XXXX was sline_start) */
		int16 slin_stop;		/* source stopping line.(tape) (XXXX was sline_stop) */
		int16 dlin_start;		/* destination starting line.(page) (XXXX was dline_start) */
		char sourcetape[58];	/* A source tape that page edit got it's text from. */
};


struct pe_blk
{
		int16 pe_rev;			/* reverse block link */
		int16 pe_count;			/* no of records in blk */
		int16 pe_fwd;			/* forward block link */
#define PE_MAX_RECS		7
		struct pe_record pe_records[PE_MAX_RECS];
		int16 junk[29];
};


/* the remaining text blocks in the file */
struct tx_block
{
#define REVERSE(p)						(p->txb_rev)
		int16 txb_rev;					/* reverse link sector number. Must be
										 * zero in the first text block. */

#define BLK_HILINE(p)					(p->txb_hln)
		uint16 txb_hln;					/* Highest line # to start this block */

#define FORWARD(p)						(p->txb_fwd)
		int16 txb_fwd;					/* forward link sector number. Must be
										 * zero in the last logical (as opposed
										 * to physical) text block. */

#define BLK_SIZE(p)						(p->txb_size)
		int16 txb_size;					/* Nr of words used in block, including header */

#define TEXT_START(p)					(p->txb_start)
		int16 txb_start;				/* 1-based word index to start of text */

#define LAST_FLASH(p)					(p->txb_lstat)
		int16 txb_lstat;				/* Last sector # with flash status */

#define BUFFER(p)						(p->txb_buff)
		unsigned char txb_buff[500];	/* All text and status (XXXX was char) */
		#define FWIB(p)					((int16*)BUFFER(p) + TEXT_START(p) - 7)	/* first word in block */
		#define LWIB(p)					((int16*)BUFFER(p) + BLK_SIZE(p) - 6)	/* last word in block */
		#define MAXBYTE(p)				(((BLK_SIZE(p) - 7) << 1) + 1)			/* last byte in block */
		#define txb_lastbyte(size)		(((size - 7) << 1) + 1)					/* last byte in block */
		#define START_BYTE(p)			((TEXT_START(p) - 7) << 1)				/* first byte in block */
		#define TX_BLOCK_SIZE			9										/* (XXXX TX_ added) */
		/* #define SIZE_OF_BLK(nbytes)	((nbytes >> 1) + 6)  (XXXX) */
};


/* EOL and EOF defines: SWAP THESE BYTES IF YOU ARE NOT RUNNING ON AN IBM PC. (XXXX added) */
#define TX_ff	0xff
#define TX_eol	0xfc
#define TX_eof	0xfd
#ifdef i386
#define TX_BOL  0xfcff		/* -4 */
#define TX_EOL	0xfcff		/* -4 */
#define TX_EOF  0xfdff		/* -3 */
#else
#define TX_BOL	0xfffc		/* -4 */
#define TX_EOL	0xfffc		/* -4 */
#define TX_EOF  0xfffd		/* -3 */
#endif

/* Get the next tx_block in a chain, freeing the current block and seting
 * values for file position, first and last bytes in block
 */
#if 0	/* XXXX */
#define NEXT_BLK(ptr,chan,filepos,numread,link) \
		{ \
		filepos = ptr->link << BLOCK_SIZE; \
		fseek(chan, filepos, 0); \
		numread = fread((char*)ptr, sizeof(struct tx_block), 1, chan); \
		}
#else
#define NEXT_BLK(bk,fp,sb,mb) \
		{ \
		tx_free(bk); \
		fp = (uint32) FORWARD(bk) << 9; \
		bk = tx_rblk(CHANNEL(cb), fp); \
		sb = START_BYTE(bk); \
		mb = MAXBYTE(bk); \
		}
#endif

/* #define WORD_PTR(p,nbytes)	(int16 *)(BUFFER(p) + nbytes) (XXXX) */

/* Convert to block pointer and offset */
#if 0	/* XXXX */
#define	CONVERT(op,fp,sb) \
		{ \
		fp = op; \
		sb = (fp & 0x1FF); \
		fp -= sb; \
		}
#else
#define	CONVERT(op,fp,sb) \
		{ \
		fp = op; \
		sb = (uint16)(fp & 0x1FF); \
		fp -= (uint32)sb; \
		}
#endif


/* Line start header. This header is always aligned on a
 * 16-bitword boundary in the file. Append a null
 * character to the previous line if necessary to force
 * this alignment.  (XXXX txl_ was l_)
 */
struct tx_line
{
#define MINUS_4(l)						(l->txl_minus_4)
		int16 txl_minus_4;				/* Always a -4 */

#define LINUM(l)						(l->txl_linum)
		int16 txl_linum;				/* line number (XXXX was uint16) */

#define LN_STATUS(l)					(l->txl_status)
		int16 txl_status;				/* H&J status of line */
		#define LS_CMD_ERR		0x0001
		#define LS_LINTOUCH		0x0002	/* Line modified */
		#define LS_SPOT_HJ		0x0020	/* Automatic spot H&J modifications */
		#define LS_LSVALLO		0x01c0	/* Letsp/Kern value (abs), bottom 3 bits */
		#define LS_AA			0x0200	/* AA modifications */
		#define LS_PE			0x0400	/* PE modifications */
		#define LS_LSVALHI		0x3800	/* Letsp/Kern value (abs), top 3 bits */
		#define GetLSVAL(i)		(((i & LS_LSVALHI) >> 8) | ((i & LS_LSVALLO) >> 6))
		#define BuildLSVAL(i)	(((i << 8) & LS_LSVALHI) | ((i << 6) & LS_LSVALLO))
		#define LS_LETTERSPACE	0x4000
		#define LS_KERN			0x8000

#define L_INDENT(l)						(l->txl_lindent)
		int16 txl_lindent;				/* indent left */

#define R_INDENT(l)						(l->txl_rindent)
		int16 txl_rindent;				/* indent right */

#define BAND_DR(l)						(l->txl_band)
		int16 txl_band;					/* band drive */

#define LEAD(l)							(l->txl_lead)
		int16 txl_lead;					/* leading of the line */

#define FLAGS(l)						(l->txl_vflags)
		int16 txl_vflags;				/* video & editorial context status flags */
		#define VFVIDEO	0x003f			/* mask isolates all video bits */
		#define VFCCMND 0x0080			/* Composition Command */
		#define VFLSMOD 0x0100			/* secondary mode */
		#define VFLEMSG 0x0200			/* error message */
		#define VFL1INS 0x0400			/* level 1 insert */
		#define VFL2INS 0x0800			/* level 2 insert */
		#define VFL1DEL 0x1000			/* level 1 deletion */
		#define VFL2DEL 0x2000			/* level 2 deletion */
#if EDAUTH
		#define VFENOTE 0x4000			/* note to editor */
		#define VFANOTE 0x8000			/* note to author */
#endif
};


/* how H&J will interpret the above passthrus (XXXX mv only)
 *
 * 0080 0040 020 010
 * 0200 0100 040 020   (dec oct hx - dec oct hx) bits
 *  1    0    0   0    (129 201 81 - 143 217 8f) video
 *  1    1    0   0    (192 300 c0 - 207 317 cf) strip and recreate
 *  1    1    0   1    (208 320 d0 - 223 337 df) strip to end and recreate
 *  1    1    1   0    (224 340 e0 - 239 357 ef) pass-on
 *  1    1    1   1    (240 360 f0 - 255 377 ff) pass-on, ignore to end
 */

/*
129 201 81 - 143 217 8f 1000
144 220 90 - 159 237 9f 1001
160 240 a0 - 175 257 af 1010
176 260 b0 - 191 277 bf 1011

192 300 c0 - 207 317 cf 1100
208 320 d0 - 223 337 df 1101
224 340 e0 - 239 357 ef 1110
240 360 f0 - 255 377 ff 1111
*/

/* The following defines give the meaning of all passthru
 * characters as well as a few generally useful masks
 */
#define PASSTHRU 0200			/* (128 200 80 1000) bit mask -- char is a passthru */
#define PASSPASS 0100			/* (0x40) bit on means non-video passthru */
#define PASSNVID 0100			/* (0x40) bit off means video change passthru -
								 *        new video is in the 6 lsb of this char */
						/* strip and recreate: */
								/* (192 300 c0 1100) */
#define PASSBCMD 0301			/* (193 301 c1 1100) begin composition command */
#define PASSECMD 0302			/* (194 302 c2 1100) end composition command */
#define PASSTBNT 0303			/* (195 303 c3 1100) next tab */
#define PASSTBBT 0304			/* (196 304 c4 1100) begin tab */
#define PASSBSMD 0305			/* (197 305 c5 1100) begin secondary mode */
#define PASSESMD 0306			/* (198 306 c6 1100) end secondary mode */
#define PASSTBDS 0307			/* (199 307 c7 1100) define straddle */
#define PASSTBET 0310			/* (200 310 c8 1100) end tab */
#define PASSHJLE 0311			/* (201 311 c9 1100) H&J line ending */
#define PASSHJEP 0312			/* (202 312 ca 1100) end para */
#define PASSHJQL 0313			/* (203 313 cb 1100) quad left */
#define PASSHJQR 0314			/* (204 314 cc 1100) quad right */
#define PASSHJQC 0315			/* (205 315 cd 1100) quad centre */
#define PASSHJFJ 0316			/* (206 316 ce 1100) force justify */
								/* (207 317 cf 1100) */
						/* strip to end and recreate: */
								/* (208 320 d0 1101) */
#define PASSBEMS 0321			/* (209 321 d1 1101) begin error message */
#define PASSEEMS 0322			/* (210 322 d2 1101) end error message */
#if NEWSGML
#define PASSBPTH 0323			/* (211 323 d3 1101) begin SGML path */
#define PASSEPTH 0324			/* (212 324 d4 1101) end SGML path */
#define PASSBTAG 0325			/* (213 325 d5 1101) begin SGML tag */
#define PASSETAG 0326			/* (214 326 d6 1101) end SGML tag */
#define PASSBETY 0327			/* (215 327 d7 1101) begin SGML entity */
#define PASSEETY 0330			/* (216 330 d8 1101) end SGML entity */
#define PASSBCOD 0331			/* (217 331 d9 1101) begin SGML code */
#define PASSECOD 0332			/* (218 332 da 1101) end SGML code */
#define PASSBFLT 0333			/* (219 333 db 1101) begin SGML Floating element */
#define PASSEFLT 0334			/* (220 334 dc 1101) end SGML Floating element */
#endif
								/* (221 335 dd 1101) */
								/* (222 336 de 1101) */
								/* (223 337 df 1101) */
						/* pass-on: */
								/* (224 340 e0 1110) */
#define PASSRFMK 0341			/* (225 341 e1 1110) next char is a reference mark */
#define PASSFEOL 0342			/* (226 342 e2 1110) next char is forced end of line */
#define PASS1BIN 0343			/* (227 343 e3 1110) level 1 begin insert */
#define PASS1EIN 0344			/* (228 344 e4 1110) level 1 end insert */
#define PASS2BIN 0345			/* (229 345 e5 1110) level 2 begin insert */
#define PASS2EIN 0346			/* (230 346 e6 1110) level 2 end insert */
								/* (231 347 e7 1110) */
								/* (232 350 e8 1110) */
								/* (233 351 e9 1110) */
								/* (234 352 ea 1110) */
								/* (235 353 eb 1110) */
								/* (236 354 ec 1110) */
								/* (237 355 ed 1110) */
								/* (238 356 ee 1110) */
								/* (239 357 ef 1110) */
						/* pass-on, ignore to end */
								/* (240 360 f0 1111) */
#define PASS1BDE 0361			/* (241 361 f1 1111) level 1 begin delete */
#define PASS1EDE 0362			/* (242 362 f2 1111) level 1 end delete */
#define PASS2BDE 0363			/* (243 363 f3 1111) level 2 begin delete */
#define PASS2EDE 0364			/* (244 364 f4 1111) level 2 end delete */
#if EDAUTH
#define PASSBEDN 0365			/* (245 365 f5 1111) begin editor's note */
#define PASSEEDN 0366			/* (246 366 f6 1111) end editor's note */
#define PASSBATN 0367			/* (247 367 f7 1111) begin author's note */
#define PASSEATN 0370			/* (248 370 f8 1111) end author's note */
#endif
								/* (249 371 f9 1111) */
								/* (250 372 fa 1111) */
								/* (251 373 fb 1111) */
								/* (252 374 fc 1111) */
								/* (253 375 fd 1111) */
								/* (254 376 fe 1111) */
								/* (255 377 ff 1111) */


/* list of special editorial character values (XXXX mv only)
 *
 *  1  1 = rubout
 *  2  2 = paragraph
 *  3  3 = quad right
 *  4  4 = quad left
 *  5  5 = quad center
 *  6  6 = em space
 *  7  7 = en space
 *  8  8 = thin space
 *  9  9 = begin note
 * 10  a = end note
 * 11  b = +1 relative unit
 * 12  c = -1 relative unit
 * 13  d = em dash
 * 14  e = en dash
 * 15  f = sgml <
 * 16 10 = sgml >
 * 17 11 = sgml (
 * 18 12 = sgml >
 * 19 13 = sgml .
 * 20 14 = sgml ,
 * 21 15 = sgml %
 * 22 16 = sgml #
 * 23 17 = 
 * 24 18 = 
 * 25 19 = 
 * 26 1a = 
 * 27 1b = 
 * 28 1c = 
 * 29 1d = 
 * 30 1e = 
 * 31 1f = 
 * 32 20 = ascii space
 */
#define LOWnull			0x00
#define LOWrubout		0x01
#define LOWEndPara		0x02
#define LOWQuadRight	0x03
#define LOWQuadLeft		0x04
#define LOWQuadCenter	0x05
#define LOWEmSpace		0x06
#define LOWEnSpace		0x07
#define LOWThinSpace	0x08
#define LOWBegComment	0x09
#define LOWEndComment	0x0a
#define LOWPlusRU		0x0b
#define LOWMinusRU		0x0c
#define LOWEmDash		0x0d
#define LOWEnDash		0x0e
#define LOWGMLtago		0x0f
#define LOWGMLtagc		0x10
#define LOWGMLOpenParen	0x11
#define LOWGMLCloseParen	0x12
#define LOWGMLPeriod	0x13
#define LOWGMLComma		0x14
#define LOWGMLPercent	0x15
#define LOWGMLPound		0x16
#endif
