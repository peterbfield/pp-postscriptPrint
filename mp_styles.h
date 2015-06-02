#ifndef _MP_STYLES_H

#define _MP_STYLES_H

#include "penta.h"
#include "que.h"

#define TO_ASCII 1
#define TO_DATA 0
#define VS_EXLD 0
#define VS_CUTS 1
#define VS_FNS 2
#define VS_LINES 3
#define VS_PARAS 4
#define VS_DEPTH 5

typedef struct mp_control
	{
	char *mp_string;            /* string for ascii file            */
	int16 *mp_style_word;       /* pts to word.**** in mp_style_io  */
	void (*func)();             /* do this for i/o                  */
	} MP_STYLE_CNTRLS;

typedef struct
	{
	QE que;
	char filename[64];
	int16 style_group_num;				/* 1=primary style 2-5=re-try styles */
	int16 place_any;
	int16 place_only_singles;
	int16 place_only_multi;
	int16 never_before;
	int16 before_same_col;
	int16 before_same_pg;
	int16 before_face_pg;
	int16 before_any_pg;
	int16 top_qual;
	int16 center_qual;
	int16 bottom_qual;
	int16 stagger_col;
	int16 stagger_pg;
	int16 stagger_face_pg;
	int16 stack_hor;
	int16 stack_ver;
	int16 read_l_r;
	int16 read_t_b;
	int16 min_dep_w_ins;				/* in machine units			*/
	int16 normal_cut_space;				/* in machine units			*/
	int16 min_pg_qual;
									/* Second half describes how to fit &
									   break Autotab tables. */
									/* What page elements may a table's width
									   span? Priority #s (used by H&J): */
	int16 span_1col;					/* Single text column */
	int16 span_1col_snote;				/* 1 col plus sidenote area */
	int16 span_2cols;					/* Two text columns plus gutter */
	int16 span_3plus_cols;				/* Three+ text columns plus gutters */
	int16 span_full_page;				/* Full page, when there's extra sidenote 
											or other area */
	int16 span_past_wid_out;			/* Into outside mgn by picas_past_wid_out*/
	int16 span_past_wid_in;				/* Into inside mgn by picas_past_wid_in*/
	int16 span_rot_depth;				/* Rotated: Pg depth */
	int16 span_rot_beyond_depth;		/* Rotated: Past BOP by picas_beyond_dep */
	int16 span_seam_plus_cols;			/* Left pg + inner cols of right pg */
	int16 span_seam_2pages;				/* Left pg + right pg */
	int16 picas_past_wid_out;			/* Distance allowed into outside margin */
	int16 picas_past_wid_in;			/* Distance allowed into inside margin */
	int16 picas_beyond_dep;				/* Rotated: Distance allowed beyond pg
											dep, if span_rot_beyond_depth on */
									/* How table should break & place (MP): */
	int16 break_beyond_dep;				/* Distance by which table may break
											beyond page depth. */
	int16 rotated_wid_span_type;		/* 0=All cols 1=PgWid 2=Beyond pgWid. */
	int16 rotated_wid_span_picas;		/* Distance allowed beyond pg wid. */
	int16 min_dep_after_brk;			/* Min table dep after break. */
	int16 place_uprite_table;			/* 0=Top closest pg 1=Closest even pg */
	int16 place_rot_table;				/* 0=Top closest pg 1=Closest even pg */
	int16 place_xp_uprite_table;		/* 0=NA 1=Next pg 2=Even pg before last
											text 3=Same pg as text ends. */
	int16 place_xp_rot_table;			/* 0=NA 1=Next pg 2=Even pg before last
											text. */
	int16 rotated_alignment;			/* O-size rotated: 1=Top 2=Cen 3=Bot */
	int16 rotated_cover_foot;			/* O-size rotated: 0=None 1=Run foot
											2=Running head 3=Both. */
	int16 rotated_x_center;				/* Shallow rotated: 1=center in X  */
	int16 repeat_box_rot;				/* 1=Rpt boxhead on right page, for
											turned table. */

	} IL_STYLE;						/* Illustration Styles */

typedef struct
	{
	QE que;
	char filename[64];
	int16 style_group_num;				/* 1=primary style 2-5=re-try styles */
	int16 allow_widows;					/* 0=no, 1=yes				*/
	int16 widow_pct;					/* 0 -> 100%				*/
	int16 allow_orphans;				/* 0=no, 1=yes				*/
	int16 lines_above_head;
	int16 lines_below_head;
	int16 pg_non_term;					/* length of non-terminator
										   character list			*/
	int16 pg_non_term_list[10];			/* actual list of characters*/
	} BR_STYLE;						/* Breaking Style */

typedef struct
	{
	QE que;
	char filename[64];
	int16 style_group_num;				/* 1=primary style 2-5=re-try styles */
	int16 begin_unit_on;				/* 0=n/a,1=odd,2=even,3=next */ 
	int16 min_last_pg_lines;
	int16 auto_align;					/* 0=n/a,1=face,2=back,3=cols */
	int16 align_amt;					/* machine units			*/
	int16 align_opt;					/* 1=short only, 2=long only
										   3=short then long
										   4=long then short		*/
	int16 long_short_ok;				/* return to normal depth
										   after alignment,
										   0 = yes, 1 = no			*/
	int16 allow_short_pg;				/* when tries fail
										   0=n/a, 1=set short,
										   2=set long next short	*/
	int16 postproc_members;				/* 1=Process all member files 
										   thru H&J at end of page. */
	int16 max_err_count;				/* Default 1000 mp errors b4 abort */
	} CH_STYLE;						/* Chapter Style */

typedef struct
	{
	QE que;
	char filename[64];
	int16 style_group_num;				/* 1=primary style 2-5=re-try styles */
	int16 fn_place;						/* 1=bottom col where ref'd
										   2=bottom of page
										   4=specific column		*/
	int16 bot_ill_fn_place;				/* 0=Feet under text
										   1=Feet under bot ill if any */
	int16 fn_cols;						/* # if fn_place=2			*/
	int16 fn_col_wid;					/* col wid for fn_cols, 
										   in machine units			*/
	int16 fn_gutter;					/* gutter wid for fn_cols,
										   in machine units			*/
	int16 fn_chp_even;					/* 1=inside, 2=outside, 4=col */
	int16 fn_chp_even_col;				/* # if fn_chap_even = 4	*/
	int16 fn_chp_odd;					/* 1=inside, 2=outside, 4=col */
	int16 fn_chp_odd_col;				/* # if fn_chap_odd = 4		*/
	int16 fn_even;						/* 1=inside, 2=outside, 4=col */
	int16 fn_even_col;					/* # if fn_even = 4			*/
	int16 fn_odd;						/* 1=inside, 2=outside, 4=col */
	int16 fn_odd_col;					/* # if fn_even = 4         */
	int16 last_pg_fn_place;				/* 0=under text,
										   1=bottom of page			*/
	int16 dbl_up_evenly;				/* 0=use doube_up_space
										   1=space evenly in measure */
	int16 dbl_short_fns;				/* 0=no, 1=yes				*/
	int16 dbl_num;						/* 0 = 2, 1 = 3, 2 = 4,
										   3 = >4(as many wil fit) */
	int16 dbl_space;					/* space between short fns
										   in machine units			*/
	int16 space_above_cr;				/* space above cut off rule
										   in machine units			*/
	int16 space_below_cr;				/* space below cut off rule
										   in machine units         */
	int16 length_of_cr;					/* lenght of cut off rule	*/
	int16 indent_of_cr;					/* indent of cut off rule	*/
	int16 color_of_cr;					/* color of cut off rule	*/
	int16 weight_of_cr;					/* weight of cut off rule	*/
	int16 space_above_scr;				/* space above special cut off 
										   rule in machine units	*/
	int16 space_below_scr;				/* space below special cut off
										   rule in machine units    */
	int16 fn_space_place;				/* % adjust above cut off rule*/

	int16 allow_foot_break;				/* allow auto break of fns
										   0=no, 1=yes				*/
	int16 min_dep_w_broke_fn;			/* in machine units			*/
	int16 ccfn_allow_ep;				/* for col to col fns
										   0=no, 1=yes              */
	int16 ccfn_allow_orphans;			/* for col to col fns
										   0=no, 1=yes              */
	int16 ccfn_allow_widows;			/* for col to col fns
										   0=no, 1=yes				*/
	int16 ccfn_widow_pct;				/* 0 -> 100%				*/
	int16 ppfn_allow_ep;				/* for pg. to pg. fns
										   0=no, 1=yes              */
	int16 ppfn_allow_orphans;			/* for pg. to pg. fns
										   0=no, 1=yes              */
	int16 ppfn_allow_widows;			/* for pg. to pg. fns
										   0=no, 1=yes              */
	int16 ppfn_widow_pct;				/* 0 -> 100%                */
	int16 length_of_scr;				/* lenght special cut off rule*/
	int16 indent_of_scr;				/* indent special cut off rule*/
	int16 color_of_scr;					/* color special cut off rule */
	int16 weight_of_scr;				/* weight special cut off rule*/
	int16 space_above_ccr;				/* space above ccr 
										   rule in machine units	*/
	int16 space_below_ccr;				/* space below ccr
										   rule in machine units    */
	int16 length_of_ccr;				/* lenght ccr rule*/
	int16 indent_of_ccr;				/* indent ccr rule*/
	int16 color_of_ccr;					/* color ccr rule */
	int16 weight_of_ccr;				/* weight ccr rule*/
	} FN_STYLE;						/* Footnote Style */

typedef struct
	{
	QE que;
	char filename[64];
	int16 style_group_num;				/* 1=primary style 2-5=re-try styles */
	int16 side_notes;					/* 0=no, 1=yes				*/
	int16 sn_space;						/* let sn run into next para
										   0=yes, 1=no				*/
	int16 area_refs;					/* 0=no, 1=yes				*/
	int16 area_ref_inc;					/* line inc for area refs	*/
	int16 no_area_ref;					/* count head line for area ref
										   0=count, 1=dont count	*/
	int16 var_contin_lines;				/* 1=Support variable-depth continued-line 
											frames.    0=Don't.
											(NOTE: Support causes MP to call H&J
											on each CL frame.  MUCH slower!!!)  */
	} SN_STYLE;						/* SnArn Stles */

typedef struct
	{
	QE que;
	char filename[64];
	int16 style_group_num;				/* 1=primary style 2-5=re-try styles */
	int16 try_order[6];
	int16 try_table_expand[6][10];
	int16 try_table_shrink[6][10];
	int16 full_pg_abort;				/* 0 -> 12					*/
	int16 full_pg_alert;				/* 0 -> 12					*/
	int16 full_pg_align;				/* 0 -> 12					*/
	int16 repg_abort;	              	/* 0 -> 12                  */
	int16 repg_alert;					/* 0 -> 12                  */
	int16 repg_align;					/* 0 -> 12                  */
	} VS_STYLE;						/* Vertical Spacing Style */

typedef struct						/* This one's for a slightly different */
	{								/*  purpose: To extract certain key values
										from master .lay files & create a
										summary .pelem file for H&J Autotab. */
	int16 num_cols;					/* # columns in master layout */
	int16 col_width;				/* Basic column width */
	int16 gutter_width;				/* Gutter width between basic columns */
	int16 page_width;				/* Master layout page width */
	int16 page_depth;				/* Master layout page depth */
	} PELEM;

typedef struct
	{
	int16 align_folio_move;				/* move folio with
										   0=never, 1=short page
										   2=long page, 3=long&short */
	int16 basic_text_ld;				/* in machine units			*/
	int16 text_related_strings;			/* 0=no, 1=text related head
										   2=continued lines
										   3=trh & con line			*/
	} MP_DATA;			/* leftovers from old data base */

int get_br_style(WYSIWYG *wn, BR_STYLE *br_style,
						char *tree, char *dir, char *filename);
int put_br_style(WYSIWYG *wn, BR_STYLE *br_style,
						char *tree, char *dir, char *filename);
int get_ch_style(WYSIWYG *wn, CH_STYLE *ch_style,
						char *tree, char *dir, char *filename);
int put_ch_style(WYSIWYG *wn, CH_STYLE *ch_style,
						char *tree, char *dir, char *filename);
int get_fn_style(WYSIWYG *wn, FN_STYLE *fn_style,
						char *tree, char *dir, char *filename);
int put_fn_style(WYSIWYG *wn, FN_STYLE *fn_style,
						char *tree, char *dir, char *filename);
int get_il_style(WYSIWYG *wn, IL_STYLE *il_style,
						char *tree, char *dir, char *filename);
int put_il_style(WYSIWYG *wn, IL_STYLE *il_style,
						char *tree, char *dir, char *filename);
int get_sn_style(WYSIWYG *wn, SN_STYLE *sn_style,
						char *tree, char *dir, char *filename);
int put_sn_style(WYSIWYG *wn, SN_STYLE *sn_style,
						char *tree, char *dir, char *filename);
int get_vs_style(WYSIWYG *wn, VS_STYLE *vs_style,
						char *tree, char *dir, char *filename);
int put_vs_style(WYSIWYG *wn, VS_STYLE *vs_style,
						char *tree, char *dir, char *filename);
int put_pelem   (WYSIWYG *wn, PELEM *pel_data,
						char *tree, char *dir, char *filename);

#endif
