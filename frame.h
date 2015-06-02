#ifndef _FRAME_H

#define _FRAME_H

#define FRAME_SELECTED			0x01	/* Flag for f_flag in FRAME */
#define FRAME_WILL_BE_SELECTED	0x02	/* Flag for f_flag in FRAME */

#define NO_SELECTED_FRAME	0			/* Frame 0 is the spec data */
#define NO_SELECTED_POLY	-1

#define FIRST	0
#define LAST	1

#define RECTANGLE		1			/* Draw rectangle */
#define OVALE			2			/* Draw ovale */
#define CIRCLE_ONLY		4			/* Draw circle only */
#define OVALE_CENTER	8			/* Draw based on center-center */

/*
 * Many symbols in this file provide access to struct RELATIVES.
 * See window.h for definition of struct RELATIVES and for comment pointing
 *  the way to related header files.
 */
#define LAYOUT_X_OFFSET	\
			(18 * wn -> msb + lmt_off_to_abs(wn, X_REF, TRIM_LENGTH))
#define LAYOUT_Y_OFFSET	\
			(18 * wn -> ldb + lmt_off_to_abs(wn, Y_REF, TRIM_LENGTH))

#define OVERFLOW_TEXT	1
#define UNDERFLOW_TEXT	2
#define PREV_LAYOUT		3
#define NEXT_LAYOUT		4
#define VERT_JUST_TEXT	5

#define NO	0
#define YES	1

#define IGNORE_MOUSE	0
#define PROCESS_MOUSE	1

#define LOCK_LEFT		0x01
#define LOCK_RIGHT		0x02
#define LOCK_X_CENTER	0x03
#define LOCK_TOP		0x10
#define LOCK_BOTTOM		0x20
#define LOCK_Y_CENTER	0x30
#define LOCK_HEIGHT		0x40

#define LOCK_MASK_X		0x0F
#define LOCK_MASK_Y		0xF0

#define TL  0x11	/* 021 */
#define TC  0x13	/* 023 */
#define TR  0x12	/* 022 */
#define BL  0x21	/* 041 */
#define BC  0x23	/* 043 */
#define BR  0x22	/* 042 */
#define CL  0x31	/* 061 */
#define CC  0x33	/* 063 */
#define CR  0x32	/* 062 */
#define HL  0x41	/* 0101 */
#define HC  0x43	/* 0102 */
#define HR  0x42	/* 0103 */

#define TL_SIDE     0
#define TC_SIDE     6
#define TR_SIDE     3
#define BL_SIDE     2
#define BC_SIDE     7
#define BR_SIDE     5
#define CL_SIDE     1
#define CR_SIDE     4

/* The define statements for the toolbox MUST match
	the ones in layout.uil */

#define TOOLBOX_DRAW_TEXT		1
#define TOOLBOX_DRAW_GRAPHIC	2
#define TOOLBOX_DRAW_BOX		6
#define TOOLBOX_DRAW_EMPTY		12
#define TOOLBOX_DRAW_FLOW		13
#define TOOLBOX_DRAW_RULE		93
#define TOOLBOX_SELECT_FRAME	99
#define TOOLBOX_GRAB			98
#define TOOLBOX_DELETE_FRAME	97
#define TOOLBOX_CROSSHAIR		96
#define TOOLBOX_LINK			95
#define TOOLBOX_UNLINK			94
#define TOOLBOX_TEXT_EDITOR		92
#define TOOLBOX_FILL			91
#define TOOLBOX_VIEW_TEXT		90
#define TOOLBOX_JOIN            89
#define TOOLBOX_DISJOIN         88
#define TOOLBOX_LINE_FORWARD    87
#define TOOLBOX_LINE_BACKWARD   86
#define TOOLBOX_CLEAR           85
#define TOOLBOX_POLYGON         84
#define TOOLBOX_OVALE			83
#define TOOLBOX_ZOOM			82
#define TOOLBOX_ROTATE			81
#define TOOLBOX_ROUNDED			80
#define TOOLBOX_CROP            79
#define TOOLBOX_SCALE           78
#define TOOLBOX_GRAPHIC         77

#define PL_SPECS	0
#define PL_TEXT		1
#define PL_GRAPHIC	2
#define PL_FN		3
#define PL_REF		4
#define PL_MISC		5
#define PL_RBX		6
#define PL_DUMMY	7
#define PL_ALSET	8
#define PL_INCL		9
#define PL_TRS		10
#define PL_SNARN	11
#define PL_EMPTY	12
#define PL_FLOW		13

#define PL_LETTER   14
#define PL_LEGAL    15
#define PL_BSIZE    16
#define PL_CUSTOM   17

#define PORT	1
#define LAND	2

#define ABS		1
#define REL		2

#define PL_REL_SHIFT    27			/* Bit-shift for PL_REL_BITS */
#define PL_SIGNBIT	0x80000000		/* Sign bit */
#define PL_REL_BITS	0x78000000		/* Type of value */
#define PL_SYNBITS	0x07ffffff		/* last 27 bits */

#define NOT_SET -1				/* Initial value for specs text */

#define PAGE_REF		0			/* Coordinate based on side of
										the page */
#define TRIM_REF		1			/* Coordinate based on side of
										the trim */

#define POINT			-1			/* points */
#define PICA			1			/* picas */
#define INCH_32			2			/* inches and 32ths */
#define CENTIMETER		3			/* centimeters and millimeters */
#define LINE_LEAD		4			/* multiple of line leading */
#define POINT_HALF		5			/* points and halves */
#define POINT_QUARTER	6			/* points and quarters*/
#define POINT_TENTH		7			/* points and tenths */
#define ORG_LINE_LEAD	8			/* multiple of original spec line
										leading */
#define INCH_10         9			/* Decimal inches */

#define DESIGN_TEXT		0x00
#define CONT_LINE		0x01
#define SIDE_NOTE		0x02
#define AREA_REF		0x03

#define NO_QUAD			0x00
#define FQUAD_LEFT		0x01
#define FQUAD_RIGHT		0x02
#define FQUAD_CENTER	0x03
#define TYPESET_FLAG	0x04

#define CROP_BIT_TOP    0x0001
#define CROP_BIT_LEFT   0x0002
#define CROP_BIT_BOTTOM 0x0004
#define CROP_BIT_RIGHT  0x0008

#endif
