/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    **
     *****     **    **    **   **    **   **    *******    *******   */

	/* COMMENTS:
	User programs can access this code by calling
	'error = lmt_interface()'. Such programs must contain the structures
	'rel_data' and the array 'yx_convert' as declared
	below. Prior to calling 'lmt_interface', the relative data must be
	read into 'rel_data', move TOT_BLOCKS into NEW_TOT_BLOCKS
	(and add any new blocks to this number), and set up yx_convert.
	yx_convert must contain whatever divisors will convert 5400ths of a
	point to the machine units you want. If the data in 'rel_data' is
	already in machine units, set yx_convert to 0. yx_convert[0] is for
	vertical units and [1] is for horizontals.

	Database description:

   The offset words are 32-bit signed values with four type bits and
   their locations are documented in pg_layout.h. The most significant
   bit is the sign bit, the next four bits are type bits, and the
   remaining 27 bits are the value itself.

   Type bits are as follows:
   1 = picas/points			carried in points
   2 = inches/32ths			carried in 32ths
   3 = cm/mm				carried in millimeters
   4 = lines
   5 = pts/halves			carried in half points
   6 = pts/quarters			carried in quarter points
   7 = pts/tenths			carried in tenths of points

   (The following type values are octal)

   15 = picas/pts/halves	carried in half points
   16 = picas/pts/quarters	carried in quarter points
   17 = picas/pts/tenths	carried in tenths of points

   The relationship words are 16 bit values and if two relationships
   define a value,	that value is the mean of the two. The top nibble of
   the top byte is the relationship itself, the bottom nibble is the
   kind of thing that was referred to, and the low byte is the block
   number if any.

   Top nibble of top byte:
   x relationship			y relationship
   1 = left					1 = top
   2 = right				2 = bottom
   3 = center				3 = center
   4 = same as				4 = sane as
   5 = first line
   6 = cap height
   Bottom nibble of top byte:
   x relationship			y relationship
   0 = trim					0 = trim
   1 = page					1 = page
   2 = block				2 = block
   3 = gutter				3 = gutter
   */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rel_data.h"
#include "interfer.h"
#include "frame.h"
#include "traces.h"
#include "lmt.f"

/* Must match the one in pwx_lib.c */
#define TB_LR(a, b)	(a -> rot_rect_top >= b -> rot_rect_bottom		 \
					 || a -> rot_rect_bottom <= b -> rot_rect_top \
					 || a -> rot_rect_left >= b -> rot_rect_right \
					 || a -> rot_rect_right <= b -> rot_rect_left)

extern void p_draw_arc(int32 left, int32 top,
					   int32 right, int32 bottom,
					   DRAW_POINT_X_Y **ret_pts, int16 *ret_num,int line_width,
					   int a1, int a2);
extern void clean_draw_point_x_y(DRAW_POINT_X_Y *ele_list, int num_pts);
extern void set_layout_process_flag(LAYOUT_DESC *layout);
extern void set_tf_process_flag(WYSIWYG *wn, int text_article_id,
								int command_flag);
extern void clean_ele(ELEMENT *ele);
extern ELEMENT *merge_neg_ele(ELEMENT *old_ele, ELEMENT *neg_ele);
extern void copy_bearoff_pts(WYSIWYG *wn, int num);
extern int shape_bearoff(WYSIWYG *wn, int num, hmu msb, vmu ldb);
extern int trap_bearoff(WYSIWYG *wn, int num,
						hmu msb, vmu ldb, ELEMENT *ele);
extern BOOLEAN read_px_graphic_in(WYSIWYG *wn, int num);
extern void copy_shape_pts(WYSIWYG *wn, int num, hmu msb, vmu ldb);
extern void clean_items_que(QUEUE *items);
/* The very first time we open a layout,
   we do not want to set the reprocessing
   flag of the text files and layout */
int not_first_time = FALSE;
static BOOLEAN ovale_flag = FALSE;

static void create_interference(WYSIWYG *wn, ELEMENT *i_ele,
								ELEMENT *j_ele, int i, int j);
static void create_inter_frame(WYSIWYG *, int);
static void interference(WYSIWYG *, int, int16 *, int16 *,
						 int32 *, int32 *, int32 *, int);
static void get_box_around_bearoff_shape(WYSIWYG *wn, int frame_num);
static void adjust_data(WYSIWYG *wn, int i,
						ELEMENT **elements, ELEMENT **neg_elements);
void do_crop(WYSIWYG *wn, int i, int32 start_x, int32 start_y,
				   int32 end_x, int32 end_y);
#ifdef TRACE
static void trace_end_interface(WYSIWYG *wn);
static void print_pts(char *str, DRAW_POINT_X_Y *pts, int n_points);
static void trace_begin_create_interference(
											ELEMENT *i_ele, ELEMENT *j_ele,
											int i, int j);
static void trace_end_create_interference(WYSIWYG *wn, int j);
#endif

static void create_interference(WYSIWYG *wn, ELEMENT *new_ele,
								ELEMENT *old_ele, int new, int old)
{
	if (!TB_LR(old_ele, new_ele))
	{
#ifdef TRACE
		trace_begin_create_interference(new_ele, old_ele, new, old);
#endif
		lmt_BuildElements(&FRAME_DATA(old) ele,
						  &FRAME_DATA(old) neg_ele, old_ele, new_ele, 1);
#ifdef TRACE
		trace_end_create_interference(wn, old);
#endif
	}
}

static void create_inter_frame(WYSIWYG *wn, int i)
{
	ELEMENT *ele;
	int top, bottom, left, right;
	
	for (ele = FRAME_DATA(i) ele; ele; ele = ele -> next)
	{
		left = MIN(ele -> rot_rect_left, ele -> rot_rect_right);
		right = MAX(ele -> rot_rect_left, ele -> rot_rect_right);
		top = MIN(ele -> rot_rect_top, ele -> rot_rect_bottom);
		bottom = MAX(ele -> rot_rect_top, ele -> rot_rect_bottom);
		if (ele == FRAME_DATA(i) ele)
		{
			FRAME_DATA(i) inter_left = left;
			FRAME_DATA(i) inter_right= right;
			FRAME_DATA(i) inter_top = top;
			FRAME_DATA(i) inter_bottom = bottom;
		}
		else
		{
	    	if (left < FRAME_DATA(i) inter_left)
       			FRAME_DATA(i) inter_left = left;
    		if (right > FRAME_DATA(i) inter_right)
       			FRAME_DATA(i) inter_right = right;
    		if (top < FRAME_DATA(i) inter_top)
       			FRAME_DATA(i) inter_top = top;
    		if (bottom > FRAME_DATA(i) inter_bottom)
       			FRAME_DATA(i) inter_bottom = bottom;
		}
	}
}

static void process_element(WYSIWYG *wn, int new)
{
	ELEMENT *new_ele, *old_ele, *old_ele_next, *save_new_ele;
	ELEMENT *keep_save_new_ele = NULL, *list_save_new_ele = NULL;
	int old;
	
	/* Rotate all the elements in the frame
	   we are comparing to the old ones with
	   a lower number */
	for (new_ele = FRAME_DATA(new) ele;
		 new_ele; new_ele = new_ele -> next)
	{
		save_new_ele = lmt_set_rot_ele(wn, new, new, new_ele);
		if (!list_save_new_ele)
			keep_save_new_ele = list_save_new_ele = save_new_ele;
		else
		{
			keep_save_new_ele -> next = save_new_ele;
			save_new_ele -> prev = keep_save_new_ele;
			keep_save_new_ele = save_new_ele;
		}
	}
	for (save_new_ele = list_save_new_ele; save_new_ele;
		 save_new_ele = save_new_ele -> next)
	{
		for (old = 1; old < new; old++)	/* previous blocks around this one */
		{
			if (TYPE_OF_FRAME(old) == PL_INCL)
				continue;		/* not include blocks */
			if (FRAME_FLAGS(old) & OVERLAY)
				continue;		/* Old block is overlay */
			/* Old element is rotated, apply it to new frame for accuracy */
	 		new_ele = lmt_set_rot_ele(wn, new, old, save_new_ele);
			/* Setup rectangle around new element */
			create_inter_frame(wn, new);
			/* Interfer all the old elements with
			   new element */
#ifdef LINT
			old_ele_next = NULL;
#endif
			for (old_ele = FRAME_DATA(old) ele; old_ele; old_ele =old_ele_next)
			{
				old_ele_next = old_ele -> next;
				create_interference(wn, new_ele, old_ele, new, old);
			}
			if (ovale_flag || (FRAME_FLAGS(old) & OVALE_SHAPE))
			{
				DRAW_POINT_X_Y *start_arc = NULL;
				
				for (old_ele = FRAME_DATA(old) ele; 
					 old_ele; old_ele = old_ele -> next)
				{
					DRAW_POINT_X_Y *pt = old_ele -> list_points;
					int i;
					
					for (i = 0, pt = old_ele -> list_points; 
						 i < old_ele -> n_points; pt++, i++)
					{
						pt -> off_x = -1;
						if (pt -> arc)
						{
							if (!start_arc)
							{
								start_arc = pt;
								start_arc -> off_x = i;
								start_arc -> draw_type = D_ARC;
							}
							else
								start_arc -> off_x = i;
						}
						else
							start_arc = NULL;
					}
					if (start_arc)
					{			/* Try to loop the arc */
						if (old_ele -> list_points -> off_x != -1)
						{
							start_arc -> off_x = 0;
							start_arc = NULL;
						}
					}
				}
			}
			if (FRAME_DATA(old) neg_ele)
			{
				if (FRAME_DATA(old) neg_ele -> next)
					FRAME_DATA(old) neg_ele =
						lmt_MergeElements( FRAME_DATA(old) neg_ele);
				for (old_ele = FRAME_DATA(old) ele; 
					 old_ele; old_ele = old_ele_next)
				{
					ELEMENT *neg_ele, *sv_next_neg_ele;
					
					old_ele_next = old_ele -> next;
#ifdef LINT
					sv_next_neg_ele = NULL;
#endif
					for (neg_ele = FRAME_DATA(old) neg_ele;
						 neg_ele; neg_ele = sv_next_neg_ele)
					{
						sv_next_neg_ele = neg_ele -> next;
						if (!(TB_LR(old_ele, neg_ele)))
						{
							ELEMENT *merge_ele;
							
							if ((merge_ele =
								 merge_neg_ele(old_ele, neg_ele)) != NULL)
							{
								if (old_ele -> prev)
									old_ele -> prev -> next = merge_ele;
								else
									FRAME_DATA(old) ele = merge_ele;
								if (old_ele -> next)
									old_ele -> next -> prev = merge_ele;
								merge_ele -> next = old_ele -> next;
								merge_ele -> prev = old_ele -> prev;
								p_free((char *)old_ele -> list_points);
								p_free((char *)old_ele);
								if (neg_ele -> prev)
									neg_ele -> prev -> next
										= neg_ele -> next;
								else
									FRAME_DATA(old) neg_ele
										= neg_ele -> next;
								if (neg_ele -> next)
									neg_ele -> next -> prev
										= neg_ele -> prev;
								p_free((char *)neg_ele -> list_points);
								p_free((char *)neg_ele);
							}
						}
					}
				}
			}
			old_ele = FRAME_DATA(old) ele;
			if (old_ele)
			{					/* Rebuild rectangle around new
								   "old element" */
				FRAME_DATA(old) inter_top = old_ele -> rot_rect_top;
				FRAME_DATA(old) inter_bottom
					= old_ele -> rot_rect_bottom;
				FRAME_DATA(old) inter_left = old_ele -> rot_rect_left;
				FRAME_DATA(old) inter_right = old_ele -> rot_rect_right;
				for (old_ele = old_ele -> next;
					 old_ele; old_ele = old_ele -> next)
				{
					if (FRAME_DATA(old) inter_top
						> old_ele -> rot_rect_top)
						FRAME_DATA(old) inter_top
							= old_ele -> rot_rect_top;
                   	if (FRAME_DATA(old) inter_bottom
						< old_ele -> rot_rect_bottom)
                       	FRAME_DATA(old) inter_bottom
							= old_ele -> rot_rect_bottom;
                   	if (FRAME_DATA(old) inter_left
						> old_ele -> rot_rect_left)
                       	FRAME_DATA(old) inter_left
							= old_ele -> rot_rect_left;
                   	if (FRAME_DATA(old) inter_right
						< old_ele -> rot_rect_right)
                       	FRAME_DATA(old) inter_right
							= old_ele -> rot_rect_right;
				}
				FRAME_DATA(old) inter_x_center
					= (FRAME_DATA(old) inter_left
					   + FRAME_DATA(old) inter_right) >> 1;
				FRAME_DATA(old) inter_y_center
					= (FRAME_DATA(old) inter_top
					   + FRAME_DATA(old) inter_bottom) >> 1;
			}
			clean_ele(new_ele);
		}
	}
	/* Destroy all the rotated new elements */
	for (save_new_ele = list_save_new_ele; save_new_ele;
		 save_new_ele = keep_save_new_ele)
	{
		keep_save_new_ele = save_new_ele -> next;
		clean_ele(save_new_ele);
	}
}

static void get_box_around_bearoff_shape(WYSIWYG *wn, int frame_num)
{
	PPOINT xmin, xmax, ymin, ymax;
	int i;
	
	xmin = xmax = ELE_DATA(frame_num) list_points[0].x;
	ymin = ymax = ELE_DATA(frame_num) list_points[0].y;
	for (i = 1; i < ELE_DATA(frame_num) n_points; i++)
	{
		if (xmin > ELE_DATA(frame_num) list_points[i].x)
			xmin = ELE_DATA(frame_num) list_points[i].x;
		else if (xmax < ELE_DATA(frame_num) list_points[i].x)
			xmax = ELE_DATA(frame_num) list_points[i].x;
		if (ymin > ELE_DATA(frame_num) list_points[i].y)
			ymin = ELE_DATA(frame_num) list_points[i].y;
		else if (ymax < ELE_DATA(frame_num) list_points[i].y)
			ymax = ELE_DATA(frame_num) list_points[i].y;
		
	}
	FRAME_DATA(frame_num) inter_top
		= ELE_DATA(frame_num) rot_rect_top = ymin;
	FRAME_DATA(frame_num) inter_bottom
		= ELE_DATA(frame_num) rot_rect_bottom = ymax;
	FRAME_DATA(frame_num) inter_y_center = (ymin + ymax) << 1;
	FRAME_DATA(frame_num) inter_left
		= ELE_DATA(frame_num) rot_rect_left = xmin;
	FRAME_DATA(frame_num) inter_right
		= ELE_DATA(frame_num) rot_rect_right = xmax;
	FRAME_DATA(frame_num) inter_x_center = (xmin + xmax) << 1;
}

void do_crop(WYSIWYG *wn, int num, int32 start_x, int32 start_y,
				   int32 end_x, int32 end_y)
{
if((wn->toolbox_mode == TOOLBOX_CROP || wn->toolbox_mode == TOOLBOX_SCALE) &&
   ((TYPE_OF_FRAME(num)) == PL_GRAPHIC))
  { 
    int start, corr = ZOOM_GR(num) / 2, corry = ZOOM_GRY(num) / 2;
    start = (((FRAME_DATA(num)prev_right - FRAME_DATA(num)prev_left - 
	       (lmt_off_to_abs(wn,X_REF,CROP_LEFT(num))*ZOOM_GR(num) )/ZOOM_UNIT - end_x + start_x) *  ZOOM_UNIT + corr) /
	     ZOOM_GR(num));
    if(start <= 0) {
      CROP_FLAG(num) &= ~CROP_BIT_RIGHT;
      start = 0;
    }
    else
      CROP_FLAG(num) |= CROP_BIT_RIGHT;
    CROP_RIGHT(num) =
      lmt_abs_to_off(wn, X_REF, PICA << PL_REL_SHIFT,start);
    start = (((FRAME_DATA(num)prev_bottom - FRAME_DATA(num)prev_top - (lmt_off_to_abs(wn,Y_REF,CROP_TOP(num))*ZOOM_GRY(num))/ ZOOM_UNIT- end_y + start_y) * ZOOM_UNIT + corry) /
	     ZOOM_GRY(num));
    if(start <= 0) {
      CROP_FLAG(num) &= ~CROP_BIT_BOTTOM;
      start = 0;
    }
    else
      CROP_FLAG(num) |= CROP_BIT_BOTTOM;
    
    CROP_BOTTOM(num) =
      lmt_abs_to_off(wn, Y_REF,  PICA << PL_REL_SHIFT,start);
    FRAME_DATA(num) do_crop = 0;
  }
}

static void interference(WYSIWYG *wn, int i, int16 *start_vj, int16 *num_vj,
						 int32 *top_vj, int32 *depth_vj, int32 *frame_vj,
						 int do_vj)
{
	int j, k;
	int16 type_of_rule = 0;
	int32 h_weight, v_weight;
	int32 sv_top, sv_bottom, sv_left, sv_right, sv_x_center, sv_y_center;
	DRAW_POINT_X_Y *list_pt;
	ELEMENT *save_ele;

	FRAME_DATA(i) ele = (ELEMENT *)p_alloc(sizeof(ELEMENT));
#ifdef NOT_USED
	clean_ele_list(FRAME_DATA(i) neg_ele);
#endif
	FRAME_DATA(i) neg_ele = NULL;
	if (TYPE_OF_FRAME(i) == PL_TEXT || TYPE_OF_FRAME(i) == PL_MISC
		|| TYPE_OF_FRAME(i) == PL_FLOW)
		LAYOUT(wn) block_leading = lmt_off_to_abs(wn, 0, LEADING(i));
	/* save absolute lead */
	else
		LAYOUT(wn) block_leading = LAYOUT(wn) spec_leading;
	/* depth offset */
	LAYOUT(wn) dpo = lmt_off_to_abs(wn, Y_REF, DEPTH_OFFSET(i));
	/* width offset */
	LAYOUT(wn) wo = lmt_off_to_abs(wn, X_REF, WIDTH_OFFSET(i));
	
	lmt_compute_y(wn, i);	/* Eval. rot_rect_{top, bottom, y_center} */
	lmt_compute_x(wn, i);	/* Eval. rot_rect_{left, right, x_center} */
	
	if (TYPE_OF_FRAME(i) == PL_RBX)
		/* Get type of rule, adjust rot_rect_{...}
		   accordingly to the type and the weight
		   of the rule */
		lmt_IsARule(wn, i, &type_of_rule, &h_weight, &v_weight);
	if (do_vj && !*start_vj && NUM_VJ(i))
	{							/* If you do vert. just., initialize
								   variables for it */
		if (i - 1 > 0)
			*top_vj = ELE_DATA(i - 1) rot_rect_top;
		else
			*top_vj = lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
		*start_vj = (int16)i;
		*num_vj = 0;
		*depth_vj = 0;
	}
	if (*start_vj && do_vj)
	{							/* Do vertical justification */
		char *ptr;
		int16 svi6, svi7;
		int32 svd5, drvj, dpvj;
		
		j = (int16)NUM_VJ(i);
		if (j || i == *start_vj)
			*depth_vj += ELE_DATA(i) rot_rect_bottom -
				ELE_DATA(i) rot_rect_top;
		else
			*depth_vj += ELE_DATA(i) rot_rect_bottom -
				ELE_DATA(i - 1) rot_rect_top;
		if (j < 0)
			j = abs(j) << 1;
		*num_vj += j;
		svi6 = D_REL_1(i);
		svi7 = D_REL_2(i);
		svd5 = DEPTH_OFFSET(i);
		ptr = &VJ_REL_1_1(i);
		D_REL_1(i) = (int16)*ptr << 8 | (int16)*(ptr + 1);
		D_REL_2(i) = (int16)*(ptr + 2) << 8 | (int16)*(ptr + 3);
		DEPTH_OFFSET(i) = VJ_Y_OFFSET(i);
		drvj = lmt_rel_to_abs(wn, 6, 6, &FRAME_DATA(i) rel_data);
		D_REL_1(i) = svi6;
		D_REL_2(i) = svi7;
		DEPTH_OFFSET(i) = svd5;
		dpvj = lmt_off_to_abs(wn, Y_REF, VJ_Y_OFFSET(i));
		if (drvj || dpvj)		/* Do vertical justification */
		{
			int16 a = 0, b = 0;
			int32 vj, c = 0, d = 0;

			*frame_vj = ((int32)i << 16) | (int32)*start_vj;
			if (!drvj)
				drvj = *top_vj;
			vj = drvj + dpvj - *top_vj - *depth_vj;
			if (vj <= 0)
				vj = 0;
			else
				vj /= (int32)num_vj;
			for (j = *start_vj; j <= i; j++)
			{
				if (NUM_VJ(j))
				{
					k = (int16)NUM_VJ(j);
					if (k < 0)
						k = abs(k);
					if (j - 1 < 1)
						Y_REL_1(j) = 0x1100;
					else
						Y_REL_1(j) = 0x2200 | ((j - 1) & 0xFF);
					Y_REL_2(j) = 0;
					WIDTH_OFFSET(j) = ((k * wn -> yx_convert[0])
									   & PL_SYNBITS) | 0x8000000;
				}
				interference(wn, (int)j, &a, &b, &c, &d, frame_vj, 0);
			}
			start_vj = 0;
		}
	}
							/* Vertical justification done */
	FRAME_DATA(i) top = sv_top = ELE_DATA(i) rot_rect_top;
	FRAME_DATA(i) bottom = sv_bottom = ELE_DATA(i) rot_rect_bottom;
	FRAME_DATA(i) y_center =
		sv_y_center = ELE_DATA(i) rot_rect_y_center;
	FRAME_DATA(i) left = sv_left = ELE_DATA(i) rot_rect_left;
	FRAME_DATA(i) right = sv_right = ELE_DATA(i) rot_rect_right;
        if (FRAME_DATA(i) do_crop == 1)  
	  do_crop(wn,i,FRAME_DATA(i)left,FRAME_DATA(i)top,FRAME_DATA(i)right, 
		  FRAME_DATA(i)bottom);
        else FRAME_DATA(i) do_crop = 0; 
	FRAME_DATA(i) x_center =
		sv_x_center = ELE_DATA(i) rot_rect_x_center;
	/* Setup element(s) for interference 
	   by taking the points from the frame,
	   adding to it the gutters (for
	   interference purposes and the (x,y)
	   offset of the frame to it and building
	   the rectangle aroung the result */
	
   	if(TYPE_OF_FRAME(i) == PL_GRAPHIC)
	{
       	/* prev_top    = org_pos top */
       	/* prev_left   = org_pos left */
       	/* prev_bottom = org_pos bottom */
       	/* prev_right  = org_pos left */
		
       	FRAME_DATA(i) prev_top =
			(FRAME_DATA(i)top - ((lmt_off_to_abs(wn,Y_REF, CROP_TOP(i)) *
								  ZOOM_GRY(i) + HALF_ZOOM_UNIT) / ZOOM_UNIT));
        FRAME_DATA(i) prev_left = (FRAME_DATA(i)left -
								   ((lmt_off_to_abs(wn,X_REF, CROP_LEFT(i)) *
									 ZOOM_GR(i) + HALF_ZOOM_UNIT) / ZOOM_UNIT));
      	FRAME_DATA(i) prev_bottom = (FRAME_DATA(i)bottom +
									 ((lmt_off_to_abs(wn,Y_REF,
													  CROP_BOTTOM(i)) *
									   ZOOM_GRY(i) + HALF_ZOOM_UNIT) / ZOOM_UNIT));
        FRAME_DATA(i) prev_right = (FRAME_DATA(i)right + 
									((lmt_off_to_abs(wn,X_REF,CROP_RIGHT(i)) *
									  ZOOM_GR(i) + HALF_ZOOM_UNIT) / ZOOM_UNIT));
	}
	if (type_of_rule != 3)
	{						/* All frames BUT open box */
		BOOLEAN done = FALSE;
		
		ovale_flag = FALSE;
		if (FRAME_FLAGS(i) & OVALE_SHAPE)
		{
#ifdef AAA
			ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y));
			ELE_DATA(i) n_points = 1;
			ELE_DATA(i) list_points -> draw_type = D_ARC;
			ELE_DATA(i) list_points -> arc =
				(P_ARC *)p_alloc(sizeof(P_ARC));
			ELE_DATA(i) list_points -> x = FRAME_DATA(i) left;
			ELE_DATA(i) list_points -> y = FRAME_DATA(i) top;
			ELE_DATA(i) list_points -> arc -> left = FRAME_DATA(i) left;
			ELE_DATA(i) list_points -> arc -> top = FRAME_DATA(i) top;
			ELE_DATA(i) list_points -> arc -> width =
				abs(FRAME_DATA(i) right - FRAME_DATA(i) left);
			ELE_DATA(i) list_points -> arc -> depth =
				abs(FRAME_DATA(i) bottom - FRAME_DATA(i) top);
			ELE_DATA(i) list_points -> arc -> start_angle = 360 * 64;
			ELE_DATA(i) list_points -> arc -> end_angle = 360 * 64;
#endif
			if (ELE_DATA(i) n_points == 1 || ELE_DATA(i) n_points == 0)
			{
				p_draw_arc(FRAME_DATA(i) left, FRAME_DATA(i) top, 
						   FRAME_DATA(i) right, FRAME_DATA(i) bottom,
						   &ELE_DATA(i) list_points, &ELE_DATA(i) n_points,
						   0, 0, 360 * 64);
				ELE_DATA(i) list_points -> off_x = 0;
				ELE_DATA(i) list_points -> draw_type = D_ARC;
			}
			ovale_flag = TRUE;
			done = TRUE;
		}
		else if (FRAME_FLAGS(i) & BEAROFF_SHAPE
				 && FRAME_DATA(i) num_bearoffs)
		{
			ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
										  FRAME_DATA(i) num_bearoffs);
			ELE_DATA(i) n_points = FRAME_DATA(i) num_bearoffs;
			copy_bearoff_pts(wn, i);
        	ELE_DATA(i) rot_rect_left = FRAME_DATA (i) inter_left;
        	ELE_DATA(i) rot_rect_top = FRAME_DATA (i) inter_top;
        	ELE_DATA(i) rot_rect_right = FRAME_DATA (i) inter_right;
        	ELE_DATA(i) rot_rect_bottom = FRAME_DATA (i) inter_bottom;
			done = TRUE;
			get_box_around_bearoff_shape(wn, i);
		}
		else if (FRAME_FLAGS(i) & POLYGON_SHAPE
				 && FRAME_DATA(i) num_shapes)
		{						/* We have a hand-made polygon */
			shape_bearoff(wn, i, 1, 1);	/* Add the bearoff to shape */
			ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
										  FRAME_DATA(i) num_bearoffs);
			ELE_DATA(i) n_points = FRAME_DATA(i) num_bearoffs;
			copy_bearoff_pts(wn, i);
        	ELE_DATA(i) rot_rect_left = FRAME_DATA (i) inter_left;
        	ELE_DATA(i) rot_rect_top = FRAME_DATA (i) inter_top;
        	ELE_DATA(i) rot_rect_right = FRAME_DATA (i) inter_right;
        	ELE_DATA(i) rot_rect_bottom = FRAME_DATA (i) inter_bottom;
			done = TRUE;
			get_box_around_bearoff_shape(wn, i);
		}
		else if (TYPE_OF_FRAME(i) == PL_GRAPHIC && WRAP_GRAPHIC(i)
				 && FRAME_DATA(i) gr)
		{						/* We have a wrapped graphic */
			FRAME_DATA(i) inter_top = ELE_DATA(i) rot_rect_top
				-= lmt_off_to_abs(wn, Y_REF, TOP_GUTTER(i));
			FRAME_DATA(i) inter_bottom = ELE_DATA(i) rot_rect_bottom
				+=lmt_off_to_abs(wn, Y_REF, BOT_GUTTER(i));
			FRAME_DATA(i) inter_y_center =
				ELE_DATA(i) rot_rect_y_center;
			FRAME_DATA(i) inter_left = ELE_DATA(i) rot_rect_left
				-= lmt_off_to_abs(wn, X_REF, LEFT_GUTTER(i));
			FRAME_DATA(i) inter_right = ELE_DATA(i) rot_rect_right
				+= lmt_off_to_abs(wn, X_REF, RIGHT_GUTTER(i));
			FRAME_DATA(i) inter_x_center = ELE_DATA(i) rot_rect_x_center;
			if (FRAME_FLAGS(i) & RECTANGLE_BEAROFF)
			{
				p_free((char *)FRAME_DATA(i) bearoff_pts);
				FRAME_DATA(i) bearoff_pts = NULL;
				FRAME_DATA(i) num_bearoffs = 0;
				FRAME_FLAGS(i) &= ~RECTANGLE_BEAROFF;
			}
			if (read_px_graphic_in(wn, i))
			{
				if(FRAME_DATA(i) num_bearoffs > 0)
					ELE_DATA(i) list_points = (DRAW_POINT_X_Y *)p_alloc(
																		sizeof(DRAW_POINT_X_Y)
																		* FRAME_DATA(i) num_bearoffs);
				ELE_DATA(i) n_points = FRAME_DATA(i) num_bearoffs;
				copy_bearoff_pts(wn, i);
				if (TYPE_OF_FRAME(i) == PL_GRAPHIC && WRAP_GRAPHIC(i) && (ZOOM_GR(i) != ZOOM_UNIT || ZOOM_GRY(i) != ZOOM_UNIT))
				{
					save_ele = lmt_set_scale_ele(wn, i, i, FRAME_DATA(i) ele);
					if(FRAME_DATA(i) num_bearoffs > 0)
						p_free((char *)ELE_DATA(i) list_points);
					p_free((char *)(FRAME_DATA(i) ele));
					FRAME_DATA(i) ele = save_ele;
				}
        		ELE_DATA(i) rot_rect_left = FRAME_DATA (i) inter_left;
        		ELE_DATA(i) rot_rect_top = FRAME_DATA (i) inter_top;
        		ELE_DATA(i) rot_rect_right = FRAME_DATA (i) inter_right;
        		ELE_DATA(i) rot_rect_bottom
					= FRAME_DATA (i) inter_bottom;
				done = TRUE;
			}
		}
		if (!done)
		{						/* Just add gutter to the rectangle around
								   the frame and create list of points */
			FRAME_DATA(i) inter_top = ELE_DATA(i) rot_rect_top
				-= lmt_off_to_abs(wn, Y_REF, TOP_GUTTER(i));
			FRAME_DATA(i) inter_bottom = ELE_DATA(i) rot_rect_bottom
				+=lmt_off_to_abs(wn, Y_REF, BOT_GUTTER(i));
			FRAME_DATA(i) inter_y_center =
				ELE_DATA(i) rot_rect_y_center;
			FRAME_DATA(i) inter_left = ELE_DATA(i) rot_rect_left
				-= lmt_off_to_abs(wn, X_REF, LEFT_GUTTER(i));
			FRAME_DATA(i) inter_right = ELE_DATA(i) rot_rect_right
				+= lmt_off_to_abs(wn, X_REF, RIGHT_GUTTER(i));
			FRAME_DATA(i) inter_x_center
				= ELE_DATA(i) rot_rect_x_center;
			FRAME_FLAGS(i) |= RECTANGLE_BEAROFF;
			list_pt = ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
			lmt_BuildRectangle(list_pt, FRAME_DATA (i) ele,
							   FRAME_DATA (i) inter_top,
							   FRAME_DATA (i) inter_bottom,
							   FRAME_DATA (i) inter_left,
							   FRAME_DATA (i) inter_right);
			if(FRAME_DATA(i) bearoff_pts)
				p_free((char *)FRAME_DATA(i) bearoff_pts);
			FRAME_DATA(i) bearoff_pts =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
			FRAME_DATA(i) bearoff_pts[0].x = FRAME_DATA(i) inter_left
				- FRAME_DATA(i) left;
			FRAME_DATA(i) bearoff_pts[0].y = FRAME_DATA(i) inter_top
				- FRAME_DATA(i) top;
			FRAME_DATA(i) bearoff_pts[1].x = FRAME_DATA(i) inter_right
				- FRAME_DATA(i) left;
			FRAME_DATA(i) bearoff_pts[1].y = FRAME_DATA(i) inter_top
				- FRAME_DATA(i) top;
			FRAME_DATA(i) bearoff_pts[2].x = FRAME_DATA(i) inter_right
				- FRAME_DATA(i) left;
			FRAME_DATA(i) bearoff_pts[2].y = FRAME_DATA(i) inter_bottom
				- FRAME_DATA(i) top;
			FRAME_DATA(i) bearoff_pts[3].x = FRAME_DATA(i) inter_left
				- FRAME_DATA(i) left;
			FRAME_DATA(i) bearoff_pts[3].y = FRAME_DATA(i) inter_bottom
				- FRAME_DATA(i) top;
			FRAME_DATA(i) num_bearoffs = 4;
		}
		/* Copy back into the frame
		   the new list of points */
		if (FRAME_DATA (i) out_lst_pts &&
			FRAME_DATA (i) out_lst_pts != FRAME_DATA(i) list_points)
		{
			p_free((char *)FRAME_DATA(i) out_lst_pts);
			FRAME_DATA(i) out_lst_pts = NULL;
		}
		if (FRAME_DATA(i) list_points)
		{
			p_free((char *)FRAME_DATA(i) list_points);
			FRAME_DATA(i) list_points = NULL;
			FRAME_DATA(i) out_lst_pts = NULL;
		}
		FRAME_DATA(i) n_points = ELE_DATA(i) n_points;
		if (FRAME_DATA(i) n_points)
			FRAME_DATA(i) out_lst_pts = FRAME_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
										  FRAME_DATA(i) n_points);
		memcpy(FRAME_DATA(i) list_points, ELE_DATA(i) list_points,
			   sizeof(DRAW_POINT_X_Y) * FRAME_DATA(i) n_points);
	}
    else
	{							/* Only frames that are an open box */
        ELEMENT *ele;

		FRAME_DATA(i) inter_top = ELE_DATA(i) rot_rect_top
			-= lmt_off_to_abs(wn, Y_REF, TOP_GUTTER(i));
		FRAME_DATA(i) inter_bottom = ELE_DATA(i) rot_rect_bottom
			+=lmt_off_to_abs(wn, Y_REF, BOT_GUTTER(i));
		FRAME_DATA(i) inter_y_center = ELE_DATA(i) rot_rect_y_center;
		FRAME_DATA(i) inter_left = ELE_DATA(i) rot_rect_left
			-= lmt_off_to_abs(wn, X_REF, LEFT_GUTTER(i));
		FRAME_DATA(i) inter_right = ELE_DATA(i) rot_rect_right
			+= lmt_off_to_abs(wn, X_REF, RIGHT_GUTTER(i));
		FRAME_DATA(i) inter_x_center = ELE_DATA(i) rot_rect_x_center;
        ele = FRAME_DATA(i) ele;
		if (ele -> list_points)
			p_free((char *)ele -> list_points);
        list_pt = ele -> list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
		lmt_BuildRectangle(list_pt, FRAME_DATA (i) ele,
						   FRAME_DATA (i) inter_top, 
						   FRAME_DATA (i) inter_bottom,
						   FRAME_DATA (i) inter_left,
						   FRAME_DATA (i) inter_right);
		if (FRAME_DATA (i) out_lst_pts &&
			FRAME_DATA (i) out_lst_pts !=  FRAME_DATA(i) list_points)
			p_free((char *)FRAME_DATA(i) out_lst_pts);
		if (FRAME_DATA(i) list_points)
			p_free((char *)FRAME_DATA(i) list_points);
		FRAME_DATA(i) n_points = ELE_DATA(i) n_points;
		FRAME_DATA(i) out_lst_pts = FRAME_DATA(i) list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
									  FRAME_DATA(i) n_points);
		memcpy(FRAME_DATA(i) list_points, ELE_DATA(i) list_points,
			   sizeof(DRAW_POINT_X_Y) * FRAME_DATA(i) n_points);
		/* Top horizontal */
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) inter_top,
						   FRAME_DATA(i) top + v_weight,
						   FRAME_DATA(i) inter_left,
						   FRAME_DATA(i) inter_right);
        ele -> next = (ELEMENT *)p_alloc(sizeof(ELEMENT));
		ele -> next -> prev = ele;
        ele = ele -> next;
        list_pt = ele -> list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
		/* Right vertical */
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) inter_top,
						   FRAME_DATA(i) inter_bottom,
						   FRAME_DATA(i) right - h_weight,
						   FRAME_DATA(i) inter_right);
        ele -> next = (ELEMENT *)p_alloc(sizeof(ELEMENT));
		ele -> next -> prev = ele;
        ele = ele -> next;
        list_pt = ele -> list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
		/* Bottom horizontal */
        lmt_BuildRectangle(list_pt, ele,
						   FRAME_DATA(i) bottom - v_weight,
						   FRAME_DATA(i) inter_bottom,
						   FRAME_DATA(i) inter_left,
						   FRAME_DATA(i) inter_right);
        ele -> next = (ELEMENT *)p_alloc(sizeof(ELEMENT));
		ele -> next -> prev = ele;
        ele = ele -> next;
        list_pt = ele -> list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
		/* Left vertical */
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) inter_top,
						   FRAME_DATA(i) inter_bottom,
						   FRAME_DATA(i) inter_left,
						   FRAME_DATA(i) left + h_weight);
	}
	lmt_set_rot_frame(wn, i);
	/* Save bearoff list in case we want to display it */
	if (FRAME_DATA(i) bearoff_lst)
		p_free((char *)FRAME_DATA(i) bearoff_lst);
	FRAME_DATA(i) bearoff_lst =
		(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
								  FRAME_DATA(i) n_points);
	FRAME_DATA(i) b_points = FRAME_DATA(i) n_points;
	memcpy(FRAME_DATA(i) bearoff_lst, FRAME_DATA(i) out_lst_pts,
		   sizeof(DRAW_POINT_X_Y) * FRAME_DATA(i) n_points);
	if (!(FRAME_FLAGS(i) & OVERLAY))
		process_element(wn, i);	/* Do the actual interference */
	/* Restore the data without
	   the gutters */
    FRAME_DATA(i) inter_top = sv_top;
    FRAME_DATA(i) inter_bottom = sv_bottom;
    FRAME_DATA(i) inter_y_center = sv_y_center;
    FRAME_DATA(i) inter_left = sv_left;
    FRAME_DATA(i) inter_right = sv_right;
    FRAME_DATA(i) inter_x_center = sv_x_center;
	/* Setup element(s) for interference 
	   by taking the points from the frame
	   and adding the (x,y) offset of the
	   frame to it and building the rectangle
	   aroung the result */
    if (type_of_rule != 3)
	{							/* All frames BUT open box */
		int x_rounded = lmt_off_to_abs(wn, X_REF, TL_ROUNDED(i));

		if (ELE_DATA(i) list_points)
			clean_draw_point_x_y(ELE_DATA(i) list_points,ELE_DATA(i) n_points);
		if (x_rounded)
		{
			DRAW_POINT_X_Y *lp, *cp, *plp;
			int16 pts;
			int y_rounded = lmt_off_to_abs(wn, Y_REF, TL_ROUNDED(i));
			int z;
			
			ELE_DATA(i) rot_rect_top = sv_top;
			ELE_DATA(i) rot_rect_bottom = sv_bottom;
			ELE_DATA(i) rot_rect_y_center = sv_y_center;
			ELE_DATA(i) rot_rect_left = sv_left;
			ELE_DATA(i) rot_rect_right = sv_right;
			ELE_DATA(i) rot_rect_x_center = sv_x_center;
			p_draw_arc(0, 0, x_rounded << 1, y_rounded << 1,
										&lp, &pts, 0, 90 * 64, 90 * 64);
/*			p_info(PI_TRACE, "pts = %d\n",pts); */
			ELE_DATA(i) n_points = 4 * pts;
			cp = ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc((sizeof(DRAW_POINT_X_Y) *
										   ELE_DATA(i) n_points));
			/* Top right */
			for (z = 0, plp = lp; z < pts; z++, cp++, plp++)
			{
				cp -> x = plp -> x + FRAME_DATA(i) left;
				cp -> y = plp -> y + FRAME_DATA(i) top;
			}
			/* Top right */
			for (z = 0, plp = lp + pts - 1; z < pts; z++, cp++, plp--)
			{
				cp -> x = -(plp -> x) + FRAME_DATA(i) right;
				cp -> y = plp -> y + FRAME_DATA(i) top;
			}
			/* Bottom right */
			for (z = 0, plp = lp; z < pts; z++, cp++, plp++)
			{
				cp -> x = -(plp -> x) + FRAME_DATA(i) right;
				cp -> y = -(plp -> y) + FRAME_DATA(i) bottom;
			}
			/* Bottom left */
			for (z = 0, plp = lp + pts - 1; z < pts; z++, cp++, plp--)
			{
				cp -> x = plp -> x + FRAME_DATA(i) left;
				cp -> y = -(plp -> y) + FRAME_DATA(i) bottom;
			}
			p_free((char *)lp);
		}
		else if (FRAME_FLAGS(i) & OVALE_SHAPE)
		{
			p_draw_arc(FRAME_DATA(i) left, FRAME_DATA(i) top,
					   FRAME_DATA(i) right, FRAME_DATA(i) bottom,
					   &ELE_DATA(i) list_points,  &ELE_DATA(i) n_points,
					   0, 0, 360 * 64);
			ELE_DATA(i) list_points -> off_x = 0;
			ELE_DATA(i) list_points -> draw_type = D_ARC;
		}
#ifdef ARC_ARC
		else if (FRAME_FLAGS(i) & OVALE_SHAPE)
		{
			ELE_DATA(i) list_points = (DRAW_POINT_X_Y *)p_alloc(
																sizeof(DRAW_POINT_X_Y));
			ELE_DATA(i) n_points = 1;
			ELE_DATA(i) list_points -> draw_type = D_ARC;
			ELE_DATA(i) list_points -> arc =
				(P_ARC *)p_alloc(sizeof(P_ARC));
			ELE_DATA(i) list_points -> x = FRAME_DATA(i) inter_left;
			ELE_DATA(i) list_points -> y = FRAME_DATA(i) inter_top;
			ELE_DATA(i) list_points -> arc -> left = FRAME_DATA(i) inter_left;
			ELE_DATA(i) list_points -> arc -> top = FRAME_DATA(i) inter_top;
			ELE_DATA(i) list_points -> arc -> width =
				abs(FRAME_DATA(i) inter_right - FRAME_DATA(i) inter_left);
			ELE_DATA(i) list_points -> arc -> depth =
				abs(FRAME_DATA(i) inter_bottom - FRAME_DATA(i) inter_top);
			ELE_DATA(i) list_points -> arc -> start_angle = 360 * 64;
			ELE_DATA(i) list_points -> arc -> end_angle = 360 * 64;
		}
#endif
		else if (FRAME_FLAGS(i) & POLYGON_SHAPE  && FRAME_DATA(i) num_shapes)
		{
			ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
										  FRAME_DATA(i) num_shapes);
			ELE_DATA(i) n_points = FRAME_DATA(i) num_shapes;
			copy_shape_pts(wn, i, 1, 1);
        	ELE_DATA(i) rot_rect_left = FRAME_DATA (i) left;
        	ELE_DATA(i) rot_rect_top = FRAME_DATA (i) top;
        	ELE_DATA(i) rot_rect_right = FRAME_DATA (i) right;
        	ELE_DATA(i) rot_rect_bottom = FRAME_DATA (i) bottom;
		}
		else if (TYPE_OF_FRAME(i) == PL_GRAPHIC && WRAP_GRAPHIC(i)
				 && FRAME_DATA(i) gr && read_px_graphic_in(wn, i))
		{
			ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
										  FRAME_DATA(i) num_shapes);
			ELE_DATA(i) n_points = FRAME_DATA(i) num_shapes;
			copy_shape_pts(wn, i, wn -> msb, wn -> ldb);
        	ELE_DATA(i) rot_rect_left = FRAME_DATA (i) left;
        	ELE_DATA(i) rot_rect_top = FRAME_DATA (i) top;
        	ELE_DATA(i) rot_rect_right = FRAME_DATA (i) right;
        	ELE_DATA(i) rot_rect_bottom = FRAME_DATA (i) bottom;
		}
		else
		{
			list_pt = ELE_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
        	lmt_BuildRectangle(list_pt, FRAME_DATA (i) ele,
							   FRAME_DATA (i) top, FRAME_DATA (i) bottom,
							   FRAME_DATA (i) left, FRAME_DATA (i) right);
		}
		if (FRAME_DATA (i) out_lst_pts &&
			FRAME_DATA (i) out_lst_pts != FRAME_DATA(i) list_points)
			p_free((char *)FRAME_DATA(i) out_lst_pts);
		if (FRAME_DATA(i) list_points)
			p_free((char *)FRAME_DATA(i) list_points);
		FRAME_DATA(i) n_points = ELE_DATA(i) n_points;
		FRAME_DATA(i) out_lst_pts = FRAME_DATA(i) list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
									  FRAME_DATA(i) n_points);
		memcpy(FRAME_DATA(i) list_points, ELE_DATA(i) list_points,
			   sizeof(DRAW_POINT_X_Y) * FRAME_DATA(i) n_points);
	}
    else
	{							/* Only frames that are an open box */
        ELEMENT *ele;

        ele = FRAME_DATA(i) ele;
		if (ele -> list_points)
			p_free((char *)ele -> list_points);
		list_pt = ele -> list_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
									  ELE_DATA(i) n_points);
		lmt_BuildRectangle(list_pt, FRAME_DATA (i) ele,
						   FRAME_DATA (i) top, FRAME_DATA (i) bottom,
						   FRAME_DATA (i) left, FRAME_DATA (i) right);
		if (FRAME_DATA (i) out_lst_pts &&
			FRAME_DATA (i) out_lst_pts !=  FRAME_DATA(i) list_points)
			p_free((char *)FRAME_DATA(i) out_lst_pts);
		if (FRAME_DATA(i) list_points)
			p_free((char *)FRAME_DATA(i) list_points);
		FRAME_DATA(i) n_points = ele -> n_points;
		FRAME_DATA(i) out_lst_pts =
			FRAME_DATA(i) list_points =
				(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
										  FRAME_DATA(i) n_points);
		memcpy(FRAME_DATA(i) list_points, ELE_DATA(i) list_points,
			   sizeof(DRAW_POINT_X_Y) * FRAME_DATA(i) n_points);
		list_pt = ele -> list_points;
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) top,
						   FRAME_DATA(i) top + v_weight, FRAME_DATA(i) left,
						   FRAME_DATA(i) right);
		ele = ele -> next;
		list_pt = ele -> list_points;
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) top, 
						   FRAME_DATA(i) bottom,
						   FRAME_DATA(i) right - h_weight,FRAME_DATA(i) right);
		ele = ele -> next;
		list_pt = ele -> list_points;
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) bottom - v_weight,
						   FRAME_DATA(i) bottom, FRAME_DATA(i) left,
						   FRAME_DATA(i) right);
		ele = ele -> next;
		list_pt = ele -> list_points;
        lmt_BuildRectangle(list_pt, ele, FRAME_DATA(i) top, 
						   FRAME_DATA(i) bottom, FRAME_DATA(i) left,
						   FRAME_DATA(i) left + h_weight);
	}
}

static void rotate_ele(WYSIWYG *wn, int frame_num, ELEMENT *ele)
{
	ELEMENT *save_ele;
	int i;

	if (ROT_DEGREE(frame_num))
	{
		save_ele = lmt_set_rot_ele(wn, frame_num, frame_num, ele);
		ele -> out_lst_pts = save_ele -> list_points;
		ele -> rot_rect_top = save_ele -> rot_rect_top;
		ele -> rot_rect_bottom = save_ele -> rot_rect_bottom;
		ele -> rot_rect_left = save_ele -> rot_rect_left;
		ele -> rot_rect_right = save_ele -> rot_rect_right;
		ele -> rot_rect_x_center = save_ele -> rot_rect_x_center;
		ele -> rot_rect_y_center = save_ele -> rot_rect_y_center;
		p_free((char *)save_ele);
		ele -> rect_top = ele -> list_points[0].y;
		ele -> rect_bottom = ele -> list_points[0].y;
		ele -> rect_left = ele -> list_points[0].x;
		ele -> rect_right = ele -> list_points[0].x;
		for (i = 1; i < ele -> n_points; i++)
		{
			if (ele -> rect_top > ele -> list_points[i].y)
				ele -> rect_top = ele -> list_points[i].y;
			if (ele -> rect_bottom < ele -> list_points[i].y)
				ele -> rect_bottom = ele -> list_points[i].y;
			if (ele -> rect_left > ele -> list_points[i].x)
				ele -> rect_left = ele -> list_points[i].x;
			if (ele -> rect_right < ele -> list_points[i].x)
				ele -> rect_right = ele -> list_points[i].x;
		}
	}
	else if (TYPE_OF_FRAME(frame_num) == PL_GRAPHIC && WRAP_GRAPHIC(frame_num) && (ZOOM_GR(frame_num) != ZOOM_UNIT || ZOOM_GRY(frame_num) != ZOOM_UNIT))
	{
		save_ele = lmt_set_scale_ele(wn, frame_num, frame_num, ele);
		ele -> out_lst_pts = save_ele -> list_points;
		ele -> rot_rect_top = save_ele -> rot_rect_top;
		ele -> rot_rect_bottom = save_ele -> rot_rect_bottom;
		ele -> rot_rect_left = save_ele -> rot_rect_left;
		ele -> rot_rect_right = save_ele -> rot_rect_right;
		ele -> rot_rect_x_center = save_ele -> rot_rect_x_center;
		ele -> rot_rect_y_center = save_ele -> rot_rect_y_center;
		p_free((char *)save_ele);
		ele -> rect_top = ele -> list_points[0].y;
		ele -> rect_bottom = ele -> list_points[0].y;
		ele -> rect_left = ele -> list_points[0].x;
		ele -> rect_right = ele -> list_points[0].x;
		for (i = 1; i < ele -> n_points; i++)
		{
			if (ele -> rect_top > ele -> list_points[i].y)
				ele -> rect_top = ele -> list_points[i].y;
			if (ele -> rect_bottom < ele -> list_points[i].y)
				ele -> rect_bottom = ele -> list_points[i].y;
			if (ele -> rect_left > ele -> list_points[i].x)
				ele -> rect_left = ele -> list_points[i].x;
			if (ele -> rect_right < ele -> list_points[i].x)
				ele -> rect_right = ele -> list_points[i].x;
		}
    }	   	
	else
	{
		ele -> out_lst_pts = ele -> list_points;
		ele -> rect_top = ele -> rot_rect_top;
		ele -> rect_bottom = ele -> rot_rect_bottom;
		ele -> rect_left = ele -> rot_rect_left;
		ele -> rect_right = ele -> rot_rect_right;
		ele -> rect_x_center = ele -> rot_rect_x_center;
		ele -> rect_y_center = ele -> rot_rect_y_center;
	}
}

static void adjust_data(WYSIWYG *wn, int i, ELEMENT **elements,
						ELEMENT **neg_elements)
{
	ELEMENT *ele, *i_ele, *j_ele, *neg_i_ele, *neg_j_ele;
	int same_element;
	
	/* Rotate the frame */
	lmt_set_rot_frame(wn, i);
	/* Rotate all the elements inside
	   the frame */
	for (ele = FRAME_DATA(i) ele; ele; ele = ele -> next)
		rotate_ele(wn, i, ele);
	for (i_ele = FRAME_DATA(i) ele; i_ele; i_ele = i_ele -> next)
	{
		same_element = 0;
		j_ele = *(elements + i);
		if (not_first_time && !j_ele && FRAME_DATA(i) Fmap_data)
		{						/* The element is re-born,
								   give it some map_data values */
			memcpy((char *)&i_ele -> map_data,
				   (char *)FRAME_DATA(i) Fmap_data, sizeof(MAP_DATA));
			/* Make sure only the last element
			   shows the end of a flow */
			if (i_ele -> next)
				i_ele -> map_data.command_flags &= ~EF;
			continue;
		}
		for (; j_ele; j_ele = j_ele -> next)
		{
			i_ele -> oitems.head = j_ele -> oitems.head;
			i_ele -> oitems.tail = j_ele -> oitems.tail;
			j_ele -> oitems.head = NULL;
			j_ele -> oitems.tail = NULL;
			if(FRAME_DATA(i) Fmap_data &&
			   FRAME_DATA(i) Fmap_data -> vj_mode & 0x4000)
			{
				same_element = 1;
				break;
			}
			if (lmt_IsTwoDiffEle(i_ele, j_ele) == NO)
			{					/* Same element */
				same_element = 1;
				break;
			}
		}
		if (same_element && !FRAME_DATA(i) neg_ele)
		{
			memcpy((char *)&i_ele -> map_data,
				   (char *)&j_ele -> map_data, sizeof(MAP_DATA));
			i_ele -> preview_done = j_ele -> preview_done;
			i_ele -> items.head = j_ele -> items.head;
			i_ele -> items.tail = j_ele -> items.tail;
			j_ele -> items.head = NULL;
			j_ele -> items.tail = NULL;
			i_ele -> shape.head = j_ele -> shape.head;
			i_ele -> shape.tail = j_ele -> shape.tail;
			j_ele -> shape.head = NULL;
			j_ele -> shape.tail = NULL;
			i_ele -> trap_points = j_ele -> trap_points;
			j_ele -> trap_points = NULL;
			i_ele -> trap_pts = j_ele -> trap_pts;
			j_ele -> trap_pts = 0;
		}
	}
	if (not_first_time && !(*(elements + i)) && FRAME_DATA(i) ele)
	{			/* The element is re-born, flash the text file */
		if (ARTICLE_ID(i))
			set_tf_process_flag(wn, ARTICLE_ID(i), NEED_TO_MAP_HNJ);
		else if (TYPE_OF_FRAME(i) == PL_TEXT && HNJ_TEXT(i) && *HNJ_TEXT(i))
			set_layout_process_flag(wn -> selected_lay);
	}
	else for (i_ele = *(elements + i); i_ele; i_ele = j_ele)
	{
		j_ele = i_ele -> next;
		if (i_ele -> items.head)
		{						/* Queue of items was not re-assigned,
								   hence the shape of the element
								   changed (or died...), flash the
								   text file */
			if (ARTICLE_ID(i))
				set_tf_process_flag(wn, ARTICLE_ID(i), NEED_TO_MAP_HNJ);
			else if (TYPE_OF_FRAME(i) == PL_TEXT
					 && HNJ_TEXT(i) && *HNJ_TEXT(i))
				set_layout_process_flag(wn -> selected_lay);
		}
		clean_ele(i_ele);
	}
	if (LAYOUT(wn) Frame == i)
	{						/* This frame is being previewed,
							   stop it  */
		LAYOUT(wn) Frame = 0;
		LAYOUT(wn) Ele = NULL;
		LAYOUT(wn) end_rec = LAYOUT(wn) fo_rec;
		LAYOUT(wn) end_wrd = LAYOUT(wn) fo_wrd;
		LAYOUT(wn) frame_vs_overflow = 0;
		clean_items_que(&LAYOUT(wn) items);
		for (ele = FRAME_DATA(i) ele; ele; ele = ele -> next)
			ele -> preview_done = 1;
	}
	/* Just delete the old negative element
	   since interference rebuilt it */
#ifdef LINT
	neg_j_ele = NULL;
#endif
	for (neg_i_ele = *(neg_elements + i); neg_i_ele; neg_i_ele = neg_j_ele)
	{
		neg_j_ele = neg_i_ele -> next;
		clean_ele(neg_i_ele);
	}
	/* Rotate all the negative elements
	   inside the frame */
	for (ele = FRAME_DATA(i) neg_ele; ele; ele = ele -> next)
		rotate_ele(wn, i, ele);
}

int lmt_interface(WYSIWYG *wn)
{
	int16 start_vj, num_vj;
	int32 top_vj, depth_vj, frame_vj;
	int i;
	ELEMENT **elements, **neg_elements, *ele;
	
#ifdef TRACE
	if (trace_debugger)
		p_info(PI_TRACE, "In lmt_interface(wn: %#X)\n", wn);
#endif
	start_vj = num_vj = 0;
	frame_vj = top_vj = depth_vj = 0L;
	FRAME_DATA(0) bottom = ELE_DATA(0) rot_rect_bottom =
		lmt_off_to_abs(wn, Y_REF, TOT_BLOCKS);
	FRAME_DATA(0) right = ELE_DATA(0) rot_rect_right =
		lmt_off_to_abs(wn, X_REF, NEW_TOT_BLOCKS);
	LAYOUT(wn) spec_leading = lmt_off_to_abs(wn, Y_REF, LEADING(0));
	if (NEW_TOT_BLOCKS)
	{
		elements = (ELEMENT **)p_alloc((NEW_TOT_BLOCKS + 1)
									   * sizeof(ELEMENT *));
		neg_elements = (ELEMENT **)p_alloc((NEW_TOT_BLOCKS + 1)
										   * sizeof(ELEMENT *));
		for (i = 1; i <= NEW_TOT_BLOCKS; i++) /* for all blocks */
		{
			*(elements + i) = FRAME_DATA(i) ele;
			*(neg_elements + i) = FRAME_DATA(i) neg_ele;
			interference(wn, i, &start_vj, &num_vj, &top_vj, &depth_vj,
						 &frame_vj, 1);
		}
		for (i = 1; i <= NEW_TOT_BLOCKS; i++) /* for all blocks */
		{
			adjust_data(wn, i, elements, neg_elements);
			for (ele = FRAME_DATA(i) ele; ele; ele = ele -> next)
				/*		if (!ele -> trap_points)						TMS */
				trap_bearoff(wn, i, 1, 1, ele);
		}

		p_free((char *)elements);
		p_free((char *)neg_elements);
	}
#ifdef TRACE
	if (trace_debugger)
		p_info(PI_TRACE, "exiting lmt_interface()\n");
	if (trace_lmt)
		trace_end_interface(wn);
#endif
	return(0);
}

#ifdef TRACE
static void print_pts(char *str, DRAW_POINT_X_Y *pts, int n_points)
{
	int k;

	if (str)
		p_info(PI_TRACE, str);
	for (k = 0; k < n_points; k++)
	{
		p_info(PI_TRACE, "<k: %d x: %d, y: %d> ", k, pts[k].x, pts[k].y);
		if (!(k % 5))
			p_info(PI_TRACE, "\n");
	}
	if (k % 5)
		p_info(PI_TRACE, "\n");
}

static void trace_end_interface(WYSIWYG *wn)
{
	int i, k;
	ELEMENT *ele;
	
	for (i = 1; i <= NEW_TOT_BLOCKS; i++) /* for all blocks */
	{
		p_info(PI_TRACE, "Frame %d\n", i);
		p_info(PI_TRACE, "list_points:\n");
		for (k = 0; k < FRAME_DATA(i) n_points; k++)
		{
			p_info(PI_TRACE, "<k: %d x: %d, y: %d> ",
					  k, FRAME_DATA(i) list_points[k].x,
					  FRAME_DATA(i) list_points[k].y);
			if (k & 1)
				p_info(PI_TRACE, "\n");
		}
		if (!((k - 1) & 1))
			p_info(PI_TRACE, "\n");
		p_info(PI_TRACE, "out_lst_points:\n");
		for (k = 0; k < FRAME_DATA(i) n_points; k++)
		{
			p_info(PI_TRACE, "<k: %d x: %d, y: %d> ",
					  k, FRAME_DATA(i) out_lst_pts[k].x,
					  FRAME_DATA(i) out_lst_pts[k].y);
			if (k & 1)
				p_info(PI_TRACE, "\n");
		}
		if (!((k - 1) & 1))
			p_info(PI_TRACE, "\n");
		for (ele = FRAME_DATA(i) ele; ele; ele = ele -> next)
		{
			p_info(PI_TRACE, "rect_left: %d, rect_top: %d\n\
rect_right: %d, rect_bottom: %d\n",
					  ele -> rect_left, ele -> rect_top,
					  ele -> rect_right, ele -> rect_bottom);
			p_info(PI_TRACE, "rot_rect_left: %d, rot_rect_top: %d\n\
rot_rect_right: %d, rot_rect_bottom: %d\n",
					  ele -> rot_rect_left, ele -> rot_rect_top,
					  ele -> rot_rect_right, ele -> rot_rect_bottom);
			print_pts("list_points:\n", ele -> list_points, ele -> n_points);
			print_pts("out_lst_points:\n", ele -> out_lst_pts,ele -> n_points);
		}
	}
	return;
}

static void trace_begin_create_interference(i_ele, j_ele, i, j)
ELEMENT *i_ele, *j_ele;
int i, j;
{
	if (trace_lmt)
	{
		p_info(PI_TRACE, "\nTesting Ele old: %d, new: %d\n", j, i);
		p_info(PI_TRACE, "COOR I: %d\n", i);
		p_info(PI_TRACE, "\tTop: %ld, Bottom: %ld,	Left: %ld, Right: %ld\n\n",
				  i_ele -> rot_rect_top, i_ele -> rot_rect_bottom,
				  i_ele -> rot_rect_left, i_ele -> rot_rect_right);
		p_info(PI_TRACE, "COOR J: %d\n", j);
		p_info(PI_TRACE, "\tTop: %ld, Bottom: %ld,	Left: %ld, Right: %ld\n\n",
				  j_ele -> rot_rect_top, j_ele -> rot_rect_bottom,
				  j_ele -> rot_rect_left, j_ele -> rot_rect_right);
		p_info(PI_TRACE, "\nlmt_BuildElements= old: %d, new: %d, j_ele: %X\n",
				  j, i, j_ele);
	}
}

static void trace_end_create_interference(WYSIWYG *wn, int j)
{
	if (trace_lmt)
	{
		ELEMENT *ele;
		
		p_info(PI_TRACE, "Result for new old frame  %d\n", j);
		for (ele = FRAME_DATA(j) ele; ele; ele = ele -> next)
		{
			p_info(PI_TRACE, "Number of points: %d\n", ele -> n_points);
			print_pts(NULL, ele -> list_points, ele -> n_points);
		}
		if (ele)
			p_info(PI_TRACE, "Result for new negative old frame  %d\n", j);
		for (ele = FRAME_DATA(j) neg_ele; ele; ele = ele -> next)
		{
			p_info(PI_TRACE, "Number of points: %d\n", ele -> n_points);
			print_pts(NULL, ele -> list_points, ele -> n_points);
		}
	}
}

#endif











