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
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include "rel_data.h"
#include "frame.h"
#include "locks.h"
#include "menu_frame.h"
#include "menus.h"				/* Defines ANY_MENU */
#include "traces.h"
#include "widget_list.h"
#include "lmt.f"

#define ADD	 1
#define SUB	-1
#define RELATIVE_COMMAND	'*'

extern int16 lmt_parse_numeric(REL_WORDS *syntax,  
							   char input[], int m, char sign);
extern void lmt_syntax_to_ascii(REL_WORDS *syntax, char list[], char buffer[]);
extern void lmt_syntax_to_ascii_pts(REL_WORDS *syntax, char buffer[]);
extern int16 lmt_size_parser(REL_WORDS *syntax, char input[], int dflt);
extern int16 lmt_xy_syntax_parser(REL_WORDS *syntax, char input[],char list[],
								  int no_default, int frame, int tot_frame);
extern void UpdateLeading(WYSIWYG *wn, int16 fn);

extern char lmt_x_syntax_options[];
extern char lmt_y_syntax_options[];

void make_absolute(WYSIWYG *wn, CNTRLS *control,
				   int frame, char *data, BOOLEAN force_absolute);

void do_mathematic(WYSIWYG *wn, REL_WORDS *syntax, char *data, uint32 *ptr32,
				   int op, int type)
{
	int32 diff, old_value;

	if (!*data)
	{
		*ptr32 = 0;
		return;
	}
	lmt_parse_numeric(syntax, data, 0, '+');
	diff = lmt_off_to_abs(wn, type, syntax -> offset);
	old_value = lmt_off_to_abs(wn, type, *ptr32);
	if (op == ADD)				/* Add */
		old_value += diff;
	else						/* Subtract */
		old_value -= diff;
	*ptr32 = lmt_abs_to_off(wn, type, *ptr32 & PL_REL_BITS, old_value);
}
int32 do_check_mathematic(WYSIWYG *wn, REL_WORDS *syntax, char *data, uint32 *ptr32,
				   int op, int type)
{
	int32 diff, old_value;

	if (!*data)
	{
		*ptr32 = 0;
		return 0;
	}
	lmt_parse_numeric(syntax, data, 0, '+');
	diff = lmt_off_to_abs(wn, type, syntax -> offset);
	old_value = lmt_off_to_abs(wn, type, *ptr32);
	if (op == ADD)				/* Add */
		old_value += diff;
	else						/* Subtract */
		old_value -= diff;
        return old_value;
}

/*ARGSUSED*/
int MFSynPicas(WYSIWYG *wn, CNTRLS *control, int junk, char *data, int to_ac)
{
	REL_WORDS syntax;
	int error = 0;

	memset((char *)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[12];
		
		memset(buffer, 0, sizeof(buffer));
		switch (control->off_rel) 
		{
		  case 0:				/* Col Width */
			syntax.offset = COL_GUIDES(wn).col_width;
			break;
		  case 1:				/* Col Gutter */
			syntax.offset = COL_GUIDES(wn).col_gutter;
			break;
		  case 2:				/* Row Depth */
			syntax.offset = COL_GUIDES(wn).row_depth;
			break;
		  case 3:				/* Row Gutter */
			syntax.offset = COL_GUIDES(wn).row_gutter;
			break;
		  default:				/* Preference Gutters */
			p_info(PI_ELOG, "\n*****PROGRAMMING ERROR: gutters not in preferences\n");
			/*
			  syntax.offset = 
			  *(&wn->global_data.gutter_top + control->off_rel%10);
			  */
		}
		lmt_syntax_to_ascii(&syntax, 0, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		if (!(error = lmt_size_parser(&syntax, data, 0)))
			switch (control->off_rel) 
			{
			  case 0:			/* Col Width */
				COL_GUIDES(wn).col_width = syntax.offset;
				break;
			  case 1:			/* Col Gutter */
				COL_GUIDES(wn).col_gutter = syntax.offset;
				break;
			  case 2:			/* Row Depth */
				COL_GUIDES(wn).row_depth = syntax.offset;
				break;
			  case 3:			/* Row Gutter */
				COL_GUIDES(wn).row_gutter = syntax.offset;
				break;
			  default:			/* Preference Gutters */
				p_info(PI_ELOG, "\n*****PROGRAMMING ERROR: gutters not in preferences\n");
				/*
				 *(&wn->global_data.gutter_top + control->off_rel%10) =
				 syntax.offset;
				 */
				break;
			}
	}
	return(error);
}

/*ARGSUSED*/
int MFIntShort(WYSIWYG *wn, CNTRLS *control, int junk, char *data, int to_ac)
{
	if (to_ac)
	{
		switch (control -> off_rel)
		{
		  case 0:
			*data = (char)COL_GUIDES(wn).column_guides_origin;
			break;
		  case 1:
			sprintf(data, "%d", COL_GUIDES(wn).rows);
			break;
		  case 2:
			sprintf(data, "%d", COL_GUIDES(wn).columns);
			break;
		}
	}
	else
	{
		switch (control -> off_rel)
		{
		  case 0:
			COL_GUIDES(wn).column_guides_origin = (char)*data;
			break;
		  case 1:
			COL_GUIDES(wn).rows = atoi(data);
			break;
		  case 2:
			COL_GUIDES(wn).columns = atoi(data);
			break;
		}
	}
	return(0);
}

int MFPrefSynPts(WYSIWYG *wn, CNTRLS *control,int frame, char *data, int to_ac)
{								/* 0 = picas, 1 = points for lmt_size_parser */
	REL_WORDS syntax;
	int error = 0;

	memset ((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[12];

		memset(buffer, 0, sizeof(buffer));
		switch (control->off_rel)
		{
		  case -1:
			syntax.offset = CUR_LOCKS(wn).v_lines;
			break;
		  case 0:
		  case 1:
		  case 6:
		  case 7:
		  case 8:
		  case 9:
			if (frame)
				syntax.offset = *(&REL_DATA(frame) d0 + control->off_rel);
			else
				syntax.offset = *(&wn->preferences.rule_weight +
								  control->off_rel);
			break;
		  default:
			p_info(PI_ELOG, "MFPrefSynPts Illegal control->off_rel: %d\n", 
					  control->off_rel);
			return(0);
		}
		if (control -> off_rel <= 1)
			lmt_syntax_to_ascii_pts(&syntax, buffer);
		else
			lmt_syntax_to_ascii(&syntax, 0, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		if(!(error = lmt_size_parser(&syntax, data,
									 control->off_rel <= 1 ? 1 : 0)))
			switch (control->off_rel)
			{
			  case -1:
				CUR_LOCKS(wn).v_lines = syntax.offset;
				break;
			  case 0:
			  case 1:
			  case 6:
			  case 7:
			  case 8:
			  case 9:
				if (frame)
					*(&REL_DATA(frame) d0 + control -> off_rel)
						= syntax.offset;
				else
					*(&wn->preferences.rule_weight+control->off_rel)
						= syntax.offset;
				break;
			  default:
				p_info(PI_ELOG, "MFPrefSynPts Illegal control->off_rel: %d\n",
						  control->off_rel);
				break;
			}
    }
	return(error);
}


int MFnonframeHort(WYSIWYG *wn, CNTRLS *control,  
				   int frame, char *data, BOOLEAN to_ac)
{
	REL_WORDS syntax;
	uint32 *offset;
	int32 *x_machine;
	int16 *rel0, *rel1;
	int error = 0;

	offset = &CUR_LOCKS(wn).h_offset;
	rel0 = &CUR_LOCKS(wn).h_rel0;
	rel1 = &CUR_LOCKS(wn).h_rel1;
	x_machine = &CUR_LOCKS(wn).x_machine;
	memset ((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[16];
		
		memset (buffer, 0, sizeof(buffer));
		syntax.offset = *offset;
		syntax.relationships[0] = *rel0;
		syntax.relationships[1] = *rel1;
		lmt_syntax_to_ascii(&syntax, lmt_x_syntax_options, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		uint32 *ptr32;

		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		ptr32 = offset;
		switch (*data)
		{
		  case '+':
			do_mathematic(wn, &syntax, data + 1, ptr32, ADD, X_REF);
			(*x_machine) += lmt_off_to_abs(wn, X_REF, syntax.offset);
			MFnonframeHort(wn, control, frame, data, True);
			break;
		  case '-':
			do_mathematic(wn, &syntax, data + 1, ptr32, SUB, X_REF);
			(*x_machine) -= lmt_off_to_abs(wn, X_REF, syntax.offset);
			MFnonframeHort(wn, control, frame, data, True);
			break;
		  default:
			error = lmt_xy_syntax_parser(&syntax, data, lmt_x_syntax_options,
										 0, frame, (int)TOT_BLOCKS);
			if (error)
				break;
			*ptr32 = syntax.offset;
			*rel0 = syntax.relationships[0];
			*rel1 = syntax.relationships[1];
			if (wn -> absolute_mode)
			{
				RELATIVES rel_data;
				int32 units, machine;

				memset((char *)&rel_data, 0, sizeof(RELATIVES));
				rel_data.XR1 = *rel0;
				rel_data.XR2 = *rel1;
				machine = lmt_rel_to_abs(wn, 3, 0, &rel_data);
				*x_machine = machine += lmt_off_to_abs(wn, X_REF, *ptr32);
				machine -= lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
				units = *ptr32 & PL_REL_BITS;
				if (!units)
					units = PICA << PL_REL_SHIFT;
				*ptr32 = lmt_abs_to_off(wn, X_REF, units, machine);
				*rel0 = 0x1100;
				*rel1 = 0;
				MFnonframeHort(wn, control, frame, data, True);
			}
			break;
		}
    }
	return(error);
}

int MFnonframeVert(WYSIWYG *wn, CNTRLS *control, 
				   int frame, char *data, BOOLEAN to_ac)
{
	REL_WORDS syntax;
	uint32 *offset;
	int32 *y_machine;
	int16 *rel0, *rel1;
	int error = 0;

	offset = &CUR_LOCKS(wn).v_offset;
	rel0 = &CUR_LOCKS(wn).v_rel0;
	rel1 = &CUR_LOCKS(wn).v_rel1;
	y_machine = &CUR_LOCKS(wn).y_machine;
	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[16];

		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *offset;
		syntax.relationships[0] = *rel0;
		syntax.relationships[1] = *rel1;
		lmt_syntax_to_ascii(&syntax, lmt_y_syntax_options, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		uint32 *ptr32;
		RELATIVES rel_data;
		int32 units, machine;
		
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		ptr32 = offset;
		switch (*data)
		{
		  case '+':
			do_mathematic(wn, &syntax, data + 1, ptr32, ADD, Y_REF);
			(*y_machine) +=	lmt_off_to_abs(wn, Y_REF, syntax.offset);
			MFnonframeVert(wn, control, frame, data, True);
			break;
		  case '-':
			do_mathematic(wn, &syntax, data + 1, ptr32, SUB, Y_REF);
			(*y_machine) -=	lmt_off_to_abs(wn, Y_REF, syntax.offset);
			MFnonframeVert(wn, control, frame, data, True);
			break;
		  default:
			error = lmt_xy_syntax_parser(&syntax, data, lmt_y_syntax_options,
										 0, frame, (int)TOT_BLOCKS);
			if (error)
				break;
			*ptr32 = syntax.offset;
			*rel0 = syntax.relationships[0];
			*rel1 = syntax.relationships[1];
			UpdateLeading(wn, *rel0);
			memset((char *)&rel_data, 0, sizeof(RELATIVES));
			rel_data.YR1 = *rel0;
			rel_data.YR2 = *rel1;
			machine = lmt_rel_to_abs(wn, 0, 2, &rel_data);
			(*y_machine) =
				machine += lmt_off_to_abs(wn, Y_REF, *ptr32);
			if (wn -> absolute_mode)
			{
				machine -= lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
				units = *ptr32 & PL_REL_BITS;
				if (!units)
					units = PICA << PL_REL_SHIFT;
				*ptr32 = lmt_abs_to_off(wn, Y_REF, units, machine);
				*rel0 = 0x1100;
				*rel1 = 0;
				MFnonframeVert(wn, control, frame, data, True);
			}
			break;
		}
    }
	return(error);
}


int MFAnyIntChar(WYSIWYG *wn, CNTRLS *control, int frame, char *data, 
				 int to_ac)
{
	if (to_ac)
		sprintf(data, "%d", *(&REL_DATA(frame) c0 + control -> off_rel));
	else
		*(&REL_DATA(frame) c0 + control -> off_rel) = (char)atoi(data);
	return(0);					/* Just so it returns something */
}

int MFAnyStrPtr(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	if (trace_misc)
		p_info(PI_TRACE, "entering MFAnyStrPtr(p%1d:%#x %s data:%#x)\n",
				  control->off_rel, *(&REL_DATA(frame) p0 + control-> off_rel),
				  to_ac ? "-->" : "<--", data);
	
	if (to_ac)
	{
		if (*(&REL_DATA(frame) p0 + control -> off_rel) && data)
			strcpy(data, *(&REL_DATA(frame) p0 + control -> off_rel));
	}
	else if (data)
	{
		if (*(&REL_DATA(frame) p0 + control -> off_rel))
			*(&REL_DATA(frame) p0 + control -> off_rel) =
				p_realloc(*(&REL_DATA(frame) p0	+ control -> off_rel),
						  strlen(data)+1);
		else
			*(&REL_DATA(frame) p0 + control -> off_rel) =
				p_alloc(strlen(data)+1);
		
		strcpy(*(&REL_DATA(frame) p0 + control -> off_rel), data);
	}
	
	if (trace_misc)
		p_info(PI_TRACE, "leaving MFAnyStrPtr(p%1d:%#x %s data:%#x (%s))\n",
				  control->off_rel, *(&REL_DATA(frame) p0 + control-> off_rel),
				  to_ac ? "-->" : "<--", data, data ? data : "NULL");
	return(0);					/* Just so it returns something */
}

int MFAnyFloZoom(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
    if(*(&REL_DATA(frame) i0 + control -> off_rel) == 0)
        *(&REL_DATA(frame) i0 + control -> off_rel) = ZOOM_UNIT;
    if (to_ac)
	{
		if (data != NULL)
			sprintf(data, "%.1f", (float) *(&REL_DATA(frame) i0 + control -> off_rel)/10);
	}
    else
        *(&REL_DATA(frame) i0 + control -> off_rel) = (int)(atof(data)*10.);
    return(0);					/* Just so it returns something */
}

int MFAnyFloDeg(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{

    if (to_ac)
	{
		if (data != NULL)
			sprintf(data, "%.1f", (float) *(&REL_DATA(frame) i0 + control -> off_rel)/10);
	}
    else
        *(&REL_DATA(frame) i0 + control -> off_rel) = (int)(atof(data)*10.);
    return(0);					/* Just so it returns something */
}

int MFAnyBlDeg(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{

  if (to_ac)
    {
      if (data != NULL)
	{
	  switch (*(&REL_DATA(frame) i0 + control -> off_rel))
	    {
	    case BLEND_ANGLE_L:
	      sprintf(data, "%c",'L');
	      break;
	    case BLEND_ANGLE_R:
	      sprintf(data, "%c",'R');
	      break;
	    case BLEND_ANGLE_T:
	      sprintf(data, "%c",'T');
	      break;
	    default:
	      sprintf(data, "%.1f", (float) *(&REL_DATA(frame) i0 + control -> off_rel)/10);
	    }
	}
    }
  else
    {
      switch (toupper(data[0]))
	{
	case 'L':
	  *(&REL_DATA(frame) i0 + control -> off_rel) = BLEND_ANGLE_L;
	  break;
	case 'R':
	  *(&REL_DATA(frame) i0 + control -> off_rel) = BLEND_ANGLE_R;	  
	  break;
	case 'T':
	  *(&REL_DATA(frame) i0 + control -> off_rel) = BLEND_ANGLE_T;
	  break;
	default:
	  *(&REL_DATA(frame) i0 + control -> off_rel) = (int)(atof(data)*10.);
	}
    }
  return(0);					/* Just so it returns something */
}

int MFAnyIntZoom(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
    if(*(&REL_DATA(frame) i0 + control -> off_rel) == 0)
        *(&REL_DATA(frame) i0 + control -> off_rel) = 100;
    if (to_ac)
        sprintf(data, "%d", *(&REL_DATA(frame) i0 + control -> off_rel));
    else
        *(&REL_DATA(frame) i0 + control -> off_rel) = atoi(data);
    return(0);					/* Just so it returns something */
}

int MFAnyIntShort(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
	if (to_ac)
		sprintf(data, "%d", *(&REL_DATA(frame) i0 + control -> off_rel));
	else
		*(&REL_DATA(frame) i0 + control -> off_rel) = atoi(data);
	return(0);					/* Just so it returns something */
}

int MFAnyFloShort(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{								/* Old m_ps_ss_ld */
	if(to_ac)
    {
		char *ptr;

		sprintf(data, "%.2f", ((float)(long)*(&REL_DATA(frame) i0
											  + control -> off_rel) / 100.));
		ptr = data + strlen(data) - 1;
		while (*ptr == '0') *ptr-- = 0;
		if (*ptr == '.')
			*ptr = 0;
    }		
	else
		*(&REL_DATA(frame) i0 + control -> off_rel) =
			(short)(long)(atof(data) * 100.);
	return(0);					/* Just so it returns something */
}

int MFFlag(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	if (to_ac)
		*data = ((!frame ? TRIM_FLAGS : FRAME_FLAGS(frame))
				 >> control -> off_rel) & 0x0001;
	else
    {
		if (*data)
		{
			if (frame)
				FRAME_FLAGS(frame) |= 0x0001 << control -> off_rel;
			else
				TRIM_FLAGS |= 0x0001 << control -> off_rel;
		}
		else
		{
			if (frame)
				FRAME_FLAGS(frame) &= ~(0x0001 << control -> off_rel);
			else
				TRIM_FLAGS &= ~(0x0001 << control -> off_rel);
		}
    }
	return(0);
}

int MFMpFlag(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	if (to_ac)
	{
		if (control -> off_rel == 0)
			*data = MP_FLAGS(frame) & 0x03;
		else if (control -> off_rel == 1)
			*data = (MP_FLAGS(frame) & 0x04) >> 2;
	}
	else
	{
		if (control -> off_rel == 0)
			MP_FLAGS(frame) = (MP_FLAGS(frame) & ~0x03) | *data;
		else if (control -> off_rel == 1)
			MP_FLAGS(frame) = (MP_FLAGS(frame) & 0x03) | (*data << 2);
	}
	return(0);
}

int MFStoreIntChar(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
	if (to_ac)
		*data = (char) *(&REL_DATA(frame) c0 + control -> off_rel);
	else
		*(&REL_DATA(frame) c0 + control -> off_rel) = (char)*data;
	return(0);
}

int MFSyntaxPts(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
	REL_WORDS syntax;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[12];

		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		lmt_syntax_to_ascii_pts(&syntax, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		if (!(error = lmt_size_parser(&syntax, data, 1)))
			*(&REL_DATA(frame) d0 + control -> off_rel) = syntax.offset;
    }
	return(error);
}

int MFSyntaxPicas(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
	REL_WORDS syntax;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[12];
		
		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		lmt_syntax_to_ascii(&syntax, 0, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		extern CNTRLS controls[];
		
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		if(!(error = lmt_size_parser(&syntax, data, 0)))
		{
			if (TYPE_OF_FRAME(frame) == PL_SPECS && 
				control -> off_rel == controls[wig_p_page_depth].off_rel
				&& ((syntax.offset & PL_REL_BITS) 
					== (LINE_LEAD << PL_REL_SHIFT)))
				syntax.offset = ((syntax.offset & PL_SYNBITS) | 
								 (ORG_LINE_LEAD << PL_REL_SHIFT));
			*(&REL_DATA(frame) d0 + control -> off_rel) = syntax.offset;
		}
    }
	return(error);
}

int MFHorizontal(WYSIWYG *wn, CNTRLS *control, int frame, char *data,int to_ac)
{
	char *ptr;
	REL_WORDS syntax;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[16];

		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		syntax.relationships[0] = X_REL_1(frame);
		syntax.relationships[1] = X_REL_2(frame);
		lmt_syntax_to_ascii(&syntax, lmt_x_syntax_options, buffer);
		if (REL_FLAGS(frame) & ABS_HORIZONTAL)
		{
			*data = RELATIVE_COMMAND;
			ptr = data + 1;
		}
		else
			ptr = data;
		memcpy(ptr, buffer, control -> length);
    }
	else
    {
		uint32 *ptr32;
		
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		ptr32 = &REL_DATA(frame) d0 + control -> off_rel;
		switch (*data)
		{
		  case '+':
			do_mathematic(wn, &syntax, data + 1, ptr32, ADD, X_REF);
			MFHorizontal(wn, control, frame, data, 1);
			break;
		  case '-':
			do_mathematic(wn, &syntax, data + 1, ptr32, SUB, X_REF);
			MFHorizontal(wn, control, frame, data, 1);
			break;
		  default:
			if (*data == RELATIVE_COMMAND)
			{
				REL_FLAGS(frame) |= ABS_HORIZONTAL;
				ptr = data + 1;
			}
			else
			{
				REL_FLAGS(frame) &= ~ABS_HORIZONTAL;
				ptr = data;
			}
			error = lmt_xy_syntax_parser(&syntax, ptr, lmt_x_syntax_options, 0,
										 frame, (int)TOT_BLOCKS);
			if (error)
				break;
			*ptr32 = syntax.offset;
			X_REL_1(frame) = syntax.relationships[0];
			X_REL_2(frame) = syntax.relationships[1];
			if (*data == RELATIVE_COMMAND && (X_REL_1(frame) & 0xFF) < 254
				&& (X_REL_1(frame) & 0xFF) > frame)
			{
				memcpy(data, data + 1, strlen(data) + 1);
				REL_FLAGS(frame) &= ~ABS_HORIZONTAL;
			}
			break;
		}
    }
	return(error);
}

int MFVertical(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	REL_WORDS syntax;
	char *ptr;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[16], lock_lead[12];

		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		syntax.relationships[0] = Y_REL_1(frame);
		syntax.relationships[1] = Y_REL_2(frame);
		lmt_syntax_to_ascii(&syntax, lmt_y_syntax_options, buffer);
		while ((ptr = strrchr(buffer, ' ')) != NULL)
			*ptr = 0;
		switch ((int16)((LOCK_LEAD(frame) & 0xC0000000) >> 30))
		{
		  case 1:
			sprintf(lock_lead, "/U%ld", LOCK_LEAD(frame) & 0x3FFFFFFF);
			strcat(buffer, lock_lead);
			break;
		  case 2:
			sprintf(lock_lead, "/D%ld", LOCK_LEAD(frame) & 0x3FFFFFFF);
			strcat(buffer, lock_lead);
			break;
		  case 3:
			sprintf(lock_lead, "/C%ld", LOCK_LEAD(frame) & 0x3FFFFFFF);
			strcat(buffer, lock_lead);
			break;
		}
		if (REL_FLAGS(frame) & ABS_VERTICAL)
		{
			*data = RELATIVE_COMMAND;
			ptr = data + 1;
		}
		else
			ptr = data;
		memcpy(ptr, buffer, control -> length);
    }
	else
    {
		int16 i;
		uint32 *ptr32;

		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		ptr32 = &REL_DATA(frame) d0 + control -> off_rel;
		ptr = strchr(data, '/');
		if (ptr++)
		{
			i = atoi(ptr + 1);
			if (i < frame)
			{
				switch (*ptr)
				{
				  case 'U':
					LOCK_LEAD(frame) = 0x40000000 | (int32)i;
					break;
				  case 'D':
					LOCK_LEAD(frame) = 0x80000000 | (int32)i;
					break;
				  case 'C':
					LOCK_LEAD(frame) = 0xC0000000 | (int32)i;
					break;
				  default:
					error = 1;
					break;
				}
				*(--ptr) = 0;
			}
			else
				error = 1;
		}
		else
			LOCK_LEAD(frame) = 0L;
		
		
		if (!error)
			switch (*data)
			{
			  case '+':
				do_mathematic(wn, &syntax, data + 1, ptr32, ADD, Y_REF);
				MFVertical(wn, control, frame, data, 1);
				break;
			  case '-':
				do_mathematic(wn, &syntax, data + 1, ptr32, SUB, Y_REF);
				MFVertical(wn, control, frame, data, 1);
				break;
			  default:
				if (*data == RELATIVE_COMMAND)
				{
					REL_FLAGS(frame) |= ABS_VERTICAL;
					ptr = data + 1;
				}
				else
				{
					REL_FLAGS(frame) &= ~ABS_VERTICAL;
					ptr = data;
				}
				error = lmt_xy_syntax_parser(&syntax, ptr, 
											 lmt_y_syntax_options, 0,
											 frame, (int)TOT_BLOCKS);
				if (error)
					break;
				*ptr32 = syntax.offset;
				Y_REL_1(frame) = syntax.relationships[0];
				Y_REL_2(frame) = syntax.relationships[1];
				if (*data == RELATIVE_COMMAND && (Y_REL_1(frame) & 0xFF) < 254
					&& (Y_REL_1(frame) & 0xFF) > frame)
				{
					memcpy(data, data + 1, strlen(data) + 1);
					REL_FLAGS(frame) &= ~ABS_VERTICAL;
				}
				break;
			}
    }
	return(error);
}

int MFWidth(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	char *ptr;
	REL_WORDS syntax;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[16];
		
		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		syntax.relationships[0] = W_REL_1(frame);
		syntax.relationships[1] = W_REL_2(frame);
		lmt_syntax_to_ascii(&syntax, lmt_x_syntax_options, buffer);
		if (REL_FLAGS(frame) & ABS_WIDTH)
		{
			*data = RELATIVE_COMMAND;
			ptr = data + 1;
		}
		else
			ptr = data;
		memcpy(ptr, buffer, control -> length);
    }
	else
    {
		uint32 *ptr32;
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		ptr32 = &REL_DATA(frame) d0 + control -> off_rel;
		switch (*data)
		{
		  case '+':
			do_mathematic(wn, &syntax, data + 1, ptr32, ADD, X_REF);
			MFWidth(wn, control, frame, data, 1);
			break;
		  case '-':
			do_mathematic(wn, &syntax, data + 1, ptr32, SUB, X_REF);
			MFWidth(wn, control, frame, data, 1);
			break;
		  default:
			if (*data == RELATIVE_COMMAND)
			{
				REL_FLAGS(frame) |= ABS_WIDTH;
				ptr = data + 1;
			}
			else
			{
				REL_FLAGS(frame) &= ~ABS_WIDTH;
				ptr = data;
			}
			error = lmt_xy_syntax_parser(&syntax, ptr, lmt_x_syntax_options,
										 1, frame, (int)TOT_BLOCKS);
			if (error)
				break;
			*ptr32 = syntax.offset;
			W_REL_1(frame) = syntax.relationships[0];
			W_REL_2(frame) = syntax.relationships[1];
			if (*data == RELATIVE_COMMAND
				&& (W_REL_1(frame) & 0xFF) < 254
				&& (W_REL_1(frame) & 0xFF) > frame)
			{
				memcpy(data, data + 1, strlen(data) + 1);
				REL_FLAGS(frame) &= ~ABS_WIDTH;
			}
			break;
		}
    }
	return(error);
}

int MFOkWidth(WYSIWYG *wn, CNTRLS *control, int frame, char *data)
{
/* Checks if the width(requested) stored in data does not exceed the frame's size */ 
  char *ptr;
  REL_WORDS syntax;
  uint32 *ptr32;
  int error = 0;
  int32 offset = 0;/* Width after the calculations */
  memset((char*)&syntax, 0, sizeof(REL_WORDS));
  if (strlen(data) > control -> length)
    data[control -> length] = 0;
  ptr32 = &REL_DATA(frame) d0 + control -> off_rel;
  switch (*data)
    {
    case '+':
      offset = do_check_mathematic(wn, &syntax, data + 1, ptr32, ADD, X_REF);
      break;
    case '-':
      offset = do_check_mathematic(wn, &syntax, data + 1, ptr32, SUB, X_REF);
      break;
    default:
      if (*data == RELATIVE_COMMAND)
	{
	  ptr = data + 1;
	}
      else
	{
	  ptr = data;
	}
      error = lmt_xy_syntax_parser(&syntax, ptr, lmt_x_syntax_options,
				   1, frame, (int)TOT_BLOCKS);
      if (error)
	return FALSE;
      else
	offset = syntax.offset;
      break;
    }
/* Do not change width if offset<=0 or more that the original width of a graphic frame */
  if (offset <= 0 || (FRAME_DATA(frame) gr &&
     (lmt_off_to_abs(wn, X_REF, offset) > (FRAME_DATA(frame) prev_right - FRAME_DATA(frame) prev_left - 
     (lmt_off_to_abs(wn, X_REF, CROP_LEFT(frame)) * ZOOM_GR(frame) + HALF_ZOOM_UNIT) / ZOOM_UNIT))))
    return FALSE;
  else
    return TRUE;
}

int MFOkDepth(WYSIWYG *wn, CNTRLS *control, int frame, char *data)
{
/* Checks if the depth(requested) stored in data does not exceed the frame's size */ 
  char *ptr;
  REL_WORDS syntax;
  uint32 *ptr32;
  int error = 0;
  int32 offset = 0;/* Depth after the calculations */
  memset((char*)&syntax, 0, sizeof(REL_WORDS));
  if (strlen(data) > control -> length)
    data[control -> length] = 0;
  ptr32 = &REL_DATA(frame) d0 + control -> off_rel;

  switch (*data)
    {
    case '+':
      offset = do_check_mathematic(wn, &syntax, data + 1, ptr32, ADD, Y_REF);
      break;
    case '-':
      offset = do_check_mathematic(wn, &syntax, data + 1, ptr32, SUB, Y_REF);
      break;
    default:
      if (*data == RELATIVE_COMMAND)
	{
	  ptr = data + 1;
	}
      else
	{
	  ptr = data;
	}
      error = lmt_xy_syntax_parser(&syntax, ptr, lmt_y_syntax_options,
				   1, frame, (int)TOT_BLOCKS);
      if (error)
	return FALSE;
      else
	offset = syntax.offset;
      break;
    }
/* Do not change depth if offset<=0 or more that the original depth of a graphic frame */
  if (offset <= 0 || (FRAME_DATA(frame) gr &&
     (lmt_off_to_abs(wn, Y_REF, offset) > (FRAME_DATA(frame) prev_bottom - FRAME_DATA(frame) prev_top - 
     (lmt_off_to_abs(wn, Y_REF, CROP_TOP(frame)) * ZOOM_GRY(frame) + HALF_ZOOM_UNIT) / ZOOM_UNIT))))
    return FALSE;
  else
    return TRUE;
}

int MFDepth(WYSIWYG *wn, CNTRLS *control, int frame, char *data,  int to_ac)
{
	char *ptr;
	REL_WORDS syntax;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	if (to_ac)
    {
		char buffer[16];

		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		syntax.relationships[0] = D_REL_1(frame);
		syntax.relationships[1] = D_REL_2(frame);
		lmt_syntax_to_ascii(&syntax, lmt_y_syntax_options, buffer);
		if (REL_FLAGS(frame) & ABS_DEPTH)
		{
			*data = '*';
			ptr = data + 1;
		}
		else
			ptr = data;
		memcpy(ptr, buffer, control -> length);
    }
	else
    {
		uint32 *ptr32;

		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		ptr32 = &REL_DATA(frame) d0 + control -> off_rel;
		switch (*data)
		{
		  case '+':
			do_mathematic(wn, &syntax, data + 1, ptr32, ADD, X_REF);
			MFDepth(wn, control, frame, data, 1);
			break;
		  case '-':
			do_mathematic(wn, &syntax, data + 1, ptr32, SUB, X_REF);
			MFDepth(wn, control, frame, data, 1);
			break;
		  default:
			if (*data == RELATIVE_COMMAND)
			{
				REL_FLAGS(frame) |= ABS_DEPTH;
				ptr = data + 1;
			}
			else
			{
				REL_FLAGS(frame) &= ~ABS_DEPTH;
				ptr = data;
			}
			error = lmt_xy_syntax_parser(&syntax, ptr, lmt_y_syntax_options, 1,
										 frame, (int)TOT_BLOCKS);
			if (error)
				break;
			*ptr32 = syntax.offset;
			D_REL_1(frame) = syntax.relationships[0];
			D_REL_2(frame) = syntax.relationships[1];
			if (*data == RELATIVE_COMMAND
				&& (D_REL_1(frame) & 0xFF) < 254
				&& (D_REL_1(frame) & 0xFF) > frame)
			{
				memcpy(data, data + 1, strlen(data) + 1);
				REL_FLAGS(frame) &= ~ABS_DEPTH;
			}
			break;
		}
    }
	return(error);
}

int MFVjDepth(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
	/* PATCH FOR MISPLACEMENT */
	REL_WORDS syntax;
	char *ptr;
	int error = 0;

	memset((char*)&syntax, 0, sizeof(REL_WORDS));
	ptr = &VJ_REL_1_1(frame);
	if (to_ac)
    {
		char buffer[12];

		memset(buffer, 0, sizeof(buffer));
		syntax.offset = *(&REL_DATA(frame) d0 + control -> off_rel);
		syntax.relationships[0] = (int16)*ptr++ << 8;
		syntax.relationships[0] |= (int16)*ptr++;
		syntax.relationships[1] = (int16)*ptr++ << 8;
		syntax.relationships[1] |= (int16)*ptr;
		lmt_syntax_to_ascii(&syntax, lmt_y_syntax_options, buffer);
		memcpy(data, buffer, control -> length);
    }
	else
    {
		if (strlen(data) > control -> length)
			data[control -> length] = 0;
		if(!(error = lmt_xy_syntax_parser(&syntax, data, lmt_y_syntax_options,
										  1, frame, (int)TOT_BLOCKS)))
		{
			*(&REL_DATA(frame) d0 + control -> off_rel) = syntax.offset;
			*ptr++ = (char)(syntax.relationships[0] >> 8 & 0xff);
			*ptr++ = (char)(syntax.relationships[0] & 0xff);
			*ptr++ = (char)(syntax.relationships[1] >> 8 & 0xff);
			*ptr++ = (char)(syntax.relationships[1] & 0xff);
		}
    }
	return(error);
}

int MFSTFIntLong(WYSIWYG *wn, CNTRLS *control, int frame, char *data,int to_ac)
{
	if (to_ac)
		sprintf(data, "%ld",
				(STF_IDS(frame) & (0x000003FF << control -> off_rel))
				>> (10 * control -> off_rel));
	else
		STF_IDS(frame) = (STF_IDS(frame) 
						  & ~(0x3FF << (10 * control -> off_rel)))
			| (atol(data) << (10 * control -> off_rel));
	return(0);
}

int MFMiscID(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	if(to_ac)
		sprintf(data, "%u", *(&REL_DATA(frame) i0 + control -> off_rel));
	else
		*(&REL_DATA(frame) i0 + control -> off_rel) = atoi(data);
	return(0);
}

int MF20IntShort(WYSIWYG *wn, CNTRLS *control, int frame, char *data,int to_ac)
{			/* Old m_20numeric */
	char *ptr, *ptr2;
	float l;

	if (to_ac)
    {
		char buffer[12];

		memset(buffer, 0, sizeof(buffer));
		l = (float)(long)*(&REL_DATA(frame) i0 + control -> off_rel);
		l /= 20.;
		ptr = ptr2 = buffer;
		sprintf(ptr2, "%.3f", l);
		while (*ptr == '0')
			ptr++;
		ptr2 = buffer + strlen(buffer) - 1;
		while (*ptr2 == '0')
			*ptr2-- = 0;
		if (*ptr2 == '.')
			*ptr2 = 0;
		if (*ptr == 0)
			sprintf(data, "%s", "0");
		else
			sprintf(data, "%s", ptr);
    }
	else
    {
		int total = 0;

		ptr = strchr(data, 'P');
		if (!ptr)
			ptr = strchr(data, 'p');
		if (ptr)
		{
			*ptr++ = 0;
			if (*data)
				total = atoi(data) * 12;
		}
		else
		{
			ptr = strchr(data, '.');
			if (ptr)
			{
				ptr2 = strchr(ptr + 1, '.');
				if (ptr2)
				{
					*ptr++ = 0;
					if (*data)
						total = atoi(data) * 12;
				}
				else
					ptr = data;
			}
			else
				ptr = data;
		}
		l = (atof(ptr) + (float)total) * 20. + .5;
		*(&REL_DATA(frame) i0 + control -> off_rel) = (short)(long)l;
    }
	return(0);
}

int MFLinkNumber(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				 int to_ac)
{
	char *ptr;
	int16 i;

	/* For command_flags, care only for BF and EF */
	if (to_ac)
	{
		i = *(&REL_DATA(frame) i0 + control -> off_rel);
		if (PREV_LAY(frame) || NEXT_LAY(frame)) {
		  int len;
		  ptr = p_alloc(LONG_STRING);
		  if (!ptr)
		    p_warning("Could not allocate memory\n");
			sprintf(ptr, "%d; %d; %ld; %ld; %ld; %ld",
					i, FRAME_DATA(frame) Fmap_data -> command_flags,
					PREV_LAY(frame), PREV_TEXT_ID(frame),
					NEXT_LAY(frame), NEXT_TEXT_ID(frame));
			len = strlen(ptr);
			if (len >= LINK_STRING_LENGTH)
			  p_warning("Layout links string is too long, can't write it\n");
			else
			  memcpy(data, ptr, len);
			p_free(ptr);
		}
		else if (FRAME_DATA(frame) Fmap_data
				 && FRAME_DATA(frame) Fmap_data -> command_flags)
			sprintf(data, "%d; %d", i,
					FRAME_DATA(frame) Fmap_data -> command_flags & 0x03);
		else if (i && (TYPE_OF_FRAME(frame) == PL_TEXT))
			sprintf(data, "%d; 3", i);
		else if (i)
			sprintf(data, "%d; 0", i);
		else
			sprintf(data, "0");
	}
	else
	{
		*(&REL_DATA(frame) i0 + control -> off_rel) = atoi(data);
		if (*(&REL_DATA(frame) i0 + control -> off_rel)
			&& !FRAME_DATA(frame) Fmap_data)
		{
			FRAME_DATA(frame) Fmap_data= (MAP_DATA *)p_alloc(sizeof(MAP_DATA));
			FRAME_DATA(frame) Fmap_data -> frame_num = (int16)frame;
		}
		if ((ptr = strchr(data, ';')) != NULL)
		{
			ptr++;
			FRAME_DATA(frame) Fmap_data -> command_flags = atoi(ptr) & 0x03;
			ptr = strchr(ptr, ';');
			if (ptr)
			{
				ptr++;
				PREV_LAY(frame) = atol(ptr);
				ptr = strchr(ptr, ';') + 1;
				PREV_TEXT_ID(frame) = atol(ptr);
				ptr = strchr(ptr, ';') + 1;
				NEXT_LAY(frame) = atol(ptr);
				ptr = strchr(ptr, ';') + 1;
				NEXT_TEXT_ID(frame) = atol(ptr);
			}
		}
	}
	return(0);
}

int MFPolyDots(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	int i, num_pts;
	char str[20];
	DRAW_POINT_X_Y *pts;

	if (to_ac)
	{
		if (control -> offset == SHAPE)
		{
			pts = FRAME_DATA(frame) shape_pts;
			num_pts = FRAME_DATA(frame) num_shapes;
		}
		else
		{
			pts = FRAME_DATA(frame) bearoff_pts;
			num_pts = FRAME_DATA(frame) num_bearoffs;
		}
		for (i = 0; i < num_pts; i++)
		{
			if (i && !(i % 4))
				strcat(data, " \\\n\t");
			else
				strcat(data, "; ");
			sprintf(str, "%ld, %ld", pts[i].x, pts[i].y);
			strcat(data, str);
		}
	}
	return(0);
}

int MFGlobalData(WYSIWYG *wn, CNTRLS *control, int frame, char *data,int to_ac)
{
	switch (control -> off_rel)
	{
	  case 0:					/* Display options */
	  case 7:					/* Snap to vertical */
		if (to_ac)
			*(uint16 *)data = *(&wn -> global_data.display_options 
								+ control -> off_rel);
		else
			*(&wn -> global_data.display_options
			  + control -> off_rel) = *(uint16 *)data;
		break;
	  case 1:					/* Text display */
	  case 2:					/* Graphic display */
	  case 3:					/* Zoom option */
	  case 5:					/* Snap from */
	  case 6:					/* Snap to horizontal */
	  case 8:					/* Horizontal ruler origin */
	  case 9:					/* Vertical ruler origin */
	  case 10:					/* Horizontal ruler units */
	  case 11:					/* Vertical ruler units */
		if (to_ac)
			*data = (char)(*(&wn -> global_data.display_options
							 + control -> off_rel));
		else
			*(&wn -> global_data.display_options
			  + control -> off_rel) = (uint16)(*data);
		break;
	  case 4:					/* Zoom value */
			/* Somewhere, DM gives the zoom huge garbage values. So
				restrict them here, at I/O from .lay, to 5 < zm < 300.
				Assume other is garbage, change to 75.  */
		if (to_ac)
		{
			uint16 zm = *(&wn -> global_data.display_options
								  + control -> off_rel);
			if ((zm < 5) || (zm > 300)) zm = 75;
			sprintf(data, "%u", zm);
		}
		else
		{
			uint16 zm = (uint16)atoi(data);
			if ((zm < 5) || (zm > 300)) zm = 75;
			*(&wn -> global_data.display_options
			  + control -> off_rel) = zm;
		}
		break;
	  case 12:					/* Default overlay value */
		if (to_ac)
		{
			if (frame)
				*data = (FRAME_FLAGS(frame) >> 2) & 0x0001;
			else
				*data = (DEF_FLAGS >> 2) & 0x0001;
		}
		else
		{
			if (*data)
			{
				if (frame)
					FRAME_FLAGS(frame) |= OVERLAY;
				else
					DEF_FLAGS |= OVERLAY;
			}
			else
			{
				if (frame)
					FRAME_FLAGS(frame) &= ~OVERLAY;
				else
					DEF_FLAGS &= ~OVERLAY;
			}
		}
		break;
	  default:
		p_info(PI_ELOG, "Illegal value: %d in MFGlobalData\n",
				  control -> off_rel);
		break;
	}
	return(0);
}

void make_absolute(WYSIWYG *wn, CNTRLS *control, int frame, char *data,
				   BOOLEAN force_absolute)
{
	int32 machine, width, units, depth;
	int32 anchor_x, anchor_y;
	uint32 *ptr32;

	ptr32 = &REL_DATA(frame) d0 + control -> off_rel;
	switch (control -> off_rel)
	{
	  case 2:
		if (force_absolute)
			REL_FLAGS(frame) &= ~ABS_HORIZONTAL;
		if (!(REL_FLAGS(frame) & ABS_HORIZONTAL))
		{
			lmt_compute_param_x(wn, frame, &machine, &width);
			machine -= lmt_off_to_abs(wn, X_REF, X_PG_ORIGIN);
			units = *ptr32 & PL_REL_BITS;
			if (!units)
				units = PICA<<PL_REL_SHIFT;	/* Picas */
			*ptr32 = lmt_abs_to_off(wn, X_REF, units, machine);
			X_REL_1(frame) = 0x1100;
			X_REL_2(frame) = 0;
			if (data)
				MFHorizontal(wn, control, frame, data, 1);
		}
		break;
	  case 3:
		if (force_absolute)
			REL_FLAGS(frame) &= ~ABS_VERTICAL;
		if (!(REL_FLAGS(frame) & ABS_VERTICAL))
		{
			lmt_compute_param_y(wn, frame, &machine, &depth);
			machine -= lmt_off_to_abs(wn, Y_REF, Y_PG_ORIGIN);
			units = *ptr32 & PL_REL_BITS;
			if (!units)
				units = PICA<<PL_REL_SHIFT;	/* Picas */
			*ptr32 = lmt_abs_to_off(wn, Y_REF, units, machine);
			Y_REL_1(frame) = 0x1100;
			Y_REL_2(frame) = 0;
			if (data)
				MFVertical(wn, control, frame, data, 1);
		}
		break;
	  case 4:
		if (force_absolute)
			REL_FLAGS(frame) &= ~ABS_WIDTH;
		if (!(REL_FLAGS(frame) & ABS_WIDTH) && W_REL_1(frame))
		{						/* There is a relationship */
			lmt_compute_param_x(wn, frame, &anchor_x, &machine);
			units = *ptr32 & PL_REL_BITS;
			if (!units)
				units = PICA<<PL_REL_SHIFT;	/* Picas */
			*ptr32 = lmt_abs_to_off(wn, X_REF, units, machine);
			W_REL_1(frame) = W_REL_2(frame) = 0;
			if (data)
				MFWidth(wn, control, frame, data, 1);
		}
		break;
	  case 5:
		if (force_absolute)
			REL_FLAGS(frame) &= ~ABS_DEPTH;
		if (!(REL_FLAGS(frame) & ABS_DEPTH) && D_REL_1(frame))
		{						/* There is a relationship */
			lmt_compute_param_y(wn, frame, &anchor_y, &machine);
			units = *ptr32 & PL_REL_BITS;
			if (!units)
				units = PICA<<PL_REL_SHIFT;	/* Picas */
			*ptr32 = lmt_abs_to_off(wn, Y_REF, units, machine);
			D_REL_1(frame) = D_REL_2(frame) = 0;
			if (data)
				MFDepth(wn, control, frame, data, 1);
		}
		break;
	  default:
		p_info(PI_ELOG, "Illegal value: %d in make_absolute\n",  control -> off_rel);
		break;
	}
}

int MFMpLong(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	if (to_ac)
		sprintf(data, "%ld", *(&REL_DATA(frame) t0 + control -> off_rel));
	else
		*(&REL_DATA(frame) t0 + control -> off_rel) = atoi(data);
	return(0);
}

int MFObjRef(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	char *ptr;

	if (to_ac)
	{
		sprintf(data, "%ld; %ld",
				(*(&REL_DATA(frame) d0 + control-> off_rel) >> 12) & MP_OB_LAY,
				*(&REL_DATA(frame) d0 + control -> off_rel) & MP_OB_FRAME);
	}
	else
	{
		*(&REL_DATA(frame) d0 + control -> off_rel) =
			(atol(data) & MP_OB_LAY) << 12; /* high 20 bits */
		if ((ptr = strchr(data, ';')) != NULL)
		{
			ptr++;
			*(&REL_DATA(frame) d0 + control -> off_rel) |=
				(atoi(ptr) & MP_OB_FRAME); /* low 12 bits */
		}
	}
	return(0);
}

int MFFrameUID(WYSIWYG *wn, CNTRLS *control, int frame, char *data, int to_ac)
{
	if (to_ac)
		sprintf(data, "%ld", *(&FRAME_DATA(frame) frame_uid ));
	else
		*(&FRAME_DATA(frame) frame_uid) = atol(data);
	return(0);
}

int MFFrameMaxUID(WYSIWYG *wn, CNTRLS *control, int frame, char *data, 
				  int to_ac)
{
	if (to_ac)
		sprintf(data, "%ld", *(&FRAME_DATA(frame) max_uid ));
	else
		*(&FRAME_DATA(frame) max_uid) = atol(data);
	return(0);
}
