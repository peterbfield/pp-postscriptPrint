#include <stdio.h>
#include "p_lib.h"
#include "psjob.h"

extern int err;					/* Holds er-ror code for stop() */
extern char UserdataPath[];

Pfd wdfd;						/* File descriptor for WIDTH.xx */
int16 FontBase;					/* Font-base from WIDTH.xx (1000) */
struct wid_val Widval[10];		/* Hold most-recently-used 10 fonts. */
int Widindex;					/* Index of current width set among 10.*/
int16 current_font;				/* FONT num currently in widths_data  */
struct widths					/* Structure of rec in WIDTH.xx */
{
    int16 char_width[255];		/* 1-255 */
    int16 video;				/* 256 */
    char  font_name[18];		/* 257-265 */
    int16 font_base;			/* 266 */
    int16 rounding_factor;		/* 267 */
    int16 junk1;				/* 268 */
    int16 ligature_sw;			/* 269 */
    int16 last_printable_char;	/* 270 */
    int16 first_fixed_space;	/* 271 */
    int16 last_fixed_space;		/* 272 */
    int16 font_master_size;		/* sometimes machine font num */
    int16 kern_vals_rec_no;		/* Used in old kerning */
    int16 kern_status;			/* 275 */
    int16 loc_of_fi;			/* 276 */
    int16 loc_of_fl;			/* 277 */
    int16 loc_of_ff;			/* 278 */
    int16 loc_of_ffi;			/* 279 */
    int16 loc_of_ffl;			/* 280 */
    int16 trk123_pnt_sizes[20];	/* 281-300 pnt size 4 width adj */
    int16 wdth_adj_trk1[20];	/* 301-320 Width adj vals trk 1 */
    int16 wdth_adj_trk2[20];	/* 321-340 Width adj vals trk 2 */
    int16 wdth_adj_trk3[20];	/* 341-360 Width adj vals trk 3 */
    int16 space_stuff[100];		/* 361-460 */
    int16 unused[22];			/* 461-482 */
    int16 min_max[30];			/* 483-512  10 font-ranges */
};
struct widths widths_data;

/**********************************************************************/
void width_open(void)			/* Open the WIDTH file. */
{
	int i;
	
    wdfd = p_open(TreeName, USERDATA, UserdataPath, "width", "r");
    if(wdfd <= 0)
		stop("Unable to open file width ", UserdataPath,0);
    current_font = 0;			/* Init: No rec in mem from WIDTH. */
    Widindex = -1;				/* No widths converted yet. */
    for (i=0; i<10; i++)
		Widval[i].fontnum = 0;	/* All 10 width-sets empty.*/
}

/**********************************************************************/
void width_read(void)			/* Read in the PostScript font# record
								   fo_line_def.SolFont from
								   WIDTHS.xx, convert wids to points or remain-
								   ders.  Maintain the 10 most-recently-used
								   converted fonts in a 10-entry structure. */
{
	int i;
    
    for (Widindex=0;Widindex<10;Widindex++) /* Search for needed font/size
											  among 10 in mem: Must match*/
	{
		if (Widval[Widindex].fontnum != fo_line_def.SolFont) /* font number */
			continue;
		if (Widval[Widindex].setsize == Holdss)	/* and set-size. */
			return;
    }
/*
  The converted font we need is not in memory.  Build it in entry zero
  of the 10-entry widths structure, sliding entries 1,2... into entries 2,3....
*/
    for (Widindex=8; Widindex>=0; Widindex--) /* Loop to open entry 0. */
    {
		Widval[Widindex+1].fontnum = Widval[Widindex].fontnum;
		Widval[Widindex+1].setsize = Widval[Widindex].setsize;
		for (i=0; i<255; i++)
			Widval[Widindex+1].roundoffs[i]=Widval[Widindex].roundoffs[i];
    }
    if (fo_line_def.SolFont != current_font) /* If only setsize changed, */
								/*  font may be in mem now. */
    {							/* No: */
		err = p_read((char *)&widths_data, 1024,1,wdfd,
					 fo_line_def.SolFont, SW_WIDTH);
		if( !err)
			stop("Read of width failed, can't find that font, err"," ",0);
		FontBase = widths_data.font_base; /* Save for universal use. */
		if (FontBase < 10)
			stop("Font is empty in width file"," ",0);
		current_font = fo_line_def.SolFont;	/* Remember which font in mem */
    }

/* CONVERT & STORE THE FONT IN INDEX 0: */

    Widindex = 0;
    Widval[0].fontnum = fo_line_def.SolFont; /* Font number */
    Widval[0].setsize = Holdss;	/* Set size */
	for(i=0;i<243;i++)			/* Convert every char wid to its
								   remainder lost by H&J. */
		Widval[0].roundoffs[i] =
			(widths_data.char_width[i] * Holdss) % FontBase;
	for(i=243;i<256;i++)		/* Fixed spaces have no error. */
		Widval[0].roundoffs[i] = 0;
}
/**********************************************************************/





