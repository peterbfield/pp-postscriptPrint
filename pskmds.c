#include <stdio.h>
#include "p_lib.h"
#include "psjob.h"
#include "frame.h"
#include "rel_data.h"

extern double sqrt (double x);
extern double asin (double x);

static void stack_one_char(int ichar, int width);
static void scan_fm2(void);
static void xpos_adj(int16 adjustment);
static void output_delayed_et( void);
static void set_strikethru_rule(void);
static int repeatGlobalfn(int fnglobalnum);
static void storeGlobalfnRef(int fnglobalnum, int reference);

extern void ProcessPdfMarks(int16 nargs);
extern void ProcessEnd();

extern int16 Current_FgColor;	/*  used to save color field for after underscore print  BUG 594q  */
extern int16 Current_FgShade;	/*  used to save shade field for after underscore print  BUG 594q  */
extern int16 total_points;
extern int us_beg_x1;			/* Start X pos of curr unders seg 1 */
extern int us_beg_x2;			/* Start X pos of curr unders seg 2 */
extern int16 usoff[];
extern float uswei[];
extern float Ofy;
extern int PSTabGutterRuleColor[];
extern int PSTabGutterRuleShade[];
extern int CmdFM_FootCount, CmdFM_TextCount;
extern int WidthOfNumeric;
extern int us_start_Ypos[];
extern int prologue_output_flag;
extern int old_freq;
extern int Bol_Ypos;
extern int DoExtractBreaks;		/* 1=Output page breaks in pp/pq extracts.  0=Don't. */
extern int16 LastLinemaxErr;	/* Prevent mult err msgs on Line[] overflow.  */

extern int StartSaveText;		/* Start of saving text for background [sr command */
static int16 cmd_nt_on_the_line_flag;
static int16 cmd_fm_adjustment;
static int16 cmd_fm_flag;		/* don't set text in kmd73 if true */
static int save_cur_Xpos;
static float excess;
static int16 save_vj_pending;	/* Stores vert just amount when in a table 
									to extend [bx...] rules in tabs */
static int16 pdfnargs;


char export_char[2];
char export_name[52];
extern int export_ctr;
int exported=0;
int export_frametype;
extern float export_x;			/* set in psdtxt for frame offset */
extern float export_y;
extern float export_end_x;
extern float export_end_y;
float export_ii=0;
float export_jj=0;
int16 ackhar;					/* Actual char value from .fo */
int16 holdforec, holdfowrd;
int16 wordsp;					/* Total extra interwd in just ln */
int16 nbands;					/* Total # spaces in justified ln */
int16 LsRemainder;				/* Total extra in letterspaced no-band ln */
int16 fill_space;
int16 fill_count;
int16 khar_wid;					/* Input char width. */
int16 RoundoffApply;
int16 underscore_min;
int16 us_min_mru;
int tab_split_index;
int tab_split_index_start;
int tab_split_index_end;
int blend_flag;					/* always set for [bf (kmd80) by kmd64, -9 */
int BlendAngle;
int BlendEndColor;
float BlendEndShade;
int16 anchor_amounts[21];
int anchor_colors[21];
float anchor_shades[21];
int AnchorCount;
int RoundoffBucket;				/* Sum remainders until space band */
int UncompensatedRoundoff;		/* Sum remainders until used */
int16 repeatno;					/* # times to repeat lin in kmd10 */
int save_starting_Xpos, save_khar_wid;
int ZeroSetFlag;					/* 1=Just got [zs.  After next char,
								   reset to Xpos at its start
								   (ie. where [zs occurred.) */
int continued_bf_flag;
int AccentStartFlag;
int AccentEndFlag;
int MathStartFlag;
int StrikeThruRuleStartFlag;
int StrikeThruTop;
int StrikeThruBot;
int StrikeThruWt;
int StrikeThruColor;
int StrikeThruShade;
int StrikeThruTrapWt;
int StrikeThruTrapColor;
int StrikeThruTrapShade;
int StrikeThru_beg_x;
int StrikeThru_start_Top;
int StrikeThru_end_Bot;
int StrikeThruLayer = 0;		/* 0 rule on top, 1 rule behind */
int Kmd_PP_Started;
double StrikeThruRuleAngle;
F_LIST *E_fonts;				/* first font entry for exported fonts */
extern struct fn_global fnglobals[];	/* Max 10  */
extern int num_fnglobals;		/* Bump at each new global fn on the page.  */
extern int flagGlyph;				/* 1=stack() has a glyph ready to print.  */
extern char GlyphName[];			/* Postscript ID of an [OF character (kmd83)  */

/***********************************************************************
 **  STACK(): Add next char to line, and its wid offset to Kspace stack.*
 ***********************************************************************/
void stack(uchar khar)
{
    int16 kval;
    
    if(Oldfont != Holdfont || Oldps != Holdps || Oldss != Holdss ||
       Old_ob  != Hold_ob)
    {
		put_font();				/* Put font change to TP file.
								   Also sets "old-" variables.*/
		do_pskmd('M',"move after stack calls put_font");
    }
	else if (!khar && *GlyphName)
	{
		do_pskmd('M',"move in stack before glyphshow");
	}
		
    kval = fo_line_def.LsPerChar;
	if (LsRemainder > 0)		/* Apply units to first letterspaces */
	{							/*  in line, until remainder used up. */
		kval++;
		LsRemainder--;
	}
	save_starting_Xpos = Xpos;
	save_khar_wid = khar_wid;
	Xpos += khar_wid + kval;	/* Bump X-pos by char wid
								   and by LS adjustment. */
	kval -= RoundoffApply;		/* Pull char back by any accummulated    */
	RoundoffApply = 0;			/*  round-off from previous characters.  */
	if (ackhar > 0)				/* Assuming this is Penta-font char:  */
	{							/*  Add its calculated-to-typeset rounding
									error to the roundoff accumulator.  */
		RoundoffBucket += Widval[Widindex].roundoffs[ackhar-1];
		UncompensatedRoundoff += Widval[Widindex].roundoffs[ackhar-1];
	}
	if ( !cc_mask && !active_trap_cc_mask )
		return;
    if (FlashOn && (khar != 32))	/* Text character or not? */
	{								/* Placing a text character */
		kval += fo_line_def.SolCwAdj; /* It gets tracking value */
		if (!khar)					/* Special: char by PS name instead of loc  */
		{
			flagGlyph = 1;			/* A glyph is ready to print.   */
			Kspace[Linesub+1] += (float )kval/HorizontalBase;
			stack_print();			/* Will branch on flagGlyph  */
			return;
		}
		if (Linesub < 1999)
			Line[++Linesub] = khar;	    
		else if (fo_line_def.LineNum > LastLinemaxErr)
		{
			p_info (PI_WLOG,
				"At line %d: Limit of 2000 characters exceeded -- character dropped\n",
				fo_line_def.LineNum);
			LastLinemaxErr = fo_line_def.LineNum;
		}
		if ( !DontPaintFlag)
			PaintOnPage |= BlackPaint_cc_mask;
	}
    else
		kval += khar_wid;		/* flash-off or a band or fix-space. Adjust next char */
    Kspace[Linesub+1] += (float )kval/HorizontalBase;
}

/***********************************************************************
 **  PS_DRAWLINE();                                                    **
 ***********************************************************************/
void ps_drawline(void)
{
	char trace_string[16];
	int16 ichar;				/* Input char, code. */
	int16 rounding_error;

    int
		kmd0(),kmd1(),kmd2(),kmd3(),kmd4(),kmd5(),kmd6(),kmd7(),kmd8(),
		kmd9(),kmd10(),kmd11(),kmd12(),kmd13(),kmd14(),kmd15(),kmd16(),
		kmd17(),kmd18(),kmd19(),kmd20(),kmd21(),kmd22(),kmd23(),kmd24(),
		kmd25(),kmd26(),kmd27(),kmd28(),kmd29(),kmd30(),kmd31(),kmd32(),
		kmd33(),kmd34(),kmd35(),kmd36(),kmd37(),kmd38(),kmd39(),kmd40(),
		kmd41(),kmd42(),kmd43(),kmd44(),kmd45(),kmd46(),kmd47(),kmd48(),
		kmd49(),kmd50(),kmd51(),kmd52(),kmd53(),kmd54(),kmd55(),kmd56(),
		kmd57(),kmd58(),kmd59(),kmd60(),kmd61(),kmd62(),kmd63(),kmd64(),
		kmd65(),kmd66(),kmd67(),kmd68(),kmd69(),kmd70(),kmd71(),kmd72(),
		kmd73(),kmd74(),kmd75(),kmd76(),kmd77(),kmd78(),kmd79(),kmd80(),
		kmd81(),kmd82(),kmd83(),kmd84();
    
    static int (*kmds[])() =
    {kmd0,kmd1,kmd2,kmd3,kmd4,kmd5,kmd6,kmd7,kmd8,
		 kmd9,kmd10,kmd11,kmd12,kmd13,kmd14,kmd15,kmd16,
		 kmd17,kmd18,kmd19,kmd20,kmd21,kmd22,kmd23,kmd24,
		 kmd25,kmd26,kmd27,kmd28,kmd29,kmd30,kmd31,kmd32,
		 kmd33,kmd34,kmd35,kmd36,kmd37,kmd38,kmd39,kmd40,
		 kmd41,kmd42,kmd43,kmd44,kmd45,kmd46,kmd47,kmd48,
		 kmd49,kmd50,kmd51,kmd52,kmd53,kmd54,kmd55,kmd56,
		 kmd57,kmd58,kmd59,kmd60,kmd61,kmd62,kmd63,kmd64,
		 kmd65,kmd66,kmd67,kmd68,kmd69,kmd70,kmd71,kmd72,
		 kmd73,kmd74,kmd75,kmd76,kmd77,kmd78,kmd79,kmd80,
		 kmd81,kmd82,kmd83,kmd84};
/* NOTE: When adding a new FO cmd processor to lists above, also adjust the
	following special cmd-processing loops in pp:
	
	ps_drawline() "if(ichar <= 84)" statement below.
	pstabs.c:tscan(), add arg count to the foreads[100] array,
		and bump variable foreadsTOP.
	
	ALSO ADJUST FO-cmd processor loops in all other programs that read FO:
		designmaster
		foprint
*/
    
#ifdef TRACE
	int found_char_flag = 0;
    
    if(text_trace)
		p_info(PI_TRACE, "****START LINE**** (%d)\n",fo_line_def.LineNum);
#endif
	cmd_fm_adjustment = 0;
	if (fo_line_def.SolSlant & 1)
		Obdegree = (fo_line_def.SolPointSize *
					0.21255) / HorizontalBase; /* tan(12) = 0.21255 */
	else
		Obdegree = 0;		/* slant switch off */
	if (fo_line_def.SolSlant &0100)
		FlashOn = 0;			/* cancel flash */
	else
		FlashOn = 1;			/* restore flash */
	if((fo_line_def.SolFont != Holdfont) ||
	   (fo_line_def.SolPointSize != Holdps) ||
	   (fo_line_def.SolSetSize != Holdss) ||
	   ( Holdfont == 0) || (Hold_ob != Obdegree) )
    {
		getftab();
		create1psfont(PentaToPsf[PsfNameIndex].name, fo_line_def.SolPointSize,
					  fo_line_def.SolSetSize, Obdegree);
    }
    if((fo_line_def.Quad==1) && (fo_line_def.Bands>0) &&
	   (fo_line_def.UnusedMeasure<0))
		fo_line_def.Quad = 5;
	if(fo_line_def.MiscLineFlags & 0x20) /* H&J sez [fm-] or [fh18] in the line: */
	{
		scan_fm2();     /* get adjustment if [fm2] in the line  */
						/* Put out any adjustment resulting from diff between
							wid given in [fm and actual font char widths:  */
		xpos_adj(cmd_fm_adjustment);
	}
    wordsp = 0;					/* Assume quadded lines so no */
    nbands = 0;					/*   adjustment to spacebands. */
	LsRemainder = 0;			/* Assume no adj to the letterspacing.  */
    if (fo_line_def.Quad==5)	/* Justified line: */
	{
		if (fo_line_def.Bands > 0)	/* If there are bands, spread  */
		{						/*   excess (+ letsp for last char), either:  */
								/*   as extra LS, if H&J thinks that's right,  */
			if (fo_line_def.MiscLineFlags & 0x400)
				LsRemainder = fo_line_def.UnusedMeasure +
					fo_line_def.LsPerChar;
			else				/*   or, normally, into all bands.  */
				wordsp = fo_line_def.UnusedMeasure +
					fo_line_def.LsPerChar; /* Set up for running interword */
			nbands = fo_line_def.Bands;	/* space adjustment. */
		}
		else if (fo_line_def.LsPerChar>0) /* No bands. If letterspacing:  */
		{						/* Spread excess into letterspaces (as long as
									UnusedMeasure < next incremental letterspace
									applied across whole line). NB, this includes
									cases where LsPerChar>0 && MiscLineFl&0x400:  */
			if (fo_line_def.UnusedMeasure <=
				(fo_line_def.LsTotal/fo_line_def.LsPerChar))
				LsRemainder = fo_line_def.UnusedMeasure;
		}
		else if (fo_line_def.MiscLineFlags & 0x400)
		{						/* Special case: No full LS on line but H&J sez
									to put remainder into LS:  */
			LsRemainder = fo_line_def.UnusedMeasure;
		}
		else					/* Neither bands nor letterspace.  Ie. one long  */
		{						/*	wd (or part) ends ragged. If within 10% of
									measure, letterspace to make it fit:  */
			if (fo_line_def.UnusedMeasure <
				(fo_line_def.SolMeasure/10))
				LsRemainder = fo_line_def.UnusedMeasure;
		}
	}
	fill_space = fo_line_def.UnusedMeasure;
	fill_count = fo_line_def.Bands;
	if (fill_count < 1)
		fill_count = 1;
	if (fo_line_def.Quad == 3)
		fill_space *= 2;
    ZeroSetFlag = 0;			/* Reset character 0-width flag. */
    RoundoffBucket = 0;			/* Init sum of H&J roundoff errs. */
	UncompensatedRoundoff = 0;
	ET_LineStartedFlag = 0;
	cmd_nt_on_the_line_flag = 0;
	/* Check for pdf link restart */
	if (pdflinkctr) {			/* If line continues a pdfmark link area,  */
		pdflinkAtSOL = 1;		/*  delay marking the starting X/Y until first
									cmd or char of line. This guarantees that
									any pending beg_Page will happen first.  */
	}
    while ((ichar = foget()) != -8) /* Loop until end of FO line. */
    {
		if(ichar <= 0)			
		{						/* If ichar is < 0 then it is a command. */
			ichar = -ichar;		/* Make ichar pos */
			if(ichar <= 84)		/* and see if its a valid command. */
			{
				switch (ichar)
				{
				  case 25:		/* autosort */
				  case 74:		/* accent */
					ForceLineNbrFont = 1;
				  case 4:		/* band */
				  case 7:		/* leader */
				  case 12:		/* fixed space */
				  case 33:		/* zero set */
				  case 54:		/* horiz move */
				  case 67:		/* end underscore */
				  case 69:		/* rule */
				  case 77:		/* graphic */
				  case 80:		/* [bf */
				  case 84:		/* rule breaking type for [bf command */
				  case 82:		/* box */
					ET_FakeTabLineFlag = 0;
					output_delayed_et();
					break;
				  case 22:		/* [bt */
				  	/* Save vj amount so [bx_] rules can be extended at [et] */
				  	save_vj_pending = vj_pending;
				  case 15:		/* straddle_head */
				  case 23:		/* [nt */
				  case 24:		/* [et */
					ET_FakeTabLineFlag++;
					cmd_nt_on_the_line_flag++;
					if (ichar == 24)
					{
						/* Restore vj amount so [bx_] rules can be extended */
						vj_pending = save_vj_pending;
						save_vj_pending = 0;
					}
					break;
					
				  case 34:		/* EditTrace */
					break;
				}				/* end switch(ichar) */
#ifdef TRACE
				if(text_trace)
				{
					trace_string[0] = 0;
					if (ichar == 26)
						sprintf(trace_string, "_");
					else if (ichar == 4)
						sprintf(trace_string, "^");
					else if (ichar == 25)
						sprintf(trace_string, "<KMD25>");
					if (trace_string[0] )
					{
						if( !found_char_flag)
							p_info(PI_TRACE, "\t");
						p_info(PI_TRACE, "%s",trace_string);
						found_char_flag++;
					}
					else
					{
						if (found_char_flag)
							p_info(PI_TRACE, "\n");
						p_info(PI_TRACE, "KMD%d\n",ichar);
						found_char_flag = 0;
					}
				}
#endif
				(*kmds[ichar])(); /* Call corresponding command. */
			}
			else p_info(PI_WLOG, "Invalid input command = %1d. \n",ichar*-1);
		}
		else
		{						/* Character. */
#ifdef TRACE
			if(text_trace && !found_char_flag)
				p_info(PI_TRACE, "\t");
			found_char_flag++;
#endif
			ForceLineNbrFont = 0;
			ET_FakeTabLineFlag = 0;
			output_delayed_et();
			ET_LineStartedFlag++;
			if (ichar >= 256)
				ichar = 0;		/* (should not happen) */
			ackhar = ichar;		/* Save Penta font loc. */
			RoundoffApply = 0;
			if ( !MathStartFlag)
			{					/* rounding if not in math */
				if ((rounding_error = RoundoffBucket / FontBase))
				{				/* adjust Xpos to true position */
					RoundoffBucket %= FontBase;
					UncompensatedRoundoff = RoundoffBucket;
					if (nbands > 1)	/* With 2+ bands left in line,  */
					{				/*  apply accum roundoff to bands.  */
						Xpos += rounding_error;
						wordsp -= rounding_error;
					}
					else			/*  else apply it to chars, because bands can
									    squeeze too much or line can end long.  */
						RoundoffApply = rounding_error;	
				}
			}
			khar_wid = foget();	/* Get char width  */
			ichar = (((int16)PsfCode[PsfCodeIndex].code[ichar]) & 0xff);
								/* Get PostScript translation */
			if ((ichar <= 0) || (ichar > 255)) /* If illegal, error msg. */
			{
				p_info(PI_WLOG, "No Postscript char for Penta font loc %d, use ? \n",ackhar);
				ichar = '?';
			}
#ifdef TRACE
			if(text_trace)
				p_info(PI_TRACE, "%c",ichar);
#endif
			if (ZeroSetFlag)
			{
				if ((cc_mask  | active_trap_cc_mask) && FlashOn)
				{				/* if not active, no output needed */
					stack_print(); /* Put preceeding chars, if any, to TP. */
					m_fprintf("currentpoint\n");
					save_cur_Xpos = Xpos;
					excess = (save_khar_wid - khar_wid) / 2;
					Xpos = save_starting_Xpos + excess;
					Ofx = -4001; /* force Move command output */
					do_pskmd('m', "zeroset=2");
					stack(ichar); /* Stack up char & width adj. */
					stack_print(); /* Put cur char to TP. */
					m_fprintf("M\n"); /* move to currentpoint */
					Xpos = save_cur_Xpos;
				}				/* end output of if(ZeroSetFlag) */
				ZeroSetFlag = 0; /* Zero-set no longer pending. */
			}					/* end if(ZeroSetFlag) */
			else
				stack(ichar);	/* Stack up char & width adj. */
		}
    }							/* end while */
	output_delayed_et();
	if ( !cmd_nt_on_the_line_flag)
		ET_FakeTabLineFlag = 0;
#ifdef TRACE
	if(text_trace)
	{
		if (found_char_flag)
			p_info(PI_TRACE, "\n");
		found_char_flag = 0;
		p_info(PI_TRACE, "**END LINE**\n");
	}
#endif
    Ofx = -4000;				/* Force output of M  */
    jbl = 0;					/* Reset baseline shift at EOL */

	/* Wrap Up PDF Links */

	if (pdflinkctr &&			/* If PDF link area is active this line,
									and this is text from .fo file,  */
		(!FileType || TYPE_OF_FRAME(CurrentFrame) != PL_TEXT))
	{							/*  activate the line as a pdf link:  */
		do_pskmd('M',"move after stack calls put_font");
		ProcessEnd();
		do_pskmd('M',"move after stack calls put_font");
		pdflinkctr++;
	}
    if(FlashOn)
	{
		if( usw)
			set_under_score();	/* Draw underscores to end of line. */

		if ( StrikeThruRuleStartFlag)
		{
			set_strikethru_rule();
		}
		if ((EditTraceStarted & 1))
			set_edit_trace(1);	/* draw underscores for cur edit trace */
		if ((EditTraceStarted & 2))
			set_edit_trace(2);	/* draw underscores for prev edit trace */
	}
	stack_print();
    if(repeatno > 1)
    {
		if ( forec != holdforec)
			foread(holdforec);
		fowrd = holdfowrd - 1;
		repeatno--;
    }
    else if (repeatno == 1)
		repeatno = 0;
}								/* end ps_drawline */

/***********************************************************************
 **  KMD0()             Null Command **
 ***********************************************************************/
int kmd0()
{return(0);}

/***********************************************************************
 **  KMD4()    Space Band. **
 ***********************************************************************/
int kmd4()
{
	int16 rounding_error;
    int k1, k2;
	int k3 = 0;
	
    if (nbands <= 0)
	{
		k1 = 0;
		if ( (fo_line_def.Quad < 5) && (wordsp < 0) && !nbands)
			k3 = wordsp;		/* acumulated rounding error, is negative */
		wordsp = 0;
	}
    else
	{
		k1 = (int )wordsp / nbands; /* Space left for this band */
		nbands--;
	}
	rounding_error = RoundoffBucket / FontBase;
	RoundoffBucket %= FontBase; /* Reset sum for next word. */
	UncompensatedRoundoff = RoundoffBucket;
	k3 -= rounding_error;		/* reduce band width by sum of H&J roundoff 
								   errors found in preceding word. */
/* Band width: FO val optimum), + space leftover, per band, 
   + letterspace per char. + (negative) acumulated rounding error */
    k2 = foget() + k1 + fo_line_def.LsPerChar + k3; 
    Xpos += (k2 + rounding_error); /* X-pos to new true value. */
	if (cc_mask || active_trap_cc_mask)
		Kspace[Linesub+1] += (float)k2/
			HorizontalBase;		/* Convert to pts for output. */
    
    wordsp -= k1;				/* Adjust running band totals */
	return(0);
}

/***********************************************************************
 **  KMD5()		Change Color (also see kmd64 for trapping) **
 ***********************************************************************/
int kmd5()						/* foreground color */
{
	int in_color, in_shade;

    in_color = foget();
	in_shade = foget();

	/****save new color values for reset after underscore print
	 ****BUG 594q
	 ****/
	 Current_FgColor = in_color;
	 Current_FgShade = in_shade;

	if (in_overflow)
		return(0);				/* ignore color change in overflow */
	color_func(in_color, in_shade);
#ifdef TRACE
    if (color_trace)
		p_info(PI_TRACE, "KMD5 - color= %d, shade= %d, cc_mask= %lo, cc_hit= %lo\n",
			   in_color, in_shade, cc_mask, cc_hit);
#endif
	return(0);
}

/************************************************************************/
void color_func(int in_color, int in_shade)
{
	int i;
	uint32 plates_hold;
	uint32 loop_temp;
	uint32 old_cc_mask, new_cc_mask;
	uint32 temp_clr_mask = 0;
	float temp_fx, temp_fy, temp_fx2, temp_fy3;

	if (in_overflow)
	{
		cc_mask = 1 & Plates;
		FgColor = DefaultFrameFgColor;
		return;
	}
	temp_fx = Fx;
	temp_fy = Fy;
	temp_fx2 = Fx2;
	temp_fy3 = Fy3;
	old_cc_mask = cc_mask;
	if (in_color < 0)
		in_color = DefaultFrameFgColor;
	set_pass_2_color(in_color, &temp_clr_mask, 1);
	FgColor = in_color;
	FgShade = in_shade;
	stack_print();
	Kspace[0] = 0;
	Linesub = -1;
	flagGlyph = 0;
	was_text = 0;
	if(spot_pass == 1)			/* 1 = black, 2 = all other colors */
	{							/* doing the black pass */
		cc_mask = 1 & Plates;
		FgColor = color_check(in_color, -1);
		if (FgColor < 0)
			new_cc_mask = 0;
		else
			new_cc_mask = 1 & Plates;
		if (KeyOutputType)		/* pass 2 only if not composite */
			cc_hit |= ((temp_clr_mask & ~1) & Plates);
	}							/* end if(spot_pass==1) */
	else						/* pass 2 */
	{
		new_cc_mask = (temp_clr_mask & ~1) & Plates; /* all colors on pass 2 */
		if (!new_cc_mask)
			FgColor = -1;
	}
	BlackPaint_cc_mask = 0;
	if ( new_cc_mask)
	{							/* output the colors for this mask */
		int save_SolFgColor, save_SolFgShade;

		for (i=0; i<MAX_CLRS-1; i++)
		{						/* output the color */
			plates_hold = ( loop_temp = (1 << i) ) & new_cc_mask;
			if(loop_temp > new_cc_mask)
				break;
			if ( !plates_hold)
				continue;
			cc_mask = loop_temp;
			save_SolFgColor =  fo_line_def.SolFgColor;
			save_SolFgShade = fo_line_def.SolFgShade;
			fo_line_def.SolFgColor =  FgColor;
			fo_line_def.SolFgShade = FgShade;
			put_font();
			vid_color(FgColor, FgShade, i+1); /* output color for this plate */
			do_pskmd('M',"after mc color");
			fo_line_def.SolFgColor =  save_SolFgColor;
			fo_line_def.SolFgShade = save_SolFgShade;
		}						/* end for(i=0;i<MAX_CLRS-1;i++)  */
	}
	Fx = temp_fx;
	Fy = temp_fy;
	Fx2 = temp_fx2;
	Fy3 = temp_fy3;
	cc_mask = new_cc_mask;
}								/* end function */

/***********************************************************************
 **  KMD8()      (handled elsewhere) **
 ***********************************************************************/
int kmd8()
{return(0);}

/***********************************************************************
 **  KMD10()            Repeat line. **
 ***********************************************************************/
int kmd10()
{
    if (repeatno == 0)
		repeatno = foget();		/* times to repeat line */
    else
		foget();
	return(0);
}

/***********************************************************************
 **  KMD11()		(unused) **
 ***********************************************************************/
int kmd11()
{return(0);}

/***********************************************************************
 **  KMD16()		(unused) **
 ***********************************************************************/
int kmd16()
{return(0);}

/***********************************************************************
 **  KMD17()            Inc/Dec width. **
 ***********************************************************************/
int kmd17()
{
    fo_line_def.SolCwAdj = foget();
	return(0);
}

/***********************************************************************
 **  KMD18()		(unused) **
 ***********************************************************************/
int kmd18()
{return(0);}

/***********************************************************************
 **  KMD19()      Align top (See SKMD19) **
 ***********************************************************************/
int kmd19()
{return(0);}

/***********************************************************************
 **  KMD20()      Align bottom (See SKMD20) **
 ***********************************************************************/
int kmd20()   	
{return(0);}

/***********************************************************************
 **  KMD21()      Align center (See SKMD21) **
 ***********************************************************************/
int kmd21()
{return(0);}

/***********************************************************************
 **  KMD29()		(unused) **
 ***********************************************************************/
int kmd29()
{return(0);}

/***********************************************************************
 **  KMD33()      Zero Set. Set NEXT char centered over previous. **
 ***********************************************************************/
int kmd33()
{
    if (FlashOn)
		ZeroSetFlag = 1;		/* Flag to treat next character specially. */
	return(0);
}

/***********************************************************************
 **  KMD35()      Define Slug. **
 ***********************************************************************/
int kmd35()
{
    foget();					/* starting number */
    foget();					/* small galley size */
    foget();					/* large galley size */
	return(0);
}
/***********************************************************************
 **  KMD36()      Output Slug. **
 ***********************************************************************/
int kmd36()
{
	foget();					/* Slug Indicator */
	return(0);
}

/***********************************************************************
 **  KMD37()		(unused) **
 ***********************************************************************/
int kmd37()
{return(0);}

/***********************************************************************
 **  KMD38()      Conditional Slug. **
 ***********************************************************************/
int kmd38()
{
    foget();
	return(0);
}

/***********************************************************************
 **  KMD39()		(unused) **
 ***********************************************************************/
int kmd39()
{return(0);}

/***********************************************************************
 **  KMD40()		(unused) **
 ***********************************************************************/
int kmd40()
{return(0);}

/***********************************************************************
 **  KMD41()		(unused) **
 ***********************************************************************/
int kmd41()
{return(0);}

/***********************************************************************
 **  KMD43()		(unused) **
 ***********************************************************************/
int kmd43()
{return(0);}

/***********************************************************************
 **  KMD44()		(unused) **
 ***********************************************************************/
int kmd44()
{return(0);}

/***********************************************************************
 **  KMD45()		(unused) **
 ***********************************************************************/
int kmd45()
{return(0);}

/***********************************************************************
 **  KMD46()		(unused) **
 ***********************************************************************/
int kmd46()
{return(0);}

/***********************************************************************
 **  KMD48()		(unused) **
 ***********************************************************************/
int kmd48()
{return(0);}

/***********************************************************************
 **  KMD53()		(unused) **
 ***********************************************************************/
int kmd53()
{return(0);}

/***********************************************************************
 **  KMD56()		(unused) **
 ***********************************************************************/
int kmd56()
{return(0);}

/***********************************************************************
 **  KMD61()		(unused) **
 ***********************************************************************/
int kmd61()
{return(0);}

/***********************************************************************
 **  KMD62()		(unused) **
 ***********************************************************************/
int kmd62()
{return(0);}

/***********************************************************************
 **  KMD63()		(unused) **
 ***********************************************************************/
int kmd63()
{return(0);}

/***********************************************************************
 **  KMD64()   Fielded user command. Underscore gaps. Make color. **
 ***********************************************************************/
int kmd64()
{
    int16 action;
    int16 fo_char;
    int ii, jj;
	int16 gap_1_start, gap_1_end, gap_2_start, gap_2_end;
	uint32 save_cc_mask;
	int input_color, input_shade;
	int16 rule_wt;
	int save_xpos, save_ypos;
    float line_bredth;
	int rule_color, new_rule_color;
	int rule_shade;
	int16 nargs;
	uint32 save_start_color_cc_mask;
	uint32 plates_hold, BlackPaint_cc_mask_sav;
	uint32 loop_temp;
	uint32 temp_clr_mask = 0;
	uint32 current_color_cc_mask;
	int prev_rule_color = -2;
	F_LIST *vp;					/* list-walking pointer */

        action =  foget();			/* + = Fielded user command.
								   - = G.P. variable-len command:
								   -1 to -3 = Underscore gaps
								   -4 = Make color.
								   -5 = PSI (PageSizeImage) command.
								   -6 = Underscore gap min percent of setwidth.
								   -7 = Tab gutter rule colors.
								   -8 = No break depth.
								   -9 = Data for blends:
								   		end_color, end shade, followed by:
								   		0 through 3599 is angle.
										-1 is top left (angle > 270).
										-2 is top right (angle > 180).
										if anchor points (up to 20),
												new end color, end shade, and
												depth or percent. Percent is
												negative, depth is +.
								   -10 = white paint = 1, black paint = 0.
								   -11 = [ev gutter rules.
								   -12 = [ev flag for top of tab.
								   -13 = width of numeric.
								   -14 = [bf continuation at frame start.
								   -15 = Marks start of accent.
								   -16 = Marks end of accent.
								   -17 = Marks start of math.
								   -18 = Marks end of math.
								   -19 = [sr - strikethru rule.
								   -20 = [pp - pp_to_eps.
								   -21 = tab split index.
								   -22 = branch off to pdfmarks 
								   -23 = Available for more.  */
    nargs = foget();
    switch(action)
    {
	  case -22:					/* pdfmarks */
		pdfnargs=nargs;
                do_pskmd('M',"kmd64, -22, PDFMark start");
		ProcessPdfMarks(nargs);
                do_pskmd('M',"kmd64, -22, PDFMark end");

		break;
	  case -21:					/* tab split index */
		tab_split_index = foget();
		foget();				/* not needed for now */
		if (tab_split_index < 2)
		{
			tab_split_index = 0;
			tab_split_index_start = 0;
			tab_split_index_end = 0;
		}
		else if (PageNo1 & 1)
		{						/* odd page = right-hand page */
			tab_split_index_start = tab_split_index;
			tab_split_index_end = 99;
		}
		else
		{						/* even page = left-hand page */
			tab_split_index_start = 0;
			tab_split_index_end = tab_split_index - 1;
		}
		break;
	  case -20:					/* [pp and [pq - pp_to_eps. */
		save_start_color_cc_mask = cc_mask;
		if(spot_pass == 1)
			cc_mask = 1 & Plates;
		else
			cc_mask = cc_hit & Plates;

		if ( !nargs || (nargs == 2))
		{						/* [pq - end pp_to_eps */
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "end export after font list\n");
#endif
			if (nargs == 2)
			{					/* read vert limits for math */
				ii = foget();	/* above top baseline */
				jj = foget();	/* below bottom baseline */
			}
			else
			{					/* in text, no limits included */
				ii = 0;
				jj = 0;
			}
			export_ii=ii;
			export_jj=jj;
			stack_print();		/* Put preceding chars, if any, to .tp  */
			vp  = E_fonts;
			while(vp != 0)		/* for all fonts in the list */
			{
				m_fprintf("%%PSI_ExportFont: %s\n", vp->font);
				vp = vp->nxt;
			}
			clear_flist(&E_fonts); /* clear list of export fonts	*/
			m_fprintf("%%PSI_ExportEnd: ");
			exported=0;
			if (export_end_y)			/* Floating ill/table is exported:  */
			{
										/* Width and Bottom of frame: */
				digi_print((export_end_x - export_x) / HorizontalBase);
				digi_print((Imageheight - export_end_y) / VerticalBase);
			}
			else					/* All other exports:  */
			{
				digi_print((float)MaxPP_Meas / HorizontalBase);
				digi_print(Ofy);
			}
			digi_print((float)ii / VerticalBase);
			digi_print((float)jj / VerticalBase);
			m_fprintf("\n");
			Kmd_PP_Started = 0;
		}
		else
		{						/* [pp - start pp_to_eps */
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "\nStart export ");
#endif
								/* This [pp was put out at start of line. */
			if (fo_line_def.MiscLineFlags & 0x200)
			{					/* Skip name args here, and break from switch.  */
				for(ii=0; ii<nargs; ii++)
					fo_char = foget();
				break;
			}
			Kmd_PP_Started++;

			if (DoExtractBreaks &&		/* If option is enabled by argument, and */
				!FileType &&			/*  in galley mode, and */
				DidPage)				/*  in a page (not just done a page break), then: */
				{		
				/***  Bug 357p - add page break at beg of [pp cmd  ***/
				do_pskmd('P', "at [pp cmd");	/***  Gen an end-page sequence  ***/
				beg_PAGE();  			/*** Gen a begin-page sequence, ***/
				}

			MaxPP_Meas = fo_line_def.SolMeasure + fo_line_def.SolMarginAdjust;
			m_fprintf("%%PSI_Export: ");
			exported=1;
			export_ctr = 0;
			if (FileType)				/* In page mode, save export's MP frame type:
											0=Design text	3=Footnote
											4=Ill/Table		11=Sidenote
											13=Flow text  */
				export_frametype = REL_DATA(CurrentFrame) t0;
			digi_print(export_x / HorizontalBase);	/* starting X including frame offset */
			digi_print(Ofy);	/* starting Y */
			export_y = 0;		/* Set only for floating ill/table. */
			export_end_y = 0;	/* Set only for floating ill/table. */
			memset(export_name,0,sizeof(export_name));
			for(ii=0; ii<nargs; ii++)
			{
				fo_char = foget();
				sprintf(export_char,"%c", (char)fo_char);
				strcat(export_name,export_char);

				m_fprintf("%c", (char)fo_char); /* name */
#ifdef TRACE
				if (debugger_trace)
					p_info(PI_TRACE, "%c", (char)fo_char);
#endif
			}
			m_fprintf("\n");
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "/n");
#endif
			was_text = 1;
			Ofx = -4000;		/* Force output of M  */
			old_freq = -1;
			color_func(fo_line_def.SolFgColor, fo_line_def.SolFgShade);
		}
		cc_mask = save_start_color_cc_mask;
		break;
	  case -19:					/* [sr - strikethru rule. */
		if (nargs == 1)
		{						/* end strikethru rule, output it */
			if ( !StrikeThruRuleStartFlag)
				break;			/* ignore end if not started */
			foget();			/* ignore dummy argument */
			nargs -= 1;
			if ( FlashOn) {
				set_strikethru_rule(); }
			StrikeThru_beg_x = Xpos;
			StrikeThruRuleStartFlag = 0;
			StrikeThruLayer = 0;
			break;
		}
		if (StrikeThruRuleStartFlag)
		{						/* ignore start if already in progress */
			for(ii=0; ii<nargs; ii++)
				foget();
			break;
		}
		StrikeThruRuleStartFlag = 1;
		StrikeThruTop = foget();
		StrikeThruBot = foget();
		StrikeThruWt = foget();
		StrikeThruColor = foget();
		StrikeThruShade = foget();
		StrikeThruTrapWt = foget();
		StrikeThruTrapColor = foget();
		StrikeThruTrapShade = foget();
		if (nargs > 8)
			StrikeThruLayer = foget();
		else
			StrikeThruLayer = 0;

		do_pskmd('M',"kmd64, -19, strike_thru rule start");
		StrikeThru_beg_x = Xpos;
		StrikeThru_start_Top = Ypos + StrikeThruTop;
		break;
	  case -18:					/* End math flag. */
		MathStartFlag = 0;
		break;
	  case -17:					/* Start math flag. */
		MathStartFlag = 1;
		break;
	  case -16:					/* Accent end flag. */
		if ( MathStartFlag)
			AccentEndFlag = 0;
		else if ( AccentStartFlag)
			AccentEndFlag++;
		AccentStartFlag = 0;
		break;
	  case -15:					/* Accent start flag. */
		if ( (cc_mask || active_trap_cc_mask) && !MathStartFlag)
		{
			AccentStartFlag++;
			AccentEndFlag = 0;
		}
		else 
		{
			AccentStartFlag = 0;
			AccentEndFlag = 0;
		}
		break;
	  case -14:					/* [bf continuation at frame start. */
		continued_bf_flag++;
		break;
	  case -13:					/* width of numeric for [fh18 */
		WidthOfNumeric = foget();
		break;
	  case -12:					/* [ev gutter rules flag extend at [bt top */
		for(ii=0; ii<nargs; ii++) /* ignored in output */
			foget();
		break;
	  case -11:					/* [ev gutter rules - wt, color, shade, UL X */
		if ( ((Ypos - Bol_Ypos) <= 0) || !FlashOn)
		{						/* skip if zero lead to base or no flash */
#ifdef TRACE
			if(debugger_trace)
				p_info(PI_TRACE, 
					   "[EV rule depth = 0, skip gutter rule extension.\n");
#endif
			for(ii=0; ii<nargs; ii++)
				foget();
			break;
		}
#ifdef TRACE
			if(debugger_trace)
				p_info(PI_TRACE, "[EV rule depth = %d, extending gutter rules.\n",
					   fo_line_def.BolLeading);
#endif
		save_xpos = Xpos;
		save_ypos = Ypos;
		stack_print();
		current_color_cc_mask = cc_mask;
		save_start_color_cc_mask = cc_mask;
		for(ii=0; ii<nargs; ii+=4)
		{
			rule_wt = foget();	/* rule weight in 1/10 points */
			rule_color = foget();
			rule_shade = foget();
			Fx = foget() + fo_line_def.SolMarginAdjust;	/* starting X */
			line_bredth = ((float)rule_wt * HorizontalBase) / Jrule;
			if( FileType)
				Fx += Xmark;
			Fx2 = Fx + line_bredth;
			Fy = Bol_Ypos;		/* Y-pos of rule top. */
			Fy3 = save_ypos;	/* Y-pos of rule bottom. */
			Xpos = Fx;
			Ypos = Fy;
			if (rule_color < 0)
			{
				rule_color = DefaultFrameFgColor;
				rule_shade = DefaultFrameFgShade;
			}
			temp_clr_mask = 0;
			set_pass_2_color(rule_color, &temp_clr_mask, 1);
			temp_clr_mask &= Plates;
			if(spot_pass == 1)
			{					/* doing the black pass */
				cc_mask = 1 & Plates;
				new_rule_color = color_check(rule_color, -1);
				if (new_rule_color < 0)
					current_color_cc_mask = 0;
				else
				{
					new_rule_color = rule_color;
					current_color_cc_mask = 1 & Plates;
				}
				if (KeyOutputType) /* pass 2 only if not composite */
					cc_hit |= (temp_clr_mask & ~1);
			}					/* end if(spot_pass==1) */
			else
			{					/* pass 2 */
				new_rule_color = rule_color;
				current_color_cc_mask = temp_clr_mask & ~1;
			}
			if ( current_color_cc_mask )
			{					/* output the colors for this mask */
				BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
				for (jj=0; jj<MAX_CLRS-1; jj++)
				{				/* output the color and v-rule */
					plates_hold = ( loop_temp = (1 << jj) ) & 
						current_color_cc_mask;
					if(loop_temp > current_color_cc_mask)
						break;
					if ( !plates_hold)
						continue;
					cc_mask = loop_temp;
					if ( prev_rule_color != rule_color)
					{
						BlackPaint_cc_mask = 0;
						vid_color(new_rule_color, rule_shade,
								  jj+1); /* output color */
					}
					Fx = Xpos;	/* Restore X/Y each color pass */
					Fy = Ypos;
					Fx2 = Fx + line_bredth;
					Fy3 = save_ypos;
					do_pskmd('L',"draw evrule");
					Ofx = -4000; /* force Move command output */
					do_pskmd('M',"after draw evrule");
					PaintOnPage |= BlackPaint_cc_mask;
				}				/* end for(jj=0;jj<MAX_CLRS-1;jj++)  */
				BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
			}					/* done with output of all the colors */
			prev_rule_color = -2;
		}						/* end of this gutter-rule */
		cc_mask = current_color_cc_mask;
		Xpos = save_xpos;
		Ypos = save_ypos;
		Ofx = -4000;			/* force Move command output */
		do_pskmd('M',"end of ev");
		color_func(fo_line_def.SolFgColor, fo_line_def.SolFgShade);
		cc_mask = save_start_color_cc_mask;
		break;

	  case -10:					/* paint is black(0) or white(1). */
		DontPaintFlag = foget();
		break;

	  case -9:					/* Blends. */
		save_cc_mask = cc_mask;
		if (spot_pass == 1)
			cc_mask = 1 & Plates;
		else
			cc_mask = cc_hit;
		AnchorCount = 0;
		if ( nargs == 1)
		{						/* no blend */
			foget();			/* ignore dummy argument */
			blend_flag = 0;
			break;
		}
		nargs -= 3;
		nargs /= 3;
		BlendEndColor = foget();
		BlendEndShade = (float)foget() / 100.;
		BlendAngle = foget();
		blend_flag = 1;
		for(ii=0; ii<nargs; ii++)
		{
			input_color = foget();
			anchor_colors[ii] = input_color;
			anchor_shades[ii] = (float)foget() / 100.;
			anchor_amounts[ii] = foget();
			AnchorCount++;
		}
		cc_mask = save_cc_mask;
		break;

	  case -8:					/* No break depth. */
		action = foget();
		if ( (action >= Imageheight) || !DidPage || CurrentLineBcEc)
			break;				/* skip nobreak if too large or no page yet
								 or BcEc with output off */
		if ( ((!FileType || in_overflow) && ((Ypos + action) >=
											 Imageheight)) && !Ktabmode)
		{						/* not enough room */
			save_cc_mask = cc_mask;
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			if (in_overflow)
				OverflowOrColorFlag = -1;
			do_pskmd('P',"no break"); /* show page, start new. */
			Ypos = 0;
			Ofx = -4000;		/* force Move command output */
			do_pskmd('M', "no break"); /* Move to current XY coords. */
			us_beg_x1 = Xpos;
			us_beg_x2 = Xpos;
			StrikeThru_beg_x = Xpos;
			us_start_Ypos[1] = Ypos;
			us_start_Ypos[2] = Ypos;
			cc_mask = save_cc_mask;
		}
		break;

	  case -7:					/* Tab gutter rule colors. */
		for(ii=0; ii<(nargs/2); ii++)
		{
			PSTabGutterRuleColor[ii] = foget();
			PSTabGutterRuleShade[ii] = foget();
		}
		break;

	  case -6:					/* Underscore gap min percent of setwidth. */
		underscore_min = foget();
		us_min_mru = ((int)fo_line_def.SolSetSize * underscore_min) / 100;
		break;

      case -5:					/* [PSi-,-] */
		kmd60();				/* Use logic of [PS-,-] */
		if (FileType)			/* layout? */
			break;				/* yes, ignore cmd */
		if (total_points)
		{						/* if any, set new page size */
			PageH = total_points;
			if ( !Orient)
				Imageheight = (PageH - OffT - OffB) * VerticalBase;
			else
				Imageheight = (PageW - OffL - OffR) * VerticalBase;
		}
		if (DidPage)
		{
			save_cc_mask = cc_mask;
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			do_pskmd('P', "at PSI cmd"); /* break page if past page start. */
			cc_mask = save_cc_mask;
		}
		break;

      case -4:					/* Trapping Change */
		Ofx = -4000;			/* force Move command out */
		do_pskmd('M',"trapping kmd64 start"); /* flush the buffer */
		input_color = foget();
		input_shade = foget();
		ActiveTrapStrokeWidth = foget();	/* in horizontal base units */
		if (in_overflow)
			return(0);				/* ignore for overflow */
#ifdef TRACE
			if (color_trace)
				p_info(PI_TRACE, "KMD64 -4 trap color= %d, shade= %d, stroke= %d\n",
					   input_color, input_shade, ActiveTrapStrokeWidth);
#endif
		active_trap_cc_mask = 0; /* zero unless otherwise set */
		if (input_color < 0)
			ActiveTrapColor = -1;
		else
		{
			save_cc_mask = cc_mask;
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			ActiveTrapColor = color_check(input_color, -1);
			if (ActiveTrapColor >= 0)
			{					/* set active_trap_cc_mask */
				if ( spot_pass == 1)
					active_trap_cc_mask = 1 & Plates;
				else
					set_pass_2_color(input_color, &active_trap_cc_mask, 2);
				ActiveTrapShade = input_shade;
			}
			if ( !active_trap_cc_mask)
				ActiveTrapColor = -1; /* no trap if clr undefined for pass */
			else
			{
				Ofx = -4000;	/* force Move command out */
				do_pskmd('M',"trapping kmd64 move"); /* flush the buffer */
			}
			if ( (spot_pass == 1) && KeyOutputType ) 
				set_pass_2_color(input_color, &cc_hit, 2); /* set if needed */
			cc_mask = save_cc_mask;
		}
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "KMD64 -4 end, active trap color= %d %d, trap_mask(oct)= %lo\n",
				   ActiveTrapColor, ActiveTrapShade, active_trap_cc_mask);
#endif
		break;

      case -3:					/* underscore both with gaps */
	  case -2:					/* underscore gap, only #2 */
	  case -1:					/* underscore gap, only #1 */
		gap_1_start = foget();
		gap_1_end = foget();
		gap_2_start = foget();
		gap_2_end = foget();;
		us_min_mru = ((int)fo_line_def.SolSetSize * underscore_min) / 100;
		if (utype & 1)
		{
			Fx = us_beg_x1;
			Fx2 = Xpos + gap_1_start;
			Rwei = uswei[1] / ((float) HorizontalBase * Jrule);
			Fy = us_start_Ypos[1] + usoff[1];
			Fy3 = Fy + (Rwei * VerticalBase);
			if ( ((Fx2 - Fx) > us_min_mru) && FlashOn )
				do_pskmd('L',"set_under_gap1");	/* only output if > 2 points */
			us_beg_x1 = Xpos + gap_1_end;
		}
		if (utype & 2)
		{
			Fx = us_beg_x2;
			Fx2 = Xpos + gap_2_start;
			Rwei = uswei[2] / ((float) HorizontalBase * Jrule);
			Fy = us_start_Ypos[2] + usoff[2];
			Fy3 = Fy + (Rwei * VerticalBase);
			if ( ((Fx2 - Fx) > us_min_mru) && FlashOn )
				do_pskmd('L',"set_under_gap2");	/* only output if > 2 points */
			us_beg_x2 = Xpos + gap_2_end;
		}
		Ofx = -4000;			/* force Move command out */
		do_pskmd('M',"end_under_gap");
		break;
		
      default:
		for(ii=0; ii<nargs; ii++)
			foget();			/* For now, skip all rest. */
		break;
    }
	return(0);
}

/***********************************************************************
 **  KMD70()   Page number mark. **
 ***********************************************************************/
int kmd70()
{
	int i;
    for (i=1; i<= 5; i++)
		foget();
	return(0);
}
/***********************************************************************
 **  KMD71()   Spot space.  Extra lead between [ec and [bc lines. **
 ***********************************************************************/
int kmd71()
{
    BcEcExtraLead = foget();
	return(0);
}
/***********************************************************************
 **  KMD73()   Footnote mark. From [fm1 thru [fm4 **
 ***********************************************************************/
int kmd73()
{
	int type, val, handj_val, ii;
	int val26[26];
	int reference;
	int pentachar;
	int fnglobalnum = 0;
	
	type = foget();				/* 1,11= 1-digit number.   3,13= l.c. letter.
								   2,12= 2-digit number.   4,14= u.c. letter.  */
	if (type > 10)				/* 10 added to type 1-4 means:  */
		fnglobalnum = foget();	/*   Get extra arg, the id# of global
									 footnote marked by this cmd.   */
if (debugger_trace)
p_info(PI_TRACE, "type=%d in frame-type %s, fnglobalnum=%d.\n", 
type, (FootnoteFrameFlag == 0 ? "Flow" : "Ftnote"), fnglobalnum);

	type %= 10;					/* Strip arg back to 1,2,3,4.  */
	if ( !FootnoteFrameFlag) 
	{							/* In Text frame  */
								/* If fnglobalnum is nonzero (ie. the footnote
									is a multiref), add the number to the list
									of this page's global fn IDs, and return ref
									number for this page if already in list:  */
		if ( (reference = repeatGlobalfn(fnglobalnum)) == 0)
		{						/* Inc CmdFM_Count for all normal notes and
									for 1st occurence each pg of multi-refs:  */
			reference = ++CmdFM_TextCount;
			if (fnglobalnum)
				storeGlobalfnRef(fnglobalnum, reference);
		}
	}
	else						/* In Footnote frame  */
		reference = ++CmdFM_FootCount;
if (debugger_trace)
p_info(PI_TRACE, "  ref#=%d\n", reference);

	if(type > 2)
	{
		if(reference > 26)
			reference = 26;
		pentachar = reference;	/* a=1 b=2...	*/
		handj_val = foget();	/* read h&j's guess for width */
		for (ii=0; ii<=25; ii++)
			val26[ii] = foget(); /* read all 26 words */
		val = val26[reference-1]; /* here's the 'real'width - use it */
		if(type == 4)
			pentachar += 32;	/* go to upper case */
		cmd_fm_adjustment += (handj_val - val);
	}
	else
	{
		val = foget();			/* read 1 word width */
		if(type==2)				/* range 1 - 99	*/
		{
			if(reference>99)
				reference = 99; /* limit upper end */
			val /=2;	/* widths are doubled if two chars */
			if(reference>9)		/* need two chars */
			{
				pentachar = PsfDefault->fmchar[reference/10]; /* tens */
				stack_one_char(pentachar, val);
			}
			else
				cmd_fm_adjustment += val;
		}
		else if(reference>9)
			reference = 9;		/* limit upper end */
		pentachar = PsfDefault->fmchar[reference%10]; /* ones */
	}
	stack_one_char(pentachar, val);
	return(0);
}

/*
 *	stack_one_char - translate & stack one penta location, pass a width
 */
static void stack_one_char(int ichar, int width)
{
	int ackhar = ichar;
	
	if(cmd_fm_flag)
		return;
	ichar = (((int16)PsfCode[PsfCodeIndex].code[ichar]) & 0xff);
	/* Get PostScript translation */
	if ((ichar <= 0) || (ichar > 255)) /* If illegal, error msg. */
	{
		p_info(PI_WLOG, "No Postscript char( in [fm] for Penta font loc %d, use ? \n",
			   ackhar);
		ichar = '?';
	}
	khar_wid = width;			/* use passed value	*/
	stack(ichar);
}
/***********************************************************************
 **  KMD75()   Output drawing.    [od. **
 ***********************************************************************/
int kmd75()
{
	int16 i;
    int16 draw_info;
    int16 draw_words;

    do_pskmd('m',"od command");
    draw_words = foget();		/* #words make up drawing info */
    for (i=1; i<=draw_words; i++)
    {
		draw_info = foget();	/* input drawing information */
		if(draw_info >= 512)
			m_putc(draw_info - 512);
		else
			m_putc(((int16)PsfCode[PsfCodeIndex].code[draw_info]) & 0xff);
    }
	m_fprintf("\n    ");
	return(0);
}

/***********************************************************************
 **  KMD76()  Change Line - To draw a vertical rule (line) to the **
 **	     left of text line.  1000k 2000k in ldef turnthison **
 ***********************************************************************/
int kmd76()
{
    float bar_wid;
    float bar_wei;
    int16 in_line_change;
    int16 bar_start_x;
    int16 bar_start_y;

    bar_wid = foget();
    bar_wei = foget();			/* Rule (Line) Weight. */
    if (bar_wei <= 0)
		bar_wei = 1;			/* Default is 1 unit. */
    bar_start_x = 20 * HorizontalBase; /* Offset from left of page. */
    bar_start_y = Ypos - fo_line_def.SolPointSize; /* y top */
    in_line_change++;
	return(0);
}

/***********************************************************************
 **  KMD78()   Graphic Rule Command (NOT IMPLEMENTED) **
 ***********************************************************************/
int kmd78()
{
    p_info (PI_WLOG, "kmd78 graphic rule not implemented \n");
	return(0);
}

/***********************************************************************
 **  scan_fm2() - collect [fm2 and [fh18 adjustments **
 ***********************************************************************/
static void scan_fm2(void)
{
	int16 kcmd, k64type, argcount, got, j;
	int16 forecsv, fowrdsv;
	int temp_page_nmbr;
	int save_ref = CmdFM_FootCount;
	int save_txt = CmdFM_TextCount;	/* save current FM indicies	*/

	cmd_fm_adjustment = 0;		/* clear */
	cmd_fm_flag = 1;			/* suppress output */
	forecsv = forec;
	fowrdsv = fowrd;
	do 
	{
		if((got = foget()) >= 0) /* If a character,throw it away */
		{
			foget();			/* as well as its width.*/
			continue;
		}
		kcmd = -got;			/* It's a command. Get cmnd# */
		if(kcmd == 73)
		{						/* process footnote mark.*/
			kmd73();
			continue;
		}
		/* Command is not in special list.*/
		if(kcmd > foreadsTOP)
			continue;			/* Illegal command#: Loop. */
/*
  The foreads array (see module pstabs.c, entry tscan(),)
  shows how many arguments follow each possible FO command:
  0		means there are no arguments.
  1-89	is the argument count.
  -1		means arg 1 contains number of remaining arguments.
  -2		means arg 2 contains number of remaining arguments.
  -8		means arg 8 contains number of remaining arguments.
  99		is special for command 42 (pagination mark).  Argument 1 is key:
				-12 or -13 mean 26 args follow.
				-21 means 1 arg follows.
				all other values mean no more args.
  98		is special for command 73 (footnote mark). Argument 1 is key:
				3 or 4 means 27 args follow.
  				all other values mean 1 arg follows.
  */
		argcount = foreads[kcmd-1];	/* For all other commands, get
									   the arg count constant.	*/
		if ( !argcount)
			continue;			/* No arguments? Loop.	*/
		/* Pos arg count is default value */
		if (argcount < 0)		/* Neg arg count means: */
		{
			k64type = 0;
			if (kcmd == 64)
			{
				k64type = foget();
				argcount += 1;
				if ( k64type == -13)
				{
					argcount = foget();
					for (j=0; j< argcount; j++)
						WidthOfNumeric = foget(); /* get the last one */
					continue;
				}
			}
			for (j=-1; j>argcount; j--)
				foget();		/* skip (argcount-1) args, then */
			argcount = foget();	/* ARGCOUNTth arg contains # */
		}						/* remaining args (join default) */
		else if (argcount == 99) /* Special arg list */
		{
			char page_nbr[16];
			int page_string_len;

			j = foget();
			switch (j)
			{
			  case -12:
			  case -13:
				argcount = 26;
				break;
			  case -16:			
				ChapterPage = 1 - PageNo1;
				ChapterPageSetFlag++; /* Flag: Sec folio now in offset form.  */
				continue;
			  case -18:			/* adjust of [fh18 space - secondary folio */
				if ( !FileType)
					temp_page_nmbr = 1;
				else 
					temp_page_nmbr = PageNo1 + ChapterPage;
				itoa(temp_page_nmbr, page_nbr);
				page_string_len = strlen(page_nbr);
				cmd_fm_adjustment = -page_string_len * WidthOfNumeric;
				continue;
			  case -21:
				ChapterPage = foget() - PageNo1;
				ChapterPageSetFlag++; /* Flag: Sec folio now in offset form.  */
				continue;
			  default:			/* 1st arg was only one. Loop.*/
				continue;
			}
		}
		else if (argcount == 98) /* Special arg list */
		{
			j = foget();
			argcount = 1;
			if ((j==3) || (j==4) )
				argcount = 27;
		}						/* End of if/else sequence */
		for (j=1; j<=argcount; j++) /* Loop to ignore string of args */
			foget();
	} while (got != -8);		/* -8 = End of line(end of do{} */
	if (forec != forecsv)
	{
		forec = forecsv;
		foread(forecsv);
	}
	fowrd = fowrdsv;
	CmdFM_FootCount = save_ref;
	CmdFM_TextCount = save_txt;	/* restore current FM indicies	*/
	cmd_fm_flag = 0;			/* allow output */
}

/***********************************************************************
 **  xpos_adj - make a move because of inserted, non-justified text **
 ***********************************************************************/
static void xpos_adj(int16 adjustment)
{
	fo_line_def.UnusedMeasure += adjustment;
	switch(fo_line_def.Quad)
    {
	  case 3:					/* QC */
        adjustment /=2;
	  case 2:					/* QR */
        Xpos += adjustment;
        do_pskmd('M', "QR/QC adjustment");
        break;
    }
}

/***********************************************************************
 **  output_delayed_et - output delayed edittrace markers **
 ***********************************************************************/
static void output_delayed_et( void)
{
	if ( ET_FakeTabLineFlag)
		return;
	if ( ET_InsertStartDelay1)
	{
		EditTrace_beg_x1 = Xpos;
		EditTraceStarted |= 1;
		ET_InsertStartDelay1 = 0;
	}
	if ( ET_InsertStartDelay2)
	{
		EditTrace_beg_x2 = Xpos;
		EditTraceStarted |= 2;
		ET_InsertStartDelay2 = 0;
	}
	if ( ET_InsertEndDelay1)
	{
		set_edit_trace(1);
		ET_InsertEndDelay1 = 0;
	}
	if ( ET_InsertEndDelay2)
	{
		set_edit_trace(2);
		ET_InsertEndDelay2 = 0;
	}
	if (ET_DeleteDelay1)
	{
		set_edit_trace(3);
		ET_DeleteDelay1 = 0;
	}
	if (ET_DeleteDelay2)
	{
		set_edit_trace(4);
		ET_DeleteDelay2 = 0;
	}
}

/***********************************************************************
 **  output strikethrough rule **
 ***********************************************************************/
static void set_strikethru_rule(void)
{
	int16 trap_stroke_width_hold;
	int cc_mask_hold, trap_color_hold, trap_shade_hold;
	int input_color;
	uint32 trap_cc_mask_hold, save_cc_mask;
	float save_fx, save_fx2, save_fy, save_fy3;
	double rule_length;
	float fx2_temp = Xpos;
	float fy_temp = Ypos;



	if (Xpos <= StrikeThru_beg_x)
		return;					/* negative-going rule */
	Ofx = -4000;				/* force Move command out */
	do_pskmd('M',"start end strikethru");

	StrikeThru_end_Bot = Ypos + StrikeThruBot;
	cc_mask_hold = cc_mask;
	trap_color_hold = ActiveTrapColor;
	trap_shade_hold = ActiveTrapShade;
	trap_cc_mask_hold = active_trap_cc_mask;
	trap_stroke_width_hold = ActiveTrapStrokeWidth;
	Fx = StrikeThru_beg_x;
	Fx2 = fx2_temp;
	Fy = StrikeThru_start_Top * 2; /* converts Vert to Horiz Base */
	Fy3 = StrikeThru_end_Bot * 2;
	rule_length = sqrt((((double)Fx2 - Fx) * ((double)Fx2 - Fx)) + 
						(((double)Fy3 - Fy) * ((double)Fy3 - Fy)));	/* H Base */
	Fx2 = rule_length;
	StrikeThruRuleAngle = asin( ((double)Fy - Fy3) / rule_length); /* radians */
	StrikeThruRuleAngle *= 57.29578; /* in degrees */
	Fy /= 2;					/* convert back to Vert Base */
	Fy3 = StrikeThruWt / ((float) HorizontalBase * Jrule);

	/* End save of Text */
	if (StrikeThruLayer == 1)
	{
		m_fprintf("} def\n");
	}

	active_trap_cc_mask = 0;
	cc_mask = 0;
	save_fx = Fx;
	save_fx2 = Fx2;
	save_fy = Fy;
	save_fy3 = Fy3;
	if (!in_overflow)
	{					/* process rule colors and traps */
		color_func(StrikeThruColor, StrikeThruShade);
		input_color = StrikeThruTrapColor;
		if (input_color < 0)
			ActiveTrapColor = -1;
		else
		{

			save_cc_mask = cc_mask;
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			ActiveTrapColor = color_check(input_color, -1);
			if (ActiveTrapColor >= 0)
			{					/* set active_trap_cc_mask */
				if ( spot_pass == 1)
					active_trap_cc_mask = 1 & Plates;
				else
				{
					active_trap_cc_mask = 0;
					set_pass_2_color(input_color, &active_trap_cc_mask, 2);
				}
				ActiveTrapShade = StrikeThruTrapShade;
				ActiveTrapStrokeWidth = StrikeThruTrapWt;
			}
			if ( !active_trap_cc_mask) {

				ActiveTrapColor = -1; /* no trap if clr undefined  for pass */
									  /* no trap is stroke width is 0 */
			}
			else
			{
				Ofx = -4000;	/* force Move command out */
				do_pskmd('M',"trap underscore move"); /* flush buffer*/
				Fx = save_fx;
				Fx2 = save_fx2;
				Fy = save_fy;
				Fy3 = save_fy3;
			}
			if ( (spot_pass == 1) && KeyOutputType ) 
				set_pass_2_color(input_color, &cc_hit, 2); /*as need*/
			cc_mask = save_cc_mask;
		}						/* end else input_color >= 0 */
	}
	if ( cc_mask || active_trap_cc_mask)
		do_pskmd('S',"set strikethru rule");
	Xpos = fx2_temp;
	Ypos = fy_temp;
	Ofx = -4000;				/* force Move command out */
	do_pskmd('M',"end strikethru");
/*	color_func(fo_line_def.SolFgColor, fo_line_def.SolFgShade); */
	cc_mask = cc_mask_hold;
	ActiveTrapColor = trap_color_hold;
	ActiveTrapShade = trap_shade_hold;
	active_trap_cc_mask = trap_cc_mask_hold;
	ActiveTrapStrokeWidth = trap_stroke_width_hold;

	/* Move to saved current point and reset text */
	if (StrikeThruLayer == 1 ) 
	{
		m_fprintf("SRX SRY M\n");
		m_fprintf("doText\n");
		StartSaveText = 0;
	}


}

/*
 *  We have a global footnote [fm ref mark on this page.
 *  Add its ID to list of up to ten, or if ID already
 *  in list, return it's ref# which was set earlier on page.
 */
int repeatGlobalfn(int fnglobalnum)
{
	int ii;

if (debugger_trace)
p_info(PI_TRACE, "repeatGlobalfn: fnglobalnum=%d.\n", fnglobalnum);

	if (fnglobalnum)
	{
		for (ii=0; ii<num_fnglobals; ii++)
		{
if (debugger_trace)
p_info(PI_TRACE, "Loop fnglobal %d out of %d: id=%d.\n",
ii, num_fnglobals, fnglobals[ii].id);
				/* If found fn ID#, return it's original ref#:  */
			if (fnglobalnum == fnglobals[ii].id)
				return fnglobals[ii].reference;
		}
				/* If more than 10 diff global fns on pg (probably
					never happen), return ref# of tenth one:  */
		if (num_fnglobals >= 10)
			return fnglobals[9].reference;

		/* Add this global fn ID# to next slot in list:  */
		if (!cmd_fm_flag)	/* (but not if scan_fm2() at SoL called kmd73 
								in pre-scan of line for width adjustments) */
			fnglobals[num_fnglobals++].id = fnglobalnum;
	}
	return 0;
}

/*
 *  It's a new global fn on page, and we now have its [fm ref#.
 *  Store it in list of globals, for later testing on this page.
 */
void storeGlobalfnRef(int fnglobalnum, int reference)
{
	int ii;

if (debugger_trace)
p_info(PI_TRACE, "storeGlobalfnRef: fnglobalnum=%d, ref=%d.\n",
fnglobalnum, reference);

	for (ii=0; ii<num_fnglobals; ii++)
	{
if (debugger_trace)
p_info(PI_TRACE, "Loop fnglobal %d out of %d: id=%d.\n",
ii, num_fnglobals, fnglobals[ii].id);

		if (fnglobalnum == fnglobals[ii].id)
			fnglobals[ii].reference = reference;
	}
}

