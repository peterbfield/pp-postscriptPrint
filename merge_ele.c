/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include "window.h"
#include "interfer.h"
#include "clip.f"
#include "traces.h"
#include "lmt.f"

extern void lmt_free_clip_struct(CLIP_POINTS *start_old, 
								 CLIP_POINTS *start_new);
extern int lmt_IsPtInElement(CLIP_POINTS *cur, CLIP_POINTS *start);
extern void trace_start_ele(CLIP_POINTS *start_old, CLIP_POINTS *start_new);

ELEMENT *merge_neg_ele(ELEMENT *old_ele, ELEMENT *neg_ele);

static ELEMENT *merge_2_elements(ELEMENT *old_ele, ELEMENT *new_ele);

ELEMENT *CreateElement(LIST_PT *list_pt)
{
    ELEMENT *new_ele;

    new_ele = (ELEMENT *)p_alloc(sizeof(ELEMENT));
    new_ele -> n_points = list_pt -> pts_used;
    new_ele -> list_points = list_pt -> points;
    new_ele -> rot_rect_top = list_pt -> min_y;
    new_ele -> rot_rect_bottom = list_pt -> max_y;
    new_ele -> rot_rect_left = list_pt -> min_x;
    new_ele -> rot_rect_right = list_pt -> max_x;
    p_free((char *)list_pt);
#ifdef TRACE
	if (trace_lmt)
	{
		int i;

		for (i = 0; i < new_ele -> n_points; i++)
		{
			p_info(PI_TRACE, "i: %d x: %d, y: %d   ", i, new_ele -> list_points[i].x,
					  new_ele -> list_points[i].y);
			if (!(i % 5))
				p_info(PI_TRACE, "\n");
		}
		if (i % 5)
			p_info(PI_TRACE, "\n");
	}
#endif
	return(new_ele);
}

void InsertElement(ELEMENT **head, ELEMENT *new_ele)
{
	ELEMENT *prev_ele, *ele;

	if (*head)
	{
		prev_ele = *head;
		ele = (*head) -> next;
		while (ele && ele -> rot_rect_left < new_ele -> rot_rect_left)
		{
			prev_ele = ele;
			ele = ele -> next;
		}
		while (ele && ele -> rot_rect_left == new_ele -> rot_rect_left
			   && ele -> rot_rect_top < new_ele -> rot_rect_top)
		{
			prev_ele = ele;
			ele = ele -> next;
		}
		if (ele)
		{						/* Insert before */
			new_ele -> next = ele;
			new_ele -> prev = ele -> prev;
			ele -> prev = new_ele;
			new_ele -> prev -> next = new_ele;
		}
		else
		{						/* Insert after prev_ele */
			new_ele -> prev = prev_ele;
			prev_ele -> next = new_ele;
		}
	}
	else
	{
		*head = new_ele;
		new_ele -> prev = new_ele -> next = NULL;
	}
}

static ELEMENT *merge_2_elements(ELEMENT *old_ele, ELEMENT *new_ele)
{
	CLIP_POINTS *cur, *start_old, *start_new, *start;
	int *forward, forward_old, forward_new, in_old_ele;
	BOOLEAN intersect = FALSE;
	LIST_PT *list_pt = 0;

#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "merge_2_elements\n");
#endif
    start_old = lmt_BuildClipPoints(old_ele-> list_points, old_ele-> n_points);
    start_new = lmt_BuildClipPoints(new_ele-> list_points, new_ele-> n_points);
#ifdef TRACE
	trace_start_ele(start_old, start_new);
#endif
	lmt_BuildIntersections(start_old, start_new);
#ifdef TRACE
	trace_start_ele(start_old, start_new);
#endif
	cur = start_new;
	do
	{
		if (cur -> twin)
		{
			intersect = TRUE;	/* We do have intersection */
			break;
		}
		cur = cur -> next;
	} while (cur != start_new);
	if (!intersect)
	{
		lmt_free_clip_struct(start_old, start_new);
		return(NULL);
	}
	cur = start_old;
	do
	{
		if (!cur -> twin)
			break;
		cur = cur -> next;
	} while (cur != start_old);
	start_old= cur;
	list_pt = lmt_AddPtToList(list_pt, cur);
	in_old_ele = 1;
	forward_old = forward_new = 1;
	cur = cur -> next;
	/*
	  This cur is the starting point of a new old-element.
	  This point is a hard and/or intersection, unused point.
	  Use it as the new start.
	  */
	while (cur != start_old)
	{
#ifdef TRACE
		if (trace_lmt)
			p_info(PI_TRACE, "While in_old_ele: %d, cur: %X\n", in_old_ele, cur);
#endif
		list_pt = lmt_AddPtToList(list_pt, cur);
		if (!cur -> intersection)
		{
			if (in_old_ele)
				cur = forward_old ? cur -> next : cur -> prev;
			else
				cur = forward_new ? cur -> next : cur -> prev;
			continue;
		}
		cur = cur -> twin;
		in_old_ele = !in_old_ele;
		if (in_old_ele)
		{
			forward = &forward_old;
			start = start_new;
		}
		else
		{
			forward = &forward_new;
			start = start_old;
		}
		if (!cur -> next -> used &&
			!lmt_IsPtInElement(cur -> next, start))
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case C\n");
#endif
			cur = cur -> next;
			*forward = 1;
		}
		else if (!cur -> prev -> used &&
				 !lmt_IsPtInElement(cur -> prev, start))
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case D\n");
#endif
			cur = cur -> prev;
			*forward = 0;
		}
        else if (cur -> next -> intersection && !cur -> next -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case E\n");
#endif
            cur = cur -> next;
			*forward = 1;
		}
        else if (cur -> prev -> intersection && !cur -> prev -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case F\n");
#endif
            cur = cur -> prev;
			*forward = 0;
		}
        else if (!cur -> next -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case G\n");
#endif
            cur = cur -> next;
			*forward = 1;
		}
        else if (!cur -> prev -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case H\n");
#endif
            cur = cur -> prev;
			*forward = 0;
		}
		else
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case I ");
#endif
			if (in_old_ele && (cur -> next == start_old
							   || cur -> prev == start_old))
			{		/* We are done, get out */
#ifdef TRACE
				if (trace_lmt) p_info(PI_TRACE, "ok\n");
#endif
				break;
			}
			else if (!in_old_ele && cur -> twin
					 && (cur -> twin -> next == start_old
						 || cur -> twin -> prev == start_old))
			{		/* We are done, get out */
#ifdef TRACE
				if (trace_lmt) p_info(PI_TRACE, "ok\n");
#endif
				break;
			}
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "impossible\n");
#endif
			break;
		}
	}			/* End while ((!in_old_ele && cur -> twin) ? ...) */
#ifdef TRACE
	if (trace_lmt) p_info(PI_TRACE, "end while ((...\n");
#endif
	lmt_free_clip_struct(start_old, start_new);
	return(CreateElement(list_pt));
}

ELEMENT *lmt_MergeElements(ELEMENT *head)
{
	ELEMENT *ele1, *ele2, *new_ele;
	
#ifdef TRACE
	if (trace_lmt)
	{
		p_info(PI_TRACE, "lmt_MergeElements\n");
		ele1 = head;
		p_info(PI_TRACE, "Before List -> ");
		while (ele1)
		{
			p_info(PI_TRACE, " %X", ele1);
			ele1 = ele1 -> next;
		}
		p_info(PI_TRACE, "\n");
	}
#endif
	ele1 = ele2 = head;
	while (ele1)
	{
		ele2 = ele1 -> next;
		while (ele2)
		{
#ifdef TRACE
			if (trace_lmt)
				p_info(PI_TRACE, "Check 2 elements-> Ele1: %X, Ele2: %X\n", ele1, ele2);
#endif
			if (!(ele1 -> rot_rect_top >= ele2 -> rot_rect_bottom ||
				  ele1 -> rot_rect_bottom <= ele2 -> rot_rect_top ||
				  ele1 -> rot_rect_left >= ele2 -> rot_rect_right ||
				  ele1 -> rot_rect_right <= ele2 -> rot_rect_left))
			{
#ifdef TRACE
				if (trace_lmt)
					p_info(PI_TRACE, "Merge those 2 elements\n");
#endif
				if ((new_ele = merge_2_elements(ele1, ele2)) != NULL)
				{		/* We did merge them! */
#ifdef TRACE
					if (trace_lmt)
						p_info(PI_TRACE, "We do have a merge!!!\n");
#endif
					if (ele1 -> prev)
						ele1 -> prev -> next = new_ele;
					new_ele -> prev = ele1 -> prev;
					if (ele1 -> next == ele2)
					{		/* then ele2 -> prev == ele1 */
						new_ele -> next = ele2 -> next;
						if (ele2 -> next)
							ele2 -> next -> prev = new_ele;
					}
					else
					{
						ele1 -> next -> prev = new_ele;
						new_ele -> next = ele1 -> next;
						ele2 -> prev -> next = ele2 -> next;
						if (ele2 -> next)
							ele2 -> next -> prev = ele2 -> prev;
					}
					if (head == ele1)
						head = new_ele;
					p_free((char *)ele1 -> list_points);
					p_free((char *)ele1);
					p_free((char *)ele2 -> list_points);
					p_free((char *)ele2);
					ele1 = ele2 = new_ele;
				}
			}
			ele2 = ele2 -> next;
		}
		ele1 = ele1 -> next;
	}
#ifdef TRACE
	if (trace_lmt)
	{
		ele1 = head;
		p_info(PI_TRACE, "After List -> ");
		while (ele1)
		{
			p_info(PI_TRACE, " %X", ele1);
			ele1 = ele1 -> next;
		}
		p_info(PI_TRACE, "\n");
	}
#endif
	return(head);
}

ELEMENT *merge_neg_ele(ELEMENT *old_ele, ELEMENT *neg_ele)
{
	CLIP_POINTS *cur, *start_old, *start_neg;
	int *forward, forward_old, forward_neg, in_old_ele;
	BOOLEAN intersect = FALSE;
	LIST_PT *list_pt = 0;

#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "merge_neg_ele\n");
#endif
    start_old = lmt_BuildClipPoints(old_ele -> list_points,
									old_ele -> n_points);
    start_neg = lmt_BuildClipPoints(neg_ele -> list_points,
									neg_ele -> n_points);
#ifdef TRACE
	trace_start_ele(start_old, start_neg);
#endif
	lmt_BuildIntersections(start_old, start_neg);
#ifdef TRACE
	trace_start_ele(start_old, start_neg);
#endif
	cur = start_neg;
	do
	{
		if (cur -> twin)
		{
			intersect = TRUE;	/* We do have intersection */
			break;
		}
		cur = cur -> next;
	} while (cur != start_neg);
	if (!intersect)
	{
		lmt_free_clip_struct(start_old, start_neg);
		return(NULL);
	}
	cur = start_old;
	do
	{
		if (!cur -> twin)
			break;
		cur = cur -> next;
	} while (cur != start_old);
	start_old= cur;
	list_pt = lmt_AddPtToList(list_pt, cur);
	in_old_ele = 1;
	forward_old = forward_neg = 1;
	cur = cur -> next;
	/*
	  This cur is the starting point of a neg old-element.
	  This point is a hard and/or intersection, unused point.
	  Use it as the neg start.
	  */
	while (cur != start_old)
	{
#ifdef TRACE
		if (trace_lmt)
			p_info(PI_TRACE, "While in_old_ele: %d, cur: %X\n", in_old_ele, cur);
#endif
		list_pt = lmt_AddPtToList(list_pt, cur);
		if (!cur -> intersection)
		{
			if (in_old_ele)
				cur = forward_old ? cur -> next : cur -> prev;
			else
				cur = forward_neg ? cur -> next : cur -> prev;
			continue;
		}
		cur = cur -> twin;
		in_old_ele = !in_old_ele;
		if (in_old_ele)
			forward = &forward_old;
		else
			forward = &forward_neg;
        if (!cur -> next -> intersection && !cur -> next -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case E\n");
#endif
            cur = cur -> next;
			*forward = 1;
		}
        else if (!cur -> prev -> intersection && !cur -> prev -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case F\n");
#endif
            cur = cur -> prev;
			*forward = 0;
		}
        else if (cur -> next -> hard && !cur -> next -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case E\n");
#endif
            cur = cur -> next;
			*forward = 1;
		}
        else if (cur -> prev -> hard && !cur -> prev -> used)
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case F\n");
#endif
            cur = cur -> prev;
			*forward = 0;
		}
		else
		{
#ifdef TRACE
			if (trace_lmt) p_info(PI_TRACE, "Case G impossible\n");
#endif
			break;
		}
	}			/* End while ((!in_old_ele && cur -> twin) ? ...) */
#ifdef TRACE
	if (trace_lmt) p_info(PI_TRACE, "end while ((...\n");
#endif
	lmt_free_clip_struct(start_old, start_neg);
	return(CreateElement(list_pt));
}
