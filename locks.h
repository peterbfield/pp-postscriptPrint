#ifndef _LOCKS_H

#define _LOCKS_H
 
/* 3 define values returned for lock_a_point */
#define X_IS_LOCK		1
#define Y_IS_LOCK		2
#define X_AND_Y_IS_LOCK	3

#define LOCK_X	0x0C
#define LOCK_Y	0x03

/* 8 define values returned for lock_a_rectangle */
#define TOP_IS_LOCK				1
#define BOTTOM_IS_LOCK			2
#define LEFT_IS_LOCK			4
#define TOP_LEFT_IS_LOCK		5
#define BOTTOM_LEFT_IS_LOCK		6
#define RIGHT_IS_LOCK			8
#define TOP_RIGHT_IS_LOCK		9
#define BOTTOM_RIGHT_IS_LOCK	10

#define CG_TRIM		1
#define CG_PAGE		2
#define CG_MARGIN	3

/* The following values are either bitmap values, which may be OR'ed
   together, or just unique values, used for radio buttons */

					/* Snap from */

#define FREE_FORM		0 
#define FRAME_EDGES		0x0001

					/* Snap to */
/*#define FRAME_EDGES	0x0001		See above */
#define CROSSHAIR1_H_V	0x0002
#define CROSSHAIR2_H_V	0x0004
#define COLUMN_GUIDES	0x0008
#define TRIM            0x0010
#define PAGE            0x0020
#define MARGIN          0x0040
#define FRAMES          0x0070			/* Not a bitmap, just unique */
#define LEADING_GUIDES	0x0080
#define RULERS          LEADING_GUIDES /* Same - Never occur in same bitmap */
#define W_BASELINE		0x0100			/* Wysiwyg baseline of text */
#define W_CAP_HEIGHT	0x0200			/* Wysiwyg cap height of text */

#define BOX_FILL        0x0100
#define TRACK_MOUSE     0x0200
#define SAVE_TRACK_MOUSE 0x0400
#define AUTO_SZ_SEL     0x0800
#define LINKS           0x1000
#define TINTS           0x2000
#define FACING_PAGES    0x4000

#define CROSSHAIRS_H_V  (CROSSHAIR1_H_V | CROSSHAIR2_H_V)

#endif
