#ifndef _WINDOW_H
#define _WINDOW_H

#include "p_lib.h"
#include "que.h"
#include "image.h"
#include "llist.h"
#include "map.h"

typedef struct struct_wysiwyg WYSIWYG;		/* Needed for map.h */

typedef struct struct_file_list {			/* Needed for map.h */
						/* pub_display.c:OH_Scan_Pub_Display fills the fl's.  */
	QE que;				/* pointer to next & previous items */
	short selected;		/* this item is currently selected on screen */
	WYSIWYG *wn_file;	/* if != 0, it is on the screen */
	short file_num;		/* Inside a WYSIWYG structure for a unit window,
							it is the file number from 0 to n */
	short type;			/* type of file */
	short misc;			/* per type of file:
							FL_galley, FL_member => Text file number
							FL_master, FL_layout, FL_page => Layout number
							...
						*/
	char PageNum[SZ_PAGE_STRING];/* For FL_layout => "0"
										FL_page => "23", "IV", "5A", ...
								 */
	void *label;		/* if used - string displaying File info */
	short index;		/* file index in misc lists ??? Is it used ? */
	short flags;		/* misc flags, see defines */
	int dir_type;		/* type of directory passed to p_open */
	char filename[MAX_NAME*2];	/* Name of file (can now be 64 to support
									long graphics file names.  */
	QUEUE Qmap_data;	/* data from .map file */
} FILE_LIST;

#define BYTE  8
#define BUF_SIZE 2048
#define ZOOM_UNIT 1000      /* Internal value for zoom equal to 100%  */
#define HALF_ZOOM_UNIT 500
									/* defines for flags in FILE_LIST */
#define NEED_TO_MAP			1		/* Need to create a .map file */
#define NEED_TO_HNJ			2		/* Need to flash and NEED_TO_MAP*/
#define NEED_TO_MAP_HNJ		3		/* Need to do both */
#define FILE_SENT_TO_HNJ	4		/* File has been sent to HNJ */
#define FILE_UP_TO_DATE		8		/* File has already been written
										to disk during flash. Do not
										rewrite it so HandJ can read
										it. */
#define DELETE_FILE			16		/* When re-scanning a project.
										assume you have to delete every-
										ting until proven otherwise
										(flag being reset).
									   Short life (only valid during
									   re-scan and only for layouts
									   and text file. */

#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define DEFAULT_COLOR	-1

#define HIDE_DISPLAY	0
#define CREATE_DISPLAY	1
#define MERGE_DISPLAY	2

#define h_pix(win,hu) ((int32)(((hu)*win->izt) + \
						((hu>0) ? win->mr : -win->mr)) / (int32)win->mb)
#define v_pix(win,vu) ((int32)(((vu)*win->izt) + \
						((vu>0) ? win->lr : -win->lr)) / (int32)win->lb)

#define pix_h(win,px) ((int32)(((px)*win->mb) + \
				(((int32)px>0) ? (win->izt >> 1) : -(win->izt >> 1))) \
													/ (int32)win->izt)
#define pix_v(win,px) ((int32)(((px)*win->lb) + \
				(((int32)px>0) ? (win->izt >> 1) : -(win->izt >> 1))) \
													/ (int32)win->izt)

#define COL_GUIDES(wn)	wn -> selected_lay -> col_guides
#define CUR_LOCKS(wn)	wn -> cur_locks
#define LAYOUT(wn)		wn -> selected_lay ->
#define ELE_DATA(i)		wn -> selected_lay -> frames[(i)] -> ele ->
#define EMAP_DATA(i)	wn -> selected_lay -> frames[(i)] -> ele -> map_data.
#define REL_DATA(i)		wn -> selected_lay -> frames[(i)] -> rel_data.
#define FRAME_DATA(i)	wn -> selected_lay -> frames[(i)] ->
#define FMAP_DATA(i)	wn -> selected_lay -> frames[(i)] -> Fmap_data ->
#define lELE_DATA(layout, i)	layout -> frames[(i)] -> ele ->
#define lEMAP_DATA(layout, i)	layout -> frames[(i)] -> ele -> map_data.
#define lREL_DATA(layout, i)	layout -> frames[(i)] -> rel_data.
#define lFRAME_DATA(layout, i)	layout -> frames[(i)] ->
#define lFMAP_DATA(layout, i)	layout -> frames[(i)] -> Fmap_data ->

#define FPC 5

#define FILE_NOT_SELECTED	0	/* Used for penta pushb. in unit menu */
#define FILE_SELECTED		1
#define FILE_CLOSED			FILE_NOT_SELECTED
#define FILE_OPENED			2
#define FILE_ICONIFIED		4
#define FILE_EXITING		8
#define FILE_READ_IN		16

#define FL_publication	0x01
#define FL_page			0x02
#define FL_layout		0x04
#define FL_member		0x08
#define FL_galley		0x10
#define FL_graphic		0x20
#define FL_master		0x40
#define FL_memberunit	0x80

							/* types of wysiwyg windows */
#define WW_publication	1
#define WW_page			2	/* Layout with a page number assigned */
#define WW_layout		3
#define WW_member		4
#define WW_galley		5
#define WW_graphic		6
#define WW_system		7
#define WW_trace		8
#define WW_edit_view	9
#define WW_specs		10

									/* For use with lmt_off_to_abs() */
#define X_REF			1			/* Transform coordinate into
										the "x" environment */
#define Y_REF			0			/* Transform coordinate into
										the "y" environment */

typedef struct struct_element {
	int32 rect_top;
	int32 rect_bottom;
	int32 rect_y_center;
	int32 rect_left;
	int32 rect_right;
	int32 rect_x_center;
	int32 rot_rect_top;
	int32 rot_rect_bottom;
	int32 rot_rect_y_center;
	int32 rot_rect_left;
	int32 rot_rect_right;
	int32 rot_rect_x_center;
	int16 preview_done;
	int16 n_points;					/* Number of points in list_points
										and out_lst_pts. */
	MAP_DATA map_data;				/* Wysiwyg data for HNJ */
	DRAW_POINT_X_Y *list_points;	/* Original list */
	DRAW_POINT_X_Y *out_lst_pts;	/* Processed list */
	int16 trap_pts;					/* Number of pts in out_trap_pts */
	DRAW_POINT_X_Y *trap_points;	/* List of pts for trapping if
										trapping != 0 */
	QUEUE items;					/* WYSIWYG */
	QUEUE oitems;					/* overflow WYSIWYG */
	QUEUE shape;					/* Shape of the element */
	struct struct_element *prev;	/* Inside the same frame */
	struct struct_element *next;	/* Inside the same frame */
	struct struct_element *fwd;		/* Inside the same page */
	struct struct_element *rev;		/* Inside the same page */
} ELEMENT;

/*
 * Usage of RELATIVES elements is detailed in the following header files:
 *   rel_data.h		-- General usage by designmaster, HandJ, MasterPage, pp.
 *   frame.h		-- Value ranges and symbols used to access the elements,
 *						for general usage.
 *   pg_layout.h	-- More specialized usage by MasterPage.  Contains
 *						overlap with definitions in rel_data.h & frame.h .
 * and in the following doc file:
 *   layout.doc		-- Summarizes usage of elements by 5 main record types,
 *						in tabular format.
 * Comments should be entered in those files rather than here.
 */
typedef struct {
	char	c0,  c1,  c2,  c3,  c4,  c5,  c6,  c7,  c8,  c9;
	char   c10, c11, c12, c13, c14, c15;

	char	*p0, *p1, *p2, *p3;

	int16   i0,  i1,  i2,  i3,  i4,  i5,  i6,  i7,  i8,  i9;
	int16  i10, i11, i12, i13, i14, i15, i16, i17, i18, i19;
	int16  i20, i21, i22, i23, i24, i25, i26, i27, i28, i29;
	int16  i30, i31;

	uint32  d0,  d1,  d2,  d3,  d4,  d5,  d6,  d7,  d8,  d9;
	uint32 d10, d11, d12, d13, d14, d15, d16, d17, d18, d19;
	uint32 d20, d21, d22, d23, d24, d25, d26, d27, d28, d29;

	uint32  t0,  t1,  t2,  t3,  t4,  t5,  t6,  t7,  t8,  t9;
	uint32 t10;
} RELATIVES;


typedef struct {
	int num_boxes;				/* Number of widget (boxes)
														around frame */
	void **w_boxes;				/* List of widget ptr */
	int parent;					/* Frame number of parent */
	int f_flags;				/* Flag for frame */
	int32 max_uid;
	int32 frame_uid;
	int32 top;
	int32 bottom;
	int32 y_center;
	int32 left;
	int32 right;
	int32 x_center;
    int32 prev_top;
    int32 prev_left;
    int32 prev_right;
    int32 prev_bottom;
	POINT_X_Y rot_top_left;
	POINT_X_Y rot_top_right;
	POINT_X_Y rot_top_x_center;
	POINT_X_Y rot_bottom_left;
	POINT_X_Y rot_bottom_right;
	POINT_X_Y rot_bottom_x_center;
	POINT_X_Y rot_y_center_left;
	POINT_X_Y rot_y_center_right;
	POINT_X_Y rot_y_center_x_center;
	int32 inter_top;
	int32 inter_bottom;
	int32 inter_y_center;
	int32 inter_left;
	int32 inter_right;
	int32 inter_x_center;
	int32 last_bearoffs;		/* Bearoff values at last calculation */
	int32 leading;				/* Used for xL command
									where x is a number */
	int16 n_points;				/* Number of points in the list */
	DRAW_POINT_X_Y *list_points;		/* Original list */
	DRAW_POINT_X_Y *out_lst_pts;		/* Processed list */
	int32 num_shapes;			/* Number of points in gr_list_pts */
	DRAW_POINT_X_Y *shape_pts;	/* Points from .PX or shape define */
	int32 num_bearoffs;			/* Number of points in bearoff */
	DRAW_POINT_X_Y *bearoff_pts;/* Shape points expanded by bearoffs */
	int16 b_points;				/* Number of points in bearoff list */
								/* b_points == num_bearoffs? */
	DRAW_POINT_X_Y *bearoff_lst;/* Bearoff processed list */
	GRAPHIC_IMAGE *gr;			/* Pointer to graphic */
	MAP_DATA *Fmap_data;		/* Wysiwyg data for the frame */
	ELEMENT *ele;
	ELEMENT *neg_ele;
	RELATIVES rel_data;
        BOOLEAN do_crop;
} FRAME;

typedef struct {
	QE que;						/* pointer to next and previous items */
	FRAME frames[FPC];			/* frames */
} FR_CHUNK;

typedef struct {
	short gwid;			/* width of glyph in dots */
	short gdep;			/* heigth of glyph in dots */
	short xoff;			/* distance above baseline of glyph start */
	short yoff;			/* distance left of baseline of glyph start */
	short wid;			/* total width of character */
	char *ptr;			/* character pointer to bitstream */
} CHDEF;

typedef struct {
	QE que;				/* pointer to next & previous items */
	uint32 offset;		/* Place of the guideline on the layout */
} GUIDELINES;

typedef struct {
	char column_guides_origin;
	int columns;
	uint32 col_width;
	uint32 col_gutter;
	int rows;
	uint32 row_depth;
	uint32 row_gutter;
	hmu start_x, end_x;
	vmu start_y, end_y;					/* Rectangle around it */
} COLUMN_GUIDE;

typedef struct {
	int lead_guide_frame_number;
	QUEUE hor_guidelines;
	QUEUE ver_guidelines;
	QUEUE crosshairs;
								/* Rulers */
	uint32 h_offset;
	int16 h_rel0;
	int16 h_rel1;
	int32 x_machine;
	uint32 ex;
	uint32 ew;
	uint32 v_lines;
	uint32 v_offset;
	int16 v_rel0;
	int16 v_rel1;
	uint32 ey;
	uint32 eh;
	int32 y_machine;
} LOCKS;

typedef struct {
	int x;
	int hoff;
	int hl[32];
	int hc;
	int hlab;
	int hcorr;
	float hbase;
	float hinc;
	float hzero;
	float hmin;
	float hmax;
	int y;
	int voff;
	int vl[32];
	int vc;
	int vlab;
	int vcorr;
	float vbase;
	float vinc;
	float vzero;
	float vmin;
	float vmax;
} RULER_STRUCT;

/* See image.h for the other typedefs */

typedef struct struct_layout {
	QE que;
	void *menu;					/* For layouts, pointer to menu */
	void *menu_info;			/* LAY_INFO information */
	FILE_LIST *fl;
	COLUMN_GUIDE col_guides;	/* data in column guides menu */
	int preview_pending;		/* things to be done */
	int page;					/* page number if page display */
	char filename[MAX_NAME];
	Pfd fofd;					/* .fo file descriptor */
	int fo_rec;
	int fo_wrd;
	int CmdFM_FootCount;
	int CmdFM_TextCount;
	int last_color;				/* last color used in previewing */
	int Frame;					/* Frame being previewed */
	ELEMENT *Ele;				/* Element being previewed */
	int end_rec;				/* end pointer for page preview */
	int end_wrd;				/* end pointer for page preview */
								/* vert just display data */
	int vj_line_cnt;			/* current basis for adjustments */
	int vj_line_adj;			/* adjustments per cnt above */
	int vj_para_cnt;
	int vj_para_adj;
	int vj_exld_cnt;
	int vj_exld_adj;
	int vj_vbnd_cnt;
	int vj_vbnd_adj;
	int vj_top;
	int vj_bottom;
	int vj_flags;
	vmu ldtot;
	int32 spec_leading;			/* Temporary value for interference */
	int32 block_leading;		/* Temporary value for interference */
	int32 dpo;					/* Temporary value for interference */
	int32 wo;					/* Temporary value for interference */
	hmu xmin;					/* left limit of display */
	vmu ymin;					/* top limit of display */
	hmu xmax;					/* right limit of display */
	vmu ymax;					/* bottom limit of display */
	hmu xgmax;                                      /* max x for frame in the graphic menu*/ 
	vmu ygmax;                                      /* max y for frame in the graphic menu*/ 
	hmu x_move;					/* left border for display layout */
	vmu y_move;					/* top border for display layout */
	x_coord xpin;				/* left border for display WYSIWYG */
	y_coord ypin;				/* top border for display WYSIWYG */
	int frame_vs_overflow;		/* frame[0] or frame overflow[1] */
	DISP_ITEM *tabmode;			/* points to [bt until [et or [be->[ee*/
	int tabmode_flags;			/* in use for [bt,[et or [be,[ee */
	QUEUE items;				/* first to last items on screen */
	QUEUE link_frame;			/* Queue of frames lined together */
	int num_frames;				/* number of frames in use */
	int max_frames;				/* size of current frame array */
	FRAME **frames;				/* pointer to array of frame pointers */
	QUEUE f_chunks;				/* queue of mem alloc. to frame data */
} LAYOUT_DESC;

typedef struct {
	QE que;
	int facs_id;
	LAYOUT_DESC *layout;
	char filename[MAX_NAME];
} LOCKED_FILE;

typedef struct {
	QE que;					/* pointer to next and previous items */
	LAYOUT_DESC *lay;		/* Layout which crosshair is based upon */
	int32 x_machine;		/* Machine unit */
	int32 y_machine;		/* Machine unit */
	uint32 h_offset;		/* horizontal offset of crosshair */
	uint32 v_offset;		/* vertical offset of crosshair */
	int16 h_rel0;			/* Relative for horizontal crosshair */
	int16 h_rel1;			/* Relative for horizontal crosshair */
	int16 v_rel0;			/* Relative for vertical crosshair */
	int16 v_rel1;			/* Relative for vertical crosshair */
	PPOINT v_ch_top;		/* Top in pixel of vertical crosshair */
	PPOINT v_ch_bottom;		/* Bottom in pixel of vertical crosshair */
	PPOINT v_ch_left_right;	/* Left and right in pixel of vertical
								crosshair (1 pixel wide) */
	PPOINT h_ch_left;		/* Left in pixel of horizontal crosshair */
	PPOINT h_ch_right;		/* Right in pixel of horizontal crosshair */
	PPOINT h_ch_top_bottom;	/* Top and bottom in pixel of horizontal
								crosshair (1 pixel high) */
	uint32 mask;			/* Mask indicating
								if primary or secondary */
} CROSSHAIRS;				/* see locks.h for defines for
								this structure */

	/* The next two structure, GLOBAL_DATA and PREFERENCES, are global
		for all layouts in a window. The only difference is one is
		made of uint16 and the other one of uint32.
	*/

typedef struct {
	uint16 display_options;		/* 0 DO NOT CHANGE THE ORDER */
	uint16 text_display;		/* 1 */
	uint16 graphic_display;		/* 2 */
	uint16 zoom_option;			/* 3 */
	uint16 zoom_value;			/* 4 */
	uint16 snap_from;			/* 5 */
	uint16 snap_to_h;			/* 6 */
	uint16 snap_to_v;			/* 7 */
	uint16 horz_ruler_origin;	/* 8 */
	uint16 vert_ruler_origin;	/* 9 */
	uint16 horz_ruler_units;	/* 10 */
	uint16 vert_ruler_units;	/* 11 */
	uint16 default_flags;		/* 12 */
} GLOBAL_DATA;

typedef struct {
	uint32 rule_weight;			/* 0 Weight applied to rules */
	uint32 box_weight;			/* 1 Weight applied to open boxes */
	uint32 dummy_2;				/* 2 */
	uint32 dummy_3;				/* 3 */
	uint32 dummy_4;				/* 4 */
	uint32 dummy_5;				/* 5 */
	uint32 gutter_top;			/* 6 Default frame gutters */
	uint32 gutter_bottom;		/* 7 */
	uint32 gutter_left;			/* 8 */
	uint32 gutter_right;		/* 9 */
} PREFERENCES;

struct struct_wysiwyg {
	QE que;						/* pointer to next & previous items */
	void *X11_vars;				/* All the X11 information for DM */
	char tree_name[MAX_NAME];	/* tree ID for this window */
	char dir_name[MAX_NAME];	/* name of unit/galley/graphic dir */

	GLOBAL_DATA global_data;
	PREFERENCES preferences;
	int expose_pending;			/* expose event occured, this window */

	int top, left;				/* top left corner of */
	unsigned int width, height;	/* rectangle to be exposed (pixel) */

	int type;					/* type of window, see #defines above */

	WYSIWYG *unit_win;			/* unit to which this window belongs */
	vmu name_box;				/* Height of name box based on zoom */
	hmu mu_xmin;				/* Size in mach. unit of drawing area */
	vmu mu_ymin;				/* Size in mach. unit of drawing area */
	hmu mu_xmax;				/* Size in mach. unit of drawing area */
	vmu mu_ymax;				/* Size in mach. unit of drawing area */
	int num_files;				/* total number of file in file_list */
	int horiz_files;			/* number of files across in display */
	int last_accessed;			/* For publication window:
										number of last selected icon.
								   For layout or page window:
										number of files being hnj'ed. */
	BOOLEAN last_action;		/* For publication window:
										Select/Deselect action
														of last icon.
								   For layout or page window:
										Use to pass frame number you
												want to clear text. */
	QUEUE files;				/* queue of first to last filenames */
	QUEUE locked_files;			/* queue of files that are locked */
	int starting_line_number;	/* Line to start displaying
									on opening (galley) */
	int izt;					/* zoom factor */
	int izty;                   /* zoom y factor */
	BOOLEAN AutoFit;			/* calculate zoom
									factor automatically */
	int16 yx_convert[2];		/* fill in with # to divide into 5400ths
									of a point to yield desired machine
									unit when calling 'interface'. [0]
									is for y and [1] is for x. */
	short dtext;				/* Display what in frame (layout) */
	short dgraph;				/* Display what in graphic
									frame (layout) */
	short mb,mr;				/* horizontal conversion factors */
	short lb,lr;				/* vertical conversion factors */
	hmu msb;					/* horizontal mru base */
	vmu ldb;					/* vertical mru base */
	LOCKS cur_locks;			/* data in locks menu */
	RULER_STRUCT rulers;		/* Ruler position data */
	BOOLEAN edit_bearoff;		/* Edit bearoff or shape */
	BOOLEAN absolute_mode;		/* Convert everything from relative
									to absolute */
	LLIST *LLcm_cell;			/* Color map information for wysiwyg */
	LLIST *LLcolor;				/* Color information from Penta
										color table */
	LAYOUT_DESC *selected_lay;	/* Selected layout */
	int selected_frame;			/* Quantity of frame selected */
	int selected_poly;			/* Number of dot selected */
	int sel_poly_frame;			/* Number of frame selected for poly */
	int toolbox_mode;			/* For layout or page window.
									For publication or edit_view,
									means are you in single or multiple
									window mode */
	QUEUE layout_list;			/* List of layout in this window */
	int min_ma;                 /* For negative margins alignment - the smallest one */
};

/* ==============  function prototypes ============= */

void clean_wysiwyg(WYSIWYG *wn);

typedef struct raw_graphic {
	QE que;						/* from graphics.h typdef MUM_SGF_ENTRY */
	char px_file_name[64];		/* Name of the original .pixel file  */
	time_t st_mtim;				/* Time of last modification */
	int16 horiz_dpi;			/* Horizontal dots/inch, (dots/inch) */
	int16 vert_dpi;				/* Vertical dots/inch,   (dots/inch) */
	int16 dpl;					/* Dots per line,        (pixels) */
	int16 lpg;					/* Lines per graphic,    (pixels) */
	int trim_left;				/* From left of bbox,    (20ths) */
	int trim_top;				/* From top of bbox,     (20ths) */
	int16 width;				/* Graphic width,        (20ths) */
	int16 depth;				/* Graphic depth,        (20ths) */
	int crop_left;				/* From left of bbox,    (20ths) */
	int crop_top;				/* From top of bbox,     (20ths) */
	int crop_width;				/* Cropped width,        (20ths) */
	int crop_depth;				/* Cropped depth,        (20ths) */
	int16 scale,scaley;         /* Scale percent                 */
	int16  file_format; /* 0 - old format, 1 - new format (versions 7.0 and up */
	int16 line_alignment;
	int16 bits_per_pixel;
	char *datafile;
} RAW;

#endif
