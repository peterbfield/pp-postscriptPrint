#include <stdio.h>
#include <math.h>
#include "p_lib.h"
#include "psjob.h"

static void trap_cmd_bf(float ulx, float uly, float lrx, float lry,
						int16 offset, int redo_flag);
static void trap_cmd_bf_rounded(float x1, float y1, float x2, float y2);

static void trap_cmd_bf_flat(float x1, float y1);

static float corner_radius;
static int corner_radius_int;

int rule_break_type;
int blend_num;
int piece_ctr;
int total_depth;


extern int16 SetY_Bottom;
extern int continued_bf_flag;
extern int tab_offset;


extern int16 Current_FgColor;	/**  Bug352p  **/
extern int16 Current_FgShade;	/**  Bug352p  **/

int16 total_points;
int NewBgColor;
int NewBlendColor;
int CornerRadiusBfFlag;			/* 1=bf with no blend, 2=bf with blend
								   3=bf with blend, clip bottom
								   4=bf with blend, clip top
								   5=bf with blend, clip top and bottom */

float BfClipFx, BfClipFy, BfClipFx2, BfClipFy3;
float DegAngle, RadAngle;		/* angle in radians */
float SinAngle, CosAngle;
float BfBgShadeStart;

struct kmd80_stuff bf_kmd80[40]; /* [bf storage */

/***********************************************************************
 **  KMD59()    Move media.
 ***********************************************************************/
int kmd59()						/* Move media */
{return(0);}

/***********************************************************************
 **  KMD60()      Page size command, followed by 5 int16 arguments. **
 **                Of those 10 bytes the first five are an ascii **
 **                string that represents the horizontal dimension **
 **		  (which we ignore for now), **
 **  [ps           and the last five are an ascii string that **
 **                represents the vertical dimension. This is the **
 **		  minimum depth for this page only when an [mm **
 **		  command is encountered. **
 **								      **
 **	Note: [PSi-,-] not handled here.  See kmd64() **
 ***********************************************************************/

int kmd60()
{
    int16 asc_args[5], count, k, point_ind;
    char *ptr;
	int16 points = 0;
	int16 picas = 0;
    
    for(count= 0; count< 5; count++)
		asc_args[count] = foget(); /* get 10 ascii chars */
    ptr = (char *)&asc_args[0];	/* Point to it for byte access. */
    k = 0;
    point_ind = 0;				/* =1 hit decimal */
    for(count = 5;count < 10; count++) /* get page depth  */
    {							/* (ignore pg width)  */
		k = *(ptr+count);		/* Each of 5 chars */
		if(k == 32)continue;	/* skip space */
		k = k - 48;				/* Assume digit 0-9 */
		if(k < 0 || k > 9)		/* Is it? */
		{
			point_ind = 1;      /* No. set . or P ind */
			continue;			/* and loop for next */
		}
		if(point_ind==1)        /* picas or points */
			points = points * 10 + k; /* accum points */
		else
			picas  = picas  * 10 + k; /* accum picas */
    }
	total_points = (picas * 12) + points;
	return(0);
}								/* end kmd60 */

/***********************************************************************
 **  KMD79()   [xy, [hv    command. 1987 S.Bittman **
 ***********************************************************************/
int kmd79()
{return(0);}

/***********************************************************************
 *  KMD80()		Build Rectangular form command ([bf-]). *
 *	Draws background immediately, switches to fg immediately. *
 *	9 arguments in FO command: corner radius, rule break type, bottom offset, trapping offset, fg%shade,
 *	bg%shade, ULX, ULY, WID, DEP  *
 ***********************************************************************/
int kmd80()
{
	float arctan_angle;
	float bound_rect_width;
	float save_RoundCornerRadiusTL;
	uint32 vlist_plates, cc_mask_hold, trap_cc_hold;
	int16 bottom_offset, trapping_offset;
	int save_xpos, save_ypos;
	int hold_fg_color, hold_bg_color;
	int hold_fg_shade, hold_bg_shade;
	int new_fg_color;
	int hold_x, hold_y, hold_width, hold_depth;  
	int ii, jj;
	int extra_anchor_depth = 0;
	int final_depth_piece =  0;
	int pct_count = 0;
	int total_anchor_depths = 0;
	int total_anchor_percents = 0;
	int trap_type = 0;
	int temp_clip_flag = 0;

	if (in_overflow)
	{							/* no action for overflow */
		for (ii=0; ii<9; ii++)
			foget();
		return(0);
	}
	stack_print();				/* Put preceeding chars to TP. */
	cc_mask_hold = cc_mask;
	save_xpos = Xpos;
	save_ypos = Ypos;
	if ( !FileType)
		Xpos = 0;				/* set from left edge of galley absolute */
	else
		Xpos = SetX;			/* else set from left edge of frame */
    bottom_offset = foget();	/* bottom offset */
    trapping_offset = foget();	/* trapping offset */
	if (spot_pass == 1)
		cc_mask = 1 & Plates;
	else
		cc_mask = cc_hit;
    hold_fg_color = foget();
    hold_fg_shade = foget();
    hold_bg_color = foget();
    hold_bg_shade = foget();
	if ( continued_bf_flag)
	{
		hold_fg_color = fo_line_def.SolFgColor;
		hold_fg_shade = fo_line_def.SolFgShade;
	}
	BfBgShadeStart = (float)hold_bg_shade / 100.;
	new_fg_color = color_check(hold_fg_color, -1); /* Foreground (text) clr */
	Current_FgColor = new_fg_color;     /*  Bug 370p  */
	Current_FgShade = hold_fg_shade;    /*  Bug 370p  */

    NewBgColor = color_check(hold_bg_color, -1); /* Background color */
	if (blend_flag)
		NewBlendColor = color_check(BlendEndColor, -1); /* blend */
	else
		NewBlendColor = -1;
	if ( (new_fg_color < 0) && (NewBgColor < 0) && !active_trap_cc_mask
		  && (NewBlendColor < 0) )
	{							/* undefined, ignore cmd */
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "KMD80: fg, bg, blend = undefined, ignore cmd for pass %d.\n",
				   spot_pass);
#endif
		for (ii=0; ii<5; ii++)
			foget();			/* ignore xy, radius, rule break type */
		if ( (spot_pass == 1) && KeyOutputType)
		{
			set_pass_2_color(hold_bg_color, &cc_hit, 2);
			set_pass_2_color(hold_fg_color, &cc_hit, 2);
			if ( blend_flag)
			{
				set_pass_2_color(BlendEndColor, &cc_hit, 2);
				for (ii=0; ii<AnchorCount; ii++)
					set_pass_2_color(anchor_colors[ii], &cc_hit, 2);
			}
		}
		cc_mask = 0;
		continued_bf_flag = 0;
		return(0);
	}
#ifdef TRACE
	if ( blend_flag && (spot_pass == 1))
	{
		set_pass_2_color(hold_bg_color, &cc_hit, 2);
		set_pass_2_color(BlendEndColor, &cc_hit, 2);
		for (ii=0; ii<AnchorCount; ii++)
			set_pass_2_color(anchor_colors[ii], &cc_hit, 2);
	}
    if (color_trace)
	{
		p_info(PI_TRACE, "KMD80: fg= %d %d, bg= %d %d\n", new_fg_color, hold_fg_shade,
			   NewBgColor, hold_bg_shade);
		if ( blend_flag)
			p_info(PI_TRACE, "KMD80: blend =%d, angle= %d\n",
				   NewBlendColor, BlendAngle);
	}
#endif
	hold_x = foget() + fo_line_def.SolMarginAdjust;
	hold_y = foget();
	hold_width = foget();
	hold_depth = foget();
    corner_radius_int = foget();
    corner_radius = corner_radius_int / HorizontalBase;
	if ( (NewBgColor >= 0) || active_trap_cc_mask ||
		 (NewBlendColor >= 0) )
	{							/* paint bg now */
		BgColor = NewBgColor;
		BgShade = hold_bg_shade;
		Fx = hold_x + SetX;		/* Upper-left corner x-coordinate */
		Fy = hold_y + Ypos;		/* Upper-left corner y-coordinate */
		if (continued_bf_flag)
			Fy = SetY;
		Fx2 = hold_width + Fx;	/* Lower-rite corner x-coordinate */
		Fy3 = hold_depth + Fy;	/* Lower-rite corner y-coordinate */
		if (spot_pass == 1)
		{
			set_pass_2_color(hold_bg_color, &cc_hit, 2);
			set_pass_2_color(hold_fg_color, &cc_hit, 2);
			cc_mask = 1 & Plates;
		}
		else
		{						/* pass 2 */
			vlist_plates = 0;
			set_pass_2_color(NewBgColor, &vlist_plates, 2);
			set_pass_2_color(hold_fg_color, &vlist_plates, 2);
			if ( blend_flag)
			{
				set_pass_2_color(BlendEndColor, &vlist_plates, 2);
				for (ii=0; ii<AnchorCount; ii++)
					set_pass_2_color(anchor_colors[ii], &vlist_plates, 2);
			}
			vlist_plates |= active_trap_cc_mask;
			cc_mask = (vlist_plates & ~1) & Plates;
		}
#ifdef TRACE
    if (color_trace)
		p_info(PI_TRACE, "KMD80: cc_mask= %lo, ULX= %.3f, ULY=%.3f LRX= %.3f LRY=%.3f\n",
			   cc_mask,Fx,Fy,Fx2,Fy3);
#endif
		trap_cc_hold = active_trap_cc_mask;
		active_trap_cc_mask = 0;
		if ( blend_flag)
		{
			if (BlendAngle < 0)
			{
				arctan_angle = 572.957795 *	/* 1 rad in tenths degrees */
					atan(hold_width / (((double)hold_depth *
										HorizontalBase) /
									   VerticalBase));
				if (BlendAngle == -1)
					BlendAngle = 3600. - arctan_angle;	/* TL to BR */
				else if (BlendAngle == -2)
					BlendAngle = arctan_angle + 1800.;	/* TR to BL */
                else if (BlendAngle == -3)
                    BlendAngle = -3;       /* C */
                else if (BlendAngle == -4)
                    BlendAngle = -4;       /* O */
			}					/* end if(BlendAngle<0) */
			if (BlendAngle >= 0)
			{
				DegAngle = (float)BlendAngle / 10; /* angle in degrees */
				RadAngle = DegAngle * .0174532925; /* angle in radians */
				SinAngle = sin(RadAngle);
				if ( SinAngle < 0)
					SinAngle *= -1.;
				CosAngle = cos(RadAngle);
				if ( CosAngle < 0)
					CosAngle *= -1.;
			}
			if ( AnchorCount)
			{
				bound_rect_width = ((Fx2 - Fx) * CosAngle) +
					(((Fy3 - Fy) * SinAngle * HorizontalBase / VerticalBase));
				for (ii=0; ii<AnchorCount; ii++)
				{				/* check for excess amounts */
					if ( anchor_amounts[ii] < 0)
					{
						pct_count++;
						total_anchor_percents -= anchor_amounts[ii];
					}
					else
						total_anchor_depths += anchor_amounts[ii];
				}
				if ( total_anchor_depths >= bound_rect_width)
				{				/* change any percents to 1%, extend end */
					for (ii=0; ii<AnchorCount; ii++)
					{
						if ( anchor_amounts[ii] < 0)
						{
							anchor_amounts[ii] = -total_anchor_depths /
								100; /* 1 percent */
							extra_anchor_depth += anchor_amounts[ii];
						}
						total_anchor_percents = pct_count;
					}
#ifdef TRACE
					if (color_trace)
						p_info(PI_TRACE, "KMD80: extra width: fixed (%d),  (%.3f), pct (%d)\n",
							   total_anchor_depths, bound_rect_width, 
							   pct_count);
#endif
				}
				total_anchor_depths = 0;
				for (ii=0; ii<AnchorCount; ii++)
				{				/* change percents to fixed amounts */
					if ( anchor_amounts[ii] < 0)
						anchor_amounts[ii] = (-anchor_amounts[ii] *
											  bound_rect_width) / 100;
					total_anchor_depths += anchor_amounts[ii];
				}
				if ( bound_rect_width > total_anchor_depths)
					final_depth_piece = bound_rect_width; /* mark totl depth */
				else
					final_depth_piece = total_anchor_depths +
						(total_anchor_depths / 100); /* add 1% */
				anchor_amounts[20] = final_depth_piece;
#ifdef TRACE
				if (color_trace)
					p_info(PI_TRACE, "KMD80: anchor width (%d)\n",
						   final_depth_piece);
#endif
			}					/* end if(AnchorCount) */
		}						/* end if (blend_flag */
/* set up a clip path */
		BfClipFx = Fx;
		BfClipFy = Fy;
		BfClipFx2 = Fx2;
		BfClipFy3 = Fy3;
		if ( BfClipFy < 0 && (!FileType))
		{
			BfClipFy = 0;
			temp_clip_flag = 3;	/* clip top */
		}
		if ( BfClipFy > Imageheight && (!FileType))
		{
			BfClipFy = Imageheight;
			if ( temp_clip_flag)
				temp_clip_flag = 5;	/* clip both */
			else
				temp_clip_flag = 4;	/* clip bottom */
		}
		if ( BfClipFy3 > Imageheight && (!FileType))
		{
			BfClipFy3 = Imageheight;
			if ( temp_clip_flag)
				temp_clip_flag = 5;	/* clip both */
			else
				temp_clip_flag = 4;	/* clip bottom */
		}
		if (BfClipFy == BfClipFy3)
			BfClipFy3 += VerticalBase;	/* make it 1 point */
		save_RoundCornerRadiusTL = RoundCornerRadiusTL;
		RoundCornerRadiusTL = corner_radius;
		if ( !blend_flag)
		{
			if ( corner_radius)
				CornerRadiusBfFlag = 1;
			do_pskmd('V',"kmd80 [bf-]");
		}
		else
		{
			if ( corner_radius && !temp_clip_flag)
				CornerRadiusBfFlag = 2;
			else
				CornerRadiusBfFlag = temp_clip_flag;
			do_pskmd('B',"kmd80 [bf- blend]"); /* segment the output */
		}
		RoundCornerRadiusTL = save_RoundCornerRadiusTL;
		CornerRadiusBfFlag = 0;
		active_trap_cc_mask = trap_cc_hold;
		trap_type = 0;
/* DSV: I do not see a way for trap_type to be anything other than 0 */

		if ( BfClipFy3 >= Imageheight )
			trap_type = 2;		/* for upside down U-shaped box  */
		if ( active_trap_cc_mask)
			trap_cmd_bf(BfClipFx, BfClipFy, BfClipFx2, BfClipFy3,
						trapping_offset, trap_type);
		Xpos = save_xpos;
		Ypos = save_ypos;
		Ofx = -4000;			/* force Move command output */
		do_pskmd('m',"at kmd80"); /* Move back to latest Xpos, Ypos. */
		if ( !FileType && ((Ypos + hold_depth) > Imageheight) )
		{						/* galley mode save , cmd > sheet depth  */
			for (ii=0; ii<40; ii++)
			{
				if ( !bf_kmd80[ii].active_flag)
					break;
			}
			if ( ii < 40)
			{					/* save for next sheet */
				bf_kmd80[ii].active_flag++;
				bf_kmd80[ii].start_depth = Ypos - hold_y;
				bf_kmd80[ii].corner_radius = corner_radius_int;
				bf_kmd80[ii].trapping_offset = trapping_offset;
				bf_kmd80[ii].bg_color = BgColor;
				bf_kmd80[ii].bg_shade = BgShade;	/***Bug352p***  fixes bg color printing on continued pages ***/
				bf_kmd80[ii].upr_left_x = hold_x;
				bf_kmd80[ii].width = hold_width;
				bf_kmd80[ii].depth = hold_depth;
				bf_kmd80[ii].original_depth = hold_depth;
				bf_kmd80[ii].active_trap_cc_mask = active_trap_cc_mask;
				bf_kmd80[ii].active_trap_color = ActiveTrapColor;
				bf_kmd80[ii].active_trap_stroke_width =
					ActiveTrapStrokeWidth;
				bf_kmd80[ii].blend_flag = blend_flag;
				bf_kmd80[ii].blend_angle = BlendAngle;
				bf_kmd80[ii].blend_end_color = BlendEndColor;
				bf_kmd80[ii].anchr_cnt = AnchorCount;
				for ( jj=0; jj<AnchorCount; jj++)
				{
					bf_kmd80[ii].anchr_amts[jj] = anchor_amounts[jj];
					bf_kmd80[ii].anchr_clrs[jj] = anchor_colors[jj];
				}
			} 
		}						/* end galley mode save */
	}							/* end paint bg now */
	blend_flag = 0;

/* ---------- set up fg ---------- */

	if ( continued_bf_flag)
	{
		Xpos = Xmark + fo_line_def.LeftIndent + tab_offset + 
			fo_line_def.SolMarginAdjust;
		switch(fo_line_def.Quad)
		{
		  case  2:				/* Quad right */
		  case  3:				/* Quad center */
			Xpos += fo_line_def.UnusedMeasure;
			break;
		}
		continued_bf_flag = 0;
	}
	if (new_fg_color >= 0)
	{
		color_func (new_fg_color, hold_fg_shade); /* fg is defined   Bug352p */
		vid_color(new_fg_color, hold_fg_shade, cc_mask ); /* fg */
	}
	else
		cc_mask = 0;
	if ( (spot_pass == 2) || !KeyOutputType)
		return(0);
	set_pass_2_color(hold_bg_color, &cc_hit, 2);
	set_pass_2_color(hold_fg_color, &cc_hit, 2);
	return(0);
}

/***********************************************************************
 *  KMD81();	Define Color command ([tt-]). *
 *	Allows specification of a numbered color, in terms of dot *
 *	shape, size, density, angle, dot/rule.  Will be referenced *
 *	by later Build Rectangle command. 5 arguments in FO command. *

 *********  NOT USED  *********

 ***********************************************************************/
int kmd81()
{
    int16 color_index, gray_level;
    uint16 rwidth; 
    int16 color_type, color_switch;
    
    color_index = foget();		/* Color: 0=white 1=black 2+=color */
    rwidth = foget();			/* Width between screen elements (dots
								   or rules) in HorizontalBase units. */
								/* 1st pass: ignore, use default screen	*/
    gray_level = foget();		/* Density %, range .01% to 100%.
								   (I'm not sure about the following def)
								   %s stored as "points", in Horiz Base units:
								   In base 40: 1% is 40, 2% is 80,
								   100% is 4000, .1% is 4, .05% is 2 */
    color_type = foget();		/* Pattern type (integer, to be de-
								   termined for PostScript.) */
								/* is 1-8 to emulate L300:
								   1 = 45 degree round dot
								   2 = 90 degree round dot
								   3 = 45 degree square dot
								   4 = 90 degree square dot
								   5 = vert line at +45 degree (/)?
								   6 = vert line at -45 degree (\)?
								   7 = horizontal lines
								   8 = vertical lines */
    color_switch = foget();		/* Switch used by L300:
								   0=shape-optimized  1=density-opt. */
/*    add_color(color_index, rwidth, gray_level, color_type, color_switch); */
	return(0);
}

/***********************************************************************
 **  KMD82();  Box - [BX - command .  The 1st arg from foget is the    **
 **            total number of int16s that make up all the arguments   **
 **            to the command. Each argument is a six word unit        **
 **            that describes a segment. A segment is a portion        **
 **            of either a vertical or horizontal line.                **
 **                                                                    **
 **                A Segment is described as follows.                  **
 **                                                                    **
 **            WORD                  CONTENTS                          **
 **                                                                    **
 **	       		1       The type of segment being described.		   **
 **								1 = vertical rule segment.		       **
 **								2 = horizontal rule segment.		   **
 **								3 = horizontal redline seg, level 1.   **
 **								4 = horizontal redline seg, level 2.   **
 **                                                                    **
 **             2       The weight of the the rule containing this     **
 **                     segment. The weight will supplied in units     **
 **                     as specified by the system standards item #    **
 **                     96 (in 1/10's of a point).                     **
 **						Note: High order bit is set if vertical rule   **
 **                     is to be extended by vertical justification.   **
 **                                                                    **
 **             3       X POS  from the absolute left margin  supplied **
 **                     in whatever horizontal base is in use.         **
 **                                                                    **
 **             4       The vertical offset from the line base and     **
 **                     leading. Zero indicates no offset, a positive  **
 **                     number indicates offset toward the top of      **
 **                     the page and a negative number indicates an    **
 **                     offset toward the bottom of the page.          **
 **                                                                    **
 **             5       Length of the line if horizontal and height    **
 **                     of the line if vertical. If line is horizontal **
 **                     dimension is given in increments of the        **
 **                     horizontal base.                               **
 **                                                                    **
 **				6		Segment color number                           **
 **                                                                    **
 **				7		Segment shade                                  **
 **                                                                    **
 ***********************************************************************/

int kmd82()
{
#define  vertical    1
#define  horizontal  2
#define  autotab_vertical  3

    int16 num_segs;
    int16 seg_type;
    int16 seg_weight;
    int seg_x_pos;
    int16 seg_vert_off;
    int16 seg_width;
	int seg_color;
	int seg_shade;
	int new_fg_color;
	uint32 old_cc_mask;
	int old_color = -3;
	int old_shade = -3;
	int extend_depth_flag;		/* 1 = add vert just data to rule depth */

	old_cc_mask = cc_mask;
    num_segs = foget() / 7;		/* Get count of # 7-word segments */
	stack_print();				/* Put preceeding chars to TP. */
    while( num_segs)
    {
		seg_type = foget();
		seg_weight = foget();
		seg_x_pos = foget() + fo_line_def.SolMarginAdjust;
		seg_vert_off = foget();
		seg_width = foget();
		seg_color = (int)foget();
		seg_shade = (int)foget();
		num_segs--;
		extend_depth_flag = seg_weight & 0x8000; /* save flag bit */
		seg_weight &= ~0x8000;	/* remove flag bit */
		if(FileType)
			seg_x_pos += Xmark;
		if ( (seg_color != old_color) || (seg_shade != old_shade) )
		{
			if (seg_color < 0)
				seg_color = DefaultFrameFgColor;
			if ( spot_pass == 1)
				cc_mask = 1 & Plates;
			else
				cc_mask = cc_hit;
			new_fg_color = color_check(seg_color, -1);
			if (new_fg_color < 0)
			{					/* undefined, ignore cmd */
#ifdef TRACE
				if (color_trace)
					p_info(PI_TRACE, "KMD82: fg = -1, ignore cmd.\n");
#endif
				if ( (spot_pass == 1) && KeyOutputType)
					set_pass_2_color(seg_color, &cc_hit, 2);
				continue;
			}
#ifdef TRACE
			if (color_trace)
				p_info(PI_TRACE, "KMD82: fg=%d %d\n", new_fg_color, seg_shade);
#endif
			if (FlashOn)
				color_func(new_fg_color, seg_shade);
			old_color = seg_color;
			old_shade = seg_shade;
			FgShade = old_shade;
		}						/*end if(seg_color!=old_color||seg_shade....) */
		if((seg_type == vertical) || (seg_type == autotab_vertical))
		{
			Rwei = ((float)seg_weight * HorizontalBase) / Jrule;
			if ( !extend_depth_flag || !FileType)
			{
				Fy = (float)Ypos - (float) seg_vert_off - (float)seg_width;
				Fy3 = Fy + (float)seg_width;
			}
			else
			{					/* extend top */
				Fy = (float)Ypos - (float)seg_vert_off - (float)seg_width -
					vj_pending;
				Fy3 = (float)Ypos - (float) seg_vert_off;
			}
			if ( FileType)
			{					/* limit to frame bounds if not galley */
				if ( Fy < SetY )
					Fy = SetY;	/* rule cannot go above frame top */
				if ( Fy3 < SetY )
					continue;	/* no rule */
				if ( Fy3 > SetY_Bottom )
					Fy3 = SetY_Bottom; /* rule cannot go below frame bottom */
				if ( Fy > SetY_Bottom )
					continue;	/* no rule */
			}
			Fx = (float)seg_x_pos;
			Fx2 = Fx + Rwei;
/*
p_info(PI_TRACE, "type %d, weight %d, x %d, v off %d, wid %d, bol= %d, vj= %d, ext %d\n",
	   seg_type,seg_weight,seg_x_pos,seg_vert_off,seg_width,
	   fo_line_def.BolLeading,vj_pending,extend_depth_flag);
p_info(PI_TRACE, "-- Fx= %.0f, Fy= %.0f, Fy3= %.0f, Ypos= %d\n",Fx,Fy,Fy3,Ypos);
*/
			if (FlashOn)
				do_pskmd('L',"rulebox vert seg");
		}
		else if(seg_type == horizontal)
		{
			Rwei = ((float)seg_weight * VerticalBase) / Jrule;
			Fx = (float)seg_x_pos;
			Fx2 = Fx + (float)seg_width;
			Fy = (float)Ypos - (float)seg_vert_off - Rwei;
			Fy3 = Fy + Rwei;
/*
p_info(PI_TRACE, "type %d, weight %d, x %d, v off %d, wid %d, bol= %d, vj= %d, ext= %d\n",
	   seg_type,seg_weight,seg_x_pos,seg_vert_off,seg_width,
	   fo_line_def.BolLeading,vj_pending,extend_depth_flag);
p_info(PI_TRACE, "-- Fx= %.0f, Fy= %.0f, Fy3= %.0f, Ypos= %d\n",Fx,Fy,Fy3,Ypos);
*/
			if (FlashOn)
				do_pskmd('L',"rulebox hor seg");
		}
		else
		{
			error(" Unknown box rule type."," ", 0);
			continue;
		}
		Ofx = -4000;			/* force Move command out */
		do_pskmd('M',"end kmd82");
#ifdef TRACE
		if (debugger_trace)
			p_info(PI_TRACE, "type %d, weight %d, x %d, v off %d, wid %d\n",
				   seg_type,seg_weight,seg_x_pos,seg_vert_off,seg_width);
#endif
    }							/* end while(num_segs) */
	Ofx = -4000;				/* force Move command out */
    do_pskmd('M',"rulebox");
	vj_pending = 0;				/* Done w/ box/rule piece, reset its VJ adj.  */
	cc_mask = old_cc_mask;		/* restore old color */
	return(0);
}								/* end function */
int kmd84() {

/* 1st field is rule break type */
/* 2nd field is blend number */
/* 3rd field is piece number  */
/* 4th field is total depth */

	stack_print();
	rule_break_type=foget();
	blend_num = foget();
	piece_ctr  = foget();
	total_depth = foget();
	return(0);
}

/*------------------------------------------------------------------------*/
void fdigi_print(float num)
{
    int i;
    char str[20];
    sprintf(str,"%5.5f",num);
    for(i=0;i<6;i++)
		if(str[i] == '.') break;
    str[i+4] = 0;
    if(str[i+3] == '0')
    {
		str[i+3] = 0;
		if(str[i+2] == '0')
		{
			str[i+2] = 0;
			if(str[i+1] == '0')
			{
				str[i+1] = 0;
				str[i] = 0 ;
			}
		}
    }
    fprintf(TrailerName,"%s ",str);
}

/*------------------------------------------------------------------------*/
void digi_print(float num)
{
    int i;
    char str[20];
    sprintf(str,"%5.5f",num);
    for(i=0;i<6;i++)
		if(str[i] == '.') break;
    str[i+4] = 0;
    if(str[i+3] == '0')
    {
		str[i+3] = 0;
		if(str[i+2] == '0')
		{
			str[i+2] = 0;
			if(str[i+1] == '0')
			{
				str[i+1] = 0;
				str[i] = 0 ;
			}
		}
    }
    m_fprintf("%s ",str);
}

/*------------------------------------------------------------------------*/
void resend_kmd_bf(int16 gal_depth)
{
	int save_active_trap_color;
	int16 save_active_trap_stroke_width;
    int ii, jj, depth_used;
	uint32 vlist_plates, cc_mask_hold, save_active_trap_cc_mask;
	int save_xpos, save_ypos;
	float save_fx, save_fx2, save_fy, save_fy3;
	float save_RoundCornerRadiusTL;
	int temp_clip_flag = 0;

	cc_mask_hold = cc_mask;
	save_xpos = Xpos;
	save_ypos = Ypos;
	save_active_trap_cc_mask = active_trap_cc_mask;
	save_active_trap_color = ActiveTrapColor;
	save_active_trap_stroke_width = ActiveTrapStrokeWidth;
	for (ii=0; ii<40; ii++)
	{
		if ( bf_kmd80[ii].active_flag)
		{						/* output a new [bf */
			depth_used = gal_depth;

			/***  Bug 352p - fixes bg box too large  ***/
			bf_kmd80[ii].depth = bf_kmd80[ii].depth - (Imageheight - bf_kmd80[ii].start_depth);	  

			BgColor = bf_kmd80[ii].bg_color;
			BgShade = bf_kmd80[ii].bg_shade;
			corner_radius_int = bf_kmd80[ii].corner_radius;
			corner_radius = corner_radius_int / HorizontalBase;
			Fx = SetX + bf_kmd80[ii].upr_left_x; /* Upr-lft corner x-coord */

			/*** Bug 352p fixes bg overprint of header     ***/
			Fy = Fy - depth_used + fo_line_def.BolLeading;  /* Upper-left corner y-coord ***/

			Fx2 = bf_kmd80[ii].width + Fx; /* Lower-rite corner x-coordinate */

			/*** Bug 352p  print bg proper size  ***/
			Fy3 = bf_kmd80[ii].depth;

			if ( bf_kmd80[ii].depth <= Imageheight)
			{
				bf_kmd80[ii].active_flag = 0;
				bf_kmd80[ii].depth = 0;
			}
			/*** End of print bg proper size fix *****/

			if (spot_pass == 1)
				cc_mask = 1 & Plates;
			else
			{
				vlist_plates = 0;
				set_pass_2_color(BgColor, &vlist_plates, 2);
				if ( bf_kmd80[ii].blend_flag)
					set_pass_2_color(bf_kmd80[ii].blend_end_color,
									 &vlist_plates, 2);
				for (jj=0; jj<AnchorCount; jj++)
					set_pass_2_color(anchor_colors[jj], &vlist_plates, 2);
				cc_mask = vlist_plates & ~1;
			}
#ifdef TRACE
			if (color_trace)
				p_info(PI_TRACE, "resend KMD80: cc_mask= %lo, ULX= %.3f, ULY=%.3f LRX= %.3f LRY=%.3f\n",
					   cc_mask,Fx,Fy,Fx2,Fy3);
#endif
			blend_flag = bf_kmd80[ii].blend_flag;
			if (blend_flag)
			{
				BlendAngle = bf_kmd80[ii].blend_angle;
				BlendEndColor = bf_kmd80[ii].blend_end_color;
				if (BlendAngle >= 0)
				{
					DegAngle = (float)BlendAngle / 10; /* angle in degrees */
					RadAngle = DegAngle * .0174532925; /* angle in radians */
					SinAngle = sin(RadAngle);
					CosAngle = cos(RadAngle);
					if ( SinAngle < 0)
						SinAngle *= -1.0;
					if ( CosAngle < 0)
						CosAngle *= -1.0;
				}
				AnchorCount = bf_kmd80[ii].anchr_cnt;
				for (jj=0; jj<AnchorCount; jj++)
				{
					anchor_amounts[jj] = bf_kmd80[ii].anchr_amts[jj];
					anchor_colors[jj] = bf_kmd80[ii].anchr_clrs[jj];
				}
			}					/* end if(blend_flag) */
/* set up a clip path */
			BfClipFx = Fx;
			BfClipFy = Fy;
			BfClipFx2 = Fx2;
			BfClipFy3 = Fy3;

			if ( (BfClipFy - fo_line_def.BolLeading) < 0)	/*** Bug 352 - fix bg box opening at both ends  ***/
			{
				BfClipFy = 0;
				temp_clip_flag = 3;	/* clip top */
			}
			if ( BfClipFy > Imageheight)
			{
				BfClipFy = Imageheight;
				if ( temp_clip_flag)
					temp_clip_flag = 5;	/* clip both */
				else
					temp_clip_flag = 4;	/* clip bottom */
			}
			if ( BfClipFy3 > Imageheight)
			{
				BfClipFy3 = Imageheight;
				if ( temp_clip_flag)
					temp_clip_flag = 5;	/* clip both */
				else
					temp_clip_flag = 4;	/* clip bottom */
			}
			if (BfClipFy == BfClipFy3)
				BfClipFy3 += VerticalBase;	/* make it 1 point */
			save_fx = Fx;
			save_fy = Fy;
			save_fx2 = Fx2;
			save_fy3 = Fy3;
			do_pskmd('m',"resend kmd80 [bf-], force page start ");
			Fx = save_fx;
			Fy = save_fy;
			Fx2 = save_fx2;
			Fy3 = save_fy3;
			save_RoundCornerRadiusTL = RoundCornerRadiusTL;
			RoundCornerRadiusTL = corner_radius;
			if ( !blend_flag)
			{
				if ( corner_radius)
					CornerRadiusBfFlag = 1;
				do_pskmd('V',"resend kmd80 [bf-]");
			}
			else
			{
				if ( corner_radius && !temp_clip_flag)
					CornerRadiusBfFlag = 2;
				else
					CornerRadiusBfFlag = temp_clip_flag;
				do_pskmd('B',"kmd80 [resend bf- blend]"); /* segment output */
			}
			RoundCornerRadiusTL = save_RoundCornerRadiusTL;
			CornerRadiusBfFlag = 0;
			active_trap_cc_mask = bf_kmd80[ii].active_trap_cc_mask;
			ActiveTrapColor = bf_kmd80[ii].active_trap_color;
			ActiveTrapStrokeWidth = bf_kmd80[ii].active_trap_stroke_width;
			if ( active_trap_cc_mask)

				/*** Bug 352p - fix bg box on continued pages, U, upsidedown U, open both ends  ***/
				trap_cmd_bf(BfClipFx, BfClipFy, BfClipFx2, BfClipFy3,
					bf_kmd80[ii].trapping_offset, (temp_clip_flag - 2));

			Xpos = save_xpos;
			Ypos = save_ypos;
			Ofx = -4000;		/* force Move command output */
			do_pskmd('m',"at kmd80"); /* Move back to latest Xpos, Ypos. */
			bf_kmd80[ii].start_depth = 0;	
			cc_mask = cc_mask_hold;
			active_trap_cc_mask = save_active_trap_cc_mask;
			ActiveTrapColor = save_active_trap_color;
			ActiveTrapStrokeWidth = save_active_trap_stroke_width;
			blend_flag = 0;

			/*** Bug 352p  fix text on bg not printing on continued pages  ***/
			color_func(Current_FgColor,Current_FgShade);
		}
	}
}

/*------------------------------------------------------------------------*/
void trap_cmd_bf(float ulx, float uly, float lrx, float lry, int16 offset, int redo_flag)
{
	uint32 save_cc_mask, loop_temp, trap_cc_hold, BlackPaint_cc_mask_sav;
	float ullx, ully, llrx, llry;
	float path_offset_x, path_offset_y;
	int i;

    redo_flag = 0;

/* Trap offset is the distance from the frame edge to the inside of the trap
   rule.  Offset of zero puts the inside edge of the rule along the
   edge of the frame.  Positive values move the rule edge toward the inside
   of the frame, negative values move it toward the outside of the frame.
   redo_flag = 0 draw entire box. = 1, u shaped box. =2 is upside down u-shaped 
   box.   = 3 for open box at top & bottom */

	path_offset_x = ((float)ActiveTrapStrokeWidth / 2) -
		(((float)offset * HorizontalBase) / VerticalBase);
	path_offset_y = (((float)ActiveTrapStrokeWidth * VerticalBase) /
					 (2 * HorizontalBase)) - (float)offset;
	ullx = ulx - trim_mark_width - path_offset_x;
	ully = uly - trim_mark_depth - path_offset_y;
	llrx = lrx - trim_mark_width + path_offset_x;
	llry = lry - trim_mark_depth + path_offset_y;
	save_cc_mask = cc_mask;
	cc_mask = active_trap_cc_mask;
	trap_cc_hold = active_trap_cc_mask;
	active_trap_cc_mask = 0;
	for (i=0; i<MAX_CLRS-1; i++)
	{
		cc_mask = ( loop_temp = (1 << i) ) & save_cc_mask;
		if(loop_temp > save_cc_mask)
			break;
		if (!cc_mask)
			continue;
		if (find_color(ActiveTrapColor, i+1) )
		{						/* found the color structure */
			m_fprintf("GS newpath %3.3f setlinewidth\n",
					  (float)ActiveTrapStrokeWidth /
					  HorizontalBase);
			BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
			BlackPaint_cc_mask = 0;
			vid_color(ActiveTrapColor, ActiveTrapShade, i+1);
			PaintOnPage |= BlackPaint_cc_mask;
			BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
			/* DSV: Using redo_flag code here for my own purposes */
			/* redo_flag does not seem to be implemented yet */


			/* Setting next three cases to no rule on break */
			/* Set redo flag based on segment type [bfx */
			if (rule_break_type == 20 )
			{
				/* End Segment */
				redo_flag = 1;
			}	
			if (rule_break_type == 12)
			{
				/* Mid Segment */
					redo_flag = 3;

			}

			if (rule_break_type == 4)
			{
				/* Start Segment */
				redo_flag = 2;
			}



			if ( redo_flag <= 0)  /***  Bug 352 allow print of open ended box  ***/ 
			{					
				if ( !corner_radius)
				{
					output_ele_xy (ullx, ully);
					m_fprintf(" M "); /* move to the UL corner */
					output_ele_xy (llrx, ully);
					m_fprintf(" L "); /* UR */
					output_ele_xy (llrx, llry);
					m_fprintf(" L "); /* LR */
					output_ele_xy (ullx, llry);
					m_fprintf(" L "); /* LL */
				}				/* end if(!corner_radius_int)  */
				else
				{				/* round corners */

					if (rule_break_type == 0 || rule_break_type >= 32)
					{
						/* entire box */
						output_ele_xy (ullx + corner_radius_int, ully);
						m_fprintf(" M "); /* move to the UL corner */

						trap_cmd_bf_rounded (llrx, ully, llrx, llry);
						trap_cmd_bf_rounded (llrx, llry, ullx, llry);
						trap_cmd_bf_rounded (ullx, llry, ullx, ully);
						trap_cmd_bf_rounded (ullx, ully, ullx + corner_radius_int, ully);

					}
					else
					{

						switch (rule_break_type)
						{

							case 17: /* end segment [bfb */
								/* entire box */
								output_ele_xy (ullx + corner_radius_int, ully);
								m_fprintf(" M "); /* move to the UL corner */

								trap_cmd_bf_rounded (llrx, ully, llrx, llry);
								trap_cmd_bf_rounded (llrx, llry, ullx, llry);
								trap_cmd_bf_rounded (ullx, llry, ullx, ully);
								trap_cmd_bf_rounded (ullx, ully, ullx + corner_radius_int, ully);
								break;

							case 18: /* end segment [bff */
								output_ele_xy (ullx + corner_radius_int, ully);
								m_fprintf(" M "); /* move to the UL corner */
								trap_cmd_bf_flat (llrx, ully);
								trap_cmd_bf_rounded (llrx, llry, ullx, llry);
								trap_cmd_bf_rounded (ullx, llry, ullx, ully);
								trap_cmd_bf_flat (ullx, ully);
								break;


							case 9: /* mid segment [bfb */
								output_ele_xy (ullx + corner_radius_int, ully);
								m_fprintf(" M "); /* move to the UL corner */
								trap_cmd_bf_rounded (llrx, ully, llrx, llry);
								trap_cmd_bf_flat (llrx, llry);
								trap_cmd_bf_flat (ullx, llry);
								trap_cmd_bf_rounded (ullx, ully, ullx + corner_radius_int, ully);
								break;

							case 10: /* mid segment [bff */
								output_ele_xy (ullx, ully);
								m_fprintf(" M "); /* move to the UL corner */
								trap_cmd_bf_flat (llrx, ully);
								trap_cmd_bf_flat (llrx, llry);
								trap_cmd_bf_flat (ullx, llry);
								trap_cmd_bf_flat (ullx, ully);
								break;

							case 1: /* start segment [bfb */
								/* fall thru */

							case 2: /* start segment [bff */
								output_ele_xy (ullx + corner_radius_int, ully);
								m_fprintf(" M "); /* move to the UL corner */
								trap_cmd_bf_rounded (llrx, ully, llrx, llry);
								trap_cmd_bf_flat (llrx, llry);
								trap_cmd_bf_flat (ullx, llry);
								trap_cmd_bf_rounded (ullx, ully, ullx + corner_radius_int, ully);
								break;


						}
					}
				}				
				m_fprintf("closepath ");
			}					/* end if (!redo_flag) */
			else if (redo_flag == 1)
			{					/* draw U-shaped box */
				if ( !corner_radius)
				{
					output_ele_xy (llrx, ully);
					m_fprintf(" M "); /* move to the UR corner */
					output_ele_xy (llrx, llry);
					m_fprintf(" L "); /* LR */
					output_ele_xy (ullx, llry);
					m_fprintf(" L "); /* LL */
					output_ele_xy (ullx, ully);
					m_fprintf(" L "); /*  UL */

				}				/* end if(!corner_radius_int)  */
				else
				{				/* round corners */
					output_ele_xy (ullx, ully);
					m_fprintf(" M "); /* move to the UL corner */
					trap_cmd_bf_rounded (ullx, llry, llrx, llry);
					trap_cmd_bf_rounded (llrx, llry, llrx, ully);
					output_ele_xy (llrx, ully);
					m_fprintf(" L "); /* finish line */

				}
			}					/* end draw U-shaped box */
			else if (redo_flag == 2)	 /***  Bug 352 allow print of open ended box  ***/
			{					/* draw upside down U-shaped box for flag=2 */
				if ( !corner_radius)
				{
					output_ele_xy (ullx, llry);
					m_fprintf(" M "); /* move to the LL corner */
					output_ele_xy (ullx, ully);
					m_fprintf(" L "); /* UL */
					output_ele_xy (llrx, ully);
					m_fprintf(" L "); /* UR */
					output_ele_xy (llrx, llry);
					m_fprintf(" L "); /*  LR */
				}				/* end if(!corner_radius_int)  */
				else
				{				/* round corners */
					output_ele_xy (ullx, llry);
					m_fprintf(" M "); /* move to the LL corner */
					trap_cmd_bf_rounded (ullx, ully, llrx, ully);
					trap_cmd_bf_rounded (llrx, ully, llrx, llry);
					output_ele_xy (llrx, llry);
					m_fprintf(" L "); /*  LR */
				}
			}					/* end draw upside down U-shaped box */

			/*** bug 352p - fixed bg box to print with both ends open on 
				 continued pages.  rounded corners not needed   ***/
			else if (redo_flag == 3)
			{					/* draw box open top & bottom for flag=3 */
					output_ele_xy (ullx, llry);
					m_fprintf(" M "); /* move to the LL corner */
					output_ele_xy (ullx, ully);
					m_fprintf(" L "); /* UL */
					output_ele_xy (llrx, ully);
					m_fprintf(" M "); /* UR */
					output_ele_xy (llrx, llry);
					m_fprintf(" L "); /*  LR */
					/* Draw top && bottom Rule if rule_type is not 20 12 or 8 */
					if (rule_break_type != 20 && rule_break_type != 12 && rule_break_type != 8)
					{
						m_fprintf("stroke GR\n");
						m_fprintf("GS \n");
						output_ele_xy(ullx, llry);
						m_fprintf("M\n");
						output_ele_xy(llrx, llry);
						m_fprintf("L\n");
						m_fprintf("stroke GR\n");
						m_fprintf(" GS ");
						output_ele_xy(ullx, ully);
						m_fprintf("M\n");
						output_ele_xy(llrx, ully);
						m_fprintf("L\n");

					}
			}					/* end draw box open top & bottom */
		/*** End fixed bg box to print with both ends open on continued pages  ***/

			m_fprintf("stroke GR\n"); /* Stroke the path. */


		}						/* end if(find_color(ActiveTrapColor,i+1) ) */
	}		/* end for (i=0;i<MAX_CLRS-1;i++) */
	cc_mask = save_cc_mask;
	active_trap_cc_mask = trap_cc_hold;
}						/* end function */

/*------------------------------------------------------------------------*/
void trap_cmd_bf_rounded(float x1, float y1, float x2, float y2)
{
		output_ele_xy (x1, y1);
		output_ele_xy (x2, y2);
		m_fprintf("%5.2f arcto 4 {pop} repeat\n", corner_radius);
}
void trap_cmd_bf_flat(float x1, float y1)
{
		output_ele_xy (x1, y1);
		m_fprintf("lineto \n");
}

/****       EOF        ****/


