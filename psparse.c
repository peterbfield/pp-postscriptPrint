#include <stdio.h>
#include <stdlib.h>
#include "psjob.h"
#include "rel_data.h"
#include "list_control.h"
#include "llist.h"
#include "mem_lay.f"
#include "traces.h"

static void crop_set(int type, int setx, int sety);
static int do_crops(void);
static void ps_get_page(void);
static void get_frame_params(void);

extern char uBR_UnitName[];
extern char uCH_UnitName[];
extern char uFN_UnitName[];
extern char uIL_UnitName[];
extern char uSN_UnitName[];
extern char uVS_UnitName[];
extern int KeyTrimWeight;		/* in 1/20 pts */
extern int KeyRegisterLength;	/* in points */
extern int KeyTrimLength;
extern int KeyTrimRegGap;

struct ps_pieces ps_end_file;
char JobName[MAX_NAME];			/* unit or galley name, no extensions */
char SubDirName[MAX_NAME + 4];	/* sub_dir name + ext .prj or .gal */
char ParentDirName[MAX_NAME + 4]; /* parent's sub_dir name + ext .prj */
char NextDirName[MAX_NAME + 4];	/* sub_dir name + ext .prj or .gal */
char TreeName[MAX_NAME];
int FirstPage;
int LastPage;
int16 HorizontalBase;
int16 VerticalBase;
char FoName[132];				/* Full pathname for FO file */
WYSIWYG *wn;
int next_dot_page_page;
int num_dot_page_recs;
int parse_trace;
int BGOutputFlag;				/* 1 = only output BG colors */
int delay_records_flag;
int16 yx_convert[2] = {1350,100};
int RuleType;					/* 0=vertical, 1=horizontal, 2=solid */
int RuleWt;
int RuleWid;
int RuleHgt;
int ResendPageFlag;
int crop_mode;
int16 mu_rot_degree;
int16 mu_rot_x, mu_rot_y;
int parse_mode;
LLIST *QueLayList = NULL;
LLIST *QueMemList = NULL;
LLIST *QueMemListParent = NULL;
int ParentIndex, LastParentIndex;
int16 SetY_Bottom;
int dashwid;
int gapwid;
int odashwid;
int ogapwid;
static int next_page;
static int delay_mode;
static int graphic_count;
static int psinit_entered_flag;
static int16 crop_type;			/* 0 = vertical, 1 = horizontal  */
static int16 crop_wid;			/* Width of horiz crop mark in HorizBase */
static int16 crop_dep;			/* Depth of vert crop mark in VertBase */
static int16 crop_weight;		/* Weight in 1/20 point */
static int16 crop_cx;			/* Horizontal page center from 0,0 */
static int16 crop_cy;			/* Vert page center from 0,0 */
static int trim_x;				/* Trim width */
static int trim_y;				/* Trim depth */
static int crop_vert_wt;		/* Crop mark thickness in VertBase */
static int crop_hor_wt;			/* Crop mark thickness in HorizBase */
static int crop_vert_space;		/* Crop or registermark depth in VBase */
static int crop_hor_space;		/* Crop or register mark width in HBase */
static int x_extra, y_extra;	/* Extra space between crop mark and trim */

/*********************************************************************/

int psinit(void)
{
    LC_LAY_REC *entry;
    LC_MEMB_REC *membentry;
    int lay_index, list_result;
	LLIST *mp_setup;

    psinit_entered_flag = 1;
    next_page = FirstPage;
    if(next_page < 0)
		next_page = 0;
    if(LastPage < 0)
		LastPage = 0;
    ResendPageFlag = 0;
    parse_mode = -1;

	if (BookPrint)				/* Doing whole-book output:  */
	{
		strcpy (ParentDirName, SubDirName);
    	if(QueMemListParent)
			LLclearItems(QueMemListParent);
    	else
			QueMemListParent = LLcreateList();
								/* Pull in member list of parent:  */
    	if(lc_lock_and_read_list_file(TreeName, DESK, ParentDirName,
						  	LC_LST_MEMBERS, QueMemListParent, FALSE) == 0)
    	{
			p_info (PI_ELOG,
				"OUTPUT failed to get members list for parent %s/%s\n",
			   	TreeName, ParentDirName);
			return(1);
    	}
								/* Find 1st requested prj in it:  */
		if ((membentry =
			 (LC_MEMB_REC *)find_member_filename(QueMemListParent, FirstProjofBook))
			== NULL)
		{
			p_info (PI_ELOG,
				"Did not find chapter %s for parent project %s/%s\n",
			   	FirstProjofBook, TreeName, ParentDirName);
			return(1);
		}
		ParentIndex = membentry->index;
		LastParentIndex = ParentIndex;
		strcpy (SubDirName, FirstProjofBook);
		strcpy (NextDirName, FirstProjofBook);
								/* If requested last prj diff from
									1st, find that one too:  */
		if (strcmp (FirstProjofBook, LastProjofBook) && 
			(membentry =
			 (LC_MEMB_REC *)find_member_filename(QueMemListParent, LastProjofBook))
			== NULL)
		{
			p_info (PI_ELOG,
				"Did not find ending chapter %s for parent project %s/%s\n",
			   	LastProjofBook, TreeName, ParentDirName);
		}
		else
			if (membentry->index > ParentIndex)
				LastParentIndex = membentry->index;
    }

    if(QueMemList)
		LLclearItems(QueMemList);
    else
		QueMemList = LLcreateList();
    if(lc_lock_and_read_list_file(TreeName, DESK, SubDirName,
								  LC_LST_MEMBERS, QueMemList, FALSE) == 0)
    {
		p_info(PI_ELOG, "OUTPUT failed to get members list for %s/%s/%s\n",
			   TreeName, SubDirName, JobName);
		return(1);
    }

    if(QueLayList)
		LLclearItems(QueLayList);
    else
		QueLayList = LLcreateList();
    if(lc_lock_and_read_list_file(TreeName, DESK, SubDirName,
								  LC_LST_LAYOUTS, QueLayList, FALSE) == 0)
    {
		p_info(PI_ELOG, "OUTPUT failed to get layout list for %s/%s/%s\n",
			   TreeName,SubDirName,JobName);
		return(1);
    }
	LLSORTED(QueLayList) = FALSE;
	if (MasterNameFlag)
		LLsort(QueLayList, compare_filename); /* sort lays by file name */
	else
		LLsort(QueLayList, compare_layout_real_page); /* sort lays by pg nbr */
	if (BookPrint)			/* Doing parent/child print: Find first  */
	{						/*   page number in first child project.  */
		entry = (LC_LAY_REC *)LLhead (QueLayList, &lay_index);
		while (entry && entry->flag==0 &&
			   (atoi(entry->PageNum) < 1))
			entry = (LC_LAY_REC *)LLnext(QueLayList, &lay_index);
		if (!entry || entry -> flag > 0)	
		{
			p_info(PI_ELOG, "OUTPUT: layout list for %s/%s has no MP pages.\n",
	   			TreeName,SubDirName);
			return(1);
		}
    	next_page = atoi(entry->PageNum);
	}

	strcpy (uBR_UnitName, "default");
	strcpy (uCH_UnitName, "default");
	strcpy (uFN_UnitName, "default");
	strcpy (uIL_UnitName, "default");
	strcpy (uSN_UnitName, "default");
	strcpy (uVS_UnitName, "default");
	mp_setup = LLcreateList();
	if (BookPrint)		/* Parent/child print: Use parent's styles.  */
    	list_result = lc_lock_and_read_list_file(TreeName, DESK, 
						ParentDirName, LC_LST_MP_SETUP, mp_setup, FALSE);
	else				/* Otherwise use project's styles.  */
    	list_result = lc_lock_and_read_list_file(TreeName, DESK, 
						SubDirName, LC_LST_MP_SETUP, mp_setup, FALSE);
    if (list_result)
	{
		for(entry = LLhead(mp_setup, &lay_index);
			entry;
			entry = LLnext(mp_setup, &lay_index))
		{
			if( (entry -> flag == 1) && (entry -> type <= -11) &&
				(entry -> type >= -16) ) /* MasterPage and style? */
			{
				switch (entry -> type)
				{
				  case -16:
					strcpy (uBR_UnitName, entry -> filename) ;
					break;
				  case -15:
					strcpy (uIL_UnitName, entry -> filename) ;
					break;
				  case -14:
					strcpy (uVS_UnitName, entry -> filename) ;
					break;
				  case -13:
					strcpy (uSN_UnitName, entry -> filename) ;
					break;
				  case -12:
					strcpy (uFN_UnitName, entry -> filename) ;
					break;
				  case -11:
					strcpy (uCH_UnitName, entry -> filename) ;
					break;
				}				/* end switch(entry->type) */
			}					/* end if MasterPage and style */
		}						/* end LC_LAY_REC loop */
	}
	LLclear(mp_setup);

#ifdef TRACE
    if(parse_trace)
    {
		p_info(PI_TRACE, "leave psinit with next_page= %d\n",
			   next_page);
		LLtrace(QueLayList, print_layout);
		LLtrace(QueMemList, print_member);
    }
#endif
    return(0);
}								/* end function */

/**********************************************************************/

void get_next_piece(void)
{
	int16 sv;

	if ( !crop_flag)
	{
#ifdef TRACE
		if(parse_trace)
			p_info(PI_TRACE, "start get_next_piece(), dly_rec_flg= %d, delay_mode= %d\n",
				   delay_records_flag, delay_mode);
#endif
		if (psinit_entered_flag && (delay_records_flag > 0) )
		{
			psinit_entered_flag = 0;
			delay_mode = 0;
		}
		if (delay_records_flag > 0)
			delay_records();
		else
			ps_get_page();
		CurrentFrame = PsCurRec -> frame_nbr;
		CurrentEle = PsCurRec -> ele_nbr;
		wn = PsCurRec -> frame_wn;
	}							/* end if(!crop_flag) */
	switch (PsCurRec -> frame_type)
	{
	  case 3:					/* crop marks */
		if ( !crop_flag)
		{
			crop_flag = 1;
			crop_mode = 0;
			crop_type = (KeyTrimFlags & 3);
			crop_weight = KeyTrimWeight;
			crop_wid = trim_mark_width;
			crop_dep = trim_mark_depth;
			crop_cx = lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN)
				+ (lmt_off_to_abs(wn, X_REF, PAGE_WIDTH) / 2) + crop_wid;
			crop_cy = lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN)
				+ (lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH) / 2) + crop_dep;
		}
		if ( !do_crops())
			crop_flag = 0;		/* end of crop marks */
		break;

	  case 4:					/* rule/box */
		SetX = trim_mark_width + PsCurRec->elem->rect_left;
		SetY  = trim_mark_depth + PsCurRec->elem->rect_top;
		get_frame_params();
		dashwid = 0;
		gapwid = 0;
		RuleWid = PsCurRec -> elem -> rect_right -
			PsCurRec -> elem -> rect_left;
		if(RuleWid < 0)
			RuleWid = 0;
		RuleHgt = PsCurRec -> elem -> rect_bottom -
			PsCurRec -> elem -> rect_top;
		if(RuleHgt < 0)
			RuleHgt = 0;
		sv = wn -> yx_convert[0];
		wn -> yx_convert[0] = 270;
		RuleWt = lmt_off_to_abs(wn, Y_REF, LEADING(CurrentFrame));
		wn -> yx_convert[0] = sv;
		if (DASH_ON(CurrentFrame)) /* Rule/Box Dashes */
		{
			dashwid = (lmt_off_to_abs(wn, Y_REF, DASHWID(CurrentFrame))/10);
			gapwid = (lmt_off_to_abs(wn, Y_REF, GAP_WID(CurrentFrame))/10);
		}
		if (RuleWt == 0)
			RuleType = 2;		/* solid box */
		else if (RuleWid == RuleWt)
			RuleType = 0;		/* vertical rule */
		else
			RuleType = 1;		/* horiziontal rule */
		break;

	  case 7:					/* text */
		SetX = trim_mark_width + PsCurRec->orig_elem->rect_left;
		SetY  = trim_mark_depth + PsCurRec->orig_elem->rect_top;
		SetY_Bottom = trim_mark_depth + PsCurRec->orig_elem->rect_bottom;
		if (wn != PsCurRec->text_wn)
		{
			wn = PsCurRec -> text_wn;
			CurrentFrame = PsCurRec -> frame_nbr;
		}
		get_frame_params();
		odashwid = 0;
		ogapwid = 0;
		if (ODASHON(CurrentFrame))
		{
			odashwid = (lmt_off_to_abs(wn, Y_REF, ODASHWD(CurrentFrame))/10);
			ogapwid = (lmt_off_to_abs(wn, Y_REF, OGAPWID(CurrentFrame))/10);
		}
		wn = PsCurRec -> frame_wn;
		CurrentFrame = PsCurRec -> orig_frame_nbr;
		break;

	  case 8:					/* graphic */
        SetX = trim_mark_width + FRAME_DATA(CurrentFrame) left;
        SetY  = trim_mark_depth + FRAME_DATA(CurrentFrame) top;
		get_frame_params();
		odashwid = 0;
		ogapwid = 0;
		if (ODASHON(CurrentFrame))
		{
			odashwid = (lmt_off_to_abs(wn, Y_REF, ODASHWD(CurrentFrame))/10);
			ogapwid = (lmt_off_to_abs(wn, Y_REF, OGAPWID(CurrentFrame))/10);
		}

		break;
	} /* End switch */
	if(parse_trace == 0)
    	return;
    trace_idtape(PsCurRec);
}								/* end function */

/********************************************************************/
void delay_records(void)
{ 
/* POSTSCRIPT delay mode:		0 = Output crop_marks and backgrounds only
								1 = graphics, if any
								2 = anything but graphics */
	for ( ;; )
	{
		ps_get_page();
		switch (delay_mode)
		{
		  case 0:				/* Output crop marks and backgrounds only */
			switch (PsCurRec -> frame_type)
			{
			  case 0:			/* END OF FILE. start next pass */
				return;
			  case 2:			/* END PAGE. start next pass */
				BGOutputFlag = 0;
				if ( graphic_count)
					delay_mode = 1;
				else
					delay_mode = 2;
				PsCurRec = PsHead;
				parse_mode = 1;
#ifdef TRACE
				if(parse_trace)
					p_info(PI_TRACE, "end delay mode case 0, new delay_mode= %d, graphic count= %d\n",
						   delay_mode, graphic_count);
#endif
				continue;

			  case 1:			/*NEW PAGE. Output it */
				graphic_count = 0;
				return;			/* Output the NEW PAGE */

			  case 3:			/* CROP MARKS. Output them */
				return;

			  case 8:			/* GRAPHIC. */
				graphic_count += 1;
			  case 6:			/* DESIGN TEXT */
			  case 7:			/* TEXT LINES.  */
				BGOutputFlag = 1; /* only allow BG output colors */
				return;			/* output for pass 0, background only */

			  default:
				continue;
			}					/* end switch(PsCurRec->frame_type) */
		  case 1:				/* output a graphic unless FG_LAYER == 1 */
			switch (PsCurRec -> frame_type) 
			{
			  case 0:			/* END OF FILE. start next pass */
			  case 2:			/* END PAGE. start next pass */
				delay_mode = 2;
				graphic_count = 0;
				PsCurRec = PsHead;
				parse_mode = 1;
#ifdef TRACE
				if(parse_trace)
					p_info(PI_TRACE, "end delay mode case 1, start case 2\n");
#endif
				continue;

			  case 8:			/* GRAPHIC. */
				graphic_count -= 1;
				if ( !graphic_count &&
					 !(FRAME_FLAGS(PsCurRec -> frame_nbr) &  FG_LAYER))
				{
#ifdef TRACE
					if(parse_trace )
						p_info(PI_TRACE, "final graphic being output.\n");
#endif
				}
				if ( !(FRAME_FLAGS(PsCurRec -> frame_nbr) & FG_LAYER))
					return;			/* output graphic for pass 1 */
/* fall thru and don't output it if flag is 1 */

			  default:
				if ( !graphic_count)
				{
#ifdef TRACE
				if(parse_trace)
					p_info(PI_TRACE, "skipping to end of page\n");
#endif
					PsCurRec = PsTail; /* point to end of page. */
					parse_mode = 1;
				}
				continue;
			}					/* end switch(PsCurRec->frame_type) */

		  case 2:				/* trying to output remainder of a page */
			switch (PsCurRec -> frame_type) 
			{
			  case 0:			/* END OF FILE */
			  case 2:			/* END PAGE. start next pass 0 */
				delay_mode = 0;
				graphic_count = 0;
#ifdef TRACE
				if(parse_trace)
					p_info(PI_TRACE, "end delay mode case 2\n");
#endif
				return;			/* output end page/file for pass 2 */

			  case 1:			/* NEW PAGE */
			  case 3:			/* CROP MARKS */
				continue;		/* no output */

			  case 8:			/* GRAPHIC. */
				if ( FRAME_FLAGS(PsCurRec -> frame_nbr) & FG_LAYER)
					return;		/* output graphic if flag = 1 */
				else
					continue;	/* no graphic output if flag = 0 */

			  default:
				return;			/* output other frames for pass 2 */
			}					/* end switch(PsCurRec->frame_type) */
		}						/* end switch(delay_mode) */
	}							/* end for(;;) */
}								/* end function */

/*********************************************************************/
static void ps_get_page(void)
{
    int i;
    
    for(;;)
    {
/*
printf ("In ps_get_page(): parse_mode=%d, next_page=%d, ResendPageFlag=%d, LastPage=%d.\n",
parse_mode, next_page, ResendPageFlag, LastPage);
*/
		switch (parse_mode)
		{
		  case -2:				/* end of file	*/
			ps_end_file.frame_type = 0;
			PsCurRec = (struct ps_pieces *)&ps_end_file;
			return;

		  case -1:				/* need a new page */
			if( (next_page < 0) && !ResendPageFlag)
			{
				parse_mode = -2; /* end of file */
				continue;
			}
			if ( (next_page >= 0) || ResendPageFlag)
			{					/* if there is another color on the page
								   just set, then do it again */
				if ( !ResendPageFlag)
				{
					if(LastPage)
					{
						if(next_page > LastPage)
						{
							parse_mode = -2; /* end of file */
							continue;
						}
					}
					i = psbuild(next_page);
					if (i)
					{
						p_info(PI_ELOG, "Error building page %d\n",next_page);
						parse_mode = -2; /* end of file */
						continue;
					}
					next_page = next_dot_page_page;
				}				/* end not another color */
				PsCurRec = PsHead;
				parse_mode = 0;	/* get first record */
				return;
			}					/* end if((next_page>=0)||ResendPageFlag) */
			
		  case 0:				/* get next rec */
			PsCurRec = PsCurRec -> next;
/* fall thru */
		  case 1:				/* getting the first record */
			if ( parse_mode)
				parse_mode = 0;
			if ( PsCurRec)
				return;
			parse_mode = -1;	/* end page, get new one or restart current */
			continue;
		}						/* end switch(parse_mode) */
    }							/* end for(;;)*/
}								/* end function */

/*********************************************************************/
static int do_crops(void)
{

    for(;;)
    {
#ifdef TRACE
		if (parse_trace)
			p_info(PI_TRACE, "MODE: %d\n",
				   crop_mode);
#endif
		
		switch(crop_mode)
		{
		  case 0:
			x_extra = KeyTrimRegGap * HorizontalBase;
			y_extra = KeyTrimRegGap * VerticalBase;
			trim_x = lmt_off_to_abs(wn, X_REF, TRIM_WIDTH) + trim_mark_width;
			trim_y = lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH) + trim_mark_depth;
			crop_hor_wt = (crop_weight * HorizontalBase) / 20;
			crop_vert_wt = (crop_weight * VerticalBase) / 20;
			crop_vert_space = KeyTrimLength * VerticalBase;
			crop_hor_space = KeyTrimLength * HorizontalBase;
#ifdef TRACE
			if(parse_trace)
				p_info(PI_TRACE, "crop type %d  weight %d  cenx %d  ceny %d  wid %d  dep %d, x_ex= %d, y_ex=%d\n",
					   crop_type,crop_weight,crop_cx,crop_cy,
					   crop_wid,crop_dep,x_extra,y_extra);
#endif
			
		  case 1:				/* TL | */
			if(crop_type & 1)
			{
				crop_mode = 1;
				crop_set(0, trim_mark_width, trim_mark_depth - y_extra -
						 crop_vert_space);
				crop_mode = 2;
				return(1);
			}
		  case 2:				/* TR | */
			if(crop_type & 1)
			{
				crop_mode = 2;
				crop_set(0, trim_x - crop_hor_wt, trim_mark_depth - y_extra -
						 crop_vert_space);
				crop_mode = 3;
				return(1);
			}
		  case 3:				/* TL _ */
			if(crop_type & 1)
			{
				crop_mode = 3;
				crop_set(1, trim_mark_width - crop_hor_space - x_extra,
						 trim_mark_depth);
				crop_mode = 4;
				return(1);
			}
		  case 4:				/* TR _ */
			if(crop_type & 1)
			{
				crop_mode = 4;
				crop_set(1, trim_x + x_extra, trim_mark_depth);
				crop_mode = 5;
				return(1);
			}
		  case 5:				/* BL _ */
			if(crop_type & 1)
			{
				crop_mode = 5;
				crop_set(1, trim_mark_width - crop_hor_space - x_extra, 
						 trim_y - crop_vert_wt);
				crop_mode = 6;
				return(1);
			}
		  case 6:				/* BR _ */
			if(crop_type & 1)
			{
				crop_mode = 6;
				crop_set(1, trim_x + x_extra, trim_y - crop_vert_wt);
				crop_mode = 7;
				return(1);
			}
		  case 7:				/* BL | */
			if(crop_type & 1)
			{
				crop_mode = 7;
				crop_set(0, trim_mark_width, trim_y + y_extra);
				crop_mode = 8;
				return(1);
			}
		  case 8:				/* BR | */
			if(crop_type & 1)
			{
				crop_mode = 8;
				crop_set(0, trim_x - crop_hor_wt, trim_y + y_extra);
				crop_mode = 9;
				if ( !(crop_type & 2) )
					return(0);	/* end trim, no crops, final element */
				return(1);		/* register marks next */
			}
		  case 9:				/* TOP REGISTER */
			if(crop_type & 2)
			{
				crop_vert_space = KeyRegisterLength * VerticalBase;
				crop_hor_space = KeyRegisterLength * HorizontalBase;
				crop_mode = 9;
				crop_set(5, crop_cx, trim_mark_depth - y_extra - 
						 crop_vert_space);
				crop_mode = 10;
				return(1);
			}
		  case 10:				/* LEFT REGISTER */
			if(crop_type & 2)
			{
				crop_mode = 10;
				crop_set(5, trim_mark_width - x_extra - crop_hor_space,
						 crop_cy);
				crop_mode = 11;
				return(1);
			}
		  case 11:				/* RIGHT REGISTER */
			if(crop_type & 2)
			{
				crop_mode = 11;
				crop_set(5, trim_x + x_extra + crop_hor_space, crop_cy);
				crop_mode = 12;
				return(1);
			}
		  case 12:				/* BOTTOM REGISTER */
			if(crop_type & 2)
			{
				crop_mode = 12;
				crop_set(5, crop_cx, trim_y + y_extra + crop_vert_space);
				crop_mode = 13;
				return(0);		/* end of trim and register marks */
			}
		  case 13:				/* END  */
#ifdef TRACE
			if(parse_trace)
				p_info(PI_TRACE, "END crop marks");
#endif
			return(0);
		}						/* end switch(crop_mode) */
    }							/* end for(;;) */
}								/* end func do_crops() */

/*********************************************************************/
static void crop_set(int type, int setx, int sety)
{

	RuleType = type;
    SetX = setx;
    SetY = sety;
    switch (type)
    {
	  case 0:					/* vertical rule */
		RuleWid = 0;
		RuleHgt = KeyTrimLength * VerticalBase;
		RuleWt = crop_weight;
		break;
	  case 1:					/* horiz rule */
		RuleWid = KeyTrimLength * HorizontalBase;
		RuleHgt = 0;
		RuleWt = crop_weight;
		break;
	  case 5:					/* register mark */
		break;
    }							/* end switch(type) */

#ifdef TRACE
    if (parse_trace)
		p_info (PI_TRACE, "MODE %d -- SetX: %d, SetY: %d, Wid: %d, Hgt: %d, Wt: %d, Type: %d\n",
				crop_mode, SetX, SetY, RuleWid, RuleHgt, RuleWt, type);
#endif
}								/* end func crop_set() */

/*********************************************************************/
void trace_idtape(struct ps_pieces *current_rec)
{
	switch (current_rec ->frame_type)
	{
	  case 0:
		p_info(PI_TRACE, "\tcase 0 - end file ");
		break;
	  case 1:
		p_info(PI_TRACE, "\tcase 1 - new page ");
		break;
	  case 2:
		p_info(PI_TRACE, "\tcase 2 - end page ");
		break;
	  case 3:
		p_info(PI_TRACE, "\tcase 3 - crop mrk ");
		break;
	  case 4:
		p_info(PI_TRACE, "\tcase 4 - box/rule ");
		break;
	  case 6:
		p_info(PI_TRACE, "\tcase 6 - ovfl txt ");
		break;
	  case 7:
		p_info(PI_TRACE, "\tcase 7 - text     ");
		break;
	  case 8:
		p_info(PI_TRACE, "\tcase 8 - graphic  ");
		break;
	}							/* end switch(current_rec->frame_nbr) */
	p_info(PI_TRACE, "frame nbr %d, ele nbr %d, type %d, ele* %x\n",
		   current_rec -> frame_nbr, current_rec -> ele_nbr,
		   current_rec -> frame_type, (unsigned)current_rec -> elem);
}								/* end function */

/*********************************************************************/
/* This routine is used to get the rotation angle and center of a frame. */
/* Called ONLY from psdtext.c:ps_set_rotation().		*/
void find_mu_rot(int i)			/* set up machine unit rotation for block i */
{
	int t,b,l,r;				/* top bottom left right */
								/* i13 holds Rotational Degrees  */
	if((mu_rot_degree = (i ? REL_DATA(i) i13 : 0)) == 0)
		return;
/* Set up t,b,l,r in units of measure base * leading base */
/* Don't get values from wn, which is from PsCurRec->text_wn, which
	differs from PsCurRec->frame_wn in the case of an object frame.
	Instead get these 4 directly from frame_wn.	*/
	t = LAYOUT(PsCurRec -> frame_wn) frames[i] -> top;
	b = LAYOUT(PsCurRec -> frame_wn) frames[i] -> bottom;
	l = LAYOUT(PsCurRec -> frame_wn) frames[i] -> left;
	r = LAYOUT(PsCurRec -> frame_wn) frames[i] -> right;
	
	switch(ROT_LOCKPOINT(i))
	{
	  case 0x11:				/* TL */
		mu_rot_y = t;
		mu_rot_x = l;
		break;
	  case 0x13:				/* TC */
		mu_rot_y = t;
		mu_rot_x = ((l + r) / 2);
		break;
	  case 0x12:				/* TR */
		mu_rot_y = t;
		mu_rot_x = r;
		break;
	  case 0x31:				/* CL */
		mu_rot_y = ((t + b) / 2);
		mu_rot_x = l;
		break;
	  case 0x33:				/* CC */
		mu_rot_y = ((t + b) / 2);
		mu_rot_x = ((l + r) / 2);
		break;
	  case 0x32:				/* CR */
		mu_rot_y = ((t + b) / 2);
		mu_rot_x = r;
		break;
	  case 0x21:				/* BL */
		mu_rot_y = b;
		mu_rot_x = l;
		break;
	  case 0x23:				/* BC */
		mu_rot_y = b;
		mu_rot_x = ((l + r) / 2);
		break;
	  case 0x22:				/* BR */
		mu_rot_y = b;
		mu_rot_x = r;
		break;
	  default:
		mu_rot_degree = 0;
		mu_rot_x = 0;
		mu_rot_y = 0;
		break;
	}							/* end switch(ROT_LOCKPOINT(i)) */
	if (mu_rot_degree)
	{
		mu_rot_x += trim_mark_width;
		mu_rot_y += trim_mark_depth;
	}
}								/* end function */

/*********************************************************************/
static void get_frame_params(void)
{
	FgClr = FG_COLOR(CurrentFrame);
	FgShd = FG_SHADE(CurrentFrame);
	BgClr = BG_COLOR(CurrentFrame);
	BgShd = BG_SHADE(CurrentFrame);
	ps_set_rotation();
	BgBlendStartClr = BLEND_START(CurrentFrame);
	BgBlendStartShd = BLEND_SSHADE(CurrentFrame);
	BgBlendEndClr = BLEND_END(CurrentFrame);
	BgBlendEndShd = BLEND_ESHADE(CurrentFrame);
	BlendAngle = BLEND_ANGLE(CurrentFrame);
	if ( !BgBlendStartShd)
		BgBlendStartShd = 100;
	if ( !BgBlendEndShd)
		BgBlendEndShd = 100;
}

/**********************************************************************/
/**** EOF ****/
