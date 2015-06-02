/*    *****     **    **       *       ******     *******    *******
     **   **    **    **      ***      **   **    **         **    **
     **         **    **     ** **     **   **    **         **     **
     **         **    **    **   **    **   **    **         **     **
      *****     ********    *******    ******     *****      **     **
          **    **    **    **   **    ****       **         **     **
          **    **    **    **   **    ** **      **         **     **
     **   **    **    **    **   **    **  **     **         **    ** 
      *****     **    **    **   **    **   **    *******    *******  */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "p_lib.h"
#include "window.h"
#include "menu_frame.h"
#include "frame.h"
#include "mp_styles.h"
#include "lmt.f"
#include "interfer.f"


extern char *itoa(int, char *);

static IL_STYLE il;				/* Illustration Styles */
static BR_STYLE br;				/* Breaking Style */
static CH_STYLE ch;				/* Chapter Style */
static FN_STYLE fn;				/* Footnote Style */
static SN_STYLE sn;				/* SnArn Stles */
static VS_STYLE vs;				/* Vertical Spacing Style */

static PELEM pel;				/* Page-elements data in *.pelem file */

static char ascii_try[10][32];
/* stuff for try table			*/
void cal_try_ascii(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr);
void calc_try_data(WYSIWYG *, MP_STYLE_CNTRLS *);
void calc_try_num(MP_STYLE_CNTRLS *, int);
int fill_try(char *, int);
void mp_ld_try(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_cuts_try(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_fn_try(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_ln_try(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_para_try(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_pg_try(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_try_order(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_non_terms(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_preference(WYSIWYG *,MP_STYLE_CNTRLS *,char *,int);

/* general purpose stuff		*/
void mp_numeric(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_pica_hor(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_pica_ver(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
void mp_pts_ver(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
#ifndef NOT_USED_BY_ANYONE
void mp_pts_hor(WYSIWYG *, MP_STYLE_CNTRLS *, char *, int);
#endif 
int get_style (WYSIWYG *, Pfd, char *, int16, MP_STYLE_CNTRLS *);

MP_STYLE_CNTRLS il_controls[] = {
	{"PLACE_ANY"				,&il.place_any			,mp_preference},
	{"PLACE_ONLY_SINGLES"		,&il.place_only_singles	,mp_preference},
	{"PLACE_ONLY_MULTI"			,&il.place_only_multi	,mp_preference},
	{"NEVER_BEFORE"				,&il.never_before		,mp_preference},
	{"BEFORE_SAME_COL"			,&il.before_same_col	,mp_preference},
	{"BEFORE_SAME_PG"			,&il.before_same_pg		,mp_preference},
	{"BEFORE_FACE_PG"			,&il.before_face_pg		,mp_preference},
	{"BEFORE_ANY_PG"			,&il.before_any_pg		,mp_preference},
	{"TOP_QUAL"					,&il.top_qual			,mp_preference},
	{"CENTER_QUAL"				,&il.center_qual		,mp_preference},
	{"BOTTOM_QUAL"				,&il.bottom_qual		,mp_preference},
	{"STAGGER_COL"				,&il.stagger_col		,mp_preference},
	{"STAGGER_PG"				,&il.stagger_pg			,mp_preference},
	{"STAGGER_FACE_PG"			,&il.stagger_face_pg	,mp_preference},
	{"STACK_HOR"				,&il.stack_hor			,mp_preference},
	{"STACK_VER"				,&il.stack_ver			,mp_preference},
	{"READ_L_R"					,&il.read_l_r			,mp_preference},
	{"READ_T_B"					,&il.read_t_b			,mp_preference},
	{"MIN_PG_QUAL"				,&il.min_pg_qual		,mp_preference},
	{"MIN DEPTH WITH INS"		,&il.min_dep_w_ins		,mp_pica_ver},
	{"NORMAL CUT SPACE"			,&il.normal_cut_space	,mp_pica_ver},

	{"SPAN_1COL"				,&il.span_1col			,mp_numeric},
	{"SPAN_1COL_SIDES"			,&il.span_1col_snote	,mp_numeric},
	{"SPAN_2COLS"				,&il.span_2cols			,mp_numeric},
	{"SPAN_3PLUS_COLS"			,&il.span_3plus_cols	,mp_numeric},
	{"SPAN_FULL_PAGE"			,&il.span_full_page		,mp_numeric},
	{"SPAN_BEYOND_WID_OUT"		,&il.span_past_wid_out	,mp_numeric},
	{"SPAN_BEYOND_WID_IN"		,&il.span_past_wid_in	,mp_numeric},
	{"SPAN_ROT_DEP"				,&il.span_rot_depth		,mp_numeric},
	{"SPAN_ROT_BEYOND_DEP"		,&il.span_rot_beyond_depth,mp_numeric},
	{"SPAN_SEAM_PLUS_COLS"		,&il.span_seam_plus_cols,mp_numeric},
	{"SPAN_SEAM_2PAGES"			,&il.span_seam_2pages	,mp_numeric},
	{"PICAS_BEYOND_WID_OUT"		,&il.picas_past_wid_out	,mp_pica_hor},
	{"PICAS_BEYOND_WID_IN"		,&il.picas_past_wid_in	,mp_pica_hor},
	{"PICAS_BEYOND_DEP"			,&il.picas_beyond_dep	,mp_pica_ver},
	{"BREAK_BEYOND_DEP"			,&il.break_beyond_dep	,mp_pica_ver},
	{"ROTATED_WID_SPAN"			,&il.rotated_wid_span_type,mp_numeric},
	{"ROTATED_PICAS_BEYOND_WID"	,&il.rotated_wid_span_picas,mp_pica_ver},
	{"MIN_DEP_AFTER_BREAK"		,&il.min_dep_after_brk	,mp_pica_ver},
	{"PLACE_UPRITE_TABLE"		,&il.place_uprite_table	,mp_numeric},
	{"PLACE_ROT_TABLE"			,&il.place_rot_table	,mp_numeric},
	{"PLACE_XP_UPRITE_TABLE"	,&il.place_xp_uprite_table,mp_numeric},
	{"PLACE_XP_ROT_TABLE"		,&il.place_xp_rot_table	,mp_numeric},
	{"ROTATED_ALIGN"			,&il.rotated_alignment	,mp_numeric},
	{"ROTATED_COVER"			,&il.rotated_cover_foot	,mp_numeric},
	{"ROTATED_CENTER"			,&il.rotated_x_center	,mp_numeric},
	{"REPEAT_BOX"				,&il.repeat_box_rot		,mp_numeric}};

#define NUM_IL_DATA		(sizeof(il_controls) / sizeof(MP_STYLE_CNTRLS))
int put_il_style(WYSIWYG *wn, IL_STYLE *il_style, 
				 char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	
	if((fdout = p_open(tree, IL_STYLE_FILE, dir, filename, "w+")) == P_ERROR)
		return(1);
	/* copy to working struct */
	memcpy((char *)&il, (char *)il_style, sizeof(IL_STYLE));
	sprintf(buff, "# IL STYLE FILE FOR /%s/%s/%s\n#\n#\n", 
			tree, JOBS_DIR, dir);
	p_fputs(buff, fdout);
	/* output data */
	for(i = 0; i < NUM_IL_DATA; i++)
	{
		char *temp;

		memset(buff, 0, sizeof(buff));			/* Init line:  All nulls,	*/
		sprintf(buff, "%s:  ", il_controls[i].mp_string);	/* & keyword.	*/
		temp = strchr (buff, '\0');				/* Find loc for value,	*/
		if(il_controls[i].func)					/*  translate struct values
													into ascii line:	*/
			(*(il_controls[i].func))(wn, &il_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');				/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);					/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}

int get_il_style(WYSIWYG *wn, IL_STYLE *il_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdin;
/*
 * Open *.ils style file.
 */
	if((fdin = p_open(tree, IL_STYLE_FILE, dir, filename, "r")) == P_ERROR)
		return(1);
/*
 * Init all values in IL_STYLE il, call general routine that loops to read 
 *   all lines from *.ils, store its values into il.  Struct il_controls,
 *   passed to get_style, contains pointers to il's elements.  Close fdin.
 */
	memset((char *)&il, 0, sizeof(IL_STYLE));
	if (get_style(wn, fdin, filename, (int16)NUM_IL_DATA, il_controls))
		return(1);
/*
 * Copy data just read in static IL_STYLE il into perm struct il_style.
 */
	strcpy(il.filename,filename);
	memcpy((char *)il_style, (char *)&il, sizeof(IL_STYLE));
	return(0);
}

MP_STYLE_CNTRLS br_controls[] = {
	{"ALLOW ORPHAN"				,&br.allow_orphans		,mp_numeric },
	{"ALLOW WIDOW"				,&br.allow_widows		,mp_numeric },
	{"LINES ABOVE HEAD"			,&br.lines_above_head	,mp_numeric },
	{"LINES BELOW HEAD"			,&br.lines_below_head	,mp_numeric },
	{"PG NON TERM LIST"			,&br.pg_non_term		,mp_non_terms},
	{"WIDOW %"					,&br.widow_pct			,mp_numeric }};
#define NUM_BR_DATA		(sizeof(br_controls) / sizeof(MP_STYLE_CNTRLS))

int put_br_style(WYSIWYG *wn, BR_STYLE *br_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	
	if((fdout = p_open(tree, BR_STYLE_FILE, dir, filename, "w+")) == P_ERROR)
		return(1);
	/* copy to working struct */
	memcpy((char *)&br, (char *)br_style, sizeof(BR_STYLE));
	sprintf(buff, "# BR_STYLE FILE FOR /%s/%s/%s\n#\n#\n", 
			tree, JOBS_DIR, dir);
	p_fputs(buff, fdout);
	/* output data */
	for(i = 0; i < NUM_BR_DATA; i++)
	{
		char *temp;

		memset(buff, 0, sizeof(buff));			/* Init line:  All nulls,	*/
		sprintf(buff, "%s:  ", br_controls[i].mp_string);	/* & keyword.	*/
		temp = strchr (buff, '\0');				/* Find loc for value,	*/
		if(br_controls[i].func)					/*  translate struct values
													into ascii line:	*/
			(*(br_controls[i].func))(wn, &br_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');				/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);					/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}

int get_br_style(WYSIWYG *wn, BR_STYLE *br_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdin;
/*
 * Open *.brs style file.
 */
	if((fdin = p_open(tree, BR_STYLE_FILE, dir, filename, "r")) == P_ERROR)
		return(1);
/*
 * Init all values in BR_STYLE br, call general routine that loops to read 
 *   all lines from *.brs, store its values into br.  Struct br_controls,
 *   passed to get_style, contains pointers to br's elements.  Close fdin.
 */
	memset((char *)&br, 0, sizeof(BR_STYLE));
	br.widow_pct = 100;
	if (get_style(wn, fdin, filename, (int16)NUM_BR_DATA, br_controls))
		return(1);
/*
 * Copy data just read in static BR_STYLE br into perm struct br_style.
 */
	strcpy(br.filename,filename);
	memcpy((char *)br_style, (char *)&br, sizeof(BR_STYLE));
	return(0);
}

MP_STYLE_CNTRLS ch_controls[] = {
	{"ALLOW SHORT PG"			,&ch.allow_short_pg		,mp_numeric },
	{"ALIGN AMT"				,&ch.align_amt			,mp_pts_ver},
	{"ALIGN OPT"				,&ch.align_opt			,mp_numeric },
	{"AUTO ALIGN"				,&ch.auto_align			,mp_numeric },
	{"BEGIN UNIT ON"			,&ch.begin_unit_on		,mp_numeric },
	{"LONG SHORT OK"			,&ch.long_short_ok		,mp_numeric },
	{"MIN LINES LAST PG"		,&ch.min_last_pg_lines	,mp_numeric },
	{"POST-PROCESS MEMBERS"		,&ch.postproc_members	,mp_numeric },
	{"MAX ERROR COUNT"			,&ch.max_err_count		,mp_numeric }};
#define NUM_CH_DATA		(sizeof(ch_controls) / sizeof(MP_STYLE_CNTRLS))
int put_ch_style(WYSIWYG *wn, CH_STYLE *ch_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	
	if((fdout = p_open(tree, CH_STYLE_FILE, dir, filename, "w+")) == P_ERROR)
		return(1);
	/* copy to working struct			*/
	memcpy((char *)&ch, (char *)ch_style, sizeof(CH_STYLE));
	sprintf(buff, "# MP_STYLE FILE FOR /%s/%s/%s\n#\n#\n", 
			tree, JOBS_DIR, dir);
	p_fputs(buff, fdout);
	/* output data */
	for(i = 0; i < NUM_CH_DATA; i++)
	{
		char *temp;

		memset(buff, 0, sizeof(buff));			/* Init line:  All nulls,	*/
		sprintf(buff, "%s:  ", ch_controls[i].mp_string);	/* & keyword.	*/
		temp = strchr (buff, '\0');				/* Find loc for value,	*/
		if(ch_controls[i].func)					/*  translate struct values
													into ascii line:	*/
			(*(ch_controls[i].func))(wn, &ch_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');				/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);					/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}
int get_ch_style(WYSIWYG *wn, CH_STYLE *ch_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdin;
/*
 * Open *.chs style file.
 */
	if((fdin = p_open(tree, CH_STYLE_FILE, dir, filename, "r")) == P_ERROR)
		return(1);
/*
 * Init all values in CH_STYLE ch, call general routine that loops to read 
 *   all lines from *.chs, store its values into ch.  Struct ch_controls,
 *   passed to get_style, contains pointers to ch's elements.  Close fdin.
 */
	memset((char *)&ch, 0, sizeof(CH_STYLE));
	if (get_style(wn, fdin, filename, (int16)NUM_CH_DATA, ch_controls))
		return(1);
/*
 * Copy data just read in static CH_STYLE ch into perm struct ch_style.
 */
	strcpy(ch.filename,filename);
	memcpy((char *)ch_style, (char *)&ch, sizeof(CH_STYLE));
	return(0);
}

MP_STYLE_CNTRLS fn_controls[] = {
	{"ALLOW FN BRK"				,&fn.allow_foot_break	,mp_numeric },
	{"BOTTOM ILLS FN PLACE"		,&fn.bot_ill_fn_place	,mp_numeric },
	{"CCFN ALLOW EP"			,&fn.ccfn_allow_ep		,mp_numeric },
	{"CCFN ALLOW ORPHAN"		,&fn.ccfn_allow_orphans	,mp_numeric },
	{"CCFN ALLOW WIDOW"			,&fn.ccfn_allow_widows	,mp_numeric },
	{"CCFN WIDOW %"				,&fn.ccfn_widow_pct		,mp_numeric },
	{"DBL SHORT EVENLY"			,&fn.dbl_up_evenly		,mp_numeric },
	{"DBL SHORT FN"				,&fn.dbl_short_fns		,mp_numeric },
	{"DBL NUM"					,&fn.dbl_num			,mp_numeric },
	{"DBL SPACE"				,&fn.dbl_space			,mp_pica_hor},
	{"FN CHAP EVEN"				,&fn.fn_chp_even		,mp_numeric },
	{"FN CHAP EVEN COL"			,&fn.fn_chp_even_col	,mp_numeric },
	{"FN CHAP ODD"				,&fn.fn_chp_odd			,mp_numeric },
	{"FN CHAP ODD COL"			,&fn.fn_chp_odd_col		,mp_numeric },
	{"FN COLS"					,&fn.fn_cols			,mp_numeric },
	{"FN COL WID"				,&fn.fn_col_wid			,mp_pica_hor},
	{"FN EVEN"					,&fn.fn_even			,mp_numeric },
	{"FN EVEN COL"				,&fn.fn_even_col		,mp_numeric },
	{"FN GUTTER"				,&fn.fn_gutter			,mp_pica_hor},
	{"FN ODD"					,&fn.fn_odd				,mp_numeric },
	{"FN ODD COL"				,&fn.fn_odd_col			,mp_numeric },
	{"FN PLACEMENT"				,&fn.fn_place			,mp_numeric },
	{"FN SPACE PLACEMENT"		,&fn.fn_space_place		,mp_numeric },
	{"LAST PG FN PLACE"			,&fn.last_pg_fn_place	,mp_numeric },
	{"MIN DEPTH WITH BROKE FN"	,&fn.min_dep_w_broke_fn	,mp_pica_ver},
	{"PPFN ALLOW EP"			,&fn.ppfn_allow_ep		,mp_numeric },
	{"PPFN ALLOW ORPHAN"		,&fn.ppfn_allow_orphans	,mp_numeric },
	{"PPFN ALLOW WIDOW"			,&fn.ppfn_allow_widows	,mp_numeric },
	{"PPFN WIDOOW %"			,&fn.ppfn_widow_pct		,mp_numeric },
	{"SPACE ABOVE CR"			,&fn.space_above_cr		,mp_pica_ver},
	{"SPACE BELOW CR"			,&fn.space_below_cr		,mp_pica_ver},
	{"LENGTH OF CR"				,&fn.length_of_cr		,mp_pica_hor},
	{"INDENT OF CR"				,&fn.indent_of_cr		,mp_pica_hor},
	{"COLOR OF CR"				,&fn.color_of_cr		,mp_numeric},
	{"WEIGHT OF CR"				,&fn.weight_of_cr		,mp_pts_ver},
	{"SPACE ABOVE SCR"			,&fn.space_above_scr	,mp_pica_ver},
	{"SPACE BELOW SCR"			,&fn.space_below_scr	,mp_pica_ver},
	{"LENGTH OF SCR"			,&fn.length_of_scr		,mp_pica_hor},
	{"INDENT OF SCR"			,&fn.indent_of_scr		,mp_pica_hor},
	{"COLOR OF SCR"				,&fn.color_of_scr		,mp_numeric},
	{"WEIGHT OF SCR"			,&fn.weight_of_scr		,mp_pts_ver},
	{"SPACE ABOVE CCR"			,&fn.space_above_ccr	,mp_pica_ver},
	{"SPACE BELOW CCR"			,&fn.space_below_ccr	,mp_pica_ver},
	{"LENGTH OF CCR"			,&fn.length_of_ccr		,mp_pica_hor},
	{"INDENT OF CCR"			,&fn.indent_of_ccr		,mp_pica_hor},
	{"COLOR OF CCR"				,&fn.color_of_ccr		,mp_numeric},
	{"WEIGHT OF CCR"			,&fn.weight_of_ccr		,mp_pts_ver}};
#define NUM_FN_DATA		(sizeof(fn_controls) / sizeof(MP_STYLE_CNTRLS))
int put_fn_style(WYSIWYG *wn, FN_STYLE *fn_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	if((fdout = p_open(tree, FN_STYLE_FILE, dir,  filename, "w+")) == P_ERROR)
		return(1);
	/* copy to working struct */
	memcpy((char *)&fn, (char *)fn_style, sizeof(FN_STYLE));
	sprintf(buff, "# MP_STYLE FILE FOR /%s/%s/%s\n#\n#\n", 
			tree, JOBS_DIR, dir);
	p_fputs(buff, fdout);
	/* output data */
	for(i = 0; i < NUM_FN_DATA; i++)
	{
		char *temp;

		memset(buff, 0, sizeof(buff));			/* Init line:  All nulls,	*/
		sprintf(buff, "%s:  ", fn_controls[i].mp_string);	/* & keyword.	*/
		temp = strchr (buff, '\0');				/* Find loc for value,	*/
		if(fn_controls[i].func)					/*  translate struct values
													into ascii line:	*/
			(*(fn_controls[i].func))(wn, &fn_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');				/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);					/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}

int get_fn_style(WYSIWYG *wn, FN_STYLE *fn_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdin;
/*
 * Open *.fns style file.
 */
	if((fdin = p_open(tree, FN_STYLE_FILE, dir, filename, "r")) == P_ERROR)
		return(1);
/*
 * Init all values in FN_STYLE fn, call general routine that loops to read 
 *   all lines from *.fns, store its values into fn.  Struct fn_controls,
 *   passed to get_style, contains pointers to fn's elements.  Close fdin.
 */
	memset((char *)&fn, 0, sizeof(FN_STYLE));
	fn.allow_foot_break = 1;
	fn.ccfn_widow_pct = 100;
	fn.fn_place = 1;
	fn.fn_space_place = 100;
	fn.last_pg_fn_place = 1;
	fn.ppfn_widow_pct = 100;
	if (get_style(wn, fdin, filename, (int16)NUM_FN_DATA, fn_controls))
		return(1);
/*
 * Copy data just read in static FN_STYLE fn into perm struct fn_style.
 */
	strcpy(fn.filename,filename);
	memcpy((char *)fn_style, (char *)&fn, sizeof(FN_STYLE));
	return(0);
}

MP_STYLE_CNTRLS sn_controls[] = {
	{"AREA REFS"				,&sn.area_refs			,mp_numeric },
	{"AREA REF INC"				,&sn.area_ref_inc		,mp_numeric },
	{"NO AREA REF HEAD"			,&sn.no_area_ref		,mp_numeric },
	{"SIDE NOTES"				,&sn.side_notes			,mp_numeric },
	{"SN SPACE"					,&sn.sn_space			,mp_numeric },
	{"CONTINLINES VARDEPTH"		,&sn.var_contin_lines	,mp_numeric }};
#define NUM_SN_DATA		(sizeof(sn_controls) / sizeof(MP_STYLE_CNTRLS))
int put_sn_style(WYSIWYG *wn, SN_STYLE *sn_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	
	if((fdout = p_open(tree, SN_STYLE_FILE, dir, filename, "w+")) == P_ERROR)
		return(1);
	/* copy to working struct			*/
	memcpy((char *)&sn, (char *)sn_style, sizeof(SN_STYLE));
	sprintf(buff, "# MP_STYLE FILE FOR /%s/%s/%s\n#\n#\n", 
			tree, JOBS_DIR, dir);
	p_fputs(buff, fdout);
	/* output data						*/
	for(i = 0; i < NUM_SN_DATA; i++)
	{
		char *temp;

		memset(buff, 0, sizeof(buff));			/* Init line:  All nulls,	*/
		sprintf(buff, "%s:  ", sn_controls[i].mp_string);	/* & keyword.	*/
		temp = strchr (buff, '\0');				/* Find loc for value,	*/
		if(sn_controls[i].func)					/*  translate struct values
													into ascii line:	*/
			(*(sn_controls[i].func))(wn, &sn_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');				/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);					/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}

int get_sn_style(WYSIWYG *wn, SN_STYLE *sn_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdin;
/*
 * Open *.sns style file.
 */
	if((fdin = p_open(tree, SN_STYLE_FILE, dir, filename, "r")) == P_ERROR)
		return(1);
/*
 * Init all values in SN_STYLE sn, call general routine that loops to read 
 *   all lines from *.sns, store its values into sn.  Struct sn_controls,
 *   passed to get_style, contains pointers to sn's elements.  Close fdin.
 */
	memset((char *)&sn, 0, sizeof(SN_STYLE));
	if (get_style(wn, fdin, filename, (int16)NUM_SN_DATA, sn_controls))
		return(1);
/*
 * Copy data just read in static SN_STYLE sn into perm struct sn_style.
 */
	strcpy(sn.filename,filename);
	memcpy((char *)sn_style, (char *)&sn, sizeof(SN_STYLE));
	return(0);
}

MP_STYLE_CNTRLS vs_controls[] = {
	{"FULL PG ABORT"			,&vs.full_pg_abort		,mp_numeric },
	{"FULL PG ALERT"			,&vs.full_pg_alert		,mp_numeric },
	{"FULL PG ALIGN"			,&vs.full_pg_align		,mp_numeric },
	{"REPAGE ABORT"				,&vs.repg_abort			,mp_numeric },
	{"REPAGE ALERT"				,&vs.repg_alert			,mp_numeric },
	{"REPAGE ALIGN"				,&vs.repg_align			,mp_numeric },
	{"TRY ORDER    ",&vs.try_order[0]					,mp_try_order },
	{"TRY + EX LD  ",&vs.try_table_expand[VS_EXLD][0]	,mp_ld_try },
	{"TRY - EX LD  ",&vs.try_table_shrink[VS_EXLD][0]	,mp_ld_try },
	{"TRY + OP SP  ",&vs.try_table_expand[VS_CUTS][0]	,mp_cuts_try},
	{"TRY - OP SP  ",&vs.try_table_shrink[VS_CUTS][0]	,mp_cuts_try},
	{"TRY + FN SP  ",&vs.try_table_expand[VS_FNS][0]	,mp_fn_try},
	{"TRY - FN SP  ",&vs.try_table_shrink[VS_FNS][0]	,mp_fn_try},
	{"TRY + LN SP  ",&vs.try_table_expand[VS_LINES][0]	,mp_ln_try},
	{"TRY - LN SP  ",&vs.try_table_shrink[VS_LINES][0]	,mp_ln_try},
	{"TRY + PARA SP",&vs.try_table_expand[VS_PARAS][0]	,mp_para_try},
	{"TRY - PARA SP",&vs.try_table_shrink[VS_PARAS][0]	,mp_para_try},
	{"TRY - PG DEP ",&vs.try_table_expand[VS_DEPTH][0]	,mp_pg_try},
	{"TRY + PG DEP ",&vs.try_table_shrink[VS_DEPTH][0]	,mp_pg_try}};

#define NUM_VS_DATA		(sizeof(vs_controls) / sizeof(MP_STYLE_CNTRLS))
/*********************************************************************/

int put_vs_style(WYSIWYG *wn, VS_STYLE *vs_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	
	if((fdout = p_open(tree, VS_STYLE_FILE, dir, filename, "w+")) == P_ERROR)
		return(1);
	/* copy to working struct */
	memcpy((char *)&vs, (char *)vs_style, sizeof(VS_STYLE));
	sprintf(buff, "# MP_STYLE FILE FOR /%s/%s/%s\n#\n#\n",
			tree, JOBS_DIR, dir);
	p_fputs(buff, fdout);
	/* output data */
	for(i = 0; i < NUM_VS_DATA; i++)
	{
		char *temp;

		memset(buff, 0, sizeof(buff));			/* Init line:  All nulls,	*/
		sprintf(buff, "%s:  ", vs_controls[i].mp_string);	/* & keyword.	*/
		temp = strchr (buff, '\0');				/* Find loc for value,	*/
		if(vs_controls[i].func)					/*  translate struct values
													into ascii line:	*/
			(*(vs_controls[i].func))(wn, &vs_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');				/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);					/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}

int get_vs_style(WYSIWYG *wn, VS_STYLE *vs_style,
				 char *tree, char *dir, char *filename)
{
	Pfd fdin;
	int i;
/*
 * Open *.vss style file.
 */
	if((fdin = p_open(tree, VS_STYLE_FILE, dir, filename, "r")) == P_ERROR)
		return(1);
/*
 * Init all values in VS_STYLE vs, call general routine that loops to read 
 *   all lines from *.vss, store its values into vs.  Struct vs_controls,
 *   passed to get_style, contains pointers to vs's elements.  Close fdin.
 */
	memset((char *)&vs, 0, sizeof(VS_STYLE));
	vs.full_pg_align = vs.repg_align = 10;		/* "Try alignment" default 10 */
	for(i = 0; i < 6; i++)						/* Default try-order 1-6 */
		vs.try_order[i] = i + 1;
	if (get_style(wn, fdin, filename, (int16)NUM_VS_DATA, vs_controls))
		return(1);
/*
 * Copy data just read in static VS_STYLE vs into perm struct vs_style.
 */
	strcpy(vs.filename,filename);
	memcpy((char *)vs_style, (char *)&vs, sizeof(VS_STYLE));
	return(0);
}
/******************************************************************/

MP_STYLE_CNTRLS pelem_controls[] = {
	{"NUM_COLS"				,&pel.num_cols		,mp_numeric },
	{"COL_WIDTH"			,&pel.col_width		,mp_pica_hor },
	{"GUTTER_WIDTH"			,&pel.gutter_width	,mp_pica_hor },
	{"PAGE_WIDTH"			,&pel.page_width	,mp_pica_hor },
	{"PAGE_DEPTH"			,&pel.page_depth	,mp_pica_ver }};

#define NUM_PELEM_DATA (sizeof(pelem_controls) / sizeof(MP_STYLE_CNTRLS))

int put_pelem(WYSIWYG *wn, PELEM *pel_data,
				char *tree, char *dir, char *filename)
{
	Pfd fdout;
	char buff[256];
	int i;
	
	if ((fdout = p_open(tree, PELEM_FILE, dir, filename, "w+")) == P_ERROR)
		return(1);
	memcpy((char *)&pel, (char *)pel_data, sizeof(PELEM));
	for(i = 0; i < NUM_PELEM_DATA; i++)	/* output data */
	{
		char *temp;

		memset(buff, 0, sizeof(buff));	/* Init line:  All nulls,	*/
										/*  & keyword.	*/
		sprintf(buff, "%s:  ", pelem_controls[i].mp_string);
		temp = strchr (buff, '\0');		/* Find loc for value,	*/
		if(pelem_controls[i].func)		/*  translate struct values
											into ascii line:	*/
			(*(pelem_controls[i].func))(wn, &pelem_controls[i], temp, TO_ASCII);
		temp = strchr (buff, '\0');		/* Put NL after that,	*/
		*temp = '\n';
		p_fputs(buff, fdout);			/* Write line to disk file.	*/
	}
	p_close(fdout);
	return(0);
}

/*********************************************************************/

/*ARGSUSED*/
void mp_numeric(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	int ival;
	if(io == TO_ASCII)
		itoa((int)(*cntrl -> mp_style_word), ptr);
	else
	{
		ival = atoi(ptr);
		*cntrl -> mp_style_word = (int16)(ival & 0xffff);
	}
}

/********************************************************************/

void mp_pica_hor(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	int32 ival;
	uint32 units;
	REL_WORDS syntax;
	if(io == TO_ASCII)
	{
		memset((char *)&syntax, 0, sizeof(REL_WORDS));
		units = PICA << PL_REL_SHIFT;
		ival = (int32)(*cntrl -> mp_style_word);
		syntax.offset = lmt_abs_to_off(wn, X_REF, units, ival);
		lmt_syntax_to_ascii(&syntax, 0, ptr);
	}
	else
	{
		if(lmt_size_parser(&syntax, ptr, 0)) /* 0 to 1 */
			p_info(PI_ELOG, "MP, ascii convert error for %s\n", ptr);
		else
		{
			ival = lmt_off_to_abs(wn, X_REF, syntax.offset);
			*cntrl -> mp_style_word = (int16)(ival & 0xffff);
		}
	}
}

/********************************************************************/

void mp_pica_ver(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	int32 ival;
	uint32 units;
	REL_WORDS syntax;
	
	if(io == TO_ASCII)
	{
		memset((char *)&syntax, 0, sizeof(REL_WORDS));
		units = PICA << PL_REL_SHIFT;
		ival = (int32)(*cntrl -> mp_style_word);
		syntax.offset = lmt_abs_to_off(wn, Y_REF, units, ival);
		lmt_syntax_to_ascii(&syntax, 0, ptr);
	}
	else
	{
		if(lmt_size_parser(&syntax, ptr, 0))
			p_info(PI_ELOG, "MP, ascii convert error for %s\n", ptr);
		else
		{
			ival = lmt_off_to_abs(wn, Y_REF, syntax.offset);
			*cntrl -> mp_style_word = (int16)(ival & 0xffff);
		}
	}
}


/********************************************************************/

void mp_pts_ver(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	int32 ival;
	uint32 units;
	REL_WORDS syntax;
	
	if(io == TO_ASCII)
	{
		memset((char *)&syntax, 0, sizeof(REL_WORDS));
		units = PICA << PL_REL_SHIFT;
		ival = (int32)(*cntrl -> mp_style_word);
		syntax.offset = lmt_abs_to_off(wn, Y_REF, units, ival);
		lmt_syntax_to_ascii_pts(&syntax, ptr);
	}
	else
	{
		if(lmt_size_parser(&syntax, ptr, 1))
			p_info(PI_ELOG, "MP, ascii convert error for %s\n", ptr);
		else
		{
			ival = lmt_off_to_abs(wn, Y_REF, syntax.offset);
			*cntrl -> mp_style_word = (int16)(ival & 0xffff);
		}
	}
	return;
}

/********************************************************************/

#ifdef NOT_USED_BY_ANYONE
void mp_pts_hor(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	int32 ival;
	uint32 units;
	REL_WORDS syntax;
	
	if(io == TO_ASCII)
	{
		memset((char *)&syntax, 0, sizeof(REL_WORDS));
		units = PICA << PL_REL_SHIFT;
		ival = (int32)(*cntrl -> mp_style_word);
		syntax.offset = lmt_abs_to_off(wn, X_REF, units, ival);
		lmt_syntax_to_ascii_pts(&syntax, ptr);
	}
	else
	{
		if(lmt_size_parser(&syntax, ptr, 1))
			p_info(PI_ELOG, "MP, ascii convert error for %s\n", ptr);
		else
		{
			ival = lmt_off_to_abs(wn, X_REF, syntax.offset);
			*cntrl -> mp_style_word = (int16)(ival & 0xffff);
		}
	}
}
#endif 

/********************************************************************/

void cal_try_ascii(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr)
{
	int i;
	int32 ival;
	uint32 units;
	REL_WORDS syntax;
	
	memset(&ascii_try[0][0], 0, sizeof(ascii_try));
	for(i = 0; i < 10; i++)
	{
		memset((char *)&syntax, 0, sizeof(REL_WORDS));
		units = PICA << PL_REL_SHIFT;
		ival = (int32)(*(cntrl -> mp_style_word + i));
		syntax.offset = lmt_abs_to_off(wn, Y_REF, units, ival);
		lmt_syntax_to_ascii_pts(&syntax, &ascii_try[i][0]);
	}							/* end for all ten tries */
	sprintf(ptr, "%3s, %3s, %3s, %3s, %3s, %3s, %3s, %3s, %3s, %3s",
			&ascii_try[0][0], &ascii_try[1][0], &ascii_try[2][0],
			&ascii_try[3][0], &ascii_try[4][0], &ascii_try[5][0], 
			&ascii_try[6][0], &ascii_try[7][0], &ascii_try[8][0], 
			&ascii_try[9][0]);
}


/********************************************************************/

void calc_try_data(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl)
{
	int i;
	int32 ival;
	REL_WORDS syntax;
	
	for(i = 0; i < 10; i++)
	{
		if(ascii_try[i][0] == 0)
			continue;
		if(lmt_size_parser(&syntax, ascii_try[i], 1))
		{
			p_info(PI_ELOG, "MP, ascii convert error for %s\n", &ascii_try[i]);
			return;
		}
		ival = lmt_off_to_abs(wn, Y_REF, syntax.offset);
		*(cntrl -> mp_style_word + i) = (int16)(ival & 0xffff);
	}							/* end for all tries */
}

/********************************************************************/

void calc_try_num(MP_STYLE_CNTRLS *cntrl, int num)
{
	int i;
	int ival;
	
	for(i = 0; i <= num; i++)
	{
		ival = atoi(&ascii_try[i][0]);
		*(cntrl -> mp_style_word + i) = (int16)(ival & 0xffff);
	}
}

/********************************************************************/
/*	parse a try table line											*/

int fill_try(char *ptr, int num)
{
	int i, j;
	char *p;
	
	memset(&ascii_try[0][0], 0, sizeof(ascii_try));
	i = j = 0;
	p = ptr;
	while(*p)
	{
		if(*p == ',')
		{
			i++;
			if(i > num)
			{
				p_info(PI_ELOG, "MP, illegal try number in %s\n", ptr);
				return(1);
			}
			j = 0;
			p++;
			continue;
		}
		else if(isdigit(*p))
			ascii_try[i][j++] = *p++;
		else if(*p == '.')
			ascii_try[i][j++] = *p++;
		else if((*p == ' ') || (*p == '\t'))
			p++;
		else
		{
			p_info(PI_ELOG, "MP, illegal try %s\n", ptr);
			return(1);
		}
	}
	return(0);
}

/********************************************************************/

/*ARGSUSED*/
void mp_ld_try(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	int i;
	
	if(io == TO_ASCII)
	{
		memset(&ascii_try[0][0], 0, sizeof(ascii_try));
		for(i = 0; i < 10; i++)
			itoa((int)(*(cntrl -> mp_style_word + i)), &ascii_try[i][0]);
		sprintf(ptr, "%3s, %3s, %3s, %3s, %3s, %3s, %3s, %3s, %3s, %3s",
				&ascii_try[0][0], &ascii_try[1][0], &ascii_try[2][0],
				&ascii_try[3][0], &ascii_try[4][0], &ascii_try[5][0],
				&ascii_try[6][0], &ascii_try[7][0], &ascii_try[8][0],
				&ascii_try[9][0]);
	}
	else
	{
		if(fill_try(ptr, 9))
			return;
		calc_try_num(cntrl, 9);
	}
}

/********************************************************************/

void mp_cuts_try(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	if(io == TO_ASCII)
		cal_try_ascii(wn, cntrl, ptr);
	else
	{
		if(fill_try(ptr, 9))
			return;
		calc_try_data(wn, cntrl);
	}
}


/********************************************************************/

void mp_fn_try(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	
	if(io == TO_ASCII)
		cal_try_ascii(wn, cntrl, ptr);
	else
	{
		if(fill_try(ptr, 9))
			return;
		calc_try_data(wn, cntrl);
	}
}


/********************************************************************/

void mp_ln_try(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	
	if(io == TO_ASCII)
		cal_try_ascii(wn, cntrl, ptr);
	else
	{
		if(fill_try(ptr, 9))
			return;
		calc_try_data(wn, cntrl);
	}
}

/********************************************************************/

void mp_para_try(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	
	if(io == TO_ASCII)
		cal_try_ascii(wn, cntrl, ptr);
	else
	{
		if(fill_try(ptr, 9))
			return;
		calc_try_data(wn, cntrl);
	}
}

/********************************************************************/

void mp_pg_try(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	
	if(io == TO_ASCII)
		cal_try_ascii(wn, cntrl, ptr);
	else
	{
		if(fill_try(ptr, 9))
			return;
		calc_try_data(wn, cntrl);
	}
}

/*********************************************************************/

/*ARGSUSED*/
void mp_try_order(WYSIWYG *wn, MP_STYLE_CNTRLS *cntrl, char *ptr, int io)
{
	char ascii_tries[6][32];
	int i;
	
	if(io == TO_ASCII)
	{
		memset(&ascii_tries[0][0], 0, sizeof(ascii_tries));
		for(i = 0; i < 6; i++)
			itoa((int)(*(cntrl -> mp_style_word + i)), &ascii_tries[i][0]);
		sprintf(ptr, "%3s, %3s, %3s, %3s, %3s, %3s",
				&ascii_tries[0][0], &ascii_tries[1][0], &ascii_tries[2][0],
				&ascii_tries[3][0], &ascii_tries[4][0], &ascii_tries[5][0]);
	}
	else
	{
		if(fill_try(ptr, 5))
			return;
		calc_try_num(cntrl, 5);
	}
}

/*********************************************************************/

/*ARGSUSED*/
void mp_non_terms(WYSIWYG *wn,MP_STYLE_CNTRLS *cntrl,char *ptr,int io)
{
	int i;
	char *p;
	if(io == TO_ASCII)
	{
		*ptr = 0;
		for(i = 0; i < br.pg_non_term; i++)
			sprintf(ptr + strlen(ptr),"%4d,",br.pg_non_term_list[i]);
		sprintf(ptr + strlen(ptr),"0");
	}
	else
	{
		i = 0;
		br.pg_non_term = -1;
		p = ptr;
		while(*p)
		{
			if(*p == ',')
			{
				i++;
				p++;
				if(i >= 10)
				{
					if ( (*p == '0') || !*p)
						return;
					p_info(PI_ELOG, "Too many page non-terminators %s\n",ptr);
					return;
				}
				continue;
			}
			else if(isdigit(*p))
			{
				br.pg_non_term = i;
				br.pg_non_term_list[i] *= 10;
				br.pg_non_term_list[i] += *p - '0';
				p++;
			}
			else if((*p == ' ') || (*p == '\t'))
				p++;
			else
			{
				p_info(PI_ELOG, "MP, illegal try %s\n", ptr);
				return;
			}
		}
	}
}

/*********************************************************************/

/*ARGSUSED*/
void mp_preference(WYSIWYG *wn,MP_STYLE_CNTRLS *cntrl,char *ptr,int io)
{
	int ival;
	
	if(io == TO_ASCII)
	{
		ival = *cntrl -> mp_style_word;
		itoa(ival , ptr);
	}
	else
	{
		ival = atoi(ptr);
		*cntrl -> mp_style_word = (int16)(ival & 0xffff);
	}
}

/*********************************************************************/
/*  General-use routine to read in the contents of any of the six MP style
	files (*.brs, *.chs, *.vss, *.ils, *.fns, *.sns), based on the passed
	control structure MP_STYLE_CNTRLS *controls.			*/

int get_style (WYSIWYG *wn, Pfd fdin, char *filename,
			int16 num_data, MP_STYLE_CNTRLS *controls)
{
	int16 i, match;
	char line[256];
	char *string, *val;
/*
 * Loop to read in each line from file, store it into a struct item(s).
 */
	for(;;)
	{
		memset(line, 0, sizeof(line));	/* Init & read in next line	*/
		if ((p_fgets(line, sizeof(line) - 1, fdin)) == NULL)
			break;
		if (*line == '#') continue;		/* Comment, skip		*/
		P_CLEAN (line);					/* Get rid of \n or \r	*/
		string = line;					/* Pointer to keyword	*/
		while ((*string) && ((*string == ' ') || (*string == '\t')))
			string++;
		if (*string == '\0') continue;	/* No keyword, skip the line.	*/
										/* Find ":" and null it out.	*/
		if ((val = strchr (line, ':')) == NULL)
		{
			p_info(PI_ELOG, "MP, illegal line in file \"%s\": \"%s\"\n", 
					filename, line);
			continue;
		}
		*val++ = '\0';					/* isolate string */
										/* Pointer to value string:	*/
		while ((*val) && ((*val == ' ') || (*val == '\t')))
			val++;
										/* Look up keyword in struct:	*/
		match = -1;
		for (i = 0; i < num_data; i++)
		{
			if ((strcmp (string, controls[i].mp_string)) == 0)
			{
				match = i;
				break;
			}
		}
							/* If keyword valid, call function to translate 
								ascii val(s) into proper struct values:	*/
		if (match >= 0 && controls[match].func != 0)
			(*(controls[match].func))(wn, &controls[match], val, TO_DATA);
		else
			p_info(PI_ELOG, "MP, line \"%s:\" in file \"%s\" is unknown.\n",
					string, filename);
	}
/*
 * File is finished.  Close, return.
 */
	p_close(fdin);
	return(0);
}
