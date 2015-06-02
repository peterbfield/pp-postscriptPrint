/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    **
     *****     **    **    **   **    **   **    *******    *******   */

#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "interfer.h"
#include "clip.f"
#include "traces.h"

extern void clean_ele(ELEMENT *ele);
extern int lmt_IsPtInElement(CLIP_POINTS *cur, CLIP_POINTS *start);
extern ELEMENT *CreateElement(LIST_PT *list_pt);
extern void InsertElement(ELEMENT **head, ELEMENT *new);

#ifdef TRACE
void trace_start_ele(CLIP_POINTS *start_old, CLIP_POINTS *start_new);
#endif

static int check_status(CLIP_POINTS *cur, int in_old_ele,
						CLIP_POINTS *start_old, CLIP_POINTS *start_new,
						int forward_old, int forward_new);

static void CreateInsertEle(ELEMENT **head, LIST_PT *list_pt);

static void CreateInsertEle(ELEMENT **head, LIST_PT *list_pt)
{
	ELEMENT *new_ele;
	
	new_ele = CreateElement(list_pt);
	InsertElement(head, new_ele);
}

LIST_PT *lmt_AddPtToList(LIST_PT *list_pt, CLIP_POINTS *cur)
{
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "lmt_AddPtToList: x: %d, y: %d\n", cur -> pt.x, cur -> pt.y);
#endif
	cur -> used = 1;
	if (cur -> twin)
		cur -> twin -> used = 1;
	if (!list_pt)
	{
		list_pt = (LIST_PT *)p_alloc(sizeof(LIST_PT));
		list_pt -> min_x = list_pt -> max_x = cur -> pt.x;
		list_pt -> min_y = list_pt -> max_y = cur -> pt.y;
	}
	if (list_pt -> pts_used == list_pt -> total_pts)
	{
		list_pt -> total_pts += NUM_PTS;
		list_pt -> points =
			(DRAW_POINT_X_Y *)p_remalloc((char *)list_pt -> points,
										 sizeof(DRAW_POINT_X_Y) *
										 (list_pt -> total_pts - NUM_PTS),
										 sizeof(DRAW_POINT_X_Y) *
										 list_pt -> total_pts);
	}
	/*
	  if (cur -> twin)
	  p_info(PI_TRACE, "cur -> pt.arc: %#X, cur -> twin -> pt.arc: %#X\n",
	  cur -> pt.arc, cur -> twin -> pt.arc);
	  if (cur -> twin && cur -> twin -> pt.arc)
	  */
	if (0 && cur -> twin)
	{
		p_info(PI_TRACE, "Writing twin\n");
		memcpy((char *)&list_pt -> points[list_pt -> pts_used++],
			   (char *)&cur -> twin -> pt, sizeof(DRAW_POINT_X_Y));
	}
	else
		memcpy((char *)&list_pt -> points[list_pt -> pts_used++],
			   (char *)&cur -> pt, sizeof(DRAW_POINT_X_Y));
	if (cur -> pt.x < list_pt -> min_x)
		list_pt -> min_x = cur -> pt.x;
	else if (cur -> pt.x > list_pt -> max_x)
		list_pt -> max_x = cur -> pt.x;
	if (cur -> pt.y < list_pt -> min_y)
		list_pt -> min_y = cur -> pt.y;
	else if (cur -> pt.y > list_pt -> max_y)
		list_pt -> max_y = cur -> pt.y;
	return(list_pt);
}

CLIP_POINTS *lmt_BuildClipPoints(DRAW_POINT_X_Y *lp, int n_pts)
{
	CLIP_POINTS *start = NULL, *list, *prev = NULL;
	int i;
	
	for (i = 0; i < n_pts; i++)
	{
		list = (CLIP_POINTS *)p_alloc(sizeof(CLIP_POINTS));
		memcpy((char *)&list -> pt,
					 (char *)lp, sizeof(DRAW_POINT_X_Y));
		list -> hard = 1;
		if (prev)
		{
			prev -> next = list;
			list -> prev = prev;
		}
		else
			start = list;
		prev = list;
		lp++;
	}
	prev -> next = start;
	start -> prev = prev;
	return(start);
}

void lmt_free_1_clip_struct(CLIP_POINTS *start)
{
	CLIP_POINTS *next_cur;
	
	if (start -> prev)
		start -> prev -> next = NULL; /* Break the circular list */
	while (start)
	{
		next_cur = start -> next;
		p_free((char *)start);
		start = next_cur;
	}
}

void lmt_free_clip_struct(start_old, start_new)
CLIP_POINTS *start_old;
CLIP_POINTS *start_new;
{
	lmt_free_1_clip_struct(start_old);
	lmt_free_1_clip_struct(start_new);
}

/* This routine determines whether it's time to switch over to other element.
 * because we are at intersection point *cur between the two, and we need to
 * follow the interference path.  The NEW element always cuts a piece out of
 * the OLD element when they collide.
 * Return value:
 *    0 = Switch over to other element
 *    1 = Stay in current element
 */
static int check_status(CLIP_POINTS *cur, int in_old_ele, 
						CLIP_POINTS *start_old, CLIP_POINTS *start_new,
						int forward_old, int forward_new)
{
	int i;
	
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "check_status cur : %#X, in_old_ele: %d, start_old: %#X, start_new: %#X, forward_old: %#X, forward_new: %#X, cur -> twin -> hard: %#X\n",
				  cur, in_old_ele, start_old, start_new, forward_old, forward_new, cur -> twin -> hard);
#endif
	if (cur -> twin -> hard && !in_old_ele  
		&& !(cur -> next -> intersection
			 || cur -> prev -> intersection))
	{
		if (forward_new)
			i = lmt_IsPtInElement(cur -> next, start_old);
		else
			i = lmt_IsPtInElement(cur -> prev, start_old);
	}
	else if (in_old_ele)
	{
		if (forward_old)
			if (!cur -> next -> intersection && !cur -> next -> used)
				i = !lmt_IsPtInElement(cur -> next, start_new);
			else
				i = 0;
		else
			if (!cur -> prev -> intersection && !cur -> prev -> used)
				i = !lmt_IsPtInElement(cur -> prev, start_new);
			else
				i = 0;
	}
	else		/* (In new file) */
	{
		if (cur -> twin &&
				(forward_new?
				(cur->prev->twin && cur->prev->used && !cur->next->twin) :
				(cur->next->twin && cur->next->used && !cur->prev->twin)))
		{
			if (forward_new)
				i = lmt_IsPtInElement(cur -> next, start_old);
			else
				i = lmt_IsPtInElement(cur -> prev, start_old);
#ifdef TRACE
			if (trace_lmt)
				p_info(PI_TRACE, "I: %d\n", i);
#endif
		}
		else
			i = 0;
	}
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "check_status return: %d\n", i);
#endif
	return(i);
}

int lmt_BuildElements(ELEMENT **head, ELEMENT **neg_head, ELEMENT *old_ele,
					  ELEMENT *new_ele, int neg_flag)
{
	ELEMENT *neg_ele;
	CLIP_POINTS *cur, *start_old, *start_new;
	int i, forward_old, forward_new;
	int in_old_ele, destroy;
	int InNext, InPrev, iNext, iPrev, NextUsed, PrevUsed;
	int do_add_pt = TRUE;
	int specpt = 0;
	int prev_in_old_ele;
	LIST_PT *list_pt = 0;
	
	start_old = lmt_BuildClipPoints(old_ele -> list_points, 
									old_ele -> n_points);
	start_new = lmt_BuildClipPoints(new_ele -> list_points,
									new_ele -> n_points);
#ifdef TRACE
	trace_start_ele(start_old, start_new);
#endif
	lmt_BuildIntersections(start_old, start_new);
#ifdef TRACE
	trace_start_ele(start_old, start_new);
#endif
	for (;;)
	{
		cur = start_old;
		prev_in_old_ele = 0;
		in_old_ele = 1;
		destroy = 0;
		/*
		  Find the next hard point and/or intersection not used
		  and not inside if any.
		  */
		i = 0;
		while (cur -> next != start_old &&
			   (cur -> used || cur -> intersection || !cur -> hard ||
				lmt_IsPtInElement(cur, start_new)))
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "In: cur: %X\n", cur);
#endif
			cur = cur -> next;
		}
		if (cur -> next == start_old && cur -> used)
		{
			cur = start_old;
			while (cur -> next != start_old &&
				   (cur -> used || !cur -> intersection || cur -> hard))
			{
#ifdef TRACE
				if (trace_lmt) p_info(PI_TRACE, "Special In: cur: %X\n", cur);
#endif
				cur = cur -> next;
			}
			if (cur -> next != start_old && !cur -> used)
				specpt = 1;
		}
#ifdef TRACE
		if (trace_lmt) p_info(PI_TRACE, "Starting cur: %X\n", cur);
#endif
		/*
		  This cur is the starting point of a new old-element.
		  This point is a hard and/or intersection, unused point.
		  Use it as the new start.
		  */
		start_old = cur;
		if (!specpt && (cur -> used || lmt_IsPtInElement(cur, start_new)))
		{			/* We are done with old element. */
			if (old_ele -> next)
				old_ele -> next -> prev = old_ele -> prev;
			if (old_ele -> prev)
				old_ele -> prev -> next = old_ele -> next;
			else
				*head = old_ele -> next;
			clean_ele(old_ele);
			old_ele = NULL;
			cur = start_new;
			while (cur -> next != start_new)
			{
				if (cur -> used)
				{	/* We had interference. Therefore, what is left
					   (if any) is completely and strictly outside
					   old element. */
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Exit c\n");
#endif
					lmt_free_clip_struct(start_old, start_new);
					return(0);
				}
				cur = cur -> next;
			}
			cur = start_new;
			while (cur -> next != start_new)
			{
				if (!cur -> used && !(lmt_IsPtInElement(cur, start_old)))
	     		{	/* The point tested is outside old element.
					   Therefore, what is left is outside. */
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Exit d\n");
#endif
					lmt_free_clip_struct(start_old, start_new);
					return(0);
				}
				cur = cur -> next;
	     	}
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Exit a\n");
#endif
			/* New element is completely and strictly
			   inside old element. Two things can happen:
			   - No interference happened at all.
			   - All the points are hard and intersected.
			   In both cases, build a hole in old element. */
			cur = start_new;
			do
			{
				list_pt = lmt_AddPtToList(list_pt, cur);
				cur = cur -> next;
			}
			while (cur != start_new);
			if (!neg_flag)
			{
#ifdef TRACE
				if (trace_lmt) p_info(PI_TRACE, "Exit e\n");
#endif
				lmt_free_clip_struct(start_old, start_new);
				return(0);
			}
			neg_ele = *neg_head;
			while (neg_ele && neg_ele -> next)
				neg_ele = neg_ele -> next;
			CreateInsertEle(neg_head, list_pt);
#ifdef TRACE
			if (trace_lmt)
			{
				p_info(PI_TRACE, "Add a negative element: ");
				neg_ele = *neg_head;
				while (neg_ele)
				{
					p_info(PI_TRACE, " %X", neg_ele);
					neg_ele = neg_ele -> next;
				}
				p_info(PI_TRACE, "\nExit f\n");
			}
#endif
			lmt_free_clip_struct(start_old, start_new);
			return(1);
		}			/* End if (cur == start_old) */
		specpt = 0;
		/*	  while (cur -> hard && !cur -> intersection && !cur -> used)
			  {
			  list_pt = lmt_AddPtToList(list_pt, cur);
			  cur = cur -> next;
			  }*/
		/*eject*/
		forward_old = forward_new = 1;
		while ((!in_old_ele && cur -> twin) ? 
			   (start_old != cur -> twin -> next &&
				start_old != cur -> twin -> prev)
			   /*
				 (forward_old) ? (start_old != cur -> twin -> next)
				 : (start_old != cur -> twin -> prev)
				 */
			   : (forward_old) ? (start_old != cur -> next)
			   : (start_old != cur -> prev))
		{
#ifdef TRACE
			if (trace_lmt)
			{
				p_info(PI_TRACE, "While do_add_pt: %d, in_old_ele: %d, cur: %#X\n",
						  do_add_pt, in_old_ele, cur);
				p_info(PI_TRACE, "\tforward_old: %d, forward_new: %d\n",
						  forward_old, forward_new);
			}
#endif
			if (do_add_pt)
				list_pt = lmt_AddPtToList(list_pt, cur);
			if (cur -> intersection)
			{
				i = check_status(cur, in_old_ele, start_old, start_new,
								 forward_old, forward_new);
				if (!i || !do_add_pt)
				{
					cur = cur -> twin;
					list_pt -> points -> arc = cur -> pt.arc;
					prev_in_old_ele = in_old_ele;
					in_old_ele = !in_old_ele;
#ifdef TRACE
					if (trace_lmt)
						p_info(PI_TRACE, "Switch level (in_old_ele): %d\n",
								  in_old_ele);
#endif
				}
				else
				{
					if (cur -> twin && in_old_ele)
						/* Even though we just hit an
						   intersection point, we are not
						   changing level. Therefore,
						   unset the used field in twin. */
						cur -> twin -> used = 0;
				}
			}
			do_add_pt = TRUE;
#ifdef TRACE
			if (trace_lmt)
				p_info(PI_TRACE, "in_old_ele: %d, analyse cur: %#X\n",
						  in_old_ele, cur);
#endif
			if (in_old_ele)
			{
				if (in_old_ele != prev_in_old_ele)
				{				/* We just changed element */
					NextUsed = cur -> next -> used;
					PrevUsed = cur -> prev -> used;
					prev_in_old_ele = in_old_ele;
				}
				else
				{
					if (forward_old)
					{
						NextUsed = 0;		/* Even if next is used,
											   use it again if needs be. */
						PrevUsed = cur -> prev -> used;
					}
					else
					{
						NextUsed = cur -> next -> used;
						PrevUsed = 0;		/* Even if prev is used,
											   use it again if needs be. */
					}
				}
				if (!NextUsed && PrevUsed)
                {
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case A\n");
#endif
					cur = cur -> next;
					forward_old = 1;
                }
				else if (!PrevUsed && NextUsed)
                {
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case B\n");
#endif
					cur = cur -> prev;
					forward_old = 0;
                }
				else if (NextUsed && PrevUsed)
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case C\n");
#endif
					destroy = 1;
					break;
				}
				/* At this point, both are not used. */
				else if (!lmt_IsPtInElement(cur -> next, start_new))
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case D\n");
#endif
					cur = cur -> next;
					forward_old = 1;
				}
				else if (!lmt_IsPtInElement(cur -> prev, start_new))
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case E\n");
#endif
					cur = cur -> prev;
					forward_old = 0;
				}
				else
				{
					CLIP_POINTS fake_cur;
					int PrevInside, NextInside;
					
					memset((char *)&fake_cur, 0, sizeof(CLIP_POINTS));
					fake_cur.pt.x = (cur -> pt.x +
									 cur -> prev -> pt.x + 1) >> 1;
					fake_cur.pt.y = (cur -> pt.y +
									 cur -> prev -> pt.y + 1) >> 1;
					PrevInside = lmt_IsPtInElement(&fake_cur, start_new);
					memset((char *)&fake_cur, 0, sizeof(CLIP_POINTS));
					fake_cur.pt.x = (cur -> pt.x +
									 cur -> next -> pt.x + 1) >> 1;
					fake_cur.pt.y = (cur -> pt.y +
									 cur -> next -> pt.y + 1) >> 1;
					NextInside = lmt_IsPtInElement(&fake_cur, start_new);
					if (PrevInside)
					{
#ifdef TRACE
						if (trace_lmt) p_info(PI_TRACE, "Case F\n");
#endif
						cur = cur -> next;
						forward_old = 1;
					}
					else if (NextInside)
					{
#ifdef TRACE
						if (trace_lmt) p_info(PI_TRACE, "Case G\n");
#endif
						cur = cur -> prev;
						forward_old = 0;
					}
					else						/* Just pick one */
					{
#ifdef TRACE
						if (trace_lmt) p_info(PI_TRACE, "Case H\n");
#endif
						cur = cur -> next;
						forward_old = 1;
					}
				}
			}
			else				/* New element */
			{
				InNext = lmt_IsPtInElement(cur -> next, start_old);
				InPrev = lmt_IsPtInElement(cur -> prev, start_old);
				iNext = cur -> next -> intersection;
				iPrev = cur -> prev -> intersection;
				if (in_old_ele != prev_in_old_ele)
				{				/* We just changed element */
					NextUsed = cur -> next -> used;
					PrevUsed = cur -> prev -> used;
					prev_in_old_ele = in_old_ele;
				}
				else
				{
					if (forward_new)
					{
						NextUsed = 0;		/* Even if next is used,
											   use it again if needs be. */
						PrevUsed = cur -> prev -> used;
					}
					else
					{
						NextUsed = cur -> next -> used;
						PrevUsed = 0;		/* Even if prev is used,
											   use it again if needs be. */
					}
				}
#ifdef TRACE
				if (trace_lmt)
					p_info(PI_TRACE, "InNext: %d, InPrev: %d, iNext: %d, \
iPrev: %d, NextUsed: %d, PrevUsed: %d\n",
							  InNext, InPrev, iNext, iPrev, NextUsed, PrevUsed);
#endif
				if (InNext && !NextUsed && InPrev && !PrevUsed)
				{					/* Both are not used.  Both are
									   inside the old element. */
					if (iNext && iPrev)
					{				/* Both are an intersection. */
#ifdef DO_NOT_USE_FOR_NOW
						if (!cur -> next -> hard
							&& !cur -> next -> twin -> hard
							&& (cur -> prev -> hard
								|| cur -> prev -> twin -> hard))
						{
#ifdef TRACE
							if (trace_lmt) p_info(PI_TRACE, "Case 1\n");
#endif
							cur = cur -> next;
							forward_new = 1;
						}
						else if (!cur -> prev -> hard
								 && !cur -> prev -> twin -> hard
								 && (cur -> next -> hard
									 || cur -> next -> twin -> hard))
						{
#ifdef TRACE
							if (trace_lmt) p_info(PI_TRACE, "Case 2\n");
#endif
							cur = cur -> prev;
							forward_new = 0;
						}
						else
#endif /* DO_NOT_USE_FOR_NOW */
							if (cur -> next -> hard
								&& !cur -> next -> twin -> hard
								&& cur -> prev -> hard
								&& cur -> prev -> twin -> hard)
							{
#ifdef TRACE
								if (trace_lmt) p_info(PI_TRACE, "Case 3\n");
#endif
								cur = cur -> next;
								forward_new = 1;
							}
							else if (cur -> prev -> hard
									 && !cur -> prev -> twin -> hard
									 && cur -> next -> hard
									 && cur -> next -> twin -> hard)
							{
#ifdef TRACE
								if (trace_lmt) p_info(PI_TRACE, "Case 4\n");
#endif
								cur = cur -> prev;
								forward_new = 0;
							}
							else
							{
/* At this point, both choices are hard for new and soft or hard
 for old.  That means that one of the new edge is parallel
 to the old. The other edge goes across inside OR outside
 the old. If it goes inside, use this edge, else use the other one. */
								CLIP_POINTS fake_cur;
								int PrevParallel, PrevAcross;
								int NextParallel, NextAcross;
								
								memset((char *)&fake_cur, 0, 
											 sizeof(CLIP_POINTS));
								if (cur -> next -> twin -> prev ==
									cur -> twin
									|| cur -> next -> twin -> next ==
									cur -> twin)
								{ /* next is the parallel one. */
									fake_cur.pt.x = (cur -> pt.x +
													 cur -> prev -> pt.x + 1)
										>> 1;
									fake_cur.pt.y = (cur -> pt.y +
													 cur -> prev -> pt.y + 1)
										>> 1;
									PrevAcross =
										lmt_IsPtInElement(&fake_cur,start_old);
									NextParallel = 1;
									NextAcross = PrevParallel = 0;
								}
								else
								{	/* prev is the parallel one. */
									fake_cur.pt.x = (cur -> pt.x +
													 cur -> next -> pt.x + 1)
										>> 1;
									fake_cur.pt.y = (cur -> pt.y +
													 cur -> next -> pt.y + 1)
										>> 1;
									NextAcross =
										lmt_IsPtInElement(&fake_cur,start_old);
									PrevParallel = 1;
									PrevAcross = NextParallel = 0;
								}
#ifdef TRACE
								if (trace_lmt)
									p_info(PI_TRACE, "PrevParallel: %d, \
PrevAcross: %d, NextParallel: %d, NextAcross: %d\n",
											  PrevParallel, PrevAcross,
											  NextParallel, NextAcross);
#endif
								if ((PrevParallel && !NextAcross) ||
									(NextParallel && PrevAcross))
								{
#ifdef TRACE
									if (trace_lmt)
										p_info(PI_TRACE, "Case 5\n");
#endif
									cur = cur -> prev;
									forward_new = 0;
								}
								else if ((NextParallel && !PrevAcross) ||
										 (PrevParallel && NextAcross))
								{
#ifdef TRACE
									if (trace_lmt) p_info(PI_TRACE, "Case 6\n");
#endif
									cur = cur -> next;
									forward_new = 1;
								}
#ifdef WE_SHOULD_NOT_NEED_THAT
								else if (cur -> next -> hard)
								{
#ifdef TRACE
									if (trace_lmt) p_info(PI_TRACE, "Case 7\n");
#endif
									cur = cur -> next;
									forward_new = 1;
								}
								else if (cur -> prev -> hard)
								{
#ifdef TRACE
									if (trace_lmt) p_info(PI_TRACE, "Case 8\n");
#endif
									cur = cur -> prev;
									forward_new = 0;
								}
#endif /* WE_SHOULD_NOT_NEED_THAT */
								else
								{
									p_info(PI_ELOG, "Impossibility number 1\n");
									p_info(PI_ELOG, "InNext: %d, InPrev: %d, \
iNext: %d, iPrev: %d, NextUsed: %d, PrevUsed: %d\n",
											  InNext, InPrev, iNext, iPrev,
											  NextUsed, PrevUsed);
									p_info(PI_ELOG, "PrevParallel: %d, \
PrevAcross: %d, PrevParallel: %d, PrevAcross: %d\n",
											  PrevParallel, PrevAcross,
											  NextParallel, NextAcross);
									if (trace_error)
										abort();
								}
							}
					}
					else if (!iNext && iPrev)
					{
#ifdef TRACE
						if (trace_lmt) p_info(PI_TRACE, "Case 3\n");
#endif
						cur = cur -> next;
						forward_new = 1;
					}
					else if (!iPrev && iNext)
					{
#ifdef TRACE
						if (trace_lmt) p_info(PI_TRACE, "Case 4\n");
#endif
						cur = cur -> prev;
						forward_new = 0;
					}
					else
					{
						p_info(PI_ELOG, "Impossibility number 2\n");
						if (trace_error)
							abort();
					}
				}
				else if (InNext && !NextUsed)
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case 5\n");
#endif
					cur = cur -> next;
					forward_new = 1;
				}
				else if (InPrev && !PrevUsed)
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case 6\n");
#endif
					cur = cur -> prev;
					forward_new = 0;
				}
				else if (InNext && !InPrev)
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case 7\n");
#endif
					cur = cur -> next;
					forward_new = 1;
				}
				else if (InPrev && !InNext)
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case 8\n");
#endif
					cur = cur -> prev;
					forward_new = 0;
				}
				else if (cur -> intersection)
				{				/* Just switch back to old element */
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case 9\n");
#endif
					do_add_pt = FALSE;
				}
				else
				{
#ifdef TRACE
					if (trace_lmt) p_info(PI_TRACE, "Case 10 (destroy)\n");
#endif
					destroy = 1;
					break;
				}
			}		/* End else if (in_old_ele) */
		}			/* End while ((!in_old_ele && cur -> twin) ? ...) */
#ifdef TRACE
		if (trace_lmt) p_info(PI_TRACE, "end while ((...\n");
#endif
		list_pt = lmt_AddPtToList(list_pt, cur);
		if (cur -> intersection)
		{
			if (cur -> twin -> hard && !in_old_ele)
			{
				if (forward_new)
					i = lmt_IsPtInElement(cur -> next, start_old);
				else
					i = lmt_IsPtInElement(cur -> prev, start_old);
			}
			else if (in_old_ele)
			{
				if (forward_old)
					if (!cur -> next -> intersection)
						i = !lmt_IsPtInElement(cur -> next, start_new);
					else
						i = 0;
				else
					if (!cur -> prev -> intersection)
						i = !lmt_IsPtInElement(cur -> prev, start_new);
					else
						i = 0;
			}
			else
				i = 0;
			if (i)
			{
				if (cur -> twin && in_old_ele)
					/* Even though we just hit an intersect point, we are not
					   changing level. Thus, unset the used field in twin. */
					cur -> twin -> used = 0;
			}
		}
		if (destroy)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Destroy\n");
#endif
			p_free((char *)list_pt -> points);
			p_free((char *)list_pt);
			list_pt = NULL;
		}
		else
		{
			CreateInsertEle(&old_ele, list_pt);
			list_pt = NULL;
		}
	}			/* End for (;;) */
	/* The next comment is bogus and is for the benefit of CodeCenter */
	/*NOTREACHED*/
#ifdef TRACE
	if (trace_lmt) p_info(PI_TRACE, "Exit b\n");
#endif
	if (old_ele -> next)
		old_ele -> next -> prev = old_ele -> prev;
	if (old_ele -> prev)
		old_ele -> prev -> next = old_ele -> next;
	else
		*head = old_ele -> next;
#ifdef TRACE
	if (trace_lmt)
	{
		ELEMENT *ele;
		
		ele = old_ele;
		while (ele)
		{
			for (i = 0; i < ele -> n_points; i++)
			{
				p_info(PI_TRACE, "i: %d x: %d, y: %d   ",
						  i, ele -> list_points[i].x, ele -> list_points[i].y);
				if (!(i % 5))
					p_info(PI_TRACE, "\n");
			}
			ele = ele -> next;
			if (i % 5)
				p_info(PI_TRACE, "\n");
		}
		ele = new_ele;
		while (ele)
		{
			for (i = 0; i < new_ele -> n_points; i++)
			{
    			p_info(PI_TRACE, "i: %d x: %d, y: %d   ", 
						  i, new_ele -> list_points[i].x,
						  old_ele -> list_points[i].y);
				if (!(i % 5))
					p_info(PI_TRACE, "\n");
			}
			ele = ele -> next;
			if (i % 5)
				p_info(PI_TRACE, "\n");
		}
		p_info(PI_TRACE, "\n\n");
	}
#endif
	clean_ele(old_ele);
	lmt_free_clip_struct(start_old, start_new);
	return(0);
}

#ifdef TRACE
void trace_start_ele(CLIP_POINTS *start_old, CLIP_POINTS *start_new)
{
	CLIP_POINTS *cur;
	
	if (!trace_lmt)
		return;
	p_info(PI_TRACE, "\nStart old\n");
	cur = start_old;
	do
	{
    	p_info(PI_TRACE, "cur: %X, x: %d, y: %d, \
prev: %X, next: %X, twin: %X\n",
				  cur, cur -> pt.x, cur -> pt.y, cur -> prev, cur -> next,
				  cur -> twin);
		p_info(PI_TRACE, "         hard: %d, intersection: %d\n",
				  cur -> hard, cur -> intersection);
		cur = cur -> next;
	} while (cur != start_old);
	p_info(PI_TRACE, "\nStart new\n");
	cur = start_new;
	do
	{
    	p_info(PI_TRACE, "cur: %X, x: %d, y: %d, \
prev: %X, next: %X, twin: %X\n",
				  cur, cur -> pt.x, cur -> pt.y, cur -> prev, cur -> next,
				  cur -> twin);
		p_info(PI_TRACE, "         hard: %d, intersection: %d\n",
				  cur -> hard, cur -> intersection);
    	cur = cur -> next;
	} while (cur != start_new);
	p_info(PI_TRACE, "\n");
}
#endif
