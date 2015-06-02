#ifndef _REL_DATA_H

#define _REL_DATA_H

#include "window.h"				/* for REL_DATA */

#define FULL_DEGREE 360

#define FULL_CIRCLE 3600
#define HALF_CIRCLE 1800
#define QUAT_CIRCLE 900
#define THQU_CIRCLE 2700

#define ABS_HORIZONTAL	0x01
#define ABS_VERTICAL	0x02
#define ABS_WIDTH		0x04
#define ABS_DEPTH		0x08

/*
 * Most macros and symbols in this file provide access to struct RELATIVES.
 * See window.h for definition of struct RELATIVES and for comment pointing
 *  the way to related header files.
 */
#define TF c0
#define LK c1

#define HNJ_T	p0				/* hnj text string */
#define STYL	p1				/* Style string */
#define B_RULES	p2				/* Breaking rules name */
#define G_NAME	p2				/* Graphic name */
#define V_TABLE	p3				/* Vertical spacing table */

#define XR1 i0					/* x relationship 1 */
#define XR2 i1					/* x relationship 2 */
#define YR1 i2					/* y relationship 1 */
#define YR2 i3					/* y relationship 2 */
#define WR1 i4					/* width relationship 1 */
#define WR2 i5					/* width relationship 2 */
#define DR1 i6					/* depth relationship 1 */
#define DR2 i7					/* depth relationship 2 */
#define T_ID i8					/* text id */
#define STF_PS i10				/* Set to fit point size */
#define STF_SS i11				/* Set to fit set size */
#define STF_LD i12				/* Set to fit leading */
#define FRD i13
#define CH i14
#define H_ID i15				/* Link number for specs */
#define LK_NUM i15				/* Link number */
#define ART_ID i19				/* Article id */

#define PS d0
#define LDG d1
#define WT d1
#define T_WIDTH d2				/* Trim width (specs) */
#define XO d2
#define YO d3
#define WO d4
#define DO d5
#define TG d6
#define BG d7
#define LG d8
#define RG d9
#define GR_TG d10
#define GR_BG d11
#define GR_LG d12
#define GR_RG d13
#define SS d15
#define P_LAY d10					/* previous layout */
#define P_T_ID d13					/* prevous text id */
#define N_LAY d16					/* next layout */
#define N_T_ID d17					/* next text id */
#define TL_ROUND d21				/* top left rounded corner */

#define OUTLINE				0x0001
#define OVALE_SHAPE			0x0002
#define OVERLAY				0x0004
#define POLYGON_SHAPE		0x0008
#define BEAROFF_SHAPE		0x0010
#define RECTANGLE_BEAROFF	0x0020
#define LAYER				0x0040
#define FG_ATTRIB           0x0080
#define FG_LAYER			0x0100

#define MP_OB_FRAME			0xfff /*frame_uid# - least signif bits OBJ_REF */
#define MP_OB_LAY			0xfffff /* lay# - most signif 20 bits OBJ_REF */
/*
 * struct RELATIVES locations common to multiple frame types:
 */
#define TYPE_OF_FRAME(i)	(REL_DATA(i) TF)
#define LOCKPOINT(i)		(REL_DATA(i) LK)
#define REL_FLAGS(i)		(REL_DATA(i) c5)
#define ROT_LOCKPOINT(i)    (REL_DATA(i) c6)
#define NUM_VJ(i)			(REL_DATA(i) c9)
#define VJ_REL_1_1(i)       (REL_DATA(i) c10)
#define VJ_REL_1_2(i)       (REL_DATA(i) c11)
#define VJ_REL_2_1(i)       (REL_DATA(i) c12)
#define VJ_REL_2_2(i)       (REL_DATA(i) c13)
#define DASH_ON(i)          (REL_DATA(i) c14)
#define ODASHON(i)          (REL_DATA(i) c15)

#define X_REL_1(i)          (REL_DATA(i) XR1)
#define X_REL_2(i)          (REL_DATA(i) XR2)
#define Y_REL_1(i)          (REL_DATA(i) YR1)
#define Y_REL_2(i)          (REL_DATA(i) YR2)
#define W_REL_1(i)          (REL_DATA(i) WR1)
#define W_REL_2(i)          (REL_DATA(i) WR2)
#define D_REL_1(i)          (REL_DATA(i) DR1)
#define D_REL_2(i)          (REL_DATA(i) DR2)
#define ZOOM_GR(i)          (REL_DATA(i) i11)
#define ROT_DEGREE(i)       (REL_DATA(i) FRD)
#define CAP_HT(i)           (REL_DATA(i) CH)
#define CROP_FLAG(i)        (REL_DATA(i) i14)
#define FG_COLOR(i)         (REL_DATA(i) i16)
#define BG_COLOR(i)         (REL_DATA(i) i17)
#define OUT_COLOR(i)		(REL_DATA(i) i18)
#define FRAME_FLAGS(i)		(REL_DATA(i) i20)
#define FG_SHADE(i)	    	(REL_DATA(i) i21)
#define BG_SHADE(i)		    (REL_DATA(i) i22)
#define OUT_SHADE(i)		(REL_DATA(i) i23)
#define ZOOM_GRY(i)         (REL_DATA(i) i24)
#define BLEND_START(i)		(REL_DATA(i) i25)
#define BLEND_END(i)		(REL_DATA(i) i26)
#define BLEND_ANGLE(i)		(REL_DATA(i) i27)
#define BLEND_SSHADE(i)		(REL_DATA(i) i30)
#define BLEND_ESHADE(i)		(REL_DATA(i) i31)


#define POINT_SIZE(i)		(REL_DATA(i) PS)
#define LEADING(i)			(REL_DATA(i) LDG)
#define X_OFFSET(i)         (REL_DATA(i) XO)
#define Y_OFFSET(i)         (REL_DATA(i) YO)
#define WIDTH_OFFSET(i)     (REL_DATA(i) WO)
#define DEPTH_OFFSET(i)     (REL_DATA(i) DO)
#define TOP_GUTTER(i)		(REL_DATA(i) TG)
#define BOT_GUTTER(i)		(REL_DATA(i) BG)
#define LEFT_GUTTER(i)		(REL_DATA(i) LG)
#define RIGHT_GUTTER(i)		(REL_DATA(i) RG)
#define VJ_Y_OFFSET(i)      (REL_DATA(i) d14)
#define CROP_TOP(i)         (REL_DATA(i) d14)
#define SET_SIZE(i)			(REL_DATA(i) SS)
#define CROP_BOTTOM(i)      (REL_DATA(i) d15)
#define CROP_LEFT(i)        (REL_DATA(i) d16)
#define OUT_WEIGHT(i)		(REL_DATA(i) d18)
#define OUT_TRAP(i)         (REL_DATA(i) d19)
#define CROP_RIGHT(i)       (REL_DATA(i) d20)
#define TL_ROUNDED(i)		(REL_DATA(i) TL_ROUND)
#define TR_ROUNDED(i)		(REL_DATA(i) d22)
#define BL_ROUNDED(i)		(REL_DATA(i) d23)
#define BR_ROUNDED(i)		(REL_DATA(i) d24)

#define STYLE(i)			(REL_DATA(i) STYL)
#define BRK_RULES(i)		(REL_DATA(i) B_RULES)
#define VERT_TABLE(i)		(REL_DATA(i) V_TABLE)

/*
 * struct RELATIVES locations used by Spec frame (0) only:
 */
#define DEF_FLAGS			(wn -> global_data.default_flags)
#define DEF_TOP_GUTTER		(wn -> preferences.gutter_top)
#define DEF_BOT_GUTTER		(wn -> preferences.gutter_bottom)
#define DEF_LEFT_GUTTER		(wn -> preferences.gutter_left)
#define DEF_RIGHT_GUTTER	(wn -> preferences.gutter_right)
#define ORIG_FRM_TYPE       (REL_DATA(0) c1)
#define BRK_RULE            (REL_DATA(0) c3)

#define MP_DESIGN			(REL_DATA(0) p0)

#define NUMB_COLS           (REL_DATA(0) i1)
#define TOT_BLOCKS          (REL_DATA(0) i2)
#define NEW_TOT_BLOCKS      (REL_DATA(0) i3)
#define TRIM_FLAGS          (REL_DATA(0) i5)
#define TRIM_WEIGHT			(REL_DATA(0) i6)
#define TRIM_LENGTH			(REL_DATA(0) i7)
#define NUMB_ROWS           (REL_DATA(0) i8)
#define HI_ID               (REL_DATA(0) H_ID)

#define TRIM_WIDTH			(REL_DATA(0) T_WIDTH)
#define TRIM_DEPTH			(REL_DATA(0) d3)
#define PAGE_WIDTH			(REL_DATA(0) d4)
#define PAGE_DEPTH			(REL_DATA(0) d5)
#define LEFT_MARGIN			(REL_DATA(0) d10)
#define RIGHT_MARGIN		(REL_DATA(0) d11)
#define FRM_WIDTH           (REL_DATA(0) d12)
#define FRM_DEPTH           (REL_DATA(0) d13)
#define GUT_WIDTH           (REL_DATA(0) d14)
#define GUT_DEPTH           (REL_DATA(0) d16)
#define X_PG_ORIGIN			(REL_DATA(0) d17)
#define Y_PG_ORIGIN			(REL_DATA(0) d18)
#define TOP_MARGIN			(REL_DATA(0) d19)
#define BOT_MARGIN			(REL_DATA(0) d20)

/*
 * struct RELATIVES locations used by text and flow frames only:
 */
#define BREAKING_RULES(i)	(REL_DATA(i) c3)
#define MP_FLAGS(i)			(REL_DATA(i) c7)
#define MP_SUBTYPE(i)		(REL_DATA(i) c8)

#define TEXT_ID(i)			(REL_DATA(i) T_ID)
#define REL_FRAME(i)		(REL_DATA(i) i9)
#define STF_POINT_SIZE(i)	(REL_DATA(i) STF_PS)
#define STF_SET_SIZE(i)		(REL_DATA(i) STF_SS)
#define STF_LEADING(i)		(REL_DATA(i) STF_LD)
#define LINK_NUMBER(i)		(REL_DATA(i) LK_NUM)
#define ARTICLE_ID(i)		(REL_DATA(i) ART_ID)

#define PREV_LAY(i)			(REL_DATA(i) P_LAY)
#define STF_IDS(i)			(REL_DATA(i) d11)
#define LOCK_LEAD(i)		(REL_DATA(i) d12)
#define PREV_TEXT_ID(i)		(REL_DATA(i) P_T_ID)
#define NEXT_LAY(i)			(REL_DATA(i) N_LAY)
#define NEXT_TEXT_ID(i)		(REL_DATA(i) N_T_ID)

/*
 * struct RELATIVES locations used by text frame only:
 */
#define HNJ_TEXT(i)         (REL_DATA(i) HNJ_T)

/*
 * struct RELATIVES locations used by flow frame only:
 */
#define OBJ_REF(i)			(REL_DATA(i) d25)

/*
 * struct RELATIVES locations used by graphic frame only:
 */
#define WRAP_GRAPHIC(i)		(REL_DATA(i) c2)

#define GRAPHIC_NAME(i)		(REL_DATA(i) G_NAME)

#define GR_TOP_GUTTER(i)	(REL_DATA(i) GR_TG)
#define GR_BOT_GUTTER(i)	(REL_DATA(i) GR_BG)
#define GR_LEFT_GUTTER(i)	(REL_DATA(i) GR_LG)
#define GR_RIGHT_GUTTER(i)	(REL_DATA(i) GR_RG)

/*
 * struct RELATIVES locations used by rule/box frame only:
 */
#define IS_R_OR_B(i)		(REL_DATA(i) c4)

#define R_B_WEIGHT(i)		(REL_DATA(i) WT)
#define DASHWID(i)			(REL_DATA(i) d26)
#define GAP_WID(i)			(REL_DATA(i) d27)
#define ODASHWD(i)			(REL_DATA(i) d28)
#define OGAPWID(i)			(REL_DATA(i) d29)


#define DISP_OPTS           (wn -> global_data.display_options)
#define TEXT_DISP           (wn -> global_data.text_display)
#define GRAPH_DISP          (wn -> global_data.graphic_display)
#define ZOOM_OPT            (wn -> global_data.zoom_option)
#define ZOOM_VAL            (wn -> global_data.zoom_value)
#define SNAP_FROM           (wn -> global_data.snap_from)
#define SNAP_TO_H           (wn -> global_data.snap_to_h)
#define SNAP_TO_V           (wn -> global_data.snap_to_v)
#define H_RUL_ORG           (wn -> global_data.horz_ruler_origin)
#define V_RUL_ORG           (wn -> global_data.vert_ruler_origin)
#define H_RUL_UNIT          (wn -> global_data.horz_ruler_units)
#define V_RUL_UNIT          (wn -> global_data.vert_ruler_units)

#define BLEND_ANGLE_L -1
#define BLEND_ANGLE_R -2
#define BLEND_ANGLE_T -3

#endif
