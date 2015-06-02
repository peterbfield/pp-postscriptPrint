/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include "p_lib.h"
#include "interfer.h"
#include "clip.f"
#include "frame.h"
#include "traces.h"

#define LIST_SIZE	1

#define MIN_MAX(z1, z2, min, max)	if (z1 < z2)			\
										{					\
										min = z1; max = z2;	\
										}					\
									else					\
										{					\
										min = z2; max = z1;	\
										}

void lmt_compute_slope(PPOINT pt1_x, PPOINT pt1_y, PPOINT pt2_x, PPOINT pt2_y,
					   float *slope, float *b, int *invalid);

void lmt_compute_slope(PPOINT pt1_x, PPOINT pt1_y, PPOINT pt2_x, PPOINT pt2_y,
					   float *slope, float *b, int *invalid)
{
	PPOINT delta_x;
	
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "lmt_compute_slope x1: %ld, x2: %ld, y1: %ld, y2: %ld\n",
				  pt1_x, pt2_x, pt1_y, pt2_y);
#endif
	delta_x = pt2_x - pt1_x;
	if (delta_x)
	{
		*slope = (float)(pt2_y - pt1_y) / (float)delta_x;
		*b = (float)pt1_y - (*slope * (float)pt1_x);
		*invalid = 0;
	}
	else
	{							/* Vertical line */
		*slope = 0.;
		*invalid = 1;
		*b = (float)pt1_x;
	}
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "		invalid: %d, b: %.5f, slope: %.5f\n",
				  *invalid, *b, *slope);
#endif
}

static void compare_edges(EDGES *cur_old_edge, EDGES *cur_new_edge,
						  CLIP_POINTS *cur_old_pt1, CLIP_POINTS *cur_new_pt1,
						  CLIP_POINTS *cur_old_pt2, CLIP_POINTS *cur_new_pt2)
{
	int hard_intersection;
	float temp_x, temp_y;
	PPOINT x, y;
	PPOINT old_x_min, old_x_max, old_y_min, old_y_max;
	PPOINT new_x_min, new_x_max, new_y_min, new_y_max;
	
	if ((cur_old_edge -> invalid && cur_new_edge -> invalid)
		|| (cur_old_edge -> delta == cur_new_edge -> delta
			&& !cur_old_edge -> invalid && !cur_new_edge -> invalid))
	{				/* Parallel lines */
#ifdef TRACE
		if (trace_lmt) p_info(PI_TRACE, "Return from compare_edges\n");
#endif
		return;
	}
	hard_intersection = 0;
	if (cur_old_edge -> pt1.x == cur_new_edge -> pt1.x
		&& cur_old_edge -> pt1.y == cur_new_edge -> pt1.y)
	{
		hard_intersection++;
		if (!cur_old_pt1 -> intersection
			&& !cur_new_pt1 -> intersection)
			lmt_AddIntersection(-1, -1, cur_old_pt1, cur_new_pt1, 1, 1);
	}
	else if (cur_old_edge -> pt1.x == cur_new_edge -> pt2.x
			 && cur_old_edge -> pt1.y == cur_new_edge -> pt2.y)
	{
		hard_intersection++;
		if (!cur_old_pt1 -> intersection
			&& !cur_new_pt2 -> intersection)
			lmt_AddIntersection(-1, -1, cur_old_pt1, cur_new_pt2, 1, 1);
	}
	if (cur_old_edge -> pt2.x == cur_new_edge -> pt1.x
		&& cur_old_edge -> pt2.y == cur_new_edge -> pt1.y)
	{
		hard_intersection++;
		if (!cur_old_pt2 -> intersection
			&& !cur_new_pt1 -> intersection)
			lmt_AddIntersection(-1, -1, cur_old_pt2, cur_new_pt1, 1, 1);
	}
	else if (cur_old_edge -> pt2.x == cur_new_edge -> pt2.x
			 && cur_old_edge -> pt2.y == cur_new_edge -> pt2.y)
	{
		hard_intersection++;
		if (!cur_old_pt2 -> intersection
			&& !cur_new_pt2 -> intersection)
			lmt_AddIntersection(-1, -1, cur_old_pt2, cur_new_pt2, 1, 1);
	}
	else if (!hard_intersection
			 && !(cur_old_edge -> invalid && cur_new_edge -> invalid)
			 && !(cur_old_edge -> delta == cur_new_edge -> delta
				  && !cur_old_edge -> invalid  && !cur_new_edge -> invalid))
	{
		if (cur_old_edge -> invalid)
		{
			temp_x = cur_old_edge -> pt1.x;
			temp_y = temp_x * cur_new_edge -> delta + cur_new_edge -> b;
		}
		else if (cur_new_edge -> invalid)
		{
			temp_x = cur_new_edge -> pt1.x;
			temp_y = temp_x * cur_old_edge -> delta + cur_old_edge -> b;
		}
		else
		{
			temp_x = (cur_new_edge -> b - cur_old_edge -> b)
				/ (cur_old_edge -> delta - cur_new_edge -> delta);
			temp_y = cur_old_edge -> delta * temp_x + cur_old_edge -> b;
		}
		x = (int32)((temp_x > 0.) ? (temp_x + .5) : (temp_x - .5));
		y = (int32)((temp_y > 0.) ? (temp_y + .5) : (temp_y - .5));
		MIN_MAX(cur_old_edge -> pt1.x, cur_old_edge -> pt2.x,
				old_x_min, old_x_max);
		MIN_MAX(cur_old_edge -> pt1.y, cur_old_edge -> pt2.y,
				old_y_min, old_y_max);
		MIN_MAX(cur_new_edge -> pt1.x, cur_new_edge -> pt2.x,
				new_x_min, new_x_max);
		MIN_MAX(cur_new_edge -> pt1.y, cur_new_edge -> pt2.y,
				new_y_min, new_y_max);
#ifdef TRACE
		if (trace_lmt) p_info(PI_TRACE, "x: %ld, y: %ld\n\
old_x_min: %ld, old_x_max: %ld\n\
old_y_min: %ld, old_y_max: %ld\n\
new_x_min: %ld, new_x_max: %ld\n\
new_y_min: %ld, new_y_max: %ld\n",
								 x, y, old_x_min, old_x_max, old_y_min,
								 old_y_max, new_x_min, new_x_max,
								 new_y_min, new_y_max);
#endif
		if (x >= old_x_min && x <= old_x_max && y >= old_y_min
			&& y <= old_y_max && x >= new_x_min
			&& x <= new_x_max && y >= new_y_min && y <= new_y_max)
		{		/* (x,y) is an inter. located on inside both ele. */
			if (x == cur_old_edge -> pt1.x && y == cur_old_edge -> pt1.y)
				lmt_AddIntersection(x, y, cur_old_pt1, cur_new_pt1, 1, 0);
			else if (x == cur_old_edge -> pt2.x && y == cur_old_edge -> pt2.y)
				lmt_AddIntersection(x, y, cur_old_pt2, cur_new_pt1, 1, 0);
			else if (x == cur_new_edge -> pt1.x && y == cur_new_edge -> pt1.y)
				lmt_AddIntersection(x, y, cur_old_pt1, cur_new_pt1, 0, 1);
			else if (x == cur_new_edge -> pt2.x && y == cur_new_edge -> pt2.y)
				lmt_AddIntersection(x, y, cur_old_pt1, cur_new_pt2, 0, 1);
			else
				lmt_AddIntersection(x, y, cur_old_pt1, cur_new_pt1, 0, 0);
		}
	}							/* if (!hard_intersection && ... */
}

void lmt_BuildIntersections(CLIP_POINTS *start_old, CLIP_POINTS *start_new)
{
	EDGES *cur_old_edge = NULL, *cur_new_edge = NULL, *prev_edge = NULL;
	EDGES *edge_start_old = 0, *edge_start_new = 0;
	CLIP_POINTS *cur_old_pt1, *cur_new_pt1, *cur_old_pt2, *cur_new_pt2;
	int old_i = 0, new_i = 0, i;
	
	/*
	  Build all the old edges.
	  */
	cur_old_pt1 = start_old;
	do
	{
		if (!(old_i % LIST_SIZE))
			cur_old_edge = (EDGES *)p_alloc(sizeof(EDGES) * LIST_SIZE);
		else
			cur_old_edge++;
		old_i++;
		if (!edge_start_old)
			edge_start_old = cur_old_edge;
		else
			prev_edge -> next = cur_old_edge;
		memcpy((char *)&cur_old_edge -> pt1, (char *)&cur_old_pt1 -> pt,
			   sizeof(DRAW_POINT_X_Y));
		memcpy((char *)&cur_old_edge -> pt2,
			   (char *)&cur_old_pt1 -> next -> pt, sizeof(DRAW_POINT_X_Y));
		if (cur_old_edge -> pt1.draw_type == D_LINE)
			lmt_compute_slope(cur_old_edge -> pt1.x, cur_old_edge -> pt1.y,
							  cur_old_edge -> pt2.x, cur_old_edge -> pt2.y,
							  &cur_old_edge -> delta, &cur_old_edge -> b,
							  &cur_old_edge -> invalid);
		prev_edge = cur_old_edge;
		cur_old_pt1 = cur_old_pt1 -> next;
	} while (cur_old_pt1 != start_old);
	/*
	  Build all the new edges.
	  */
	cur_new_pt1 = start_new;
	do
	{
		if (!(new_i % LIST_SIZE))
			cur_new_edge = (EDGES *)p_alloc(sizeof(EDGES) * LIST_SIZE);
		else
			cur_new_edge++;
		new_i++;
		if (!edge_start_new)
			edge_start_new = cur_new_edge;
		else
			prev_edge -> next = cur_new_edge;
		memcpy((char *)&cur_new_edge -> pt1, (char *)cur_new_pt1,
			   sizeof(DRAW_POINT_X_Y));
		memcpy((char *)&cur_new_edge -> pt2,
			   (char *)cur_new_pt1 -> next, sizeof(DRAW_POINT_X_Y));
		if (cur_new_edge -> pt1.draw_type == D_LINE)
			lmt_compute_slope( cur_new_edge -> pt1.x, cur_new_edge -> pt1.y,
							  cur_new_edge -> pt2.x, cur_new_edge -> pt2.y,
							  &cur_new_edge -> delta, &cur_new_edge -> b,
							   &cur_new_edge -> invalid);
		prev_edge = cur_new_edge;
		cur_new_pt1 = cur_new_pt1 -> next;
	} while (cur_new_pt1 != start_new);
	/*
	  Compare all the old edges against the new edges.
	  */
	cur_old_edge = edge_start_old;
	cur_old_pt1 = cur_old_pt2 = start_old;
	do
		cur_old_pt2 = cur_old_pt2 -> next;
	while (!cur_old_pt2 -> hard); /* Do nothing */
	/* Skip over the existing intersections */
	while (cur_old_edge)
	{
		cur_new_pt1 = cur_new_pt2 = start_new;
		/* Skip over the existing intersections */
		for (cur_new_edge = edge_start_new; cur_new_edge;
			 cur_new_edge = cur_new_edge -> next)
		{
#ifdef TRACE
			if (trace_lmt)
			{
				p_info(PI_TRACE, "Old_x1: %d, New_x1: %d, Old_y1: %d, New_y1: %d\n\
Old_x2: %d, New_x2: %d, Old_y2: %d, New_y2: %d\n\
Old_delta: %.2f, New_delta: %.2f\n\
Old_invalid: %d, New_invalid: %d\n\
Old_i1: %d, New_i1: %d\n\
Old_i2: %d, New_i2: %d\n",
						  cur_old_edge -> pt1.x, cur_new_edge -> pt1.x,
						  cur_old_edge -> pt1.y, cur_new_edge -> pt1.y,
						  cur_old_edge -> pt2.x, cur_new_edge -> pt2.x,
						  cur_old_edge -> pt2.y, cur_new_edge -> pt2.y,
						  cur_old_edge -> delta, cur_new_edge -> delta,
						  cur_old_edge -> invalid, cur_new_edge -> invalid,
						  cur_old_pt1 -> intersection, cur_new_pt1 -> intersection,
						  cur_old_pt2 -> intersection, cur_new_pt2 -> intersection);
			}
#endif
			cur_new_pt1 = cur_new_pt2;
			do					/* Skip over the existing inter. */
				cur_new_pt2 = cur_new_pt2 -> next;
			while (!cur_new_pt2 -> hard);
			compare_edges(cur_old_edge, cur_new_edge, cur_old_pt1,
						  cur_new_pt1, cur_old_pt2, cur_new_pt2);
		}
		cur_old_edge = cur_old_edge -> next;
		cur_old_pt1 = cur_old_pt2;
		do						/* Skip over the existing inter. */
			cur_old_pt2 = cur_old_pt2 -> next;
		while (!cur_old_pt2 -> hard);
	}							/* End while (cur_old_edge) */
	cur_old_edge = edge_start_old;
	prev_edge = NULL;
	for (i = 0; i < old_i; i++)
	{
		if (!(i % LIST_SIZE))
		{
			if (prev_edge)
				p_free((char *)prev_edge);
			prev_edge = cur_old_edge;
			cur_old_edge = cur_old_edge -> next;
		}
		else
			cur_old_edge = cur_old_edge -> next;
	}
	if (prev_edge)
		p_free((char *)prev_edge);
	cur_new_edge = edge_start_new;
	prev_edge = NULL;
	for (i = 0; i < new_i; i++)
	{
		if (!(i % LIST_SIZE))
		{
			if (prev_edge)
				p_free((char *)prev_edge);
			prev_edge = cur_new_edge;
			cur_new_edge = cur_new_edge -> next;
		}
		else
			cur_new_edge = cur_new_edge -> next;
	}
	if (prev_edge)
		p_free((char *)prev_edge);
}
