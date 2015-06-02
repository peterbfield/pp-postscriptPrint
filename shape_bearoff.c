/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */


/**********************************************************************
 *      This is a routine that expands the outline of a graphic       *
 *      stored in the shape_pts of the FRAME structure, by the        *
 *      bearoffs from the rel-data of the frame, and puts the         *
 *      result in the bearoff_pts of the FRAME structure.             *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "window.h"
#include "p_lib.h"
#include "traces.h"
#include "rel_data.h"
#include "frame.h"
#include "lmt.f"
#include "px_header.h"
#include "gs_header.h"

typedef struct {
	QE que;
	int y;						/* vertical position of scan_line */
	int on;						/* start of black */
	int off;					/* end of black */
	int used;					/* flags for points used in new image */
} Swath;

typedef struct {
	QE que;
	int x;
	int y;
} Point;

int cr_shapes_fr_pix(int *px2_sh_recs, PXL_SHAPE **px2_shapes, char *px, char *data);
static void smoothing_scan();
static void smooth_out_points(Point *point1, Point *point2);
static void smooth_final(Point *p1, Point *p2);
static void plot_point(int,int);
static void track_point(int,int,int);
static void make_edge(int,int,int);
static void make_swaths();
static void spread_swaths(int,int,int,int);
static void merge_swath(int,int,int,Swath *);
static void follow_the_shape();
static void plot_x_y(int,int,int);
static void dump_points(WYSIWYG *,int);
static void dump_trap_points(ELEMENT *);


static QUEUE points = {0,0};
static int num_points = 0;
static QUEUE work = {0,0};
static int last_dy;
static int last_y;
static int last_x1;
static int last_x2;
static int first_edge;
static int curx = 0;
static int cury = 0;

int file_type;

int shape_bearoff(WYSIWYG *wn, int num, hmu msb, vmu ldb)
{
	int i;
	int top,bottom,left,right;
	int x,y;

	if(FRAME_FLAGS(num) & BEAROFF_SHAPE && FRAME_DATA(num) num_bearoffs)
		return(0);
	top = (lmt_off_to_abs(wn, 0, TOP_GUTTER(num)) 
		   + (wn -> ldb >> 1)) / wn -> ldb;
	bottom = (lmt_off_to_abs(wn, 0, BOT_GUTTER(num))
			  + (wn -> ldb >> 1)) / wn -> ldb;
	left = (lmt_off_to_abs(wn, 1, LEFT_GUTTER(num))
			+ (wn -> msb >> 1)) / wn -> msb;
	right = (lmt_off_to_abs(wn, 1, RIGHT_GUTTER(num))
			 + (wn -> msb >> 1)) / wn -> msb;
	i = (top << 18) + (bottom << 12) + (left << 6) + right;
	if (FRAME_DATA(num) bearoff_pts != 0 && FRAME_DATA(num) last_bearoffs == i)
		return(0);				/* same as before! */
	if (trace_misc)
		p_info(PI_TRACE, "spread %d %d %d %d\n",top,bottom,left,right);
	if(FRAME_DATA(num) bearoff_pts)
	{
		p_free((char *)FRAME_DATA(num) bearoff_pts);
		FRAME_DATA(num) bearoff_pts = NULL;
		FRAME_DATA(num) num_bearoffs = 0;
	}
	FRAME_DATA(num) last_bearoffs = i;
	if (!i)			/* No gutter */
	{
		i = sizeof(DRAW_POINT_X_Y) * FRAME_DATA(num) num_shapes;
		FRAME_DATA(num) num_bearoffs = FRAME_DATA(num) num_shapes;
		FRAME_DATA(num) bearoff_pts = (DRAW_POINT_X_Y *)p_alloc(i);
		memcpy((char *)FRAME_DATA(num) bearoff_pts,
			   (char *)FRAME_DATA(num) shape_pts, i);
		return(1);
	}
	/*
	 *	Set up for initial plot of outline
	 */
	x = FRAME_DATA(num) shape_pts[0].x;
	y = FRAME_DATA(num) shape_pts[0].y;
	if(msb != wn -> msb)
		x = ((x * msb) + (wn -> msb / 2)) / wn -> msb; /* = points */
	if(ldb != wn -> ldb)
		y = ((y * ldb) + (wn -> ldb / 2)) / wn -> ldb; /* = points */
	curx = x;
	cury = y;
	last_dy = -1;
	last_y = cury;
	last_x1 = curx;
	last_x2 = curx;
	first_edge = -1;
	/*
	 *	Plot outline marking each minimun and maximun y vertex twice
	 */
	for(i = 0; i < FRAME_DATA(num) num_shapes; i++)
	{
		x = FRAME_DATA(num) shape_pts[i].x;
		y = FRAME_DATA(num) shape_pts[i].y;
		if(msb != wn -> msb)
			x = ((x * msb) + (wn -> msb / 2)) / wn -> msb; /* = points */
		if(ldb != wn -> ldb)
			y = ((y * ldb) + (wn -> ldb / 2)) / wn -> ldb; /* = points */
		plot_point(x,y);		/* plot all points */
	}
	x = FRAME_DATA(num) shape_pts[0].x;
	y = FRAME_DATA(num) shape_pts[0].y;
	if(msb != wn -> msb)
		x = ((x * msb) + (wn -> msb / 2)) / wn -> msb; /* = points */
	if(ldb != wn -> ldb)
		y = ((y * ldb) + (wn -> ldb / 2)) / wn -> ldb; /* = points */
	plot_point(x,y);			/* re-plot last point */
	make_edge(last_y,last_x1,last_x2); /* flag last point */
	/*
	 *	convert edges into swaths, ignore special last point if odd
	 */
	make_swaths();
	/*
	 *	perform actual bearoff
	 */
	spread_swaths(top,bottom,left,right);
	/*
	 *	trace along edges of swaths to create que of outline points
	 */
	follow_the_shape();
	/*
	 *	dump compacted edge outline into frame structure
	 */
	dump_points(wn,num);
	/*
	 *	clean-up and return
	 */
	if (trace_misc)
		p_info(PI_TRACE, "frame %d, %d points\n",
				  num,FRAME_DATA(num) num_bearoffs);
	curx = 0;
	cury = 0;
	if(trace_misc)
	{
		for(i = 0; i < FRAME_DATA(num) num_bearoffs; i++)
		{
			x = FRAME_DATA(num) bearoff_pts[i].x;
			y = FRAME_DATA(num) bearoff_pts[i].y;
			p_info(PI_TRACE, "%2d x =%4d,  y =%4d  dx/dy=%4d/%4d\n",
					  i,x,y,curx-x,cury-y);
			curx = x;
			cury = y;
		}
	}
	if(msb != wn -> msb || ldb != wn -> ldb)
	{
		for(i = 0; i < FRAME_DATA(num) num_bearoffs; i++)
		{
			x = FRAME_DATA(num) bearoff_pts[i].x;
			y = FRAME_DATA(num) bearoff_pts[i].y;
			x = ((x * wn -> msb) + (msb >> 1)) / msb;
			y = ((y * wn -> ldb) + (ldb >> 1)) / ldb;
			FRAME_DATA(num) bearoff_pts[i].x = x;
			FRAME_DATA(num) bearoff_pts[i].y = y;
		}
	}
	Qclear(&work);
	Qclear(&points);
	return(1);
}

static BOOLEAN IsItARectangle(ELEMENT *ele)
{
	if ((ele -> list_points[0].x == ele -> list_points[3].x
		 && ele -> list_points[1].x == ele -> list_points[2].x
		 && ele -> list_points[0].y == ele -> list_points[1].y
		 && ele -> list_points[2].y == ele -> list_points[3].y)
		||(ele -> list_points[0].x == ele -> list_points[1].x
		   && ele -> list_points[2].x == ele -> list_points[3].x
		   && ele -> list_points[0].y == ele -> list_points[3].y
		   && ele -> list_points[1].y == ele -> list_points[2].y))
		return(TRUE);
	return(FALSE);
}

int trap_bearoff(WYSIWYG *wn, int num, hmu msb, vmu ldb, ELEMENT *ele)
{
	int i;
	int top,bottom,left,right;
	int x,y,weight;
	int32 h_weight, v_weight, sv;
	DRAW_POINT_X_Y *ele_list;
	if (ele -> trap_points)
	{
		p_free((char *)ele -> trap_points);
		ele -> trap_points = NULL;
		ele -> trap_pts = 0;
	}
	
	if (!(FRAME_FLAGS(num) & OUTLINE))
		return(0);
	ele_list = ele -> list_points;
	sv = wn -> yx_convert[X_REF];
	wn -> yx_convert[X_REF] = 270; /* 20ths of a point */
	weight = lmt_off_to_abs(wn, X_REF, OUT_WEIGHT(num));
	wn -> yx_convert[X_REF] = sv;
	if (ele -> n_points == 4
		&& !(FRAME_FLAGS(ele -> map_data.frame_num) & POLYGON_SHAPE)
		&& !(FRAME_FLAGS(ele -> map_data.frame_num) & OVALE_SHAPE)
		&& IsItARectangle(ele))
	{							/* Element is a rectangle */
		top = lmt_off_to_abs(wn, Y_REF, OUT_TRAP(num));
		left = lmt_off_to_abs(wn, X_REF, OUT_TRAP(num));
		v_weight = (((weight * wn -> ldb) + 10L) >> 1) / 20L;
		h_weight = (((weight * wn -> msb) + 10L) >> 1) / 20L;
		if(top == v_weight)
			return(0);
		bottom = top = v_weight - top;
		right = left = h_weight - left;
		ele -> trap_pts = 4;
		ele -> trap_points =
			(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * 4);
		if(ele->trap_points)
		{
			ele->trap_points[0].x = (MIN(ele_list[2].x,ele_list[0].x) - left);
			ele->trap_points[0].y = (MIN(ele_list[2].y,ele_list[0].y) - top);
			ele->trap_points[1].x = (MAX(ele_list[3].x,ele_list[1].x) + right);
			ele->trap_points[1].y = (MIN(ele_list[3].y,ele_list[1].y) - top);
			ele->trap_points[2].x = (MAX(ele_list[2].x,ele_list[0].x) + right); 
			ele->trap_points[2].y = (MAX(ele_list[2].y,ele_list[0].y) + bottom);
			ele->trap_points[3].x = (MIN(ele_list[3].x,ele_list[1].x) - left);
			ele->trap_points[3].y = (MAX(ele_list[3].y,ele_list[1].y) + bottom);
		}
		return(0);
	}
	top = (lmt_off_to_abs(wn, Y_REF, OUT_TRAP(num))
		   + (wn -> ldb >> 1)) / wn -> ldb;
	left = (lmt_off_to_abs(wn, X_REF, OUT_TRAP(num))
			+ (wn -> msb >> 1)) / wn -> msb;
	v_weight = (((weight * wn -> ldb) + 10L) >> 1) / (20L * wn -> ldb);
	h_weight = (((weight * wn -> msb) + 10L) >> 1) / (20L * wn -> msb);
	if(top == v_weight)
		return(0);
	bottom = top = v_weight - top;
	right = left = h_weight - left;
	if (trace_misc)
		p_info(PI_TRACE, "spread %d %d %d %d\n",top,bottom,left,right);
	x = ele_list[0].x;
	y = ele_list[0].y;
	if(msb != wn -> msb)
		x = ((x * msb) + (wn -> msb / 2)) / wn -> msb; /* = points */
	if(ldb != wn -> ldb)
		y = ((y * ldb) + (wn -> ldb / 2)) / wn -> ldb; /* = points */
	curx = x;
	cury = y;
	last_dy = -1;
	last_y = cury;
	last_x1 = curx;
	last_x2 = curx;
	first_edge = -1;
	/*
	 *	Plot outline marking each minimun and maximun y vertex twice
	 */
	for(i = 0; i < ele -> n_points; i++)
	{
		x = ele_list[i].x;
		y = ele_list[i].y;
		if(msb != wn -> msb)
			x = ((x * msb) + (wn -> msb / 2)) / wn -> msb; /* = points */
		if(ldb != wn -> ldb)
			y = ((y * ldb) + (wn -> ldb / 2)) / wn -> ldb; /* = points */
		plot_point(x,y);		/* plot all points */
	}
	x = ele_list[0].x;			/*	ele -> list_points[0].x; */
	y = ele_list[0].y;			/*	ele -> list_points[0].y; */
	if(msb != wn -> msb)
		x = ((x * msb) + (wn -> msb / 2)) / wn -> msb; /* = points */
	if(ldb != wn -> ldb)
		y = ((y * ldb) + (wn -> ldb / 2)) / wn -> ldb; /* = points */
	
	plot_point(x,y);			/* re-plot last point */
	make_edge(last_y,last_x1,last_x2); /* flag last point */
	/*
	 *	convert edges into swaths, ignore special last point if odd
	 */
	make_swaths();
	/*
	 *	perform actual bearoff
	 */
	spread_swaths(top, bottom, left, right);
	/*
	 *	trace along edges of swaths to create que of outline points
	 */
	follow_the_shape();
	/*
	 *	dump compacted edge outline into frame structure
	 */
	dump_trap_points(ele);
	/*
	 *	clean-up and return
	 */
	if (trace_misc)
		p_info(PI_TRACE, "frame %d, %d points\n", num, ele -> trap_pts);
	curx = 0;
	cury = 0;
	if(trace_misc)
	{
		for(i = 0; i < ele -> trap_pts; i++)
		{
			x = ele -> trap_points[i].x;
			y = ele -> trap_points[i].y;
			p_info(PI_TRACE, "%2d x =%4d,  y =%4d  dx/dy=%4d/%4d\n",
					  i,x,y,curx-x,cury-y);
			curx = x;
			cury = y;
		}
	}
	if(msb != wn -> msb || ldb != wn -> ldb)
	{
		for(i = 0; i < ele -> trap_pts; i++)
		{
			x = ele -> trap_points[i].x;
			y = ele -> trap_points[i].y;
			x = ((x * wn -> msb) + (msb >> 1)) / msb;
			y = ((y * wn -> ldb) + (ldb >> 1)) / ldb;
			ele -> trap_points[i].x = x;
			ele -> trap_points[i].y = y;
		}
	}
	Qclear(&work);
	Qclear(&points);
	return(1);
}

static void plot_point(int x, int y)
{
	int xcnt,ycnt;				/* distance to travel  */
	int dx,dy;					/* direction of travel */
	int cnt,i,x1,x2;

	if(trace_misc)
		p_info(PI_TRACE, "plot point %d %d\n",x,y);
	xcnt = x - curx;			/* set up direction & amt */
	ycnt = y - cury;
	dx = dy = 1;
	if(xcnt < 0)
	{
		xcnt = -xcnt;
		dx = -1;
	}
	if(ycnt < 0)
	{
		ycnt = -ycnt;
		dy = -1;
	}
	/* always increment along the y axis */
	cnt = ycnt/2;				/* count down for diag move */
	for(i=0;i<ycnt;i++)
	{
		cnt -= xcnt;
		cury += dy;
		if(cnt < 0)
		{
			cnt += ycnt;
			curx += dx;
		}
		x1 = curx;
		while(cnt < 0)
		{
			cnt += ycnt;
			curx += dx;
		}
		x2 = curx;
		track_point(cury,x1,x2); /* plot each point on the line*/
	}							/* all vectors drawn */
	if(ycnt == 0 && xcnt != 0)
	{
		track_point(cury,curx,x);
		curx = x;
	}
}

static void track_point(int y, int x1, int x2)
{
	if(trace_misc)
		p_info(PI_TRACE, "track %d %d %d\n",y,x1,x2);
	if(x1 > x2)					/* always left to right */
	{
		int xx;
		xx = x1;
		x1 = x2;
		x2 = xx;
	}
	if(last_y == y)				/* horizontal line? */
	{
		if(last_x1 > x1)
			last_x1 = x1;
		if(last_x2 < x2)
			last_x2 = x2;
	}
	else if(last_y > y && last_dy > 0) /* same direction? */
	{
		make_edge(last_y,last_x1,last_x2);
		last_y = y;
		last_x1 = x1;
		last_x2 = x2;
	}
	else if(last_y < y && last_dy < 0) /* same direction? */
	{
		make_edge(last_y,last_x1,last_x2);
		last_y = y;
		last_x1 = x1;
		last_x2 = x2;
	}
	else						/* direction switch! */
	{
		make_edge(last_y,last_x1,last_x2); /* do min's * max's twice */
		make_edge(last_y,last_x1,last_x2);
		last_dy = last_y - y;
		last_y = y;
		last_x1 = x1;
		last_x2 = x2;
	}
}

static void make_edge(int y, int x1, int x2)
{
	Swath *ptr;
	Swath *scan;

	if(trace_misc)
		p_info(PI_TRACE, "at %d,%d  %d\n",y,x1,first_edge);
	
	ptr = (Swath *)p_alloc(sizeof(Swath)); /* create a swath ptr */
	if (ptr == NULL)
	{
		p_info(PI_ALOG, "Memory not avalaible...\n");
		exit(1);
	}
	ptr -> y = y;
	ptr -> on = x1;
	ptr -> off = x2;
	ptr -> used = first_edge;	/* put in proper place*/
	first_edge = 0;
	for(scan = (Swath *)work.head;scan;scan = (Swath *)scan -> que.next)
	{
		if(y > scan -> y || (y == scan -> y && x1 > scan -> on))
			continue;
		QinsertBefore(&work,(QE *)scan,(QE *)ptr);
		return;
	}
	QinsertTail(&work,(QE *)ptr);
}

/*
 *	Edge points have been plotted, make swaths from edges
 */

static void make_swaths()		/* This is a routine to combine */
{								/* pairs of edges into swaths. */
	Swath *hold;
	Swath *ptr;
	Swath *scan;
	Swath *last = NULL;
	int count = 0;

	hold = (Swath *)work.head;
	/*SUPPRESS558*/
	for(ptr = hold; 1 ; ptr = (Swath *)ptr -> que.next)
	{
		if(ptr && hold -> y == ptr -> y)
		{
			count++;
			if(ptr -> used)		/* is this the "special" last point? */
			{
				last = ptr;
				ptr -> used = 0;
			}
		}
		else
		{
			if(count & 1)		/* must have even start/stops */
			{
				if(last)		/* unless last point is odd one */
				{
					if(hold == last)
						hold = (Swath *)last -> que.next;
					QremoveElement(&work,(QE *)last);	/* drop last */
					p_free((char *)last);
				}
				else
					p_info(PI_ELOG, "============= odd count ============\n");
			}
			for(scan = hold;scan != ptr;scan= (Swath *)scan -> que.next)
			{
				Swath *next;
				next = (Swath *)scan -> que.next; /* two edges =*/
				QremoveElement(&work, (QE *)next); /*  one swath */
				if(next -> on < scan -> on)
					scan -> on = next -> on;
				if(next -> off > scan -> off)
					scan -> off = next -> off;
				p_free((char *)next);
			}
			hold = ptr;
			if(ptr == 0)
				break;
			last = 0;
			count = 1;
			if(ptr -> used)		/* is this the "special" last point? */
			{
				last = ptr;
				ptr -> used = 0;
			}
		}
	}
}

static void spread_swaths(int top, int bottom, int left, int right)
{
	int cnt;
	int max;
	Swath *scan;

	for(scan = (Swath *)work.head; scan; scan = (Swath *)scan -> que.next)
	{
		Swath *next;
		scan -> on -= left;		/* spread to left */
		scan -> off += right;	/* spread to right */
		while((next = (Swath *)scan -> que.next) != NULL)
		{						/* test for overlap */
			if(scan -> y !=  next -> y)
				break;
			if(scan -> off < (next -> on - left))
				break;
			
			QremoveElement(&work, (QE *)next);
			scan -> off = next -> off + right;
			p_free((char *)next);
		}
	}
	if(top > bottom)
		max = top;
	else
		max = bottom;
	for(cnt = 0; cnt < max; cnt++) /* do up's and down's */
	{							/*  one at a time */
		if(cnt < top)			/* by dup'ing swath  */
		{						/* above or below */
			for(scan = (Swath *)work.head; /* its current */
				scan;			/* location */
				scan = (Swath *)scan -> que.next)
			{
				Swath *test;
				int   scany;

				scany = scan -> y - 1;
				for(test = scan; test -> que.prev; 
					test = (Swath *)test -> que.prev)
				{
					if(test -> y < scany)
						break;
				}
				merge_swath(scan -> y - 1,scan -> on, scan -> off,test);
			}
		}
		if(cnt < bottom)
		{
			for(scan = (Swath *)work.tail; scan; 
				scan = (Swath *)scan -> que.prev)
				merge_swath(scan -> y + 1,scan -> on, scan -> off,scan);
		}
	}
}

static void merge_swath(y,x1,x2,scan1) /* merge this swath with  */
int y,x1,x2;					/* existing swaths and/or */
Swath *scan1;					/* create a new swath     */
{
	Swath *scan;
	Swath *ptr;

	ptr = (Swath *)p_alloc(sizeof(Swath));
	ptr -> y = y;
	ptr -> on = x1;
	ptr -> off = x2;
	if(trace_misc)
		p_info(PI_TRACE, "merge_swath %d %d %d\n",y,x1,x2);
	for(scan = scan1;scan;scan = (Swath *)scan -> que.next)
	{
		if(y < scan -> y)
		{
			QinsertBefore(&work,(QE *)scan,(QE *)ptr); /* new swath */
			return;
		}
		if(y > scan -> y)
			continue;
		while(scan && scan -> y == ptr -> y)
		{
			Swath *temp;

			if(ptr -> off < scan -> on)
			{
				QinsertBefore(&work,(QE *)scan,(QE *)ptr);
				return;
			}
			if(ptr -> on > scan -> off)
			{
				scan = (Swath *)scan -> que.next;
				if(scan && scan -> y == y)
					continue;
				if(scan == 0)
					break;
				QinsertBefore(&work,(QE *)scan,(QE *)ptr);
				return;
			}
			if(scan -> on < ptr -> on) /* merge swaths */
				ptr -> on = scan -> on;
			if(scan -> off > ptr -> off)
				ptr -> off = scan -> off;
			temp = (Swath *)scan -> que.next;
			QremoveElement(&work,(QE *)scan);
			p_free((char *)scan);
			scan = temp;
		}
		if(scan == 0)
			break;
		QinsertBefore(&work,(QE *)scan,(QE *)ptr);
		return;
	}
	QinsertTail(&work,(QE *)ptr);
}

static void follow_the_shape()
{
	int direction;				/* 1 = down left   2 = up right */
	int reverse;
	int ytest;
	int x;
	int y;
	Swath *scan;				/* scan line we are working on */
	Swath *left = NULL;			/* scan line to left of current */
	Swath *right = NULL;		/* scan line to right of current */
	Swath *next;
	Swath *test;

	num_points = 0;
	scan = (Swath *)work.head;	/* first swath */
	direction = 0;
	y = scan -> y;
	x = scan -> on;
	plot_x_y(x,y,direction);
	direction = 1;				/* going down the left side */
	scan -> used = 1;			/* mark point used */
	reverse = 0;
	while(reverse < 2)			/* while points remain */
	{
		if(reverse == 0)
		{
			if((left = (Swath *)scan -> que.prev) != NULL)
			{
				if(left -> y != y)
					left = 0;
			}
			if((right = (Swath *)scan -> que.next) != NULL)
			{
				if(right -> y != y)
					right = 0;
			}
		}
		if(direction == 1)		/* down the left side */
		{
			ytest = y + 1;
			for(next = scan;next; next = (Swath *)next -> que.next)
			{
				if(ytest < next -> y)
				{
					next = 0;	/* past scan line */
					break;		/* force white under */
				}
				if(ytest > next -> y)
					continue;	/* still above line */
				if (next -> on > scan -> off || next -> off < scan -> on)
					continue;	/* disjoint swaths */
				break;
			}
			if(next == 0)		/* white under current scan */
			{
				reverse++;
				x = scan -> off;
				direction = 0;
				plot_x_y(x,y,direction);
				direction = 2;
				scan -> used |= 2;
				continue;
			}
			reverse = 0;
			if(left)			/* is there a gap on the left? */
			{
				if (scan -> on  <= next -> off &&
					left -> off >= next -> on)
				{
					plot_x_y(scan -> on,ytest,0);
					plot_x_y(left -> off,ytest,0);
					direction = 2;
					scan = left;
					x = scan -> off;
					y = scan -> y;
					plot_x_y(x,y,direction);
					scan -> used |= 2;
					continue;
				}
			}
			if(next -> used & 1)
				return;
			scan = next;
			y = scan -> y;
			x = scan -> on;
			plot_x_y(x,y,direction);
			next -> used |= 1;
			continue;
		}
		else					/* direction == 2 */
		{
			ytest = y - 1;
			for(next = scan;next; next = (Swath *)next -> que.prev)
			{
				if(ytest == next -> y)
				{
					while(next)
					{
						if(next -> que.prev)
						{
							test = (Swath *)next -> que.prev;
							if(test -> y != next -> y)
							{
								break;
							}
							next = test;
						}
						else
							break;
					}
					break;
				}
			}
			if(next == 0)
			{
				reverse++;
				x = scan -> on;
				direction = 0;
				plot_x_y(x,y,direction);
				direction = 1;
				continue;
			}
			for(;next;next = (Swath *)next -> que.next)
			{
				if(next == 0 || next -> y != ytest)
				{
					reverse++;
					x = scan -> on;
					direction = 0;
					plot_x_y(x,y,direction);
					direction = 1;
					break;
				}
				if(next -> on > scan -> off || next -> off < scan -> on)
					continue;
				while(next)
				{
					if((test = (Swath *)next -> que.next) == 0)
						break;
					if (test -> y != next -> y)
						break;
					if (test -> on > scan -> off ||
						test -> off < scan -> on)
						break;
					next = test;
				}
				if(next -> used & 2)
					return;
				reverse = 0;
				if(right)		/* is there a gap on the left? */
				{
					if (scan -> off >= next -> on &&
						right -> on <= next -> off)
					{
						plot_x_y(scan -> off,ytest,0);
						plot_x_y(right -> on,ytest,0);
						direction = 1;
						scan = right;
						x = scan -> on;
						y = scan -> y;
						plot_x_y(x,y,direction);
						scan -> used |= 1;
						break;
					}
				}
				scan = next;
				y = scan -> y;
				x = scan -> off;
				plot_x_y(x,y,direction);
				next -> used |= 2;
				break;
			}
		}
	}
}								/* end while(reverse < 2) while points remain*/

static int xx1;					/* Doubled letters because of math.h */
static int yy1;
static int xx2;
static int yy2;
static int ldir;
static int ycnt;
static int ldif;
static Point *point;

static void plot_x_y(int x, int y, int direction)
{
	int dif;

	/*
	  p_info(PI_TRACE, "x/y/d %3d/%3d/%d   px/py/pd %3d/%3d/%d  dx/dy %3d/%d\n",
	  x,y,direction,px,py,pd,px-x,py-y);
	  px = x;
	  py = y;
	  pd = direction;
	  */
	if(trace_misc)
		p_info(PI_TRACE, "xy %3d,%3d, %d %3d", y,x,direction,num_points);
	if(ldir == direction)
	{
		switch (direction)
		{
		  case 1:
			if(trace_misc)
				p_info(PI_TRACE, "  %3d,%3d  %3d,%3d %3d  %3d,%3d",
						  xx1,yy1,xx2,yy2,ycnt,x,y);
			dif = ((xx2 - xx1) * (ycnt + 1) * 10) / ycnt;
			dif += xx1 * 10;
			if(trace_misc)
				p_info(PI_TRACE, "  dif %3d %3d %3d",dif,x,(x * 10) - dif);
			dif = (x * 10) - dif;
			ldif += dif;
			if(ldif < -10 || ldif > 10)
				break;
			if(dif < -10 || dif > 10)
				break;
			ycnt++;
			point -> x = x;
			point -> y = y;
			xx2 = x;
			yy2 = y;
			if(trace_misc)
				p_info(PI_TRACE, "  join\n");
			return;
		  case 2:
			if(trace_misc)
				p_info(PI_TRACE, "  %3d,%3d  %3d,%3d %3d  %3d,%3d",
						  xx1,yy1,xx2,yy2,ycnt,x,y);
			dif = ((xx2 - xx1) * (ycnt + 1) * 10) / ycnt;
			dif += xx1 * 10;
			if(trace_misc)
				p_info(PI_TRACE, "  dif %3d %3d %3d",dif,x,(x * 10) - dif);
			dif = (x * 10) - dif;
			ldif += dif;
			if(ldif < -10 || ldif > 10)
				break;
			if(dif < -10 || dif > 10)
				break;
			ycnt++;
			point -> x = x;
			point -> y = y;
			xx2 = x;
			yy2 = y;
			if(trace_misc)
				p_info(PI_TRACE, "  join\n");
			return;
		  default:
			break;
		}
	}
	point = (Point *)p_alloc(sizeof(Point));
	point -> x = x;
	point -> y = y;
	xx1 = xx2;
	yy1 = yy2;
	xx2 = x;
	yy2 = y;
	ycnt = 1;
	ldir = direction;
	QinsertTail(&points,(QE *)point);
	num_points++;
	ldif = 0;
	if(trace_misc)
	{
		p_info(PI_TRACE, "   new  ");
		p_info(PI_TRACE, "%3d,%3d\n",x,y);
	}
}

static void dump_points(wn,num)
WYSIWYG *wn;
int num;
{
	Point *point;
	int cnt;

	smoothing_scan();
	FRAME_DATA(num) bearoff_pts =
		(DRAW_POINT_X_Y *)p_alloc( sizeof(DRAW_POINT_X_Y) * (num_points + 1));
	cnt = 0;
	for(point = (Point *)points.head; point; point =(Point *)point -> que.next)
	{
		FRAME_DATA(num) bearoff_pts[cnt].x = point -> x;
		FRAME_DATA(num) bearoff_pts[cnt].y = point -> y;
		cnt++;
	}
	FRAME_DATA(num) num_bearoffs = --cnt;
}

static void dump_trap_points(ELEMENT *ele)
{
	Point *point;
	int cnt;

	smoothing_scan();
	ele -> trap_points =
		(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * (num_points + 1));
	cnt = 0;
	for(point = (Point *)points.head; point;
		point = (Point *)point -> que.next)
	{
		ele -> trap_points[cnt].x = point -> x;
		ele -> trap_points[cnt].y = point -> y;
		cnt++;
	}
	ele -> trap_pts = --cnt;
}

static void smoothing_scan()
{								/* find group of points with same slope
								   direction */
	Point *point;
	Point *point1;
	Point *point2;
	int x1,x2,y1,y2,dx,dy;
	int cnt = 0;
	int ldir = 0;
	int dir = 0;

	if(trace_misc)
	{
		p_info(PI_TRACE, "smooth out points\n");
		point1 = 0;
		for(point = (Point *)points.head; point;
			point = (Point *)point -> que.next)
		{
			if(point1)
			{
				if(point1 -> x != point -> x)
				{
					dx = point1 -> x - point -> x;
					dy = point1 -> y - point -> y;
					p_info(PI_TRACE, "            %5d\n",(dy*1000)/dx);
				}
				else
					p_info(PI_TRACE, "            99999\n");
			}
			p_info(PI_TRACE, " %3d,%3d\n",point -> x, point -> y);
			point1 = point;
		}
	}
	point = (Point *)points.head;
	if(point == 0)
		return;
	x1 = point -> x;
	y1 = point -> y;
	point1 = point;
	point2 = point;
	cnt++;
	for(point = (Point *)point -> que.next; point;
		point = (Point *)point -> que.next)
	{
		x2 = point -> x;
		y2 = point -> y;
		dx = x1-x2;
		dy = y1-y2;
		if(dx > 0)
			dir |= 1;
		if(dx < 0)
			dir &= 2;
		if(dy > 0)
			dir |= 2;
		if(dy < 0)
			dir &= 1;
		if(trace_misc)
			p_info(PI_TRACE, "smooth   1  %2d:  %3d/%3d  %3d/%3d  %3d/%3d  %d->%d\n",
					  cnt,x1,y1,x2,y2,dx,dy,ldir,dir);
		if(ldir != dir)
		{
			smooth_out_points(point1,point2);
			point1 = point2;
		}
		ldir = dir;
		x1 = x2;
		y1 = y2;
		point2 = point;
		cnt++;
	}
	smooth_out_points(point1,point2);
}

static void smooth_out_points(Point *point1, Point *point2)
{
	Point *point;
	Point *p1;
	Point *p2;
	int ss = 0;
	int x1,x2,y1,y2,dx,dy;
	int slope;

	if(point1 == point2)
	{
		if(trace_misc)
			p_info(PI_TRACE, "point\n");
		return;
	}
	point = point1;
	x1 = point -> x;
	y1 = point -> y;
	p1 = point;
	p2 = 0;
	for(;;)
	{
		if(point == point2 || point == 0)
			break;
		point = (Point *)point -> que.next;
		if(point == 0)
			break;
		x2 = point -> x;
		y2 = point -> y;
		dx = x1-x2;
		dy = y1-y2;
		if(dx)
		{
			float slope_f;
			
			slope_f = (((float)dy)/dx) * 100.;
			if (slope_f > 0)
				slope = (int)(slope_f + .5);
			else
				slope = (int)(slope_f - .5);
		}
		else
			slope = 999999;
		if(trace_misc)
		{
			p_info(PI_TRACE, " smooth  2 %3d/%3d  %3d/%3d  %3d/%3d           %7d",
					  x1,y1,x2,y2,dx,dy,slope);
			if(p2)
				p_info(PI_TRACE, "  %d\n",ss);
			else
				p_info(PI_TRACE, "\n");
		}
		if(p2)
		{
			if(abs(ss - slope) > 50)
			{
				smooth_final(p1,p2);
				point = p2;
				p1 = p2;
				x1 = point -> x;
				y1 = point -> y;
				p2 = 0;
				continue;
			}
			else
			{
				x1 = p1 -> x;
				y1 = p1 -> y;
				dx = x1-x2;
				dy = y1-y2;
				p2 = point;
				if(dx)
				{
					float slope_f;
					
					slope_f = (((float)dy)/dx) * 100.;
					if (slope_f > 0)
						ss = (int)(slope_f + .5);
					else
						ss = (int)(slope_f - .5);
				}
				else
					ss = 999999;
			}
		}
		else
		{
			p2 = point;
			ss = slope;
		}
		x1 = x2;
		y1 = y2;
	}
	smooth_final(p1,p2);
}

static void smooth_final(Point *p1, Point *p2)
{
	Point *p;
	int x1,y1,x2,y2;
	float slope;
	float b;

	if(p1 == 0 || p2 == 0 || p1 == p2 || (Point *)p1 -> que.next == p2)
	{
		if(trace_misc)
			p_info(PI_TRACE, "  smooth 3       ?????\n");
		return;
	}
	if(p1 -> x == p2 -> x || p1 -> y == p2 -> y)
	{
		if(trace_misc)
			p_info(PI_TRACE, "  smooth 3       ?????\n");
		return;
	}
	if(trace_misc)
		p_info(PI_TRACE, "  smooth 3       %d,%d -> %d,%d\n",
				  p1->x,p1->y,p2->x,p2->y);
	x1 = p1 -> x;
	y1 = p1 -> y;
	x2 = p2 -> x;
	y2 = p2 -> y;
	slope = (float)(y2 - y1) / (float)(x2 - x1);
	b = (float)y2 - (slope * (float)x2);
	for(p = p1;p != p2; p = (Point *)p -> que.next)
	{
		float diffx;
		float diffy;

		diffx = (p -> y - b) * 100 / slope - p -> x;
		diffy = ((slope * p -> x) + b - p -> y) * 100;
		if(trace_misc)
			p_info(PI_TRACE, "   smooth 4  %3d/%3d   %3d/%3d\n",
					  p -> x,p -> y,diffx,diffy);
		if(fabs(diffx) > 50. && fabs(diffy) > 50.)
			return;
	}
	while((Point *)p1 -> que.next != p2)
	{
		Point *drop;
		drop = (Point *)p1 -> que.next;
		if(trace_misc)
			p_info(PI_TRACE, "    drop %d/%d\n",drop -> x,drop -> y);
		QremoveElement(&points,(QE *)drop);
		p_free((char *)drop);
	}
}

/**********************************************************************
 *                                                                    *
 *   This is a routine to convert MV graphic file to AViiON format    *
 *                                                                    *
 **********************************************************************/



int cr_shapes_fr_pix(int *px2_sh_recs, PXL_SHAPE **px2_shapes, char *px, char *data)
{
	int start_bit;
	int shift;
	int dpl;
	int lpg;
	int SourceBit;
	char *SourcePtr;
	int first_pixel;
	int last_pixel;
	int last_swath;
	int alignment = 0;
	int prev_on = 0;
	int prev_off = 0;
	int cnt = 0;
	PXL_HDR px2;
	memcpy((char *)&px2, px,sizeof(px2));
	if (px2.file_format == 1) {
/* Data is in a new format, or 1 byte per pixel for color images, 1 bit per pixel for b/w ones */
		if(px2.color_mask != 64)		/* color graphic */
		{
			start_bit = 0x3f;
			shift = 8;
		}
		else
		{
			start_bit = 0x80;
			shift = 1;
		}
	}
	else {
		if(file_type == 2)		/* color graphic */
		{
			start_bit = 0xf0;
			shift = 4;
		}
		else
		{
			start_bit = 0x80;
			shift = 1;
		}
	}
	/*SUPPRESS558*/
	if(0)
		/*SUPPRESS529*/
		p_info(PI_TRACE, "lpg: %d, dpl: %d, align: %d\n",
				  px2.lpg,px2.dpl,px2.line_alignment);
	switch(px2.line_alignment)
	{
	  case 0:				/* word alignment */
		if((px2.dpl % 16) > 7)
			alignment = 1;
		else if(px2.dpl % 8)
			alignment = 2;
		else
			alignment = 0;
		break;
	  case 1:				/* byte alignment */
		if(px2.file_format == 1 || file_type == 2)
		{
			if(px2.dpl % 2)
				alignment = 1;
			else
				alignment = 0;
		}
		else
		{
			if(px2.dpl % 8)
				alignment = 1;
			else
				alignment = 0;
		}
		break;
	  case 2:				/* bit alignment */
		alignment = 0;
		break;
	}
	if (data) SourcePtr = data;
	else SourcePtr = px + 512;
	SourceBit = start_bit;
	last_swath = -1;
	for(lpg = 0; lpg < px2.lpg; lpg++)
	{
		/*SUPPRESS558*/
		if(0)
			/*SUPPRESS529*/
			p_info(PI_TRACE, " %d & %2x ",SourcePtr,SourceBit);
		first_pixel = -1;
		last_pixel = -1;
		for(dpl = 0; dpl < px2.dpl; dpl++)
		{
			/*SUPPRESS558*/
			if(0)
			{
				/*SUPPRESS529*/
				if(SourceBit & 0x80)
				{
					int x;
					x = (*SourcePtr & 0x0f) + ((*SourcePtr >> 4) & 0x0f);
					p_info(PI_TRACE, "%c",x ? '@' + x : '.');
				}
			}
			if(*SourcePtr & SourceBit)
			{
				if(first_pixel < 0)
					first_pixel = dpl;
				last_pixel = dpl;
			}
			if ((SourceBit >>= shift) == 0)
			{
				SourceBit = start_bit;
				SourcePtr++;
			}
		}					/* end for(dpl...) */
		if(last_pixel > 0)
		{
			if(last_swath < 0)
				last_swath = lpg - 1;
			else
			{
				if(last_pixel < prev_on)
					last_pixel = prev_off;
				if(first_pixel > prev_off)
					first_pixel = prev_on;
			}
			while(last_swath < lpg)
			{
				Swath *swath;
				
				swath = (Swath *)p_alloc(sizeof(Swath));
				if (swath == NULL)
				{
					p_info(PI_ALOG, "Memory not avalaible...\n");
					exit(1);
				}
				swath -> y = last_swath;
				swath -> on = first_pixel;
				swath -> off = last_pixel;
				swath -> used = 0;
				/*SUPPRESS558*/
				if(0)
					/*SUPPRESS529*/
					p_info(PI_TRACE, "swath at %d  %d,%d\n",
							  last_swath,first_pixel,last_pixel);
				first_edge = 0;
				QinsertTail(&work,(QE *)swath);
				last_swath++;
			}
			prev_on = first_pixel;
			prev_off = last_pixel;
		}
		if(alignment)
		{
			SourcePtr += alignment;
			SourceBit = start_bit;
		}
	}				/* end for(lpg...) */
	if(trace_misc)
	{
		Swath *scan;
		
		p_info(PI_TRACE, "Swath queue %d -> %d\n",work.head,work.tail);
		for(scan = (Swath *)work.head; scan; 
			scan = (Swath *)scan -> que.next)
			p_info(PI_TRACE, "Swath at %d from %d to %d\n",
					  scan -> y, scan -> on, scan -> off);
	}
	if (work.head && work.tail)
		follow_the_shape();
	cnt = 0;
	for(point = (Point *)points.head; point; 
		point = (Point *)point -> que.next)
		cnt++;
	/*SUPPRESS558*/
	if(0)
		/*SUPPRESS529*/
		p_info(PI_TRACE, " sizes: %d %d %d %d\n",
				  sizeof (PXL_SHAPE),sizeof (((PXL_SHAPE *)*px2_shapes) -> pts),
				  sizeof (SHAPE_X_Y),cnt);
	*px2_sh_recs = ((sizeof (PXL_SHAPE) - sizeof (((PXL_SHAPE *)*px2_shapes) -> pts)) +
				   (sizeof (SHAPE_X_Y) * cnt)) / 512 + 1;
	*px2_shapes = (PXL_SHAPE *) p_alloc(*px2_sh_recs * 512);
	cnt = 0;
	for(point = (Point *)points.head; point;
		point = (Point *)point -> que.next)
	{
		((PXL_SHAPE *)*px2_shapes) -> pts[cnt].x = point -> x;
		((PXL_SHAPE *)*px2_shapes) -> pts[cnt].y = point -> y;
		cnt++;
	}
	((PXL_SHAPE *)*px2_shapes) -> shape = 1;
	((PXL_SHAPE *)*px2_shapes) -> of_sh = 1;
	((PXL_SHAPE *)*px2_shapes) -> num_pts = cnt;
	((PXL_SHAPE *)*px2_shapes) -> num_recs = *px2_sh_recs;
	if (data) {
/* Do some extra stuff if called not from convert gr */
		*px2_sh_recs *= 512;
		Qclear(&work);	
		Qclear(&points);
	}
	return (1);
}
