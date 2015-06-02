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
#include <memory.h>
#include "p_lib.h"
#include "interfer.h"
#include "traces.h"

static CLIP_POINTS *FindSpot(x, y, point)
int32 x, y;
CLIP_POINTS *point;
{
	CLIP_POINTS *new_pt;
#ifdef IF_YOU_NEED_IT_USE_IT
	CLIP_POINTS *keep;
	
	keep = point;
	do
	{
		p_info(PI_TRACE, "point: %#X, point -> pt.x: %d, point -> pt.y: %d\n",
				  point, point -> pt.x, point -> pt.y);
		point = point -> next;
	}
	while (keep != point);
	point = keep;
#endif
	if (point -> pt.x < point -> next -> pt.x)
		while (x > point -> next -> pt.x)
			point = point -> next;
	else if (point -> pt.x > point -> next -> pt.x)
		while (x < point -> next -> pt.x)
			point = point -> next;
	if (x == point -> pt.x)
	{
		if (point -> pt.y < point -> next -> pt.y)
			while (y > point -> next -> pt.y
				   && x == point -> next -> pt.x)
				point = point -> next;
		else if (point -> pt.y > point -> next -> pt.y)
			while (y < point -> next -> pt.y
				   && x == point -> next -> pt.x)
				point = point -> next;
	}
#ifdef IF_YOU_NEED_IT_USE_IT
	p_info(PI_TRACE, "Point choosen: %#X\n", point);
#endif
	if ((x == point -> pt.x && y == point -> pt.y) ||
		(x == point -> prev -> pt.x && y == point -> prev -> pt.y) ||
		(x == point -> next -> pt.x && y == point -> next -> pt.y))
		return(0);
	/* Setup new point */
	new_pt = (CLIP_POINTS *)p_alloc(sizeof(CLIP_POINTS));
	new_pt -> pt.x = x;
	new_pt -> pt.y = y;
	new_pt -> prev = point;
	new_pt -> next = point -> next;
	point -> next = new_pt;
	new_pt -> next -> prev = new_pt;
	if (new_pt -> prev -> pt.arc && new_pt -> next -> pt.arc)
	{
		p_info(PI_TRACE, "Make an arc out of it\n");
		new_pt -> pt.arc = (P_ARC *)p_alloc(sizeof(P_ARC));
		memcpy((char *)new_pt -> pt.arc,
			   (char *)new_pt -> prev -> pt.arc, sizeof(P_ARC));
	}
	return(new_pt);
}

void lmt_AddIntersection(x, y, old_pt, new_pt, type_old, type_new)
int32 x, y;
CLIP_POINTS *old_pt, *new_pt;
int type_old, type_new;
{
	CLIP_POINTS *new_old_pt, *new_new_pt;
	
	/*
	  Inserts (x, y) after the right old_pt and after the right new_pt.
	  Find the right place for insertion.
	  */
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "++Addintersec: x: %d, y: %d, old_pt: %X, new_pt: %X, \
type_old: %d, type_new: %d\n", x, y, old_pt, new_pt, type_old,type_new);
#endif
	if (type_old)
		new_old_pt = old_pt;
	else
	{			/* Inside the old element. */
		new_old_pt = FindSpot(x, y, old_pt);
	}
	if (type_new)
		new_new_pt = new_pt;
	else
	{			/* Inside the new element. */
		new_new_pt = FindSpot(x, y, new_pt);
	}
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "new_old_pt: %#X, new_new_pt: %#X\n",
				  new_old_pt, new_new_pt);
#endif
	/* Setup twins and intersection for both */
	if (new_old_pt && new_new_pt)
	{
		new_old_pt -> intersection = 1;
		new_new_pt -> intersection = 1;
		new_old_pt -> twin = new_new_pt;
		new_new_pt -> twin = new_old_pt;
	}
}
