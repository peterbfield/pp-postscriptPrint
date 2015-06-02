#ifndef _GS_HEADER_H

#define _GS_HEADER_H

#define GS_REV_NUMBER	1	/* Current rev number. */

#define MIDDLE_PIXELS   0	/* Group of pix not touching either side. */
#define FIRST_PIXELS    1	/* First group of pix on the scan line. */
#define LAST_PIXELS     2	/* Last group of pix on the scan line. */
#define LINE_OF_PIXELS  3	/* Entire scan line is one group of pix. */
							/* If it is the first and the last group of
								pixels, FIRST_PIXELS and LAST_PIXELS
								are ORed. */

typedef struct _GS_MODS
	{
	int16 horiz_pct_scale;		/* Percent scaling, horizontal. */
	int16 vert_pct_scale;		/* Percent scaling, vertical. */
	int16 crop_left;			/* Crop point, left side, X coor. */
	int16 crop_right;			/* Crop point, right side, X coor. */
	int16 crop_top;				/* Crop point, top, Y coordinate. */
	int16 crop_bottom;			/* Crop point, bottom, Y coordinate. */
	int16 flip_flag;			/* Flip around Y axis. */
	int16 rotate_degrees;		/* Degrees of rotation. */
	int16 rotate_origin_x;		/* X coor. around which the graphic
									is rotated. */
	int16 rotate_origin_y;		/* Y coor. around which the graphic
									is rotated. */
	} GS_MODS;

typedef struct _GS_HEADER
	{							/* Graphic-shape file description */
	int16 dpl;					/* Number of dots per line. */
	int16 lpg;					/* Number of lines per graphic. */
	int16 horiz_dpi;			/* Number of dots per inch, horiz. */
	int16 vert_dpi;				/* Number of dots per inch, vertical. */
/*
 * The next 4 variables are not used at this point
 * and should be set to zero.
 */
	int16 top_lines_dropped;	/* Number of lines dropped from top. */
	int16 white_space_left;		/* White space on left of graphic. */
	int16 white_space_right;	/* White space on right of graphic. */
	int16 white_space_bottom;	/* White space on bottom of graphic. */
	int16 start_scan_lines;		/* 1 = No coordinates present in this
									file. Absolute block number where
									the scan lines blocks start. (0 is
									illegal) This number times
									512 bytes = offset */

	int16 flag_px_file;			/* 0 = no .PX, 1 = .PX exists */
	int16 flag_update;			/* 0 = up-to-date, 1 = need to be
									updated */
	char unused_2[448];
	GS_MODS current;			/* Changes to current graphic. */
	GS_MODS cumulative;			/* Cumulative changes to original
									graphic. */
	int16 gs_structure_revision;/* Revision nr. of the GS structure. */
	} GS_HEADER;

typedef struct _GROUP_PIXELS
	{
	int16 y;					/* Y coord. of that scan line. */
	int16 on;					/* X coord. of the first pixel on. */
	int16 off;					/* X coord. of the last pixel on. */
	int16 first_last;			/* Flag indicating if it is the
									first/last group of pixels on this
									scan line. */
	} GROUP_PIXELS;

typedef struct _GRAPHIC_SWATH
	{
	struct _GRAPHIC_SWATH *prev;
	struct _GRAPHIC_SWATH *next;
	int16 y;					/* Y coord. of that scan line. */
	int16 on;					/* X coord. of the first pixel on. */
	int16 off;					/* X coord. of the last pixel on. */
	int16 first_last;			/* Flag indicating if it is the
									first/last group of pixels on this
									scan line. */
	} GRAPHIC_SWATH;

typedef struct _COORDINATE
	{
	int16 x;
	int16 y;
	} COORDINATE;

#endif
