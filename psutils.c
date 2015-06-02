#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "p_lib.h"
#include "psjob.h"
#include "rel_data.h"
#include "frame.h"

extern void trap_a_rule(int strikethru_flag);

extern int rule_break_type;
extern int ExitError;
extern int dashwid;
extern int gapwid;
extern int odashwid;
extern int ogapwid;
extern int bg_color_flag;
extern DRAW_POINT_X_Y *lp;
extern int np_from_ele;
extern char holdfontname[64];
extern int16 output_ss;
extern int LineStats;	/* 1+=Print line stats after line #s.  0=Don't.
							5+=Print the stats using val as the Pt-size.  */
extern int lnumbers;
extern int suppresslp;
extern int err;
extern int16 fobuf[256];
extern int overflow_depth;
extern int prologue_output_flag;
extern char PrinterName[];
extern int U_PrtCharDepth;
extern int U_PrtCharWid;
extern float anchor_shades[21];
extern int start_dxy_flag;
extern int last_y;
extern int16 fobuf[256];
extern int IncludeScreens;	/* 1 = set screens; 0 = use device screens */
extern int LPI; /* 0 = use colortable color frequency */
extern char rtype;
extern double StrikeThruRuleAngle;
extern int output_crop_mark_flag;
extern int StrikeThruLayer;
extern int StrikeThruTrapWt;
extern int StrikeThruTop;
extern int StrikeThruBot;

extern int16 Current_FgColor;	
extern int16 Current_FgShade;
extern int blend_num;
extern int piece_ctr;
extern int total_depth;
float percentage;

int segment_type;				/* Used for breaking bf */
int OffT;						/* Non-imageable top-of-pg distance, in pnts */
int OffB;						/* Non-imageable bot-of-pg distance, in pnts */
int OffL;						/* Non-imageable left-mrgn distance, in pnts */
int OffR;						/* Non-imageable rite-mrgn distance, in pnts */
int PageW;						/* Width of physical page in points. Includes
								   left and right margins (OffL & OffR). */
int PageH;						/* Depth of physical page in points. Includes
								   top and bottom margins (OffT & OffB). */
float ScaleFactorX;				/* Optional scale, applied to page horizontal*/
float ScaleFactorY;				/* Optional scale, applied to page vertical */
int y_max;
float Ofx;
float Ofy;
int Imageheight;				/* Vert page size in Vert Base, uses Orient */
int16 header_offset;			/* raise galley slug by this many points */
int ReportSize;
int NoEofCtrlD;					/* 1 means inhibit EOF control D */
int PsfNameIndex;				/* Penta font number (index to PentaToPsf)*/
int PsfCodeIndex;				/* Index to the PsfCode translation table */
int PageW_key_flag;				/* if non-zero, keyword PageW in input */
int PageD_key_flag;				/* if non-zero, keyword PageD in input */
int Pofft_key_flag;				/* if non-zero, keyword Pofft in input */
int Poffb_key_flag;				/* if non-zero, keyword Poffb in input */
int Poffl_key_flag;				/* if non-zero, keyword Poffl in input */
int Poffr_key_flag;				/* if non-zero, keyword Poffr in input */


int StartSaveText = 0;
float key_scale_x;
float key_scale_y;
int Orient;						/* 0=Portrait   90=Landscape */
int DidPage;					/* have we issued the %%PAGE: comment */
uint16 last_lnum;
int16 jbl;						/* Current bslin shift for Moveto */

float cblend_start[4];				/* Used for continued blends to hold new start color */
float cblend_delta[4];				/* Used for continued blends to hold delta color values */
float cshade_delta;
float cshade_start;
float cshade_end;




/***********************************************************************
 **  FOGET()   Return a int16 integer from .FO file or -1 if EOF **
 **            fowrd : current word pointer   (Start from 0) **
 **            forec : current record pointer (Start from 0) **
 **            fobuf : array[0..255] of int16, for 1 FO record **
 ***********************************************************************/
int16 foget(void)
{
    if (fowrd >= 255)
    {
		fowrd = -1;
		forec++;   /* increment record pointer to new current record */
		if ((err = p_read((char *)fobuf, sizeof(fobuf), 1, Fofd,
						  forec,BS16)) == 0)
		{
#ifdef TRACE
			p_info(PI_TRACE, "READ ERROR in .FO file %s record %d.\n",
				   FoName,forec);
			if(debugger_trace)
				m_fprintf("\n%%Read error in FO file %s record %d.\n",
						  FoName,forec);
#endif
			stack_print();
			stop("Unexpected end of FO file"," ",0);
		}
    }
    fowrd++;					/* increment word pointer, (count from 0) */
    return(fobuf[fowrd]);
}

/***************************************************************************
 **  FOREAD(recnum)     Input record "recnum" of the FO file into fobuf  **
 **************************************************************************/
void foread(int16 recnum)
{
#ifdef TRACE
    if(text_trace)
		p_info(PI_TRACE, "FOREAD(%d)\n",recnum);
#endif
    if(recnum == 0)
    {
		if ( !Orient)
			Imageheight = (PageH - OffT - OffB) * VerticalBase;
		else
			Imageheight = (PageW - OffL - OffR) * VerticalBase;
    }							/* end if(recnum==0) */
    else
    {
		if( (err = p_read((char *) fobuf, sizeof(fobuf), 1, Fofd,
						  recnum, BS16))== 0)
			stop(FoName," <- FO file is empty or past end-of-file.",0);
    }
    forec = recnum;
    fowrd = -1;
}								/* end function */

/***********************************************************************
 **  GETPGSIZE() To input Postscript Page Dimensions & other data.
 **  Parses the PAGESIZE section of psftable, of following structure:

 **  											   		Line mandatory
 **    Line contents									or optional?
 **    ---------------------------------				-------------
 **    PAGESIZE AND DEFAULTS SECTION:					Mandatory
 **    PageH=792			       		  				Optional
 **    PageW=612			       		  				Optional
 **    POffT=10						  					All lines
 **    POffB=10						   					before /END
 **    POffL=25						   					are optional.
 **    POffR=25						      				    |
 **    ACOFF=.175 - not used		      				    |
 **    SCALE=.996264					      			    |
 **    SCALEX=.996264	   <--	Or these 2 (1 or       	    |
 **    SCALEY=.9948  	   <--	   both).		      	    |
 **    OPSTX=0    					      				    |
 **    HDRUP=0                                              |
 **    %				       		      				    V
 **    %  SEND   CMYK   DEVICE  DEVICE   HEADER		NO EOF
 **    %  PORT  SUPPORT PageW   PageH     TYPE      CTRL D
 **    %				       			      **
 **    SENDP=1    0      612      792       0       1
 **    SENDP=2    1      864     1440       1       0
 **    %					       		      **
 **    /END PAGESIZE SECTION.				  Mandatory   **
 **								      **
 ***********************************************************************/
void getpgsize(void)
{
	int temp, i;
	float scale_factor = 0;
	float key_scale = 0;

/* Default values, if not supplied in psftable: */
	key_scale_x = 0;
	key_scale_y = 0;
	NoEofCtrlD = 1;				/* No CTRL D at EOF unless negated */
	if ( !PageW_key_flag) 
		PageH = PsfDefault -> pageh; /* no def by key word */ 
	if ( !PageD_key_flag)
		PageW = PsfDefault -> pagew; /* no def by key word */
	if ( !Pofft_key_flag)
		OffT  = PsfDefault -> pofft; /* no def by key word */
	if ( !Poffb_key_flag)
		OffB  = PsfDefault -> poffb; /* no def by key word */
	if ( !Poffl_key_flag)
		OffL  = PsfDefault -> poffl; /* no def by key word */
	if ( !Poffr_key_flag)
		OffR  = PsfDefault -> poffr; /* no def by key word */
	key_scale = PsfDefault -> scale;
	key_scale_x = PsfDefault -> scalex;
	key_scale_y = PsfDefault -> scaley;
	if (header_offset == -32760) /* Pts to raise slug line */
		header_offset = PsfDefault -> hdrup; /* HDRUP- slug line extra lead */
	for (i=0; i<MaxSendp; i++)
	{
		if( !strcmp(PrinterName, PsfSendp[i].name) )
		{						/* got a match */
			if (CMYK_Allowed < 0)
				CMYK_Allowed = PsfSendp[i].cmyk; /* no def by key word */
			temp = PsfSendp[i].pagew;
			if ( PageW_key_flag)
				temp = 0;		/* already defined by key word */
			if (temp)
				PageW  = temp;
			temp = PsfSendp[i].pageh;
			if ( PageD_key_flag)
				temp = 0;		/* already defined by key word */
			if (temp)
				PageH  = temp;
			if (setpage_allowed < 0)
				setpage_allowed = PsfSendp[i].htype;
			NoEofCtrlD = PsfSendp[i].noeof;
			break;
		}
	}							/* end for(i=0;i<MaxSendp;i++) */
	ScaleFactorX = 0;
	ScaleFactorY = 0;
	if ( (KeyScale == -1) && FileType )
	{							/* size layout to fit psftable size */
		KeyScaleX = 0;
		KeyScaleY = 0;
	}		
	else if (KeyScale == -2)
		KeyScale = 0;
	if ( key_scale || key_scale_x || key_scale_y)
	{
		if ( !key_scale)
		{
			if ( key_scale_x)
				key_scale = key_scale_x;
			else
				key_scale = key_scale_x;
		}
		if ( !key_scale_x)
			key_scale_x = key_scale;
		if ( !key_scale_y)
			key_scale_y = key_scale;
	}
	else
		key_scale = key_scale_x = key_scale_y = 1.0;
	if ( ( KeyScale > 0) || ( KeyScaleX > 0) || ( KeyScaleY > 0) )
	{							/* setup from keyword - use psftable */
		if (KeyScale)
			scale_factor = ((float)KeyScale * key_scale) / 100.;
		else
			scale_factor = 1.0;
		if (KeyScaleX > 0)
			ScaleFactorX = ((float)KeyScaleX * key_scale_x) / 100.;
		else
			ScaleFactorX = scale_factor;
		if (KeyScaleY > 0)
			ScaleFactorY = ((float)KeyScaleY * key_scale_y) / 100.;
		else
			ScaleFactorY = scale_factor;
	}							/* end setup from keyword */
	else if ( key_scale || key_scale_x  || key_scale_y )
	{							/* set up from psftable */
		scale_factor = key_scale;
		ScaleFactorX = key_scale_x;
		ScaleFactorY = key_scale_y;
	}
	if (ScaleFactorX == 0)
		ScaleFactorX = scale_factor; /* Default val */
	if (ScaleFactorY == 0)
		ScaleFactorY = scale_factor; /* Default val */
	if (ScaleFactorX + ScaleFactorY) /* If either given, ensure
										neither non-0. */
	{
		if (ScaleFactorX == 0)
			ScaleFactorX = 1.0;
		if (ScaleFactorY == 0)
			ScaleFactorY = 1.0;
	}
	if (setpage_allowed < 0)
		setpage_allowed = 0;	/* not set, use default */
	if (PageW <= 0)
		PageW = 612;			/* Page width = 8 1/2 " */
	if (PageH <= 0)
		PageH = 792;			/* Page height = 11" */
	if (OffT < 0)				/* if not defined */
		OffT  = 0;				/* Top offset = 2 picas 1 pnt. */
	if (OffB < 0)				/* if not defined */
		OffB  = 0;				/* Bot offset =   -- " --  */
	if (OffL < 0)				/* if not defined */
		OffL  = 0;				/* Left offset = 2 picas 1 pnt. */
	if (OffR < 0)				/* if not defined */
		OffR  = 0;				/* Right offset = 2 picas 1 pnt. */
	if (CMYK_Allowed < 0)
		CMYK_Allowed = 0;		/* no CMYK output is default */
	if (header_offset == -32760) /* Pts to raise slug line */
		header_offset = 0;		/* Points to raise the slug line */
	if (LYPrintFlag)
	{
		U_PrtCharDepth = (PageH - OffT - OffB) / 10; /* 10 pts per line */
		if (U_PrtCharDepth > 199)
			U_PrtCharDepth = 199;
		U_PrtCharWid = (PageW - OffL - OffR) / 6; /* 6 pts per char at 10 pt */
		ReportSize = 10;
		if (U_PrtCharWid > 131)
			U_PrtCharWid = 131;
		else if (U_PrtCharWid < 100)
		{
			ReportSize = (10 * U_PrtCharWid) / 100;
			if (ReportSize < 3)
				ReportSize = 3;
			U_PrtCharWid = 100;
			U_PrtCharDepth = (PageH - OffT - OffB) / ReportSize;
			if (U_PrtCharDepth > 199)
				U_PrtCharDepth = 199;
		}
	}							/* end if(LYPrintFlag) */
}								/* end function */

/***********************************************************************
 **  GETPROLOG()   Reads Start-of-TP section from PSFTABLE, transfers  **
 **		it to the TP file, uninterpreted.		      **
 **  Parses the START-OF-TEXT section of psftable, structured:	      **
 **  								      **
 **  							Line mandatory**
 **    Line contents					or optional?  **
 **    ---------------------------------       		------------- **
 **    START-OF-TEXT SECTION:         			  Mandatory   **
 **    Line 1 of user's PostScript header coding.	  Optional    **
 **    Line 2 of user's PostScript header coding.	  Optional    **
 **      (etc.)							      **
 **    Last line of user's PostScript header coding.	  Optional    **
 **    //END START-OF-TEXT SECTION			  Mandatory   **
 **  								      **
 ***********************************************************************/
void getprolog(void)
{
	int string_length;

#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "getprolog - Line count = %d.\n",PsfPrologueLineCount);
#endif
	if ( PsfPrologue && PsfPrologueLineCount)
	{
		string_length = sizeof(PsfPrologue);
		while (string_length >= m_fprintf_size)
		{
			m_fprintf_buff = p_remalloc(m_fprintf_buff, m_fprintf_size, 
										m_fprintf_size + 4096);
			m_fprintf_size += 4096;
		}
		m_fprintf("%s\n", PsfPrologue); /* write the prologue to the TP. */
	}
}

/***********************************************************************
 **  GETFTAB()   To input Postscript Font and its Character Codes. **
 ***********************************************************************/
void getftab(void)
{								/* look in psftable for font and code */
	int found = 0;

	if ( fo_line_def.SolFont <= MaxPentaFont)
	{							/* font number not too high */
		if ( PentaToPsf[fo_line_def.SolFont].name != NULL)
			if ( PentaToPsf[fo_line_def.SolFont].name[0])
				found = 1;
	}
	if ( !found)
	{
		error(": In psftable unable to find font "," ",fo_line_def.SolFont);
		if ( !PsfNameIndex)
			stop("Default font not defined, program exit.", " ", 0);
		return;
	}
	PsfNameIndex = fo_line_def.SolFont;
	PsfCodeIndex = PentaToPsf[fo_line_def.SolFont].code;
}								/* end getftab */

/***********************************************************************
 **  ERROR(s1,s2)   Print error messages **
 ***********************************************************************/
void error(char *s1, char *s2, int eror)
{
    p_info(PI_ELOG, "PostPrint ERROR ");
	if (fo_line_def.LineNum > 0)
	{
		if (*FoName > ' ')		/* printing pages  */
			p_info(PI_ELOG, "(file %s, line %d) ",
				FoName, fo_line_def.LineNum);
		else					/* printing galley  */
			p_info(PI_ELOG, "(file %s, line %d) ",
				JobName, fo_line_def.LineNum);
	}
    if(s1[0] != ' ')
		p_info(PI_ELOG, "%s",s1);
    if(eror > 0)
		p_info(PI_ELOG, " %d",eror);
    if(s2[0] != ' ')
		p_info(PI_ELOG, " %s",s2);
    p_info(PI_ELOG, ".\n");
	ExitError = 1;
}

/***********************************************************************
 **  DO_PSKMD('X') **
 ***********************************************************************/
void do_pskmd(char action, char *cmd_str)
{
	float save_fx, save_fx2, save_fy, save_fy3;
	float density, x_start, max_steps;
	float del_c, del_m, del_y, del_k, max_delta;
	float save_BfBgShadeStart, save_BlendEndShade;
    uint32 cc_mask_hold, cc_trap_mask_hold, BlackPaint_cc_mask_sav;
	uint32 full_cc_mask;
    uint32 loop_temp, plate;
    int ii, jj, steps;
	int save_NewBgColor, save_NewBlendColor;
	int save_odashwid, save_ogapwid;
	struct clr_lst *clr_start, *clr_end;
	struct clr_lst blend_start, blend_end, blend_params;

	int control_loop_flag;		/* -1 = blend, start; > 0 = anchor points */

#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "DO_PS %c %s, cc_mask= %lo, Fx= %.3f, Fy= %.3f\n",
			   action, cmd_str, cc_mask, (float)Xpos / HorizontalBase,
			   (((float)Imageheight - Ypos - jbl) / VerticalBase));
#endif
	if ( !DidPage && !(LYPrintFlag &3 ) )
	{
		if ( (FileType || (spot_pass != 1)) && (action != 'Z') )
		{						/* no header yet in page or galley pass 2 */
			cc_mask_hold = cc_mask;
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit & Plates;
			if ( !CurrentLineBcEc)
				beg_PAGE();
			cc_mask = cc_mask_hold;
		}
		else if( (action == 'M') && (y_max == 0) )
		{						/* galley only, pass 1, ignore 'M' */
			Ofx = Fx = (float)Xpos / HorizontalBase;
			Ofy = Fy = ((float)Imageheight - Ypos - jbl) / VerticalBase;
			stack_print();
			was_text = 0;
			return;
		}
		else if (action != 'Z')
		{						/* no header yet, misc action */
			cc_mask_hold = cc_mask;
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			if ( !CurrentLineBcEc)
				beg_PAGE();
			cc_mask = cc_mask_hold;
		}
		if (pdflinkAtSOL && pdflinkctr	/* If PDF link area was needed at start of line,
											and this is text from .fo file,  */
			&& (!FileType || TYPE_OF_FRAME(CurrentFrame) != PL_TEXT))
		{								/*  and this is first char or
											cmd of line, mark start of area:  */
			char pdfstring[48];

			pdflinkAtSOL = 0;			/* Do once only, per line.  */
			m_fprintf("currentpoint\n");
			memset(pdfstring,0,sizeof(pdfstring));
			sprintf(pdfstring,"/lSy%d exch def /lSx%d exch def\n",pdflinkctr,pdflinkctr);
			m_fprintf(pdfstring);
		}
	}							/* end if(!DidPage) */
    stack_print();
    switch(action)
    {
      case 'B':					/* Blend */
		clr_start = 0;
		clr_end = 0;
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		Fy3 = (Imageheight - Fy3) / VerticalBase;
		save_fx = Fx;
		save_fy = Fy;
		save_fx2 = Fx2;
		save_fy3 = Fy3;
		save_NewBgColor = NewBgColor;
		save_NewBlendColor = NewBlendColor;
		save_BfBgShadeStart = BfBgShadeStart;
		save_BlendEndShade = BlendEndShade;
#ifdef TRACE
		if(debugger_trace)
		{
			p_info(PI_TRACE, "%c blend, bg= %d, Fx=%.2f, Fy=%.2f, Fx2=%.2f, Fy3=%.2f\n",
				   action, BgColor, Fx, Fy, Fx2, Fy3);
			p_info(PI_TRACE, "	ang= %d, clr= %d\n", BlendAngle, BlendEndColor);
		}
#endif
		if ( !cc_mask || ((BgColor < 0) || (BlendEndColor < 0)) )
			break;
		cc_mask_hold = cc_mask;
		for (ii=0; ii<MAX_CLRS-1; ii++)
		{						/* clip path */
			cc_mask = ( loop_temp = (1 << ii) ) & cc_mask_hold;
			if(loop_temp > cc_mask_hold)
				break;
			if (!cc_mask)
				continue;
			NewBgColor = save_NewBgColor;
			NewBlendColor = save_NewBlendColor;
/* Put out clip path */
			Fx = BfClipFx / HorizontalBase;
			Fx2 = BfClipFx2 / HorizontalBase;
			Fy = (Imageheight - BfClipFy) / VerticalBase;
			Fy3 = (Imageheight - BfClipFy3) / VerticalBase;
			m_fprintf("%5.2f %5.2f M GS ", Fx, Fy);
			if ( !CornerRadiusBfFlag)
				dbox ( 1, 3);
			else
				dbox ( 1, CornerRadiusBfFlag);
/* Translate to center of frame and rotate through angle */
			if (BlendAngle >= 0)
			{
				m_fprintf("%.3f %.3f translate %.2f rotate\n",
						  (save_fx + save_fx2) / 2, (save_fy + save_fy3) / 2,
						  DegAngle);
/* Get blend box corners. Translate origin to lower left corner of blend box */
				Fx = 0;
				Fx2 = ((save_fx2 - save_fx) * CosAngle) +
					((save_fy - save_fy3) * SinAngle);
				Fy3 = 0;
				Fy = ((save_fx2 - save_fx) * SinAngle) +
					((save_fy - save_fy3) * CosAngle);
				m_fprintf("%.3f %.3f translate\n", (Fx - Fx2) / 2, (Fy3 - Fy) / 2);
			}
			else
/* move to center of frame */
				m_fprintf("%.3f %.3f moveto\n", (save_fx + save_fx2) / 2, (save_fy + save_fy3) /2);
/* Figure out the colors */
#ifdef TRACE
			if (color_trace)
				p_info(PI_TRACE, "starting do_pskmd blend(%d to %d), plate %d\n",
					   NewBgColor, NewBlendColor, ii + 1);
#endif
			if ( (NewBgColor < 0) || (NewBlendColor < 0))
			{
				m_fprintf("GR\n");	/* get rid of frame clip path */
				continue;
			}
			if ( !DontPaintFlag)
				PaintOnPage |= cc_mask;
			control_loop_flag = -1;
			x_start = 0;
			BfBgShadeStart = save_BfBgShadeStart;
			BlendEndShade = save_BlendEndShade;
			while ( control_loop_flag)
			{
				if ( (control_loop_flag == -1) && AnchorCount)
					Fx2 = (float )anchor_amounts[0] /
						HorizontalBase; /* colors ok, depth is not */
				else if (AnchorCount)
				{				/* cycle colors, get new start & end depths */
					NewBgColor = NewBlendColor;
					BfBgShadeStart = BlendEndShade;
					NewBlendColor =
						anchor_colors[AnchorCount - control_loop_flag];
					BlendEndShade = 
						anchor_shades[AnchorCount - control_loop_flag];
					Fx = Fx2;
					Fx2 = Fx +
						(float)(anchor_amounts[AnchorCount -
											   control_loop_flag + 1] /
								HorizontalBase);
					if (control_loop_flag == 1)
						Fx2 = (float)anchor_amounts[20] /
							HorizontalBase;	/* last segment */
				}
				if ( !KeyOutputType)
					plate = 0;	/* use plate 0 color for composite */
				else			
					plate = ii + 1; /* normal color */
				clr_start = find_color(NewBgColor, plate);
				clr_end = find_color(NewBlendColor, plate);
				if( !clr_start && !clr_end)
				{
#ifdef TRACE
					if ( color_trace)
						p_info(PI_TRACE, "start(%d), end color (%d),  plate (%lo) not defined\n",
							   NewBgColor, NewBlendColor, plate);
#endif
					if ( control_loop_flag == -1)
					{
						if ( AnchorCount)
							control_loop_flag = AnchorCount;
						else
							control_loop_flag = 0;
					}
					else
						control_loop_flag -= 1;
					x_start = Fx2; /* start the next piece here */
					continue;
				}				/* end if(!clr_start&!clr_end) */
				if ( !clr_start)
				{				/* start color not defined, use 0 */
					memset( (char *)&blend_start.color, 0,
							sizeof(struct clr_lst) );
					blend_start.density = 1.0;
					memcpy( (char *)&blend_params.color,
							(char *)clr_end, sizeof(struct clr_lst) );
				}
				else			/* starting color defined */
				{
					memcpy( (char *)&blend_start.color, (char *)clr_start,
							sizeof(struct clr_lst) );
					blend_start.density = 1.0 - 
						((1.0 - blend_start.density) * BfBgShadeStart);
					for (jj=0; jj<4; jj++)
						blend_start.cmyk[jj] *= BfBgShadeStart;
					memcpy( (char *)&blend_params.color, (char *)clr_start,
							sizeof(struct clr_lst) );
					blend_params.density = 1.0 - 
						((1.0 - blend_params.density) * BfBgShadeStart);
					for (jj=0; jj<4; jj++)
						blend_params.cmyk[jj] *= BfBgShadeStart;

				}
				if ( !clr_end)
				{
					memset( (char *)&blend_end.color, 0,
							sizeof(struct clr_lst) );
					blend_end.density = 1.0;
				}
				else			/* ending color defined */
				{

					memcpy( (char *)&blend_end.color, (char *)clr_end,
							sizeof(struct clr_lst) );
					blend_end.density = 
						1.0 - ((1.0 - blend_end.density) * BlendEndShade);
					for (jj=0; jj<4; jj++)
						blend_end.cmyk[jj] *= BlendEndShade;
				}
#ifdef TRACE
				if (color_trace)
				{
					p_info(PI_TRACE, "start color= %d, reverse= %d, freq= %d, cmyk_flag= %d\n",
						   blend_start.color, blend_start.reverse,
						   blend_start.freq, blend_start.cmyk_flag);
					p_info(PI_TRACE, "  c= %f, m= %f, y= %f, k= %f, dens= %f, angle= %f, func= %s\n",
						   blend_start.cmyk[0], blend_start.cmyk[1],
						   blend_start.cmyk[2], blend_start.cmyk[3],
						   blend_start.density, blend_start.angle,
						   blend_start.func);
					p_info(PI_TRACE, "%d %f %s setscreen  %5.2f setgray\n",
						   blend_start.freq,
						   blend_start.angle + (double)Orient,
						   blend_start.func, blend_start.density);
					p_info(PI_TRACE, "end color= %d, reverse= %d, freq= %d, cmyk_flag= %d\n",
						   blend_end.color, blend_end.reverse,
						   blend_end.freq, blend_end.cmyk_flag);
					p_info(PI_TRACE, "  c= %f, m= %f, y= %f, k= %f, dens= %f, angle= %f, func= %s\n",
						   blend_end.cmyk[0], blend_end.cmyk[1],
						   blend_end.cmyk[2], blend_end.cmyk[3],
						   blend_end.density, blend_end.angle,
						   blend_end.func);
					p_info(PI_TRACE, "%d %f %s setscreen  %5.2f setgray\n",
						   blend_end.freq,
						   blend_end.angle + (double)Orient,
						   blend_end.func, blend_end.density);
				}
#endif

/* Set the screen from the params structure */
				if (IncludeScreens)
				{
					if (LPI != 0)
						blend_params.freq = LPI;
	
					m_fprintf("%d %5.2f {%s} setscreen\nGS\n",
							  blend_params.freq,
							  blend_params.angle + (double)Orient,
							  blend_params.func);
				}
				else
					m_fprintf("GS\n");
/* If black and white, figure out the steps and output the blend */
				if ( !CMYK_Allowed || !blend_params.cmyk_flag)
				{				/* only B & W */
					density = blend_start.density - blend_end.density;
					if (density < 0)
						steps = (density * -255) + 1;
					else
						steps = (density * 255) + 1;
					if ( (float)steps > (max_steps = ((((Fx2 - x_start) *
											1.1) * blend_params.freq) / 72.)) )
						steps = (int )max_steps;
#ifdef TRACE
					if (color_trace)
						p_info(PI_TRACE, " B&W setgray diff= %.3f, steps= %d\n",
							   density, steps);
#endif
                    if (BlendAngle >= 0 )
                        m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %d BWBLEND\n",
                              (int)max_steps, Fx2, Fy, blend_end.density,
                              blend_start.density, x_start, steps);
                    else /* oval and circle blend */
                        {
                        if (BlendAngle == -4 ) /* oval blend */
 
                        {
                            m_fprintf("%5.2f setgray fill\n", blend_start.density);
                            m_fprintf("%.3f %.3f moveto\n", (save_fx + save_fx2) / 2, (
save_fy + save_fy3) /2);
                            m_fprintf("%5.2f %5.2f CALCBLENDSCALE\n",
                                (save_fx2 - save_fx), (save_fy - save_fy3));
                        }
                        m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %d RADBWBLEND\n",
                              (int)max_steps, (save_fx2 - save_fx), (save_fy - save_fy3), blend_end.density,
                              blend_start.density, x_start, steps);
                        }

					x_start = Fx2;
				}				/* end if(!CMYK_Allowed) */

/* Figure out the steps and output the color blend now */
				else if (blend_start.cmyk_flag && blend_end.cmyk_flag &&
						 !KeyOutputType && CMYK_Allowed)
				{				/* set the specified color */
					del_c = blend_start.cmyk[0] - blend_end.cmyk[0];
					if (del_c < 0)
						del_c *= -1.;
					del_m = blend_start.cmyk[1] - blend_end.cmyk[1];
					if (del_m < 0)
						del_m *= -1.;
					del_y = blend_start.cmyk[2] - blend_end.cmyk[2];
					if (del_y < 0)
						del_y *= -1.;
					del_k = blend_start.cmyk[3] - blend_end.cmyk[3];
					if (del_k < 0)
						del_k *= -1.;
					max_delta = 0;
					if (del_c > del_m)
						max_delta = del_c;
					else
						max_delta = del_m;
					if (del_y > max_delta)
						max_delta = del_y;
					if (del_k > max_delta)
						max_delta = del_k;

					steps = (max_delta * 256) + 1; /* number of steps */
					if ( (float)steps > (max_steps = ((((Fx2 - x_start) *
											1.1) * blend_params.freq) / 72.)) )
						steps = (int )max_steps;

#ifdef TRACE
					if (color_trace)
						p_info(PI_TRACE, " Color diff= %.3f, steps= %d\n",
							   max_delta, steps);
#endif
/* Add blending Pantone to White */
/* Add blending Pantone to custom color */
/* Add blending Pantone to another Pantone */

					if (BlendAngle >= 0) {
/* If both start and end colors are spot, and custom color names are the same,  */
/*  output blend as spot color.  */ 
					if ( (strcmp(blend_start.type,"spot") == 0) &&
						 (strcmp(blend_end.type,"spot") == 0) &&
						 (strcmp(blend_start.color_name,blend_end.color_name) == 0) ) 
					{
if (total_depth != 1)
{

/* Calculate new percentage for each piece */
percentage = (Fx2 * 10 / total_depth);

if (piece_ctr == 1)
{
	cshade_delta = (BfBgShadeStart - BlendEndShade) * 100 ;
	cshade_start = BfBgShadeStart;

}
/* Calculate new end blend shade */

	cshade_end = cshade_start - ((percentage * cshade_delta) / 100);


						/* DSV: This area needs work because we only use the first four color numbers in PP */

						if (BlendEndShade == 0)
						{
						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %d PCLRBLEND\n",
							(int)max_steps, Fx2, Fy,
							blend_start.cmyk[0] / BfBgShadeStart, blend_start.cmyk[1] / BfBgShadeStart,
							blend_start.cmyk[2] / BfBgShadeStart, blend_start.cmyk[3] / BfBgShadeStart,
							blend_start.color_name, 1 - cshade_start,
							blend_end.cmyk[0] / 1, blend_end.cmyk[1] / 1,
							blend_end.cmyk[2] / 1, blend_end.cmyk[3] / 1, 
							blend_end.color_name, 1 - cshade_end,
							x_start, steps);

						}
						else
						{

							if (BfBgShadeStart == 0)
							{
/* Swap end to start because start is already zeros. */

						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %d PCLRBLEND\n",
							(int)max_steps, Fx2, Fy,
							blend_end.cmyk[0] , blend_end.cmyk[1] ,
							blend_end.cmyk[2] , blend_end.cmyk[3] ,
							blend_start.color_name, 1 - cshade_start,
							blend_end.cmyk[0] / BlendEndShade, blend_end.cmyk[1] / BlendEndShade,
							blend_end.cmyk[2] / BlendEndShade, blend_end.cmyk[3] / BlendEndShade, 
							blend_end.color_name, 1 + cshade_end,
							x_start, steps);

							}
							else
							{


						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %d PCLRBLEND\n",
							(int)max_steps, Fx2, Fy,
							blend_start.cmyk[0] / BfBgShadeStart, blend_start.cmyk[1] / BfBgShadeStart,
							blend_start.cmyk[2] / BfBgShadeStart, blend_start.cmyk[3] / BfBgShadeStart,
							blend_start.color_name, 1 - cshade_start,
							blend_end.cmyk[0] / BlendEndShade, blend_end.cmyk[1] / BlendEndShade,
							blend_end.cmyk[2] / BlendEndShade, blend_end.cmyk[3] / BlendEndShade, 
							blend_end.color_name, 1 - cshade_end,
							x_start, steps);
							}

						}
/* Reset New start Shade */
	cshade_start = cshade_end;
}
else
{
/* Normal Blends */
						if (BlendEndShade == 0)
						{
						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %d PCLRBLEND\n",
							(int)max_steps, Fx2, Fy,
							blend_start.cmyk[0] / BfBgShadeStart, blend_start.cmyk[1] / BfBgShadeStart,
							blend_start.cmyk[2] / BfBgShadeStart, blend_start.cmyk[3] / BfBgShadeStart,
							blend_start.color_name, 1 - BfBgShadeStart,
							blend_end.cmyk[0] / 1, blend_end.cmyk[1] / 1,
							blend_end.cmyk[2] / 1, blend_end.cmyk[3] / 1, 
							blend_end.color_name, 1 - BlendEndShade,
							x_start, steps);

						}
						else
						{

							if (BfBgShadeStart == 0)
							{
/* Swap end to start because start is already zeros. */

						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %d PCLRBLEND\n",
							(int)max_steps, Fx2, Fy,
							blend_end.cmyk[0] , blend_end.cmyk[1] ,
							blend_end.cmyk[2] , blend_end.cmyk[3] ,
							blend_start.color_name, 1 - BfBgShadeStart,
							blend_end.cmyk[0] / BlendEndShade, blend_end.cmyk[1] / BlendEndShade,
							blend_end.cmyk[2] / BlendEndShade, blend_end.cmyk[3] / BlendEndShade, 
							blend_end.color_name, 1 - BlendEndShade,
							x_start, steps);

							}
							else
							{


						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %5.2f %5.2f %5.2f %s %5.2f %5.2f %d PCLRBLEND\n",
							(int)max_steps, Fx2, Fy,
							blend_start.cmyk[0] / BfBgShadeStart, blend_start.cmyk[1] / BfBgShadeStart,
							blend_start.cmyk[2] / BfBgShadeStart, blend_start.cmyk[3] / BfBgShadeStart,
							blend_start.color_name, 1 - BfBgShadeStart,
							blend_end.cmyk[0] / BlendEndShade, blend_end.cmyk[1] / BlendEndShade,
							blend_end.cmyk[2] / BlendEndShade, blend_end.cmyk[3] / BlendEndShade, 
							blend_end.color_name, 1 - BlendEndShade,
							x_start, steps);
							}
						}
}
					} /* End if Spot */
					else
					{
if (total_depth != 1) 
{


/* Calculate new percentage for each piece */
percentage = (Fx2 * 10 / total_depth);

/* load up cblend_delta and cblend_start arrays only for first piece */
if (piece_ctr == 1) 
{
	cblend_delta[0] = (blend_start.cmyk[0] - blend_end.cmyk[0]) * 100;
	cblend_delta[1] = (blend_start.cmyk[1] - blend_end.cmyk[1]) * 100;
	cblend_delta[2] = (blend_start.cmyk[2] - blend_end.cmyk[2]) * 100;
	cblend_delta[3] = (blend_start.cmyk[3] - blend_end.cmyk[3]) * 100;

	cblend_start[0] = blend_start.cmyk[0];
	cblend_start[1] = blend_start.cmyk[1];
	cblend_start[2] = blend_start.cmyk[2];
	cblend_start[3] = blend_start.cmyk[3];
}

/* Calculate new end color */
	blend_end.cmyk[0] = cblend_start[0] - ((percentage * cblend_delta[0]) / 100);
	blend_end.cmyk[1] = cblend_start[1] - ((percentage * cblend_delta[1]) / 100);
	blend_end.cmyk[2] = cblend_start[2] - ((percentage * cblend_delta[2]) / 100);
	blend_end.cmyk[3] = cblend_start[3] - ((percentage * cblend_delta[3]) / 100);

/* Output Piece */
	m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %d CLRBLEND\n",
        (int)max_steps, Fx2, Fy, blend_end.cmyk[0], blend_end.cmyk[1],
        blend_end.cmyk[2], blend_end.cmyk[3],
        cblend_start[0], cblend_start[1],
        cblend_start[2], cblend_start[3],
        x_start, steps);

/* Now change start color to end color for next piece */
	cblend_start[0] = blend_end.cmyk[0];
	cblend_start[1] = blend_end.cmyk[1];
	cblend_start[2] = blend_end.cmyk[2];
	cblend_start[3] = blend_end.cmyk[3];




} /* End if total_depth != 1 */
else
{
/* Normal blend output */
						m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %d CLRBLEND\n",
                              (int)max_steps, Fx2, Fy, blend_end.cmyk[0], blend_end.cmyk[1],
                              blend_end.cmyk[2], blend_end.cmyk[3],
                              blend_start.cmyk[0], blend_start.cmyk[1],
                              blend_start.cmyk[2], blend_start.cmyk[3],
                              x_start, steps);
}


					}
					}
                    else /* oval or circle blend */
                    {
                    if (BlendAngle == -4) /* oval blend */
                    {
                        m_fprintf("%5.2f %5.2f %5.2f %5.2f setcmykcolor fill\n",
                            blend_start.cmyk[0], blend_start.cmyk[1],
                            blend_start.cmyk[2], blend_start.cmyk[3]);
                        m_fprintf("%.3f %.3f moveto\n", (save_fx + save_fx2) / 2, (save_fy + save_fy3) /2);
                        m_fprintf("%5.2f %5.2f CALCBLENDSCALE\n", (save_fx2 - save_fx),
							(save_fy - save_fy3));
                    }
                    m_fprintf("%d SMOOTH\n%5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %d RADCLRBLEND\n",
                              (int)max_steps, (save_fx2 - save_fx), (save_fy - save_fy3), blend_end.cmyk[0], blend_end.cmyk[1],
                              blend_end.cmyk[2], blend_end.cmyk[3],
                              blend_start.cmyk[0], blend_start.cmyk[1],
                              blend_start.cmyk[2], blend_start.cmyk[3],
                              x_start, steps);
                    }
					x_start = Fx2;
				}				/* end else if(blend.cmyk_flag....) */
				m_fprintf("GR\n"); /* get rid of blend paths */
				if ( control_loop_flag == -1)
				{
					if ( AnchorCount)
						control_loop_flag = AnchorCount;
					else
						control_loop_flag = 0;
				}
				else
					control_loop_flag -= 1;
			}					/* end while(control_loop_flag) */
			m_fprintf("GR\n");	/* get rid of frame clip path */
			Fx = save_fx;
			Fy = save_fy;
			Fx2 = save_fx2;
			Fy3 = save_fy3;
		}						/* end for (ii=0;ii<MAX_CLRS-1;ii++) */
		NewBgColor = save_NewBgColor;
		NewBlendColor = save_NewBlendColor;
		cc_mask = cc_mask_hold;
		break;

	  case 'E':					/* Edit trace delete marker */
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		Fy3 = (Imageheight - Fy3) / VerticalBase;
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "'E' marker Fx=%.2f, Fy=%.2f, Fx2=%.2f, Fy3=%.2f\n",
				   Fx,Fy,Fx2,Fy3);
#endif
		cc_mask_hold = active_trap_cc_mask;
		active_trap_cc_mask = 0;
		m_fprintf("GS newpath ");
		digi_print(Fx);
		digi_print(Fy);
		m_fprintf("M ");
		digi_print(Fx2);
		digi_print(Fy);
		m_fprintf("L ");
		digi_print((Fx + Fx2) / 2);
		digi_print(Fy3);
		m_fprintf("L ");
		digi_print(Fx);
		digi_print(Fy);
		m_fprintf("L ");
		m_fprintf(" fill\nGR\n");	/* Fill the triangle. */
		if ( !DontPaintFlag)
			PaintOnPage |= BlackPaint_cc_mask;
		active_trap_cc_mask = cc_mask_hold;
		break;

	  case 'F':					/* Frame Rule */
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		Fy3 = (Imageheight - Fy3) / VerticalBase;
#ifdef TRACE 
		if(debugger_trace) 
			p_info(PI_TRACE, "'F' rule Fx=%.2f, Fy=%.2f, Fx2=%.2f, Fy3=%.2f\n",
				   Fx,Fy,Fx2,Fy3);
#endif
		if ( ( !cc_mask && !active_trap_cc_mask) || (BgColor < 0) )
			break;
		cc_mask_hold = cc_mask;
		for (ii=0; ii<MAX_CLRS-1; ii++)
		{
			int temp_plate = 1;

			cc_mask = ( loop_temp = (1 << ii) ) & cc_mask_hold;
			if(loop_temp > cc_mask_hold)
				break;
			if (!cc_mask)
				continue;
			if ( !output_crop_mark_flag)
				temp_plate = ii + 1;
			if (find_color(BgColor, temp_plate) )
			{					/* found the color structure */
				digi_print(Fx);
				digi_print(Fy);
				m_fprintf("M\n");
				BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
				BlackPaint_cc_mask = 0;
				vid_color(BgColor, BgShade,ii+1); /*  background area */
										/* fix with GR below */
				m_fprintf("GS\n"); /* save  */
				if ( !DontPaintFlag)
					PaintOnPage |= BlackPaint_cc_mask;
				BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
				dfrule(); /* do a box from globals */
				m_fprintf("GR\n"); /* restore */
			}
		}						/* end for (ii=0;ii<MAX_CLRS-1;ii++) */
		cc_mask = cc_mask_hold;
		break;
		
	  case 'L':					/* Rule */
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		Fy3 = (Imageheight - Fy3) / VerticalBase;
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "'L' rule Fx=%.2f, Fy=%.2f, Fx2=%.2f, Fy3=%.2f\n",
				   Fx,Fy,Fx2,Fy3);
#endif
		cc_mask_hold = active_trap_cc_mask;
		active_trap_cc_mask = 0;
		drule();				/* do a rule from Fx, Fy to Fx2,Fy3 */
		if ( !DontPaintFlag)
			PaintOnPage |= BlackPaint_cc_mask;
		active_trap_cc_mask = cc_mask_hold;
		break;
		
      case 'P':					/* Showpage */
		if ( !CurrentLineBcEc)
		{
			end_PAGE();
			Ypos = 0;
			overflow_depth = 0;
			y_max = 0;
		}
		break;
		
      case 'R':					/* Relative moveto */
		Fx /= HorizontalBase;
		Fy /= VerticalBase;
		digi_print(Fx);
		digi_print(Fy);
		m_fprintf(" RM\n");
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "%c rmoveto, Fx=%.2f, Fy=%.2f\n",action,Fx,Fy);
#endif
		break;

      case 'S':					/* Strikethru Rule */
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		save_fx = Fx;
		save_fy = Fy;
		save_fx2 = Fx2;
		save_fy3 = Fy3;			/* Fy3 is rule wt in points */
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "'L' rule Fx=%.2f, Fy=%.2f, Fx2=%.2f, Fy3=%.2f\n",
				   Fx,Fy,Fx2,Fy3);
#endif
		cc_mask_hold = cc_mask;
		cc_trap_mask_hold = active_trap_cc_mask;
		cc_mask |= active_trap_cc_mask;
		full_cc_mask = cc_mask;
		active_trap_cc_mask = 0;
		m_fprintf("GS newpath \n");
		digi_print( Fx);
		digi_print( Fy);
		m_fprintf("translate 0 0 M\n"); /* move to start of rule */

		if ( StrikeThruRuleAngle && ( StrikeThruTop != StrikeThruBot ))
		{						/* rotate about rule start */
			m_fprintf(" %5.2f rotate\n", (float)StrikeThruRuleAngle);
#ifdef TRACE
			if(debugger_trace)
				p_info(PI_TRACE, "strikethru rule, angle= %f.\n", 
					   (float)StrikeThruRuleAngle);
#endif
		}
		cc_mask = cc_mask_hold;
		digi_print( Fx2);
		digi_print(0);
		m_fprintf("L ");
		digi_print(Fy3);		/* rule wt */
		m_fprintf("setlinewidth ");
		m_fprintf(" [] 0 setdash stroke\n"); /* Stroke the path. */
		if ( !DontPaintFlag)
			PaintOnPage |= BlackPaint_cc_mask;
		Fx = 0;
		Fx2 = save_fx2;
		Fy = Fy3 / 2;			/* top of rule */
		Fy3 = -Fy;				/* bottom of rule */
		active_trap_cc_mask = cc_trap_mask_hold;
		cc_mask = cc_trap_mask_hold;
		color_func(ActiveTrapColor, ActiveTrapShade);
		save_odashwid = odashwid;
		save_ogapwid = ogapwid;
		odashwid = 0;
		ogapwid = 0;
		/* Do not set Trap if trap rule weight is 0 */
		if (StrikeThruTrapWt > 0) 
		{
			trap_a_rule(1);
		}
		odashwid = save_odashwid;
		ogapwid = save_ogapwid;
		cc_mask = full_cc_mask;
		m_fprintf("GR\n");
		/* Return current color */
	    color_func(Current_FgColor,Current_FgShade);

		break;

      case  'T':				/* Translate */
		digi_print(Fx);
		digi_print(Fy);
		m_fprintf("T \n");
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "%c translate, Fx=%.2f, Fy=%.2f\n",action,Fx,Fy);
#endif
		break;

      case 'V':					/* Video Box */
		Fx /= HorizontalBase;
		Fx2 /= HorizontalBase;
		Fy = (Imageheight - Fy) / VerticalBase;
		Fy3 = (Imageheight - Fy3) / VerticalBase;
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "%c box, bg= %d, Fx=%.2f, Fy=%.2f, Fx2=%.2f, Fy3=%.2f\n",
				   action,BgColor,Fx,Fy,Fx2,Fy3);
#endif
		if ( ( !cc_mask && !active_trap_cc_mask) || (BgColor < 0) )
			break;
		cc_mask_hold = cc_mask;
		for (ii=0; ii<MAX_CLRS-1; ii++)
		{
			cc_mask = ( loop_temp = (1 << ii) ) & cc_mask_hold;
			if(loop_temp > cc_mask_hold)
				break;
			if (!cc_mask)
				continue;
			if (find_color(BgColor, ii+1) )
			{					/* found the color structure */
				digi_print(Fx);
				digi_print(Fy);
				m_fprintf("M\n");
				BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
				BlackPaint_cc_mask = 0;
				vid_color(BgColor, BgShade,ii+1); /*  background area */
										/* fix with GR below */
				m_fprintf("GS\n"); /* save  */
				if ( !DontPaintFlag)
					PaintOnPage |= BlackPaint_cc_mask;
				BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
				dbox(0, CornerRadiusBfFlag); /* do a box from globals */
				m_fprintf("GR\n"); /* restore */
			}
		}						/* end for (ii=0;ii<MAX_CLRS-1;ii++) */
		cc_mask = cc_mask_hold;
		break;
		
      case  'X':                /* rotate n degrees about x,y */
		if (Rotate < 0)
			suppresslp=1;
		else
			suppresslp=0;
       	digi_print(Rotate/10); /* Rotate =n, Fx,Fy = x,y */
        digi_print(Fx);
        digi_print(Fy);
        m_fprintf("RXY\n");
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "%c RXY, deg= %.2f, Fx=%.2f, Fy=%.2f\n",
				   action, Rotate, Fx, Fy);
#endif
        break;

      case 'Z':					/* End-of-file showpage */
		CurrentLineBcEc = 0;
		end_JOB();				/* output trailer */
        if ( !NoEofCtrlD)
			m_fprintf("\004");	/* add on the EOF byte */
		Ypos = 0;
		overflow_depth = 0;
		break;

	  case 'm':					/* move, do page start if needed */
      default:					/* Move to Xpos,Ypos */
		if (StrikeThruLayer == 1 && !StartSaveText)
		{

			/* Save Current Point */
    		m_fprintf("currentpoint /SRY exch def /SRX exch def\n");
			/* Start save of text */
			m_fprintf("/doText {\n");
			StartSaveText = 1;
		}

		Fx = (float)Xpos / HorizontalBase; /* Convert Xpos to points */
		Fy = ((float)Imageheight - Ypos -
			  jbl) / VerticalBase; /* Convert Ypos relative to LL corner,
										jump base-line, convert to points. */
		if (Ofx == Fx &&		/* Eliminate redundant moveto's: */
			Ofy == Fy &&		/* If X- and Y-pos same as last moveto */
			Ypos &&				/* and not at top of page, and */
			!was_text)			/* there's been no text since last, */
			return;				/* there's no need to move. Exit. */
		if (y_max < Ypos)
			y_max = Ypos;		/* Keep running deepest-point value. */
		if (lnumbers & 1)
			put_lnum();			/* Put out line number if requested */
		was_text = 0;			/* Clear "there has been text" flag. */
		digi_print(Fx);
		digi_print(Fy);

		m_fprintf("M\n");
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "%c M, Fx=%.2f, Fy=%.2f\n",action,Fx,Fy);
#endif
		break;
    }							/* End Switch. */
    Ofx = Fx;
    Ofy = Fy;
    Fx=0;
    Fy=0;
}

/*************************************************************************
 ** DBOX - draw box with Fx,Fy to Fx2, Fy3  or use element path points **
*************************************************************************/
void dbox(int fill_or_stroke_flag, int bf_radius_flag)
{
	int i;
	DRAW_POINT_X_Y *lp_temp;
	float x_radius, y_radius;

	lp_temp = lp;
	if (bg_color_flag || fill_or_stroke_flag || CircleFlag || bf_radius_flag)
	{							/* output the frame points for bg */
		m_fprintf("newpath ");
		if ( CircleFlag)
		{			
			digi_print(x_radius = ((Fx2 - Fx)/2)); /* x radius */
			digi_print(y_radius = ((Fy - Fy3)/2)); /* y radius */
			digi_print(Fx + x_radius); /* x center */
			digi_print(Fy3 + y_radius); /* y center */
			m_fprintf("ES ");
		}
		else if ( RoundCornerRadiusTL)
		{
			if ( bf_radius_flag < 3)
			{
				digi_print(Fx + RoundCornerRadiusTL);
				digi_print(Fy);
				m_fprintf("M "); /* move to the UL start */

				if ( rule_break_type == 0 || rule_break_type > 32 )
				{
				dbox_sub( Fx2, Fy, Fx2, Fy3); /* upper right */
				dbox_sub( Fx2, Fy3, Fx, Fy3); /* lower right */
				dbox_sub( Fx, Fy3, Fx, Fy); /* lower left */
				dbox_sub( Fx, Fy, Fx + RoundCornerRadiusTL, Fy); /* upr left */
				}
				else
				{
				/* determine segment type */
				if (rule_break_type < 32 && rule_break_type > 16)
				{
					segment_type = 3; /* end segment */
				}
				if (rule_break_type < 16 && rule_break_type > 8)
				{
					segment_type = 2; /* mid segment */
				}
				if (rule_break_type < 8 && rule_break_type > 0)
				{
					segment_type = 1; /* start segment */
				}

				switch (segment_type)
				{

					case 1: /* start segment  - always flat bottom */
						dbox_sub( Fx2, Fy, Fx2, Fy3); /* upper right */
						digi_print(Fx2);
						digi_print(Fy3);
						m_fprintf("lineto\n");
						digi_print(Fx);
						digi_print(Fy3);
						m_fprintf("lineto\n");
						dbox_sub( Fx, Fy, Fx + RoundCornerRadiusTL, Fy);
						break;

					case 2:	/* mid segment */
							/* flat top for bfF and bfX */

						if (rule_break_type > 9)
						{
							/* flat top and bottom */

							digi_print(Fx);
							digi_print(Fy);
							m_fprintf("M ");
							digi_print(Fx2);
							digi_print(Fy);
							m_fprintf("L ");
							digi_print(Fx2);
							digi_print(Fy3);
							m_fprintf("L ");
							digi_print(Fx);
							digi_print(Fy3);
							m_fprintf("L ");

						}
						else
						{
							/* flat bottom only */
						dbox_sub( Fx2, Fy, Fx2, Fy3); /* upper right */
						digi_print(Fx2);
						digi_print(Fy3);
						m_fprintf("lineto\n");
						digi_print(Fx);
						digi_print(Fy3);
						m_fprintf("lineto\n");
						dbox_sub( Fx, Fy, Fx + RoundCornerRadiusTL, Fy);
						}
						break;

					case 3:	/* end segment */
							/* flat top only for bfF  and bfX*/

						if (rule_break_type > 17)
						{
							digi_print(Fx);
							digi_print(Fy);
							m_fprintf("M\n ");
							digi_print(Fx2);
							digi_print(Fy);
							m_fprintf("L\n ");
							dbox_sub( Fx2, Fy3, Fx, Fy3); /* lower right */
							dbox_sub( Fx, Fy3, Fx, Fy); /* lower left */
						}  
						else
						{
							dbox_sub( Fx2, Fy, Fx2, Fy3); /* upper right */
							dbox_sub( Fx2, Fy3, Fx, Fy3); /* lower right */
							dbox_sub( Fx, Fy3, Fx, Fy); /* lower left */
							dbox_sub( Fx, Fy, Fx + RoundCornerRadiusTL, Fy);
						}
						break;
				} /* End Switch */	

			}




			}					/* end if(bf_radius_flag < 3) */
			else
			{
				switch (bf_radius_flag)
				{
				  case 3:		/* clip top */
					digi_print(Fx);
					digi_print(Fy);
					m_fprintf("M "); /* move to the UL start */
					digi_print(Fx2);
					digi_print(Fy);
					m_fprintf("L "); /* move across top to UR */
					dbox_sub( Fx2, Fy3, Fx, Fy3);
					dbox_sub( Fx, Fy3, Fx, Fy); /* upper left */
					break;

				  case 4:		/* clip bottom */
					digi_print(Fx);
					digi_print(Fy3);
					m_fprintf("M "); /* move to the LL start */
					digi_print(Fx2);
					digi_print(Fy3);
					m_fprintf("L "); /* move across bottom to LR */
					dbox_sub( Fx2, Fy, Fx, Fy);
					dbox_sub( Fx, Fy, Fx, Fy3); /* lower left */
					break;

				  case 5:		/* clip both */
					digi_print(Fx);
					digi_print(Fy);
					m_fprintf("M ");
					digi_print(Fx2);
					digi_print(Fy);
					m_fprintf("L ");
					digi_print(Fx2);
					digi_print(Fy3);
					m_fprintf("L ");
					digi_print(Fx);
					digi_print(Fy3);
					m_fprintf("L ");
					break;
				}				/* end switch(bf_radius_flag) */
			}					/* end else bs_radius_flag > 2 */
		}						/* end else if(RoundCornerRadiusTL) */
		else if ( bf_radius_flag >= 3)
		{
			digi_print(Fx);
			digi_print(Fy);
			m_fprintf("M ");
			digi_print(Fx2);
			digi_print(Fy);
			m_fprintf("L ");
			digi_print(Fx2);
			digi_print(Fy3);
			m_fprintf("L ");
			digi_print(Fx);
			digi_print(Fy3);
			m_fprintf("L ");
		}
		else
		{
			for (i=0; i<np_from_ele; i++)
			{
				output_ele_xy((float)lp_temp -> x, (float)lp_temp ->y);
				if (i)
					m_fprintf("L "); /* move to the next spot corner */
				else
					m_fprintf("M "); /* move to the UL corner */
				if ( (i & 3) == 3 )
					m_fprintf("\n"); /* up to 4 moves on a line */
				lp_temp++;
			}
		}

		if ( fill_or_stroke_flag && (bf_radius_flag < 2))
		{
			if ( odashwid == 0 && ogapwid == 0) /* If outline dashes, set them here */
				m_fprintf("[] 0 setdash\n");
			else
				m_fprintf("[%d %d] 0 setdash\n", odashwid, ogapwid);
			m_fprintf(" closepath stroke\n"); /* Stroke the path. */
		}
		else if (bf_radius_flag >= 2)
			m_fprintf(" closepath clip\n"); /* Clip the path. */
		else
			if (Fx != Fx2 && Fy != Fy3)
			m_fprintf(" closepath fill\n"); /* Fill the container. */
	}
	else
		drule_sub(0);
}

/*---------------------------------------------------------------*/

void dbox_sub(float x1, float y1, float x2, float y2)
{
			digi_print( x1);
			digi_print( y1);
			digi_print( x2);
			digi_print( y2);
			m_fprintf("%5.2f arcto 4 {pop} repeat\n", RoundCornerRadiusTL);
}

/*************************************************************************
 ** DRULE - draw a rule(box) around Fx,Fy to Fx2, Fy3 **
 *************************************************************************/
void drule(void)
{
    digi_print(Fx);
    digi_print(Fy);
    m_fprintf("M \n");
    m_fprintf("GS ");
	drule_sub(0);
	m_fprintf("GR\n");
}

/*--------------------------------------*/

void drule_sub(int stroke_flag)
{
	m_fprintf("newpath ");
    digi_print(Fx);
    digi_print(Fy);
    m_fprintf("M ");
    digi_print(Fx2);
    digi_print(Fy);
    m_fprintf("L ");
    digi_print(Fx2);
    digi_print(Fy3);
    m_fprintf("L ");
    digi_print(Fx);
    digi_print(Fy3);
    m_fprintf("L ");

	if ( stroke_flag)
	{
		if (odashwid == 0 && ogapwid == 0)
			m_fprintf("[] 0 setdash\n");
		else
			m_fprintf("[%d %d] 0 setdash\n", odashwid, ogapwid);
		m_fprintf(" closepath stroke\n"); /* Stroke the path. */
	}
	else
		if (Fx != Fx2 && Fy != Fy3)
		m_fprintf(" closepath fill\n");	/* Fill the box. */
}

/*************************************************************************
 ** DFRULE - draw frame rules                                          **
 *************************************************************************/
void dfrule(void)
{
		
    digi_print(Fx);
    digi_print(Fy);
    m_fprintf("M \n");
    m_fprintf("GS ");


	if ((dashwid == 0) && (gapwid == 0))
		m_fprintf("[] 0 setdash\n");
	else
		m_fprintf("[%d %d] 0 setdash\n", dashwid, gapwid);


	if (rtype == 'h') /* Draw horizontal frame rule */
	{
		digi_print((Fy - Fy3));
		m_fprintf("setlinewidth\n");
		m_fprintf("newpath ");
		digi_print(Fx);
		digi_print(Fy + ((Fy3 - Fy)/2));
		m_fprintf("M \n");
		digi_print(Fx2);
		digi_print(Fy + ((Fy3 - Fy)/2));
		m_fprintf("L \n");
		m_fprintf("stroke\n");
	}
	else if (rtype == 'v') /* Draw vertical frame rule */
	{
		digi_print(Fx2 - Fx);
		m_fprintf("setlinewidth\n");
		m_fprintf("newpath ");
		digi_print(Fx + ((Fx2 - Fx)/2));
		digi_print(Fy);
		m_fprintf("M \n");
		digi_print(Fx + ((Fx2 - Fx)/2));
		digi_print(Fy3);
		m_fprintf("L \n");
		m_fprintf("stroke\n");
	}
	m_fprintf("GR\n");
}

/***********************************************************************
 **  put_lnum();   For outputting line numbers.                        **
 ***********************************************************************/
void put_lnum(void)
{
	float x_pos=-25;
	float test_point_size=10.;
	float layout_pagew;
	float layout_paged;
	float layout_xoffset;
	float layout_yoffset;
	float digval = 5.56;	/* Width of a Helvetica digit at 10-point.  */
	int CurrentFrame, bandval, LinePS;
	WYSIWYG *wn;
	char jjunk[10];

	if (suppresslp)
		return;

	LinePS = 10;		/* Assume output line#s in 10-point.  */
	if (FileType) {
		/* Pages - Text frames only */
	   CurrentFrame = PsCurRec -> frame_nbr;
	   wn = PsCurRec -> frame_wn;
		if (TYPE_OF_FRAME(CurrentFrame) != PL_FLOW)
			return;

           layout_pagew = (((float)lmt_off_to_abs(wn,X_REF,PAGE_WIDTH) ) / HorizontalBase);
           layout_paged = (((float)lmt_off_to_abs(wn,Y_REF,PAGE_DEPTH) ) / VerticalBase);
           layout_xoffset = (((float)lmt_off_to_abs(wn,X_REF,X_PG_ORIGIN) ) / HorizontalBase);
           layout_yoffset = (((float)lmt_off_to_abs(wn,Y_REF,Y_PG_ORIGIN) ) / VerticalBase);
		test_point_size = 7.;
		/* Fx is current X offset on page */
		if (Fx >= ( (layout_pagew + layout_xoffset) / 2) ) {
			x_pos = layout_pagew + layout_xoffset + 24;
		} else {
			if (layout_xoffset) 
				x_pos = 10;
			else
				x_pos = 5;
		}
	}
	else {
		/* Galley  - H&J stats optionally printed after line number   */
		test_point_size = 10.;
		if (LineStats) {
			x_pos = -65;	/* Room for line# and h&j stats  */
			if (LineStats > 4) { /* Arg may also carry PS of line#s.
									So set ptsz to that val,  */
				test_point_size = (float)LineStats;
								/* And vary digit-wid from 5.56:  */
				digval = (LineStats * 5.56) / 10;
			}
		}
		else
			x_pos = -25;	/* Room for line# only  */
		/* Right-justify line# up to 3 digits:  */
		x_pos += (3 - sprintf(jjunk, "%d", fo_line_def.LineNum)) * digval;
		if (fo_line_def.LineNum > 999)
			test_point_size = 8.;
		if (fo_line_def.LineNum > 9999)
			test_point_size = 7.; 
	}

    if((float)last_y  == Fy)
		return;
    if(last_lnum  == fo_line_def.LineNum)
		return;
    if(NoFont == 0)
		return;
    last_lnum = fo_line_def.LineNum;
    last_y = Fy;

    m_fprintf("%2.1f /Helvetica F ", test_point_size);

    digi_print(x_pos);
    digi_print(Fy);

    m_fprintf("M (%d) S ",fo_line_def.LineNum);
	if (LineStats) {
		if (fo_line_def.Bands > 0)
		{
			bandval = fo_line_def.BandSpace;
						/* If a just line & UnusedMeasure wasn't assigned to LS,  */
			if ((fo_line_def.Quad==5) && !(fo_line_def.MiscLineFlags&0x400))
				bandval += fo_line_def.UnusedMeasure;
			bandval /= fo_line_def.Bands;
		}
		else
			bandval = 0;
		/* At 10-pt, a Helvetica digit is 5.56 pts wide  */
		/* Right-justify bandwid up to 3 digits:  */
		x_pos = -40 + (3 - sprintf(jjunk, "%d", bandval)) * digval;
    	digi_print(x_pos);
    	digi_print(Fy);
		m_fprintf(" M (%d) S ", bandval);

		/* Right-justify LS value up to 2 digits:  */
		if ((fo_line_def.LsPerChar < 10) && !(fo_line_def.MiscLineFlags&0x400))
			x_pos = -19 + digval;
		else
			x_pos = -19;
    	digi_print(x_pos);
    	digi_print(Fy);
		if ((fo_line_def.MiscLineFlags&0x400) && fo_line_def.UnusedMeasure)
		{
			if (fo_line_def.UnusedMeasure > 0)
				m_fprintf(" M (%d+) S ", fo_line_def.LsPerChar);
			else
				m_fprintf(" M (%d-) S ", fo_line_def.LsPerChar);
		}
		else
			m_fprintf(" M (%d) S ", fo_line_def.LsPerChar);
	}

	not_lnum = 0;           /* Doing line number font. */
	put_font();
    not_lnum = 1;				/* Back to text font. */
    was_text = 1;
	ForceLineNbrFont = 0;
}

/**********************************************************************
 **	Open TP (PostScript) file using name built by parse_dp.      **
 ***********************************************************************/
int tp_open(void)
{
    uint32 cc_op;	/* mask of successful opens - compare to cc_mask */

	if ( !cc_mask)
		return(0);
#ifdef TRACE
    if (debugger_trace)
		p_info(PI_TRACE, "in tp_open, .TP file = %s, cc_mask= %lo\n",tpname, cc_mask);
#endif
	if ( MultiPagesUp && MultiPagesOddEvenFlag && start_dxy_flag)
		cc_op = m_fopen(tpname, "a+",cc_mask); /* append if odd page */
	else
		cc_op = m_fopen(tpname, "w+",cc_mask); /* Delete, then open for r/w */
	if ( !prologue_output_flag)
	{
		beg_JOB(tpname);
		prologue_output_flag++;
	}
    if (cc_op != cc_mask)
    {
		error (": Unable to open output file ",tpname,0);
		return (1);				/* (error return) */
    }
	if ( FileType || (spot_pass != 1))
	{							/* galley pass 2 and pages */
		if ( !LYPrintFlag)
		{
			DidPage = 0;
			beg_PAGE();			/* do start of pages */
		}
	}
	return(0);
}
/**********************************************************************/
/*********end file************/
