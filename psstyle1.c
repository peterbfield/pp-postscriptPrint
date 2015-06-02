#include <stdio.h>
#include "p_lib.h"
#include "psjob.h"
#include "frame.h"
#include "rel_data.h"

static void show_linesub(void);
static void show_trapsub(void);
static int validchar(int16 *pchar);
static int do_OF(int caller);      

void trap_a_rule(int strikethru_flag);

extern int16 Current_FgColor;	/*  Fix for BUG 594q used to reset color after underscore  */
extern int16 Current_FgShade;	/*  Fix for BUG 594q used to reset shade after underscore  */
extern int16 wordsp;
extern int16 exld_adj;
extern int16 ntabs;
extern int ktabindex;
extern int instrad;
extern int16 sindex;
extern int16 para_adj;
extern int16 fill_space;
extern int16 fill_count;
extern int16 underscore_min;
extern int16 us_min_mru;
extern int CurInsOffset;
extern int PreInsOffset;
extern int CurDelOffset;
extern int PreDelOffset;
extern int CurInsClr;
extern int PreInsClr;
extern int CurDelClr;
extern int PreDelClr;
extern float CurInsWeight;
extern float PreInsWeight;
extern int vj_para_flag;
extern int save_starting_Xpos, save_khar_wid;
extern int ZeroSetFlag;
extern int AccentStartFlag;
extern int AccentEndFlag;
extern int MathStartFlag;
extern int UncompensatedRoundoff;
extern int flagGlyph;				/* 1= stack() has a glyph ready to print.  */

extern struct onetab
{
    int16 twid, ttype, tx, ty1, ty2;
    int16 tsumlead, tdepth, quad;
} ktab[110];

extern struct onestrad
{
    int16 sx, sy1, sy2, sbeg, send;
    int16 salign, slead, level;
} stab[110];

static int accent_end_xpos;
static int accent_end_roundoff;

char holdfontname[64];
char GlyphName[64];				/* Postscript ID of an [OF character (kmd83)  */
int not_lnum;					/* Tells put_font text vs. line# */
int16 output_ss;
int was_text;					/* Was text put to TP since moveto? */
float Fx, Fy;
float Fx2, Fy3;
int utype;
int us_beg_x1;					/* Start X pos of curr unders seg 1 */
int us_beg_x2;					/* Start X pos of curr unders seg 2 */
int16 usoff[3];
float uswei[3];
int16 uscolr[3];
int16 usshade[3];
int16 ustrapcolr[3];
int16 ustrapshade[3];
int16 ustrapweight[3];
int us_start_Ypos[3];
float Obdegree;
float Old_ob;
float Hold_ob;
int16 Holdfont;
int16 Oldfont;
int16 Holdss;
int16 Holdps;
int16 Oldss;
int16 Oldps;
int usw;

/***********************************************************************/

static void show_linesub(void)
{
    int ii, ich;
	int cnt = 0;
	int space_flag = 0;

#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "show_linesub, octal cc_mask= %lo: (", cc_mask);
#endif
	for(ii=Linesub; ii>0; ii--)
	{							/* if no space, use Show, not Kshow */
		if (Kspace[ii])
		{
			space_flag++;
			break;
		}
	}
	if (space_flag)
	{
		for(ii=Linesub; ii>0; ii--)	/* Put out stack of all character */
		{						/*  offsets in line, last to first. */
			if(Kspace[ii])
				digi_print(Kspace[ii]);
			else
				m_fprintf("0 ");
			cnt++;
			if(cnt>14)
			{
				m_fprintf("\n");
				cnt=0;
			}
		}
		m_fprintf("\n(");
	}
	else
		m_fprintf( "(" );
	for(ii=0; ii<=Linesub; ii++) /* Put out all characters in line. */
	{
		was_text = 1;
		ich = Line[ii];
		if(ich > 31 && ich < 127)
		{
#ifdef TRACE
			if(text_trace)
				p_info(PI_TRACE, "%c", (char )ich);
#endif
			switch(ich)
			{
			  case 92:			/* backslash */
				m_fprintf("\\\\");
				break;
			  case 40:			/* open paren */
				m_fprintf("\\\(");
				break;
			  case 41:			/* close paren */
				m_fprintf("\\)");
				break;
			  default:			/* chars 31 - 127 */
				m_putc (ich);
				break;
			}
		}
		else
		{
			m_fprintf("\\%o", ich);
#ifdef TRACE
			if(text_trace)
				p_info(PI_TRACE, "\\%o", (char )ich);
#endif
		}
	}
	if( !space_flag)
	{
		m_fprintf(")S\n");
#ifdef TRACE
		if(text_trace)
			p_info(PI_TRACE, ")S\n");
#endif
	}
	else
	{
		m_fprintf(")K\n");
#ifdef TRACE
		if(text_trace)
			p_info(PI_TRACE, ")K\n");
#endif
	}
	if(Kspace[Linesub+1])
	{
		m_fprintf("%4.3f 0 RM\n", Kspace[Linesub+1]);
#ifdef TRACE
		if(text_trace)
			p_info(PI_TRACE, "%4.3f 0 RM\n", Kspace[Linesub+1]);
#endif
	}
}								/* end function */

/***********************************************************************/

static void show_trapsub(void)
{
    int ii, ich, jj;
	float trap_stroke;
	uint32 save_cc_mask, loop_temp,	BlackPaint_cc_mask_sav;
	uint32 save_active_trap_cc_mask;

#ifdef TRACE
    if(text_trace)
		p_info(PI_TRACE, "show_trapsub, octal cc_mask= %lo: ((", cc_mask);
#endif
	was_text = 1;
	ii = 0;
	trap_stroke = (float )ActiveTrapStrokeWidth / HorizontalBase;
	m_fprintf( "currentpoint newpath M (");
	if (flagGlyph)
		Linesub = 0;			/* Use the while-loop once.  */

	while (ii <= Linesub)
	{
/* Put out all characters in line which have no added space. */
		ich = Line[ii];
		if (flagGlyph)
			m_fprintf("/%s glyphshow\n", GlyphName);
		else if(ich > 31 && ich < 127)
		{
#ifdef TRACE
			if(text_trace)
				p_info(PI_TRACE, "%c", (char )ich);
#endif
			switch(ich)
			{
			  case 92:			/* backslash */
				m_fprintf("\\\\");
				break;
			  case 40:			/* open paren */
				m_fprintf("\\\(");
				break;
			  case 41:			/* close paren */
				m_fprintf("\\)");
				break;
			  default:			/* chars 31 - 127 */
				m_putc (ich);
				break;
			}
		}
		else
		{
			m_fprintf("\\%o",ich);
#ifdef TRACE
			if(text_trace)
				p_info(PI_TRACE, "\\%o", (char )ich);
#endif
		}
		if ( ( Kspace[ii+1] && (ii < Linesub)) || (ii == Linesub) )
		{						/* end text group and put out space and '(' if
								   needed */
			save_cc_mask = cc_mask;
			save_active_trap_cc_mask = active_trap_cc_mask;
			m_fprintf(") TPB ");
#ifdef TRACE
			if(text_trace)
				p_info(PI_TRACE, ") TPB\n");
#endif
			for (jj=0; jj<MAX_CLRS; jj++)
			{
				cc_mask = ( loop_temp = (1 << jj) ) & save_cc_mask;
				if(loop_temp > save_cc_mask)
					break;
				if (!cc_mask)
					continue;
				active_trap_cc_mask = cc_mask & save_active_trap_cc_mask;
				if ( !active_trap_cc_mask)
				{
					m_fprintf("0 TPE\n");
#ifdef TRACE
					if(text_trace)
						p_info(PI_TRACE, "0 TPE\n");
#endif
				}
				else
				{
					BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
					vid_color (ActiveTrapColor, ActiveTrapShade, jj+1);
					BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
					m_fprintf("%3.3f TPE\n", trap_stroke);
#ifdef TRACE
					if(text_trace)
						p_info(PI_TRACE, "%3.3f TPE\n", trap_stroke);
#endif
				}
				if (ii < Linesub)
				{
					if(Kspace[ii+1])
					{
						m_fprintf("%4.3f 0 RM\ncurrentpoint newpath M (",
								  Kspace[ii+1]);
#ifdef TRACE
						if(text_trace)
							p_info(PI_TRACE, "%4.3f 0 RM\ncurrentpoint newpath M (",
								   Kspace[ii+1]);
#endif
					}
				}
			}					/* end for (jj=0;jj<MAX_CLRS;jj++) */
			cc_mask = save_cc_mask;
			active_trap_cc_mask = save_active_trap_cc_mask;
		}						/* end of end text group processing */
		ii += 1;
	}							/* end while(ii<=Linesub) */
	if(Kspace[Linesub+1])
	{
		m_fprintf("%4.3f 0 RM\n",Kspace[Linesub+1]);
#ifdef TRACE
		if(text_trace)
			p_info(PI_TRACE, "%4.3f 0 RM\n",Kspace[Linesub+1]);
#endif
	}
}								/* end function */


/***********************************************************************
 **  STACK_PRINT()   Set all the chars encountered thus far. **
 ***********************************************************************/
void stack_print(void)
{
	int ii;
	uint32 save_cc_mask, save_trap_cc_mask;
	float save_kspace0;

    output_ss = Oldss;
#ifdef TRACE
    if(text_trace)
		p_info(PI_TRACE, "stack_print - octal cc_mask= %lo, Linesub= %d\n",
			   cc_mask, Linesub);
#endif
    if( Kspace[0] && ((Linesub >= 0) || flagGlyph) && cc_mask )
    {
#ifdef TRACE
		if(text_trace)
			p_info(PI_TRACE, "%f 0 RM\n", Kspace[0]);
#endif
		digi_print(Kspace[0]);
		m_fprintf("0 RM\n");
    }
	save_kspace0 = Kspace[0];
    Kspace[0] = 0;
/* put out text line (if any) and trapping (if any) */
    if ((Linesub >= 0) || flagGlyph)
	{
		if (pdflinkAtSOL && pdflinkctr &&	/* If PDF link area was needed at start of line,  */
											/*  and this is text from .fo file,  */
			(!FileType || TYPE_OF_FRAME(CurrentFrame) != PL_TEXT))
		{								/*  and this is first char or
											cmd of line, mark start of area:  */
			char pdfstring[48];

			pdflinkAtSOL = 0;
			m_fprintf("currentpoint\n");
			memset(pdfstring,0,sizeof(pdfstring));
			sprintf(pdfstring,"/lSy%d exch def /lSx%d exch def\n",pdflinkctr,pdflinkctr);
			m_fprintf(pdfstring);
		}
		if (cc_mask)
		{						/* output text as needed */
			save_trap_cc_mask = active_trap_cc_mask;
			active_trap_cc_mask &= cc_mask;
			if ( active_trap_cc_mask)
				m_fprintf("GS\n"); /* to get back to the line start */
			if (!flagGlyph)
				show_linesub();		/* push widths & show string */
			else
			{
				m_fprintf("/%s glyphshow\n", GlyphName);
				was_text = 1;
				if(Kspace[1])
					m_fprintf("%4.3f 0 RM\n", Kspace[1]);
			}
			if ( active_trap_cc_mask)
				m_fprintf("GR\n"); /* to get back to the line start */
			active_trap_cc_mask = save_trap_cc_mask;
		}						/* done with the text */
		if ( active_trap_cc_mask)
		{						/* need to output trap */
			save_cc_mask = cc_mask;
			cc_mask = active_trap_cc_mask;
			if ( save_kspace0 && cc_mask && !save_cc_mask)
			{					/* initial space never output, do it now */
#ifdef TRACE
				if(text_trace)
					p_info(PI_TRACE, "%f 0 RM\n", save_kspace0);
#endif
				digi_print( save_kspace0);
				m_fprintf("0 RM\n");
			}
			show_trapsub();
			cc_mask = save_cc_mask;
		}						/* end if(active_trap_cc_mask>=0) */
		for(ii=0; ii<Linesub+2; ii++) /* wipe the text list out */
		{
			Line[ii] = 0;
			Kspace[ii] = 0;
		}
	}							/* end if(Linesub>0) */
    if ((Linesub <= 0) && !flagGlyph)
		was_text = 0;
    Linesub = -1;
	flagGlyph = 0;
}		/* End function stack_print(void)  */

/***********************************************************************
 **  CREATE1PSFONT(psfname,psfps,psfss,obdegree) **
 ***********************************************************************/
void create1psfont(char *psfname, int psfps, int psfss, float obdegree_tmp)
{
    if(Holdfont == fo_line_def.SolFont  && Holdps  == psfps &&
       Holdss   == psfss && Hold_ob == obdegree_tmp)
		return;
    strcpy(holdfontname, psfname);
	Oldfont = -1;
    Hold_ob = obdegree_tmp;
    Holdfont = fo_line_def.SolFont;
    Holdps = psfps;
    Holdss = psfss;
}

/***********************************************************************
 **  PUT_FONT()    Put out font call at current point size, also **
 **        set size & slant if needed.  Called by stack() each time a **
 **        char is placed in line, if type style differs. **
 **	  Also called by put_lnum to call line# font, if needed. **
 ***********************************************************************/
void put_font(void)
{
	int ii;
    float tps, tss;
    char temp[20];

	if(Oldfont!=Holdfont || Oldss!=Holdss)
		width_read();			/* Re-calc font char widths. */
	if (!cc_mask && !active_trap_cc_mask)
	{
		Old_ob = Hold_ob;
		Oldfont = Holdfont;
		Oldps = Holdps;
		Oldss = Holdss;
		return;
    }
	if ( !Holdps || !Holdss)
		return;
    tps = (float)Holdps / (float)HorizontalBase; /* Point size in point */
    tss = (float)Holdss / (float)HorizontalBase; /* Set size in points */
    if (not_lnum)
	{
		Ofx = -4000;			/* force Move command output */
		do_pskmd('m', "putfont"); /* put font */
	}
    if( !Hold_ob)
    {
		if(Holdps == Holdss)
		{
			sprintf(temp, "%3.2f", tps);
			for(ii=0; ii<4; ii++)
				if(temp[ii] == '.') break;
			if(temp[ii+2] == '0') /* Reduce "xx.x0" to "xx.x" */
			{
				temp[ii+2] = 0;
				if(temp[ii+1] == '0') /* Reduce "xx.0" to "xx" */
				{
					temp[ii+1] = 0;
					temp[ii] = 0 ;
				}
			}
			m_fprintf("%s /%s F\n",temp,holdfontname);
		}						/* "10.25 /Helvetica F" */
		else
		{						/* "/Helvetica FF [10.50 0 0 10.25 0 0] MF" */
			m_fprintf("/%s FF [", holdfontname);
			sprintf(temp, "%3.2f", tss);
			for(ii=0; temp[ii]; ii++)
				m_fprintf("%c", temp[ii]);
			m_fprintf(" 0 0 ");
			sprintf(temp,"%3.2f", tps);
			for(ii=0; temp[ii]; ii++)
				m_fprintf("%c", temp[ii]);
			m_fprintf(" 0 0] MF\n");
		}
    }
    else
    {							/* "/Helvetica FF [10.5 0 2.1 10.25 0 0] MF" */
		m_fprintf("/%s FF [", holdfontname);
		sprintf(temp, "%3.2f ", tss);
		for(ii=0; temp[ii]; ii++)
			m_fprintf("%c", temp[ii]);
		m_fprintf(" 0 ");
		sprintf(temp, "%2.5f ", Hold_ob);
		for(ii=0; temp[ii]; ii++)
			m_fprintf("%c", temp[ii]);
		m_fprintf(" ");
		sprintf(temp, "%3.2f ", tps);
		for(ii=0; temp[ii]; ii++)
			m_fprintf("%c", temp[ii]);
		m_fprintf(" 0 0] MF\n");
    }
	Old_ob = Hold_ob;
	Oldfont = Holdfont;
	Oldps = Holdps;
	Oldss = Holdss;
    NoFont = 1;
	if ( !holdfontname[0])
	{
		if ( LYPrintFlag)
		{
			log_font("Courier", cc_mask | active_trap_cc_mask);
			if ( Kmd_PP_Started)
				add_font("Courier", cc_mask | active_trap_cc_mask, &E_fonts);
		}
		else
		{
			log_font("Helvetica", cc_mask | active_trap_cc_mask);
			if ( Kmd_PP_Started)
				add_font("Helvetica", cc_mask | active_trap_cc_mask, &E_fonts);
		}
	}
	else
	{
		log_font(holdfontname, cc_mask | active_trap_cc_mask);
		if ( Kmd_PP_Started)
			add_font(holdfontname, cc_mask | active_trap_cc_mask, &E_fonts);
	}
}

/***********************************************************************
 **  CAMY(kld,message_string)    To advance leading, update Ypos. **
 **				Message is for tracing only. **
 ***********************************************************************/
void camy(int16 kld, char *mes_str)
{
	uint32 save_cc_mask;
	int16 gal_depth;

	if ( CurrentLineBcEc)
	{							/* no output */
		BcEcExtraLeadNeeded = 0;
		Ofx = -4000;			/* force Move command output */
		do_pskmd('M',mes_str);	/* Move to current XY coords. */
		return;
	}
	Ypos += kld + BcEcExtraLeadNeeded;
	BcEcExtraLeadNeeded = 0;
	save_cc_mask = cc_mask;
    if ( (Ypos >= Imageheight) && /* If reached PS page limit, */
		 ( !FileType || in_overflow)  && !Ktabmode)
    {
		if (spot_pass == 1)
			cc_mask = 1 & Plates;
		else
			cc_mask = cc_hit;
		if (in_overflow)
			OverflowOrColorFlag = -1;
		gal_depth = Imageheight - (Ypos - kld);
		do_pskmd('P',mes_str);	/* show page, start new. */
		resend_kmd_bf(gal_depth);
		Ypos = kld;
    }
	Ofx = -4000;				/* force Move command output */
    do_pskmd('M', mes_str);		/* Move to current XY coords. */
	cc_mask = save_cc_mask;
}

/***********************************************************************
 **  VALIDCHAR(pchar) **
 ***********************************************************************/
static int validchar(int16 *pchar)
{
	unsigned char vchar = *pchar;

    if (vchar < 1)
    {
		p_info(PI_WLOG, "%5d : Invalid Penta character code. \n", *pchar);
		return(0);
    }
    if ( PsfCode[PsfCodeIndex].code[vchar] < 1 )
    {
		p_info(PI_WLOG, "No Postscript character for Penta code %1d, use ? \n", *pchar);
		*pchar = 59;
    }
    return(1);					/* Valid Character */
} /* end validchar */

/***********************************************************************
 **  KMD1()    Change font.                                            **
 ***********************************************************************/
int kmd1()	
{
	int16 ii;

    ii = foget();
    foget();
    if (ii != Holdfont)
    {
		fo_line_def.SolFont = ii;
		getftab();				/* get postscript font table */
		create1psfont(PentaToPsf[PsfNameIndex].name, fo_line_def.SolPointSize,
					  fo_line_def.SolSetSize, Obdegree);
    }
	return(0);
}

/***********************************************************************
 **  KMD2()    Change Point Size.                                      **
 ***********************************************************************/
int kmd2()		
{
	int16 ii;

    ii = foget();
    if (ii != Holdps || ii != Holdss) /* If this changes Ptsz or Setsz, */
    {							/* set vars to trigger change */
		fo_line_def.SolPointSize =
			fo_line_def.SolSetSize = ii; /* at next character. */
		if ( Obdegree)
			Obdegree = (ii * 0.21255) / HorizontalBase;
		create1psfont(PentaToPsf[PsfNameIndex].name, fo_line_def.SolPointSize,
					  fo_line_def.SolSetSize, Obdegree);
    }
	return(0);
}

/***********************************************************************
 **  KMD3()    Change Leading. (supported from LDEF value)             **
 ***********************************************************************/
int kmd3()
{
    foget();
	return(0);
}

/***********************************************************************
 **  KMD6()    One Line Lead  (supported from LDEF value)              **
 ***********************************************************************/
int kmd6()
{
    foget();
	return(0);
}

/***********************************************************************
 **  KMD7()   Leader fill.                                            **
 ***********************************************************************/
int kmd7()
{
	int16 k1, k2, k3, k4, k5, kchrwid;
    int16 kchar, kwid, kexsp, kmin, kolwk, ind_rite, ind_left;
    float ldr_adj;
    int hold_xpos, hold_roundoff_bucket, hold_uncompensated_roundoff;
	int Int_kwid, Int_kolwk, Int_k5, Int_k6;
	int remove_count = 0;

    kchar = foget();			/* raw character number */
    kchrwid = foget();			/* character width */
    kexsp = foget();			/* extra space around each character */
    kmin = foget();				/* preapplied minimum fill length */
    kolwk = foget();			/* Length from start of fill to end of line. */
    if((Oldfont != Holdfont) || (Oldps != Holdps) || (Oldss != Holdss) ||
       (Old_ob  != Hold_ob))
		put_font();				/* Put font change to TP file.
								   Also sets "old-" variables.*/
	k4 = fill_space/fill_count;
    if (validchar(&kchar))
    {
		hold_xpos = Xpos;
		hold_roundoff_bucket = RoundoffBucket;
		hold_uncompensated_roundoff = UncompensatedRoundoff;
		k2 = k4 + kmin;			/* Space for this [lf command
								   (In quad-type 4, fo_line_def.Bands is
								   the number of leader fills) */
		Int_k6 = UncompensatedRoundoff / FontBase;
		UncompensatedRoundoff %= FontBase;
		k2 -= Int_k6;			/* reduce space avail by rounding err to here */
		kolwk -= Int_k6;
		Xpos += Int_k6 + k2;	/* Bump X-pos by that length */
		if (FlashOn)
		{
			kwid = kchrwid + kexsp; /* Width of each ldr char + extra space */
			kolwk -= k2;		/* Align leader dots between all leader lines:
								   Get width  of text from end of ldrs to
								   end of measure. */
			Int_kolwk = kolwk * FontBase;
			Int_kwid = (kwid * FontBase) + Widval[Widindex].roundoffs[kchar-1];
			Int_k5 = Int_kolwk % Int_kwid;
			if (Int_kolwk && Int_k5)
				ind_rite = (Int_kwid - Int_k5) / 
					FontBase;	/* As if text were overlayed on leader dots 
								   that filled measure, calc space from last 
								   dot to text start, including rounding. */
			else
				ind_rite = 0;
			k2 -= ind_rite;		/* Reduce ldr-wid by the indnt */
			k5 = k2 - kchrwid;	/* ldr char + space pairs; plus 1 extra char */
			if ( k5 < 0)
			{
				fill_space += (kmin * fill_count);
				Xpos = hold_xpos;
				RoundoffBucket = hold_roundoff_bucket;
				UncompensatedRoundoff = hold_uncompensated_roundoff;
				kmd12();		/* treat as fill with white space */
				return(0);
			}
			k1   = k5 / kwid;	/* nbr of characters plus space to put. */
			ind_left = k5 % kwid; /* Remaining partial space is
									 set as left indent. */
/*  reduce left indent by sum of leader width roundoffs lost by 16-bit H&J: */
			RoundoffBucket += ((k1 +1) * Widval[Widindex].roundoffs[kchar-1]);
			ind_left -= (RoundoffBucket / FontBase);
			RoundoffBucket %= FontBase; /* (reset bucket for next) */
			while (ind_left < 0)
			{					/* rounding causes interference */
				k5 -= kwid;		/* remove one pair */
				remove_count++;
				if ( k5 < 0)
				{
					fill_space += (kmin * fill_count);
					Xpos = hold_xpos;
					RoundoffBucket = hold_roundoff_bucket;
					UncompensatedRoundoff = hold_uncompensated_roundoff;
					kmd12();	/* treat as fill with white space */
					return(0);
				}
				k1 = k5 / kwid; /* nbr of characters plus space to put. */
				ind_left = (k5 % kwid) + (remove_count * kwid);
				RoundoffBucket = hold_roundoff_bucket;
				RoundoffBucket +=((k1+1) * Widval[Widindex].roundoffs[kchar-1]);
				ind_left -= RoundoffBucket/FontBase;
				RoundoffBucket %= FontBase; /* (reset bucket for next) */
			}					/* end while(ind_left<0) */
			if (k1 <= 0)
			{					/* white space if not enough ldrs */
				fill_space += (kmin * fill_count);
				Xpos = hold_xpos;
				RoundoffBucket = hold_roundoff_bucket;
				UncompensatedRoundoff = hold_uncompensated_roundoff;
				kmd12();		/* treat as fill with white space */
				return(0);
			}
			if (cc_mask || active_trap_cc_mask)
				Kspace[Linesub+1] += (float)ind_left/
					HorizontalBase; /* goes before leaders. */
								/* Pre-calc leader-dot entrys: */
			kchar = ((int16)PsfCode[PsfCodeIndex].code[kchar]) & 0xff;
			ldr_adj = ((float)kexsp +
					   fo_line_def.SolCwAdj)/HorizontalBase; /* char adjust. */
			if ( (cc_mask || active_trap_cc_mask) && !DontPaintFlag)
				PaintOnPage |= BlackPaint_cc_mask;
			for (k3=0; k3<k1; k3++)
			{					/* Put out each leader + following space */
				if (cc_mask || active_trap_cc_mask)
				{
					Line[++Linesub] = kchar;
					if (ldr_adj) 
						Kspace[Linesub+1] += ldr_adj;
				}
			}
			if (cc_mask || active_trap_cc_mask)
			{
				Line[++Linesub] = kchar; /* final leader character goes out */
				Kspace[Linesub+1] += (float)ind_rite/
					HorizontalBase; /* Aligning space. */
			}
			UncompensatedRoundoff += RoundoffBucket;
		}
		else					/* In no-flash mode, merely */
								/*  offset nxt char by ldr wid */
			if (cc_mask || active_trap_cc_mask)
				Kspace[Linesub+1] += (float)k2 / HorizontalBase;
    }
	fill_space -= k4;
	fill_count--;
	return(0);
}

/***********************************************************************
 **  KMD9()    Change Measure. **
 ***********************************************************************/
int kmd9()
{
    fo_line_def.SolMeasure = foget();
	return(0);
}

/***********************************************************************
 **  KMD12()   [WS command - fill with space **
 ***********************************************************************/
int kmd12()
{
	int16 k1;
	int Int_k6;

	Int_k6 = UncompensatedRoundoff / FontBase;
	UncompensatedRoundoff %= FontBase;
	RoundoffBucket = UncompensatedRoundoff;
	if (cc_mask || active_trap_cc_mask)
		Kspace[Linesub+1] += (((float)fill_space /  fill_count) - 
							  (float)Int_k6) / HorizontalBase;
	k1 = (fill_space / fill_count);
    Xpos  += k1;
	fill_space -= k1;
	fill_count--;
	return(0);
}

/***********************************************************************
 **  KMD13()    Extra Lead. **
 ***********************************************************************/
int kmd13()
{
    int16 val;

    val = foget();
	camy(val, "camy extra lead."); /* Move down ExtraLead value, */
	return(0);
}

/***********************************************************************
 **  KMD25()   One Character Font Change. **
 ***********************************************************************/
int kmd25()      
{
	return (do_OF(25));
}

/***********************************************************************
 **  do_OF()  Subroutine common to three variants of one-char font change:
 **           kmd25() One Character Font Change, normal.
 **           kmd74() Accent.
 **           kmd83() One Character Font Change by PostScript char name.
 *************************************************************************/
int do_OF(int caller)      
{
    int16 jps, jss, ibl, jsl, khar, kwd, FONTSAV;
	int16 rounding_error, lenGlyphName;
	int save_cur_Xpos;
    float tempob;
	float excess;

    FONTSAV = fo_line_def.SolFont;
    fo_line_def.SolFont = foget();	/* font number */
    jps = foget();				/* point size */
    jss = foget();				/* set size */
    ibl = foget();				/* baseline deflection */
    jsl = foget();				/* slant switch */
	*GlyphName = 0;				/* Flag:  We use normal font loc, not the ID.  */
    khar = foget();				/* raw character number */
    kwd = foget();				/* character width */
	if (caller == 83)			/* But when we ID char by its name,  */
	{
		int ii;
		lenGlyphName = kwd + kwd;/*  last arg holds length of name,  */
		kwd = khar;				/*  2nd-to-last holds width.  */
		for (ii=0; ii<lenGlyphName; ii+=2)
		{
			khar = foget();
			*(GlyphName+ii) = khar >> 8;
			*(GlyphName+ii+1) = khar & 0177;
		}
		*(GlyphName+ii) = 0;
		khar = 0;				/* Flag:  We use ID instead of font loc.  */
	}
    if(FONTSAV != fo_line_def.SolFont)
	{
		getftab();
					/* If this is invalid kmd25 inserted due to H&J
						bug, then getftab() has genned an err msg.
						Restore orig font, exit with no output:  */
		if ((fo_line_def.SolFont <= 0) && (khar <= 0))
		{
			fo_line_def.SolFont = FONTSAV;
			return(0);
		}
	}
    if ((khar < 0) ||				/* Direct PS font location, or  */
		(khar==0 && *GlyphName) ||	/* direct PS font character name, or  */
		validchar(&khar) )			/* regular penta font location, validated.  */
    {
		khar_wid = kwd;
		if ((jsl==1) || (jsl==5))
			tempob = (jps * 0.21255) / HorizontalBase;
		else
			tempob = 0;
		if ( ibl && (caller != 74))	/* For non-accents, go to baseline given in [of. */
		{
			jbl = ibl;
			Ofx = -4000;		/* force Move command output */
			do_pskmd('m',"kmd25 single char font change");
		}
		create1psfont(PentaToPsf[PsfNameIndex].name, jps, jss, tempob);
		if((caller == 74) && ((Oldfont != Holdfont) || (Oldps != Holdps) ||
			(Oldss != Holdss) || (Old_ob  != Hold_ob)))
			put_font();			/* Put font change to TP file.
								   Also sets "old-" variables.*/
		if (khar > 0)
		{
			ackhar = khar;
			if ( !MathStartFlag)
			{
				if ((rounding_error = RoundoffBucket / FontBase))
				{					/* adjust Xpos to true position */
					RoundoffBucket %= FontBase;
					UncompensatedRoundoff = RoundoffBucket;
					Xpos += rounding_error;
					wordsp -= rounding_error;
				}
			}
		}
		else					/* For direct font chars, avoid roundoff stuff  */
			ackhar = 0;

		if (caller == 74)		/* For accents, bump baseline by val from [of.  */
		{
			jbl += ibl;
			Ofx = -4000;			/* force Move command output */
			do_pskmd('m',"kmd74 accent");
		}
		save_cur_Xpos = Xpos;
		if (ZeroSetFlag)
		{
			if ((cc_mask  | active_trap_cc_mask) && FlashOn)
			{					/* if not active, no output needed */
				stack_print();	/* Put preceeding chars, if any, to TP. */
				m_fprintf("currentpoint\n");
				excess = (save_khar_wid - kwd) / 2;
				save_cur_Xpos = Xpos;
				Xpos = save_starting_Xpos + excess;
				Ofx = -4001;	/* force Move command output */
				do_pskmd('m', "zeroset");
			}
		}
		if (khar > 0)			/* Regular penta font location  */
			stack(((int16)PsfCode[PsfCodeIndex].code[khar]) & 0xff);
		else if (khar < 0)		/* Direct PS font location  */
		{
			uchar ii = -khar;
			stack(ii);
		}
		else					/* Direct PS font character name:  */
			stack(0);			/* Name is in global *GlyphName.  */

		if (ZeroSetFlag)
		{
			if ((cc_mask  | active_trap_cc_mask) && FlashOn)
			{					/* if not active, no output needed */
				stack_print();	/* Put cur char to TP. */
				m_fprintf("M\n"); /* move to currentpoint */
				Xpos = save_cur_Xpos;
			}					/* end output of if(ZeroSetFlag) */
			ZeroSetFlag = 0;	/* Zero-set no longer pending. */
		}						/* end if(ZeroSetFlag) */

		if ( ibl || (caller == 74))
		{
			switch (caller)
			{
			  case 25:
			  case 83:
				jbl = fo_line_def.SolBaseLine;
				break;
			  case 74:
				jbl -= ibl;
			}
			Ofx = -4000;		/* force Move command output */
			if (caller == 74)
				RoundoffBucket -= Widval[Widindex].roundoffs[ackhar-1];
			do_pskmd('M',"kmd25 single char font change");
		}
    }
    if (fo_line_def.SolFont != FONTSAV)
    {
		fo_line_def.SolFont = FONTSAV;
		getftab();
    }
    create1psfont(PentaToPsf[PsfNameIndex].name, fo_line_def.SolPointSize,
				  fo_line_def.SolSetSize, Obdegree);
	return(0);
}

/***********************************************************************
 **  KMD26()   Equation size.  Used for character pair kerning. **
 ***********************************************************************/
int kmd26()
{
	int16 k1;

    k1 = foget();
    Xpos += k1;
	if (cc_mask || active_trap_cc_mask)
		Kspace[Linesub+1] += ((float )k1/(float )HorizontalBase);
	return(0);
}

/***********************************************************************
 **  KMD27()   Flash On. **
 ***********************************************************************/
int kmd27()
{
    FlashOn = 1;
	return(0);
}

/***********************************************************************
 **  KMD28()   Flash Off. **
 ***********************************************************************/
int kmd28()
{
    FlashOn = 0;
	return(0);
}

/***********************************************************************
 **  KMD34()   EditTrace. **
 ***********************************************************************/
int kmd34()
{
	int16 k1;
	uint32 temp_clr_mask;

	temp_clr_mask = cc_mask;
    k1 = foget();				
	/* EditTrace values:		1 = delete current
								2= delete prev
								4 = start add current
								8 = end add current
								10(hex) = start add previous
								20(hex) = end add previous */
	if ( !EditTraceFlag)		/* 1 = current, 2 = prev, 3 = both enabled */
		return(0);
	if (EditTraceFlag & 1)
	{							/* current level */

		switch (k1)
		{
		  case 1:				/* 1 = delete current */
			if ( !ET_LineStartedFlag || ET_FakeTabLineFlag)
				ET_DeleteDelay1++;
			else
				set_edit_trace(3);
			break;
		  case 4:				/* 4 = start add current */
			if ( EditTraceStarted & 1)
			{
				ET_InsertEndDelay1 = 0;
				break;			/* ignore start if already started */
			}
			if ( !ET_LineStartedFlag || ET_FakeTabLineFlag)
			{
				ET_InsertStartDelay1++;
				ET_InsertEndDelay1 = 0;
			}
			else
			{
				EditTrace_beg_x1 = Xpos;
				EditTraceStarted |= 1;
			}
			break;
		  case 8:				/* 8 = end add current */
			if ( (!EditTraceStarted & 1) && !ET_InsertStartDelay1 )
				break;			/* ignore end if not started */
			if ( !ET_LineStartedFlag || ET_FakeTabLineFlag)
				ET_InsertEndDelay1++;
			else
				set_edit_trace(1);
			break;
		}						/* end switch(k1) */
	}							/* end if(EditTraceFlag&1) */
	cc_mask = temp_clr_mask;
	if (EditTraceFlag & 2)
	{							/* previous level */
		switch (k1)
		{
		  case 2:				/* 2 = delete prev */
			if ( !ET_LineStartedFlag || ET_FakeTabLineFlag)
				ET_DeleteDelay2++;
			else
				set_edit_trace(4);
			break;
		  case 0x10:			/* 0x10 = start add prev */
			if ( EditTraceStarted & 2)
			{
				ET_InsertEndDelay2 = 0;
				break;			/* ignore start if already started */
			}
			if ( !ET_LineStartedFlag || ET_FakeTabLineFlag)
			{
				ET_InsertStartDelay2++;
	/*** removed for Bug 374	ET_DeleteDelay2 = 0;	***/
				ET_InsertEndDelay2 = 0;		/*** Bug 374 ***/
			}
			else
			{
				EditTrace_beg_x2 = Xpos;
				EditTraceStarted |= 2;
			}
			break;
		  case 0x20:			/* 0x20 = end add prev */
			if ( (!EditTraceStarted & 2) && !ET_InsertStartDelay2 )
			{
				ET_InsertEndDelay2 = 0;
				break;			/* ignore end if not started */
			}
			if ( !ET_LineStartedFlag || ET_FakeTabLineFlag)
				ET_InsertEndDelay2++;
			else
				set_edit_trace(2);
			break;
		}						/* end switch(k1) */
	}							/* end if(EditTraceFlag&2) */
	cc_mask = temp_clr_mask;
	return(0);
}

/****************************************************************************
 **  SET_EDIT_TRACE()   Process Edit Trace - set color, output add or del. **
 ****************************************************************************/
void set_edit_trace(int level)
{
	int ii;
	int new_rule_color;
	uint32 plates_hold, BlackPaint_cc_mask_sav;
	uint32 loop_temp;
	uint32 current_color_cc_mask;
	int rule_color = 0;
	uint32 temp_clr_mask = 0;
	uint32 start_clr_mask;

	if ( ET_FakeTabLineFlag)
		return;
	start_clr_mask = cc_mask;
	if (spot_pass == 1)
		cc_mask = 1 & Plates;
	else
		cc_mask = cc_hit;
	if ( !cc_mask)
	{
		cc_mask = start_clr_mask;
		return;
	}
	do_pskmd('m',"begin edit trace");
	m_fprintf("GS\n");			/* save current color before rules */
	switch (level)
	{
	  case 1:					/* add level 1 */
		rule_color = CurInsClr;
		EditTraceStarted &= ~1;
		break;
	  case 2:					/* add level 2 */
		rule_color = PreInsClr;
		EditTraceStarted &= ~2;
		break;
	  case 3:					/* delete level 1 */
		rule_color = CurDelClr;
		break;
	  case 4:					/* delete level 2 */
		rule_color = PreDelClr;
		break;
	}							/* end switch(level) */
	if (rule_color <0)
		rule_color = DefaultFrameFgColor;
	temp_clr_mask = 0;
	set_pass_2_color(rule_color, &temp_clr_mask, 1);
	temp_clr_mask &= Plates;
	if(spot_pass == 1)
	{							/* doing the black pass */
		current_color_cc_mask = temp_clr_mask & 1;
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
	}							/* end if(spot_pass==1) */
	else
	{							/* pass 2 */
		new_rule_color = rule_color;
		current_color_cc_mask = temp_clr_mask & ~1;
	}
	if ( current_color_cc_mask )
	{							/* output the colors for this mask */
		BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
		for (ii=0; ii<MAX_CLRS-1; ii++)
		{						/* output the color and v-rule */
			plates_hold = ( loop_temp = (1 << ii) ) & current_color_cc_mask;
			if(loop_temp > current_color_cc_mask)
				break;
			if ( !plates_hold)
				continue;
			cc_mask = loop_temp;
			BlackPaint_cc_mask = 0;
			vid_color(new_rule_color, 100, ii+1); /* output color */
			if (level <= 2)
				set_edit_trace_add(level);
			else
				set_edit_trace_del(level);
			PaintOnPage |= BlackPaint_cc_mask;
		}						/* end for(ii=0;ii<MAX_CLRS-1;ii++)  */
		BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
	}							/* done with output of all the colors */
	if (spot_pass == 1)
		cc_mask = 1 & Plates;
	else
		cc_mask = cc_hit;
	m_fprintf("GR\n"); 			/* restore current color,from before rules */

	/* Bug 370p Edit trace does not return to current color */
	color_func(Current_FgColor, Current_FgShade); 
	cc_mask = start_clr_mask;
}

/***********************************************************************
 **  SET_EDIT_TRACE_ADD()   Process Edit Trace additions. **
 ***********************************************************************/
void set_edit_trace_add(int level)
{
	float save_fx, save_fx2, save_fy, save_fy3;
	float fx2_temp = Xpos;
	
	save_fx = Fx;
	save_fx2 = Fx2;
	save_fy = Fy;
	save_fy3 = Fy3;
	Fx2 = Xpos;
	if (level == 1)
    {
		Fx = EditTrace_beg_x1;
		Fy = Ypos + CurInsOffset;
		Fy3 = Fy + CurInsWeight;
	}
    else
    {
		Fx = EditTrace_beg_x2;
		Fy = Ypos + PreInsOffset;
		Fy3 = Fy + PreInsWeight;
	}
	if ( Fx > Fx2)
	{
		Fx = save_fx;
		Fx2 = save_fx2;
		Fy = save_fy;
		Fy3 = save_fy3;
		Xpos = fx2_temp;
		return;
	}

/*****  Removed for Bug 374p
	if ( (Fx2 - Fx) < (2 * HorizontalBase) )
		Fx2 = Fx + (2 * HorizontalBase);
 *****/
 if ( !((Fx2 - Fx) < (2 * HorizontalBase)) )	/** Bug 374p **/
	do_pskmd('L', "add edit_trace");
	Fx = save_fx;
	Fx2 = save_fx2;
	Fy = save_fy;
	Fy3 = save_fy3;
	Xpos = fx2_temp;
    EditTrace_beg_x1 = Xpos;
	Ofx = -4000;				/* force Move command out */
	do_pskmd('M',"end add edit");
}								/* end function */

/***********************************************************************
 **  SET_EDIT_TRACE_DEL()   Process Edit Trace deletions. **
 ***********************************************************************/
void set_edit_trace_del(int level)
{
	float save_fx, save_fx2, save_fy, save_fy3;
	float fx2_temp = Xpos;
	float mark_wid, mark_dep;

	if ( !FlashOn)
		return;
	mark_wid = 2.5 * HorizontalBase;
	mark_dep = 2.0 * VerticalBase;
	do_pskmd('M',"begin edit trace del"); 
	save_fx = Fx;
	save_fx2 = Fx2;
	save_fy = Fy;
	save_fy3 = Fy3;
	Fx = Xpos - mark_wid;
	Fx2 = Xpos + mark_wid;
	if (level == 3)
    {
		Fy = Ypos + CurDelOffset;
		Fy3 = Fy - mark_dep;
	}
    else
    {
		Fy = Ypos + PreDelOffset - mark_dep;
		Fy3 = Fy + mark_dep;
	}
	do_pskmd('E', "edit_trace del marker");
	Fx = save_fx;
	Fx2 = save_fx2;
	Fy = save_fy;
	Fy3 = save_fy3;
	Xpos = fx2_temp;
	Ofx = -4000;				/* force Move command out */
	do_pskmd('M',"end del edit");
}								/* end function */

/***********************************************************************
 **  KMD42()   Pagination Mark. **
 ***********************************************************************/
int kmd42()	
{
    int16 ival, hf, hp, hs;

    ival=foget();
    if (ival)
    {
		hf = fo_line_def.SolFont;
		hp = fo_line_def.SolPointSize;
		hs = fo_line_def.SolSetSize;
		kfolio(ival);			/* Output folio */
		if ( (hf != fo_line_def.SolFont) || (hp != fo_line_def.SolPointSize) ||
			 (hs != fo_line_def.SolSetSize) )
		{      		/* If folio caused font or size change, get back. */
			fo_line_def.SolFont = hf;
			fo_line_def.SolPointSize = hp;
			fo_line_def.SolSetSize = hs;
			getftab();
			create1psfont(PentaToPsf[PsfNameIndex].name,
						  fo_line_def.SolPointSize, fo_line_def.SolSetSize,
						  Obdegree);
		}
    }
    else
		vj_para_flag = 1;
	return(0);
}

/***********************************************************************
 **  KMD47()    Page Insert Size. **
 ***********************************************************************/
int kmd47()
{
	int16 k1;

    k1 = foget();
    camy(k1,"kmd47 - camy.");
	return(0);
}

/***********************************************************************
 **  KMD49()    Exact Amount of extra lead, inhibits adjustment of **
 **		next extra lead by pagination. **
 ***********************************************************************/
int kmd49()
{
	return(0);
}

/***********************************************************************
 **  KMD50()  Change Set Size.   Expand or condense char width. **
 ***********************************************************************/
int kmd50()		
{
	int16 k1;

    k1 = foget();
    if (k1 != Holdss)
    {
		fo_line_def.SolSetSize = k1;
		create1psfont(PentaToPsf[PsfNameIndex].name,
					  fo_line_def.SolPointSize, fo_line_def.SolSetSize,
					  Obdegree);
    }
	return(0);
}

/***********************************************************************
 **  KMD51()  Slant on. **
 ***********************************************************************/
int kmd51()
{
    Obdegree = (fo_line_def.SolPointSize * 0.21255) /
		HorizontalBase; /* tan(12) = 0.21255 */
    create1psfont(PentaToPsf[PsfNameIndex].name, fo_line_def.SolPointSize,
				  fo_line_def.SolSetSize, Obdegree);
	return(0);
}

/***********************************************************************
 **  KMD52()  Slant off. **
 ***********************************************************************/
int kmd52()
{
    Obdegree = 0;
    create1psfont(PentaToPsf[PsfNameIndex].name, fo_line_def.SolPointSize,
				  fo_line_def.SolSetSize, Obdegree);
	return(0);
}

/***********************************************************************
 **  KMD54()  Horizontal Move. **
 ***********************************************************************/
int kmd54()
{
	int16 last_hmove, rounding_error;

    last_hmove = foget();
    Xpos += last_hmove;
	if ( MathStartFlag || !FlashOn)
	{							/* no rounding in math */
		if (cc_mask || active_trap_cc_mask)
			Kspace[Linesub+1] += ((float )last_hmove/(float )HorizontalBase);
		return(0);
	}
	else if ((rounding_error = RoundoffBucket / FontBase))
	{							/* adjust Xpos to true position */
		RoundoffBucket %= FontBase;
		UncompensatedRoundoff = RoundoffBucket;
		Xpos += rounding_error;
		wordsp -= rounding_error;
	}
	if (cc_mask || active_trap_cc_mask)
	{
		if ( AccentStartFlag)
		{
			stack_print();
			m_fprintf("currentpoint\n");
			Kspace[Linesub+1] += ((float )last_hmove/(float )HorizontalBase);
			accent_end_xpos = Xpos;
			accent_end_roundoff = RoundoffBucket;
		}	
		else if ( AccentEndFlag)
		{
			AccentEndFlag = 0;
			stack_print();
			m_fprintf("M\n");	/* move to currentpoint */
			RoundoffBucket = accent_end_roundoff;
		}
		else
			Kspace[Linesub+1] += ((float )last_hmove/(float )HorizontalBase);
	}
	else
	{
			AccentStartFlag = 0;
			AccentEndFlag = 0;
	}
	return(0);
}

/***********************************************************************
 **  KMD55()  Set Base Line. **
 ***********************************************************************/
int kmd55()
{
    jbl = foget();				/* + moves down,  - moves up. */
    fo_line_def.SolBaseLine = jbl; /* Replace ldef[] value */
	do_pskmd('M', "set base-line"); /* Move to there (obeys jbl).*/
	return(0);
}

/***********************************************************************
 **  KMD57()  Bold on.     -- NOT SUPPORTED -- **
 ***********************************************************************/
int kmd57()
{
	return(0);
}

/***********************************************************************
 **  KMD58()  Bold slant on.     -- NOT SUPPORTED --**
 ***********************************************************************/
int kmd58()
{
	return(0);
}	

/***********************************************************************
 **  KMD65()   Define Underscore. **
 ***********************************************************************/
int kmd65()
{
	int kk, ii;

	uscolr[1] = -2;
	uscolr[2] = -2;
    kk = (int)foget();			/* kk= Num of args to follow. */
    for(ii=1; ii<=2; ii++)	/* 2 = num us's to define. */
    {
		usoff[ii] = foget();	/* Offset from base.*/
		uswei[ii] = foget();	/* Underscore weight */
    }
    for(ii=1; ii<=2; ii++)	/* 2 = num us's to define. */
	{
		uscolr[ii] = foget();
		usshade[ii] = foget();
		ustrapcolr[ii] = foget();
		ustrapshade[ii] = foget();
		ustrapweight[ii] = foget();
	}
	return(0);
}

/***********************************************************************
 **  KMD66()   Begin  Underscore. **
 ***********************************************************************/
int kmd66()
{
    usw++;						/* In Underscore Switch. */
    if(usw > 1)
    {
		foget();
		return(0);
    }
	do_pskmd('M',"kmd66 begin under");
    utype = (int)foget();		/* Get Type of underscore. */
    us_beg_x1 = Xpos;
    us_beg_x2 = Xpos;
	us_min_mru = ((int)fo_line_def.SolSetSize * underscore_min) / 100;
	us_start_Ypos[1] = Ypos;
	us_start_Ypos[2] = Ypos;
	return(0);
}
/***********************************************************************
 **  SET_UNDER_SCORE()   Process  Underscore. **
 ***********************************************************************/
void set_under_score(void)
{
	int cc_mask_hold, trap_color_hold, trap_shade_hold;
	uint32 trap_cc_mask_hold, save_cc_mask;
	int input_color;
	int16 trap_stroke_width_hold;
	float save_fx, save_fx2, save_fy, save_fy3;
	float fx2_temp = Xpos;
	
    if(!utype)
		return;
	cc_mask_hold = cc_mask;
	trap_color_hold = ActiveTrapColor;
	trap_shade_hold = ActiveTrapShade;
	trap_cc_mask_hold = active_trap_cc_mask;
	trap_stroke_width_hold = ActiveTrapStrokeWidth;
    if (utype & 1)
    {
		Fx = us_beg_x1;
		Fx2 = fx2_temp;
		Rwei = uswei[1] / ((float) HorizontalBase * Jrule);
		Fy = us_start_Ypos[1] + usoff[1];
		Fy3 = Fy + (Rwei * VerticalBase);
		if ( (Fx2 > Fx) && ( (Fx2 - Fx) > us_min_mru ) )
		{
			save_fx = Fx;
			save_fx2 = Fx2;
			save_fy = Fy;
			save_fy3 = Fy3;
			if (!in_overflow)
			{					/* process rule colors and traps */
				color_func(uscolr[1], usshade[1]);
				input_color = ustrapcolr[1];
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
					{			/* set active_trap_cc_mask */
						if ( spot_pass == 1)
							active_trap_cc_mask = 1 & Plates;
						else
						{
							active_trap_cc_mask = 0;
							set_pass_2_color(input_color, &active_trap_cc_mask,
											 2);
						}
						ActiveTrapShade = ustrapshade[1];
						ActiveTrapStrokeWidth = ustrapweight[1];
					}
					if ( !active_trap_cc_mask)
						ActiveTrapColor = -1; /* no trap if clr undefined
												   for pass */
					else
					{
						Ofx = -4000; /* force Move command out */
						do_pskmd('M',"trap underscore move"); /* flush buffer*/
						Fx = save_fx;
						Fx2 = save_fx2;
						Fy = save_fy;
						Fy3 = save_fy3;
					}
					if ( (spot_pass == 1) && KeyOutputType ) 
						set_pass_2_color(input_color, &cc_hit, 2); /*as need*/
					cc_mask = save_cc_mask;
				}
			}
			do_pskmd('L',"set_under_1");
			Fx = save_fx;
			Fx2 = save_fx2;
			Fy = save_fy;
			Fy3 = save_fy3;
			color_func(ActiveTrapColor, ActiveTrapShade);
			trap_a_rule(0);
		}
    }
    if (utype & 2)
    {
		Fx = us_beg_x2;
		Fx2 = fx2_temp;
		Rwei = uswei[2] / ((float) HorizontalBase * Jrule);
		Fy = us_start_Ypos[2] + usoff[2];
		Fy3 = Fy + (Rwei * VerticalBase);
		if ( (Fx2 > Fx) && ( (Fx2 - Fx) > us_min_mru ) )
		{
			save_fx = Fx;
			save_fx2 = Fx2;
			save_fy = Fy;
			save_fy3 = Fy3;
			if (!in_overflow)
			{					/* process rule colors and traps */
				color_func(uscolr[2], usshade[2]);
				input_color = ustrapcolr[2];
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
					{			/* set active_trap_cc_mask */
						if ( spot_pass == 1)
							active_trap_cc_mask = 1 & Plates;
						else
						{
							active_trap_cc_mask = 0;
							set_pass_2_color(input_color, &active_trap_cc_mask,
											 2);
						}
						ActiveTrapShade = ustrapshade[2];
						ActiveTrapStrokeWidth = ustrapweight[2];
					}
					if ( !active_trap_cc_mask)
						ActiveTrapColor = -1; /* no trap if clr undefined
												   for pass */
					else
					{
						Ofx = -4000; /* force Move command out */
						do_pskmd('M',"trap underscore move"); /* flush buffer*/
						Fx = save_fx;
						Fx2 = save_fx2;
						Fy = save_fy;
						Fy3 = save_fy3;
					}
					if ( (spot_pass == 1) && KeyOutputType ) 
						set_pass_2_color(input_color, &cc_hit, 2); /*as need*/
					cc_mask = save_cc_mask;
				}
			}
			do_pskmd('L',"set_under_2");
			Fx = save_fx;
			Fx2 = save_fx2;
			Fy = save_fy;
			Fy3 = save_fy3;
			color_func(ActiveTrapColor, ActiveTrapShade);
			trap_a_rule(0);
		}
    }
	Xpos = fx2_temp;
    us_beg_x1 = Xpos;
	us_beg_x2 = Xpos;
	us_start_Ypos[1] = Ypos;
	us_start_Ypos[2] = Ypos;
	Ofx = -4000;				/* force Move command out */
	do_pskmd('M',"endunder");

	/**** fix for BUG594q
	 ****replaced fo_line_def.SolFgColor with Current_FgColor and 
	 ****replaced fo_line_def.SolFgShadeColor with Current_FgShade
	 ****in function color_func to reset color to previous line
	 ****color before underscore 
	 ****/
	color_func(Current_FgColor, Current_FgShade);
	cc_mask = cc_mask_hold;
	ActiveTrapColor = trap_color_hold;
	ActiveTrapShade = trap_shade_hold;
	active_trap_cc_mask = trap_cc_mask_hold;
	ActiveTrapStrokeWidth = trap_stroke_width_hold;
	return;
}								/* end set under score */

/***********************************************************************
 **  KMD67()   End    Underscore. **
 ***********************************************************************/
int kmd67()
{
    if (FlashOn)
		set_under_score();
    usw = 0;
    utype = 0;
    us_beg_x1 = 0;
	us_beg_x2 = 0;
    Rwei = 0;
	return(0);
}
/***********************************************************************
 **  KMD69()   Rule fill. **
 ***********************************************************************/
int kmd69()
{
	float save_fx, save_fx2, save_fy, save_fy3;
    int16 lgut, rgut;
    int16 len, t_style, rwi;
	int16 ii;
	int16 il;						/* left indent for rule. */
	int16 ir;						/* right indent for rule. */
	int16 k1 = 0;
	int jj = 0;
	int kk = 0;
    
    rwi = foget();				/* Index for rule weight */
    len = foget();				/* rule length */
    t_style = foget();			/* Tab indications: 1=into left gutter
								   2=into right gutter 3=into both.  */
    il= foget();				/* indent left on rule */
    ir = foget();				/* indent right on rule */
    if(len == 0)				/* Called for full-measure rule? */
    {
		k1 = fill_space/fill_count;
		len = k1 - il - ir;		/* Length = space available */
								/*   less the two indents. */
		fill_space -= k1;
		fill_count--;
    }
    lgut = rgut = 0;			/* Left and right gutter adjustments. */
    if (t_style && Ktabmode)	/* If gutter extension requested and
								   if we're in a tab line: */
    {
		if ((t_style & 1) && !il) /* 1 or 3: Extend into left gutter. */
		{
			for (ii=ktabindex-1 ; ii>=0 ;
				 ii--)			/* Loop from column before rule start
								   backward through 1st column: */
			{
				jj = ktab[ii].ttype; /* Get column type */
				kk = ktab[ii].twid; /* and column width. */
				if (jj) break;	/* Exit on text or v-rul column. */
				lgut += kk;		/* It's an empty gutter: Rule
								   extension = gutter width. */
			}
			if (jj > 3)			/* Ended on a vert-rule gutter:	*/
			{
				if (ktab[ii].quad == 1) /* If v-rule is quadded left, */
					lgut += kk;	/* add in full rule-gutr wid. */
				else			/* But if v-rule is quadded center
								   (q-right impossible to left
								   of text column), then */
					lgut += kk/2; /* add in 1/2 rule-gutter wid. */
			}
			else if (jj)		/* Ended on a text column:	*/
				lgut /= 2;		/*   rule exten is 1/2 width of
									 gutter to its right. */
		}						/* End "if ((tstyle&1) && !il)"	*/
		if ((t_style & 2) && !ir) /* 2 or 3: Extend into right gutter. */
		{
			ii = ktabindex + 1;	/* Index of next column after [ru-]. */
			if (instrad)
				ii = stab[sindex].send; /* In straddle, index of next
										   col after all straddled cols.*/
			for ( ; ii<ntabs ; ii++) /* Loop from that column forward	*/
			{					/* through end of table. */
				jj = ktab[ii].ttype; /* Get column type */
				kk = ktab[ii].twid; /*   and column width. */
				if (jj)
					break;		/* Exit on text or v-rule colm. */
				rgut += kk;		/* It's an empty gutter: Rule
								   extension = gutter width. */
			}
			if (jj > 3)			/* Ended on a vert-rule gutter:	*/
			{
				if (ktab[ii].quad == 2) /* If v-rule is quadded right, */
					rgut += kk;	/* add in full rule-gutr wid. */
				else			/* But if v-rule is quadded center
								   (q-left impossible to right
								   of text column), then */
					rgut += kk/2; /* add in 1/2 rule-gutter wid. */
			}
			else if (jj)		/* Ended on a text column:	*/
				rgut /= 2;		/* Rule extension is 1/2 width
								   of gutter to its left. */
		}						/* End "if ((tstyle&2) && !ir)"	*/
    }							/* End of all gutter-length calculations. */
    Rwei = ((float)rwi / Jrule) * VerticalBase;
    Fx = Xpos + il - lgut;	/* Note: il or lgut or both WILL BE 0.*/
    Xpos += il + len + rgut;	/* Note: ir or rgut     -- " -- */
    Fx2 = Fx + len + lgut + rgut;
	Fy3 = Ypos + jbl;			/* add baseline shift */
    Fy = Fy3 - Rwei;
	if (FlashOn)
	{
		save_fx = Fx;
		save_fx2 = Fx2;
		save_fy = Fy;
		save_fy3 = Fy3;
		do_pskmd('L',"kmd69 rule");
		Fx = save_fx;
		Fx2 = save_fx2;
		Fy = save_fy;
		Fy3 = save_fy3;
		trap_a_rule(0);
	}
	Xpos += ir;					/* add right indent */
	Ofx = -4000;				/* force Move command out */
	do_pskmd('M', "endkmdru");
	return(0);
}

/***********************************************************************
 **  KMD74()   Accent. **
 ***********************************************************************/
int kmd74()
{
    return (do_OF(74));
}

/***********************************************************************
 **  KMD77();   GRAPHIC  INCLUDE **
 ***********************************************************************/
int kmd77()
{
	if ( !DontPaintFlag)
		PaintOnPage |= BlackPaint_cc_mask;
    graphic(1);					/* Arg val 1: From [mg-] cmd */
	return(0);
}

/***********************************************************************
 **  KMD83()   One-char font change which identifies character by
 **            its postscript name rather than by font loc.
 ***********************************************************************/
int kmd83()
{
    return (do_OF(83));
}

/***********************************************************************/
void trap_a_rule(int strikethru_flag)
{
	int jj;
	uint32 save_cc_mask, loop_temp,	BlackPaint_cc_mask_sav;
	uint32 save_active_trap_cc_mask;
	float trap_stroke;

	if ( !active_trap_cc_mask)
		return;
	if ( !DontPaintFlag)
		PaintOnPage |= active_trap_cc_mask;
	if ( !strikethru_flag)
	{
		trap_stroke = (float )ActiveTrapStrokeWidth / HorizontalBase;
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		Fy3 = (Imageheight - Fy3) / VerticalBase;
	}
	else
		trap_stroke = ActiveTrapStrokeWidth / ((float )HorizontalBase * Jrule);
	save_cc_mask = cc_mask;
	save_active_trap_cc_mask = active_trap_cc_mask;
	cc_mask = active_trap_cc_mask;
	active_trap_cc_mask = 0;
    digi_print(Fx);
    digi_print(Fy);
	for (jj=0; jj<MAX_CLRS; jj++)
	{
		cc_mask = ( loop_temp = (1 << jj) ) & save_active_trap_cc_mask;
		if(loop_temp > save_active_trap_cc_mask)
			break;
		if (!cc_mask)
			continue;
		BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
		vid_color (ActiveTrapColor, ActiveTrapShade, jj+1);
		BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
	}							/* end for(jj=0;jj<MAX_CLRS;jj++) */
	cc_mask = save_active_trap_cc_mask;
    m_fprintf("M GS %4.2f setlinewidth\n", trap_stroke);
	drule_sub(1);
	m_fprintf("GR\n");
	cc_mask = save_cc_mask;
	active_trap_cc_mask = save_active_trap_cc_mask;
}
/********** EOF **********/
