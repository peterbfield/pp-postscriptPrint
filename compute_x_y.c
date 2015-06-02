/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include "window.h"
#include "interfer.h"
#include "frame.h"
#include "rel_data.h"
#include "lmt.f"

void lmt_compute_x(WYSIWYG *wn, int i)
{
	int32 h, wr;
	
	h =  lmt_off_to_abs(wn, X_REF, X_OFFSET(i)); /* horizontal offset */
	wr = lmt_rel_to_abs(wn, 7, 4, &FRAME_DATA(i) rel_data);
	/* i4 width relationship */
	switch(LOCKPOINT(i))		/* the lock point */
	{
	  case TL:					/* 'x' = left */
	  case BL:
	  case CL:
	  case HL:
		h += lmt_rel_to_abs(wn, 3, 0, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		ELE_DATA(i) rot_rect_left = h; /* left */
		if (wr)					/* right */
			ELE_DATA(i) rot_rect_right = wr + LAYOUT(wn) wo;
		else
			ELE_DATA(i) rot_rect_right = h + LAYOUT(wn) wo;
		ELE_DATA(i) rot_rect_x_center =	/* x-center */
			(ELE_DATA(i) rot_rect_right + h) / 2;
		break;
	  case TR:					/* 'x' = right */
	  case BR:
	  case CR:
	  case HR:
		h += lmt_rel_to_abs(wn, 4, 0, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		ELE_DATA(i) rot_rect_right = h;	/* right */
		if (wr)					/* left */
			ELE_DATA(i) rot_rect_left = wr + LAYOUT(wn) wo;
		else
			ELE_DATA(i) rot_rect_left = h - LAYOUT(wn) wo;
		ELE_DATA(i) rot_rect_x_center =	/* x-center */
			(h + ELE_DATA(i) rot_rect_left) / 2;
		break;
	  case TC:					/* 'x' = center */
	  case BC:
	  case CC:
	  case HC:
		h += lmt_rel_to_abs(wn, 5, 0, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		ELE_DATA(i) rot_rect_x_center = h; /* x-center */
		if (wr)
		{
			LAYOUT(wn) wo += wr;
			LAYOUT(wn) wo -= h;
			(LAYOUT(wn) wo < 0) ? (LAYOUT(wn) wo *= -2) : (LAYOUT(wn) wo *= 2);
		}
		ELE_DATA(i) rot_rect_left = (h - (LAYOUT(wn) wo / 2)); /* left */
		ELE_DATA(i) rot_rect_right = (h + (LAYOUT(wn) wo / 2));	/* right */
		break;
	}
}

void lmt_compute_y(WYSIWYG *wn, int i)
{
	int32 v, dr;
	
	v =  lmt_off_to_abs(wn, Y_REF, Y_OFFSET(i)); /* vertical offset */
	dr = lmt_rel_to_abs(wn, 6, 6, &FRAME_DATA(i) rel_data);
	/* i6 depth relationship */
	switch(LOCKPOINT(i))		/* the lock point */
	{
	  case TL:					/* 'y' = top */
	  case TR:
	  case TC:
		v += lmt_rel_to_abs(wn, 0, 2, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		v += lmt_lock_lead(wn, i, v);
		ELE_DATA(i) rot_rect_top = v; /* top */
		if (dr)					/* bottom */
			ELE_DATA(i) rot_rect_bottom = dr + LAYOUT(wn) dpo;
		else
			ELE_DATA(i) rot_rect_bottom = v + LAYOUT(wn) dpo;
		ELE_DATA(i) rot_rect_y_center =	/* y-center */
			(ELE_DATA(i) rot_rect_bottom + ELE_DATA(i) rot_rect_top) / 2 ;
		break;
	  case HL:					/* 'y' = Height */
	  case HR:
	  case HC:
		v += lmt_rel_to_abs(wn, 0, 2, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		if (TYPE_OF_FRAME(i) == PL_TEXT || 
			TYPE_OF_FRAME(i) == PL_MISC || 
			TYPE_OF_FRAME(i) == PL_FLOW)
			v -= (int32)(((float)lmt_off_to_abs(wn, Y_REF, LEADING(i))
						  / (float)wn -> ldb - (float)CAP_HT(i)
						  * (float)lmt_off_to_abs(wn, X_REF, POINT_SIZE(i))
						  / (float)(wn -> msb * 100L)) * (float)wn -> ldb);
		v += lmt_lock_lead(wn, i, v);
		ELE_DATA(i) rot_rect_top = v; /* top */
		if (dr)					/* bottom */
			ELE_DATA(i) rot_rect_bottom = dr + LAYOUT(wn) dpo;
		else
			ELE_DATA(i) rot_rect_bottom = ELE_DATA(i) rot_rect_top
				+ LAYOUT(wn) dpo;
		ELE_DATA(i) rot_rect_y_center =	/* y-center */
			(ELE_DATA(i) rot_rect_bottom + ELE_DATA(i) rot_rect_top) / 2 ;
		break;
	  case BL:					/* 'y' = bottom */
	  case BR:
	  case BC:
		v += lmt_rel_to_abs(wn, 1, 2, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		ELE_DATA(i) rot_rect_bottom = v; /* bottom */
		if (dr)					/* top */
			ELE_DATA(i) rot_rect_top = dr + LAYOUT(wn) dpo +
				lmt_lock_lead(wn, i, dr + LAYOUT(wn) dpo);
		else
		{
			v += lmt_lock_lead(wn, i, v - LAYOUT(wn) dpo);
			ELE_DATA(i) rot_rect_top = v - LAYOUT(wn) dpo;
			ELE_DATA(i) rot_rect_bottom = v; /* bottom */
		}
		ELE_DATA(i) rot_rect_y_center =	/* y-center */
			(v + ELE_DATA(i) rot_rect_top) / 2 ;
		break;
	  case CL:					/* 'y' = center */
	  case CR:
	  case CC:
		{
			int32 a;
			
			v += lmt_rel_to_abs(wn, 2, 2, &FRAME_DATA(i) rel_data);
			/* offset + relationship */
			ELE_DATA(i) rot_rect_y_center = v; /* y-center */
			if (dr)
			{
				LAYOUT(wn) dpo += dr;
				LAYOUT(wn) dpo -= v;
				(LAYOUT(wn) dpo < 0) ? (LAYOUT(wn) dpo *= -2) :
					(LAYOUT(wn) dpo *= 2);
			}
			a = lmt_lock_lead(wn, i, v - (LAYOUT(wn) dpo / 2));
			v += a;
			ELE_DATA(i) rot_rect_top = (v - (LAYOUT(wn) dpo / 2)); /* top */
			if (dr)
				v -= a;
			ELE_DATA(i) rot_rect_bottom = (v + (LAYOUT(wn) dpo / 2)); /* bot */
		}
		break;
	}
}

void lmt_compute_param_x(WYSIWYG *wn, int i, int32 *anchor_x, int32 *width)
{
	int32 h, wr, left = 0, right = 0, x_center;
	
	LAYOUT(wn) wo =  lmt_off_to_abs(wn, X_REF, WIDTH_OFFSET(i)); /*wid offst */
	h =  lmt_off_to_abs(wn, X_REF, X_OFFSET(i)); /* horizontal offset */
	wr = lmt_rel_to_abs(wn, 7, 4, &FRAME_DATA(i) rel_data);
	/* i4 width relationship */
	switch(LOCKPOINT(i))		/* the lock point */
	{
	  case TL:					/* 'x' = left */
	  case BL:
	  case CL:
	  case HL:
		h += lmt_rel_to_abs(wn, 3, 0, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		left = h;
		if (wr)
			right = wr + LAYOUT(wn) wo;
		else
			right = h + LAYOUT(wn) wo;
		*anchor_x = left;
		break;
	  case TR:					/* 'x' = right */
	  case BR:
	  case CR:
	  case HR:
		h += lmt_rel_to_abs(wn, 4, 0, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		right = h;
		if (wr)
			left = wr + LAYOUT(wn) wo;
		else
			left = h - LAYOUT(wn) wo;
		*anchor_x = right;
		break;
	  case TC:					/* 'x' = center */
	  case BC:
	  case CC:
	  case HC:
		h += lmt_rel_to_abs(wn, 5, 0, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		x_center = h;			/* x-center */
		if (wr)
		{
			LAYOUT(wn) wo += wr;
			LAYOUT(wn) wo -= h;
			(LAYOUT(wn) wo < 0) ? (LAYOUT(wn) wo *= -2) : (LAYOUT(wn) wo *= 2);
		}
		left = (h - (LAYOUT(wn) wo / 2)); /* left */
		right = (h + (LAYOUT(wn) wo / 2)); /* right */
		*anchor_x = x_center;
		break;
	}
	*width = right - left;
}

void lmt_compute_param_y(WYSIWYG *wn, int i, int32 *anchor_y, int32 *depth)
{
	int32 v, dr, top = 0, bottom = 0, y_center;
	
	LAYOUT(wn) dpo = lmt_off_to_abs(wn, Y_REF, DEPTH_OFFSET(i)); /* dep offst*/
	v =  lmt_off_to_abs(wn, Y_REF, Y_OFFSET(i)); /* vertical offset */
	dr = lmt_rel_to_abs(wn, 6, 6, &FRAME_DATA(i) rel_data);
	/* i6 depth relationship */
	switch(LOCKPOINT(i))		/* the lock point */
	{
	  case TL:					/* 'y' = top */
	  case TR:
	  case TC:
		v += lmt_rel_to_abs(wn, 0, 2, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		v += lmt_lock_lead(wn, i, v);
		top = v;      		/* top */
		if (dr)					/* bottom */
			bottom = dr + LAYOUT(wn) dpo;
		else
			bottom = v + LAYOUT(wn) dpo;
		*anchor_y = top;
		break;
	  case HL:					/* 'y' = Height */
	  case HR:
	  case HC:
		v += lmt_rel_to_abs(wn, 0, 2, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		if (TYPE_OF_FRAME(i) == PL_TEXT || TYPE_OF_FRAME(i) == PL_MISC	|| 
			TYPE_OF_FRAME(i) == PL_FLOW)
			v -= (int32)(((float)lmt_off_to_abs(wn, Y_REF, LEADING(i))
						  / (float)wn -> ldb - (float)CAP_HT(i)
						  * (float)lmt_off_to_abs(wn, X_REF, POINT_SIZE(i))
						  / (float)(wn -> msb * 100L)) * (float)wn -> ldb);
		v += lmt_lock_lead(wn, i, v);
		top = v;      		/* top */
		if (dr)
			bottom = dr + LAYOUT(wn) dpo;
		else
			bottom = top + LAYOUT(wn) dpo;
		*anchor_y = top;
		break;
	  case BL:					/* 'y' = bottom */
	  case BR:
	  case BC:
		v += lmt_rel_to_abs(wn, 1, 2, &FRAME_DATA(i) rel_data);
		/* offset + relationship */
		bottom = v;
		if (dr)
			top = dr + LAYOUT(wn) dpo + lmt_lock_lead(wn,i,dr+ LAYOUT(wn) dpo);
		else
		{
			v += lmt_lock_lead(wn, i, v - LAYOUT(wn) dpo);
			top = v - LAYOUT(wn) dpo;
			bottom = v;
		}
		*anchor_y = bottom;
		break;
	  case CL:					/* 'y' = center */
	  case CR:
	  case CC:
		{
			int32 a;
			
			v += lmt_rel_to_abs(wn, 2, 2, &FRAME_DATA(i) rel_data);
			/* offset + relationship */
			y_center = v;		/* y-center */
			if (dr)
			{
				LAYOUT(wn) dpo += dr;
				LAYOUT(wn) dpo -= v;
				(LAYOUT(wn) dpo < 0) ? (LAYOUT(wn) dpo *= -2) :
					(LAYOUT(wn) dpo *= 2);
			}
			a = lmt_lock_lead(wn, i, v - (LAYOUT(wn) dpo / 2));
			v += a;
			top = (v - (LAYOUT(wn) dpo / 2)); /* top */
			if (dr)
				v -= a;
			bottom = (v + (LAYOUT(wn) dpo / 2)); /* bottom */
			*anchor_y = y_center;
		}
		break;
	}
	*depth = bottom - top;
}
