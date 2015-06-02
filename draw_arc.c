#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include "window.h"

#define pointer char *

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#define ISEQUAL(a,b) (fabs((a) - (b)) <= EPSILON)
#define UNEQUAL(a,b) (fabs((a) - (b)) > EPSILON)
#define PTISEQUAL(a,b) (ISEQUAL(a.x,b.x) && ISEQUAL(a.y,b.y))
#define PTUNEQUAL(a,b) (UNEQUAL(a.x,b.x) || UNEQUAL(a.y,b.y))
#define ROUNDTOINT(x)   ((int) (((x) > 0.0) ? ((x) + 0.5) : ((x) - 0.5)))
#define ADD_REALLOC_STEP	20
#define EPSILON        0.000001
#define Dsin(d)	((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
#define Dcos(d)	((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))
#define mod(a,b)	((a) >= 0 ? (a) % (b) : (b) - (-a) % (b))
#define todeg(xAngle)	(((double) (xAngle)) / 64.0)
#define lowbit(x) ((x) & (~(x) + 1))
/* 360 degrees * 64 sub-degree positions */
#define FULLCIRCLE (360 * 64)
#define RIGHT_END	0
#define LEFT_END	1
#define BINARY_LIMIT	(0.1)
#define NEWTON_LIMIT	(0.0000001)
#define allocFinalSpan()   (freeFinalSpans ?\
							((tmpFinalSpan = freeFinalSpans), \
							 (freeFinalSpans = freeFinalSpans->next), \
							 (tmpFinalSpan->next = 0), \
							 tmpFinalSpan) : \
							realAllocSpan ())
typedef struct _xArc
{
    int32 x, y;
    uint32 width, height;
    int32 angle1, angle2;
} xArc;

typedef struct _DDXPointRec
{
	int x;
	int y;
} DDXPointRec, *DDXPointPtr;
typedef struct _SppArc
{
    double  x, y, width, height;
    double  angle1, angle2;
} SppArcRec, *SppArcPtr;

typedef struct _SppPoint
{
	double	x, y;
} SppPointRec, *SppPointPtr;

typedef struct _miArcJoin
{
	int	arcIndex0, arcIndex1;
	int	phase0, phase1;
	int	end0, end1;
} miArcJoinRec, *miArcJoinPtr;

typedef struct _miArcCap
{
	int arcIndex;
	int end;		
} miArcCapRec, *miArcCapPtr;

typedef struct _miArcFace
{
	SppPointRec	clock;
	SppPointRec	center;
	SppPointRec	counterClock;
} miArcFaceRec, *miArcFacePtr;

typedef struct _miArcData
{
	xArc arc;
	int render;					/* non-zero means render after drawing */
	int join;					/* related join */
	int cap;					/* related cap */
	int selfJoin;				/* final dash meets first dash */
	miArcFaceRec bounds[2];
	double x0, y0, x1, y1;
} miArcDataRec, *miArcDataPtr;

/*
 * This is an entire sequence of arcs, computed and categorized according
 * to operation.  miDashArcs generates either one or two of these.
 */
typedef struct _miPolyArc
{
	int narcs;
	miArcDataPtr arcs;
	int ncaps;
	miArcCapPtr caps;
	int njoins;
	miArcJoinPtr joins;
} miPolyArcRec, *miPolyArcPtr;

struct bound
{
	double min, max;
};

struct line
{
	double m, b;
	int	valid;
};

/*
 * these are all y value bounds
 */
struct arc_bound
{
	struct bound ellipse;
	struct bound inner;
	struct bound outer;
	struct bound right;
	struct bound left;
};

# define BINARY_TABLE_SIZE	(512)

struct accelerators
{
	double tail_y;
	double h2;
	double w2;
	double h4;
	double w4;
	double h2mw2;
	double wh2mw2;
	double wh4;
	struct line	left, right;
	double innerTable[BINARY_TABLE_SIZE+1];
	double outerTable[BINARY_TABLE_SIZE+1];
	char innerValid[BINARY_TABLE_SIZE+1];
	char outerValid[BINARY_TABLE_SIZE+1];
};

struct arc_def
{
	double w, h, l;
	double a0, a1;
};

struct finalSpan
{
	struct finalSpan *next;
	int min, max;				/* x values */
};

struct arcData
{
	double x0, y0, x1, y1;
	int	selfJoin;
};

#ifdef AAA
static void p_cleanFinalSpans (struct finalSpan **fSpan);
#endif
static void miFreeArcs(miPolyArcPtr arcs);
static struct finalSpan *freeFinalSpans, *tmpFinalSpan;
static double arcXcenter, arcYcenter;
static int arcXoffset, arcYoffset;
static double spanY;
static int quadrantMask;
static struct finalSpan	***finalSpans;
static struct finalSpan	**qfinalSpans[4];
static int qfinalMiny[4], qfinalMaxy[4];
static int *finalMiny, *finalMaxy;
static int qfinalSize[4];
static int *finalSize;
static int qspans[4];			/* total spans, not just y coords */
static int *nspans;				/* total spans, not just y coords */

#ifdef WE_DO_NOT_CARE_ABOUT_CAP
static void miArcCap(int line_width, int capStyle, miArcFacePtr pFace,
					 int end, int xOrg, int yOrg, 
					 double xFtrans, double yFtrans);
static int miArcJoin(int line_width, int joinStyle, miArcFacePtr pRight,
					 miArcFacePtr pLeft, int xOrgRight, int yOrgRight,
					 double xFtransRight, double yFtransRight,
					 int xOrgLeft, int yOrgLeft,
					 double xFtransLeft, double yFtransLeft);
static void miRoundCap(int line_width, SppPointRec pCenter, SppPointRec pEnd,
					   SppPointRec pCorner, SppPointRec pOtherCorner, 
					   int fLineEnd, int xOrg, int yOrg, double xFtrans,
					   double yFtrans);
static int miGetArcPts(SppArcPtr parc, int cpt, SppPointPtr *ppPts);
#endif

static void disposeFinalSpans(void);
static void p_fillSpans();
static miPolyArcPtr miComputeArcs(xArc *parcs, int narcs);
static void drawArc(int x0, int y0, int w, int h, int l, int a0, int a1,
					miArcFacePtr right, miArcFacePtr left);
static void drawQuadrant(struct arc_def *def, struct accelerators *acc, 
						 int a0, int a1, int mask, miArcFacePtr right,
						 miArcFacePtr left);
static void span(double left, double right);
struct finalSpan *realAllocSpan(void);
static double ellipseY(double edge_y, struct arc_def *def, 
					   struct accelerators *acc, int outer, double y0,
					   double y1);
static void miArcSegment(xArc tarc, miArcFacePtr right, miArcFacePtr left,
						 int line_width);
static void miPolyArc(int narcs, xArc *parcs, int line_width);
#ifdef AAA
static void reorganize_pts(DDXPointPtr xSpans, int *xWidths, int numSpans);
#endif

static DRAW_POINT_X_Y **pts;
static int16 *num;
static P_ARC p_arc;
#ifdef AAA
static int level;
#endif

#define COPY_X(x) (x)
#define COPY_Y(y) (y)

#define NO_LEVEL	0
#define X_LEVEL		1
#define Y_LEVEL		2

void p_clean_mem_arc()
{
	if ( (char *)qfinalSpans[0] )
		p_free((char *)qfinalSpans[0]);
	if ( (char *)qfinalSpans[1] )
		p_free((char *)qfinalSpans[1]);
	if ( (char *)qfinalSpans[2] )
		p_free((char *)qfinalSpans[2]);
	if ( (char *)qfinalSpans[3] )
		p_free((char *)qfinalSpans[3]);
}

static void add_pt(int32 x, int32 y, DRAW_POINT_X_Y **p)
{
#ifdef AAA
	p_info(PI_TRACE, "level: %d, x: %ld, y: %ld ", level, x, y);
	if (level == X_LEVEL && x == (*p - 1) -> x)
		(*p - 1) -> y = y;
	else if (level == Y_LEVEL && y == (*p - 1) -> y)
		(*p - 1) -> x = x;
	else
	{
		if (level != NO_LEVEL || *num < 2)
			level = NO_LEVEL;
		else if (level == NO_LEVEL)
		{
			if (x == (*p - 1) -> x && x == (*p - 2) -> x)
				level = X_LEVEL;
			else if (y == (*p - 1) -> y && y == (*p - 2) -> y)
				level = Y_LEVEL;
		}
#endif
		(*p) -> draw_type = D_LINE;
		(*p) -> x = x;
		(*p) -> y = y;
		(*p) -> arc = (P_ARC *)p_alloc(sizeof(P_ARC));
		memcpy((char *)(*p) -> arc, (char *)&p_arc, sizeof(P_ARC));
		(*p)++;
		(*num)++;
#ifdef AAA
	}
#endif
	if (0)
		p_info(PI_TRACE, "*num: %d (%ld, %ld)\n",(*num - 1), (*p - 1) -> x,
			   (*p - 1) -> y);
}

#ifdef AAA
static void reorganize_pts(xSpans, xWidths, numSpans)
DDXPointPtr	xSpans;
int		*xWidths;
int		numSpans;
{
	int i, oldsize;
	int32 x, y;
	int *xWidth;
	DRAW_POINT_X_Y *p;
	DDXPointPtr xSp;
	
	p_info(PI_TRACE, "numSpans: %d\n", numSpans);
	level = NO_LEVEL;
	*num = 0;
	oldsize = (numSpans * 2) * sizeof(DRAW_POINT_X_Y);
	p = *pts = (DRAW_POINT_X_Y *)p_alloc(oldsize);
	xWidth = xWidths;
	xSp = xSpans;
	for (i = 0; i < (numSpans / 2); i += 2, xSp += 2, xWidth += 2)
	{
		x = COPY_X(xSp -> x);
		y = COPY_Y(xSp -> y);
		if (i == 0 || x != (p - 1) -> x || y != (p - 1) -> y)
		{
			p_info(PI_TRACE, "%d A ", i);
			add_pt(x, y, &p);
		}
		if (*xWidth - 1)
		{
			x = COPY_X(xSp -> x + (*xWidth - 1));
			if (x != (p - 1) -> x)
			{
				p_info(PI_TRACE, "%d B ", i);
				add_pt(x, y, &p);
			}
		}
	}
	p_info(PI_TRACE, "Switch 1\n");
	i--;
	xSp--;
	xWidth--;
	for (; i < numSpans; i += 2, xSp += 2, xWidth += 2)
	{
		x = COPY_X(xSp -> x + (*xWidth - 1));
		y = COPY_Y(xSp -> y);
		if (x != (p - 1) -> x || y != (p - 1) -> y)
		{
			p_info(PI_TRACE, "%d A ", i);
			add_pt(x, y, &p);
		}
		if (*xWidth - 1)
		{
			x = COPY_X(xSp -> x);
			if (x != (p - 1) -> x)
			{
				p_info(PI_TRACE, "%d B ", i);
				add_pt(x, y, &p);
			}
		}
	}
	p_info(PI_TRACE, "Switch 2\n");
	i = i - 3;
	xWidth = xWidth - 3;
	xSp = xSp - 3;
	for (; i >= (numSpans / 2); i -= 2, xSp -= 2, xWidth -= 2)
	{
		x = COPY_X(xSp -> x + (*xWidth - 1));
		y = COPY_Y(xSp -> y);
		if (x != (p - 1) -> x || y != (p - 1) -> y)
		{
			p_info(PI_TRACE, "%d A ", i);
			add_pt(x, y, &p);
		}
		if (*xWidth - 1)
		{
			x = COPY_X(xSp -> x);
			if (x != (p - 1) -> x)
			{
				p_info(PI_TRACE, "%d B ", i);
				add_pt(x, y, &p);
			}
		}
	}
	p_info(PI_TRACE, "Switch 3\n");
	i--;
	xSp--;
	xWidth--;
	for (; i > 0; i -= 2, xSp -= 2, xWidth -= 2)
	{
		x = COPY_X(xSp -> x);
		y = COPY_Y(xSp -> y);
		if (x != (p - 1) -> x || y != (p - 1) -> y)
		{
			p_info(PI_TRACE, "%d A ", i);
			add_pt(x, y, &p);
		}
		if (*xWidth - 1)
		{
			x = COPY_X(xSp -> x + (*xWidth - 1));
			if (x != (p - 1) -> x)
			{
				p_info(PI_TRACE, "%d B ", i);
				add_pt(x, y, &p);
			}
		}
	}
	/*	(*num)--;*/
	p_info(PI_TRACE, "*num: %d\n", *num);
	*pts = (DRAW_POINT_X_Y *)p_remalloc((char *)*pts, oldsize,
										*num * sizeof(DRAW_POINT_X_Y));
}
#endif

static void
p_fillSpans ()
{
	register struct finalSpan *span;
	register struct finalSpan **f;
	register int i, start_num = 0;
	register int spany;
	register int xwidth;
	int oldsize;
	DRAW_POINT_X_Y *p, *c_p, *p_w;
	
	*num = 0;
	oldsize = ((qspans[0] + qspans[1] + qspans[2] +
				qspans[3]) * sizeof(DRAW_POINT_X_Y)) << 1;
	p = *pts = (DRAW_POINT_X_Y *)p_alloc(oldsize);
	if (0)
		p_info(PI_TRACE, "Sizeof allocate: %d, Doing quadrant 0\n", oldsize);
	nspans = &qspans[0];
	if (0)
		p_info(PI_TRACE, "nspans[0]: %d\n", *nspans);
	if (*nspans != 0)
	{
	    finalMiny = &qfinalMiny[0];
	    finalMaxy = &qfinalMaxy[0];
	    finalSize = &qfinalSize[0];
	    f = qfinalSpans[0];
		if (0)
			p_info(PI_TRACE, "finalMiny[0]: %d, finalMaxy[0]: %d\n", *finalMiny, *finalMaxy);
	    for (spany = *finalMiny; spany < *finalMaxy; spany++, f++)
		{
			span = *f;
			if (!span || span->max <= span->min)
				continue;
			xwidth = span -> max - span -> min - 1;
			add_pt(span -> min, spany, &p);
			add_pt(span -> min + xwidth, spany, &p);
	    }
	    *nspans = 0;
	    *finalMiny = 0;
	    *finalMaxy = 0;
	    *finalSize = 0;
	    for (i = start_num + 1, c_p = *pts + start_num + 1;
			 i < *num; i++, c_p++)
		{
			if (c_p -> y == (c_p - 1) -> y || c_p -> x == (c_p - 1) -> x)
				continue;
			c_p -> x--;
			/*		c_p -> off_x = 1;*/
	    }
	}
	if (0)
		p_info(PI_TRACE, "Doing quadrant 3\n");
	start_num = *num - 1;
	if (start_num < 0)
		start_num = 0;
	nspans = &qspans[3];
	if (0)
		p_info(PI_TRACE, "nspans[1]: %d\n", *nspans);
	if (*nspans != 0)
	{
	    finalMiny = &qfinalMiny[3];
	    finalMaxy = &qfinalMaxy[3];
	    finalSize = &qfinalSize[3];
		if (0)
			p_info(PI_TRACE, "finalSize: %d\n", *finalSize);
	    f = qfinalSpans[3];
		if (0)
			p_info(PI_TRACE, "finalMiny[3]: %d, finalMaxy[3]: %d\n", *finalMiny,
				   *finalMaxy);
	    for (spany = *finalMiny; spany < *finalMaxy; spany++, f++)
		{
			span = *f;
			if (!span || span->max <= span->min)
				continue;
			xwidth = span -> max - span -> min - 1;
			add_pt(span -> min + xwidth, spany, &p);
			add_pt(span -> min, spany, &p);
	    }
	    *nspans = 0;
	    *finalMiny = 0;
	    *finalMaxy = 0;
	    *finalSize = 0;
	    for (i = start_num + 1, c_p = *pts + start_num + 1;
			 i < *num; i++, c_p++)
		{
			if (c_p -> y == (c_p - 1) -> y || c_p -> x == (c_p - 1) -> x)
				continue;
			(c_p - 1) -> x--;
			/*		(c_p - 1) -> off_x = 1;*/
	    }
	}
	if (0)
		p_info(PI_TRACE, "Doing quadrant 2\n");
	start_num = *num - 1;
	if (start_num < 0)
		start_num = 0;
	nspans = &qspans[2];
	if (0)
		p_info(PI_TRACE, "nspans[2]: %d\n", *nspans);
	if (*nspans != 0)
	{
	    finalMiny = &qfinalMiny[2];
	    finalMaxy = &qfinalMaxy[2];
	    finalSize = &qfinalSize[2];
		if (0)
			p_info(PI_TRACE, "finalSize: %d\n", *finalSize);
	    f = qfinalSpans[2] + *finalMaxy - 1;
		if (0)
			p_info(PI_TRACE, "finalMiny[2]: %d, finalMaxy[2]: %d\n", *finalMiny, *finalMaxy);
	    for (spany = *finalMaxy - 1; spany >= *finalMiny; spany--, f--)
		{
			span = *f;
			if (!span || span->max <= span->min)
				continue;
			xwidth = span -> max - span -> min - 1;
			add_pt(span -> min + xwidth, spany, &p);
			add_pt(span -> min, spany, &p);
	    }
	    *nspans = 0;
	    *finalMiny = 0;
	    *finalMaxy = 0;
	    *finalSize = 0;
	    for (i = start_num + 1, c_p = *pts + start_num + 1;
			 i < *num; i++, c_p++)
		{
			if (c_p -> y == (c_p - 1) -> y || c_p -> x == (c_p - 1) -> x)
				continue;
			c_p -> x++;
			/*		c_p -> off_x = -1;*/
	    }
	}
	if (0)
		p_info(PI_TRACE, "Doing quadrant 1\n");
	start_num = *num - 1;
	if (start_num < 0)
		start_num = 0;
	nspans = &qspans[1];
	if (0)
		p_info(PI_TRACE, "nspans[1]: %d\n", *nspans);
	if (*nspans != 0)
	{
	    finalMiny = &qfinalMiny[1];
	    finalMaxy = &qfinalMaxy[1];
	    finalSize = &qfinalSize[1];
		if (0)
			p_info(PI_TRACE, "finalSize: %d\n", *finalSize);
	    f = qfinalSpans[1] + *finalMaxy - 1;
		if (0)
			p_info(PI_TRACE, "finalMiny[1]: %d, finalMaxy[1]: %d\n", *finalMiny,
				   *finalMaxy);
	    for (spany = *finalMaxy - 1; spany >= *finalMiny; spany--, f--)
		{
			span = *f;
			if (!span || span->max <= span->min)
				continue;
			xwidth = span -> max - span -> min - 1;
			add_pt(span -> min, spany, &p);
			add_pt(span -> min + xwidth, spany, &p);
	    }
	    *nspans = 0;
	    *finalMiny = 0;
	    *finalMaxy = 0;
	    *finalSize = 0;
	    for (i = start_num + 1, c_p = *pts + start_num + 1;
			 i < *num; i++, c_p++)
		{
			if (c_p -> y == (c_p - 1) -> y || c_p -> x == (c_p - 1) -> x)
				continue;
			(c_p - 1) -> x++;
			/*		(c_p - 1) -> off_x = -1;*/
	    }
	}
	if (0)
		p_info(PI_TRACE, "*num: %d\n", *num);
	for (start_num = 1, i = 1, p = p_w = *pts + 1; i < *num; i++, p++)
	{
	    if ((p_w - 1) -> x != p -> x || (p_w - 1) -> y != p -> y)
		{
			if (p != p_w)
			{
				p_w -> x = p -> x;
				p_w -> y = p -> y;
			}
			p_w++;
			start_num++;
	    }
	}
	for (i = start_num, p = *pts + start_num; i < *num; i++, p++)
	{
	    if (p -> arc)
			p_free((char *)p -> arc);
	}
	*num = start_num;
	if (0)
		p_info(PI_TRACE, "*num: %d\n", *num);
	if (0)
		for (i = 0, p = *pts; i < *num; i++, p++)
			p_info(PI_TRACE, "i: %d, x: %ld, y: %ld\n", i, p -> x, p -> y);
#ifdef AAA
	*pts = (DRAW_POINT_X_Y *)p_remalloc((char *)*pts, oldsize, *num *
										sizeof(DRAW_POINT_X_Y));
#endif
	disposeFinalSpans ();
}

void p_draw_arc(int32 left, int32 top, int32 right, int32 bottom,
				DRAW_POINT_X_Y **ret_pts, int16 *ret_num, int line_width,
				int a1, int a2)
{
	xArc arc_data;
	
	/*p_info(PI_TRACE, "left: %ld, top: %ld, width: %ld, bottom: %ld, l_w: %d, a1: %d, a2: %d\n",
	  left, top, right, bottom, line_width, a1, a2);*/
	pts = ret_pts;
	num = ret_num;
	p_arc.left = left;
	p_arc.top = top;
	p_arc.width = right;
	p_arc.depth = bottom;
	p_arc.start_angle = a1;
	p_arc.end_angle = a2;
	arc_data.x = left;
	arc_data.y = top;
	arc_data.width = abs(left - right);
	arc_data.height = abs(top - bottom);
	arc_data.angle1 = a1;
	arc_data.angle2 = a2;
	/*p_info(PI_TRACE, "%d %d %d %d %d %d\n", arc_data.x, arc_data.y, arc_data.width, arc_data.height, arc_data.angle1, arc_data.angle2);*/
	miPolyArc(1, &arc_data, line_width);
	p_fillSpans();
}

/*
 * miPolyArc strategy:
 *
 * If there's only 1 arc, or if the arc is draw with zero width lines, we 
 * don't have to worry about the rasterop or join styles.   
 * Otherwise, we set up pDrawTo and pGCTo according to the rasterop, then
 * draw using pGCTo and pDrawTo.  If the raster-op was "tricky," that is,
 * if it involves the destination, then we use PushPixels to move the bits
 * from the scratch drawable to pDraw. (See the wide line code for a
 * fuller explanation of this.)
 */
static void miPolyArc(int narcs, xArc *parcs, int line_width)
{
    register int i;
#ifdef AAA
    int xMin, xMax, yMin, yMax;
    int dx, dy;
    int xOrg, yOrg;
    unsigned long fg, bg;
#endif
    miPolyArcPtr polyArcs;
    int cap[2], join[2];
    int iphase;
	
    {
#ifdef AAA
		fg = 1;					/*pGC->fgPixel;*/
		bg = 0;					/*pGC->bgPixel;*/
		if ((pGC->fillStyle == FillTiled) ||
			(pGC->fillStyle == FillOpaqueStippled))
			bg = fg; /* the protocol sez these don't cause color changes */
#endif
		polyArcs = miComputeArcs (parcs, narcs);
		if (!polyArcs)
			return;
		cap[0] = cap[1] = 0;
		join[0] = join[1] = 0;
		for (iphase = 0; iphase >= 0; iphase--)
		{
#ifdef AAA
			if (iphase == 1)
			{
				DoChangeGC (pGC, GCForeground, (XID *)&bg, 0);
				ValidateGC (pDraw, pGC);
			}
#endif
			for (i = 0; i < polyArcs[iphase].narcs; i++)
			{
				miArcDataPtr	arcData;
				
				arcData = &polyArcs[iphase].arcs[i];
				miArcSegment(arcData->arc, &arcData->bounds[RIGHT_END],
							 &arcData->bounds[LEFT_END], line_width);
				if (polyArcs[iphase].arcs[i].render)
				{
#ifdef AAA
					fillSpans ();
#endif
					break;
#ifdef WE_DO_NOT_CARE_ABOUT_CAP
					/*
					 * don't cap self-joining arcs
					 */
					if (polyArcs[iphase].arcs[i].selfJoin &&
						cap[iphase] < polyArcs[iphase].arcs[i].cap)
						cap[iphase]++;
					while (cap[iphase] < polyArcs[iphase].arcs[i].cap)
					{
						int	arcIndex, end;
						miArcDataPtr arcData0;
						
						arcIndex = polyArcs[iphase].caps[cap[iphase]].arcIndex;
						end = polyArcs[iphase].caps[cap[iphase]].end;
						arcData0 = &polyArcs[iphase].arcs[arcIndex];
						miArcCap(line_width, CapButt, 
								 &arcData0->bounds[end], end,
								 arcData0->arc.x, arcData0->arc.y,
								 (double) arcData0->arc.width / 2.0,
								 (double) arcData0->arc.height / 2.0);
						++cap[iphase];
					}
					while (join[iphase] < polyArcs[iphase].arcs[i].join)
					{
						int	arcIndex0, arcIndex1, end0, end1;
						int	phase0, phase1;
						miArcDataPtr arcData0, arcData1;
						miArcJoinPtr joinp;
						
						joinp = &polyArcs[iphase].joins[join[iphase]];
						arcIndex0 = joinp->arcIndex0;
						end0 = joinp->end0;
						arcIndex1 = joinp->arcIndex1;
						end1 = joinp->end1;
						phase0 = joinp->phase0;
						phase1 = joinp->phase1;
						arcData0 = &polyArcs[phase0].arcs[arcIndex0];
						arcData1 = &polyArcs[phase1].arcs[arcIndex1];
						miArcJoin(line_width, JoinMiter,
								  &arcData0->bounds[end0],
								  &arcData1->bounds[end1],
								  arcData0->arc.x, arcData0->arc.y,
								  (double) arcData0->arc.width / 2.0,
								  (double) arcData0->arc.height / 2.0,
								  arcData1->arc.x, arcData1->arc.y,
								  (double) arcData1->arc.width / 2.0,
								  (double) arcData1->arc.height / 2.0);
						++join[iphase];
					}
#endif
				}
			}
		}
    }
	miFreeArcs(polyArcs);
}

static double miDcos (double a)
{
	int	i;
	
	if (floor (a/90) == a/90)
	{
		i = (int) (a/90.0);
		switch (mod (i, 4))
		{
		  case 0:
			return 1;
		  case 1:
			return 0;
		  case 2:
			return -1;
		  case 3:
			return 0;
		}
	}
	return cos (a * M_PI / 180.0);
}

static double miDsin (double a)
{
	int	i;
	
	if (floor (a/90) == a/90)
	{
		i = (int) (a/90.0);
		switch (mod (i, 4))
		{
		  case 0:
			return 0;
		  case 1:
			return 1;
		  case 2:
			return 0;
		  case 3:
			return -1;
		}
	}
	return sin (a * M_PI / 180.0);
}

#ifdef WE_DO_NOT_CARE_ABOUT_CAP
static double miDasin (double v)
{
    if (v == 0)
		return 0.0;
    if (v == 1.0)
		return 90.0;
    if (v == -1.0)
		return -90.0;
    return asin(v) * (180.0 / M_PI);
}
#endif

double miDatan2 (double	dy, double dx)
{
    if (dy == 0)
	{
		if (dx >= 0)
			return 0.0;
		return 180.0;
    } else if (dx == 0)
	{
		if (dy > 0)
			return 90.0;
		return -90.0;
    } else if (fabs (dy) == fabs (dx))
	{
		if (dy > 0)
		{
			if (dx > 0)
				return 45.0;
			return 135.0;
		} else
		{
			if (dx > 0)
				return 225.0;
			return 315.0;
		}
    } else
		return atan2 (dy, dx) * (180.0 / M_PI);
}

/*
 * draw one segment of the arc using the arc spans generation routines
 */
static void miArcSegment(xArc tarc, miArcFacePtr right, miArcFacePtr left,
						 int line_width)
{
    int l = line_width;			/* CV */
    int a0, a1, startAngle, endAngle;
    miArcFacePtr temp;
	
    a0 = tarc.angle1;
    a1 = tarc.angle2;
    if (a1 > FULLCIRCLE)
		a1 = FULLCIRCLE;
    else if (a1 < -FULLCIRCLE)
		a1 = -FULLCIRCLE;
    if (a1 < 0)
	{
    	startAngle = a0 + a1;
		endAngle = a0;
		temp = right;
		right = left;
		left = temp;
    } else {
		startAngle = a0;
		endAngle = a0 + a1;
    }
    /*
     * bounds check the two angles
     */
    if (startAngle < 0)
		startAngle = FULLCIRCLE - (-startAngle) % FULLCIRCLE;
    if (startAngle >= FULLCIRCLE)
		startAngle = startAngle % FULLCIRCLE;
    if (endAngle < 0)
		endAngle = FULLCIRCLE - (-endAngle) % FULLCIRCLE;
    if (endAngle > FULLCIRCLE)
		endAngle = (endAngle-1) % FULLCIRCLE + 1;
    if ((startAngle == endAngle) && a1) {
		startAngle = 0;
		endAngle = FULLCIRCLE;
    }
	/*p_info(PI_TRACE, "startAngle: %d, endAngle: %d\n", startAngle, endAngle);*/
    drawArc ((int) tarc.x, (int) tarc.y,
             (int) tarc.width, (int) tarc.height, l, startAngle, endAngle,
			 right, left);
}

static void mirrorSppPoint (int quadrant, SppPointPtr sppPoint)
{
	switch (quadrant)
	{
	  case 0:
		break;
	  case 1:
		sppPoint->x = -sppPoint->x;
		break;
	  case 2:
		sppPoint->x = -sppPoint->x;
		sppPoint->y = -sppPoint->y;
		break;
	  case 3:
		sppPoint->y = -sppPoint->y;
		break;
	}
	/*
	 * and translate to X coordinate system
	 */
	sppPoint->y = -sppPoint->y;
}

#if (defined (NOINLINEICEIL)) || (defined (__CODECENTER__))
#define ICEIL(x) ((int)ceil(x))
#else
#ifdef __GNUC__
static __inline int ICEIL(double x)
{
    int _cTmp = x;
    return ((x == _cTmp) || (x < 0.0)) ? _cTmp : _cTmp+1;
}
#else
#define ICEIL(x) ((((x) == (_cTmp = (x))) || ((x) < 0.0)) ? _cTmp : _cTmp+1)
#define ICEILTEMPDECL static int _cTmp;
#endif
#endif

#ifdef ICEILTEMPDECL
ICEILTEMPDECL
#endif

#undef max
#undef min

#if defined (__GNUC__) && defined (__STDC__) && !defined (__STRICT_ANSI__) && !defined(__CODECENTER__)
#define USE_INLINE
#endif

#ifdef LINT
#define inline
#endif

#ifdef USE_INLINE
inline static const int max (const int x, const int y)
{
	return x>y? x:y;
}

inline static const int min (const int x, const int y)
{
	return x<y? x:y;
}

inline double fmax (const double x, const double y)
{
	return x>y? x:y;
}

inline double fmin (const double x, const double y)
{
	return x<y? x:y;
}

#else

/*#define max MAX
  #define min MIN*/
#define fmax MAX
#define fmin MIN

#endif

#ifdef USE_INLINE
inline static const double Sqrt (const double x)
#else
static double Sqrt (double	x)
#endif
{
	if (x < 0)
	{
		if (x > -NEWTON_LIMIT)
			return 0;
		else
			p_info(PI_ELOG, "draw_arc.c: Sqrt of negative number %g\n", x);
	}
	return sqrt (x);
}

#define boundedLt(value, bounds) \
((bounds).min <= (value) && (value) < (bounds).max)

#define boundedLe(value, bounds)\
((bounds).min <= (value) && (value) <= (bounds).max)

/*
 * this computes the ellipse y value associated with the
 * bottom of the tail.
 */

# define CUBED_ROOT_2	1.2599210498948732038115849718451499938964
# define CUBED_ROOT_4	1.5874010519681993173435330390930175781250

/*
 * inverse functions -- compute edge coordinates
 * from the ellipse
 */

static double outerXfromXY(double x, double y, struct arc_def *def,
							struct accelerators *acc)
{
	return x + (x * acc->h2 * def->l) / 
		(2 * Sqrt (x*x *acc->h4 + y*y * acc->w4));
}

static double outerXfromY (double y, struct arc_def *def,
						   struct accelerators *acc)
{
	double	x;
	
	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);
	return x + (x * acc->h2 * def->l) /
		(2 * Sqrt (x*x *acc->h4 + y*y * acc->w4));
}

static double outerYfromXY(double x, double y, struct arc_def *def,
						   struct accelerators *acc)
{
	return y + (y * acc->w2 * def->l) /
		(2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

static double outerYfromY(double y, struct arc_def *def, 
						  struct accelerators *acc)
{
	double	x;
	
	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);
	return y + (y * acc->w2 * def->l) /
		(2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

static double innerXfromXY(double x, double y, struct arc_def *def,
						   struct accelerators *acc)
{
	return x - (x * acc->h2 * def->l) /
		(2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

static double innerXfromY(double y, struct arc_def *def, 
						  struct accelerators *acc)
{
	double	x;
	
	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);
	return x - (x * acc->h2 * def->l) /
		(2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

static double innerYfromXY(double x, double y, struct arc_def *def,
						   struct accelerators *acc)
{
	return y - (y * acc->w2 * def->l) /
		(2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

static double innerYfromY(double y, struct arc_def *def, 
						  struct accelerators *acc)
{
	double	x;
	
	x = def->w * Sqrt ((acc->h2 - (y*y)) / acc->h2);
	return y - (y * acc->w2 * def->l) /
		(2 * Sqrt (x*x * acc->h4 + y*y * acc->w4));
}

static void computeLine (double x1, double y1, double x2, double y2,
						 struct line	*line)
{
	if (y1 == y2)
		line->valid = 0;
	else {
		line->m = (x1 - x2) / (y1 - y2);
		line->b = x1  - y1 * line->m;
		line->valid = 1;
	}
}

static double intersectLine (double y, struct line	*line)
{
	return line->m * y + line->b;
}

/*
 * this section computes the x value of the span at y 
 * intersected with the specified face of the ellipse.
 *
 * this is the min/max X value over the set of normal
 * lines to the entire ellipse,  the equation of the
 * normal lines is:
 *
 *     ellipse_x h^2                   h^2
 * x = ------------ y + ellipse_x (1 - --- )
 *     ellipse_y w^2                   w^2
 *
 * compute the derivative with-respect-to ellipse_y and solve
 * for zero:
 *    
 *       (w^2 - h^2) ellipse_y^3 + h^4 y
 * 0 = - ----------------------------------
 *       h w ellipse_y^2 sqrt (h^2 - ellipse_y^2)
 *
 *             (   h^4 y     )
 * ellipse_y = ( ----------  ) ^ (1/3)
 *             ( (h^2 - w^2) )
 *
 * The other two solutions to the equation are imaginary.
 *
 * This gives the position on the ellipse which generates
 * the normal with the largest/smallest x intersection point.
 *
 * Now compute the second derivative to check whether
 * the intersection is a minimum or maximum:
 *
 *    h (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 * -  -------------------------------------------
 *          w y0^3 (sqrt (h^2 - y^2)) ^ 3
 *
 * as we only care about the sign,
 *
 * - (y0^3 (w^2 - h^2) + h^2 y (3y0^2 - 2h^2))
 *
 * or (to use accelerators),
 *
 * y0^3 (h^2 - w^2) - h^2 y (3y0^2 - 2h^2) 
 *
 */

/*
 * computes the position on the ellipse whose normal line
 * intersects the given scan line maximally
 */
static double hookEllipseY (double scan_y, struct arc_bound	*bound,
							struct accelerators	*acc, int left)
{
	double	ret;
	
	if (acc->h2mw2 == 0)
	{
		if ((scan_y > 0 && !left) || (scan_y < 0 && left))
			return(bound->ellipse.min);
		return bound->ellipse.max;
	}
	ret = (acc->h4 * scan_y) / (acc->h2mw2);
	if (ret >= 0)
		return pow (ret, 1.0/3.0);
	else
		return -pow (-ret, 1.0/3.0);
}

/*
 * computes the X value of the intersection of the
 * given scan line with the right side of the lower hook
 */
static double hookX (double scan_y, struct arc_def *def,
					 struct arc_bound *bound, struct accelerators *acc,
					 int left)
{
	double	ellipse_y, x;
	double	maxMin;
	
	if (def->w != def->h)
	{
		ellipse_y = hookEllipseY (scan_y, bound, acc, left);
		if (boundedLe (ellipse_y, bound->ellipse)) {
			/*
		 	 * compute the value of the second
		 	 * derivative
		 	 */
			maxMin = ellipse_y*ellipse_y*ellipse_y * acc->h2mw2 -
				acc->h2 * scan_y * (3 * ellipse_y*ellipse_y - 2*acc->h2);
			if ((left && maxMin > 0) || (!left && maxMin < 0)) {
				if (ellipse_y == 0)
					return def->w + left ? -def->l/2 : def->l/2;
				x = (acc->h2 * scan_y - ellipse_y * acc->h2mw2) *
					Sqrt (acc->h2 - ellipse_y * ellipse_y) /
						(def->h * def->w * ellipse_y);
				return x;
			}
		}
	}
	if (left)
	{
		if (acc->left.valid && boundedLe (scan_y, bound->left))
			x = intersectLine (scan_y, &acc->left);
		else
		{
			if (acc->right.valid)
				x = intersectLine (scan_y, &acc->right);
			else
				x = def->w - def->l/2;
		}
	}
	else
	{
		if (acc->right.valid && boundedLe (scan_y, bound->right))
			x = intersectLine (scan_y, &acc->right);
		else
		{
			if (acc->left.valid)
				x = intersectLine (scan_y, &acc->left);
			else
				x = def->w - def->l/2;
		}
	}
	return x;
}

static double tailEllipseY (double	w, double h, double l)
{
	double		t;
	
	if (w != h)
	{
		t = (pow (h * l * w, 2.0/3.0) - CUBED_ROOT_4 * h*h) /
		    (w*w - h*h);
		if (t < 0)
			return 0;	/* no tail */
		return h / CUBED_ROOT_2 * Sqrt (t);
	}
	else
		return 0;
}

static double outerX (double outer_y, struct arc_def *def,
					  struct arc_bound *bound, struct accelerators *acc)
{
	double	y;
	
	/*
	 * special case for circles
	 */
	if (def->w == def->h)
	{
		double x;
		
		x = def->w + def->l/2.0;
		x = Sqrt (x * x - outer_y * outer_y);
		return x;
	}
	if (outer_y == bound->outer.min)
		y = bound->ellipse.min;
	if (outer_y == bound->outer.max)
		y = bound->ellipse.max;
	else
		y = ellipseY (outer_y, def, acc, 1, 
					  bound->ellipse.min, bound->ellipse.max);
	return outerXfromY (y, def, acc);
}

/*
 * this equation has two solutions -- it's not a function
 */
static void innerXs (double inner_y, struct arc_def *def,
					 struct arc_bound *bound, struct accelerators *acc,
					 double *innerX1p, double *innerX2p)
{
	 double	x1, x2;
	double xalt, y0, y1, altY, ellipse_y1, ellipse_y2;
	
	/*
	 * special case for circles
	 */
	if (def->w == def->h)
	{
		x1 = def->w - def->l/2.0;
		x2 = Sqrt (x1 * x1 - inner_y * inner_y);
		if (x1 < 0)
			x2 = -x2;
		*innerX1p = *innerX2p = x2;
		return;
	}
	if (boundedLe (acc->tail_y, bound->ellipse))
	{
		if (def->h > def->w)
		{
			y0 = bound->ellipse.min;
			y1 = acc->tail_y;
			altY = bound->ellipse.max;
		}
		else
		{
			y0 = bound->ellipse.max;
			y1 = acc->tail_y;
			altY = bound->ellipse.min;
		}
		ellipse_y1 = ellipseY (inner_y, def, acc, 0, y0, y1);
		ellipse_y2 = ellipseY (inner_y, def, acc, 0, y1, altY);
		if (ellipse_y1 == -1.0)
			ellipse_y1 = ellipse_y2;
		if (ellipse_y2 == -1.0)
			ellipse_y2 = ellipse_y1;
	}
	else
	{
		ellipse_y1 = ellipseY (inner_y, def, acc, 0,
							   bound->ellipse.min, bound->ellipse.max);
		ellipse_y2 = ellipse_y1;
	}
	x2 = x1 = innerXfromY (ellipse_y1, def, acc);
	if (ellipse_y1 != ellipse_y2)
		x2 = innerXfromY (ellipse_y2, def, acc);
	if (acc->left.valid && boundedLe (inner_y, bound->left))
	{
		xalt = intersectLine (inner_y, &acc->left);
		if (xalt < x2 && xalt < x1)
			x2 = xalt;
		if (xalt < x1)
			x1 = xalt;
	}
	if (acc->right.valid && boundedLe (inner_y, bound->right))
	{
		xalt = intersectLine (inner_y, &acc->right);
		if (xalt < x2 && xalt < x1)
			x2 = xalt;
		if (xalt < x1)
			x1 = xalt;
	}
	*innerX1p = x1;
	*innerX2p = x2;
}

/*
 * compute various accelerators for an ellipse.  These
 * are simply values that are used repeatedly in
 * the computations
 */
static void computeAcc (struct arc_def *def, struct accelerators *acc)
{
	int	i;
	
	acc->h2 = def->h * def->h;
	acc->w2 = def->w * def->w;
	acc->h4 = acc->h2 * acc->h2;
	acc->w4 = acc->w2 * acc->w2;
	acc->h2mw2 = acc->h2 - acc->w2;
	acc->wh2mw2 = def->w * acc->h2mw2;
	acc->wh4 = def->w * acc->h4;
	acc->tail_y = tailEllipseY (def->w, def->h, def->l);
	for (i = 0; i <= BINARY_TABLE_SIZE; i++)
		acc->innerValid[i] = acc->outerValid[i] = 0;
}

/*
 * compute y value bounds of various portions of the arc,
 * the outer edge, the ellipse and the inner edge.
 */
static void computeBound (struct arc_def *def,struct arc_bound *bound,
						  struct accelerators *acc, miArcFacePtr right,
						  miArcFacePtr left)
{
	double t;
	double innerTaily;
	double tail_y;
	struct bound innerx, outerx;
	struct bound ellipsex;
	
	bound->ellipse.min = Dsin (def->a0) * def->h;
	bound->ellipse.max = Dsin (def->a1) * def->h;
	if (def->a0 == 45 && def->w == def->h)
		ellipsex.min = bound->ellipse.min;
	else
		ellipsex.min = Dcos (def->a0) * def->w;
	if (def->a1 == 45 && def->w == def->h)
		ellipsex.max = bound->ellipse.max;
	else
		ellipsex.max = Dcos (def->a1) * def->w;
	bound->outer.min= outerYfromXY(ellipsex.min, bound->ellipse.min, def, acc);
	bound->outer.max= outerYfromXY(ellipsex.max, bound->ellipse.max, def, acc);
	bound->inner.min= innerYfromXY(ellipsex.min, bound->ellipse.min, def, acc);
	bound->inner.max= innerYfromXY(ellipsex.max, bound->ellipse.max, def, acc);
	/*p_info(PI_TRACE, "bound->outer.min: %.2f, bound->outer.max: %.2f, bound->inner.min: %.2f, bound->inner.max: %.2f\n",
	  bound->outer.min, bound->outer.max, bound->inner.min,bound->inner.max);*/
	outerx.min = outerXfromXY (ellipsex.min, bound->ellipse.min, def, acc);
	outerx.max = outerXfromXY (ellipsex.max, bound->ellipse.max, def, acc);
	innerx.min = innerXfromXY (ellipsex.min, bound->ellipse.min, def, acc);
	innerx.max = innerXfromXY (ellipsex.max, bound->ellipse.max, def, acc);
	/*p_info(PI_TRACE, "outerx.min: %.2f, outerx.max: %.2f, innerx.min: %.2f, innerx.max: %.2f\n",
	  outerx.min, outerx.max, innerx.min, innerx.max);*/
	/*
	  save the line end points for the cap code to use.  Careful here,
	  these are in cartesean coordinates (y increasing upwards)
	  while the cap code uses inverted coordinates (y increasing downwards)
	 */
	if (right)
	{
		right->counterClock.y = bound->outer.min;
		right->counterClock.x = outerx.min;
		right->center.y = bound->ellipse.min;
		right->center.x = ellipsex.min;
		right->clock.y = bound->inner.min;
		right->clock.x = innerx.min;
	}
	if (left) 
	{
		left->clock.y = bound->outer.max;
		left->clock.x = outerx.max;
		left->center.y = bound->ellipse.max;
		left->center.x = ellipsex.max;
		left->counterClock.y = bound->inner.max;
		left->counterClock.x = innerx.max;
	}
	bound->left.min = bound->inner.max;
	bound->left.max = bound->outer.max;
	bound->right.min = bound->inner.min;
	bound->right.max = bound->outer.min;
	computeLine (innerx.min, bound->inner.min, outerx.min, bound->outer.min,
				 &acc->right);
	computeLine (innerx.max, bound->inner.max, outerx.max, bound->outer.max,
				 &acc->left);
	if (bound->inner.min > bound->inner.max)
	{
		t = bound->inner.min;
		bound->inner.min = bound->inner.max;
		bound->inner.max = t;
	}
	tail_y = acc->tail_y;
	if (tail_y > bound->ellipse.max)
		tail_y = bound->ellipse.max;
	else if (tail_y < bound->ellipse.min)
		tail_y = bound->ellipse.min;
	innerTaily = innerYfromY (tail_y, def, acc);
	if (bound->inner.min > innerTaily)
		bound->inner.min = innerTaily;
	if (bound->inner.max < innerTaily)
		bound->inner.max = innerTaily;
}

/*
 * generate the set of spans with
 * the given y coordinate
 */
static void arcSpan (double y, struct arc_def *def, struct arc_bound *bounds,
					 struct accelerators	*acc)
{
	double	innerx1, innerx2, outerx1, outerx2;
	
	if (boundedLe (y, bounds->inner))
	{							/* intersection with inner edge */
		innerXs (y, def, bounds, acc, &innerx1, &innerx2);
	} else
	{							/* * intersection with left face */
		innerx2 = innerx1 = hookX (y, def, bounds, acc, 1);
		if (acc->right.valid && boundedLe (y, bounds->right))
		{
			innerx2 = intersectLine (y, &acc->right);
			if (innerx2 < innerx1)
				innerx1 = innerx2;
		}
	}
	if (boundedLe (y, bounds->outer))
	{							/* intersection with outer edge */
		outerx1 = outerx2 = outerX (y, def, bounds, acc);
	}
	else
	{							/* intersection with right face */
		outerx2 = outerx1 = hookX (y, def, bounds, acc, 0);
		if (acc->left.valid && boundedLe (y, bounds->left))
 		{
			outerx2 = intersectLine (y, &acc->left);
			if (outerx2 < outerx1)
				outerx2 = outerx1;
		}
	}
	/*
	 * there are a very few cases when two spans will be
	 * generated.
	 */
	if (innerx1 <= outerx1 && outerx1 < innerx2 && innerx2 <= outerx2)
 	{
		span (innerx1, outerx1);
		span (innerx2, outerx2);
	} else
		span (innerx1, outerx2);
}

# define SPAN_REALLOC	1024

# define findSpan(y) ((*finalMiny <= (y) && (y) < *finalMaxy) ? \
					  &(*finalSpans)[(y) - *finalMiny] : \
					  realFindSpan (y))

static struct finalSpan ** realFindSpan (int y)
{
	struct finalSpan **newSpans;
	int newSize, newMiny, newMaxy;
	int change;
	int i;
	
	if (y < *finalMiny || y >= *finalMaxy)
	{
		if (y < *finalMiny)
			change = *finalMiny - y;
		else
			change = y - *finalMaxy;
		if (change >= SPAN_REALLOC)
			change += SPAN_REALLOC;
		else
			change = SPAN_REALLOC;
		newSize = *finalSize + change;
		newSpans = (struct finalSpan **) p_alloc 
			(newSize * sizeof (struct finalSpan *));
		if (!newSpans)
		    return (struct finalSpan **)NULL;
		newMiny = *finalMiny;
		newMaxy = *finalMaxy;
		if (y < *finalMiny)
			newMiny = *finalMiny - change;
		else
			newMaxy = *finalMaxy + change;
		if (*finalSpans) {
			memcpy (((char *) newSpans) + (*finalMiny-newMiny) *
					sizeof (struct finalSpan *),
					(char *) *finalSpans, *finalSize *
					sizeof (struct finalSpan *));
			p_free ((char *)*finalSpans);
		}
		if ((i = *finalMiny - newMiny) > 0)
			memset ((char *)newSpans, 0, i * sizeof (struct finalSpan *));
		if ((i = newMaxy - *finalMaxy) > 0)
			memset ((char *)(newSpans + *finalMaxy - newMiny),
					0, i * sizeof (struct finalSpan *));
		*finalSpans = newSpans;
		*finalMaxy = newMaxy;
		*finalMiny = newMiny;
		*finalSize = newSize;
	}
	return &(*finalSpans)[y - *finalMiny];
}

static void newFinalSpan (int y, int xmin, int xmax)
{
	struct finalSpan *x;
	struct finalSpan **f;
	struct finalSpan *oldx;
	struct finalSpan *prev;
	
	f = findSpan (y);
	if (!f)
	    return;
	oldx = 0;
	for (;;)
	{
		prev = 0;
		for (x = *f; x; x=x->next)
		{
			if (x == oldx)
			{
				prev = x;
				continue;
			}
			if (x->min <= xmax && xmin <= x->max)
			{
				if (oldx)
				{
					oldx->min = MIN (x->min, xmin);
					oldx->max = MAX (x->max, xmax);
					if (prev)
						prev->next = x->next;
					else
						*f = x->next;
					--(*nspans);
				}
				else
				{
					x->min = MIN (x->min, xmin);
					x->max = MAX (x->max, xmax);
					oldx = x;
				}
				xmin = oldx->min;
				xmax = oldx->max;
				break;
			}
			prev = x;
		}
		if (!x)
			break;
	}
	if (!oldx)
	{
		x = (struct finalSpan *)allocFinalSpan ();
		if (x)
		{
		    x->min = xmin;
		    x->max = xmax;
		    x->next = *f;
		    *f = x;
		    ++(*nspans);
		}
	}
}

static void deleteFinalSpan (int y, int	xmin, int xmax)
{
	struct finalSpan *x;
	struct finalSpan **f;
	int newmax;
	
	f = findSpan (y);
	if (!f)
	    return;
	for (x = *f; x; x=x->next)
	{
	    if (x->min <= xmin && xmax <= x->max)
		{
			if (x->min == xmin)
				x->min = xmax;
			else if (x->max == xmax)
				x->max = xmin;
			else
			{
				newmax = x->max;
				x->max = xmin;
				newFinalSpan (y, xmax, newmax);
			}
			return;
	    } else if (x->min <= xmax && xmin <= x->max)
		{
			if (x->min <= xmin)
				x->max = xmin;
			else
			{
				x->min = xmax;
				if (x->min > x->max)
					x->min = x->max;
			}
	    }
	}
}

# define SPAN_CHUNK_SIZE    128

struct finalSpanChunk
{
	struct finalSpan	data[SPAN_CHUNK_SIZE];
	struct finalSpanChunk	*next;
};

static struct finalSpanChunk	*chunks;

struct finalSpan *realAllocSpan ()
{
	struct finalSpanChunk *newChunk;
	struct finalSpan *span;
	int i;
	
	newChunk = (struct finalSpanChunk *) p_alloc
		(sizeof (struct finalSpanChunk));
	if (!newChunk)
		return (struct finalSpan *) NULL;
	newChunk->next = chunks;
	chunks = newChunk;
	freeFinalSpans = span = newChunk->data + 1;
	for (i = 1; i < SPAN_CHUNK_SIZE-1; i++)
	{
		span->next = span+1;
		span++;
	}
	span->next = 0;
	span = newChunk->data;
	span->next = 0;
	return span;
}

#ifdef AAA
static void p_cleanFinalSpans (struct finalSpan **fSpan)
{
	struct finalSpan *chunk, *next = NULL;
	
	if (!*fSpan)
		return;
	for (chunk = (*fSpan) -> next; chunk; chunk = next)
	{
		next = chunk->next;
		p_info(PI_TRACE, "Freeing: %d\n", chunk);
		p_free ((char *)chunk);
	}
	p_free((char *)*fSpan);
}
#endif

static void disposeFinalSpans ()
{
	struct finalSpanChunk *chunk, *next = NULL;
	
	for (chunk = chunks; chunk; chunk = next)
	{
		next = chunk->next;
		p_free ((char *)chunk);
	}
	chunks = 0;
	freeFinalSpans = 0;
	p_free((char *)*finalSpans);
	*finalSpans = 0;
}

#ifdef AAA
static void
fillSpans ()
{
	struct finalSpan *span;
	DDXPointPtr xSpan;
	int *xWidth;
	int i;
	struct finalSpan **f;
	int spany;
	DDXPointPtr xSpans;
	int *xWidths;
	
	if (nspans == 0)
		return;
	xSpan = xSpans = (DDXPointPtr) p_alloc (nspans * sizeof (DDXPointRec));
	xWidth = xWidths = (int *) p_alloc (nspans * sizeof (int));
	if (xSpans && xWidths)
	{
	    i = 0;
	    f = finalSpans;
		/*p_info(PI_TRACE, "finalMiny: %d, finalMaxy: %d\n", finalMiny, finalMaxy);*/
	    for (spany = finalMiny; spany < finalMaxy; spany++, f++)
		{
		    for (span = *f; span; span=span->next)
			{
			    if (span->max <= span->min)
				{
					/*p_info(PI_TRACE, "i: %d, DEAD span -> min: %d, span -> max: %d, spany: %d, xWidth: %d\n",
					  i, span -> min, span -> max, spany, *xWidth);*/
				    continue;
				}
			    xSpan->x = span->min;
			    xSpan->y = spany;
			    *xWidth = span->max - span->min;
				/*p_info(PI_TRACE, "i: %d, xSpan -> x: %d, xSpan -> y: %d, xWidth: %d\n",
				  i, xSpan -> x, xSpan -> y, *xWidth); */
				++xWidth;
				++xSpan;
				++i;
			}
		}
		{
			int k;
			DDXPointPtr xSp;
		
			xWidth = xWidths;
			xSp = xSpans;
			/*	for (k = 0; k < i; k += 2, xSp += 2, xWidth += 2)
				p_info(PI_TRACE, "k: %d, xSp -> x: %d, xSp -> y: %d, xWidth: %d\n",
				k, xSp -> x, xSp -> y, *xWidth);*/
			if (i % 2)
			{					/* Even number of points */
				k = k - 3;
				xWidth = xWidth - 3;
				xSp = xSp - 3;
			}
			else
			{					/* Odd number of points */
				k = k - 1;
				xWidth = xWidth - 1;
				xSp = xSp - 1;
			}
#ifdef AAA
			for (; k > 0; k -= 2, xSp -= 2, xWidth -= 2)
				p_info(PI_TRACE, "k: %d, xSp -> x: %d, xSp -> y: %d, xWidth: %d\n",
					   k, xSp -> x, xSp -> y, *xWidth);
#endif
		}
#ifdef AAA
		(*pGC->ops->FillSpans) (pDrawable, pGC, i, xSpans, xWidths, TRUE);
#endif
	}
	reorganize_pts(xSpans, xWidths, i);
	disposeFinalSpans ();
	p_free ((char *)xSpans);
	p_free ((char *)xWidths);
	finalMiny = 0;
	finalMaxy = 0;
	finalSize = 0;
	nspans = 0;
}
#endif

static void span (double left, double right)
{
	int mask = quadrantMask, bit;
	double min = 0., max = 0., y = 0.;
	int xmin, xmax, spany;
	
	while (mask) 
	{
		bit = lowbit (mask);
		mask &= ~bit;
		switch (bit)
		{
		  case 1:
			min = left;
			max = right;
			y = spanY;
			finalSpans = &qfinalSpans[0];
			nspans = &qspans[0];
			finalMiny = &qfinalMiny[0];
			finalMaxy = &qfinalMaxy[0];
			finalSize = &qfinalSize[0];
			break;
		  case 2:
			min = -right;
			max = -left;
			y = spanY;
			finalSpans = &qfinalSpans[1];
			nspans = &qspans[1];
			finalMiny = &qfinalMiny[1];
			finalMaxy = &qfinalMaxy[1];
			finalSize = &qfinalSize[1];
			break;
		  case 4:
			min = -right;
			max = -left;
			y = -spanY;
			finalSpans = &qfinalSpans[2];
			nspans = &qspans[2];
			finalMiny = &qfinalMiny[2];
			finalMaxy = &qfinalMaxy[2];
			finalSize = &qfinalSize[2];
			break;
		  case 8:
			min = left;
			max = right;
			y = -spanY;
			finalSpans = &qfinalSpans[3];
			nspans = &qspans[3];
			finalMiny = &qfinalMiny[3];
			finalMaxy = &qfinalMaxy[3];
			finalSize = &qfinalSize[3];
			break;
		  default:
			p_info(PI_ELOG, "draw_arc.c: illegal quadrant mask bit %d\n", bit);
		}
		xmin = ICEIL (min + arcXcenter) + arcXoffset;
		xmax = ICEIL (max + arcXcenter) + arcXoffset;
		spany = ICEIL (arcYcenter - y) + arcYoffset;
		
		if (xmax > xmin)
			newFinalSpan (spany, xmin, xmax);
	}
}

static void unspan (double left, double right)
{
	int	mask = quadrantMask, bit;
	double	min = 0., max = 0., y = 0.;
	int	xmin, xmax, spany;
	
	while (mask)
	{
		bit = lowbit (mask);
		mask &= ~bit;
		switch (bit) {
		  case 1:
			min = left;
			max = right;
			y = spanY;
			break;
		  case 2:
			min = -right;
			max = -left;
			y = spanY;
			break;
		  case 4:
			min = -right;
			max = -left;
			y = -spanY;
			break;
		  case 8:
			min = left;
			max = right;
			y = -spanY;
			break;
		  default:
			p_info(PI_ELOG, "draw_arc.c: illegal quadrant mask bit %d\n", bit);
		}
		xmin = ICEIL (min + arcXcenter) + arcXoffset;
		xmax = ICEIL (max + arcXcenter) + arcXoffset;
		spany = ICEIL (arcYcenter - y) + arcYoffset;
		
		if (xmax > xmin)
			deleteFinalSpan (spany, xmin, xmax);
	}
}

/* split an arc into pieces which are scan-converted in the first-quadrant
   and mirrored into position. This is necessary as the scan-conversion code
   can only deal with arcs completely contained in the first quadrant.
   Npte - right and left save end line points */
static void drawArc (int x0, int y0, int w, int h, int l, int a0, int a1,
					 miArcFacePtr right, miArcFacePtr left)
{
	struct arc_def def;
	struct accelerators acc;
	int startq, endq, curq;
	int rightq = 0, leftq = 0, righta = 0, lefta = 0;
	miArcFacePtr passRight, passLeft;
	int q0 = 0, q1 = 0, mask;
	struct band
	{
		int	a0, a1;
		int	mask;
	}	band[5], sweep[20];
	int bandno, sweepno;
	int i, j;
	int flipRight = 0, flipLeft = 0;			
	int copyEnd = 0;
	
	/*p_info(PI_TRACE, "w: %d, h: %d\n", w, h);*/
	def.w = ((double) w) / 2;
	def.h = ((double) h) / 2;
	arcXoffset = x0;
	arcYoffset = y0;
	/*p_info(PI_TRACE, "arcXoffset: %d, arcYoffset: %d\n", arcXoffset, arcYoffset);*/
	arcXcenter = def.w;
	arcYcenter = def.h;
	/*p_info(PI_TRACE, "arcXcenter: %.2f, arcYcenter: %.2f\n", arcXcenter, arcYcenter);*/
	def.l = (double) l;
	if (l == 0)
		def.l = 1.0;
	if (a1 < a0)
		a1 += 360 * 64;
	startq = a0 / (90 * 64);
	endq = (a1-1) / (90 * 64);
	bandno = 0;
	curq = startq;
	for (;;)
	{
		switch (curq)
		{
		  case 0:
			if (a0 > 90 * 64)
				q0 = 0;
			else
				q0 = a0;
			if (a1 < 360 * 64)
				q1 = MIN (a1, 90 * 64);
			else
				q1 = 90 * 64;
			if (curq == startq && a0 == q0)
			{
				righta = q0;
				rightq = curq;
			}
			if (curq == endq && a1 == q1)
			{
				lefta = q1;
				leftq = curq;
			}
			break;
		  case 1:
			if (a1 < 90 * 64)
				q0 = 0;
			else
				q0 = 180 * 64 - MIN (a1, 180 * 64);
			if (a0 > 180 * 64)
				q1 = 90 * 64;
			else
				q1 = 180 * 64 - MAX (a0, 90 * 64);
			if (curq == startq && 180 * 64 - a0 == q1)
			{
				righta = q1;
				rightq = curq;
			}
			if (curq == endq && 180 * 64 - a1 == q0)
			{
				lefta = q0;
				leftq = curq;
			}
			break;
		  case 2:
			if (a0 > 270 * 64)
				q0 = 0;
			else
				q0 = MAX (a0, 180 * 64) - 180 * 64;
			if (a1 < 180 * 64)
				q1 = 90 * 64;
			else
				q1 = MIN (a1, 270 * 64) - 180 * 64;
			if (curq == startq && a0 - 180*64 == q0)
			{
				righta = q0;
				rightq = curq;
			}
			if (curq == endq && a1 - 180 * 64 == q1)
			{
				lefta = q1;
				leftq = curq;
			}
			break;
		  case 3:
			if (a1 < 270 * 64)
				q0 = 0;
			else
				q0 = 360 * 64 - MIN (a1, 360 * 64);
			q1 = 360 * 64 - MAX (a0, 270 * 64);
			if (curq == startq && 360 * 64 - a0 == q1)
			{
				righta = q1;
				rightq = curq;
			}
			if (curq == endq && 360 * 64 - a1 == q0)
			{
				lefta = q0;
				leftq = curq;
			}
			break;
		}
		band[bandno].a0 = q0;
		band[bandno].a1 = q1;
		band[bandno].mask = 1 << curq;
		bandno++;
		if (curq == endq)
			break;
		curq++;
		if (curq == 4)
		{
			a0 = 0;
			a1 -= 360 * 64;
			curq = 0;
			endq -= 4;
		}
	}
	sweepno = 0;
	for (;;)
	{
		q0 = 90 * 64;
		mask = 0;
		/* find left-most point  */
		for (i = 0; i < bandno; i++)
			if (band[i].a0 < q0)
			{
				q0 = band[i].a0;
				q1 = band[i].a1;
				mask = band[i].mask;
			}
		if (!mask)
			break;
		/* * locate next point of change */
		for (i = 0; i < bandno; i++)
			if (!(mask & band[i].mask))
			{
				if (band[i].a0 == q0)
				{
					if (band[i].a1 < q1)
						q1 = band[i].a1;
					mask |= band[i].mask;
 				}
				else if (band[i].a0 < q1)
					q1 = band[i].a0;
			}
		/* create a new sweep */
		sweep[sweepno].a0 = q0;
		sweep[sweepno].a1 = q1;
		sweep[sweepno].mask = mask;
		sweepno++;
		/*  subtract the sweep from the affected bands */
		for (i = 0; i < bandno; i++)
		{
			if (band[i].a0 == q0)
			{
				band[i].a0 = q1;
				/* check if this band is empty */
				if (band[i].a0 == band[i].a1)
					band[i].a1 = band[i].a0 = 90 * 64;
			}
		}
	}
	computeAcc (&def, &acc);
	for (j = 0; j < sweepno; j++)
	{
		mask = sweep[j].mask;
		passRight = passLeft = 0;
 		if (mask & (1 << rightq))
		{
			if (sweep[j].a0 == righta)
				passRight = right;
			if (sweep[j].a1 == righta)
			{
				passLeft = right;
				flipRight = 1;
			}
		}
		if (mask & (1 << leftq))
		{
			if (sweep[j].a0 == lefta)
			{
				if (passRight)
					copyEnd = 1;
				passRight = left;
				flipLeft = 1;
			}
			if (sweep[j].a1 == lefta)
			{
				if (passLeft)
					copyEnd = 1;
				passLeft = left;
			}
		}
		drawQuadrant (&def, &acc, sweep[j].a0, sweep[j].a1, mask, 
					  passRight, passLeft);
	}
	/*
	 * when copyEnd is set, both ends of the arc were computed
	 * at the same time; drawQuadrant only takes one end though,
	 * so the left end will be the only one holding the data.  Copy
	 * it from there.
	 */
	if (copyEnd)
		*right = *left;
	/* mirror the coordinates generated for the faces of the arc */
	if (right)
	{
		mirrorSppPoint (rightq, &right->clock);
		mirrorSppPoint (rightq, &right->center);
		mirrorSppPoint (rightq, &right->counterClock);
		if (flipRight)
		{
			SppPointRec	temp;
			temp = right->clock;
			right->clock = right->counterClock;
			right->counterClock = temp;
		}
	}
	if (left)
	{
		mirrorSppPoint (leftq,  &left->counterClock);
		mirrorSppPoint (leftq,  &left->center);
		mirrorSppPoint (leftq,  &left->clock);
		if (flipLeft)
		{
			SppPointRec	temp;
			temp = left->clock;
			left->clock = left->counterClock;
			left->counterClock = temp;
		}
	}
}

static void drawQuadrant (struct arc_def *def, struct accelerators *acc,
						  int a0, int a1, int mask, miArcFacePtr right,
						  miArcFacePtr left)
{
	struct arc_bound bound;
	double miny, maxy, y;
	int minIsInteger;
	double fromInt;
	
	def->a0 = ((double) a0) / 64.0;
	def->a1 = ((double) a1) / 64.0;
	fromInt = def->h - floor (def->h);
	computeBound (def, &bound, acc, right, left);
	y = fmin (bound.inner.min, bound.outer.min);
	miny = ICEIL(y - fromInt) + fromInt;
	minIsInteger = y == miny;
	y = fmax (bound.inner.max, bound.outer.max);
	maxy = floor (y - fromInt) + fromInt;
	/*p_info(PI_TRACE, "miny: %.2f, maxy: %.2f\n", miny, maxy);*/
	for (y = miny; y <= maxy; y = y + 1.0)
	{
		if (y == miny && minIsInteger)
			quadrantMask = mask & 0xc;
		else
			quadrantMask = mask;
		spanY = y;
		arcSpan (y, def, &bound, acc);
	}
	/*
	  add the pixel at the top of the arc if this segment terminates exactly
	  at the top of the arc, and the outside point is exactly an integer
	  (width is integral and (line width/2) is integral)
	  */
	if (a1 == 90 * 64 && (mask & 1) && def->w == floor (def->w) &&
	    def->l/2 == floor (def->l/2))
 	{
		quadrantMask = 1;
		spanY = def->h + def->l/2;
		span (0.0, 1.0);
		spanY = def->h - def->l/2;
		unspan (0.0, 1.0);
	}
}

/*
 * using newtons method and a binary search, compute the ellipse y value
 * associated with the given edge value (either outer or
 * inner).  This is the heart of the scan conversion code and
 * is generally called three times for each span.  It should
 * be optimized further.
 *
 * the general idea here is to solve the equation:
 *
 *                               w^2 * l
 *   edge_y = y + y * -------------------------------
 *                    2 * sqrt (x^2 * h^4 + y^2 * w^4)
 *
 * for y.  (x, y) is a point on the ellipse, so x can be
 * found from y:
 *
 *                ( h^2 - y^2 )
 *   x = w * sqrt ( --------- )
 *                (    h^2    )
 *
 * The information given at the start of the search
 * is two points which are known to bound the desired
 * solution, a binary search starts with these two points
 * and converges close to a solution, which is then
 * refined with newtons method.  Newtons method
 * cannot be used in isolation as it does not always
 * converge to the desired solution without a close
 * starting point, the binary search simply provides
 * that point.  Increasing the solution interval for
 * the binary search will certainly speed up the
 * solution, but may generate a range which causes
 * the newtons method to fail.  This needs study.
 */

# define binaryIndexFromY(y, def)	(((y) / (def)->h) * ((double) BINARY_TABLE_SIZE))
# define yFromBinaryIndex(i, def)	((((double) i) / ((double) BINARY_TABLE_SIZE)) * (def)->h)

#ifdef notdef

static double binaryValue (int i, struct arc_def *def,
						   struct accelerators *acc, char *valid,
						   double *table, double (*f)())
{
	if (!valid[i])
	{
		valid[i] = 1;
		table[i] = f (yFromBinaryIndex (i, def), def, acc);
	}
	return table[i];
}
#else

# define binaryValue(i, def, acc, valid, table, f)\
(valid[i] ? table[i] : (valid[i] = 1, table[i] = f (yFromBinaryIndex (i, def), def, acc)))
#endif

static double ellipseY (double edge_y, struct arc_def *def,
						struct accelerators *acc, int outer,
						double y0, double y1)
{
	double w, l, h2, h4, w2, w4, x, y2;
	double newtony, binaryy;
	double value0, value1;
	double newtonvalue, binaryvalue;
	double minY, maxY;
	double binarylimit;
	double (*f)();
	int index0, index1, newindex;
	char *valid;
	double *table;
	BOOLEAN result1, result2;
	
	/* compute some accelerators */
	w = def->w;
	if (outer)
	{
		f = outerYfromY;
		l = def->l;
		table = acc->outerTable;
		valid = acc->outerValid;
	}
	else
	{
		f = innerYfromY;
		l = -def->l;
		table = acc->innerTable;
		valid = acc->innerValid;
	}
	h2 = acc->h2;
	h4 = acc->h4;
	w2 = acc->w2;
	w4 = acc->w4;
	/* make sure the arguments are in the right order */
	if (y0 > y1)
	{
		binaryy = y0;
		y0 = y1;
		y1 = binaryy;
	}
	maxY = y1;
	minY = y0;
	index0 = binaryIndexFromY (y0, def);
	index1 = binaryIndexFromY (y1, def);
	if (index0 == index1)
	{
		value0 = f (y0, def, acc) - edge_y;
		if (value0 == 0)
			return y0;
		value1 = f (y1, def, acc) - edge_y;
		if (value1 == 0)
			return y1;
		if ((result1 = (value0 > 0)) == (result2 = (value1 > 0)))
			return -1.0;
	}
	else
	{
		/*  round index0 up, index1 down */
		index0++;
		value0 = binaryValue (index0, def, acc, valid, table, f) - edge_y;
		if (value0 == 0)
			return yFromBinaryIndex (index0, def);
		value1 = binaryValue (index1, def, acc, valid, table, f) - edge_y;
		if (value1 == 0)
			return yFromBinaryIndex (index1, def);
		/* make sure the result lies between the restricted end points */
		if ((result1 = (value0 > 0)) == (result2 = (value1 > 0)))
		{
			if (y0 == y1)
				return -1.0;
			binaryvalue = f(y0, def, acc) - edge_y;
			if ((result1 = (binaryvalue > 0)) != (result2 = (value0 > 0)))
			{
				/* restrict the search to the small portion at the begining */
				index1 = index0;
				value1 = value0;
				value0 = binaryvalue;
				y1 = yFromBinaryIndex (index0, def);
			}
			else
			{
				binaryvalue = f(y1, def, acc) - edge_y;
				if ((result1 = (binaryvalue > 0)) == (result2 = (value1 > 0)))
					return -1.0; /* an illegal value */
				/* restrict the search to the small portion at the end */
				index0 = index1;
				value0 = value1;
				value1 = binaryvalue;
				y0 = yFromBinaryIndex (index1, def);
			}
		}
		else
		{
			/* restrict the search to the inside portion */
			y0 = yFromBinaryIndex (index0, def);
			y1 = yFromBinaryIndex (index1, def);
		}
	}
	binarylimit = (value1 - value0) / 25.0;
	binarylimit = fabs (binarylimit);
	if (binarylimit < BINARY_LIMIT)
		binarylimit = BINARY_LIMIT;
	/* binary search for a while */
	while (fabs (value1) > binarylimit)
	{
		if (y0 == y1 || value0 == value1)
			return -1.0;
		if (index1 > index0 + 1)
		{
			newindex = (index1 + index0) / 2;
			binaryy = yFromBinaryIndex (newindex, def);
			binaryvalue =
				binaryValue (newindex, def, acc, valid, table, f) - edge_y;
		}
		else
		{
			binaryy = (y0 + y1) / 2;
			/* inline expansion of the function */
			y2 = binaryy*binaryy;
			x = w * Sqrt ((h2 - (y2)) / h2);
			binaryvalue = ( binaryy + (binaryy * w2 * l) /
							(2 * Sqrt (x*x * h4 + y2 * w4))) - edge_y;
			newindex = -1;
		}
		if ((result1 = (binaryvalue > 0)) == (result2 = (value0 > 0)))
		{
			y0 = binaryy;
			value0 = binaryvalue;
			if (newindex > 0)
				index0 = newindex;
		}
		else
		{
			y1 = binaryy;
			value1 = binaryvalue;
			if (newindex > 0)
				index1 = newindex;
		}
	}
	/* clean up the estimate with newtons method */
	while (fabs (value1) > NEWTON_LIMIT)
	{
		newtony = y1 - value1 * (y1 - y0) / (value1 - value0);
		if (newtony > maxY)
			newtony = maxY;
		if (newtony < minY)
			newtony = minY;
		/* inline expansion of the function */
		y2 = newtony*newtony;
		x = w * Sqrt ((h2 - (y2)) / h2);
		newtonvalue = ( newtony + (newtony * w2 * l) /
						(2 * Sqrt (x*x * h4 + y2 * w4))) - edge_y;
		if (newtonvalue == 0)
			return newtony;
		if (fabs (value0) > fabs (value1))
		{
			y0 = newtony;
			value0 = newtonvalue;
		}
		else
		{
			y1 = newtony;
			value1 = newtonvalue;
		}
	}
	return y1;
}

static void addCap (miArcCapPtr *capsp, int *ncapsp, int *sizep, int end,
					int arcIndex)
{
	int newsize;
	miArcCapPtr	cap;
	
	if (*ncapsp == *sizep)
	{
	    newsize = *sizep + ADD_REALLOC_STEP;
	    cap = (miArcCapPtr) p_remalloc ((char *)*capsp, 
										(newsize - ADD_REALLOC_STEP) *
										sizeof (**capsp),
										newsize * sizeof (**capsp));
	    if (!cap)
			return;
	    *sizep = newsize;
	    *capsp = cap;
	}
	cap = &(*capsp)[*ncapsp];
	cap->end = end;
	cap->arcIndex = arcIndex;
	++*ncapsp;
}

static void addJoin (miArcJoinPtr *joinsp, int *njoinsp, int *sizep, int end0,
					 int index0, int phase0, int end1, int index1, int phase1)
{
	int newsize;
	miArcJoinPtr join;
	
	if (*njoinsp == *sizep)
	{
	    newsize = *sizep + ADD_REALLOC_STEP;
	    join = (miArcJoinPtr) p_remalloc ((char *)*joinsp,
										  (newsize - ADD_REALLOC_STEP) *
										  sizeof (**joinsp),
										  newsize * sizeof (**joinsp));
	    if (!join)
			return;
	    *sizep = newsize;
	    *joinsp = join;
	}
	join = &(*joinsp)[*njoinsp];
	join->end0 = end0;
	join->arcIndex0 = index0;
	join->phase0 = phase0;
	join->end1 = end1;
	join->arcIndex1 = index1;
	join->phase1 = phase1;
	++*njoinsp;
}

static miArcDataPtr addArc (miArcDataPtr *arcsp, int *narcsp, int *sizep,
							xArc xarc)
{
	int newsize;
	miArcDataPtr arc;
	
	if (*narcsp == *sizep)
	{
	    newsize = *sizep + ADD_REALLOC_STEP;
	    arc =(miArcDataPtr) p_remalloc ((char *)*arcsp, 
										(newsize - ADD_REALLOC_STEP) *
										sizeof (**arcsp),
										newsize * sizeof (**arcsp));
	    if (!arc)
			return (miArcDataPtr)NULL;
	    *sizep = newsize;
	    *arcsp = arc;
	}
	arc = &(*arcsp)[*narcsp];
	arc->arc = xarc;
	++*narcsp;
	return arc;
}

static void miFreeArcs(miPolyArcPtr arcs)
{
	int iphase;
	
	for (iphase = 0; iphase >= 0; iphase--)
	{
	    if (arcs[iphase].narcs > 0)
			p_free((char *)arcs[iphase].arcs);
	    if (arcs[iphase].njoins > 0)
			p_free((char *)arcs[iphase].joins);
	    if (arcs[iphase].ncaps > 0)
			p_free((char *)arcs[iphase].caps);
	}
	p_free((char *)arcs);
}

/* this routine is a bit gory */
static miPolyArcPtr
miComputeArcs (xArc *parcs, int narcs)
{
	miPolyArcPtr arcs;
	int start, i, j, k = 0, nexti, nextk = 0;
	int joinSize[2];
	int capSize[2];
	int arcSize[2];
	int angle2;
	double a0, a1;
	struct arcData *data;
	miArcDataPtr arc;
	int iphase, prevphase, joinphase;
	int arcsJoin;
	int iphaseStart;
	
	data = (struct arcData *) p_alloc (narcs * sizeof (struct arcData));
	if (!data)
	    return (miPolyArcPtr)NULL;
	arcs = (miPolyArcPtr) p_alloc (sizeof(*arcs) * 1);
	if (!arcs)
	{
	    p_free((char *)data);
	    return (miPolyArcPtr)NULL;
	}
	for (i = 0; i < narcs; i++)
	{
		a0 = todeg (parcs[i].angle1);
		angle2 = parcs[i].angle2;
		if (angle2 > FULLCIRCLE)
			angle2 = FULLCIRCLE;
		else if (angle2 < -FULLCIRCLE)
			angle2 = -FULLCIRCLE;
		data[i].selfJoin = angle2 == FULLCIRCLE || angle2 == -FULLCIRCLE;
		a1 = todeg (parcs[i].angle1 + angle2);
		data[i].x0 =
			parcs[i].x + (double) parcs[i].width / 2 * (1 + miDcos (a0));
		data[i].y0 =
			parcs[i].y + (double) parcs[i].height / 2 * (1 - miDsin (a0));
		data[i].x1 =
			parcs[i].x + (double) parcs[i].width / 2 * (1 + miDcos (a1));
		data[i].y1 =
			parcs[i].y + (double) parcs[i].height / 2 * (1 - miDsin (a1));
	}
	for (iphase = 0; iphase < 1; iphase++)
	{
		arcs[iphase].njoins = 0;
		arcs[iphase].joins = 0;
		joinSize[iphase] = 0;
		arcs[iphase].ncaps = 0;
		arcs[iphase].caps = 0;
		capSize[iphase] = 0;
		arcs[iphase].narcs = 0;
		arcs[iphase].arcs = 0;
		arcSize[iphase] = 0;
	}
	iphase = 0;
	iphaseStart = iphase;
	for (i = narcs - 1; i >= 0; i--)
	{
		j = i + 1;
		if (j == narcs)
			j = 0;
		if (data[i].selfJoin || (UNEQUAL (data[i].x1, data[j].x0) ||
								 UNEQUAL (data[i].y1, data[j].y0)))
 		{
			if (iphase == 0)
				addCap (&arcs[iphase].caps, &arcs[iphase].ncaps,
						&capSize[iphase], RIGHT_END, 0);
			break;
		}
	}
	start = i + 1;
	if (start == narcs)
		start = 0;
	i = start;
	for (;;)
	{
		j = i + 1;
		if (j == narcs)
			j = 0;
		nexti = i+1;
		if (nexti == narcs)
			nexti = 0;
		arc = addArc (&arcs[iphase].arcs, &arcs[iphase].narcs,
 				      &arcSize[iphase], parcs[i]);
		if (!arc)
		    goto arcfail;
		arc->join = arcs[iphase].njoins;
		arc->cap = arcs[iphase].ncaps;
		arc->selfJoin = data[i].selfJoin;
		prevphase = iphase;
		if (prevphase == 0)
			k = arcs[prevphase].narcs - 1;
		if (iphase == 0)
			nextk = arcs[iphase].narcs;
		if (nexti == start)
			nextk = 0;
		arcsJoin = narcs > 1 && ISEQUAL (data[i].x1, data[j].x0) &&
			ISEQUAL (data[i].y1, data[j].y0) &&
				!data[i].selfJoin && !data[j].selfJoin;
		if (arc)
		{
			if (arcsJoin)
				arc->render = 0;
			else
				arc->render = 1;
		}
		if (arcsJoin &&
		    (prevphase == 0) && (iphase == 0))
 		{
			joinphase = iphase;
			if (joinphase == 0)
			{
				addJoin (&arcs[joinphase].joins, &arcs[joinphase].njoins,
						 &joinSize[joinphase], LEFT_END, k, prevphase,
						 RIGHT_END, nextk, iphase);
				arc->join = arcs[prevphase].njoins;
			}
		}
		else
		{
			/* cap the left end of this arc unless it joins itself */
			if ((prevphase == 0) && !arc->selfJoin)
			{
				addCap (&arcs[prevphase].caps, &arcs[prevphase].ncaps,
						&capSize[prevphase], LEFT_END, k);
				arc->cap = arcs[prevphase].ncaps;
			}
			nextk = arcs[iphase].narcs;
			if (nexti == start)
			{
				nextk = 0;
				iphase = iphaseStart;
			}
			/*
			  cap the right end of the next arc.  If the next arc is actually
			  the first arc, only cap it if it joins with this arc.  This case
			  will occur when the final dash segment of an on/off dash is off.
			  Of course, this  cap will be drawn at a strange time, but that
			  hardly matters...
			  */
			if ((iphase == 0) && (nexti != start))
				addCap (&arcs[iphase].caps, &arcs[iphase].ncaps,
						&capSize[iphase], RIGHT_END, nextk);
		}
		i = nexti;
		if (i == start)
			break;
	}
	/* make sure the last section is rendered */
	for (iphase = 0; iphase < 1; iphase++)
		if (arcs[iphase].narcs > 0)
		{
			arcs[iphase].arcs[arcs[iphase].narcs-1].render = 1;
			arcs[iphase].arcs[arcs[iphase].narcs-1].join = arcs[iphase].njoins;
			arcs[iphase].arcs[arcs[iphase].narcs-1].cap = arcs[iphase].ncaps;
		}
	p_free((char *)data);
	return arcs;
  arcfail:
	miFreeArcs(arcs);
	p_free((char *)data);
	return (miPolyArcPtr)NULL;
}

#ifdef WE_DO_NOT_CARE_ABOUT_CAP
static double angleBetween (SppPointRec	center, SppPointRec	point1,
							SppPointRec	point2)
{
	double	a1, a2, a;
	
	/*reflect from X coordinates back to ellipse
	  coordinates -- y increasing upwards */
	a1 = miDatan2 (- (point1.y - center.y), point1.x - center.x);
	a2 = miDatan2 (- (point2.y - center.y), point2.x - center.x);
	a = a2 - a1;
	if (a <= -180.0)
		a += 360.0;
	else if (a > 180.0)
		a -= 360.0;
	return a;
}

static translateBounds (miArcFacePtr b, int x, int y, double fx, double fy)
{
	b->clock.x -= x + fx;
	b->clock.y -= y + fy;
	b->center.x -= x + fx;
	b->center.y -= y + fy;
	b->counterClock.x -= x + fx;
	b->counterClock.y -= y + fy;
}

static int miArcJoin(int line_width, int joinStyle, miArcFacePtr pLeft,
					 miArcFacePtr pRight, int xOrgLeft, int yOrgLeft,
					 double xFtransLeft, double yFtransLeft,
					 int xOrgRight, int yOrgRight, double xFtransRight,
					 double yFtransRight)
{
	SppPointRec center, corner, otherCorner;
	SppPointRec poly[5], e;
	SppPointPtr pArcPts;
	int cpt;
	SppArcRec arc;
	miArcFaceRec Right, Left;
	int polyLen;
	int xOrg, yOrg;
	double xFtrans, yFtrans;
	double a;
	double ae, ac2, ec2, bc2, de;
	double width;
	
	xOrg = (xOrgRight + xOrgLeft) / 2;
	yOrg = (yOrgRight + yOrgLeft) / 2;
	xFtrans = (xFtransLeft + xFtransRight) / 2;
	yFtrans = (yFtransLeft + yFtransRight) / 2;
	Right = *pRight;
	translateBounds (&Right, xOrg - xOrgRight, yOrg - yOrgRight,
					 xFtrans - xFtransRight, yFtrans - yFtransRight);
	Left = *pLeft;
	translateBounds (&Left, xOrg - xOrgLeft, yOrg - yOrgLeft,
					 xFtrans - xFtransLeft, yFtrans - yFtransLeft);
	pRight = &Right;
	pLeft = &Left;
	
	if (pRight->clock.x == pLeft->counterClock.x &&
	    pRight->clock.y == pLeft->counterClock.y)
		return;
	center = pRight->center;
	if (0 <= (a = angleBetween (center, pRight->clock, pLeft->counterClock))
 	    && a <= 180.0)
 	{
		corner = pRight->clock;
		otherCorner = pLeft->counterClock;
	}
	else
	{
		a = angleBetween (center, pLeft->clock, pRight->counterClock);
		corner = pLeft->clock;
		otherCorner = pRight->counterClock;
	}
	switch (joinStyle)
	{
	  case JoinRound:
		width = line_width;
		arc.x = center.x - width/2;
		arc.y = center.y - width/2;
		arc.width = width;
		arc.height = width;
		arc.angle1 = -miDatan2 (corner.y - center.y, corner.x - center.x);
		arc.angle2 = a;
		pArcPts = (SppPointPtr) p_alloc (3 * sizeof (SppPointRec));
		if (!pArcPts)
		    return;
		pArcPts[0].x = otherCorner.x;
		pArcPts[0].y = otherCorner.y;
		pArcPts[1].x = center.x;
		pArcPts[1].y = center.y;
		pArcPts[2].x = corner.x;
		pArcPts[2].y = corner.y;
		if( cpt = miGetArcPts(&arc, 3, &pArcPts))
		{
		/* by drawing with miFillSppPoly and setting the endpoints of the arc
		 * to be the corners, we assure that the cap will meet up with the
		 * rest of the line */
		/*p_info(PI_TRACE, "A - miFillSppPoly\n");*/
		/*miFillSppPoly(pDraw, pGC, cpt, pArcPts, xOrg, yOrg, xFtrans, yFtrans);*/
		}
		p_free((char *)pArcPts);
		return;
	  case JoinMiter:
		/* don't miter arcs with less than 11 degrees between them */
		if (a < 169.0)
		{
			poly[0] = corner;
			poly[1] = center;
			poly[2] = otherCorner;
			bc2 = (corner.x - otherCorner.x) * (corner.x - otherCorner.x) +
				(corner.y - otherCorner.y) * (corner.y - otherCorner.y);
			ec2 = bc2 / 4;
			ac2 = (corner.x - center.x) * (corner.x - center.x) +
				(corner.y - center.y) * (corner.y - center.y);
			ae = sqrt (ac2 - ec2);
			de = ec2 / ae;
			e.x = (corner.x + otherCorner.x) / 2;
			e.y = (corner.y + otherCorner.y) / 2;
			poly[3].x = e.x + de * (e.x - center.x) / ae;
			poly[3].y = e.y + de * (e.y - center.y) / ae;
			poly[4] = corner;
			polyLen = 5;
			break;
		}
	  case JoinBevel:
		poly[0] = corner;
		poly[1] = center;
		poly[2] = otherCorner;
		poly[3] = corner;
		polyLen = 4;
		break;
	}
	/*p_info(PI_TRACE, "B - miFillSppPoly\n");*/
	/*	miFillSppPoly (pDraw, pGC, polyLen, poly, xOrg, yOrg, xFtrans, yFtrans);*/
}

/*ARGSUSED*/
static void miArcCap (int line_width, int capStyle, miArcFacePtrpFace,int end,
					  int xOrg, int yOrg, double xFtrans, double yFtrans)
{
	SppPointRec	corner, otherCorner, center, endPoint, poly[5];
	
	corner = pFace->clock;
	otherCorner = pFace->counterClock;
	center = pFace->center;
	switch (capStyle)
	{
	  case CapProjecting:
		poly[0].x = otherCorner.x;
		poly[0].y = otherCorner.y;
		poly[1].x = corner.x;
		poly[1].y = corner.y;
		poly[2].x = corner.x - (center.y - corner.y);
		poly[2].y = corner.y + (center.x - corner.x);
		poly[3].x = otherCorner.x - (otherCorner.y - center.y);
		poly[3].y = otherCorner.y + (otherCorner.x - center.x);
		poly[4].x = otherCorner.x;
		poly[4].y = otherCorner.y;
		/*p_info(PI_TRACE, "C - miFillSppPoly\n");*/
		/*		miFillSppPoly (pDraw, pGC, 5, poly, xOrg, yOrg, xFtrans, yFtrans);*/
		break;
	  case CapRound:
		/* miRoundCap just needs these to be unequal. */
		endPoint = center;
		endPoint.x = endPoint.x + 100;
		miRoundCap (line_width, center, endPoint, corner, otherCorner, 0,
					-xOrg, -yOrg, xFtrans, yFtrans);
		break;
	}
}

/* MIROUNDCAP -- a private helper function
 * Put Rounded cap on end. pCenter is the center of this end of the line
 * pEnd is the center of the other end of the line. pCorner is one of the
 * two corners at this end of the line.  
 * NOTE:  pOtherCorner must be counter-clockwise from pCorner.
 */
/*ARGSUSED*/
static void miRoundCap(int line_width, SppPointRec pCenter, SppPointRec pEnd,
					   SppPointRec pCorner, SppPointRecpOtherCorner,
					   int fLineEnd, int xOrg, int yOrg, double xFtrans,
					   double yFtrans)
{
    int cpt;
    double width;
    double miDatan2 ();
    SppArcRec arc;
    SppPointPtr pArcPts;
	
    width = line_width;
    arc.x = pCenter.x - width/2;
    arc.y = pCenter.y - width/2;
    arc.width = width;
    arc.height = width;
    arc.angle1 = -miDatan2 (pCorner.y - pCenter.y, pCorner.x - pCenter.x);
    if(PTISEQUAL(pCenter, pEnd))
		arc.angle2 = - 180.0;
    else
	{
		arc.angle2 = -miDatan2 (pOtherCorner.y - pCenter.y, pOtherCorner.x - pCenter.x) - arc.angle1;
		if (arc.angle2 < 0)
			arc.angle2 += 360.0;
    }
    pArcPts = (SppPointPtr) NULL;
    if( cpt = miGetArcPts(&arc, 0, &pArcPts))
    {
		/* by drawing with miFillSppPoly and setting the endpoints of the arc
		 * to be the corners, we assure that the cap will meet up with the
		 * rest of the line */
		/*p_info(PI_TRACE, "D - miFillSppPoly\n");*/
		/*	miFillSppPoly(pDraw, pGC, cpt, pArcPts, -xOrg, -yOrg, xFtrans, yFtrans);*/
		p_free((char *)pArcPts);
    }
}

#define REALLOC_STEP 10			/* how often to realloc */
#define NOARCCOMPRESSION		/* don't bother with this stuff */

/* MIGETARCPTS -- Converts an arc into a set of line segments -- a helper
 * routine for filled arc and line (round cap) code.
 * Returns the number of points in the arc.  Note that it takes a pointer
 * to a pointer to where it should put the points and an index (cpt).
 * This procedure allocates the space necessary to fit the arc points.
 * Sometimes it's convenient for those points to be at the end of an existing
 * array. (For example, if we want to leave a spare point to make sectors
 * instead of segments.)  So we pass in the Xalloc()ed chunk that contains the
 * array and an index saying where we should start stashing the points.
 * If there isn't an array already, we just pass in a null pointer and 
 * count on Xrealloc() to handle the null pointer correctly.
 */
static int miGetArcPts(SppArcPtr parc, int cpt, SppArcPtr *ppPts)
/* SppArcPtr	parc;	points to an arc */
/* int		cpt;	number of points already in arc list */
/* SppPointPtr	*ppPts;  pointer to pointer to arc-list -- modified */
{
    double 	st,					/* Start Theta, start angle */
	et,							/* End Theta, offset from start theta */
	dt,							/* Delta Theta, angle to sweep ellipse */
	cdt,						/* Cos Delta Theta, actually 2 cos(dt) */
	x0, y0,						/* recurrence formula needs 2 pts to start */
	x1, y1,
	x2, y2,						/* this will be the new point generated */
	xc, yc;						/* the center point */
    int		count, i;
    SppPointPtr	poly;
    DDXPointRec last;			/* last point on integer boundaries */
#ifndef NOARCCOMPRESSION
    double	xt, yt;
    int		axis, npts = 2;
#endif
	
    /* The spec says that positive angles indicate counterclockwise motion.
     * Given our coordinate system (with 0,0 in the upper left corner), 
     * the screen appears flipped in Y.  The easiest fix is to negate the
     * angles given */
    st = - parc->angle1;
    et = - parc->angle2;
    /* Try to get a delta theta that is within 1/2 pixel.  Then adjust it
     * so that it divides evenly into the total. 
     * I'm just using cdt 'cause I'm lazy.
     */
    cdt = fmax(parc->width, parc->height)/2.0;
    if(cdt <= 0)
		return 0;
    if (cdt < 1.0)
		cdt = 1.0;
    dt = miDasin ( 1.0 / cdt ); /* minimum step necessary */
    count = et/dt;
    count = abs(count) + 1;
    dt = et/count;	
    count++;
    cdt = 2 * miDcos(dt);
#ifdef NOARCCOMPRESSION
    if (!(poly = (SppPointPtr) p_realloc((pointer)*ppPts,
										 (cpt + count) * sizeof(SppPointRec))))
		return(0);
#else				/* ARCCOMPRESSION */
    if (!(poly = (SppPointPtr) p_realloc((pointer)*ppPts,
										 (cpt + 2) * sizeof(SppPointRec))))
		return(0);
#endif				/* ARCCOMPRESSION */
    *ppPts = poly;
    xc = parc->width/2.0;		/* store half width and half height */
    yc = parc->height/2.0;
#ifndef NOARCCOMPRESSION
    axis = (xc >= yc) ? X_AXIS : Y_AXIS;
#endif
    x0 = xc * miDcos(st);
    y0 = yc * miDsin(st);
    x1 = xc * miDcos(st + dt);
    y1 = yc * miDsin(st + dt);
    xc += parc->x;				/* by adding initial point, these become */
    yc += parc->y;				/* the center point */
    poly[cpt].x = (xc + x0);
    poly[cpt].y = (yc + y0);
    last.x = ROUNDTOINT( poly[cpt + 1].x = (xc + x1) );
    last.y = ROUNDTOINT( poly[cpt + 1].y = (yc + y1) );
    for(i = 2; i < count; i++)
    {
		x2 = cdt * x1 - x0;
		y2 = cdt * y1 - y0;
#ifdef NOARCCOMPRESSION
		poly[cpt + i].x = (xc + x2);
		poly[cpt + i].y = (yc + y2);
#else				/* ARCCOMPRESSION */
		xt = xc + x2;
		yt = yc + y2;
		if (((axis == X_AXIS) ?
			 (ROUNDTOINT(yt) != last.y) :
			 (ROUNDTOINT(xt) != last.x)) ||
			i > count - 3)		/* insure 2 at the end */
		{
			/* allocate more space if we are about to need it */
			/* include 1 extra in case minor axis swaps */
			if ((npts - 2) % REALLOC_STEP == 0)
			{
				if (!(poly = (SppPointPtr)
					  p_realloc((pointer) poly,
								((npts + REALLOC_STEP + cpt) *
								 sizeof(SppPointRec)))))
					return(0);
				*ppPts = poly;
			}
			/* check if we just switched direction in the minor axis */
			if (((poly[cpt + npts - 2].y - poly[cpt + npts - 1].y > 0.0) ?
				 (yt - poly[cpt + npts - 1].y > 0.0) :
				 (poly[cpt + npts - 1].y - yt > 0.0)) ||
				((poly[cpt + npts - 2].x - poly[cpt + npts - 1].x > 0.0) ?
				 (xt - poly[cpt + npts - 1].x > 0.0) :
				 (poly[cpt + npts - 1].x - xt > 0.0)))
			{
				/* Since the minor axis direction just switched, the final *
				 * point before the change must be included, or the        *
				 * following segment will begin before the minor swap.     */
				poly[cpt + npts].x = xc + x1;
				poly[cpt + npts].y = yc + y1;
				npts++;
				if ((npts - 2) % REALLOC_STEP == 0)
				{
					if (!(poly = (SppPointPtr)
						  p_realloc((pointer) poly,
									((npts + REALLOC_STEP + cpt) *
									 sizeof(SppPointRec)))))
						return(0);
					*ppPts = poly;
				}
			}
			last.x = ROUNDTOINT( poly[cpt + npts].x = xt );
			last.y = ROUNDTOINT( poly[cpt + npts].y = yt );
			npts++;
		}
#endif				/* ARCCOMPRESSION */
		
		x0 = x1; y0 = y1;
		x1 = x2; y1 = y2;
    }
#ifndef NOARCCOMPRESSION	/* i.e.:  ARCCOMPRESSION */
    count = i = npts;
#endif				/* ARCCOMPRESSION */
    /* adjust the last point */
    if (abs(parc->angle2) >= FULLCIRCLE)
		poly[cpt +i -1] = poly[0];
    else {
		poly[cpt +i -1].x = (miDcos(st + et) * parc->width/2.0 + xc);
		poly[cpt +i -1].y = (miDsin(st + et) * parc->height/2.0 + yc);
    }
    return(count);
}
#endif
