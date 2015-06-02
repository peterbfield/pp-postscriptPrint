/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    **
     *****     **    **    **   **    **   **    *******    *******   */

#include <stdio.h>
#include "X11_rounded.h"
#include "rel_data.h"
#include "interfer.h"
#include "px_header.h"
#include "frame.h"
#include "lmt.f"

extern int shape_bearoff(WYSIWYG *wn, int num, hmu msb, vmu ldb);
extern char *load_graphic_file(char *tree_name, char *graphic_name,
							   int *file_size, char *dir_name);

/*	lmt_off_to_abs is called with a type value (0 for vert.,
	1 for horiz.) and the offset value that is to be converted to
	machine units. */

uint32 lmt_abs_to_off(WYSIWYG *wn, int type, uint32 units, int32 machine_unit)
{
	uint32 abs;
	int sign = 0;
	
	if (machine_unit < 0)
	{
		machine_unit *= -1;
		sign = 1;
	}
	switch (units >> PL_REL_SHIFT)
	{
	  case 0:
		abs = machine_unit;
		break;
	  case LINE_LEAD:			/* frame line leading */
		abs = (LAYOUT(wn) block_leading ?
			   (machine_unit + (LAYOUT(wn) block_leading>>1))
			   / LAYOUT(wn) block_leading:
			   (LAYOUT(wn) spec_leading ?
				(machine_unit + (LAYOUT(wn) spec_leading>>1))
				/ LAYOUT(wn) spec_leading:
				0));
		break;
	  case ORG_LINE_LEAD:		/* spec line leading */
		abs = (LAYOUT(wn) spec_leading ?
			   (machine_unit + (LAYOUT(wn) spec_leading>>1))
			   / LAYOUT(wn) spec_leading:
			   0);
		break;
	  default:
		abs = machine_unit * wn -> yx_convert[type];
		break;
	}
	abs |= units;
	if (sign)
		abs |= PL_SIGNBIT;
	return(abs);
}

int32 lmt_frame_off_to_abs(WYSIWYG *wn, int frame, int type, uint32 value)
{
	if (TYPE_OF_FRAME(frame) == PL_TEXT || TYPE_OF_FRAME(frame) == PL_FLOW ||
		TYPE_OF_FRAME(frame) == PL_MISC)
		LAYOUT(wn) block_leading = lmt_off_to_abs(wn, 0, LEADING(frame));
	else
		LAYOUT(wn) block_leading = LAYOUT(wn) spec_leading;
	return(lmt_off_to_abs(wn, type, value));
}

int32 lmt_off_to_abs(WYSIWYG *wn, int type, uint32 value)
{
	int32 abs;
	int16 units;
	uint32 sign;
	
	abs = value & PL_SYNBITS;
	sign = value & PL_SIGNBIT;
	units = (int16)((value & PL_REL_BITS) >> PL_REL_SHIFT);
	if (units == LINE_LEAD)		/* lines */
		abs *= LAYOUT(wn) block_leading;
	else if (units == ORG_LINE_LEAD)
		abs *= LAYOUT(wn) spec_leading;
	else if (units)				/* 0 if already in machine units */
	{
		if (wn->yx_convert[type])
			abs = (abs + (wn -> yx_convert[type] >> 1))
				/ wn -> yx_convert[type];
		else
			p_info(PI_ELOG, "ERROR: lmt_off_to_abs() yx_convert[%d] = 0\n",
					  type);
	}
	if (sign)
		abs *= -1;
	return(abs);
}

int32 lmt_lock_lead(WYSIWYG *wn, int i, int32 Ti)
{
	int16 j, k;
	int32 Li, Lj, TLi, TLj, a, b, c = 0L;
	
	k = (int16)((LOCK_LEAD(i) & 0xC0000000) >> 30);
	if (!k || (TYPE_OF_FRAME(i) != PL_TEXT
			   && TYPE_OF_FRAME(i) != PL_GRAPHIC
			   && TYPE_OF_FRAME(i) != PL_MISC
			   && TYPE_OF_FRAME(i) == PL_FLOW
			   && TYPE_OF_FRAME(i) != PL_RBX))
		return(0L);
	j = (int16)(LOCK_LEAD(i) & 0xFFFF);
	Lj = lmt_off_to_abs(wn, 0, LEADING(j));
	TLj = ELE_DATA(j) rot_rect_top + Lj;
	Li = lmt_off_to_abs(wn, 0, LEADING(i));
	TLi = Ti + Li;
	switch (k)
	{
	  case 1:					/* Lock lead up */
		if (TLj < TLi)
		{
			a = TLi - ELE_DATA(j) rot_rect_top;
			b = (a / Lj) * Lj;
			c = -(a - b);
		}
		else
		{
			a = TLj - Ti;
			b = (a / Li) * Li;
			if (b != a)
				b += Li;
			c = -(b - a);
		}
		break;
	  case 2:					/* Lock lead down */
		if (TLj < TLi)
		{
			a = TLi - ELE_DATA(j) rot_rect_top;
			b = (a / Lj) * Lj;
			if (b != a)
				b += Lj;
			c = b - a;
		}
		else
		{
			a = TLj - Ti;
			b = (a / Li) * Li;
			c = a - b;
		}
		break;
	  case 3:					/* Closest lock lead */
		if (TLj < TLi)
		{
			a = TLi - ELE_DATA(j) rot_rect_top;
			b = (a / Lj) * Lj;
			if (a - b < Lj >> 1)
			{		/* Up */
				c = -(a - b);
			}
			else
			{		/* Down */
				if (b != a)
					b += Lj;
				c = b - a;
			}
		}
		else
		{
			a = TLj - Ti;
			b = (a / Li) * Li;
			if (a - b < Li >> 1)
			{		/* Down */
				c = a - b;
			}
			else
			{		/* Up */
				if (b != a)
					b += Li;
				c = -(b - a);
			}
		}
		break;
	}
	return(c);
}

void lmt_BuildRectangle(DRAW_POINT_X_Y *list_pt, ELEMENT *ele,
						int32 top, int32 bottom, int32 left, int32 right)
{
	ele -> rot_rect_left = list_pt -> x = left;
	ele -> rot_rect_top = list_pt -> y = top;
	list_pt++;
	list_pt -> x = right;
	list_pt -> y = top;
	list_pt++;
	ele -> rot_rect_right = list_pt -> x = right;
	ele -> rot_rect_bottom = list_pt -> y = bottom;
	list_pt++;
	list_pt -> x = left;
	list_pt -> y = bottom;
	ele -> n_points = 4;
}

void lmt_IsARule(WYSIWYG *wn, int i, int16 *type_of_rule,
				 int32 *h_weight, int32 *v_weight)
{
	int32 weight;
	int16 width, height, sv;
	
	sv = wn -> yx_convert[0];
	wn -> yx_convert[0] = 270;	/* 20ths of a point */
	weight = lmt_off_to_abs(wn, 0, R_B_WEIGHT(i));
	wn -> yx_convert[0] = sv;
	width = ELE_DATA(i) rot_rect_right - ELE_DATA(i) rot_rect_left;
	height = ELE_DATA(i) rot_rect_bottom - ELE_DATA(i) rot_rect_top;
	*v_weight = ((weight * wn -> ldb) + 10L) / 20L;
	*h_weight = ((weight * wn -> msb) + 10L) / 20L;
	if (!width)
	{
		*type_of_rule = 1;		/* vertical rule */
		switch (LOCKPOINT(i))	/* the lock point */
		{
		  case TL:				/* 'x' = left */
		  case BL:
		  case CL:
		  case HL:
			ELE_DATA(i) rot_rect_right += *h_weight;
			break;
		  case TR:				/* 'x' = right */
		  case BR:
		  case CR:
		  case HR:
			ELE_DATA(i) rot_rect_left -= *h_weight;
			*type_of_rule |= 0x10; /* Draw from right */
			break;
		  case TC:				/* 'x' = center */
		  case BC:
		  case CC:
		  case HC:
			ELE_DATA(i) rot_rect_left -= *h_weight >> 1;
			ELE_DATA(i) rot_rect_right += *h_weight - (*h_weight >> 1);
			break;
		  default:
			break;
		}
		ELE_DATA(i) rot_rect_x_center =	/* x-center */
			(ELE_DATA(i) rot_rect_left + ELE_DATA(i) rot_rect_right + 1) / 2;
	}
	else if (!height)
	{
		*type_of_rule = 2;		/* horizontal rule */
		switch (LOCKPOINT(i))	/* the lock point */
		{
		  case TL:				/* 'y' = top */
		  case TR:
		  case TC:
		  case HL:
		  case HR:
		  case HC:
			ELE_DATA(i) rot_rect_bottom += *v_weight;
			*type_of_rule |= 0x10; /* Draw from top */
			break;
		  case BL:				/* 'y' = bottom */
		  case BR:
		  case BC:
			ELE_DATA(i) rot_rect_top -= *v_weight;
			break;
		  case CL:				/* 'y' = center */
		  case CR:
		  case CC:
			ELE_DATA(i) rot_rect_top -= *v_weight >> 1;
			ELE_DATA(i) rot_rect_bottom += *v_weight - (*v_weight >> 1);
			break;
		  default:
			break;
		}
		ELE_DATA(i) rot_rect_y_center =	/* y-center */
			(ELE_DATA(i) rot_rect_top + ELE_DATA(i) rot_rect_bottom + 1) / 2;
	}
	else if (!weight)
		*type_of_rule = 4;		/* solid box */
	else
		*type_of_rule = 3;		/* open box */
	/*	  RB_WIDTH(i, 0)
		  = ELE_DATA(i) rot_rect_right - ELE_DATA(i) rot_rect_left;
		  RB_HEIGHT(i, 0)
		  = ELE_DATA(i) rot_rect_bottom - ELE_DATA(i) rot_rect_top;
		  RB_LEFT(i, 0) = ELE_DATA(i) rot_rect_left;
		  RB_BOT(i, 0) = ELE_DATA(i) rot_rect_bottom;
	  */
}

int lmt_IsTwoDiffEle(ELEMENT *ele_1, ELEMENT *ele_2)
{
	DRAW_POINT_X_Y *lp_1, *lp_2;
	PPOINT x_offset, y_offset;
	/* int found_match_x, found_match_y, i, j; */
	
	if (!(ele_1 -> list_points && ele_2 -> list_points
		  && ele_1 -> n_points == ele_2 -> n_points))
		return(YES);
	lp_1 = ele_1 -> list_points;
	lp_2 = ele_2 -> list_points;
	x_offset = ele_2 -> rect_left - ele_1 -> rect_left;
	y_offset = ele_2 -> rect_top - ele_1 -> rect_top;
	/* DSV - commented out compare below to speed up DM */ 
	/* for (i = 0; i < ele_1 -> n_points; i++)
	{
		found_match_x = found_match_y = FALSE;
		lp_2 = ele_2 -> list_points;
		for (j = 0; j < ele_2 -> n_points; j++)
		{
			if (lp_1 -> x + x_offset == lp_2 -> x)
				found_match_x = TRUE;
			if (lp_1 -> y + y_offset == lp_2 -> y)
				found_match_y = TRUE;
			if (found_match_x && found_match_y)
				break;
			lp_2++;
		}
		if (!found_match_x || !found_match_y)
			return(YES);
		lp_1++;
	} */
	return(NO);
}

void set_tf_process_flag(WYSIWYG *wn, int text_article_id, int command_flag)
{
	FILE_LIST *fl;

	if (!text_article_id)
		return;
	fl = (FILE_LIST *)wn -> unit_win  -> files.head;
	while (fl)
	{
		if (fl -> type == FL_member && fl -> misc == text_article_id)
		{
			fl -> flags |= command_flag;
			return;
		}
		fl = (FILE_LIST *)fl -> que.next;
	}
}

void copy_bearoff_pts(WYSIWYG *wn, int num)
{
	int i;
	
	for(i = 0; i < FRAME_DATA(num) num_bearoffs; i++)
	{
		ELE_DATA(num) list_points[i].x =
			FRAME_DATA(num) bearoff_pts[i].x + FRAME_DATA(num) left;
		ELE_DATA(num) list_points[i].y =
			FRAME_DATA(num) bearoff_pts[i].y + FRAME_DATA(num) top;
	}
}

void copy_shape_pts(WYSIWYG *wn, int num, hmu msb, vmu ldb)
{
	int i;
	
	for (i = 0; i < FRAME_DATA(num) num_shapes; i++)
	{
		ELE_DATA(num) list_points[i].x =
			(FRAME_DATA(num) shape_pts[i].x * msb) + FRAME_DATA(num) left;
		ELE_DATA(num) list_points[i].y =
			(FRAME_DATA(num) shape_pts[i].y * ldb) + FRAME_DATA(num) top;
	}
}

BOOLEAN read_px_graphic_in(WYSIWYG *wn, int num)
{
	int size, i;
	PXL_SHAPE *shape;
	PXL_HDR *pxl;
	PPOINT prev_x, prev_y;
	PXL_HDR *load_graphics_file();
	int dx,dy;		/* top/left movement from graphic to pixel file */

	if (FRAME_DATA(num) shape_pts)
	{
		if (shape_bearoff(wn, num, wn -> msb, wn -> ldb))
			for (i = 0; i < FRAME_DATA(num) num_bearoffs; i++)
			{
				FRAME_DATA(num) bearoff_pts[i].x *= wn -> msb;
				FRAME_DATA(num) bearoff_pts[i].y *= wn -> ldb;
			}
		return(TRUE);
	}
	FRAME_DATA(num) num_shapes = 0;
	
	pxl =
		(PXL_HDR *)load_graphic_file(wn -> tree_name,GRAPHIC_NAME(num), &size,
									 wn -> dir_name);
	if(pxl == 0)
		return(FALSE);
	if(pxl -> first_shape_rec <= 0)
		return(FALSE);
	shape = (PXL_SHAPE *)((char *)pxl + (pxl -> first_shape_rec * 512));
	FRAME_DATA(num) shape_pts =
		(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
								  (shape->num_pts + 1));
	/* distance from top/left of graphic to top/left of pixel file */
	dx = pxl->trim_left - (pxl->crop_left * pxl->horiz_dpi / 1440);
	dy = pxl->trim_top  - (pxl->crop_top * pxl->vert_dpi / 1440);
	prev_x = shape -> pts[shape -> num_pts - 1].x + dx;
	prev_y = shape -> pts[shape -> num_pts - 1].y + dy;
	for(i = 0; i < shape -> num_pts; i++)
	{
		/* Get rid of duplicate */
		if (prev_x == (shape -> pts[i].x + dx) &&  
			prev_y == (shape -> pts[i].y + dy))
			continue;
		prev_x = shape -> pts[i].x + dx;
		prev_y = shape -> pts[i].y + dy;
		FRAME_DATA(num) shape_pts[FRAME_DATA(num) num_shapes].x
			= ((shape -> pts[i].x + dx) * 72 +
			   (pxl -> horiz_dpi >> 1)) / pxl -> horiz_dpi;
		FRAME_DATA(num) shape_pts[FRAME_DATA(num) num_shapes].y
			= ((shape -> pts[i].y + dy) * 72 +
			   (pxl -> vert_dpi >> 1)) / pxl -> vert_dpi;
		FRAME_DATA(num) num_shapes++;
	}
	FRAME_DATA(num) bearoff_pts = 0;
	FRAME_DATA(num) num_bearoffs = 0;
	if (shape_bearoff(wn, num, wn -> msb, wn -> ldb))
		for (i = 0; i < FRAME_DATA(num) num_bearoffs; i++)
		{
			FRAME_DATA(num) bearoff_pts[i].x *= wn -> msb;
			FRAME_DATA(num) bearoff_pts[i].y *= wn -> ldb;
		}
	p_free((char *)pxl);
	return(TRUE);
}

void build_rounded_corner_frame(WYSIWYG *wn, int frame, int32 x_rounded,
								XSEGMENT *pSeg, XARC *pArc)
{
	/* ASSUMPTION: This is an overlay not rotated */
	int y_rounded = (20 * lmt_off_to_abs(wn, Y_REF, TL_ROUNDED(frame)))
		/ wn -> msb;
	int rounded = (x_rounded * 20) / wn -> ldb;
	
	pSeg -> x1 = ELE_DATA(frame) rot_rect_left
		+ LAYOUT(wn) x_move + x_rounded;
	pSeg -> x2 = ELE_DATA(frame) rot_rect_right
		+ LAYOUT(wn) x_move - x_rounded;
	pSeg -> y1 = pSeg -> y2 =
		ELE_DATA(frame) rot_rect_top + LAYOUT(wn) y_move;
	(pSeg + 2) -> x1 = ELE_DATA(frame) rot_rect_left
		+ LAYOUT(wn) x_move + x_rounded;
	(pSeg + 2) -> x2 = ELE_DATA(frame) rot_rect_right
		+ LAYOUT(wn) x_move - x_rounded;
	(pSeg + 2) -> y1 = (pSeg + 2) -> y2 =
		ELE_DATA(frame) rot_rect_bottom + LAYOUT(wn) y_move;
	(pSeg + 1) -> y1 = ELE_DATA(frame) rot_rect_top
		+ LAYOUT(wn) y_move + y_rounded;
	(pSeg + 1) -> y2 = ELE_DATA(frame) rot_rect_bottom
		+ LAYOUT(wn) y_move - y_rounded;
	(pSeg + 1) -> x1 = (pSeg + 1) -> x2 =
		ELE_DATA(frame) rot_rect_left + LAYOUT(wn) x_move;
	(pSeg + 3) -> y1 = ELE_DATA(frame) rot_rect_top
		+ LAYOUT(wn) y_move + y_rounded;
	(pSeg + 3) -> y2 = ELE_DATA(frame) rot_rect_bottom
		+ LAYOUT(wn) y_move - y_rounded;
	(pSeg + 3) -> x1 = (pSeg + 3) -> x2 =
		ELE_DATA(frame) rot_rect_right + LAYOUT(wn) x_move;
	pArc -> x = (pSeg + 1) -> x1;
	pArc -> y = pSeg -> y1;
	pArc -> width = pArc -> height = rounded;
	pArc -> angle1 = 90 * 64;
	pArc -> angle2 = 90 * 64;
	(pArc + 1) -> x = ELE_DATA(frame) rot_rect_right
		+ LAYOUT(wn) x_move - rounded;
	(pArc + 1) -> y = pSeg -> y1;
	(pArc + 1) -> width = (pArc + 1) -> height = rounded;
	(pArc + 1) -> angle1 = 0;
	(pArc + 1) -> angle2 = 90 * 64;
	(pArc + 2) -> x = (pArc + 1) -> x;
	(pArc + 2) -> y = ELE_DATA(frame) rot_rect_bottom
		+ LAYOUT(wn) y_move - (y_rounded << 1);
	(pArc + 2) -> width = (pArc + 2) -> height = rounded;
	(pArc + 2) -> angle1 = 270 * 64;
	(pArc + 2) -> angle2 = 90 * 64;
	(pArc + 3) -> x = (pSeg + 1) -> x1;
	(pArc + 3) -> y = (pArc + 2) -> y;
	(pArc + 3) -> width = (pArc + 3) -> height = rounded;
	(pArc + 3) -> angle1 = 180 * 64;
	(pArc + 3) -> angle2 = 90 * 64;
}
