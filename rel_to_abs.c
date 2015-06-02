/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include <stdio.h>
#include <string.h>
#include "window.h"
#include "interfer.h"
#include "rel_data.h"
#include "frame.h"
#include "lmt.f"

int32 lmt_rel_to_abs(WYSIWYG *wn,
					 int abs_index, int rel_index, RELATIVES *rel_data)
{
	int32 values[2];
	int16 i, b = 0, r;

	values[0] = 0;
	values[1] = 0;
	for(i = 0; i <= 1; i++)
	{
		switch(rel_index)
		{
		  case 0:
			b = rel_data -> i0;
			break;
		  case 1:
			b = rel_data -> i1;
			break;
		  case 2:
			b = rel_data -> i2;
			break;
		  case 3:
			b = rel_data -> i3;
			break;
		  case 4:
			b = rel_data -> i4;
			break;
		  case 5:
			b = rel_data -> i5;
			break;
		  case 6:
			b = rel_data -> i6;
			break;
		  case 7:
			b = rel_data -> i7;
			break;
		}
		r = (b >> 8);
		b &= 0377;
		switch(r)
		{
		  case 0102:			/* same as */
			switch (b)
			{
			  case 255:
				switch (abs_index)
				{
				  case 0:
					break;
				  case 1:
					values[i] = lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH);
					break;
				  case 2:
					values[i]
						= lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH) >> 1;
					break;
				  case 3:
					break;
				  case 4:
					values[i] = lmt_off_to_abs(wn, X_REF, TRIM_WIDTH);
					break;
				  case 5:
					values[i]
						= lmt_off_to_abs(wn, X_REF, PAGE_WIDTH) >> 1;
					break;
				  case 6:
					LAYOUT(wn) dpo
						+= lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH);
					break;
				  case 7:
					LAYOUT(wn) wo
						+= lmt_off_to_abs(wn, X_REF, TRIM_WIDTH);
					break;
				}
				break;
			  case 254:
				switch (abs_index)
				{
				  case 0:
					values[i] = lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
					break;
				  case 1:
					values[i] = lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH);
					break;
				  case 2:
					values[i] = (lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN)
								 + lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH)) >> 1;
					break;
				  case 3:
					values[i] = lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
					break;
				  case 4:
					values[i] = lmt_off_to_abs(wn, X_REF, PAGE_WIDTH);
					break;
				  case 5:
					values[i] = (lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN)
								 + lmt_off_to_abs(wn, X_REF, PAGE_WIDTH)) >> 1;
					break;
				  case 6:
					LAYOUT(wn) dpo
						+= lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH);
					break;
				  case 7:
					LAYOUT(wn) wo
						+= lmt_off_to_abs(wn, X_REF, PAGE_WIDTH);
					break;
				}
				break;
			  default:
				switch(abs_index)
				{
				  case 0:
					values[i] = FRAME_DATA(b) inter_top;
					break;
				  case 1:
					values[i] = FRAME_DATA(b) inter_bottom;
					break;
				  case 2:
					if (b)
						values[i] = FRAME_DATA(b) inter_y_center;
					else
						values[i] = FRAME_DATA(b) inter_bottom >> 1;
					break;
				  case 3:
					values[i] = FRAME_DATA(b) inter_left;
					break;
				  case 4:
					values[i] = FRAME_DATA(b) inter_right;
					break;
				  case 5:
					if (b)
						values[i] = FRAME_DATA(b) inter_x_center;
					else
						values[i] = FRAME_DATA(b) inter_right >> 1;
					break;
				  case 6:
					LAYOUT(wn) dpo += FRAME_DATA(b) inter_bottom -
						FRAME_DATA(b) inter_top;	/* depth */
					break;
				  case 7:
					LAYOUT(wn) wo += FRAME_DATA(b) inter_right -
						FRAME_DATA(b) inter_left;	/* width */
					break;
				}
			}
			break;
		  case 0122:			/* FL */
			values[i] = FRAME_DATA(b) inter_top 
				+ lmt_off_to_abs(wn, Y_REF, LEADING(b))
					- lmt_off_to_abs(wn, Y_REF, rel_data -> LDG);
			break;
		  case 0142:			/* CH */
			{
				int32 ps, ld;
				float cap_current, cap_ref, result; 

				if (rel_data -> TF == PL_TEXT || rel_data -> TF == PL_MISC ||
					rel_data -> TF == PL_FLOW)
				{
					ps = lmt_off_to_abs(wn, X_REF, rel_data -> PS) * wn -> ldb;
					ld = lmt_off_to_abs(wn, Y_REF, rel_data-> LDG) * wn -> msb;
					cap_current = (float)ld - (float)rel_data -> CH
						* (float)ps / 100.;
				}
				else
					cap_current = 0.;
				if (TYPE_OF_FRAME(b) == PL_TEXT || 
					TYPE_OF_FRAME(b) == PL_MISC || TYPE_OF_FRAME(b) == PL_FLOW)
				{
					ps = lmt_off_to_abs(wn, X_REF, POINT_SIZE(b)) * wn -> ldb;
					ld = lmt_off_to_abs(wn, Y_REF, LEADING(b)) * wn -> msb;
					cap_ref = (float)ld - (float)CAP_HT(b) * (float)ps / 100.;
				}
				else
					cap_ref = 0.;
				result = (cap_ref - cap_current) / (float)(long)wn -> msb;
				(result > 0.) ? (result += 0.5) : (result -= 0.5);
				values[i] = FRAME_DATA(b) inter_top + (int32)result;
				break;
			}
		  case 020:				/* LT OR TT (both equal 0) */
			values[i] = 0;
			break;
		  case 021:				/* LP OR TP */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] =  lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
			if (abs_index > 2 && abs_index !=6)		/* horizontal */
				values[i] =  lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
			break;
		  case 022:				/* LF OR TF */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] = FRAME_DATA(b) inter_top;
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] = FRAME_DATA(b) inter_left;
			break;
		  case 023:				/* LG OR TG */
			if (abs_index < 3 || abs_index == 6) /* vertical */
			{
				values[i] = FRAME_DATA(b) inter_top;
				if ((TYPE_OF_FRAME(b) == PL_GRAPHIC) 
					&& (rel_data -> TF == PL_GRAPHIC))
				{
					if (GR_TOP_GUTTER(b) >= rel_data -> GR_BG)
						values[i] -= lmt_off_to_abs(wn,Y_REF,GR_TOP_GUTTER(b));
					else
						values[i] -=
							lmt_off_to_abs(wn, Y_REF, rel_data -> GR_BG);
				}
				else
				{
					if (TOP_GUTTER(b) >= rel_data -> BG)
						values[i] -= lmt_off_to_abs(wn, Y_REF, TOP_GUTTER(b));
					else
						values[i] -= lmt_off_to_abs(wn, Y_REF, rel_data -> BG);
				}
			}
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
			{
				values[i] = FRAME_DATA(b) inter_left;
				if ((TYPE_OF_FRAME(b) == PL_GRAPHIC) 
					&& (rel_data -> TF == PL_GRAPHIC))
				{
					if (GR_LEFT_GUTTER(b) >= rel_data -> GR_RG)
						/* NOTE: ???? we changed from 0 to X_REF */
						values[i] -=
							lmt_off_to_abs(wn, X_REF, GR_LEFT_GUTTER(b));
					else
						values[i] -=
							lmt_off_to_abs(wn, X_REF, rel_data -> GR_RG);
				}
				else
				{
					if (LEFT_GUTTER(b) >= rel_data -> RG)
						values[i] -= lmt_off_to_abs(wn, X_REF, LEFT_GUTTER(b));
					else
						values[i] -= lmt_off_to_abs(wn, X_REF, rel_data -> RG);
				}
			}
			break;
		  case 040:				/* RT OR BT */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] =  lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH);
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] =  lmt_off_to_abs(wn, X_REF, TRIM_WIDTH);
			break;
		  case 041:				/* RP OR BP */
			if (abs_index < 3 || abs_index == 6) /* vertical */
			{
				values[i] =  lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH);
				values[i] +=  lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
			}
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
			{
				values[i] =  lmt_off_to_abs(wn, X_REF, PAGE_WIDTH);
				values[i] +=  lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
			}
			break;
		  case 042:				/* RF OR BF */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] = FRAME_DATA(b) inter_bottom;
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] = FRAME_DATA(b) inter_right;
			break;
		  case 043:				/* RG OR BG */
			if (abs_index < 3 || abs_index == 6) /* vertical */
			{
				values[i] = FRAME_DATA(b) inter_bottom;
				if ((TYPE_OF_FRAME(b) == PL_GRAPHIC) && 
					(rel_data -> TF == PL_GRAPHIC))
				{
					if (GR_BOT_GUTTER(b) >= rel_data -> GR_TG)
						values[i] +=
							lmt_off_to_abs(wn, Y_REF, GR_BOT_GUTTER(b));
					else
						values[i] +=
							lmt_off_to_abs(wn, Y_REF, rel_data -> GR_TG);
				}
				else
				{
					if (BOT_GUTTER(b) >= rel_data -> TG)
						values[i] +=  lmt_off_to_abs(wn, Y_REF, BOT_GUTTER(b));
					else
						values[i] += lmt_off_to_abs(wn, Y_REF, rel_data -> TG);
				}
			}
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
			{
				values[i] = FRAME_DATA(b) inter_right;
				if ((TYPE_OF_FRAME(b) == PL_GRAPHIC) &&
					(rel_data -> TF == PL_GRAPHIC))
				{
					if (GR_RIGHT_GUTTER(b) >= rel_data -> GR_LG)
						values[i] +=  lmt_off_to_abs(wn, X_REF, GR_RIGHT_GUTTER(b));
					else
						values[i] +=  lmt_off_to_abs(wn, X_REF,	rel_data -> GR_LG);
				}
				else
				{
					if (RIGHT_GUTTER(b) >= rel_data -> LG)
						values[i] +=  lmt_off_to_abs(wn, X_REF, RIGHT_GUTTER(b));
					else
						values[i] +=  lmt_off_to_abs(wn, X_REF,	rel_data -> LG);
				}
			}
			break;
		  case 060:				/* CT */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] = ((lmt_off_to_abs(wn, Y_REF, TRIM_DEPTH)) / 2);
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] = ((lmt_off_to_abs(wn, X_REF, TRIM_WIDTH)) / 2);
			break;
		  case 061:				/* CP */
			if (abs_index < 3 || abs_index == 6) /* vertical */
			{
				values[i] =  lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH);
				values[i] +=  lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
				/* establish bottom */
				values[i] +=  lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
				/* add top */
				values[i] /= 2;
			}
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
			{
				values[i] =  lmt_off_to_abs(wn, X_REF, PAGE_WIDTH);
				values[i] +=  lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
				/* establish right */
				values[i] +=  lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
				/* add left */
				values[i] /= 2;
			}
			break;
		  case 062:				/* CB */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] = FRAME_DATA(b) inter_y_center;
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] = FRAME_DATA(b) inter_x_center;
			break;
		  case 063:		/* CG, centers right or bottom bearoff for now */
			if (abs_index < 3 || abs_index == 6) /* vertical */
			{
				values[i] = FRAME_DATA(b) inter_bottom;
				values[i] += ((lmt_off_to_abs(wn, Y_REF, BOT_GUTTER(b))) / 2);
			}
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
			{
				values[i] = FRAME_DATA(b) inter_right;
				values[i] += ((lmt_off_to_abs(wn, X_REF, RIGHT_GUTTER(b))) / 2);
			}
			break;
		  case 0100:						/* TM LM margin */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] = lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN)
					+ lmt_off_to_abs(wn, Y_REF, TOP_MARGIN);
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] = lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN)
					+ lmt_off_to_abs(wn, X_REF, LEFT_MARGIN);
			break;
		  case 0101:						/* BM RM margin */
			if (abs_index < 3 || abs_index == 6) /* vertical */
				values[i] = lmt_off_to_abs(wn, Y_REF, PAGE_DEPTH)
					+ lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN)
						- lmt_off_to_abs(wn, Y_REF, BOT_MARGIN);
			if (abs_index > 2 && abs_index !=6)	/* horizontal */
				values[i] = lmt_off_to_abs(wn, X_REF, PAGE_WIDTH)
					+ lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN)
						- lmt_off_to_abs(wn, X_REF, RIGHT_MARGIN);
			break;
		  default:
			break;
		}
		rel_index++;
		if (!values[i])
			i++;
    }
	if (values[1])
		values[0] = (values[0] + values[1]) / 2;
	return(values[0]);
}
