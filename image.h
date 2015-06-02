#ifndef IMAGE_H
#define IMAGE_H

#include "penta.h"
#include "que.h"

#define LINE_IS_ASSIGNED	0x80000000
#define ImgnameSize		64

typedef struct {
	QE que;         /* pointer to next & previous items */
	short type;     /* type of display item */
	QUEUE color_image;	/* Needed for compatibility with LINE_IMAGE */
	QUEUE misc_images;	/* Needed for compatibility with LINE_IMAGE */
	}DISP_ITEM;

typedef struct {
	QE que;			/* pointer to next & previous items */
	short type;
	QUEUE color_image;	/* Needed for compatibility with LINE_IMAGE */
	QUEUE misc_images;	/* Needed for compatibility with LINE_IMAGE */
	vmu ldtot;
	vmu flexible_extra_lead;		/* extra lead in line */
	vmu expandable_extra_lead;		/* extra lead in line */
	vmu misc_extra_lead;
	vmu droppable_extra_lead;		/* non-expandable extra lead */
	vmu rigid_extra_lead;			/* non-expandable extra lead */
	int nvbands;
	int ntabs;
	int itabs[110];
	int itwids[110];
	} BEGIN_TAB_MARK;

typedef struct { 
	QE que;			/* pointer to next & previous items */
	short type;				/* type of tab mark */
	QUEUE color_image;	/* Needed for compatibility with LINE_IMAGE */
	QUEUE misc_images;	/* Needed for compatibility with LINE_IMAGE */
	vmu ldtot;					/* tot lead at this point */
	int flasher_status;			/* non-zero if output flasher if off */
	} TAB_MARK;

typedef struct {
	QE que;						/* pointer to next & previous items */
	int image_type;				/* 1 = rectangle  2 = rule segment 
								   3 = comb rule segment in tab line
								   4 = strike-thru rule  */
	int color_num;
	int x_offset;				/* horizontal offset of ulc */
	int y_offset;				/* vertical offset of ulc */
	int width;					/* width of rectangle */
	int depth;					/* depth of rectangle */
	int shade;					/* Background shade   */
	int blend_set;				/* Zero if the rectangle blend was not set, 
									notzero otherwise */
	int blend_end_color_num;	/* Will be >= 0 if frame has b.g. blend */
	int blend_end_shade;		/* End color shade if frame has b.g. blend */
	int blend_angle;			/* If >= 0, blend angle in 10ths of degrees. 
									If < 0, program must compute--if -1, blend 
									runs from top left to bottom right, if -2, 
									blend runs from top right to bottom left. */
	} COLOR_RECTANGLE;

typedef struct {
	QE que;			/* pointer to next & previous items */
	int color_num;
	int color_graphic;                  /* If it's color graphic */
	unsigned char *dots;				/* pointer to pixel data */
	unsigned char *last_dot;			/* address of end of buffer */
	int shade;                          /* shade, used for underscores */
        int16 degree, lockpt;
        y_coord voff;           /* vert offset of baseline in line image */
        x_coord hoff;           /* horiz offset of left edge in line image*/
        x_coord width;          /* width of line image */
        y_coord depth;          /* depth of line image */
	} COLOR_IMAGE;

#define VjLinePoint		1		/* do line adjust before this line */
#define VjParaPoint		2		/* do para adjust before this line */
#define VjParaLine		4		/* this line ends with [ep */

typedef struct {
	QE que;			/* pointer to next & previous items */
	short type;				/* type of image */
	QUEUE color_image;
	QUEUE misc_images;
	uint16 line_no;			/* line number */
	int line_flags;			/* tabular stuff: 
								0x00003: Alignment from [ta (1=top 2=bot 3=cen)
								0x001fc: Straddle head: # cols straddled
								0x01e00: Straddle head: tab levels
								0x02000: This is a tab line
								0x80000000: LINE_IS_ASSIGNED This line of type is 
								  assigned to a layout. */
    int16 start_forec;
    int16 start_fowrd;
    int16 end_forec;
    int16 end_fowrd;
	y_coord top;			/* top ulc of line image */
	x_coord left;			/* left ulc of line image */
	x_coord width;			/* width of line image */
	y_coord depth;			/* depth of line image */

	y_coord voff;			/* vert offset of baseline within line image */
	x_coord hoff;			/* horiz offset of left edge within line image */
	int vj_flags;			/* VjLinePoint, VjParaPoint, VjParaLine */
	vmu vj_amount;			/* total vertical adjustment on this line */
	vmu baseline;			/* mru position of baseline from top */
	vmu leading;			/* leading of line */
	vmu flexible_extra_lead;	/* extra lead in line */
	vmu expandable_extra_lead;	/* extra lead in line */
	vmu misc_extra_lead;		/* extra lead within the line */
	vmu droppable_extra_lead;	/* non-expandable extra lead */
	vmu rigid_extra_lead;	/* non-expandable extra lead */
	int nvbands;			/* number of vb's in line */

	hmu line_start;			/* mru position of left edge of line ??? used ??? */
	hmu line_measure;		/* measure of line */

	int point_size;			/* point size at start of line */
	int set_size;			/* set_size at start of line */

	short cpl;				/* characters per image scan line */
	int value;				/* Size of bitmap (in bytes) of dots */
	} LINE_IMAGE;

	typedef struct _dummyGC {
		int dummy;
		} *dummyGC;

typedef struct {
    QE que;          /* pointer to next and previous items */
    short type;             /* type of image */
    uint16 line_no;          /* line number */
    int line_flags;         /* tabular stuff etc. */

    y_coord top;            /* top ulc of line image */
    x_coord left;           /* left ulc of line image */
    x_coord width;          /* width of line image */
    y_coord depth;          /* depth of line image */

    y_coord voff;           /* vert offset of baseline in line image */
    x_coord hoff;           /* horiz offset of left edge in line image*/

                    /* from graphics.h typdef MUM_GQ_ENTRY */
    int16 display_zoom_pct; /* Percent zoom for displayed image.*/
    int16 display_dpl;      /* Displayed dots per line.*/
    int16 display_lpg;      /* Displayed lines per graphic.*/
    int16 full_cropped;     /* Graphic is full (0) or cropped (1).*/
    int32 width_20;         /* Graphic width and depth, (20ths)*/
    int32 depth_20;
    int16 scale_percent;    /* Scale percent (excludes screen zoom).*/
    int16 scale_percenty;    /* Scale percenty (excludes screen zoom).*/
    int16 scaled_width;     /* Scaled width, (20ths)*/
    int16 scaled_depth;     /* Scaled depth, (20ths)*/
    int16 flip_flag;        /* Image flipped (mirrored) around Y axis.*/
    int16 rotation;         /* Degrees rotated.*/
    int16 crop_left;        /* ULC of crop point, (20ths)*/
    int16 crop_top;
    int16 cropped_width;    /* Cropped width and depth, (20ths)*/
    int16 cropped_depth;

    short cpl;              /* characters per image scan line */
	short colors;			/* 1 = monocrome, 2 -> 8 = color */
	dummyGC gcs[8];			/* display GC's for color */
	int PtrOffset;			/* size of each pixel map */
	char old_name[ImgnameSize]; /* Old graphics name */
	int16  file_format; /* 0 - old format, 1 - new format (versions 7.0 and up */
	char	color_mask;			/* 64 when image is black_n_white*/
	int16 degree, lockpt;
	unsigned char *AllDots;	/* mask for all non-white pixel data */
	char dummy[20];			/* just make sure it fits*/
    } GRAPHIC_IMAGE;

#endif 

