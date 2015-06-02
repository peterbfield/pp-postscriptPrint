#ifndef _X11_TYPEDEFS_H

#define _X11_TYPEDEFS_H

#include "X11_rounded.h"

/* fillRule */

#define EvenOddRule     0			/* This come from X11/X.h */
#define WindingRule     1			/* This come from X11/X.h */

typedef struct {					/* This come from X11/Xlib.h */
	int x, y;
	} lmt_XPoint;

typedef struct {					/* This come from X11/region.h */
	int x1, x2, y1, y2;
	} BOX;

typedef struct _XRegion {			/* This come from X11/region.h */
	long size;
	long numRects;
	BOX *rects;
	BOX extents;
	} REGION;

typedef struct _XRegion *Region;	/* This come from X11/Xutil.h */

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200			/* This come from X11/region.h */

/*
 * used to allocate buffers for points and link
 * the buffers together
 */
typedef struct _POINTBLOCK {		/* This come from X11/region.h */
	lmt_XPoint pts[NUMPTSTOBUFFER];
	struct _POINTBLOCK *next;
	} POINTBLOCK;

#define TRUE 1						/* This come from X11/region.h */
#define FALSE 0						/* This come from X11/region.h */
#define MAXLONG  0xFFFFFFFF
#define MINLONG  -MAXLONG
#define MAXSHORT 32767				/* This come from X11/region.h */
#define MINSHORT -MAXSHORT			/* This come from X11/region.h */

#ifndef max							/* This come from X11/region.h */
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min							/* This come from X11/region.h */
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX							/* This come from X11/region.h */
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN							/* This come from X11/region.h */
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

									/* This come from X11/region.h */
#define INBOX(r, x, y) \
		( ( ((r).x2 >= x)) && \
		( ((r).x1 <= x)) && \
		( ((r).y2 >= y)) && \
		( ((r).y1 <= y)) )

#endif
