#include <stdio.h>
#include <stdlib.h>
#include "window.h"
#include "mp_styles.h"
#include "psjob.h"
#include "list_control.h"

#define LINE_SIZE 256			/* max number or chars on a single line */
#define ALIAS_MAXCNT 256		/* max number of aliases */
#define ALIAS_MAX_LINESIZE 24	/* max number of chars in a single alias */

static void chk_depth(int inc);
static int16 cnvrt_ascii(int flag);
static char *compute_it(int16 val, int16 picas_and_pts, int16 space_fill, 
						int16 buff_idx);
static char *compute_ms(int16 val, int16 picas_and_pts, int16 space_fill,
						int16 buff_idx);
static void find_next_string(void);
static char *line_clean(char *ptr);
static void mpu_print_alias_list(void);

int mpu_next_line;				/* index into ln_prt_buff[nl][] */
char ln_prt_buff[200][132];
int mpu_page;
int LineCounter = 0;

static char buff[10][32];
static char LineIn[LINE_SIZE];
static char *BeginLinePtr;
static char *EndLinePtr;
static int LineCharCount;
static int EolFlag;
static char aliases[256][ALIAS_MAX_LINESIZE+2];
static struct mpu_file *cur_rec;
static struct mpu_file *nxt_rec;
static char tree[64];
static char dir[64];
static char parent[64];
static int count;

char buff1[10][32];

char *no_yes[]		= {
	"NO",
	"YES"
	};

char *yes_no[]		= {
	"YES",
	"NO"
	};

char *trs[]		= {
	"NULL",
	"1 = Text Related Heads",
	"2 = Continued Lines",
	"3 = Text Rel Heads and Continued Lines"
	};

char *begin_unit[]	= {
	"N/A",
	"NEW ODD",
	"NEW EVEN",
	"NEXT"
	};

char *try_fails[]	= {
	"SET SHORT PAGE",
	"SET SHORT PAGE",
	"SET THIS PAGE LONG, NEXT PAGE SHORT"
	};

char *align_opt[]	= {
	"N/A",
	"FACING EACH OTHER",
	"BACKING EACH OTHER",
	"COLUMNS"};

char *align_adj[]	= {
	"N/A",
	"SHORT ONLY",
	"LONG ONLY",
	"SHORT THEN LONG",
	"LONG THEN SHORT"};

char *align_folio_move[] = {
	"NEVER",
	"SHORT PAGE",
	"LONG PAGE",
	"LONG AND SHORT PAGES"};

char *last_pg_fn[]	= {
	"UNDER TEXT",
	"BOTTOM OF PAGE"};

char *fn_specific[]	= {
	"N/A    ",
	"INSIDE ",
	"OUTSIDE",
	"COLUMN "};

char *double_up[]	= {
	"2 ",
	"3 ",
	"4 ",
	">4"};

char *dbl_up_space[]	= {
	"USE SPACE   ",
	"SPACE EVENLY"};

char *none_space[]	= {
	"[NONE]",
	"      "};

extern struct mpu_file *br_head;
extern struct mpu_file *ch_head;
extern struct mpu_file *fn_head;
extern struct mpu_file *il_head;
extern struct mpu_file *sn_head;
extern struct mpu_file *vs_head;
extern char uMemberName[];
extern char uLayoutName[];
extern char U_TreeName[];
extern char U_DirName[];
extern char U_DataName[132];
extern BR_STYLE uBR_style;
extern CH_STYLE uCH_style;
extern FN_STYLE uFN_style;
extern IL_STYLE uIL_style;
extern SN_STYLE uSN_style;
extern VS_STYLE uVS_style;
extern WYSIWYG *u_wn;
extern int U_PrtCharDepth;
extern int U_PrtCharWid;
extern char uReportType[];

/**************************** Database Print ****************************/

void mpu_data_print_br(void)
{
	int found = 0;

	memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
	sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE%s  (page %d) FOR %s",
			uReportType, mpu_page, U_DataName);
	mpu_next_line = 1;
	if ( !( cur_rec = br_head) )
	{							/* no data for report */
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "BREAKING STYLE:");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5],"[NO DATA FILES IN PROJECT]");
		chk_depth ( U_PrtCharDepth); /* force page out */
		return;
	}
	nxt_rec = cur_rec;
	while (nxt_rec)
	{							/* output report for all files */
		nxt_rec = cur_rec -> next;
		strcpy(tree, TreeName);
		strcpy(dir, SubDirName);
		for(count = 0; count <= 2; count++)
		{
			if(  !get_br_style(u_wn,&uBR_style, tree, dir, cur_rec->filename))
			{
				found = 1;
				break;
			}
			if(lc_parent(tree,dir,parent))
				p_parent_path(tree,dir,parent,tree,dir);
			else
				break;
		}
		if ( !found)
		{						/* file not present */
			chk_depth ( 2);
			mpu_next_line++;
			sprintf(&ln_prt_buff[mpu_next_line++][5], "BREAKING STYLE - FILE %s IS MISSING.",
					cur_rec -> filename);
			continue;
		}
		chk_depth ( 9);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "BREAKING STYLE - FILE %s",
				cur_rec -> filename);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Are Widows Allowed? [%s]    Widow Percent: [%d]%%",
				no_yes[uBR_style.allow_widows], uBR_style.widow_pct);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Are Orphans Allowed? [%s]",
				no_yes[uBR_style.allow_orphans]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Minimum Lines:  Above Head  [%d]   Below Head  [%d]",
				uBR_style.lines_above_head, uBR_style.lines_below_head);
		mpu_print_alias_list();
		mpu_next_line += 2;
		memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
		cur_rec = nxt_rec;
	}							/* end while(cur_rec) */
	if (mpu_next_line > 1)
		chk_depth ( U_PrtCharDepth); /* force partial page out */
}								/* end function */

/********************************************************/
void mpu_data_print_ch(void)
{
	int found = 0;

	memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
	sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE%s  (page %d) FOR %s",
			uReportType, mpu_page, U_DataName);
	mpu_next_line = 1;
	if ( !( cur_rec = ch_head) )
	{							/* no data for report */
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "CHAPTER STYLE:");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5],"[NO DATA FILES IN PROJECT]");
		chk_depth ( U_PrtCharDepth); /* force page out */
		return;
	}
	nxt_rec = cur_rec;
	while (nxt_rec)
	{							/* output report for all files */
		nxt_rec = cur_rec -> next;
		strcpy(tree, TreeName);
		strcpy(dir, SubDirName);
		for(count = 0; count <= 2; count++)
		{
			if(  !get_ch_style(u_wn,&uCH_style, tree, dir, cur_rec->filename))
			{
				found = 1;
				break;
			}
			if(lc_parent(tree,dir,parent))
				p_parent_path(tree,dir,parent,tree,dir);
			else
				break;
		}
		if ( !found)
		{						/* file not present */
			chk_depth ( 2);
			mpu_next_line++;
			sprintf(&ln_prt_buff[mpu_next_line++][5], "CHAPTER STYLE - FILE %s IS MISSING.",
					cur_rec -> filename);
			continue;
		}
		chk_depth ( 12);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "CHAPTER STYLE - FILE %s",
				cur_rec -> filename);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Begin Each Unit On A: [%s] Page",
				begin_unit[uCH_style.begin_unit_on]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Minimum Lines On Last Page: [%d]",
				uCH_style.min_last_pg_lines);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Align Pages Which Are:  [%s]",
				align_opt[uCH_style.auto_align]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Amount The Page Depth Can Go Short Or Long:  [%s]",
			compute_it(uCH_style.align_amt, 1, 0, 0));
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Adjust The Page Depth:  [%s]",
				align_adj[uCH_style.align_opt]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Return To Normal Depth After Short Or Long Page:  [%s]",
				yes_no[uCH_style.long_short_ok]);
		if (uCH_style.postproc_members == 1)
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Process All Members After Pagination:  [YES]");
		else
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Process All Members After Pagination:  [NO]");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][10], "When Try Fails:  [%s]",
				try_fails[uCH_style.allow_short_pg]);
		mpu_next_line++;
		memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
		cur_rec = nxt_rec;
	}							/* end while(cur_rec) */
	if (mpu_next_line > 1)
		chk_depth ( U_PrtCharDepth); /* force partial page out */
}								/* end function */

/********************************************************/
void mpu_data_print_fn(void)
{
	int16 i, j;
	int found = 0;

	memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
	sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE%s  (page %d) FOR %s",
			uReportType, mpu_page, U_DataName);
	mpu_next_line = 1;
	if ( !( cur_rec = fn_head) )
	{							/* no data for report */
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "FOOTNOTES:");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5],"[NO DATA FILES IN PROJECT]");
		chk_depth ( U_PrtCharDepth); /* force page out */
		return;
	}
	nxt_rec = cur_rec;
	while (nxt_rec)
	{							/* output report for all files */
		nxt_rec = cur_rec -> next;
		strcpy(tree, TreeName);
		strcpy(dir, SubDirName);
		for(count = 0; count <= 2; count++)
		{
			if(  !get_fn_style(u_wn,&uFN_style, tree, dir, cur_rec->filename))
			{
				found = 1;
				break;
			}
			if(lc_parent(tree,dir,parent))
				p_parent_path(tree,dir,parent,tree,dir);
			else
				break;
		}
		if ( !found)
		{						/* file not present */
			chk_depth ( 2);
			mpu_next_line++;
			sprintf(&ln_prt_buff[mpu_next_line++][5], "FOOTNOTES - FILE %s IS MISSING.",
					cur_rec -> filename);
			continue;
		}
		if(uFN_style.fn_place & 1)
			chk_depth ( 29);
		if(uFN_style.fn_place & 2)
			chk_depth ( 31);
		if(uFN_style.fn_place & 4)
			chk_depth ( 34);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "FOOTNOTES - FILE %s",
				cur_rec -> filename);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Footnote Placement Options:");
		if(uFN_style.fn_place & 1)
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Bottom of Column Where Referenced");
		else if(uFN_style.fn_place & 2)
		{
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Bottom Of Page:");
			sprintf(&ln_prt_buff[mpu_next_line++][12], "Number Of Footnote Columns:  [%d]",
					uFN_style.fn_cols);
			sprintf(&ln_prt_buff[mpu_next_line++][12], "Column Width:  [%s]   Gutter Width:  [%s]",
					compute_ms(uFN_style.fn_col_wid, 1, 0, 0),
					compute_ms(uFN_style.fn_gutter, 1, 0, 1));
		}
		else if(uFN_style.fn_place & 4)
		{
			i = uFN_style.fn_chp_even;
			if(i & 4)
				i = 3;
			j = uFN_style.fn_chp_even & 0xff;
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Specific Column:");
			sprintf(&ln_prt_buff[mpu_next_line++][12], "Chapter Even: [%s]  Column Number: [%d]",
					fn_specific[i], j);
			i = uFN_style.fn_chp_odd;
			if(i & 4)
				i = 3;
			j = uFN_style.fn_chp_odd & 0xff;
			sprintf(&ln_prt_buff[mpu_next_line++][12], " Chapter Odd: [%s]  Column Number: [%d]",
					fn_specific[i], j);
			i = uFN_style.fn_even;
			if(i & 4)
				i = 3;
			j = uFN_style.fn_even & 0xff;
			sprintf(&ln_prt_buff[mpu_next_line++][12], "   Even Page: [%s]  Column Number: [%d]",
					fn_specific[i], j);
			i = uFN_style.fn_odd;
			if(i & 4)
				i = 3;
			j = uFN_style.fn_odd & 0xff;
			sprintf(&ln_prt_buff[mpu_next_line++][12], "    Odd Page: [%s]  Column Number: [%d]",
					fn_specific[i], j);
		}
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Place Footnotes [%s] On Last Page",
				last_pg_fn[uFN_style.last_pg_fn_place]);
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Double Up Short Footnotes:  [%s]",
				no_yes[uFN_style.dbl_short_fns]);
		if(uFN_style.dbl_short_fns)
		{
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Number Of Footnotes On Line:  [%s]",
					double_up[uFN_style.dbl_num]);
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Space Between Short Footnotes:  [%s]",
					compute_ms(uFN_style.dbl_space, 1, 0, 0));
			sprintf(&ln_prt_buff[mpu_next_line++][10], "Spacing Options:  [%s]",
					dbl_up_space[uFN_style.dbl_up_evenly]);
		}
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Space Above:  [%s]  Below:  [%s]   Cut Off Rule",
				compute_it(uFN_style.space_above_cr, 1, 0, 0),
				compute_it(uFN_style.space_below_cr, 1, 0, 1));
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Length:  [%s]   Weight:  [%s]  of Cut Off Rule",
				compute_ms(uFN_style.length_of_cr, 1, 0, 0),
				compute_ms(uFN_style.weight_of_cr, 1, 0, 1) );
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Space Above:  [%s]   Below:  [%s]   Special Cut Off Rule",
				compute_it(uFN_style.space_above_scr, 1, 0, 0),
				compute_it(uFN_style.space_below_scr, 1, 0, 1));
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Length:  [%s]   Weight:  [%s]  of Special Cut Off Rule",
				compute_ms(uFN_style.length_of_scr, 1, 0, 0),
				compute_ms(uFN_style.weight_of_scr, 1, 0, 1) );
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Footnote Space Adjustment:  [%d]%% Above Rule, [%d]%% Below Rule",
				uFN_style.fn_space_place, 100 - uFN_style.fn_space_place);
		sprintf(&ln_prt_buff[mpu_next_line++][7], "Footnote Breaking Rules:");
		sprintf(&ln_prt_buff[mpu_next_line++][9], "Allow Program To Split Footnotes:  [%s]",
				no_yes[uFN_style.allow_foot_break]);
		sprintf(&ln_prt_buff[mpu_next_line++][11], "Widow/Orphan/Ep Rules:");
		sprintf(&ln_prt_buff[mpu_next_line++][13], "Column To Column (same page)");
		sprintf(&ln_prt_buff[mpu_next_line++][15], "Are Widows Allowed?  [%s]   Widow  [%d]%%",
				no_yes[uFN_style.ccfn_allow_widows], uFN_style.ccfn_widow_pct);
		sprintf(&ln_prt_buff[mpu_next_line++][15], "Are Orphans Allowed?  [%s]",
				no_yes[uFN_style.ccfn_allow_orphans]);
		sprintf(&ln_prt_buff[mpu_next_line++][15], "Allow Ep Break?  [%s]",
				no_yes[uFN_style.ccfn_allow_ep]);
		sprintf(&ln_prt_buff[mpu_next_line++][13], "Page To Page");
		sprintf(&ln_prt_buff[mpu_next_line++][15], "Are Widows Allowed?  [%s]   Widow  [%d]%%",
				no_yes[uFN_style.ppfn_allow_widows], uFN_style.ppfn_widow_pct);
		sprintf(&ln_prt_buff[mpu_next_line++][15], "Are Orphans Allowed?  [%s]",
				no_yes[uFN_style.ppfn_allow_orphans]);
		sprintf(&ln_prt_buff[mpu_next_line++][15], "Allow Ep Break?  [%s]",
				no_yes[uFN_style.ppfn_allow_ep]);
		sprintf(&ln_prt_buff[mpu_next_line++][9], "Minimum Text Depth With Broken Note:  [%s] ",
				compute_it(uFN_style.min_dep_w_broke_fn, 1, 0, 0));
		mpu_next_line++;
		memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
		cur_rec = nxt_rec;
	}							/* end while(cur_rec) */
	if (mpu_next_line > 1)
		chk_depth ( U_PrtCharDepth); /* force partial page out */
}								/* end function */

/********************************************************/
void mpu_data_print_sn(void)
{
	int found = 0;

	memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
	sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE%s  (page %d) FOR %s",
			uReportType, mpu_page, U_DataName);
	mpu_next_line = 1;
	if ( !( cur_rec = sn_head) )
	{							/* no data for report */
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "SIDENOTES and AREA REFERENCE NUMBERS:");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5],"[NO DATA FILES IN PROJECT]");
		chk_depth ( U_PrtCharDepth); /* force page out */
		return;
	}
	nxt_rec = cur_rec;
	while (nxt_rec)
	{							/* output report for all files */
		nxt_rec = cur_rec -> next;
		strcpy(tree, TreeName);
		strcpy(dir, SubDirName);
		for(count = 0; count <= 2; count++)
		{
			if(  !get_sn_style(u_wn,&uSN_style, tree, dir, cur_rec->filename))
			{
				found = 1;
				break;
			}
			if(lc_parent(tree,dir,parent))
				p_parent_path(tree,dir,parent,tree,dir);
			else
				break;
		}
		if ( !found)
		{						/* file not present */
			chk_depth ( 2);
			mpu_next_line++;
			sprintf(&ln_prt_buff[mpu_next_line++][5], "SIDENOTES and AREA REFERENCE NUMBERS - FILE %s IS MISSING.",
					cur_rec -> filename);
			continue;
		}
		chk_depth ( 10);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "SIDENOTES and AREA REFERENCE NUMBERS - FILE %s",
				cur_rec -> filename);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Sidenotes:  [%s]",
				no_yes[uSN_style.side_notes]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Let long SN run into next paragraph:  [%s]",
				yes_no[uSN_style.sn_space]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Area Reference Numbers:  [%s]",
				no_yes[uSN_style.area_refs]);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Line increment for ARN:  [%d]",
				uSN_style.area_ref_inc);
		sprintf(&ln_prt_buff[mpu_next_line++][10], "Count head line for ARN:  [%s]",
				yes_no[uSN_style.no_area_ref]);
		mpu_next_line++;
		memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
		cur_rec = nxt_rec;
	}							/* end while(cur_rec) */
	if (mpu_next_line > 1)
		chk_depth ( U_PrtCharDepth); /* force partial page out */
}								/* end function */

/********************************************************/
void mpu_data_print_vs(void)
{
	int found = 0;

	memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
	sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE%s  (page %d) FOR %s",
			uReportType, mpu_page, U_DataName);
	mpu_next_line = 1;
	if ( !( cur_rec = vs_head) )
	{							/* no data for report */
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "VERTICAL SPACE TRY TABLE:");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5],"[NO DATA FILES IN PROJECT]");
		chk_depth ( U_PrtCharDepth); /* force page out */
		return;
	}
	nxt_rec = cur_rec;
	while (nxt_rec)
	{							/* output report for all files */
		nxt_rec = cur_rec -> next;
		strcpy(tree, TreeName);
		strcpy(dir, SubDirName);
		for(count = 0; count <= 2; count++)
		{
			if(  !get_vs_style(u_wn,&uVS_style, tree, dir, cur_rec->filename))
			{
				found = 1;
				break;
			}
			if(lc_parent(tree,dir,parent))
				p_parent_path(tree,dir,parent,tree,dir);
			else
				break;
		}
		if ( !found)
		{						/* no data */
			chk_depth ( 2);
			mpu_next_line++;
			sprintf(&ln_prt_buff[mpu_next_line++][5], "VERTICAL SPACE TRY TABLE - FILE %s IS MISSING.",
					cur_rec -> filename);
			continue;
		}
		chk_depth ( 37);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "VERTICAL SPACE TRY TABLE - FILE %s",
				cur_rec -> filename);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"TRY  1      2      3      4      5      6      7      8      9     10   TRY");
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"Adjust Extra Lead (percent)                                             ORDER");
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"+ %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%%  %1d",
				uVS_style.try_table_expand[0][0],
				uVS_style.try_table_expand[0][1],
				uVS_style.try_table_expand[0][2],
				uVS_style.try_table_expand[0][3],
				uVS_style.try_table_expand[0][4],
				uVS_style.try_table_expand[0][5],
				uVS_style.try_table_expand[0][6],
				uVS_style.try_table_expand[0][7],
				uVS_style.try_table_expand[0][8],
				uVS_style.try_table_expand[0][9],
				uVS_style.try_order[0]);
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"- %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%% %5d%%",
				uVS_style.try_table_shrink[0][0],
				uVS_style.try_table_shrink[0][1],
				uVS_style.try_table_shrink[0][2],
				uVS_style.try_table_shrink[0][3],
				uVS_style.try_table_shrink[0][4],
				uVS_style.try_table_shrink[0][5],
				uVS_style.try_table_shrink[0][6],
				uVS_style.try_table_shrink[0][7],
				uVS_style.try_table_shrink[0][8],
				uVS_style.try_table_shrink[0][9]);
		mpu_next_line++;
	
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"Adjust Optimal Space Around Inserts (pts)");
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"+%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s   %1d",
				compute_it(uVS_style.try_table_expand[1][0], 0, 0, 0),
				compute_it(uVS_style.try_table_expand[1][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_expand[1][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_expand[1][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_expand[1][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_expand[1][5], 0, 0, 5),
				compute_it(uVS_style.try_table_expand[1][6], 0, 0, 6),
				compute_it(uVS_style.try_table_expand[1][7], 0, 0, 7),
				compute_it(uVS_style.try_table_expand[1][8], 0, 0, 8),
				compute_it(uVS_style.try_table_expand[1][9], 0, 0, 9),
				uVS_style.try_order[1]);
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"-%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s",
				compute_it(uVS_style.try_table_shrink[1][0], 0, 0, 0),
				compute_it(uVS_style.try_table_shrink[1][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_shrink[1][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_shrink[1][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_shrink[1][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_shrink[1][5], 0, 0, 5),
				compute_it(uVS_style.try_table_shrink[1][6], 0, 0, 6),
				compute_it(uVS_style.try_table_shrink[1][7], 0, 0, 7),
				compute_it(uVS_style.try_table_shrink[1][8], 0, 0, 8),
				compute_it(uVS_style.try_table_shrink[1][9], 0, 0, 9));
		mpu_next_line++;
	
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"Adjust Space Above Footnotes (pts)");
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"+%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s   %1d",
				compute_it(uVS_style.try_table_expand[2][0], 0, 0, 0),
				compute_it(uVS_style.try_table_expand[2][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_expand[2][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_expand[2][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_expand[2][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_expand[2][5], 0, 0, 5),
				compute_it(uVS_style.try_table_expand[2][6], 0, 0, 6),
				compute_it(uVS_style.try_table_expand[2][7], 0, 0, 7),
				compute_it(uVS_style.try_table_expand[2][8], 0, 0, 8),
				compute_it(uVS_style.try_table_expand[2][9], 0, 0, 9),
				uVS_style.try_order[2]);
		sprintf(&ln_prt_buff[mpu_next_line++][0],"-%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s",
				compute_it(uVS_style.try_table_shrink[2][0], 0, 0, 0),
				compute_it(uVS_style.try_table_shrink[2][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_shrink[2][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_shrink[2][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_shrink[2][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_shrink[2][5], 0, 0, 5),
				compute_it(uVS_style.try_table_shrink[2][6], 0, 0, 6),
				compute_it(uVS_style.try_table_shrink[2][7], 0, 0, 7),
				compute_it(uVS_style.try_table_shrink[2][8], 0, 0, 8),
				compute_it(uVS_style.try_table_shrink[2][9], 0, 0, 9));
		mpu_next_line++;
	
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"Adjust Space Between Each Line (pts)");
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"+%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s   %1d",
				compute_it(uVS_style.try_table_expand[3][0], 0, 0, 0),
				compute_it(uVS_style.try_table_expand[3][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_expand[3][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_expand[3][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_expand[3][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_expand[3][5], 0, 0, 5),
				compute_it(uVS_style.try_table_expand[3][6], 0, 0, 6),
				compute_it(uVS_style.try_table_expand[3][7], 0, 0, 7),
				compute_it(uVS_style.try_table_expand[3][8], 0, 0, 8),
				compute_it(uVS_style.try_table_expand[3][9], 0, 0, 9),
				uVS_style.try_order[3]);
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"-%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s",
				compute_it(uVS_style.try_table_shrink[3][0], 0, 0, 0),
				compute_it(uVS_style.try_table_shrink[3][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_shrink[3][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_shrink[3][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_shrink[3][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_shrink[3][5], 0, 0, 5),
				compute_it(uVS_style.try_table_shrink[3][6], 0, 0, 6),
				compute_it(uVS_style.try_table_shrink[3][7], 0, 0, 7),
				compute_it(uVS_style.try_table_shrink[3][8], 0, 0, 8),
				compute_it(uVS_style.try_table_shrink[3][9], 0, 0, 9));
		mpu_next_line++;
	
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"Adjust Space Between Paragraphs (pts)");
		sprintf(&ln_prt_buff[mpu_next_line++][0],"+%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s   %1d",
				compute_it(uVS_style.try_table_expand[4][0], 0, 0, 0),
				compute_it(uVS_style.try_table_expand[4][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_expand[4][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_expand[4][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_expand[4][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_expand[4][5], 0, 0, 5),
				compute_it(uVS_style.try_table_expand[4][6], 0, 0, 6),
				compute_it(uVS_style.try_table_expand[4][7], 0, 0, 7),
				compute_it(uVS_style.try_table_expand[4][8], 0, 0, 8),
				compute_it(uVS_style.try_table_expand[4][9], 0, 0, 9),
				uVS_style.try_order[4]);
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"-%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s",
				compute_it(uVS_style.try_table_shrink[4][0], 0, 0, 0),
				compute_it(uVS_style.try_table_shrink[4][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_shrink[4][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_shrink[4][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_shrink[4][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_shrink[4][5], 0, 0, 5),
				compute_it(uVS_style.try_table_shrink[4][6], 0, 0, 6),
				compute_it(uVS_style.try_table_shrink[4][7], 0, 0, 7),
				compute_it(uVS_style.try_table_shrink[4][8], 0, 0, 8),
				compute_it(uVS_style.try_table_shrink[4][9], 0, 0, 9));
		mpu_next_line++;
		
		sprintf(&ln_prt_buff[mpu_next_line++][0],"Adjust Page Depth (pts)");
		sprintf(&ln_prt_buff[mpu_next_line++][0],"*%6s %6s %6s %6s %6s %6s %6s %6s %6s %6s   %1d",
				compute_it(uVS_style.try_table_expand[5][0], 0, 0, 0),
				compute_it(uVS_style.try_table_expand[5][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_expand[5][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_expand[5][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_expand[5][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_expand[5][5], 0, 0, 5),
				compute_it(uVS_style.try_table_expand[5][6], 0, 0, 6),
				compute_it(uVS_style.try_table_expand[5][7], 0, 0, 7),
				compute_it(uVS_style.try_table_expand[5][8], 0, 0, 8),
				compute_it(uVS_style.try_table_expand[5][9], 0, 0, 9),
				uVS_style.try_order[5]);
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"**%5s %6s %6s %6s %6s %6s %6s %6s %6s %6s",
				compute_it(uVS_style.try_table_shrink[5][0], 0, 0, 0),
				compute_it(uVS_style.try_table_shrink[5][1], 0 ,0 ,1),
				compute_it(uVS_style.try_table_shrink[5][2], 0 ,0, 2),
				compute_it(uVS_style.try_table_shrink[5][3], 0 ,0, 3),
				compute_it(uVS_style.try_table_shrink[5][4], 0 ,0, 4),
				compute_it(uVS_style.try_table_shrink[5][5], 0, 0, 5),
				compute_it(uVS_style.try_table_shrink[5][6], 0, 0, 6),
				compute_it(uVS_style.try_table_shrink[5][7], 0, 0, 7),
				compute_it(uVS_style.try_table_shrink[5][8], 0, 0, 8),
				compute_it(uVS_style.try_table_shrink[5][9], 0, 0, 9));
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"NOTE:  '*' line shows the amount to REMOVE from a LONG page.");
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"      '**' line shows the amount to ADD to a SHORT page.");
		mpu_next_line ++;
		sprintf(&ln_prt_buff[mpu_next_line++][34],
				"Full Pagination  Repagination");
		sprintf(&ln_prt_buff[mpu_next_line++][19],
				"Alert At Try:        [%2d]              [%2d]",
				uVS_style.full_pg_alert, uVS_style.repg_alert);
		sprintf(&ln_prt_buff[mpu_next_line++][17],
				"Suspend At Try:        [%2d]              [%2d]",
				uVS_style.full_pg_abort, uVS_style.repg_abort);
		sprintf(&ln_prt_buff[mpu_next_line++][7],
				"Try Long/Short After Try:        [%2d]              [%2d]",
				uVS_style.full_pg_align, uVS_style.repg_align);
		mpu_next_line++;
		memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
		cur_rec = nxt_rec;
	}							/* end while(cur_rec) */
	if (mpu_next_line > 1)
		chk_depth ( U_PrtCharDepth); /* force partial page out */
}								/* end function */

/********************************************************/
static void chk_depth(int inc)
{
	if((mpu_next_line + inc) >= U_PrtCharDepth)
	{
		send_it( ln_prt_buff, &mpu_next_line);
		memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
		mpu_page++;
		sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE  (page %d) FOR %s",
				mpu_page, U_DataName);
		mpu_next_line = 1;
	}
}								/* end function */

/*******************************************************/
static char *compute_it(int16 val, int16 picas_and_pts, int16 space_fill,
						int16 buff_idx)
{
	static char *ptr;
	int16 i;
	int16 frac_pts;
	int16 j;
	int16 picas;
	int16 pts;
	int16 rem_pts;
	
	ptr = &buff[buff_idx][0];
	if(space_fill)
	    memset(ptr, 040, 32);
	else
	    memset(ptr, 0, 32);
	pts = frac_pts = picas = rem_pts = 0;
	i = 0;
	if(val == 0)
	{
		*ptr = '0';
		if(space_fill == 0)
		    *(ptr + 1) = 0;
		return(ptr);
	}
	pts = val / 10;
	frac_pts = val % 10;
	if(picas_and_pts)
		picas = pts / 12;
	rem_pts = pts - (picas * 12);
	
	if(picas)
	{
		sprintf(ptr, "%dp", picas);
		i = strlen(ptr);
	}
	if(rem_pts)
	{
		if (picas)
			sprintf(ptr+i, "%d", rem_pts);
		else
			sprintf(ptr+i, "p%d", rem_pts);
		i = strlen(ptr);
	}
	if(frac_pts)
	{
		j = (100 * frac_pts) / 10;
		if (picas || rem_pts)
			sprintf(ptr+i, ".%d", j);
		else
			sprintf(ptr+i, "p0.%d", j);
	}
	
	if(space_fill)
	    *(ptr + strlen(ptr)) = 040;
	return(ptr);
}								/* end function */

/*******************************************************/
static char *compute_ms(int16 val, int16 picas_and_pts, int16 space_fill,
						int16 buff_idx)
{
	static char *ptr;
	int16 i;
	int16 frac_pts;
	int16 j;
	int16 picas;
	int16 pts;
	int16 rem_pts;
	
	ptr = &buff[buff_idx][0];
	if(space_fill)
	    memset(ptr, 040, 32);
	else
	    memset(ptr, 0, 32);
	pts = frac_pts = picas = rem_pts = 0;
	i = 0;
	if(val == 0)
	{
		*ptr = '0';
		if(space_fill == 0)
		    *(ptr + 1) = 0;
		return(ptr);
	}
	pts = val / 20;
	frac_pts = val % 20;
	if(picas_and_pts)
		picas = pts / 12;
	rem_pts = pts - (picas * 12);
	if(picas)
	{
		sprintf(ptr, "%dp", picas);
		i = strlen(ptr);
	}
	if(rem_pts)
	{
		if (picas)
			sprintf(ptr+i, "%d", rem_pts);
		else
			sprintf(ptr+i, "p%d", rem_pts);
	}
	if(frac_pts)
	{
		j = (100 * frac_pts) / 10;
		if (picas || rem_pts)
			sprintf(ptr+i, ".%d", j);
		else
			sprintf(ptr+i, "p0.%d", j);
	}
	if(space_fill)
	    *(ptr + strlen(ptr)) = 040;
	return(ptr);
}								/* end function */

/**********************************************************/
void mpu_data_print_il(void)
{
	int tempa, tempb, tempc;
	int found = 0;
	char brk_deep_tbl_sel[14];		/*  used to print breaking deep tables  */
	char rotated_align[6];			/*  used to print breaking deep tables  */
	char rotated_cover[12];			/*  used to print breaking deep tables  */
	
	/*   initialize arrays    */
	memset(&rotated_align[0], 040, sizeof(rotated_align));
	memset(&rotated_cover[0], 040, sizeof(rotated_cover));
	memset(&brk_deep_tbl_sel[0], 040, sizeof(brk_deep_tbl_sel));
	memset(&ln_prt_buff[0], 040, sizeof(ln_prt_buff));
	sprintf(&ln_prt_buff[0][0], "MASTERPAGE DATA BASE%s  (page %d) FOR %s",
			uReportType, mpu_page, U_DataName);
	mpu_next_line = 1;
	if ( !( cur_rec = il_head) )
	{							/* no data for report */
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "ILLUSTRATION:");
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][5],"[NO DATA FILES IN PROJECT]");
		chk_depth ( U_PrtCharDepth); /* force page out */
		return;
	}
	nxt_rec = cur_rec;
	while (nxt_rec)
	{							/* output report for all files */
		nxt_rec = cur_rec -> next;
		strcpy(tree, TreeName);
		strcpy(dir, SubDirName);
		for(count = 0; count <= 2; count++)
		{
			if(  !get_il_style(u_wn,&uIL_style, tree, dir, cur_rec->filename))
			{
				found = 1;
				break;
			}
			if(lc_parent(tree,dir,parent))
				p_parent_path(tree,dir,parent,tree,dir);
			else
				break;
		}
		if ( !found)
		{						/* no data */
			chk_depth ( 2);
			mpu_next_line++;
			sprintf(&ln_prt_buff[mpu_next_line++][5], "ILLUSTRATION - FILE %s IS MISSING.",
					cur_rec -> filename);
			continue;
		}
		chk_depth ( 23);
		mpu_next_line++;
		sprintf(&ln_prt_buff[mpu_next_line++][3], "ILLUSTRATION - FILE %s",
				cur_rec -> filename);
		mpu_next_line++;
		memcpy(&ln_prt_buff[mpu_next_line++][10],
			   "What To Place                      Place Before Callout", 55);
		memcpy(&ln_prt_buff[mpu_next_line++][6],
			   "Preference                                              Preference",66);
		tempa = tempb = 0;
		if ( uIL_style.place_any)
			tempa =  uIL_style.place_any;
		if ( uIL_style.before_same_col)
			tempb = uIL_style.before_same_col;
		sprintf(&ln_prt_buff[mpu_next_line++][13],
				"Any   %d                                 Same Column   %d",
				tempa, tempb);
		tempa = tempb = 0;
		if ( uIL_style.place_only_singles)
			tempa = uIL_style.place_only_singles;
		if ( uIL_style.before_same_pg)
			tempb = uIL_style.before_same_pg;
		sprintf(&ln_prt_buff[mpu_next_line++][10],
				"Single   %d                                   Same Page   %d",
				tempa, tempb);
		tempa = tempb = 0;
		if ( uIL_style.place_only_multi)
			tempa = uIL_style.place_only_multi;
		if ( uIL_style.before_face_pg)
			tempb = uIL_style.before_face_pg;
		sprintf(&ln_prt_buff[mpu_next_line++][11],
				"Multi   %d                                 Facing Page   %d",
				tempa, tempb);
		tempa = 0;
		if ( uIL_style.before_any_pg)
			tempa = uIL_style.before_any_pg;
		sprintf(&ln_prt_buff[mpu_next_line++][11],
				"                                    Any Previous Page   %d Per Page",
				tempa);
		mpu_next_line++;
		memcpy(&ln_prt_buff[mpu_next_line++][10],
			   "Where to Place", 14);
		memcpy(&ln_prt_buff[mpu_next_line++][12],
			   "Initial Preference      Preference if Following Rules Occur", 59);
		mpu_next_line++;
		tempa = tempb = tempc = 0;
		if ( uIL_style.top_qual)
			tempa = uIL_style.top_qual;
		if ( uIL_style.stagger_col)
			tempb = uIL_style.stagger_col;
		if ( uIL_style.stack_hor)
			tempc = uIL_style.stack_hor;
		sprintf(&ln_prt_buff[mpu_next_line++][13],
				"Top  %d                 Stagger: Column %d   Stack: Horizontal %d",
				tempa, tempb, tempc);
		tempa = tempb = tempc = 0;
		if ( uIL_style.center_qual)
			tempa = uIL_style.center_qual;
		if ( uIL_style.stagger_pg)
			tempb = uIL_style.stagger_pg;
		if ( uIL_style.stack_ver)
			tempc = uIL_style.stack_ver;
		sprintf(&ln_prt_buff[mpu_next_line++][10],
				"Center  %d                            Page %d            Vertical %d",
				tempa, tempb, tempc);
		tempa = tempb = tempc = 0;
		if ( uIL_style.bottom_qual)
			tempa = uIL_style.bottom_qual;
		if ( uIL_style.stagger_face_pg)
			tempb = uIL_style.stagger_face_pg;
		if ( uIL_style.read_l_r)
			tempc = uIL_style.read_l_r;
		sprintf(&ln_prt_buff[mpu_next_line++][10],
				"Bottom  %d                     Facing Page %d Read: Left to Right %d",
				tempa, tempb, tempc);
		tempa = 0;
		if ( uIL_style.read_t_b)
			tempa = uIL_style.read_t_b;
		sprintf(&ln_prt_buff[mpu_next_line++][10],
				"                                                  Top to Bottom %d",
				tempa);
		mpu_next_line++;
		tempa = 0;
		if (uIL_style.min_pg_qual)
			tempa = uIL_style.min_pg_qual;
		sprintf(&ln_prt_buff[mpu_next_line++][5], "Minimum Preference %d",
				tempa);
		sprintf(&ln_prt_buff[mpu_next_line++][5],
				"Minimum Text Depth With Inserts:  [%s]",
				compute_it(uIL_style.min_dep_w_ins, 1, 0, 0));
		sprintf(&ln_prt_buff[mpu_next_line++][5],
				"Optimal Space Around Inserts:  [%s]",
				compute_it(uIL_style.normal_cut_space, 1, 0, 0));
		mpu_next_line++;
		memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
		cur_rec = nxt_rec;

	/******************************
	 ** begin floating tables print
	 ******************************/
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][3], "FLOATING TABLES SCREEN");
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5],
		"WIDTH OF TABLES.  A floating table's width may span different page elements.");
	sprintf(&ln_prt_buff[mpu_next_line++][23],
		"Rank those in the list below by assigning priority numbers (0 to disallow)");
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][8], "%2i  a. One text column", uIL_style.span_1col);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "%2i  b. One text column plus adjacent sidenote area", 
		uIL_style.span_1col_snote);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "%2i  c. Two text columns plus their gutter", uIL_style.span_2cols);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "%2i  d. Both columns & gutter, plus both adjacent sidenote areas", uIL_style.span_full_page); 
	sprintf(&ln_prt_buff[mpu_next_line++][8], "%2i  e. Three or more text columns plus their gutters", uIL_style.span_3plus_cols);

	 sprintf(&ln_prt_buff[mpu_next_line++][8], 
		"%2i  f. Beyond page width      %s     picas to outside (combines with g. if needed)", 
		uIL_style.span_past_wid_out, 
		compute_it( (uIL_style.picas_past_wid_out / 2), 1, 0, 0));  
	sprintf(&ln_prt_buff[mpu_next_line++][8],
		"%2i  g. Beyond page width      %s     picas to inside (combines with f. if needed)",
		uIL_style.span_past_wid_in, 
		compute_it( (uIL_style.picas_past_wid_in / 2), 1, 0, 0));
	sprintf(&ln_prt_buff[mpu_next_line++][8], "%2i  h. Page depth (rotated)", 
		uIL_style.span_rot_depth);
	sprintf(&ln_prt_buff[mpu_next_line++][8],
		"%2i  i. Beyond page depth      %s     picas (rotated)", 
		uIL_style.span_rot_beyond_depth, 
		compute_it(uIL_style.picas_beyond_dep, 1, 0, 0));
	sprintf(&ln_prt_buff[mpu_next_line++][8], 
		"%2i  j. Left page width plus any inner columns of right page (split across seam)",
		uIL_style.span_seam_plus_cols);
	sprintf(&ln_prt_buff[mpu_next_line++][8],
		"%2i  k. Left page plus right page (split across seam)",
		uIL_style.span_seam_2pages);
	mpu_next_line++;
	memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);
	
	/***********************************
	 ** begin breaking deep tables print
	 ***********************************/

	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][3], "BREAKING DEEP TABLES SCREEN");
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "BREAKING DEEP TABLES.  A floating table may break to next page if too deep.");
	sprintf(&ln_prt_buff[mpu_next_line++][28], "How deep may each piece extend?");
	sprintf(&ln_prt_buff[mpu_next_line++][5], "A.  Upright table must break after    %s    picas beyond page depth", 
		compute_it(uIL_style.break_beyond_dep, 1, 0, 0));
	sprintf(&ln_prt_buff[mpu_next_line++][5], "Rotated table must break after spanning what page elements?  (click one):");
	switch (uIL_style.rotated_wid_span_type)
	{
		case 0:
			brk_deep_tbl_sel[0] = '*';
			break;
		case 1:
			brk_deep_tbl_sel[1] = '*';
			break;
		case 2:
			brk_deep_tbl_sel[2] = '*';
	}

	sprintf(&ln_prt_buff[mpu_next_line++][6], "%1c  B.  All text columns", brk_deep_tbl_sel[0]);
	sprintf(&ln_prt_buff[mpu_next_line++][6], "%1c  C.  Full page width including sidenote area", brk_deep_tbl_sel[1]);
	sprintf(&ln_prt_buff[mpu_next_line++][6], "%1c  D.  Beyond page width                         %s     picas", brk_deep_tbl_sel[2], compute_it(uIL_style.rotated_wid_span_picas, 1, 0, 0));
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "Minimum remaining table depth after break:   %s     picas",
	compute_it(uIL_style.min_dep_after_brk, 1, 0, 0));
	mpu_next_line++;	
	sprintf(&ln_prt_buff[mpu_next_line++][5], "POSITIONING BROKEN TABLES.  Where to begin broken table that is:");
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "Before end of chapter (select one of two)                UPRIGHT        ROTATED");

	if (uIL_style.place_uprite_table)
		brk_deep_tbl_sel[4] = '*';
	else
		brk_deep_tbl_sel[3] = '*';
	
	if (uIL_style.place_rot_table)
			brk_deep_tbl_sel[6] = '*';
	else
			brk_deep_tbl_sel[5] = '*';
	sprintf(&ln_prt_buff[mpu_next_line++][8], "Begin closet to callout (obey placement rules)           %1c              %1c",
		brk_deep_tbl_sel[3], brk_deep_tbl_sel[5]);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "Begin on even page closest to callout (obey rules)       %1c              %1c",
		brk_deep_tbl_sel[4], brk_deep_tbl_sel[6]);
	switch (uIL_style.place_xp_uprite_table)
	{
		case 0:
			brk_deep_tbl_sel[7] = '*';
			break;
		case 1:
			brk_deep_tbl_sel[8] = '*';
			break;
		case 2:
			brk_deep_tbl_sel[9] = '*';
			break;
		case 3:
			brk_deep_tbl_sel[10] = '*';
	}
	switch (uIL_style.place_xp_rot_table)
	{
		case 0:
			brk_deep_tbl_sel[11] = '*';
			break;
		case 1:
			brk_deep_tbl_sel[12] = '*';
			break;
		case 2:
			brk_deep_tbl_sel[13] = '*';
	}
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "At end of chapter (select one of four)");
	sprintf(&ln_prt_buff[mpu_next_line++][8], "No special action, use same choice as above              %1c              %1c",
		brk_deep_tbl_sel[7], brk_deep_tbl_sel[11]);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "Begin on next page                                       %1c              %1c",
		brk_deep_tbl_sel[8], brk_deep_tbl_sel[12]);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "Begin on even page before last text page                 %1c              %1c",
		brk_deep_tbl_sel[9], brk_deep_tbl_sel[13]);
	sprintf(&ln_prt_buff[mpu_next_line++][8], "Start on same page as text ends, for fewest folios       %1c",
		brk_deep_tbl_sel[10]);
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "SPECIAL FOR ROTATED TABLES");
	switch (uIL_style.rotated_alignment)
	{
		case 0:
			strcpy( rotated_align, "Top");
			break;
		case 1:
			strcpy( rotated_align, "Bottom");
			break;
		case 2:
			strcpy( rotated_align, "Center");
	}
	sprintf(&ln_prt_buff[mpu_next_line++][5], "If rotated table's width exceeds page depth, align it:     %s", rotated_align); 
	switch (uIL_style.rotated_cover_foot)
	{
		case 0:
			strcpy( rotated_cover, "None");
			break;
		case 1:
			strcpy( rotated_cover, "Running Foot");
			break;
		case 2:
			strcpy( rotated_cover, "Running Head");
			break;
		case 3:
			strcpy( rotated_cover, "Both");
	}
	sprintf(&ln_prt_buff[mpu_next_line++][5], "If rotated table's width exceeds page depth, should design text be removed?     %s", rotated_cover); 
	mpu_next_line++;
	sprintf(&ln_prt_buff[mpu_next_line++][5], "%1c  Repeat boxhead on right page for rotated table", (uIL_style.repeat_box_rot ? '*' : ' '));
	
	mpu_next_line++;
	memset(&ln_prt_buff[mpu_next_line++][0], '-', U_PrtCharWid-1);

	/*   re-initialize arrays   */
	memset(&rotated_align[0], 040, sizeof(rotated_align));
	memset(&rotated_cover[0], 040, sizeof(rotated_cover));
	memset(&brk_deep_tbl_sel[0], 040, sizeof(brk_deep_tbl_sel));

	if (mpu_next_line > 1)
		chk_depth ( U_PrtCharDepth); /* force partial page out */

  }                             /* end while(cur_rec) */

}								/* end function */

/********************* remove unneeded chars, find first ************/

static char *line_clean(char *ptr)
{
	int i;
	char *start_ptr = 0;
	
	LineCharCount = strlen(ptr);
    for (i=0; i<=LineCharCount; i++)
    {							/* change all unneeded chars to nulls */
		switch ( *ptr)
        {
          case '\t':			/* tab */
          case '\n':			/* new line */
          case '\f':			/* form feed */
          case '\r':			/* carriage return */
		  case ' ' :
			  *ptr++ = '\0';
			  continue;
			default:
			  if ( !start_ptr)
				  start_ptr = ptr;
			  ptr++;
		  }						/* end switch */
    }							/* end - for (i=0; i<msg_buf_size; i++) */
	EndLinePtr = LineIn + LineCharCount;
	return(start_ptr);
}								/* end function */
/*********************** return an int16 ****************************/

static int16 cnvrt_ascii(int flag) /* 1 = find next string, 0 = don't */
{
	int len;
	int16 result;
	
	EolFlag = 0;
	len = strlen(BeginLinePtr);	/* get the length of the string */
	if( !len)
	{							/* no more data */
		EolFlag = 1;
		return(-1);
	}
	result = (int16 )atoi(BeginLinePtr); /* get the int16 value */
	BeginLinePtr += len;		/* point past end of string */
	if (flag)
		find_next_string();
	return(result);
}								/* end function */

/*********************** find the start of the next string ***********/

static void find_next_string(void)
{
	
	EolFlag = 0;
	while (BeginLinePtr <= EndLinePtr)
	{							/* find the next starting data character */
		if ( ! *BeginLinePtr++)
			continue;			/* keep looking */
		else 
		{
			BeginLinePtr--;		/* point to the start of a char string */
			break;
		}
	}				/* end while(BeginLinePtr <= EndLinePtr) */
	if (BeginLinePtr > EndLinePtr)
		EolFlag = 1;			/* no data on the line */
}

/**************************** Alias Print ******************************/
static void mpu_print_alias_list(void)
{
	char temp_alias_name[132];
	Pfd AliasFd;
	char U_UserdataPath[MAX_NAME];
	int i, length, max_alias_count;
	int input_penta_code;
	int line_pos = 51;

	sprintf(&ln_prt_buff[mpu_next_line][10], "List Of Characters Page Cannot End With: ");
	if ( uBR_style.pg_non_term < 0)
	{
		sprintf(&ln_prt_buff[mpu_next_line][line_pos],"[NONE]");
		return;
	}
	if ( p_get_data_name ( U_TreeName, U_DirName, U_UserdataPath, 0 ) )
    {
		sprintf(&ln_prt_buff[mpu_next_line++][0],
				"ERROR - missing file '/Penta/%s/desks/%s/.data', Unknown alias.\n",
				U_TreeName, U_DirName);
		return;
    }
	sprintf(temp_alias_name,"/Penta/%s/userdata/%s/%s",
			U_TreeName, U_UserdataPath, "char.alias");
	AliasFd = p_open(NO_STRING, OTHER_FILE, NO_STRING, temp_alias_name, "r");
	if (! AliasFd)
	{
		sprintf(temp_alias_name,"/Penta/.default/userdata/%s/%s",
				U_UserdataPath, "char.alias");
		AliasFd = p_open(NO_STRING, OTHER_FILE, NO_STRING, temp_alias_name,
						 "r");
		if (! AliasFd)
		{
			sprintf(&ln_prt_buff[mpu_next_line++][0],
					"ERROR - missing files /Penta/%s/userdata/%s/%s and '%s', Unknown alias.\n",
					U_TreeName, U_UserdataPath, "char.alias", temp_alias_name);
			return;
		}
	}
	memset (aliases, 0, sizeof (aliases));
	while(1)
	{							/* read in all the aliases */
		memset(LineIn, 0, LINE_SIZE);
		if((p_fgets(LineIn, LINE_SIZE, AliasFd)) == NULL)
			break;				/* end it here */
		BeginLinePtr = line_clean(LineIn); /* remove junk, get first char*/
		LineCounter++;
		if( (*BeginLinePtr == '#') || (LineCharCount < 5) ||
			(*BeginLinePtr == '\0'))
			continue;
		EolFlag = 0;
		input_penta_code = cnvrt_ascii (1);
		if ( (input_penta_code <= 0) || (input_penta_code > 255) || EolFlag )
			continue;			/* no value */
		find_next_string();
		if (EolFlag)
			continue;			/* no alias */
		if (  !(length = strlen(BeginLinePtr) ) )
			continue;
		strcpy ((char *)&aliases[input_penta_code][0], BeginLinePtr);
	}							/* end while(1) */
	if ( (max_alias_count = uBR_style.pg_non_term) > 10)
		max_alias_count = 10;
	for (i=0; i<=max_alias_count; i++)
	{							/* print all of the alias values */
		if ( !uBR_style.pg_non_term_list[i])
			break;
		length = strlen( (char *)&aliases[uBR_style.pg_non_term_list[i]][0]);
		if ( (length + line_pos + 1) >= U_PrtCharWid)
		{
			mpu_next_line++;
			line_pos = 51;
		}
		sprintf(&ln_prt_buff[mpu_next_line][line_pos], "%s ",
				(char *)&aliases[uBR_style.pg_non_term_list[i]][0]);
		line_pos += length + 1;
	}							/* end for(i=0;i<=max_alias_count;i++) */
}								/* end function */

/**********************************************************/
void output_mpu_post_report (void)
{
	int save_LYPrintFlag;
	int save_Reports;
	save_LYPrintFlag = LYPrintFlag;
	save_Reports = Reports;
	LYPrintFlag &= ~4;
	Reports = 0;
	strcpy (uReportType, " - FOR FILES IN USE ");
	psmpu_main();
	LYPrintFlag = save_LYPrintFlag;
	Reports = save_Reports;
}

/********** EOF *********/
