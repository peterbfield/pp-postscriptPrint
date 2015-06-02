#ifndef _X11_ROUNDED_H

#define _X11_ROUNDED_H


typedef struct {					/* This come from X11/Xlib.h */
	long x1, y1, x2, y2;
	} XSEGMENT;

typedef struct {					/* This come from X11/Xlib.h */
	long x, y;
	unsigned long width, height;
	short angle1, angle2;
	} XARC;

#endif
