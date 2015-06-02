#ifndef _PXL_HEADER_H

#define _PXL_HEADER_H

							/* For use with line-alignment: */
#define PXL_WORD_ALIGN	0	/* line is aligned on a word boundary */
#define PXL_BYTE_ALIGN	1	/* line is aligned on a byte boundary */
#define PXL_BIT_ALIGN	2	/* line is aligned on a bit boundary */
#define MAX_SHAPE_STR 128
#define SHAPES_LINE   10
#define HEX      16

#define PXL_REV_NUMBER	3	/* Current rev number. */
							/* Rev 1 = original .px data from MV */
							/* Rev 2 = AViiON rev, with the following:
										first_shape_rec added to header
										shape data structures added */
							/* Rev 3 = AViiON rev, with bounding box,
										pan and crop data replacing
										the old imagitex source data */

typedef struct			/* Pixel file description */
	{
	int16   dpl;					/*  00. 01 # dots per line.*/
	int16   lpg;					/*  02. 03 # lines per graphic.*/
	int16   horiz_dpi;				/*  04. 05 # dots per inch, horiz*/
	int16   vert_dpi;				/*  06. 07 # dots per inch, vert*/

	int32	bbox_top;				/*  10. 13 bbox loc in 20th pnt */
	int32	bbox_bottom;			/*  14. 17 bbox loc in 20th pnt */
	int32	bbox_left;				/*  20. 23 bbox loc in 20th pnt */
	int32	bbox_right;				/*  24. 27 bbox loc in 20th pnt */
	int32	crop_top;				/*  30. 33 bbox to image in 20th */
	int32	crop_left;				/*  34. 37 bbox to image in 20th */

	int32	crop_width;				/*  40. 43 image width in 20th */
	int32	crop_depth;				/*  44. 47 image depth in 20th */

	int16	trim_top;				/*  50, 51 box to image in pixels */
	int16	trim_left;				/*  52, 53 box to image in pixels */
	int32	dummy2;					/*  54. 57 */
	int32	dummy3;					/*  60. 63 */
	int16   source_type;			/*  64. 65 1: ImagiTex, 2: Datacopy,
											   3: Mac, 4: BitPad. */
	int16   line_alignment;			/*  66. 67 0:word, 1:byte, 2:bit */
	char	color_mask;				/*         bit per color used */
	char    bits_per_pixel;			/*  70. 71 # bits per pixel:
											screened or line art is 1,
											continuous tone is 8;
											0 defaults to 1. */
	int16   positioning_parms;		/*  72. 73 Output device positioning
											   parameters:
												starting point:
												  1 = ULC, 2 = URC,
												  3 = LLC, 4 = LRC,
												ORed with stopping point
												  16 = ULC, 32 = URC,
												  48 = LLC, 64 = LRC. */
	int16   style_number;			/*  74. 75 ImagiTex style file 
												number (IMGTXnn) */
	int16   in_process_flag;		/*  76. 77 In-Process flag:
											   0 = PXL is up-to-date with
												   source version.
											   1 = PXL is modified but
												   source is not.
											   -1 = file in use to 
													modify source. */
	int16	current_mods[10];		/* 100.123 Current graphic mods */
	int16   px_structure_revision;	/* 124.125 Rev # of file structure
												1 = MV, 2 = AViiON */
	int16	cumulative_mods[10];	/* 126.151 Cumulative mods */
	int16	first_shape_rec;		/* 152,153 Rev 2 shape data */
	int16   reserved[8];			/* 154,173 */
	int16   screen_value;			/* 174,175 */
	int16   graphic_type;			/* 176.177 Graphic type: 
												0 = line art, 
												1 = contone. */
	char	dumy[128];				/* ??? */
/* Adjust the size of dummy when new fields are added */
	char dummy[250];				/* 200.377 Output styles */
	int16 scale, scaley;
	int16  file_format; /* 0 - old format, 1 - new format (versions 7.0 and up) */
} PXL_HDR; /* Total size should be 512 bytes */

/**********************************************************************
*     Shape data structures, pointed to by PXL_HDR.first_shape_rec     *
**********************************************************************/

typedef struct 
	{
	int16 x;
	int16 y;
	} SHAPE_X_Y;

typedef struct
	{
	int16 shape;					/* n'th shape */
	int16 of_sh;					/* total shapes in graphic */
	int16 num_pts;					/* number of points in this shape */
	int16 num_recs;					/* number of 512 disk records used*/
	SHAPE_X_Y pts[126];				/* points, will overflow as needed*/
	} PXL_SHAPE;

typedef struct
   {
	   int32 crop_left, crop_top;
	   int32 crop_width, crop_depth;
	   int16 scalex, scaley;
           int16 degree, lockpt;
   } GAL_GR_DATA;
#endif






