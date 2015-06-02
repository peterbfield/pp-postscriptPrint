#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "signal.h"
#include "p_lib.h"
#include "frame.h"
#include "rel_data.h"
#include "psjob.h"
#include "lpm.h"
#include "list_control.h"

extern int odashwid;
extern int ogapwid;

static void change_rotation(int16 degrees, float x_center, float y_center);
static void frame_color(int box_flag);
static void frame_overlap(void);
static void reopen_tp_if_overflow(void);
static void xyset(void);
static void xyrulebox(void);
static int do_HandJ(char *filename, int fo_or_lfo_flag);
static void sig_usr1(int);
static void sig_alrm(int);
static void scan_pp(char *);

static int SigUsr1;
static int SigAlrm;
static int lfo_on_the_page_flag;
static char *frame_type[9] = 
{
	"EOF     ",					/* 0 */
	"Start Pg",					/* 1 */
	"End Pg  ",					/* 2 */
	"Crop Mk ",					/* 3 */
	"Rule/Box",					/* 4 */
	"UNKNOWN ",					/* 5 */
	"Overflow",					/* 6 */
	"Text    ",					/* 7 */
	"Graphic "					/* 8 */
};
static int vj_amt;
static int vj_top;
static int vj_line_cnt;
static int vj_line_adj;
static int vj_para_cnt;
static int vj_para_adj;
static int vj_exld_cnt;
static int vj_exld_adj;
static int vj_vbnd_cnt;
static int vj_vbnd_adj;
static uint32 current_vid_box_plate;

Pfd	overfd;						/* Temp file for overflow text. */
Pfd	Fofd;						/* FO file, PostScript file */
int PageNo1, PageNo2, PageNo3;
int16 nch;
int16 fobuf[256];
int16 forec, fowrd;
int crop_flag;
int FgColor;
int FgShade;
int BgColor;
int BgShade;
int bg_color_flag;
int delay_trap_flag;
int delay_bg_flag;
int DefaultFrameFgColor;
int DefaultFrameFgShade;
int ActiveTrapColor;			/* -1 if not active */
int ActiveTrapShade;
int16 ActiveTrapStrokeWidth;	/* in horizontal base units */
int FgClr, FgShd;
int BgClr, BgShd;
int BgBlendStartClr, BgBlendStartShd;
int BgBlendEndClr, BgBlendEndShd;
int FlashOn;
int16 para_adj,vbnd_adj,exld_adj,line_adj;
int in_overflow;
int was_overflow;
int Xmark;
int tab_offset;				/* horizntal mru disp for tabular */
int Ypos;
int Xpos;
int Bol_Ypos;
char nostring[] = " ";
int16 PageRotationAngle;
int16 PageRotationX;			/* x - center of rotation */
int16 PageRotationY;			/* y - center of rotation */
float Rwei;
DRAW_POINT_X_Y *lp;
int np_from_ele;
int repeat_ele, repeat_rel;
int CurrentFrame, CurrentEle;
int started_overlap_flag;
float trim_mark_width, trim_mark_depth;
int16 frame_layer_flag;
int frame_fg_attribute;			/* 0 = opaque, 1 = clear foreground */
float bb_left, bb_top, bb_right, bb_bottom;
int overflow_depth;
int last_y;
int OverflowOrColorFlag;
int start_dxy_flag;
int overflow_page_buff[256];
int overflow_page_buff_count;
int ly_page_started_flag;
int tx_page_started_flag;
int FoFileType;
struct vid_box *current_vid_box;
uint32 active_trap_cc_mask;
uint32 cc_mask;					/* current color mask */
uint32 cc_hit;					/* colors we have seen */
LDEF fo_line_def;				/* .fo line start group */
int spot_pass;					/* spot color pass (1 or 2) */
int FileType;					/* 0 = galley, 1 = layout */
int SetX;
int SetY;
int16 ArtId;
float ps_lay_page_width, ps_lay_page_depth;
int CircleFlag;
float RoundCornerRadiusTL;
float Rotate;					/* angle of rotation (degrees) */
int CmdFM_FootCount, CmdFM_TextCount;
int SavePaintOnPage1, SavePaintOnPage2;
int vj_pending;
int vj_para_flag;
int EditTraceStarted;
int EditTrace_beg_x1;
int EditTrace_beg_x2;
int ET_FakeTabLineFlag;
int ET_InsertStartDelay1;
int ET_InsertEndDelay1;
int ET_InsertStartDelay2;
int ET_InsertEndDelay2;
int ET_DeleteDelay1;
int ET_DeleteDelay2;
int ET_LineStartedFlag;
int last_good_line;
int ForceLineNbrFont;
int MaxPP_Meas;
int output_crop_mark_flag;
char rtype;
int export_newpiece;
float export_x=0;
float export_y=0;
float export_end_x=0;
float export_end_y=0;

extern struct clr_lst *clr_1st[];
extern int ResendPageFlag;
extern int BGOutputFlag;
extern char overflow_path[];
extern int U_PrtCharDepth;
extern char ln_prt_buff[200][132];
extern char tln_prt_buff[200][132];
extern int mpu_next_line;
extern int RuleType;			/* 0=vertical, 1=horizontal, 2=solid */
extern int RuleWt;
extern int RuleWid;
extern int RuleHgt;
extern int16 mu_rot_degree;
extern int16 mu_rot_x, mu_rot_y;
extern int parse_mode;
extern int KeyRegisterLength;	/* in points */
extern double KeyRegisterWt;
extern LC_LAY_REC *existing_ent;
extern int us_beg_x1;
extern int us_beg_x2;
extern int us_start_Ypos[];
extern int continued_bf_flag;
extern int AccentStartFlag;
extern int AccentEndFlag;
extern int MathStartFlag;
extern int StrikeThruRuleStartFlag;
extern int StrikeThru_beg_x;
extern int LockFlag;
extern int HNJ_LockFailureFlag;
extern int exported;
extern int export_ctr;
extern int export_frametype;
char export_name[52];

/***********************************************************************/
int dtext(void)
{
	char err_buf[128];
	int i, err;
	struct stat text_file, fo_file;
	
    if( !Fofd)					/* no channel now open */
	{
		if ( p_stat(TreeName, FO_FILE, SubDirName, JobName, &fo_file) )
		{						/* no .fo file, can we HandJ? */
			if ( p_stat(TreeName, TEXT_FILE, SubDirName, JobName, &text_file) )
				return(1);		/* no text file, give up */
			if ( do_HandJ(JobName, 0)) /* HandJ of .fo file */
			{					/* HandJ failure */
				if ( p_stat(TreeName, FO_FILE, SubDirName, JobName, &fo_file) )
					return(2);	/* HandJ failure, give up if no .fo */
			}
		}
		else
		{						/* have .fo; if not later than .txt, HandJ */
			if ( p_stat(TreeName, TEXT_FILE, SubDirName, JobName, &text_file) )
				return(3);		/* no text file, give up */
			if ( fo_file.st_mtime < text_file.st_mtime)
			{					/* .txt is later than .fo, do HandJ */
				if ( do_HandJ(JobName, 0)) /* HandJ of .fo file */
				{
					if ( p_stat(TreeName, FO_FILE, SubDirName, JobName,
								&fo_file) )
						return(4); /* HandJ failure, give up if no .fo */
					if ( fo_file.st_mtime < text_file.st_mtime)
						return (5);
				}
			}
		}
		if((Fofd=p_open(TreeName, FO_FILE, SubDirName, JobName, "r")) <=0 )
			return(1);			/* no .fo file, give up */
	}
#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "start dtext(), job= '%s', dir= '%s', pass= %d \n",
			   JobName, SubDirName, spot_pass);
#endif
    foread(0);					/* Set up page size for output */
    foread(1);					/* put record 1 in fobu */
	i = cc_mask;				/* save starting value */
	if (spot_pass == 1)
		cc_mask = 1 & Plates;
	else
		cc_mask = cc_hit & Plates;
#ifdef TRACE
	if(debugger_trace)
		p_info(PI_TRACE, "***START DTEXT PASS %d ***\n",spot_pass);
#endif
    tp_open();					/* Open PostScript file, name */
	cc_mask = i;				/* restore starting value */
	CmdFM_FootCount = 0;
	CmdFM_TextCount = 0;
    para_adj   = 0;
	vbnd_adj   = 0;
    tab_offset = 0;
    Xpos = 0;					/* x offset. */
	Ypos = 0;
	Bol_Ypos = 0;
	FgColor = 1;
	FgShade = 0;
	DefaultFrameFgColor = 1;
	DefaultFrameFgShade = 100;
	bg_color_flag = 0;
	ActiveTrapColor = -1;
	active_trap_cc_mask = 0;
	utype = 0;
	usw = 0;
	StrikeThruRuleStartFlag = 0;
	SetX = 0;
	SetY = 0;
	FlashOn = 1;
	EditTrace_beg_x1 = 0;
	EditTrace_beg_x2 = 0;
	EditTraceStarted = 0;
	ET_FakeTabLineFlag = 0;
	ET_InsertStartDelay1 = 0;
	ET_InsertEndDelay1 = 0;
	ET_InsertStartDelay2 = 0;
	ET_InsertEndDelay2 = 0;
	ET_DeleteDelay1 = 0;
	ET_DeleteDelay2 = 0;
	ET_LineStartedFlag = 0;
	last_good_line = 0;
	ForceLineNbrFont = 0;
	trim_mark_width = 0;
	trim_mark_depth = 0;
	vj_pending = 0;
	continued_bf_flag = 0;
	MathStartFlag = 0;
	Kmd_PP_Started = 0;
	MaxPP_Meas = 0;
    for(;;)
    {
		AccentStartFlag = 0;
		AccentEndFlag = 0;
		nch=foget();			/* Read start-of-line FO word */
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "forec size=%d\n",nch);
#endif
		if (nch < 0)			/* At EOF: */
		{
#ifdef TRACE
			if(debugger_trace)
				p_info(PI_TRACE, "EOF for galley\n");
#endif
			if (Fofd)
			{
				p_close(Fofd);	/* Close FO file, */
				Fofd = 0;
				FoName[0] = 0;
			}
			CurrentLineBcEc = 0;
			cc_hit &= (Plates & ~1); /* only use plates to be output */
			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			if ( DidPage)		/* only do this if page started */
			{
				if ( MultiOddPage_cc_mask && MultiPagesUp)
				{					/* if odd galley, need trailers for even gal  */
					uint32 cc_mask_eof_hold;
	
					MultiPagesOddEvenFlag = 1;	/* Force end-page output.  */
					cc_mask_eof_hold = MultiOddPage_cc_mask;
					cc_mask = MultiOddPage_cc_mask & 1;
					BlackPaint_cc_mask = cc_mask;
					if ( cc_mask)
					{
						DidPage = 1;
						spot_pass = 1;
						do_pskmd('P',"last galley pg, Multi, black");
					}
					cc_mask = cc_mask_eof_hold & ~1;
					MultiPagesOddEvenFlag = 1;
					if ( cc_mask)
					{
						DidPage = 1;
						spot_pass = 2;
						do_pskmd('P',"last galley pg, Multi, non-black");
					}
					MultiPagesOddEvenFlag = 1;
				}
				else		/* Normal (not an even page left hanging).  */
				{
					do_pskmd('P',"last galley page"); /* show last PS page, */
				}
			}		/* end  if (DidPage)   */
			if ( (spot_pass == 2 ) || (cc_hit < 2) || !KeyOutputType)
			{
				if( spot_pass == 2 )
				{
					m_close(-1); /* Close all files. */
					spot_concat(); /* fold files into black */
				}
				cc_mask = 1;
				do_pskmd('Z',"end galley take"); /* end of all galleys */
			}
			else
				m_close(-1);	/* Close all files. */
			return(0);
		}
		else if(nch == 0)    
			fowrd=255;			/* FO block empty; get next. */
		else
		{						/* Good FO line: */
			holdforec = forec;	/* Save start-of-line FO ptrs */
			holdfowrd = fowrd;
			if ( (err = read_line_def( &fo_line_def, foget )) )
			{
				sprintf(err_buf, "PP ERROR - Unable to read line start after line %d, error %d\n",
						last_good_line, err);
				stop(err_buf, 0,0);
			}
			if ( Kmd_PP_Started &&
				((fo_line_def.SolMeasure+fo_line_def.SolMarginAdjust) > MaxPP_Meas))
				MaxPP_Meas = fo_line_def.SolMeasure + fo_line_def.SolMarginAdjust;
			if ( BcEcFlag && !Ktabmode)
			{
				PrevLineBcEc = CurrentLineBcEc;
				CurrentLineBcEc = fo_line_def.MiscLineFlags & 0x100;
				if ( PrevLineBcEc && !CurrentLineBcEc)
				{				/* turn on output, force out new parameters */
					BcEcExtraLeadNeeded = BcEcExtraLead;
					FgColor = -2; /* will force out current color */
					Holdfont = 0;
					if( !DidPage)
					{
						Ypos = 0;
						Bol_Ypos = 0;
					}
				}
			}
#ifdef TRACE
			if(debugger_trace)
				p_info(PI_TRACE, "BcEc for line= %d\n", CurrentLineBcEc);
#endif
			last_good_line = fo_line_def.LineNum;
			DontPaintFlag = fo_line_def.MiscLineFlags & 0x10;
			ActiveTrapColor = -1;
			active_trap_cc_mask = 0;
			utype = 0;
			usw = 0;
			StrikeThruRuleStartFlag = 0;
			fo_line_def.BolLeading +=
				fo_line_def.BolExpandableExtraLead + 
					fo_line_def.BolFlexibleExtraLead +
						fo_line_def.BolRigidExtraLead + 
							fo_line_def.BolDroppableExtraLead + 
								fo_line_def.BolDroppableExtraLeadTop + 
									fo_line_def.BolFlexibleExtraLeadTop;
			fo_line_def.Quad = fo_line_def.Quad & 7;
			if (fo_line_def.SolFgColor < 0)		/* Init value from H&J, no color given. */
			{
				fo_line_def.SolFgColor = (int16)DefaultFrameFgColor;
				fo_line_def.SolFgShade = (int16)DefaultFrameFgShade;
			}
			if ( ((FgColor != (int)fo_line_def.SolFgColor) ||
				  (FgShade != (int)fo_line_def.SolFgShade)) && 
				 (fo_line_def.LineNum != 1) )
				color_func((int)fo_line_def.SolFgColor,
						   (int)fo_line_def.SolFgShade);
			xyset();			/* Set start-of-line X-pos, */
			jbl = fo_line_def.SolBaseLine; /* start-of-line bsln shift, */
			Bol_Ypos = Ypos;
			camy(fo_line_def.BolLeading,
				 "camy from psdtext"); /* advance leading of line */
			us_beg_x1 = Xpos;
			us_beg_x2 = Xpos;
			us_start_Ypos[1] = Ypos;
			us_start_Ypos[2] = Ypos;
			StrikeThru_beg_x = Xpos;
			EditTraceStarted = 0;
			if ((EditTraceFlag & 1) && (fo_line_def.MiscLineFlags & 0x40))
			{
				EditTrace_beg_x1 = Xpos;
				EditTraceStarted |= 1;
			}
			if ((EditTraceFlag & 2) && (fo_line_def.MiscLineFlags & 0x80))
			{
				EditTrace_beg_x2 = Xpos;
				EditTraceStarted |= 2;
			}
			ps_drawline();		/* process and set the line.*/
		}						/* End else */
    }							/* End for(;;). */
    return(0);
}								/*  End dtext. */

/***********************************************************************/
int dxy(void)
{
	int16 i, jj;
    uint32 cc_op;				/* map of opened output files */
	uint32 cc_mask_hold, cc_mask_eof_hold;
	uint32 cc_hit_hold;
    int top_of_frame_flag;		/* 1 = top-line-of-frame. */
	char err_buf[128];
	int err;
	struct ps_pieces *current_rec;
	struct ps_pieces *head;
	char *ptr;
	char old_fo_name[64];
	char new_fo_name[64];
	int start_forec, start_fowrd, start_foline;
	int end_forec, end_fowrd, end_foline;
	struct stat text_file, fo_file;
	int file_flag, result, answer;
	int save_multi_pages_odd_even_flag = 0;
	int save_BGOutputFlag;
	float page_x_scale = 0;
	float page_y_scale = 0;
	int16 BolLeading;

	cc_hit = 0;
    was_overflow = 0;
	Oldfont = -1;
    Xmark = 0;
	Xpos = 0;
    Ypos = 0;
	Bol_Ypos = 0;
    tab_offset = 0;
    foread(0);					/* Just set up page data */
	DefaultFrameFgColor = 1;
	DefaultFrameFgShade = 100;
	bg_color_flag = 0;
	repeat_ele = 0;
	repeat_rel = 0;
	current_vid_box = 0;
	current_vid_box_plate = 1;
	overflow_depth = 0;
	OverflowOrColorFlag = 0;
	start_dxy_flag = 0;
	ly_page_started_flag = 0;
	tx_page_started_flag = 0;
	lfo_on_the_page_flag = 0;	/* 1 after lfo is on a page */
	SavePaintOnPage1 = 0;
	SavePaintOnPage2 = 0;
	PaintOnPage = 0;
	MultiOddPage_cc_mask = 0;
	vj_top = 0;
	vj_line_cnt = 0;
	vj_line_adj = 0;
	vj_para_cnt = 0;
	vj_para_adj = 0;
	vj_vbnd_cnt = 0;
	vj_vbnd_adj = 0;
	vj_exld_cnt = 0;
	vj_exld_adj = 0;
	vj_para_flag = 0;
	last_y = 0;
	EditTrace_beg_x1 = 0;
	EditTrace_beg_x2 = 0;
	EditTraceStarted = 0;
	ET_FakeTabLineFlag = 0;
	ET_InsertStartDelay1 = 0;
	ET_InsertEndDelay1 = 0;
	ET_InsertStartDelay2 = 0;
	ET_InsertEndDelay2 = 0;
	ET_DeleteDelay1 = 0;
	ET_DeleteDelay2 = 0;
	ET_LineStartedFlag = 0;
	last_good_line = 0;
	ForceLineNbrFont = 0;
	continued_bf_flag = 0;
	MathStartFlag = 0;
	Kmd_PP_Started = 0;
	MaxPP_Meas = 0;
#ifdef TRACE
	if(debugger_trace)
		p_info(PI_TRACE, "***START DXY ***\n");
#endif
    for(;;)						/* Loop for each translated .lay record. */
    { 
		utype = 0;
		usw = 0;
		StrikeThruRuleStartFlag = 0;
		vj_pending = 0;
		if (spot_pass == 1)
			cc_mask = 1 & Plates;
		else
			cc_mask = cc_hit & Plates;
		cc_mask_hold = cc_mask;
		if ( !repeat_rel)
			get_next_piece();	/* get the next piece */
		cc_mask = cc_mask_hold;
		if ( LYPrintFlag)
		{
			FgClr = 1;
			FgShd = 100;
			BgClr = 0;
			BgShd = 100;
		}
		else
		{
			ActiveTrapColor = -1;
			ActiveTrapShade = 100;
			active_trap_cc_mask = 0;
		}
#ifdef TRACE
		if(debugger_trace)
		{
			if (PsCurRec->frame_type > 0)
			p_info(PI_TRACE, "NEXT %s (type %d.%d), SetX %d  SetY %d, fr %d, ele %d \n",
				   frame_type[PsCurRec->frame_type], PsCurRec->frame_type,
				   REL_DATA(CurrentFrame)t0, SetX, SetY, CurrentFrame,
				   CurrentEle);
			else
			p_info(PI_TRACE, "NEXT %s (type %d), SetX %d  SetY %d, fr %d, ele %d \n",
				   frame_type[PsCurRec->frame_type], PsCurRec->frame_type,
				   SetX, SetY, CurrentFrame, CurrentEle);

			p_info(PI_TRACE, "ResendPageFlg %d, BGOutputFlg %d, BG %d %d FG %d %d\n",
				   ResendPageFlag, BGOutputFlag, BgClr, BgShd, FgClr, FgShd);
		}
#endif
		if (in_overflow &&
			(PsCurRec -> frame_type != 7) ) /* If just did ovf line, */
		{
			Ofx = -4000;
			do_pskmd('M',"reopen TP file"); /* Flush last line */
			m_close(cc_mask);	/* close out work files, re-open TP files */
			cc_op = m_fopen(tpname,"a+",cc_mask);
			if(cc_op != cc_mask)
			{
				p_info(PI_ELOG, "ERROR - reopening PO file\n");
				return(1);
			}
			in_overflow = 0;	/* Turn switch off. */
		}
		FlashOn = 1;
		switch(PsCurRec -> frame_type)
		{
		  case 0:				/* End of file. Quit. */
			DontPaintFlag = 0;
			if(Fofd)
			{
				p_close(Fofd);
				Fofd = 0;
				FoName[0] = 0;
			}
			if ( (LYPrintFlag & 03) && Reports)
			{
				cc_mask = 1;
				output_mpu_post_report ();
			}
			if ( MultiOddPage_cc_mask && MultiPagesOddEvenFlag)
			{					/* if odd page, need trailers for even pg */
				cc_mask_eof_hold = MultiOddPage_cc_mask;
				cc_mask = MultiOddPage_cc_mask & 1;
				BlackPaint_cc_mask = cc_mask;
				if ( cc_mask)
				{
					DidPage = 1;
					spot_pass = 1;
					do_pskmd('P',".page end, Multi, black");
				}
				cc_mask = cc_mask_eof_hold & ~1;
				MultiPagesOddEvenFlag = 1;
				if ( cc_mask)
				{
					DidPage = 1;
					spot_pass = 2;
					do_pskmd('P',".page end, Multi, black");
				}
				MultiPagesOddEvenFlag = 1;
				m_close(-1);
				cc_hit = cc_mask_eof_hold & ~1;
				if ( cc_hit)
					spot_concat(); /* fold files into black */
			}
			DidPage = 1;
			cc_mask = 1;
			m_fopen ( tpname, "a+", 1);
			do_pskmd('Z',".page EOF.");	/* flag end of file */
			return(0);

		  case 1:				/* new page */
			output_crop_mark_flag = 0;
			EditTraceStarted = 0;
			CmdFM_FootCount = 0;
			CmdFM_TextCount = 0;
			num_fnglobals = 0;
			DontPaintFlag = 0;
			ly_page_started_flag = 0;
			tx_page_started_flag = 0;
			PageRotationAngle = 0;
			PageRotationX = 0;
			PageRotationY = 0;
			if ( LYPrintFlag & 0x8)
				init_mpu_lprint ();
			if ( LYPrintFlag & 0x10)
				init_mpu_txprint ();
			ps_lay_page_width =(float)(lmt_off_to_abs(wn,X_REF, TRIM_WIDTH) +
									   (trim_mark_width *2 )) / HorizontalBase;
			ps_lay_page_depth =(float)(lmt_off_to_abs(wn,Y_REF, TRIM_DEPTH) +
									   (trim_mark_depth *2 )) / VerticalBase;
			if ( MultiPagesUp)
				ps_lay_page_width = (ps_lay_page_width * 2);
			if (KeyScale == -1)
			{					/* compute scaling for page size */
				if ( !Orient)
				{				/* portrait */
					page_x_scale = ((double)(PageW - OffL - OffR)) /
						ps_lay_page_width;
					page_y_scale = ((double)(PageH - OffT - OffB)) /
						ps_lay_page_depth;
				}
				else
				{				/* landscape */
					page_x_scale = ((double)(PageH - OffT - OffB)) /
						ps_lay_page_width;
					page_y_scale = ((double)(PageW - OffL - OffR)) /
						ps_lay_page_depth;
				}
				if (page_x_scale < page_y_scale)
				{				/* x is smaller, use it to scale page */
					ScaleFactorX = page_x_scale;
					ScaleFactorY = page_x_scale;
				}
				else
				{				/* y is smaller, use it to scale page */
					ScaleFactorX = page_y_scale;
					ScaleFactorY = page_y_scale;
				}
			}
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "***START PASS %d *** (page %d)\n",
					   spot_pass, PageNo1);
#endif
			if ( !start_dxy_flag)
			{
				if (tp_open())
					return(1);	/* Open PostScript file, name */
				start_dxy_flag++;
				if ( ( LYPrintFlag & 3) && (Reports <= 0))
				{
					cc_mask = 1;
					psmpu_main();
					if ( !(LYPrintFlag & ~3) && !Reports )
					{			/* no other rpts and only directory rpt 1, 2 */
						end_JOB();
						return(0); /* no more reports, all done */
					}
				}
				else if ( ( LYPrintFlag & 3) && (Reports == 1))
					psmpu_main();
				if ( (LYPrintFlag & 0xb) && Reports)
					init_mpu_in_use_print(); /* init report on files in use */
			}
			frame_fg_attribute = 0;
			if ( !ResendPageFlag)
				current_vid_box_plate = 1;
			repeat_ele = 0;
			repeat_rel = 0;
			break;

		  case 2:				/* end of page */
			output_crop_mark_flag = 0;
			EditTraceStarted = 0;
			cc_hit &= (Plates & ~1); /* mask off the black */
			if ( MultiPagesUp)
				save_multi_pages_odd_even_flag = MultiPagesOddEvenFlag;
			DontPaintFlag = 0;
			cc_mask_hold = cc_mask;
			cc_hit_hold = cc_hit;
			if (spot_pass == 1)
			{
				cc_mask = 1 & Plates;
				cc_hit |= (Plates & ~1); /* force all plates out */
			}
			else
				cc_mask = cc_hit & ~1;
			if(PageRotationAngle && DidPage)
			{					/* unrotate if page was output */
				Rotate = -PageRotationAngle;
				Fx = PageRotationX / HorizontalBase;
				Fy = (Imageheight - PageRotationY) / VerticalBase;
				if ( !LYPrintFlag || ( LYPrintFlag & 4) )
					do_pskmd('X',"unrotate frame");
			}					/* end if(PageRotationAngle) */
			if ( (spot_pass == 1) && (Plates & 1) && !LYPrintFlag)
			{					/* output black if not output and required */
				if ( !DidPage)
					beg_PAGE();
			}
			cc_mask = cc_mask_hold;
			cc_hit = cc_hit_hold;
			PageRotationAngle = 0;
			frame_fg_attribute = 0;
			if (in_overflow)
				reopen_tp_if_overflow(); /* If now in an overflow frame. */
			if ( LYPrintFlag & 4)
			{
				cc_mask = 1;
				OverflowOrColorFlag = 1;
				do_pskmd('P',".page end"); /* Close real page in TP file */
			}
			if ( LYPrintFlag & 0x8)
				output_mpu_lprint ();
			if ( LYPrintFlag & 0x10)
				output_mpu_txprint ();
			if ( SupressBlankPages)
			{
				if ( !save_multi_pages_odd_even_flag)
				{				/* end of even page */
					if ( spot_pass == 1)
						SavePaintOnPage1 = PaintOnPage;
					else
						SavePaintOnPage2 = PaintOnPage;
				}
				else
				{				/* end of odd page */
					if ( spot_pass == 1)
					{
						PaintOnPage |= SavePaintOnPage1;
						SavePaintOnPage1 = 0;
					}
					else
					{
						PaintOnPage |= SavePaintOnPage2;
						SavePaintOnPage2 = 0;
					}
				}
			}					/* end if(SupressBlankPages) */
			if ( (spot_pass == 1) && !LYPrintFlag )
			{					/* plate 1 */
				cc_mask = 1 & Plates;
				BlackPaint_cc_mask = cc_mask;
				OverflowOrColorFlag = 1;
				do_pskmd('P',".page end"); /* Close real page in TP file */
				if(was_overflow) /* Was any overflow text on pg? */
				{				/* YES. Transfer into TP file: */
					m_tran_ov(overflow_path); /* xfer text from temp file */
					was_overflow = 0; /* Reset all overflow variables. */
					overflow_depth = 0;
				}
			}					/* end if(spot_pass==1) */
			else if ( !LYPrintFlag )
			{					/* if (spot_pass==2) */
				OverflowOrColorFlag = cc_hit;
				cc_mask = cc_hit & Plates;
				BlackPaint_cc_mask = cc_mask;
				do_pskmd('P',".color page end");
			}					/* end if(spot_pass==2) */
			if ( SupressBlankPages)
				PaintOnPage = 0;
/* color loop processing */
			if(((spot_pass == 1) && (cc_hit < 2)) || /* no color for pass 2? */
			   (spot_pass == 2) || !KeyOutputType) /* 2nd pass or composite? */
			{					/* concatenate unless odd page next */
				if( (cc_hit > 1) && KeyOutputType)
				{
					if ( (!MultiPagesUp ||
						  ( MultiPagesUp && save_multi_pages_odd_even_flag)) )
					{
						m_close(-1);
						spot_concat(); /* fold files into black */
					}
				}
				cc_hit = 0;
				cc_mask = (1 & Plates);
				BlackPaint_cc_mask = cc_mask;
				ResendPageFlag = 0; /* turn off resend loop */
								/* so we get EOF or next page */
				spot_pass = 1;	/* set pass to 1 (i.e. black, 1st pass)	*/
				for ( jj=0; jj < obj_wn_count; jj++)
				{				
					clean_wysiwyg(ObjPageWn.wns[jj]);
					ObjPageWn.page_number[jj] = 0;
					ObjPageWn.wns[jj] = 0;
				}
				obj_wn_count = 0;
				head = PsHead;
				while ( head)
				{				/* clean up page pieces structures */
					current_rec = head;
					head = current_rec -> next;
					p_free ( (char *)current_rec);
				}
				clear_vlist();	/* clear list of frame bg bound box areas */
				current_vid_box_plate = 1;
				lfo_on_the_page_flag = 0;
			}
			else
			{
				spot_pass = 2;	/* set pass to "do colors" */
				cc_hit &= (Plates & ~1); /* only use plates to be output */
				cc_mask = cc_hit; /* starting bg, fg pass 2 clrs */
				BlackPaint_cc_mask = cc_mask;
				current_vid_box_plate = 2;
				if (tp_open())
					return(1);	/* Open PostScript file, name */
				ResendPageFlag = 1; /* tell parse to resend */
			}
			current_vid_box = 0;
			parse_mode = -1;
			if ( (save_multi_pages_odd_even_flag != MultiPagesOddEvenFlag) &&
				 !MultiPagesOddEvenFlag && MultiPagesUp )
				MultiOddPage_cc_mask = 0;
			if ( MasterNameFlag || PageName[0])
			{
				if(Fofd)
				{
					p_close(Fofd);
					Fofd = 0;
					FoName[0] = 0;
				}
				if ( (LYPrintFlag & 03) && Reports)
				{
					cc_mask = 1;
					output_mpu_post_report ();
				}
				if (spot_pass == 1)
					cc_mask = 1;
				else
					cc_mask = cc_hit;
				do_pskmd('Z',".page EOF.");	/* flag end of file */
				return (0);		/* only one page if a Layout */
			}
			break;

		  case 3:				/* crop marks */
			if ( LYPrintFlag)
				break;			/* no crops if LayoutPrint */
			if (exported == 1)	/* Frame type wrong.  If exporting,  */
			{					/*  suspend PSI export */
				exported = 2;
				m_fprintf("%%PSI_ExportSuspend\n");
			}
			cc_mask = 1 & Plates;
			BlackPaint_cc_mask = 0;	/* no marks although black */
			if (spot_pass == 2)
				cc_mask = cc_hit;
			BgClr = 1;
			BgShd = 100;
			FgClr = 1;
			FgShd = 100;
			BgColor = 1;
			BgShade = 100;
			FgColor = 1;
			FgShade = 100;
			frame_layer_flag = 0;
			frame_fg_attribute = 0;
			output_crop_mark_flag = 1;
			if (RuleType != 5)
				xyrulebox();	/* vert or horiz rule */
			else
			{					/* register mark */
				if ( !DidPage)
					beg_PAGE();
				Fx = SetX / HorizontalBase;
				Fy = (Imageheight - SetY) / VerticalBase;
				m_fprintf("%d %4.2f %4.2f %4.2f registermarks\n",
						  KeyRegisterLength, KeyRegisterWt, Fx, Fy);
			}
			output_crop_mark_flag = 0;
			if ( SupressBlankPages )
				PaintOnPage = 0; /* ignore paint till text start */
			break;

		  case 4:				/* Rule/Box */
			DontPaintFlag = 0;
			reopen_tp_if_overflow();
			if (exported == 1)	/* Frame type wrong.  If exporting,  */
			{					/*  suspend PSI export */
				exported = 2;
				m_fprintf("%%PSI_ExportSuspend\n");
			}
			if ( (BgBlendStartClr || BgBlendEndClr) && !RuleWt)
			{					/* try a blend */
				frame_layer_flag = 0;
				frame_fg_attribute = FRAME_FLAGS(CurrentFrame) & FG_ATTRIB;
				bg_color_flag = 1;
				save_BGOutputFlag = BGOutputFlag;
				BGOutputFlag = 1;
				frame_color(0);
				BGOutputFlag = save_BGOutputFlag;
				bg_color_flag = 0;
			}
			else
			{					/* not blend */
				i = FgClr;
				FgClr = BgClr;
				BgClr = i;
				i = FgShd;
				FgShd = BgShd;
				BgShd = i;
				frame_layer_flag = 0;
				frame_fg_attribute = FRAME_FLAGS(CurrentFrame) & FG_ATTRIB;
				frame_color(1);
				if ( !cc_mask || LYPrintFlag)
					break;		/* no rule_box if no fg color, or LY print */
				xyrulebox();
			}
			break;

		  case 8:				/* graphic */
			DontPaintFlag = 0;
			reopen_tp_if_overflow();
			if (exported == 1)	/* Frame type wrong.  If exporting,  */
			{					/*  suspend PSI export */
				exported = 2;
				m_fprintf("%%PSI_ExportSuspend\n");
			}
			bg_color_flag = 1;
			delay_trap_flag = 1;
			delay_bg_flag = 0;
			frame_layer_flag = FRAME_FLAGS(CurrentFrame) & LAYER;
			frame_fg_attribute = 0;
			Ofx = -4000;		/* force Move command out */
			do_pskmd('m',"start graphic");
			frame_color(0);
			if ( cc_mask && ( !LYPrintFlag || ( LYPrintFlag & 4)) )
			{
				if (!BGOutputFlag)
				{
					PaintOnPage |= BlackPaint_cc_mask;
					graphic(0);		/* Arg. val 0: not from [mg-] */
				}
			}
			delay_trap_flag = 0;
			delay_bg_flag = 1;
			frame_color(0);		/* output stroke, if any */
			break;

		  case 6:				/* overflow */
			if ( BGOutputFlag)
				break;			/* ignore overflow if only printing BG */
			
		  case 7:				/* text */
			DontPaintFlag = 0;
								/* If MP frame type is footnote:  */
			if ( REL_DATA(CurrentFrame) t0 == 3)
				FootnoteFrameFlag = 1;
			else
				FootnoteFrameFlag = 0;
			strcpy (old_fo_name, FoName);
			if ( FoFileType == FO_FILE)
				strcat (old_fo_name, ".fo");
			else if ( FoFileType == LFO_FILE)
				strcat (old_fo_name, ".lfo");
			ArtId = 0;
			FoName[0] = 0;
			FoFileType = FO_FILE;		/* Assume text from .fo */
			wn = PsCurRec -> text_wn;	/* Use *wn for text content, in case
											of object-ref frame. For all but
											obj-ref frames, frame_wn and
											text_wn are identical.  */
										/* Also reset to frame# of text content,
											in case we used orig_frame_nbr to get
											frame-pos info in get_next_piece():  */
			CurrentFrame = PsCurRec -> frame_nbr;
			if (TYPE_OF_FRAME(CurrentFrame) == PL_TEXT)
			{
				FoFileType = LFO_FILE;	/* No, text from Layout */
				strcpy ( FoName, LayoutName );
			}
			else if (ARTICLE_ID(CurrentFrame))
			{				/* text from .fo */
				ArtId = ARTICLE_ID(CurrentFrame);
				ptr = get_ps_fname(ArtId);
				if (ptr)
					sprintf(FoName, "%s", ptr);
			}
#ifdef TRACE
			if(debugger_trace)
			{
				if ( !FoName[0])
					p_info(PI_TRACE, "next_piece = no article\n");
				else if ( FoFileType == FO_FILE)
					p_info(PI_TRACE, "next_piece = new .FO file %s\n",FoName);
				else
					p_info(PI_TRACE, "next_piece = new .LFO file %s\n",FoName);
			}
#endif
			if ( (LYPrintFlag & 0xb) && Reports && (spot_pass == 1) )
				verify_and_add_mpu_name ( 0);
			if (!FoName[0] && !BGOutputFlag && !LYPrintFlag)
			{
				if ( !FRAME_FLAGS(CurrentFrame) & LAYER)
				{
					wn = PsCurRec -> frame_wn;	/* Restore true wn.  */
					CurrentFrame = PsCurRec -> orig_frame_nbr;
					break;		/* skip empty frame  */
				}
			}
			if ( FoName[0])
			{
				strcpy (new_fo_name, FoName);
				file_flag = 0;
				if ( FoFileType == FO_FILE)
					strcat (new_fo_name, ".fo");
				else if ( FoFileType == LFO_FILE)
				{
					strcat (new_fo_name, ".lfo");
					file_flag = 1;
				}
				if ( strcmp (old_fo_name, new_fo_name) )
				{				/* need to open a new fo file */
					if (Fofd)
					{
						p_close(Fofd);
						Fofd = 0;
					}
					result = 0;
					answer = p_stat(TreeName, FoFileType, SubDirName,
									FoName, &fo_file );
					if ( !file_flag)
					{			/* .fo file */
						if ( answer )
						{		/* no .fo file, can we HandJ? */
							if ( !p_stat(TreeName, TEXT_FILE, SubDirName,
										 FoName, &text_file) )
							{
								result = do_HandJ(FoName, 0); /* .fo file */
								scan_map(existing_ent -> index);
							}
						}
						else
						{		/* have .fo; if not later than .txt, HandJ */
							result = p_stat(TreeName, TEXT_FILE, SubDirName,
											FoName, &text_file);
							if ( result)
								return(10);	/* no text file, give up */
							if (fo_file.st_mtime < text_file.st_mtime)
							{
								result = do_HandJ(FoName, 0); /* .txt newer */
								scan_map(existing_ent -> index);
							}
						}
					}
					else if ( !lfo_on_the_page_flag)
					{			/* .lfo file, not seen on page yet */
						if ( answer )
						{		/* no .lfo file, do the HandJ */
							result = do_HandJ(FoName, 1); /* .lfo file */
							lfo_on_the_page_flag = 1;
							scan_map(existing_ent -> index);
						}
					}
/*					if ( !result) */ /* fix for 329P on 11/13/96 */
					Fofd = p_open(TreeName, FoFileType, SubDirName, FoName, "r+");
					if(!Fofd)
					{
						Xpos = HorizontalBase; /* Default left margin: 1 pica*/
						frame_fg_attribute = 0;
						FoName[0] = 0;
						wn = PsCurRec -> frame_wn;	/* Restore true wn.  */
						CurrentFrame = PsCurRec -> orig_frame_nbr;
						break;
					}
					foread(0);
				}				/* end if (FoName[0]) */
			}					/* end open of new file */
			wn = PsCurRec -> frame_wn;	/* Restore true wn.  */
			CurrentFrame = PsCurRec -> orig_frame_nbr;
			Xpos = HorizontalBase; /* Default left margin: 1 pica */
			frame_fg_attribute = 0;
			top_of_frame_flag = 1; /* Doing line 1 of frame. */
			vj_amt = 0;
			if (PsCurRec -> frame_type == 6)
			{					/* overflow */
				vj_top = 0;
				vj_line_cnt = 0;
				vj_line_adj = 0;
				vj_para_cnt = 0;
				vj_para_adj = 0;
				vj_vbnd_cnt = 0;
				vj_vbnd_adj = 0;
				vj_exld_cnt = 0;
				vj_exld_adj = 0;
			}
			else
			{					/* normal text frame */
				if ( !BGOutputFlag)
				{
					if(PsCurRec -> elem -> map_data.vj_mode & 0x10)
						top_of_frame_flag = 0; /* no crush el at frame top */
					if(PsCurRec -> elem -> map_data.vj_mode & 0x20)
					{
						if( PsCurRec -> elem -> map_data.vj_para_adj)
						{
							vj_amt = PsCurRec->elem -> map_data.vj_para_adj /
								PsCurRec -> elem -> map_data.vj_para_cnt;
							PsCurRec -> elem -> map_data.vj_para_adj -=
								vj_amt;
							PsCurRec -> elem -> map_data.vj_para_cnt--;
							if(debugger_trace)
								p_info(PI_TRACE, "adjust paragraph at start frame %d\n",
									   vj_amt);
						}
					}
				}
				vj_top = PsCurRec -> elem -> map_data.vj_top;
				vj_line_cnt = PsCurRec -> elem -> map_data.vj_line_cnt;
				vj_line_adj = PsCurRec -> elem -> map_data.vj_line_adj;
				vj_para_cnt = PsCurRec -> elem -> map_data.vj_para_cnt;
				vj_para_adj = PsCurRec -> elem -> map_data.vj_para_adj;
				vj_vbnd_cnt = PsCurRec -> elem -> map_data.vj_vbnd_cnt;
				vj_vbnd_adj = PsCurRec -> elem -> map_data.vj_vbnd_adj;
				vj_exld_cnt = PsCurRec -> elem -> map_data.vj_exld_cnt;
				vj_exld_adj = PsCurRec -> elem -> map_data.vj_exld_adj;
#ifdef TRACE
				if(debugger_trace)
					p_info(PI_TRACE, "expansions t %d  l %d %d  p %d %d  e %d %d\n",
						   vj_top, vj_line_cnt, vj_line_adj, vj_para_cnt,
						   vj_para_adj, vj_exld_cnt, vj_exld_adj);
#endif
			}
			if ( LYPrintFlag)
			{
				if (PsCurRec -> frame_type == 6)
					break;		/* ignore if overflow */
			}
			Xmark  = Xpos = SetX; /* X-position always absolute.*/
			if (PsCurRec -> frame_type == 6) /* overflow? */
			{
				if ( (spot_pass == 2) || /* no action if overflow and pass 2 */
					 !PsCurRec->elem -> map_data.start_forec || /* or no fo*/
					 !( Plates & 1) ) /* or no plate 1 */
					break;
				if ( MultiPagesUp)
				{
					for (i=0; i< overflow_page_buff_count; i++)
					{
						if (PageNo1 == overflow_page_buff[i] )
							break;
					}
					overflow_page_buff[i] = PageNo1;
					if (i >= overflow_page_buff_count)
						overflow_page_buff_count = i+1;
					break;		/* no action for overflow if 2-up */
				}
				last_y = Ypos;
				frame_layer_flag = 0; /* no layering */
				frame_fg_attribute = 0;	/* opaque */
				FgColor = 1;	/* black text on white background */
				BgColor = 0;
				cc_mask = 1 & Plates;
				BlackPaint_cc_mask = cc_mask;
				Ofx = 4000;
				Oldfont = -1;
				do_pskmd('M',"b4 overflow frame"); /* Flush last line */
				m_close(1);		/* Close TP file */
				if(!was_overflow)
				{				/* is first overflow start */
					cc_op = m_fopen(overflow_path,"w+",1); 
					if(cc_op != 1)
					{
						p_info(PI_ELOG, "ERROR opening overflow file\n");
						return(1);
					}
#ifdef TRACE
					if (debugger_trace)
						p_info (PI_TRACE, "In overflow text, p.%d\n",PageNo1);
#endif
					was_overflow = 1; /* This pg has overflow */
					in_overflow = 1; /* Now in overflow block */
					Ypos  = 0;	/* New current Y-coord. */
					Bol_Ypos = 0;
					overflow_depth = 0; /* pg top.*/
					DidPage = 0;
					beg_PAGE();	/* Gen a begin-page sequence, */
				}				/* end 1st overflow */
				else			/* NOT 1st overflow */
				{
					if (!in_overflow)
					{			/* If last line was on real page, */
						cc_op = m_fopen(overflow_path,"a+",1); 
						if(cc_op != 1)
						{
							p_info(PI_ELOG, "ERROR re-opening overflow file\n");
							return(1);
						}
					}
					if (Ypos)
					{			/*  overflow: 20 points between */
						Ypos = (20 * VerticalBase) + overflow_depth;
						Bol_Ypos = Ypos;
						overflow_depth = Ypos;
#ifdef TRACE
						if (debugger_trace)
							p_info(PI_TRACE, "in overflow secondary spillage.\n");
#endif
					}
					if ( (Ypos + fo_line_def.BolLeading) > Imageheight)
					{
						OverflowOrColorFlag = -1;
						DidPage = 1;
						do_pskmd ('P',"overflow in overflow unit");
						in_overflow = 1; /* Now in overflow block */
						DidPage = 0;
						beg_PAGE();	/* Gen a begin-page sequence */
						Ypos = 0;
						Bol_Ypos = 0;
						overflow_depth = 0;
					}
				}				/* end else NOT 1st overflow */
				in_overflow = 1; /* Now in overflow block */
			}					/* end overflow */
			else				/* Start line on real page */
			{
				reopen_tp_if_overflow();
				Ypos = SetY + vj_amt; /* Absolute Y-position. */
				Bol_Ypos = SetY;
				if (!LYPrintFlag)
				{
					frame_layer_flag = FRAME_FLAGS(CurrentFrame) & LAYER;
					frame_fg_attribute = FRAME_FLAGS(CurrentFrame) & FG_ATTRIB;
				}
				else
				{
					frame_layer_flag = 0;
					frame_fg_attribute = 0;
				}
				if (exported &&				/* If in a [pp export, and  */
					export_end_y &&			/*  it's a floating ill/table frame, and
												this new frame is right type  */
					(REL_DATA(CurrentFrame) t0 == export_frametype) &&
											/*  with content (not a spacer), then: */
					PsCurRec -> elem -> map_data.start_forec)
				{							/*  get correct bounding box.  */
					float dimx, dimy, dimendx, dimendy;

					dimx = FRAME_DATA(CurrentFrame) left + trim_mark_width;
					dimy = FRAME_DATA(CurrentFrame) top + trim_mark_depth;
					dimendx = FRAME_DATA(CurrentFrame) right + trim_mark_width;
					dimendy = FRAME_DATA(CurrentFrame) bottom + trim_mark_depth;
					if (export_newpiece)	/* First frame of page:  */
					{						/* Save X/Y for UL & LR corners.  */
						export_x = dimx;
						export_y = dimy;
						export_end_x = dimendx;
						export_end_y = dimendy;
						export_newpiece = 0;/* Have begun [pp piece for this pg. */
					}
					else					/* A subsequent pg frame in [pp:  */
					{						/* Expand box (probably only bottom). */
						if (dimx < export_x)  export_x = dimx;
						if (dimy < export_y)  export_y = dimy;
						if (dimendx > export_end_x)  export_end_x = dimendx;
						if (dimendy > export_end_y)  export_end_y = dimendy;
					}
				}
				if ( !DidPage)
					beg_PAGE();
				if (exported)		/* If in a PP/PQ export area, does this MP  */
				{					/*  frame type match the requested export?
										0=Design text   3=Footnote	4=Ill/Table
										11=Sidenote		13=Flow text :  */
					if (REL_DATA(CurrentFrame) t0 == export_frametype)
					{
						if (exported == 2)	/* Frame type matches. If suspended,  */
						{					/*  resume PSI export */
							exported = 1;
							m_fprintf("%%PSI_ExportResume\n");
						}
					}
					else
					{
						if (exported == 1)	/* Frame type wrong.  If exporting,  */
						{					/*  suspend PSI export */
							exported = 2;
							m_fprintf("%%PSI_ExportSuspend\n");
						}
					}
				}
				bg_color_flag = 1;
				frame_color(0);
				if (!PsCurRec -> elem -> map_data.start_forec || LYPrintFlag)
					break;		/* no text, just a pattern or LY print */
				if ( BGOutputFlag)
					break;		/* omit text if only BG print. */
			}					/* end else line on real page */
			DefaultFrameFgColor = FgColor;
			if(!Fofd)
				break;			/* no fo */
/* Get to FO record and word */
			if (PsCurRec -> frame_type == 6)
			{					/* overflow */
				start_forec = PsCurRec -> elem -> map_data.end_forec;
				start_fowrd = PsCurRec -> elem -> map_data.end_fowrd - 2;;
				start_foline = PsCurRec -> elem -> map_data.end_line;
				end_forec = PsCurRec -> elem -> map_data.overflow_forec;
				end_fowrd = PsCurRec -> elem -> map_data.overflow_fowrd;
				end_foline = PsCurRec -> elem -> map_data.overflow_line;
			}
			else
			{					/* non_overflow */
				start_forec = PsCurRec -> elem -> map_data.start_forec;
				start_fowrd = PsCurRec -> elem -> map_data.start_fowrd - 2;;
				start_foline = PsCurRec -> elem -> map_data.start_line;
				end_forec = PsCurRec -> elem -> map_data.end_forec;
				end_fowrd = PsCurRec -> elem -> map_data.end_fowrd;
				end_foline = PsCurRec -> elem -> map_data.end_line;

			  if (TYPE_OF_FRAME(CurrentFrame) == PL_FLOW && (start_forec != end_forec) && 
					(pdf_bead > 0) )
			  {
				if (!strstr(FoName,"_float")) {
				/* Valid text frame - insert article bead */
				float dimx, dimy, dimendx, dimendy;
				dimx = (FRAME_DATA(CurrentFrame) left + trim_mark_width) / 20;
				dimy = (FRAME_DATA(CurrentFrame) top + trim_mark_depth) / 10;
				dimendx = (FRAME_DATA(CurrentFrame) right + trim_mark_width) / 20;
				dimendy = (FRAME_DATA(CurrentFrame) bottom + trim_mark_depth) / 10;
				m_fprintf("[ /Title (%s)\n", pdf_info.title);
				m_fprintf("\t/Rect [ %5f %5f %5f %5f ]\n", 
					dimx, (PageH - dimendy), dimendx, (PageH - dimy) );
				m_fprintf("\t/ARTICLE pdfmark\n");
				}
				
			  }

			}
			if ( (start_forec == end_forec) && 
				 ((start_fowrd + 2) == end_fowrd) ) /* test for no text */
			{
#ifdef TRACE
				if (debugger_trace)
					p_info (PI_TRACE, "No Text in frame %d (line %d)\n",
							CurrentFrame, start_foline);
#endif
				break;
			}
			foread(start_forec);
			fowrd = start_fowrd;
#ifdef TRACE
			if (debugger_trace)
				p_info (PI_TRACE, "Text from line %d to line %d\n",
						start_foline, end_foline);
#endif
			for(;;)				/* Loop once for each FO line */
			{
				AccentStartFlag = 0;
				AccentEndFlag = 0;
				do
				{
					nch = foget(); /* Get size of this FO line. */
					if(!nch)
						fowrd = 255; /* 0 means end-rec, get next. */
				} while(!nch);
				if ( (err = read_line_def( &fo_line_def, foget)) )
				{
					sprintf(err_buf, "PP ERROR - Unable to read line start after line %d, error %d\n",
							last_good_line, err);
					stop(err_buf, 0,0);
				}
				if ( Kmd_PP_Started &&
					((fo_line_def.SolMeasure+fo_line_def.SolMarginAdjust) > MaxPP_Meas))
					MaxPP_Meas = fo_line_def.SolMeasure + fo_line_def.SolMarginAdjust;
				last_good_line = fo_line_def.LineNum;
				DontPaintFlag = fo_line_def.MiscLineFlags & 0x10;
				ActiveTrapColor = -1;
				active_trap_cc_mask = 0;
				utype = 0;
				usw = 0;
				StrikeThruRuleStartFlag = 0;
				BolLeading = fo_line_def.BolLeading;
				Ypos += vj_top;			/* Put out any top-adjustment for frame here,  */
				vj_top = 0;				/*  reset val to 0 for rest of lines in frm.  */
				if(top_of_frame_flag)
				{
					int32 v;

					v =  lmt_off_to_abs(wn, Y_REF, Y_OFFSET(CurrentFrame));
					if ( v)
					{			/* not top of page, don't crush top extra ld */
						fo_line_def.BolExpandableExtraLead +=
							fo_line_def.BolFlexibleExtraLeadTop;
						fo_line_def.BolRigidExtraLead +=
							fo_line_def.BolDroppableExtraLeadTop;
					}
				}
				else
				{				/* omit for top line */
					fo_line_def.BolExpandableExtraLead +=
						fo_line_def.BolFlexibleExtraLead + 
							fo_line_def.BolFlexibleExtraLeadTop;
					fo_line_def.BolRigidExtraLead +=
						fo_line_def.BolDroppableExtraLead + 
							fo_line_def.BolDroppableExtraLeadTop;
				}
				if(vj_exld_adj && vj_exld_cnt && 
				   fo_line_def.BolExpandableExtraLead)
				{
					vj_amt = (vj_exld_adj*fo_line_def.BolExpandableExtraLead)/ 
						vj_exld_cnt;
					vj_exld_adj -= vj_amt;
					vj_exld_cnt -= fo_line_def.BolExpandableExtraLead;
					fo_line_def.BolExpandableExtraLead += vj_amt;
					vj_pending += vj_amt;
					if(debugger_trace)
						p_info(PI_TRACE, "adjust extra lead %d\n",vj_amt);
				}
				if(top_of_frame_flag == 0 && 
				   fo_line_def.MiscLineFlags & 0x000f &&
				   vj_vbnd_cnt != 0 && vj_vbnd_adj != 0)
				{
					int bol_vb;
					bol_vb = fo_line_def.MiscLineFlags & 0x000f;
					vj_amt = (vj_vbnd_adj * bol_vb) / vj_vbnd_cnt;
					vj_vbnd_adj -= vj_amt;
					vj_vbnd_cnt -= bol_vb;
					fo_line_def.BolExpandableExtraLead += vj_amt;
					vj_pending += vj_amt;
					if(debugger_trace)
						p_info(PI_TRACE, "adjust vertical bands %d\n",vj_amt);
				}
				if (debugger_trace)				
					p_info(PI_TRACE, "BolExtraLead = %d + %d\n",
						   fo_line_def.BolExpandableExtraLead,
						   fo_line_def.BolRigidExtraLead);
				fo_line_def.BolLeading += fo_line_def.BolExpandableExtraLead +
					fo_line_def.BolRigidExtraLead;
				fo_line_def.Quad = fo_line_def.Quad & 7;
				if (fo_line_def.SolFgColor < 0)
					fo_line_def.SolFgColor = (int16)DefaultFrameFgColor;
										/* If this is start of floating ill/table, whose
											first line has [pp, put out the export cmd: */
				if (fo_line_def.MiscLineFlags & 0x200)
				{
					Kmd_PP_Started++;
					exported=1;
					export_ctr = 0;
					MaxPP_Meas = fo_line_def.SolMeasure + fo_line_def.SolMarginAdjust;
					m_fprintf("%%PSI_Export: ");
					export_frametype = REL_DATA(CurrentFrame) t0;
											/* Put out X/Y for UL corner:  */
					export_x = FRAME_DATA(CurrentFrame) left + trim_mark_width;
					export_y = FRAME_DATA(CurrentFrame) top + trim_mark_depth;
											/* Save X/Y for LR corner:  */
					export_end_x = FRAME_DATA(CurrentFrame) right + trim_mark_width;
					export_end_y = FRAME_DATA(CurrentFrame) bottom + trim_mark_depth;
					export_newpiece = 0;	/* Have begun [pp piece for this pg. */

					digi_print(export_x / HorizontalBase);	/* Left of frame */
					digi_print((Imageheight - export_y) / VerticalBase);	/* Top of frame */

					memset (export_name,0,sizeof(export_name));
					scan_pp (export_name);	/* Scan ahead in .fo line to [pp, get &
												return name, reset in .fo. */
					m_fprintf ("%s\n", export_name);
					was_text = 1;
				}

				if (in_overflow)
				{
					cc_mask = 1 & Plates;
					BlackPaint_cc_mask = cc_mask;
					fo_line_def.SolFgColor = 1;
				}
				if ( (FgColor != (int)fo_line_def.SolFgColor) ||
					 (FgShade != (int)fo_line_def.SolFgShade) ||
					 top_of_frame_flag)
				{				/* dump color change if possible - omit if
								   line def color is -1 at top of frame*/
					if ( (top_of_frame_flag &&
						  (fo_line_def.SolFgColor != -1)) ||
						 !top_of_frame_flag)
						color_func((int)fo_line_def.SolFgColor,
								   (int)fo_line_def.SolFgShade);
					else
					{
						FgColor = (int)fo_line_def.SolFgColor;
						FgShade = (int)fo_line_def.SolFgShade;
					}
				}
				xyset();			/* Set start-of-line X-pos, */
				Ypos += fo_line_def.BolLeading; /* Bump Y-pos by line ld */
				jbl = fo_line_def.SolBaseLine; /* SOL base-line shift. */
				if ( (Ypos + fo_line_def.BolLeading) > Imageheight)
				{
					if ( in_overflow)
					{			/* overflow in overflow */
						OverflowOrColorFlag = -1;
						DidPage = 1;
						do_pskmd ('P',"overflow 2+ pages");
#ifdef TRACE
						if (debugger_trace)
							p_info (PI_TRACE, "overflow 2+ pages\n");
#endif
						DidPage = 0;
						beg_PAGE();	/* Gen a begin-page sequence, */
						Ypos = 0;
						Bol_Ypos = 0;
					}
				}				/* end if(Ypos>Imageheight) */
				Ofx = -4000;
				do_pskmd('M',".pgcasdef"); /* Moveto SOL x/y position. */
				vj_para_flag = 0;
				EditTraceStarted = 0;
				if ((EditTraceFlag & 1) && (fo_line_def.MiscLineFlags & 0x40))
				{
					EditTrace_beg_x1 = Xpos;
					EditTraceStarted |= 1;
				}
				if ((EditTraceFlag & 2) && (fo_line_def.MiscLineFlags & 0x80))
				{
					EditTrace_beg_x2 = Xpos;
					EditTraceStarted |= 2;
				}
				ps_drawline();	/* Set this FO line. */

				if (BolLeading > 0)		/* If this line has body leading
											(before expandable EL was added),  */
					vj_pending = 0;		/* then reset vj that was added to v-rules.
											Don't reset on 0-lead lines because they
											don't have v-rule pieces, so we must keep
											vj_pending to extend rules of next line.  */
				Bol_Ypos = Ypos;
				if(vj_para_flag)
				{
					vj_para_flag = 0;
					if(vj_para_adj != 0 && vj_para_cnt != 0)
					{
						vj_amt = vj_para_adj / vj_para_cnt;
						vj_para_adj -= vj_amt;
						vj_para_cnt--;
						Ypos += vj_amt;
						vj_pending = vj_amt;
						if(debugger_trace)
							p_info(PI_TRACE, "adjust paragraph %d\n",vj_amt);
					}
				}
				if( !Ktabmode && !MathStartFlag)
				{
					if(vj_line_adj != 0 && vj_line_cnt != 0)
					{
						vj_amt = vj_line_adj / vj_line_cnt;
						vj_line_adj -= vj_amt;
						vj_line_cnt--;
						Ypos += vj_amt;
						vj_pending += vj_amt;
						if(debugger_trace)
							p_info(PI_TRACE, "adjust line space %d\n",vj_amt);
					}
				}
				if (in_overflow)
					overflow_depth = Ypos;
				top_of_frame_flag = 0; /* No longer line 1 of frame. */
				if (Ktabmode)
					continue;	/* In tab, do FO lines 'til tab-mode is done.*/
				if( !end_forec)
					break;		/* 0 is single FO line */
				if ( (((int)forec * 256) + (int)fowrd + 2) >=
					 ((end_forec * 256) + (end_fowrd)) ||
					 ((int)forec > end_forec) ) /* past last line test */
					break;		/* end, no more lines to process */
			}					/* End for(;;) on each FO line */
			break;
			
		  default:
			p_info(PI_WLOG, "Illegal record type %d will be ignored.\n",
				   PsCurRec -> frame_type);
			break;
		}						/* End switch on XY rec case */
		if ( frame_fg_attribute && !BGOutputFlag)
			frame_overlap();	/* clear text, set up new fg if needed */
    }							/* End for(;;) on XY rec get. */
    return(1);					/* (Will never exit at bottom) */
}								/* End dxy */

/**********************************************************************/
static void xyset(void)
{
    Xpos = Xmark + fo_line_def.LeftIndent + tab_offset + 
		fo_line_def.SolMarginAdjust;
    switch(fo_line_def.Quad)
    {
      case  2:					/* Quad right */
      case  3:					/* Quad center */
		Xpos += fo_line_def.UnusedMeasure;
		break;
    }
}

/**********************************************************************/
static void reopen_tp_if_overflow(void)
{
	uint32 cc_op;

	if (in_overflow)			/* If just did ovf line */
	{
		cc_mask = 1 & Plates;
		Ofx = -4000;
		do_pskmd('M',"reopen TP file"); /* Flush last overflow line */
		m_close(1);				/* close out overflow file */
		cc_op = m_fopen(tpname,"a+",cc_mask); /* re-open TP files. */
		if(cc_op != cc_mask)
		{
			p_info(PI_ELOG, "ERROR - reopening TP file\n");
			return;
		}
		Ypos = last_y;			/* restore y before overflow */
		in_overflow = 0;		/* Turn switch off. */
		Oldfont = -1;
		BlackPaint_cc_mask = cc_mask;
	}							/* end else just did overflow */
}

/**********************************************************************/
static void xyrulebox(void)
{

    Rwei = (float) RuleWt / 20;	/* Weight comes in 20ths, convert to points */
	Fx = SetX;
    Fy = SetY;
	Fx2 = Fx + RuleWid;
	Fy3 = Fy + RuleHgt;

	bg_color_flag = 1;
    switch ( RuleType)
    {							/* rule or box to draw. */
      case 0:					/* Draw Vertical Line. */
		Fx2 = Fx + (Rwei * HorizontalBase);
		rtype='v';
 		do_pskmd('F',"xyrboxvert");
		break;
      case 1:					/* Draw Horizontal Line. */
		Fy3 = Fy + (Rwei * VerticalBase);
		rtype='h';
		do_pskmd('F',"xyrboxhoriz");
		break;
      case 2:					/* Filled Box */
		Rwei = 0;               /* not defined for filled box */
		do_pskmd('V',"xyrbox-fbox");
		break;
      default:
		p_info(PI_ELOG, "ERROR - xyrulebox record type %d unknown.\n", RuleType);
		break;				
    }
	bg_color_flag = 0;
}								/* End xyrulebox */

/**********************************************************************/
static void frame_color(int box_flag)
{
	ELEMENT *ele;
	int i, j;
	uint32 vlist_plates, cc_mask_hold, loop_temp, BlackPaint_cc_mask_sav;
	int frame_trap_color;
	int frame_trap_shade;
	int np_from_ele_save;
	int frame_trap_offset_flag;
	int temp_frame_trap_color, sv;
	float frame_trap_weight, frame_trap_amt;
	float save_fx, save_fx2, save_fy, save_fy3;
	float fx_hold, fy_hold, fx2_hold, fy3_hold;
	float ly_x_offset, ly_y_offset;
	float x_radius, y_radius;
	float arctan_angle;
	DRAW_POINT_X_Y *lp_save;
	char ly_scale_msg[32] = {0,0};
	int blend_bg_flag = 0;

/* ---------- set up mask to look for colors ---------- */

	if (spot_pass == 1)
		cc_mask = 1;
	else
		cc_mask = cc_hit;
	BgColor = color_check(BgClr, -1);
	if ( (BgColor == BgClr) && (BgColor != -1) )
		BgShade = BgShd;
	else
		BgShade = 100;
	NewBlendColor = -1;
	if ( (BgBlendStartClr > 0) || (BgBlendEndClr > 0) )
	{							/* frame blend, use these values if defined */
		int start_clr, end_clr;

		start_clr = color_check(BgBlendStartClr, -1);
		end_clr = color_check(BgBlendEndClr, -1);
		if ( (start_clr > -1) && (end_clr > -1) )
		{						/* blend defined */
			blend_bg_flag = 1;
			BgColor = start_clr;
			BgClr = BgBlendStartClr;
			BgShade = BgBlendStartShd;
			NewBlendColor = end_clr;
			BlendEndColor = end_clr;
			NewBgColor = BgBlendStartClr;
			if (spot_pass == 1)
				set_pass_2_color(BgBlendEndClr, &cc_hit, 2);
			BfBgShadeStart = BgBlendStartShd / 100.;
			BlendEndShade = BgBlendEndShd / 100.;
			AnchorCount = 0;
			CornerRadiusBfFlag = 2;
		}
		else if (spot_pass == 1)
			set_pass_2_color(BgBlendEndClr, &cc_hit, 2);
	}
	FgColor = color_check(FgClr, -1);
	if ( (FgColor == FgClr) && (FgColor != -1) )
		FgShade = FgShd;
	else
		FgShade = 100;

/* ---------- set up mask for output of bg color ---------- */

	vlist_plates = 0;			/* initialize for no background */
	if (spot_pass == 1)
	{
		if (KeyOutputType)		/* set if needed */
		{
			set_pass_2_color(BgClr, &cc_hit, 2); /* for second pass */
			set_pass_2_color(FgClr, &cc_hit, 2);
		}
		if (BgColor >= 0)
			vlist_plates = 1;	/* for background */
	}							/* end if(spot_pass==1) */
	else
	{
		set_pass_2_color(BgColor, &vlist_plates, 2); /* spot_pass==2 */
		set_pass_2_color(FgColor, &vlist_plates, 2);
	}
	cc_mask = vlist_plates & Plates; /* set mask for possible
										background colors */

/* ---------- set up bounding box, Fx thru Fy3 for each element ---------- */

	for(j = 1, ele = FRAME_DATA(CurrentFrame) ele; ele; j++, ele = ele -> next)
	{
		if (j == CurrentEle)
		{
			if (!exported ||	/* If not yet in a [pp/[pq export, or  */
				!export_end_y)	/*  in one which isn't floating ill/table:  */
				export_x = FRAME_DATA(CurrentFrame) left + trim_mark_width;
			if (PsCurRec -> frame_type == 8)
			{					/* graphic */
				lp = FRAME_DATA(CurrentFrame) list_points;
				np_from_ele = FRAME_DATA(CurrentFrame) n_points;
				Fx = FRAME_DATA(CurrentFrame) left + trim_mark_width;
				Fy = FRAME_DATA(CurrentFrame) top + trim_mark_depth;
				Fx2 = FRAME_DATA(CurrentFrame) right + trim_mark_width;
				Fy3 = FRAME_DATA(CurrentFrame) bottom + trim_mark_depth;
			}
			else
			{					/* text, rule/box */
				lp = ele ->list_points;
				np_from_ele = ele -> n_points;
				Fx = ele -> rect_left + trim_mark_width;
				Fy = ele -> rect_top + trim_mark_depth;
				Fx2 = ele -> rect_right + trim_mark_width;
				Fy3 = ele ->rect_bottom + trim_mark_depth;
			}
			RoundCornerRadiusTL =
				(float)lmt_off_to_abs(wn,X_REF,TL_ROUNDED(CurrentFrame)) /
					HorizontalBase;
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "bg= %d, fg= %d, Fx= %.2f, Fy= %.2f, Fx2= %.2f, Fy3= %.2f,rel= %d, ele= %d, corner= %f\n",
					   BgColor,FgColor,Fx,Fy,Fx2,Fy3,CurrentFrame,CurrentEle,
					   RoundCornerRadiusTL);
#endif
			fx_hold = Fx;
			fy_hold = Fy;
			fx2_hold = Fx2;
			fy3_hold = Fy3;
			if (PsCurRec -> frame_type == 8)
			{					/* graphic, find the bb */
				bb_left = FRAME_DATA(CurrentFrame) rot_top_left.x;
				if (bb_left > FRAME_DATA(CurrentFrame) rot_top_right.x)
					bb_left  =  FRAME_DATA(CurrentFrame) rot_top_right.x;
				if (bb_left > FRAME_DATA(CurrentFrame) rot_bottom_left.x)
					bb_left = FRAME_DATA(CurrentFrame) rot_bottom_left.x;
				if (bb_left > FRAME_DATA(CurrentFrame) rot_bottom_right.x)
					bb_left = FRAME_DATA(CurrentFrame) rot_bottom_right.x;
				bb_right = FRAME_DATA(CurrentFrame) rot_top_left.x;
				if (bb_right < FRAME_DATA(CurrentFrame) rot_top_right.x)
					bb_right  =  FRAME_DATA(CurrentFrame) rot_top_right.x;
				if (bb_right < FRAME_DATA(CurrentFrame) rot_bottom_left.x)
					bb_right = FRAME_DATA(CurrentFrame) rot_bottom_left.x;
				if (bb_right < FRAME_DATA(CurrentFrame) rot_bottom_right.x)
					bb_right = FRAME_DATA(CurrentFrame) rot_bottom_right.x;
				bb_top = FRAME_DATA(CurrentFrame) rot_top_left.y;
				if (bb_top > FRAME_DATA(CurrentFrame) rot_top_right.y)
					bb_top  =  FRAME_DATA(CurrentFrame) rot_top_right.y;
				if (bb_top > FRAME_DATA(CurrentFrame) rot_bottom_left.y)
					bb_top = FRAME_DATA(CurrentFrame) rot_bottom_left.y;
				if (bb_top > FRAME_DATA(CurrentFrame) rot_bottom_right.y)
					bb_top = FRAME_DATA(CurrentFrame) rot_bottom_right.y;
				bb_bottom = FRAME_DATA(CurrentFrame) rot_top_left.y;
				if (bb_bottom > FRAME_DATA(CurrentFrame) rot_top_right.y)
					bb_bottom  =  FRAME_DATA(CurrentFrame) rot_top_right.y;
				if (bb_bottom > FRAME_DATA(CurrentFrame) rot_bottom_left.y)
					bb_bottom = FRAME_DATA(CurrentFrame) rot_bottom_left.y;
				if (bb_bottom > FRAME_DATA(CurrentFrame) rot_bottom_right.y)
					bb_bottom = FRAME_DATA(CurrentFrame) rot_bottom_right.y;
			}
			else
			{					/* text, rule/box */
				bb_left = ele -> rot_rect_left;
				bb_top = ele -> rot_rect_top;
				bb_right = ele->rot_rect_right;
				bb_bottom = ele->rot_rect_bottom;
			}
			bb_left += trim_mark_width;
			bb_right += trim_mark_width;
			bb_top += trim_mark_depth;
			bb_bottom += trim_mark_depth;
			if ( (bg_color_flag || box_flag) && !LYPrintFlag)
			{					/* output bg color, if needed */
				if ( (( BGOutputFlag && !frame_layer_flag) ||
					  ( !BGOutputFlag && frame_layer_flag)) && !repeat_rel)
				{
					if ( (BgColor >= 0) && !box_flag)
					{
						if ( FRAME_FLAGS(CurrentFrame) & OVALE_SHAPE)
						{
							CircleFlag = 1;
#ifdef TRACE
							if (debugger_trace)
								p_info(PI_TRACE, "BACKGROUND TINT CIRCLE\n");
#endif
						}
						if ( !blend_bg_flag)
							do_pskmd('V',"frame background"); /* background */
						else
						{
							if (BlendAngle < 0)
							{
								arctan_angle = 572.957795 *	/*1 rad  1/10 deg */
									atan((Fx2 - Fx) / ((((double)Fy3 - Fy) *
														HorizontalBase) /
													   VerticalBase));
								if (BlendAngle == -1)
									BlendAngle = 3600. - arctan_angle; /*TL-BR*/
								else if (BlendAngle == -2)
									BlendAngle = arctan_angle + 1800.; /*TR-BL*/
								else if (BlendAngle == -3)
									BlendAngle = -3;       /* C */
								else if (BlendAngle == -4)
									BlendAngle = -4;       /* O */
							}	/* end if(BlendAngle<0) */
							if (BlendAngle >= 0)
							{
								DegAngle = (float)BlendAngle / 10; /* degrees */
								RadAngle = DegAngle * .0174532925; /* radians */
								SinAngle = sin(RadAngle);
								if ( SinAngle < 0)
									SinAngle *= -1.;
								CosAngle = cos(RadAngle);
								if ( CosAngle < 0)
									CosAngle *= -1.;
							}
							BfClipFx = Fx;
							BfClipFy = Fy;
							BfClipFx2 = Fx2;
							BfClipFy3 = Fy3;
							if (BfClipFy == BfClipFy3)
								BfClipFy3 += VerticalBase; /* make it 1 point */
							do_pskmd('B',"frame background"); /* background */
							CornerRadiusBfFlag = 0;
						}
						CircleFlag = 0;
					}
					if ( (spot_pass == 1) && ((FgColor >= 0) ||
											  (BgColor >= 0)) )
					{
						add_to_vlist(bb_left, bb_top, bb_right, bb_bottom,
									 FgColor, FgShade, BgColor, BgShade,
									 vlist_plates, 0);
						if ( blend_bg_flag)
							add_to_vlist(bb_left, bb_top, bb_right, bb_bottom,
										 BgColor, BgShade, BgBlendEndClr, 
										 BlendEndShade, vlist_plates, 0);
					}
				}
				else
				{				/* not bg output pass, no bg output */
					Fx /= HorizontalBase;
					Fx2 /= HorizontalBase;
					Fy = (Imageheight - Fy) / VerticalBase;
					Fy3 = (Imageheight - Fy3) / VerticalBase;
				}
			}

/* ---------- do layout drawing and printing, if requested ---------- */

			if ( LYPrintFlag & 4)
			{					/* drawing layouts */
				cc_mask = 1;
				beg_PAGE();
				if ( !ly_page_started_flag)
				{				/* print trim and page rectangles */
					Ofx = -4000;
					Xpos = Ypos = 0;
					do_pskmd('m', "Layout Print");
					vid_color(1, 100, 1);
					if (KeyScale == -1)
						strcpy (ly_scale_msg, "SCALED TO FIT");
					else if ( (KeyScale > 0) || (KeyScaleX > 0) ||
							  (KeyScaleY > 0))
					{
						if (KeyScale > 0)
							sprintf (ly_scale_msg,
									 "WIDTH, DEPTH SCALED TO %d%%",KeyScale);
						else
						{
							if ( KeyScaleX <= 0)
								KeyScaleX = 100;
							if ( KeyScaleY <= 0)
								KeyScaleY = 100;
							sprintf (ly_scale_msg,
									 "WIDTH SCALED TO %d%%, DEPTH SCALED TO %d%%",
									 KeyScaleX,KeyScaleY);
						}
					}
					else
						strcpy (ly_scale_msg, "WIDTH, DEPTH SCALED TO 100%");
					if ( MasterNameFlag)
					{
						m_fprintf("8 /Courier F %d %d M (MASTER '%s' FOR /%s/%s   %s)S \n",
								  0, PageH - header_offset, LayoutName,
								  TreeName,SubDirName, ly_scale_msg);
					}
					else
					{
						m_fprintf("8 /Courier F %d %d M (PAGE %d '%s' FOR /%s/%s   %s)S \n",
								  0, PageH - header_offset, PageNo1,
								  LayoutName,TreeName,SubDirName,ly_scale_msg);
					}
					save_fx = Fx;
					save_fx2 = Fx2;
					save_fy = Fy;
					save_fy3 = Fy3;
					Fx = 0;
					Fy = Imageheight / VerticalBase;
					Fx2 = Fx + ((float)lmt_off_to_abs(wn, X_REF, TRIM_WIDTH) /
								HorizontalBase);
					Fy3 = Fy - ((float)lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH) /
								VerticalBase);
					m_fprintf("GS [2] 0 setdash 1 setlinewidth\n");
					drule_sub(1); /* stroke the trim size with dash 2 */
					m_fprintf("GR\n");
					ly_x_offset = (float)lmt_off_to_abs(wn,X_REF,X_PG_ORIGIN);
					ly_y_offset = (float)lmt_off_to_abs(wn,Y_REF,Y_PG_ORIGIN);
					Fx = Fx + (ly_x_offset / HorizontalBase);
					Fy -= (ly_y_offset / VerticalBase);
					Fx2 = Fx + (((float)lmt_off_to_abs(wn,X_REF,PAGE_WIDTH) ) /
								HorizontalBase);
					Fy3 = Fy - ((float)lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH) /
								VerticalBase);
					m_fprintf("GS [9] 0 setdash 1 setlinewidth\n");
					drule_sub(1); /* stroke the page size with dash 9 */
					m_fprintf("GR\n");
					Fx = save_fx;
					Fx2 = save_fx2;
					Fy = save_fy;
					Fy3 = save_fy3;
					ly_page_started_flag = 1;
				}				/* end printing trim and page rectangles */
				vid_color(1, 100, 1);
				m_fprintf("GS 1 setlinewidth\n");
				if ( FRAME_FLAGS(CurrentFrame) & OVALE_SHAPE)
				{
					CircleFlag = 1;
#ifdef TRACE
					if (debugger_trace)
						p_info(PI_TRACE, "LAY FRAME CIRCLE\n");
#endif
					save_fx = Fx;
					save_fx2 = Fx2;
					save_fy = Fy;
					save_fy3 = Fy3;
					Fx /= HorizontalBase;
					Fx2 /= HorizontalBase;
					Fy = (Imageheight - Fy) / VerticalBase;
					Fy3 = (Imageheight - Fy3) / VerticalBase;
					digi_print(x_radius = ((Fx - Fx2)/2)); /* x radius */
					digi_print(y_radius = ((Fy - Fy3)/2)); /* y radius */
					digi_print(Fx + x_radius); /* x center */
					digi_print(Fy + y_radius); /* y center */
					if ( !odashwid && !ogapwid)
						m_fprintf("[] 0 setdash\n");
					else
						m_fprintf("[%d %d] 0 setdash\n", odashwid, ogapwid);

					m_fprintf("ES stroke\n");
					CircleFlag = 0;
					Fx = save_fx;
					Fx2 = save_fx2;
					Fy = save_fy;
					Fy3 = save_fy3;
				}
				else
					dbox(1, 0);
				if ( (PsCurRec -> frame_type != 4) ||
					 ((PsCurRec -> frame_type == 4) && ((RuleType == 2) &&
														 (CurrentEle == 1))))
				{
					output_ele_xy( (float)(ele->rect_left +
										   (4 * HorizontalBase)),
								   (float)(ele->rect_top +
										   (8 * VerticalBase)) );
					m_fprintf(" M\n");
					m_fprintf("(%d)S\n",CurrentFrame);
				}
				m_fprintf("GR\n");
			}					/* end if(LYPrintFlag&4) */
			if ( LYPrintFlag & 0x8)
			{					/* printing layout data */
				cc_mask = 1;
				if ( ly_page_started_flag < 2)
					mpu_layout_print(0, 0);	/* print page ident */
				mpu_layout_print(CurrentFrame, ele);
			}
			if ( (LYPrintFlag & 0x10) && !tx_page_started_flag &&
				 (PsCurRec -> frame_type == 7) &&
				 (PsCurRec -> elem) )
				mpu_textfile_print (0);	/* print page ident */
			if ( (LYPrintFlag & 0x10) && (PsCurRec -> frame_type == 7) &&
				 (PsCurRec -> elem) )
			{					/* printing tx file */
				cc_mask = 1;
				mpu_textfile_print (1); /* print the tx file */
			}
			if (LYPrintFlag)
			{
				frame_layer_flag = 0; /* remove layering, opaque/clear bits */
				frame_fg_attribute = 0;
			}

/* ---------- set up frame trap, if requested ---------- */

			if ( (FRAME_FLAGS(CurrentFrame) & OUTLINE) && !LYPrintFlag)
			{					/* trap requested */
				if ( ((((( BGOutputFlag && !frame_layer_flag) ||
					  ( !BGOutputFlag && frame_layer_flag))) && !repeat_rel) ||
					 ( PsCurRec -> frame_type == 4)) && !delay_trap_flag)
				{				/* time to output it if available */
					frame_trap_offset_flag = 0;
					frame_trap_color = OUT_COLOR(CurrentFrame); 
					frame_trap_shade = OUT_SHADE(CurrentFrame); 
					sv = wn -> yx_convert[0];
					wn -> yx_convert[0] = 270; /* 20ths of a point */
                    frame_trap_weight =
						lmt_off_to_abs(wn, 0, OUT_WEIGHT(CurrentFrame));
                    frame_trap_amt =
						lmt_off_to_abs(wn, 0, OUT_TRAP(CurrentFrame));
					if ( frame_trap_weight != (frame_trap_amt * 2) )
						frame_trap_offset_flag = 1;
                    frame_trap_weight /= 20.;
					wn -> yx_convert[0] = sv;
					if (spot_pass == 1)
						cc_mask = 1 & Plates;
					else		/* spot_pass = 2 */
						cc_mask = ~1 & Plates;
					temp_frame_trap_color = color_check(frame_trap_color, -1);
#ifdef TRACE
					if (debugger_trace)
						p_info(PI_TRACE, "OUT_CLR= %d, WT= %3.3f, rel= %d, ele= %d, flag= %d\n",
							   frame_trap_color, frame_trap_weight,
							   CurrentFrame, CurrentEle,
							   frame_trap_offset_flag);
#endif
					vlist_plates = 0;
					set_pass_2_color(frame_trap_color,&vlist_plates,2);
					if (spot_pass == 1)
					{
						if (KeyOutputType)
							cc_hit |= ((vlist_plates & ~1) & Plates);
						if (temp_frame_trap_color < 0)
							cc_mask = 0;
						else
							cc_mask = 1 & Plates;
					}
					else		/* spot_pass = 2 */
						cc_mask = vlist_plates & Plates;
					if ( cc_mask)
					{			/* output the frame outline trap */
						cc_mask_hold = cc_mask;
						for (i=0; i<MAX_CLRS-1; i++)
						{
							cc_mask = ( loop_temp = (1 << i) ) & cc_mask_hold;
							if(loop_temp > cc_mask_hold)
								break;
							if (!cc_mask)
								continue;
							if (find_color(frame_trap_color, i+1) )
							{	/* found the color structure */
								BlackPaint_cc_mask_sav =
									BlackPaint_cc_mask;
								BlackPaint_cc_mask = 0;
								vid_color(frame_trap_color, frame_trap_shade,
										  i+1);
								m_fprintf("GS newpath %3.3f setlinewidth\n",
										  frame_trap_weight);
								PaintOnPage |= BlackPaint_cc_mask;
								BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
								lp_save = lp;
								np_from_ele_save = np_from_ele;
								if ( ele -> trap_pts &&
									 frame_trap_offset_flag &&
									 !(FRAME_FLAGS(CurrentFrame) &OVALE_SHAPE)
									 && !RoundCornerRadiusTL)
								{
									lp = ele -> trap_points;
									np_from_ele = ele -> trap_pts;
								}
								if ( FRAME_FLAGS(CurrentFrame) & OVALE_SHAPE)
								{
									CircleFlag = 1;
#ifdef TRACE
									if (debugger_trace)
										p_info(PI_TRACE, "OUTLINE TRAP CIRCLE\n");
#endif
									save_fx = Fx;
									save_fx2 = Fx2;
									save_fy = Fy;
									save_fy3 = Fy3;
									if (PsCurRec -> frame_type == 8)
									{ /* graphic */
										Fx = FRAME_DATA(CurrentFrame) left +
											trim_mark_width;
										Fy = FRAME_DATA(CurrentFrame) top +
											trim_mark_depth;
										Fx2 = FRAME_DATA(CurrentFrame) right +
											trim_mark_width;
										Fy3 = FRAME_DATA(CurrentFrame) bottom +
											trim_mark_depth;
									}
									else
									{ /* text, rule/box */
										Fx = ele -> rect_left + trim_mark_width;
										Fy = ele -> rect_top + trim_mark_depth;
										Fx2 = ele -> rect_right +
											trim_mark_width;
										Fy3 = ele ->rect_bottom +
											trim_mark_depth;
									}
									Fx /= HorizontalBase;
									Fx2 /= HorizontalBase;
									Fy = (Imageheight - Fy) / VerticalBase;
									Fy3 = (Imageheight - Fy3) / VerticalBase;
									digi_print(x_radius =
											   ((Fx2 - Fx)/2)); /* x radius */
									digi_print(y_radius =
											   ((Fy - Fy3)/2)); /* y radius */
									digi_print(Fx + x_radius); /* x center */
									digi_print(Fy3 + y_radius); /* y center */
									if (odashwid == 0 && ogapwid == 0)
										m_fprintf("[] 0 setdash\n");
									else
										m_fprintf("[%d %d] 0 setdash\n", odashwid, ogapwid);
									m_fprintf("ES stroke\n");
									CircleFlag = 0;
									Fx = save_fx;
									Fx2 = save_fx2;
									Fy = save_fy;
									Fy3 = save_fy3;
								}
								else if ( RoundCornerRadiusTL)
								{
									Fx = fx_hold / HorizontalBase;
									Fy = (Imageheight - fy_hold) /
										VerticalBase;
									Fx2 = fx2_hold / HorizontalBase;
									Fy3 = (Imageheight - fy3_hold) /
										VerticalBase;
									dbox(1, 0);
								}
								else
									dbox(1, 0);
								lp = lp_save;
								np_from_ele = np_from_ele_save;
								m_fprintf("GR\n"); /* restore */
							}
						}		/* end for (i=0;i<MAX_CLRS-1;i++) */
						cc_mask = cc_mask_hold;
					}
				}
			}					/* end frame trap */
			bg_color_flag = 0;

/* ---------- set up mask for fg ---------- */

			if (!delay_bg_flag)
			{
				if ( box_flag)
					return;
				vlist_plates = 0;
				set_pass_2_color(FgColor, &vlist_plates, 1);
				if (spot_pass == 1)
				{
					if (KeyOutputType)
						cc_hit |= (vlist_plates & ~1) & Plates;
					if (FgColor < 0)
						cc_mask = 0; /* no fg output if no fg in plate 1 */
					else
						cc_mask = vlist_plates &1 & Plates; /* allow output if fg
															   color in plate 1 */
				}
				else
				{				/* spot_pass==2 */
					vlist_plates &= (Plates & ~1);
					cc_mask = vlist_plates & Plates;
					if ( !cc_mask)
						FgColor = -1;
				}
				BlackPaint_cc_mask = 0;
				cc_mask_hold = cc_mask;
				for (i=0; i<MAX_CLRS-1; i++)
				{
					cc_mask = ( loop_temp = (1 << i) ) & cc_mask_hold;
					if(loop_temp > cc_mask_hold)
						break;
					if (!cc_mask)
						continue;
					vid_color(FgClr, FgShd, i+1);
				}
				cc_mask = cc_mask_hold;
				return;
			}
		}						/* end if (j==CurrentEle) */
	}							/* end for(ele=FRAME_DATA(CurrentFrame)ele..) */
	bg_color_flag = 0;
}

/**********************************************************************/
void set_pass_2_color(int color, uint32 *mask_pointer, uint32 start_plate)
{
    struct clr_lst *tp;			/* list-walking pointer */
	uint k;
	
	if ( color >= 0 )
	{
		for (k=start_plate; k<MAX_CLRS; k++) /* k is the plate number */
		{
			if ( (k == 1)  && !KeyOutputType )
				tp = clr_1st[0];
			else
				tp = clr_1st[k];
			while(tp != 0)		/* we're looking for the end  */
			{
				if(tp->color == color) /* hey, we've seen this one! */
				{				/* add a plate to the hit list */
					*mask_pointer |= (1 << (k - 1));
					break;		/* look for other plates */
				}
				tp = tp->nxt;	/* move to next one */
			}					/* end while(tp != 0) */
		}						/* end for(k=start_plate; k<32; k++) */
		if ( start_plate > 1)
			*mask_pointer &= Plates;
	}							/* end if(color>=0) */
}

/**********************************************************************/
static void frame_overlap(void)
{
	int i;
	DRAW_POINT_X_Y *lp_temp;
	float save_fx, save_fy;
	uint32 cc_mask_save;

	if (spot_pass == 1)
	{
		cc_mask = 1 & Plates;
		if ( KeyOutputType)
			current_vid_box_plate = 1;
		else
			current_vid_box_plate = 0;
		current_vid_box =
			vlist_intersect(current_vid_box_plate); /* plate 0 or 1 list */
	}
	else
	{							/* look at list for plates 2-31 */
		cc_mask = cc_hit;
		for (i=current_vid_box_plate; i<MAX_CLRS; i++)
		{
			current_vid_box = vlist_intersect(i);
			if (current_vid_box)
				break;
		}
	}
	if (current_vid_box)
	{
		if (PsCurRec -> frame_type == 4)
		{
			FgClr = current_vid_box -> bg_color;
			FgShd = current_vid_box -> bg_shade;
			BgClr = current_vid_box -> fg_color;
			BgShd = current_vid_box -> fg_shade;
		}
		else
		{
			FgClr = current_vid_box -> fg_color;
			FgShd = current_vid_box -> fg_shade;
			BgClr = current_vid_box -> bg_color;
			BgShd = current_vid_box -> bg_shade;
		}
		if ( spot_pass == 1)
		{
			if (FgClr >= 0)
				cc_mask = 1 & Plates;
			else
				cc_mask = 0;
		}
		else
		{						/* spot_pass==2 */
			cc_mask = 0;
			set_pass_2_color(FgClr, &cc_mask, 2); 
			set_pass_2_color(BgClr, &cc_mask, 2); 
			cc_mask &= ~1;
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "OVERLAP frame %d\n", current_vid_box -> rel_index);
#endif
		}
	}							/* end if(current_vid_box) */
	cc_mask_save = cc_mask;		/* for when transparent frame is restarted */
	if (spot_pass == 1)			/* set up cc_mask to output this stuff */
		cc_mask = 1 & Plates;
	else
		cc_mask = cc_hit;
	if ( !current_vid_box)
	{							/* no more frames overlap, reset */
		if (started_overlap_flag)
		{
			Ofx = -4000;
			do_pskmd('M',"reset at no more overlap");
			m_fprintf("GR\n");
		}		
		if (spot_pass != 1)
			current_vid_box_plate = 2;
		started_overlap_flag = 0;
		cc_mask = cc_mask_save;
		return;
	}
	if (started_overlap_flag)
	{
		Ofx = -4000;
		do_pskmd('M',"reset at end overlap");
		m_fprintf("GR\n");
	}

/* find the element, do gsave, output clip path, set up the new foreground */

	started_overlap_flag++;
	m_fprintf(" GS \n");		/* clip to this area */
	save_fx = Fx;
	save_fy = Fy;
	Fx = PageRotationX / HorizontalBase;
	Fy = (Imageheight - PageRotationY) / VerticalBase;
	if (PageRotationAngle)
		change_rotation(-PageRotationAngle, Fx, Fy); /* unrotate */
	if (current_vid_box -> page_rotation_angle)
		change_rotation(-current_vid_box -> page_rotation_angle,
						current_vid_box -> rot_x,
						current_vid_box -> rot_y); /* rotate clip */
	m_fprintf("newpath\n");		/* clip to this area */
	lp_temp = current_vid_box -> elem ->list_points;
	if (current_vid_box -> cmd_bf_flag)
	{							/* use cmd bf outline for clip */
		output_ele_xy(current_vid_box -> x, current_vid_box -> y);
		m_fprintf("M ");		/* move to the UL corner */
		output_ele_xy(current_vid_box -> x1, current_vid_box -> y);
		m_fprintf("L ");		/* move to the next spot corner */
		output_ele_xy(current_vid_box -> x1, current_vid_box -> y1);
		m_fprintf("L ");		/* move to the next spot corner */
		output_ele_xy(current_vid_box -> x, current_vid_box -> y1);
		m_fprintf("L ");		/* move to the next spot corner */
	}
	else
	{							/* use frame outline for clip */
		for (i=0; i<current_vid_box -> elem -> n_points; i++)
		{						/* output the frame points for bg */
			output_ele_xy((float)lp_temp -> x, (float)lp_temp ->y);
			if (i)
				m_fprintf("L "); /* move to the next spot corner */
			else
				m_fprintf("M "); /* move to the UL corner */
			if ( (i & 3) == 3 )
			{
				m_fprintf("\n"); /* up to 4 moves on a line */
			}
			lp_temp++;
		}						/* end for(i=0;i<elem->n_points;i++) */
	}							/* end else use frame outline */
	m_fprintf("clip\n");
	if (current_vid_box -> page_rotation_angle)
		change_rotation(current_vid_box -> page_rotation_angle,
						current_vid_box -> rot_x,
						current_vid_box -> rot_y); /* unrotate clip area */
	if (PageRotationAngle)
		change_rotation(PageRotationAngle, Fx, Fy); /* restore */
	Fx = save_fx;
	Fy = save_fy;
	cc_mask = cc_mask_save;
}								/* end function */

/**********************************************************************/
static void change_rotation(int16 degrees, float x_center, float y_center)
{
	digi_print( (float)degrees);
	digi_print(x_center);
	digi_print(y_center);
	m_fprintf("RXY\n");
}

/**********************************************************************/
void output_ele_xy(float x, float y)
{						  
	float xout, yout;
	
	xout = (x + trim_mark_width) / HorizontalBase;
	yout = (Imageheight - y - trim_mark_depth) / VerticalBase;
	digi_print(xout);	
	digi_print(yout);
}

/**********************************************************************/
void ps_set_rotation(void)
{
	
	find_mu_rot(CurrentFrame);
	if( (mu_rot_degree == -PageRotationAngle) &&
		(mu_rot_x == PageRotationX) &&
		(mu_rot_y == PageRotationY))
		return;
/* new rotation, output it */
	cc_mask = 1 & Plates;
	if (spot_pass == 2)
		cc_mask = cc_hit;
	reopen_tp_if_overflow();
	if(PageRotationAngle)
	{							/* unrotate */
		Rotate = -PageRotationAngle;
		Fx = PageRotationX / HorizontalBase;
		Fy = (Imageheight - PageRotationY) / VerticalBase;
		if ( !LYPrintFlag || ( LYPrintFlag & 4) )
			do_pskmd('X',"unrotate frame");
		PageRotationAngle = 0;
	}							/* end if(PageRotationAngle) */
	if( mu_rot_degree)
	{							/* output if a new angle */
		PageRotationAngle = -mu_rot_degree;
		PageRotationX = mu_rot_x;
		PageRotationY = mu_rot_y;
		Rotate = PageRotationAngle;
		Fx = PageRotationX / HorizontalBase;
		Fy = (Imageheight - PageRotationY) / VerticalBase;
		if ( !LYPrintFlag || ( LYPrintFlag & 4) )
			do_pskmd('X',"rotate frame");
	}
}

/***********************************************************************/
/*ARGSUSED*/
static void sig_usr1(int sig)
{
	SigUsr1 = 1;
}

/***********************************************************************/
/*ARGSUSED*/
static void sig_alrm(int sig)
{
	SigAlrm = 1;
}

/**********************************************************************/
static int do_HandJ(char *filename, int fo_or_lfo_flag)
{
	static int first_signal = 0;
	char buf[LPM_MAXMESG];
	char *cp;
	int rc = 0;
	int fid = 0;

	if ( !first_signal)
	{
#if 1
		struct sigaction act;

		act.sa_flags = SA_RESTART;
		act.sa_handler = sig_usr1;
		sigemptyset (&act.sa_mask);
		sigaddset (&act.sa_mask, SIGALRM);
		if (sigaction (SIGUSR1, &act, NULL) == -1)
		{
			p_info(PI_ELOG, " PP could not set SIGUSR1.\n");
			rc = 1;
		}

		act.sa_flags = SA_RESTART;
		act.sa_handler = sig_alrm;
		sigemptyset (&act.sa_mask);
		sigaddset (&act.sa_mask, SIGUSR1);
		if (sigaction (SIGALRM, &act, NULL) == -1)
		{
			p_info(PI_ELOG, " PP could not set SIGALRM1.\n");
			rc = 1;
		}
#else
		if( berk_signal( SIGUSR1, (SignalHandler)sig_usr1) == (SignalHandler)SIG_ERR)
		{
			p_info(PI_ELOG, " PP could not set SIGUSR1.\n");
			rc = 1;
		}
		if( berk_signal( SIGALRM, (SignalHandler)sig_alrm) == (SignalHandler)SIG_ERR)
		{
			p_info(PI_ELOG, " PP could not set SIGALRM1.\n");
			rc = 1;
		}
#endif
		if ( rc)
			return(-1);
		else
			first_signal = 1;
	}
	if ( !fo_or_lfo_flag)		/* 0 = .fo, non-zero = .lfo */
	{
		if (LockFlag)
		{
			LcErrMsg[0] = lpm_errmsg[0] = 0;
			fid = lc_facs_req (TreeName, TEXT_FILE, SubDirName, filename,
							   0, LC_FACS_ABS_NOTIFY_LOCK);
			if (!fid)
			{
				rc = 0;
				*buf = 0;
				if (*lpm_errmsg)
					rc = sprintf (buf, ": %s", lpm_errmsg);
				if (*LcErrMsg)
					sprintf (buf + rc, ": %s", LcErrMsg);
				p_info(PI_ELOG, "pp: couldn't lock %s%s\n", filename, buf);
				HNJ_LockFailureFlag = 1;
				return (-1);
			}
		}
		sprintf( buf, "Tree %s Dir %s File %s", TreeName,
				 SubDirName, filename);
	}
	else
		sprintf( buf, "Tree %s Dir %s Layout %s", TreeName,
				 SubDirName, filename);
#ifdef TRACE
	if (debugger_trace)
	{
		p_info(PI_TRACE, "pp: starting file '%s' type %d, thru HandJ\n",
				  filename, fo_or_lfo_flag);
		p_info(PI_TRACE, "pp: HandJ: <%s>\n", buf);
	}
#endif
	lpm_errmsg[0] = 0;
	SigUsr1 = 0;
	if( !lpm_que( "HandJ", buf))
	{
		if (fid)
			lc_facs_free_id (TreeName, fid);
		p_info(PI_ELOG, "pp: lpm_que: %s\n", lpm_errmsg);
		return(-1);				/* trouble with HandJ */
	}
	if( !SigUsr1)
	{
		for( ;;)
		{
			alarm(30);
			pause();
			alarm(0);
			if( SigUsr1)
				break;
			p_info(PI_WLOG, "pp: HandJ '%s' type %d - response timing out.\n", 
				  filename, fo_or_lfo_flag);
		}
	}
	rc = -1;
	cp = lpm_getmsg();
	if( !cp)
		p_info(PI_ELOG, 
	   "pp: Could not HandJ '%s' type %d - couldn't get lpm return message.\n",
				  filename, fo_or_lfo_flag);
	else if( !lpm_retval( &rc, cp, "HandJ"))
		p_info(PI_ELOG, 
		"pp: Could not HandJ '%s' type %d - invalid lpm return message.\n",
				  filename, fo_or_lfo_flag);
	else if( rc != 0)
	{
		p_info(PI_ELOG, "pp: Could not HandJ '%s' type %d - HandJ error %d.\n",
				  filename, fo_or_lfo_flag, rc);
		rc = -1;
	}
	else
		rc = 0;
#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "pp: Completed HandJ for file '%s', type %d.\n",
			   filename, fo_or_lfo_flag);
#endif
	if (fid)
		lc_facs_free_id (TreeName, fid);
	return(rc);
}

/***********************************************************************
 **  SCAN_PP    Scan this FO line until command -64/-20 (PP: Begin EPS extract) **
 ***********************************************************************/

static void scan_pp(char *export_name)
{
    int16 fochar, kcmd, j, argcount, not_in_tab, end2;
	int16 forec2, fowrd2, nchar;
	int err;
	char err_buf[128];
	LDEF temp_fo_line_def;
/* Constant int16 list "foreads[100]" and "foreadsTOP" are defined externally in pstabs.c:tscan. */

    forec2 = forec;				/* Store current .fo rec & word positions.  */
    fowrd2 = fowrd;
    not_in_tab = 1;				/* Start out not in tab mode. */
	end2 = 0;
/*
 *	Proceed thru FO line until PP command or EOL.
 */
    do
    {
		if((fochar = foget()) >= 0)	/* If it's a character, throw it away */
		{
			foget();			/* as well as its width. */
			continue;			/* To bottom of do-loop. */
		}
		if (fochar == -8)		/* End of line: */
		{
			if (not_in_tab)
				break;
  			while ((nchar = foget()) == 0) /* Null ends record, */
				fowrd = 256;	/* start new record and retry. */
			if (nchar<0)
			{					/* Error: End of file before [et */
				end2 = 1;
				break;
			}
			if ( (err = read_line_def( &temp_fo_line_def, foget)) )
			{					/* Error in .fo syntax. */
				sprintf (err_buf, 
				"PP ERROR - Unable to read line start after line %d,  forec %d %d, error %d in eoln8\n",
				last_good_line, forec, fowrd, err);
				stop(err_buf, 0,0);
			}
		}
		else if (fochar == -22)	/* Begin tab mode. */
			not_in_tab = 0;
		else if (fochar == -24)	/* End tab mode. */
			not_in_tab = 1;

		kcmd = -fochar;		    /* It's a command. Get command #, */
		if (kcmd > foreadsTOP)
			continue;			/* Illegal command#: Loop. */
		argcount = foreads[kcmd-1];	/* For all other commands, get
									   the argument count constant. */
		if (argcount == 0) continue; /* No arguments?  Loop. */
								/* Pos arg count is default value */
		if (argcount < 0)	    /* Neg arg count means: */
		{
			nchar = 0;
			for (j=-1; j>argcount; j--)
				nchar = foget();/*  skip (argcount-1) args, then  */
			argcount = foget();	/* ARGCOUNTth arg contains # remaining args  */
			if ((kcmd == 64) && (nchar == -20) && (argcount > 2))
			{
				for (j=0; j<argcount; j++)
					*(export_name+j) = (char)foget();
				*(export_name+j) = 0;
				break;			/* Exit fo-read loop, done.  */
			}
		}
		else if (argcount == 99) /* Special arg list */
		{
			j = foget();
			switch (j)
			{
			  case -12:
			  case -13:
				argcount = 26;
				break;
			  case -21:
				argcount = 1;
				break;
			  default:
				continue;	    /* 1st arg was only one. Loop. */
			}
		}
		else if (argcount == 98)	    /* Special arg list */
		{
			argcount = 1;
			if ((j = foget())==3 || j==4)
				argcount = 27;
		}						/* End of if/else sequence */
		for (j=1; j<=argcount; j++)	/* Loop to ignore string of args: */
			foget();		    /* Read & throw away each FO char */
    } while ( (fochar != -8) && (!end2) );

    foread(forec2);			/* Restore fo record and fo word */
    fowrd = fowrd2;
}			/* end scan_pp */

/**** EOF ****/
