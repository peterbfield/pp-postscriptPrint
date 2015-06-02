#include <stdio.h>				/* so can use Pfd type */
#include "p_lib.h"
#include "traces.h"
#include "psjob.h"
#include "frame.h"
#include "rel_data.h"

static void chk_ly_depth(int inc);
static void mpu_string_location(int x_pos, int y_pos, char *string);
static void save_mpu_lprint (void);

static int lpage;
static struct mpu_lp *head;		/* pointer to first mpu_txp structure */
static struct mpu_lp *tail;		/* pointer to last mpu_txp structure */

extern int mpu_next_line;				/* index into ln_prt_buff[nl][] */
extern char ln_prt_buff[200][132];
extern int LineCounter;
extern int U_PrtCharDepth;
extern int U_PrtCharWid;
extern int FoFileType;
extern int RuleType;
extern int ly_page_started_flag;
extern char uBR_UnitName[];
extern char uBR_PageName[];
extern char uCH_UnitName[];
extern char uFN_UnitName[];
extern char uSN_UnitName[];
extern char uIL_UnitName[];
extern char uIL_PageName[];
extern char uVS_UnitName[];
extern char uVS_PageName[];
extern char uBR_FrameName[];

/********************************************************/
void mpu_layout_print(int frame, ELEMENT *ele)
{
	int x_pos, y_pos;
	char string1[64], string2[64], string3[64], string4[64];
	float frame_trap_weight;
	int16 temp_frame_trap_color, sv;
	WYSIWYG *save_frame_wn;

	if ( !frame)
	{
		ly_page_started_flag |= 2; /* 1 = report 3, 2 = report, 3 = both */
		memset(&ln_prt_buff[0][0], 040, sizeof(ln_prt_buff));
		lpage = 1;
		if ( MasterNameFlag)
			sprintf(&ln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   MASTER   (page %d)",
					LayoutName, TreeName, SubDirName, lpage);
		else
			sprintf(&ln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   PAGE %d  (page %d)",
					LayoutName, TreeName, SubDirName, PageNo1, lpage);
		x_pos = lmt_off_to_abs(wn, X_REF, TRIM_WIDTH);
		y_pos = lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH);
		mpu_string_location (x_pos, y_pos, string1);
		x_pos = lmt_off_to_abs(wn, X_REF, PAGE_WIDTH);
		y_pos = lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH);
		mpu_string_location (x_pos, y_pos, string2);
		x_pos = lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
		y_pos = lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
		mpu_string_location (x_pos, y_pos, string3);
		sprintf(&ln_prt_buff[2][0],
				"SPECS:  TRIM %s  PAGE SIZE %s PAGE ORIGIN %s",
				string1, string2, string3);
		mpu_next_line = 3;
		if ( uBR_PageName[0])
			sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE BREAKING STYLE: %s",
					uBR_PageName);
		else
			sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE BREAKING STYLE: %s",
					uBR_UnitName);
		sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE CHAPTER STYLE: %s",
				uCH_UnitName);
		if ( uIL_PageName[0])
			sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE ILLUSTRATION STYLE: %s",
					uIL_PageName);
		else
			sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE ILLUSTRATION STYLE: %s",
					uIL_UnitName);
		sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE FOOTNOTE STYLE: %s",
				uFN_UnitName);
		sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE SIDENOTES STYLE: %s",
				uSN_UnitName);
		if ( uVS_PageName[0])
			sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE VERTICAL SPACE STYLE: %s",
					uVS_PageName);
		else
			sprintf(&ln_prt_buff[mpu_next_line++][2], "MASTERPAGE VERTICAL SPACE STYLE: %s",
					uVS_UnitName);
		mpu_next_line++;
		return;
	}
	switch(PsCurRec ->frame_type)
	{
	  case 4:					/* Rule/Box */
		switch (RuleType)
		{
		  case 0:				/* vertical rule */
			strcpy(string1, "VERT RULE");
			break;
		  case 1:
			strcpy(string1, "HORZ RULE");
			break;
		  case 2:
			strcpy(string1, "SOLID BOX");
			break;
		  default:
			strcpy(string1, "UNKNOWN BOX");
			break;
		}						/* end switch(RuleType) */
		break;					/* end case 4, rule/box */
		
	  case 8:					/* graphic */
		sprintf (string3,"%-s", GRAPHIC_NAME(CurrentFrame)); /* Get name */
		if ( string3[0])
			sprintf(string1, "GRAPHIC      NAME %s", string3);
		else
			sprintf(string1, "GRAPHIC      NO NAME");
		break;
		
	  case 7:					/* text */
		if ( (FoName[0] == ' ') || !FoName[0] )
			sprintf(string1, "TEXT         NO ARTICLE");
		else
		{
			save_frame_wn = wn;
			wn = PsHead ->frame_wn;
			if ( OBJ_REF(PsCurRec -> orig_frame_nbr) )
			{							/* object frame, only print message */
				sprintf(string1, "OBJ REFERENCE: %ld; %ld",
						(OBJ_REF(PsCurRec-> orig_frame_nbr) >> 12)&MP_OB_LAY,
						OBJ_REF(PsCurRec -> orig_frame_nbr) & MP_OB_FRAME);
			}
			else if ( FoFileType == LFO_FILE )
				sprintf(string1, "DESIGN TEXT  ARTICLE %s",FoName);
			else
				sprintf(string1, "TEXT         ARTICLE %s",FoName);
			wn = save_frame_wn;
		}
		break;
	}							/* end switch(PsCurRec ->frame_type) */
	if (FRAME_FLAGS(CurrentFrame) & OUTLINE)
	{					/* frame trap requested */
		temp_frame_trap_color = OUT_COLOR(CurrentFrame); 
		sv = wn -> yx_convert[0];
		wn -> yx_convert[0] = 270; /* 20ths of a point */
		frame_trap_weight =
			lmt_off_to_abs(wn, 0, OUT_WEIGHT(CurrentFrame)) / 20;
		wn -> yx_convert[0] = sv;
		sprintf(string2, "OUTLINE: COLOR %d, WEIGHT %4.2f",
				temp_frame_trap_color, frame_trap_weight);
	}
	else
		sprintf(string2, " ");
	if (  strcmp(uBR_FrameName, "default") && (PsCurRec ->frame_type == 7) )
		chk_ly_depth(3);
	else
		chk_ly_depth(2);
	sprintf(&ln_prt_buff[mpu_next_line++][3], "FRAME %d  TYPE %s  %s",
			frame, string1, string2);
	x_pos = ele -> rect_left;
	y_pos = ele -> rect_top;
	mpu_string_location (x_pos, y_pos, string1); /* top, left */
	x_pos = ele -> rect_right - ele -> rect_left;
	y_pos =  ele -> rect_bottom - ele -> rect_top;
	mpu_string_location (x_pos, y_pos, string2); /*wid, dep*/
	sprintf (string3, "FG %d  BG %d ",
			 FG_COLOR(frame), BG_COLOR(frame));
	if ( (PsCurRec -> frame_type != 8) && (FRAME_FLAGS(frame) & FG_ATTRIB) )
		strcat (string3, "  TRANSPARENT");
	else
		strcat (string3, "");
	if (PsCurRec ->frame_type != 4)
	{
		if ( FRAME_FLAGS(frame) & LAYER)
			sprintf(string3, "  LAYERED");
	}
	if( PageRotationAngle)
		sprintf(string4, "  ROTATED %d", -PageRotationAngle);
	else
		string4[0] = 0;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "ORIGIN %s  SIZE %s  %s%s",
			string1, string2, string3, string4);
	if (  strcmp(uBR_FrameName, "default") && (PsCurRec ->frame_type == 7) )
		sprintf(&ln_prt_buff[mpu_next_line++][5], "MASTERPAGE BREAKING STYLE: %s",
				uBR_FrameName);
	mpu_next_line++;
}								/* end function */

/********************************************************/
static void chk_ly_depth (int inc)
{
	if((mpu_next_line + inc) >= U_PrtCharDepth)
	{
		save_mpu_lprint ();
		memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
		lpage++;
		if ( MasterNameFlag)
			sprintf(&ln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   MASTER  (page %d)",
					LayoutName, TreeName, SubDirName, lpage);
		else
			sprintf(&ln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   PAGE %d  (page %d)",
					LayoutName, TreeName, SubDirName, PageNo1, lpage);
		mpu_next_line = 2;
	}
}								/* end function */

/********************************************************/
static void mpu_string_location (int x_pos, int y_pos, char *string)
{
	int fract_x, fract_y;		/* in hundreths */
	int tmp_x, tmp_y;
	int pi1, pi2;
	int pt1, pt2;
	char ptstr1[12] = {""};
	char ptstr2[12] = {""};
	char frstr1[12] = {""};
	char frstr2[12] = {""};

	tmp_x = x_pos / HorizontalBase;
	tmp_y = y_pos / VerticalBase;
	pi1 = tmp_x / 12;
	pt1 = tmp_x % 12;
	fract_x = ((x_pos % HorizontalBase) * 100) / HorizontalBase;
	if ( !(fract_x % 10))
		fract_x /= 10;
	pi2 = tmp_y / 12;
	pt2 = tmp_y % 12;
	fract_y = ((y_pos % VerticalBase) * 100) / VerticalBase;
	if ( !(fract_y % 10))
		fract_y /= 10;
	itoa (pt1, ptstr1);
	if (fract_x)
		sprintf (frstr1,".%d",fract_x);
	itoa (pt2,ptstr2);
	if (fract_y)
		sprintf (frstr2,".%d",fract_y);
	sprintf (string, "%dP%s%s x %dP%s%s", pi1, ptstr1, frstr1,
			 pi2, ptstr2, frstr2);
}								/* end function */
/******************************************************************/
void init_mpu_lprint (void)
{
	head = 0;
	tail = 0;
	lpage = 0;
}								/* end function */

/******************************************************************/
static void save_mpu_lprint (void)
{
	struct mpu_lp *current_rec;
	char *page_area;
	int i, j;

	if ( !mpu_next_line)
		return;
    current_rec = (struct mpu_lp *) p_alloc (sizeof (struct mpu_lp));
	page_area = p_alloc (mpu_next_line * U_PrtCharWid);
	if ( !current_rec )
	{
		p_info(PI_ELOG, "Allocate failed for txtfile print structure\n");
		return;
	}
	if ( !page_area )
	{
		p_info(PI_ELOG, "Allocate failed for txtfile print page\n");
		p_free ( (char *)current_rec);
		return;
	}
	current_rec -> next = 0;
	current_rec -> line_count = mpu_next_line;
	if ( !head )
		head = current_rec;
    else
		tail -> next = current_rec;	/* prev struct points to this new one */
	tail = current_rec;
    current_rec -> page_saved = page_area;
	for (i=0; i<mpu_next_line; i++)
	{
		for (j=0; j<U_PrtCharWid; j++)
		{
			*(page_area++) = ln_prt_buff[i][j]; /* save page */
		}
	}
}								/* end function */

/******************************************************************/
void output_mpu_lprint (void)
{
	struct mpu_lp *current_rec;
	char *page_area;
	int i, j, line_cnt;

	if ( mpu_next_line)
		save_mpu_lprint ();
	if (!head)
	{
		beg_PAGE();
		end_PAGE();
		return;
	}
	do
	{
		current_rec = head;
		page_area = current_rec -> page_saved;
		memset((char *)&ln_prt_buff[0][0], 040, sizeof(ln_prt_buff));
		line_cnt = current_rec -> line_count;
		for (i=0; i<current_rec -> line_count; i++)
		{
			for (j=0; j<U_PrtCharWid; j++)
			{
				ln_prt_buff[i][j] = *(page_area++); /* restore page */
			}
		}
		send_it (ln_prt_buff, &line_cnt);
		head = current_rec -> next;
		p_free (current_rec -> page_saved);
		p_free ( (char *)current_rec);
	}while ( head);
}								/* end function */

/********************************************************/
/******** EOF **********/
