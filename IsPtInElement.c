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
#include "X11_typedefs.h"
#include "interfer.h"
#include "traces.h"

#define PIX_CHUNCK	10

extern Region lmt_XPolygonRegion(lmt_XPoint *Pts, int Count, int rule);
extern int lmt_XPointInRegion(Region pRegion, int x, int y);
extern void lmt_XDestroyRegion(Region r);

int lmt_IsPtInElement(cur, start)
CLIP_POINTS *cur, *start;
{					/* This one call lmt_ function because they work
					   w/ long integer */
	lmt_XPoint pixel, *pix_list;
	CLIP_POINTS *cur_pt, *stop_pt;
	int ii, ret_val;
	Region region;
	
#ifdef TRACE
	if (trace_lmt)
		p_info(PI_TRACE, "lmt_IsPtInElement: x: %d, y: %d, start: %X\n",
				  cur -> pt.x, cur -> pt.y, start);
#endif
	if (cur -> inside)
	{
#ifdef TRACE
		if (trace_lmt)
			p_info(PI_TRACE, "%s\n", (cur -> inside - 1) ? ("YES5") : ("NO5"));
#endif
		return(cur -> inside - 1);
	}
	if (cur -> intersection)
	{
		cur -> inside = 2;
#ifdef TRACE
		if (trace_lmt)
			p_info(PI_TRACE, "%s\n", "YES0");
#endif
		return(cur -> inside - 1);
	}
	pixel.x = cur->pt.x;
	pixel.y = cur->pt.y;
	pix_list = (lmt_XPoint *)p_alloc(sizeof(lmt_XPoint) * PIX_CHUNCK);
	cur_pt = start;
	do
		cur_pt = cur_pt -> next;
	while (!cur_pt -> hard);
	stop_pt = cur_pt;
	ii = 0;
	do
	{
		if (ii && !(ii % PIX_CHUNCK))
			pix_list = (lmt_XPoint *) 
				p_remalloc((char *)pix_list, sizeof(lmt_XPoint) * ii,
						   sizeof(lmt_XPoint) * (ii + PIX_CHUNCK));
		pix_list[ii].x = cur_pt -> pt.x;
		pix_list[ii].y = cur_pt -> pt.y;
		do
			cur_pt = cur_pt -> next;
		while (!cur_pt -> hard);
		ii++;
	} while (cur_pt != stop_pt);
	region = lmt_XPolygonRegion(pix_list, ii, WindingRule);
	p_free((char *)pix_list);
	ret_val = lmt_XPointInRegion(region, pixel.x, pixel.y);
	lmt_XDestroyRegion(region);
#ifdef TRACE
	if (trace_lmt || trace_misc)
		p_info(PI_TRACE, "lmt_IsPtInElement: %d\n", ret_val);
#endif
	cur -> inside = ret_val + 1;
	return(ret_val);
}
