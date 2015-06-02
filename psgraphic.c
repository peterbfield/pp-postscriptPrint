#include <stdio.h>
#include <math.h>
#include "p_lib.h"
#include "psjob.h"
#include "rel_data.h"
#include "lpm.h"

static void clip_graphic(int mode, float rotation);

static int gm_rot_x;
static int gm_rot_y;

#if LPMfloat
extern char LPMK[];
#else
extern unsigned int LPMK;
#endif
extern int GraphicsPresentFlag;
extern int ResolveOPI;

static int gsave_flag;
extern int ClipPathFlag;

void graphic(int16 fo_cmd)		/* 0 = graphic frame, 1 = [mg_] or [gm_] */
{
	char nameg[132];
	char pathg[256];
	char line[132];
    int16 fch;
	int ii;
    double tan();
	Pfd pxfd;
	float trim_x, trim_y;
	int fo_cmd_type = 0;
	int px_crop_flg = 0;
	float top, left;
	float scale = 1., scaley = 1.;
	float px_scale = 0., py_scale = 0.;
	float px_crop_left = 0.;
	float px_crop_top = 0.;
	float px_crop_width = 0.;
	float px_crop_depth = 0.;
	int gm_crop_left = 0;
	int gm_crop_top = 0;
	int gm_crop_width = 0;
	int gm_crop_depth = 0;
	float gm_scale_x = 1.;
	float gm_scale_y = 1.;
	float gm_rotation = 0;
	int gm_flip = 0;
	int gm_reverse = 0;
	double gm_skew_x = 0;
	double gm_skew_y = 0;
    
/*------------------------------------------------------------------------
  --  Start out by making sure you are at current position.		--
 ------------------------------------------------------------------------*/
	gsave_flag = 0;
	gm_rot_x = 0;
	gm_rot_y = 0;
	if ( FlashOn)
	{
		Ofx = -4000;			/* force Move command out */
		do_pskmd('m',"Graphic");
	}
/*------------------------------------------------------------------------
  --  Get pic name, position, etc.					--
 ------------------------------------------------------------------------*/
    if (fo_cmd)				
    {							/* For [MG--] command: */
		fo_cmd_type = foget();
		if ( !fo_cmd_type || (fo_cmd_type == 1) )
		{
			if ( !fo_cmd_type)	/* if == 1, only graphic name in .fo file */
			{					/* [mg before 6/64 */
				foget();		/* Y-offset, not used */
				foget();		/* % scale in X-dimension, not used */
				foget();		/* % scale in Y-dimension, not used */
				foget();		/* Degrees to rotate, not used */
				foget();		/* Skew in X-direction, not used */
				foget();		/* Skew in Y-direction, not used */
			}
			fch = foget();		/* # characters in picture name */
			for(ii=0; ii<fch; ii++) 
				nameg[ii] = foget(); /* Get picture name, */
			nameg[fch] = 0;			/*   null-terminated. */
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "[mg ");	/* Start trace */
#endif
		}
		else
		{						/* from [gm, fo_cmd_type == 2 */
#if LPMfloat
			if ( !LPMK[LPM_CropAndScaleSite])
#else
			if ( !(LPMK & LPM_CropAndScale))
#endif
			{					/* no key for [gm, treat as [mg */
				for (ii=0; ii<16; ii++)
					foget();
				fo_cmd_type = 1; /* treat as [mg */
			}
			else
			{					/* key is present */
				foget();		/* escape flag, ignored */
				foget();		/* escape_width, ignored */
				foget();		/* escape_depth, ignored */
				gm_crop_left = foget(); /* crop_left */
				gm_crop_top = foget(); /* crop_top */
				gm_crop_width = foget(); /* crop_width */
				gm_crop_depth = foget(); /* crop_depth */
				gm_scale_x = (float )foget() / ZOOM_UNIT; /* scale_x in 1/10%*/
				gm_scale_y = (float )foget() / ZOOM_UNIT; /* scale_y in 1/10%*/
				gm_rotation = foget(); /* rotate  in 1/10 degrees */
				gm_rot_x = foget() + 1; /* rotation pin horiz: l=1, r=2, c=3 */
				gm_rot_y = foget() + 1; /*rotation pin vert: t=1, b=2, c=3 */
				gm_flip = foget();
				gm_reverse = foget();
				gm_skew_x = foget(); /* Skew in X-direction */
				gm_skew_y = foget(); /* Skew in Y-direction */
				if( !gm_scale_x)
					gm_scale_x = 1.;
				if( !gm_scale_y)
					gm_scale_y = gm_scale_x;
			}
			fch = foget();		/* # characters in picture name */
			for(ii=0; ii<fch; ii++) 
				nameg[ii] = foget(); /* Get picture name, */
			nameg[fch] = 0;			/*   null-terminated. */
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "[gm ");	/* Start trace */
#endif
		}
    }
    else
    {							/* For picture from .PAGE file */
		if ( GRAPHIC_NAME(CurrentFrame) )
			sprintf (nameg, "%-s", GRAPHIC_NAME(CurrentFrame)); /* Get name */
		else
			nameg[0] = 0;
		ii = 0;
		while ( nameg[ii] )
		{						/* blanks are end of name, change to nulls */
			if (nameg[ii] == ' ')
				nameg[ii] = 0;
			ii++;
			if (ii > (sizeof(nameg) - 1))
				break;
		}
		Xmark = Xpos = SetX;	/* X-position of pic given */
		Ypos  = SetY;			/* Y-pos of pic absolute, or */
#ifdef TRACE
		if (debugger_trace)
			p_info(PI_TRACE, "LAY  ");	/* Start trace. */
#endif
    }
#ifdef TRACE
    if (debugger_trace)
		p_info(PI_TRACE, "GRAPHIC = %s\n",nameg);
#endif
/*------------------------------------------------------------------------
  --  Save graphic state, do translate to LL corner of picture,		--
  --    perform any rotate, scale & skew asked for.			--
  ------------------------------------------------------------------------*/
	if ( !nameg[0] )
		strcpy (nameg, " ");
	if (FileType && !fo_cmd)
		color_func (FgColor, FgShade); /*only if layout. gal clr already set */
    Fx = (float)Xpos / HorizontalBase;
    Fy = (float)(Imageheight - (Ypos + jbl)) / VerticalBase;
#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "Fy= %f, imageht= %d, Ypos= %d fo_cmd= %d\n",
			   Fy, Imageheight, Ypos, fo_cmd);
#endif
	if ( !cc_mask || !FlashOn)
	{
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "cc_mask= %lo, FlashOn= %d, no graphic output\n",
			   cc_mask, FlashOn);
#endif
		return;					/* omit merge graphic if not this color */
	}
	trim_x = trim_mark_width / HorizontalBase;
	trim_y = trim_mark_depth / VerticalBase;
	*pathg = 0;					/* Init graphic-path to null string.  */
	if ((pxfd = p_open(TreeName, GRAPHICS, SubDirName, nameg, "r")))
	{							/* get data from the .pixel file */
		for(;;)
		{
			line[0] = 0;
			if ( p_fgets(line, 128,pxfd) != line)
				break;		/* end of file */
			if(strncmp(line,"crop_lt",7) == 0)
			{
				sscanf(line+7,"%f %f", &px_crop_left, &px_crop_top);
				continue;
			}
			else if(strncmp(line,"crop_wd",7) == 0)
			{
				sscanf(line+7,"%f %f", &px_crop_width, &px_crop_depth);
				continue;
			}
			else if(strncmp(line,"crop_flg", 8) == 0)
			{
				sscanf(line+8,"%d", &px_crop_flg);
				continue;
			}
			else if(strncmp(line,"scale", 5) == 0)
			{
				sscanf(line+5,"%f %f", &px_scale, &py_scale);
				continue;
			}
			else if(strncmp(line,"ps_path", 7) == 0)
			{
				sscanf(line+7,"%s", pathg);
				continue;
			}
			else if(strncmp(line,"colors",6) == 0)
				break;
		}						/* end for(;;) */
	}							/* end looking at .pixel file */
	if (pxfd)
		p_close (pxfd);
	if ( !fo_cmd)
	{							/* from a page */
#if LPMfloat
		if (LPMK[LPM_CropAndScaleSite])
#else
		if ((LPMK & LPM_CropAndScale))
#endif
		{
			Fx = (float )FRAME_DATA(CurrentFrame) left / HorizontalBase;
			Fy = (float )(Imageheight - FRAME_DATA(CurrentFrame) top) /
				VerticalBase;
			Fx2 = (float )FRAME_DATA(CurrentFrame) right / HorizontalBase;
			Fy3 = (float )(Imageheight - FRAME_DATA(CurrentFrame) bottom) /
				VerticalBase;
			if((FRAME_FLAGS(CurrentFrame)& OVALE_SHAPE) || RoundCornerRadiusTL)
			{					/* circle or round corner */
				if ( FRAME_FLAGS(CurrentFrame) & OVALE_SHAPE)
					CircleFlag = 1;
				clip_graphic(1, 0);
					CircleFlag = 0;
			}
			else		
            {
                if (ClipPathFlag || px_crop_flg)
                    clip_graphic(0, 0); /* clip frame if ClipPath is set */
                else if ( CROP_TOP(CurrentFrame) || CROP_LEFT(CurrentFrame) 
                    || CROP_BOTTOM(CurrentFrame) || CROP_RIGHT(CurrentFrame))
                    clip_graphic(0, 0); /* clip frame only if cropped clip */
            }

			Fx += trim_x;
			Fx2 += trim_x;
			Fy -= trim_y;
			Fy3 -= trim_y;
			left = (float)((lmt_off_to_abs(wn, X_REF,CROP_LEFT(CurrentFrame)) /
							HorizontalBase) * ZOOM_GR(CurrentFrame)) / 
								ZOOM_UNIT;
			top = (float)((lmt_off_to_abs(wn, Y_REF, CROP_TOP(CurrentFrame)) /
						   VerticalBase) * ZOOM_GRY(CurrentFrame)) / ZOOM_UNIT;
			Fx -= left;			/* move graphic */
			Fy += top;
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "top= %f, left= %f, flag= %d\n",
					   top, left, CROP_FLAG(CurrentFrame));
#endif
			if ( ZOOM_GR(CurrentFrame) )
				scale = scaley = (float) ZOOM_GR(CurrentFrame) / ZOOM_UNIT ;
 			if ( ZOOM_GRY(CurrentFrame) )
 				scaley = (float) ZOOM_GRY(CurrentFrame) / ZOOM_UNIT;
			if ( px_scale)
				scale *= (px_scale / ZOOM_UNIT);
			if ( py_scale)
				scaley *= (py_scale / ZOOM_UNIT);
		}
#if LPMfloat
		else if ( CROP_FLAG(CurrentFrame) && !LPMK[LPM_CropAndScaleSite] )
#else
		else if ( CROP_FLAG(CurrentFrame) && !(LPMK & LPM_CropAndScale) )
#endif
			p_info(PI_WLOG, "WARNING - You do not have a license for Crop and Scale.\nImage will not be cropped or scaled.\n");
	}
	else if ((fo_cmd_type <= 1) && ((px_crop_flg || px_scale || py_scale) &&
#if LPMfloat
									LPMK[LPM_CropAndScaleSite]) )
#else
									(LPMK & LPM_CropAndScale)) )
#endif
	{							/* clip for [mg using .pixel file data */
		if  ( px_scale)
			scale = (px_scale / ZOOM_UNIT);
		if  ( py_scale)
			scaley = (py_scale / ZOOM_UNIT);
		Fx2 = Fx + (px_crop_width * scale); /* get the clip path */
		Fy3 = Fy - (px_crop_depth * scaley);
		if (FileType)
		{
			Fx -= trim_x;
			Fx2 -= trim_x;
			Fy += trim_y;
			Fy3 += trim_y;
		}
		if (ClipPathFlag || px_crop_flg)
			clip_graphic(0, 0);		/* output the clip path */
		if (FileType)
		{
			Fx += trim_x;
			Fx2 += trim_x;
			Fy -= trim_y;
			Fy3 -= trim_y;
		}
	}
	else if (fo_cmd_type == 2)
	{							/* from [gm */
		if (px_scale)
			scale = (px_scale / ZOOM_UNIT);
		if (py_scale)
			scaley = (py_scale / ZOOM_UNIT);
		Fx2 = Fx + (gm_crop_width / HorizontalBase); /* get the clip path */
		Fy3 = Fy - (gm_crop_depth / VerticalBase);
		if (FileType)
		{
			Fx -= trim_x;
			Fx2 -= trim_x;
			Fy += trim_y;
			Fy3 += trim_y;
		}
        if (ClipPathFlag || px_crop_flg)
            clip_graphic(0, gm_rotation); /* output the clip path */

		/**************************************************************
		***F I X   F O R   B U G   351p only output clippath if 
		***graphic clipped from design master or [GM command
		***
		***	comment out existing code
		***
		***else if (px_crop_width * 10 != gm_crop_width
		***		|| px_crop_depth * 10 != gm_crop_depth * 2)
		***		clip_graphic(0, gm_rotation); * only clip [gm if cropped *
		***************************************************************/

        else if ( ((px_crop_width * 20 != gm_crop_width)
            || (px_crop_depth * 10 != gm_crop_depth))
			&& (gm_scale_x == 1000 && gm_scale_y == 1000) )
            clip_graphic(0, gm_rotation); /* only clip if cropped in [gm*/
		else if ( ((int32)(px_crop_width * 20 * gm_scale_x) != gm_crop_width)
			|| ((int32)(px_crop_depth * 10 * gm_scale_y) != gm_crop_depth) )
			clip_graphic(0, gm_rotation); /* only clip scaled if cropped in [gm */
        else if (gm_rotation)
            clip_graphic(2, gm_rotation); /* no clip [gm if only rotated */
		if (FileType)
		{
			Fx += trim_x;
			Fx2 += trim_x;
			Fy -= trim_y;
			Fy3 -= trim_y;
		}
/* adjust position by gm_crop_left and top since ts does not know about them */
		Fx -= ((gm_crop_left * gm_scale_x) / HorizontalBase); /*move graphic */
		Fy += ((gm_crop_top * gm_scale_y)) / VerticalBase;
	}
	m_fprintf("\nBEGINEPSFILE\n%%%%BeginDocument: %s\n",nameg); /* encap */
	if (ResolveOPI)
	   m_fprintf("%%PSI_Resolve:\n");
    if ( !fo_cmd)					/* For [MG_] and [GM_] only: */
	{							/* graphic frame */
		if ( scale != 1. || scaley != 1.)
			m_fprintf("%%PSI_Scale  %5.3f %5.3f  scale \n", scale, scaley);
	}
	else
    {
		if( gm_skew_x)
			gm_skew_x = tan((gm_skew_x * 6.28318) / 360);
		if(gm_skew_y)
			gm_skew_y = tan((gm_skew_y * 6.28318) / 360);
		if(gm_skew_x || gm_skew_y)
			m_fprintf("[1  %7.4f  %7.4f 1 0 0] concat \n",
					  gm_skew_x, gm_skew_y);
		if( (gm_scale_x != 1.) || (gm_scale_y != 1.) )
			m_fprintf("%%PSI_Scale  %5.3f %5.3f  scale \n", gm_scale_x * scale,
					  gm_scale_y * scaley);
		else
			m_fprintf("%%PSI_Scale  %5.3f %5.3f  scale \n", scale, scaley);
    }
	do_pskmd ('T',"graphic");
	if (*pathg)
    	m_fprintf("%%PSI_IncludePath: %s\n",pathg);
    m_fprintf("%%PSI_IncludeFile: %s\n",nameg);
/*------------------------------------------------------------------------
  --  Restore the pre-picture graphic state, return.	                --
  ------------------------------------------------------------------------*/
    m_fprintf("%%PSI_EndFile\n%%%%EndDocument\nENDEPSFILE\n");
	if ( gsave_flag)
		m_fprintf("grestore\n");
    GraphicsPresentFlag++;		/* mark special graphics merge shell script */
}

static void clip_graphic(int mode, float rotation)
{								/* mode: 0=rectangle, 1=oval, 2=round corner */
	float fxc, fx2c, fyc, fy3c;
	float x_pin = 0;
	float y_pin = 0;
 

	fxc = Fx + (trim_mark_width / HorizontalBase);
	fx2c = Fx2 + (trim_mark_width / HorizontalBase);
	fyc = Fy - (trim_mark_depth / VerticalBase);
	fy3c = Fy3 - (trim_mark_depth / VerticalBase);
	gsave_flag++;
	m_fprintf("gsave\n");
	if ( rotation)
	{
		switch (gm_rot_x)
		{
		  case 1:				/* left */
			x_pin = fxc;
			break;
		  case 2:				/* right */
			x_pin = fx2c;
			break;
		  case 3:				/* center */
			x_pin = (fxc + fx2c) / 2.;
			break;
		}						/* end switch(gm_rot_x) */
		switch (gm_rot_y)
		{
		  case 1:				/* top */
			y_pin = fyc;
			break;
		  case 2:				/* bottom */
			y_pin = fy3c;
			break;
		  case 3:				/* center */
			y_pin = (fyc + fy3c) / 2;
			break;
		}						/* end switch(gm_rot_y) */
		digi_print(-rotation/10.);
		digi_print(x_pin);
		digi_print(y_pin);
		m_fprintf("RXY\n");
	}
	if (mode == 2)
		return;
	m_fprintf("newpath ");
	if ( !mode)
	{
		digi_print(fxc);
		digi_print(fyc);
		m_fprintf("M ");
		digi_print(fx2c);
		digi_print(fyc);
		m_fprintf("L ");
		digi_print(fx2c);
		digi_print(fy3c);
		m_fprintf("L ");
		digi_print(fxc);
		digi_print(fy3c);
		m_fprintf("L ");
		m_fprintf("closepath clip\n");
	}							/* end if(!mode) */
	else
	{
		if ( CircleFlag)
		{
			float x_radius, y_radius;

			digi_print(x_radius = ((fx2c - fxc)/2)); /* x radius */
			digi_print(y_radius = ((fyc - fy3c)/2)); /* y radius */
			digi_print(fxc + x_radius); /* x center */
			digi_print(fy3c + y_radius); /* y center */
			m_fprintf("ES clip\n");
		}
		else
		{
			digi_print(fxc + RoundCornerRadiusTL); /* start x */
			digi_print(fyc);		/* start y */
			m_fprintf("M ");	/* move to the UL start */
			dbox_sub( fx2c, fyc, fx2c, fy3c); /* upper right */
			dbox_sub( fx2c, fy3c, fxc, fy3c); /* lower right */
			dbox_sub( fxc, fy3c, fxc, fyc); /* lower left */
			dbox_sub( fxc, fyc, fxc + RoundCornerRadiusTL, fyc); /* upr left */
			m_fprintf("closepath clip\n");
		}
	}
}
