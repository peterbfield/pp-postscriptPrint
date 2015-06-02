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
#include <string.h>
#include <math.h>
#include "p_lib.h"
#include "menu_frame.h"
#include "interfer.h"
#include "frame.h"

extern char *itoa(int, char *);
extern char lmt_qualifier_options[];
extern uint16 lmt_unit_factors[];
extern int16 lmt_syntax_mask_table[];

void lmt_syntax_to_ascii(REL_WORDS *syntax, char list[], char buffer[])
{
	int16 i, j, m, n;
	float k,fo;
	char sign, qual ='\0';
	char *ptr,*ptr2;
	char buf[20];
	char *buf_ptr = buf;

	syntax -> block_numbers[0] = (syntax -> relationships[0] & 0377);
	syntax -> relationships[0] = (syntax -> relationships[0] >> 8);
	syntax -> block_numbers[1] = (syntax -> relationships[1] & 0377);
	syntax -> relationships[1] = (syntax -> relationships[1] >> 8);
	syntax -> qualifier = (int16)(syntax -> offset >> PL_REL_SHIFT);
	(syntax -> qualifier & 020) ? (sign = '-') : (sign = '+');
	syntax -> qualifier &= 017;
	syntax -> offset &= PL_SYNBITS;
	i = j = n = 0;
	k = fo = (float)0L;
	if (syntax -> offset)
	{
		switch(syntax -> qualifier)	/*get divisor */
		{
		  case PICA:
			i = 12;
			break;
		  case INCH_32:
			i = 32;
			break;
		  case CENTIMETER:
			i = 100;
			break;
		  case POINT_TENTH:
			i = 10;
			break;
		  case POINT_HALF:
			i = 4;
			break;
		  case POINT_QUARTER:
			i = 2;
			break;
		  case LINE_LEAD:
		  case ORG_LINE_LEAD:
			i = 1;
			break;
		}
		if (i)
		{
			fo = (float)syntax -> offset
				/ (float)(long)lmt_unit_factors[syntax -> qualifier];
			j = (int16)(long)(fo / (float)(long)i);
			if (syntax -> qualifier == CENTIMETER)
				k = floor(fo - (float)(long)(j * i)) / 10.;	/* Drop fraction */
			else
				k = fo - (float)(long)(j * i);
			qual = lmt_qualifier_options[syntax -> qualifier];
		}
		else
		{
			k = fo = 0.;
			j = 0;
		}
	}
	if (!(syntax -> relationships[0] == 0
		  && syntax -> relationships[1] == 0) && list) /* if rels. */
	{
		for(i = 0; i <= 1; i++)
		{
			for(m = 1; m < 17; m++)
				if(lmt_syntax_mask_table[m] == syntax -> relationships[i])
					break;
			m *= 3;				/*pts to relational char string in list*/
			buffer[n++] = list[m++];
			buffer[n++] = list[m];
			if (syntax -> block_numbers[i] == 255) /* SAT */
				buffer[n++] = 'T';
			else if (syntax -> block_numbers[i] == 254)	/* SAP */
				buffer[n++] = 'P';
			else if (syntax -> block_numbers[i])
			{
				itoa(syntax -> block_numbers[i], &buffer[n]);
				n = strlen(buffer);
			}
			if (!syntax -> relationships[1])
				i++;
		}
	}
	if (j || k)
	{
		if (n)
			buffer[n++] = sign;
		else if (sign == '-')
			buffer[n++] = sign;
	}
	if (j)
		itoa(j, &buffer[n]);
	n = strlen(buffer);
	if (j || k)
		buffer[n++] = qual;
	if (k)
	{
		sprintf(buf_ptr, "%.3f", k);
		ptr = buf;
		while(*ptr == '0')
			ptr++;
		ptr2 = buf + strlen(buf) - 1;
		while(*ptr2 == '0')
		{
			*ptr2 = 0;
			ptr2--;
		}
		if(*ptr2 == '.')
			*ptr2 = 0;
		strcpy(&buffer[n], ptr);
	}
	if(!n)
		strcpy(buffer, "0");
}

void lmt_syntax_to_ascii_pts(REL_WORDS *syntax, char buffer[])
{
	int16 j, n;
	float k;
	char sign;
	char *ptr,*ptr2;
	char buf[20];
	char *buf_ptr = buf;

	syntax -> qualifier = (int16)(syntax -> offset >> PL_REL_SHIFT);
	(syntax -> qualifier & 020) ? (sign = '-') : (sign = '+');
	syntax -> qualifier &= 017;
	syntax -> offset &= PL_SYNBITS;
	k = 0.;
	j = n = 0;
	if (syntax -> offset)
	{
		k = (float)(long)syntax -> offset
			/ (float)(long)lmt_unit_factors[syntax -> qualifier];
		j = 0;
		n = 0;
	}
	if ((n && (j || k)) || (sign == '-'))
		buffer[n++] = sign;
	if (j)
		itoa(j, &buffer[n]);
	n = strlen(buffer);
	if(k)
	{
		sprintf(buf_ptr, "%.3f", k);
		ptr = buf;
		while(*ptr == '0')
			ptr++;
		ptr2 = buf + strlen(buf) - 1;
		while(*ptr2 == '0')
		{
			*ptr2 = 0;
			ptr2--;
		}
		if(*ptr2 == '.')
			*ptr2 = 0;
		strcpy(&buffer[n],ptr);
		n = strlen(buffer);
	}
	if(!n)
		itoa(n, buffer);
	/*buffer[strlen(buffer)] = 040;*/
}

static int isdec(int c)
{
	if( (c >= 48 && c <= 57) || (c == 46) )
		return(1);
	else
		return(0);
}

int16 lmt_parse_numeric(REL_WORDS *syntax, char input[], int m, char sign)
{
	int16 j;
	float fo,ff;

	fo = ff = (float)0L;
	p_make_upper(input);
	fo = (float)atof(&input[m]);
	while (isdec(input[m]))
		m++;					/*skip to non-decimal */
	for (j = 1; j < strlen(lmt_qualifier_options); j++)
		if (input[m] == lmt_qualifier_options[j])
		{
			syntax -> qualifier = j;
			break;
		}
	if (!syntax -> qualifier || syntax -> qualifier == POINT || (!input[m]))
	{
		if (input[m])
			return(1);
	}
	else
	{
		m++;
		ff = (float)atof(&input[m]);
	}
	switch(syntax -> qualifier)
								/*convert all units to base in syntax->offset*/
	{
	  case POINT:				/* default is points */
		ff = fo;
		fo = (float)0L;
		/*FALLTHROUGH*/
	  case 0:					/* default is picas */
		syntax -> qualifier = PICA;
		/*FALLTHROUGH*/
	  case PICA:
		fo = fo * 12 + ff;
		syntax -> offset =
			(uint32)(fo * (float)lmt_unit_factors[syntax -> qualifier] + .5);
		break;
	  case INCH_32:				/* inches and 32ths	*/
		fo = fo * 32 + ff;
		syntax -> offset =
			(uint32)(fo * (float)lmt_unit_factors[syntax -> qualifier] + .5);
		break;
	  case CENTIMETER:			/* centimeters and millimeters */
		fo = fo * 100 + (ff * 10);
		syntax -> offset =
			(uint32)(fo * (float)lmt_unit_factors[ syntax -> qualifier] + .5);
		break;
	  case LINE_LEAD:			/* multiple of line leading */
		syntax -> offset = (uint32)fo;
		break;
	  case POINT_HALF:			/* points and halves */
		fo = fo * 2 + ff;
		syntax -> offset =
			(uint32)(fo * (float)lmt_unit_factors[ syntax -> qualifier] + .5);
		break;
	  case POINT_QUARTER:		/* points and quarters */
		fo = fo * 4 + ff;
		syntax -> offset =
			(uint32)(fo * (float)lmt_unit_factors[ syntax -> qualifier] + .5);
		break;
	  case POINT_TENTH:			/* points and tenths */
		fo = fo * 10 + ff;
		syntax -> offset =
			(uint32)(fo * (float)lmt_unit_factors[ syntax -> qualifier] + .5);
		break;
	  case ORG_LINE_LEAD:		/* multiple of original spec line leading*/
		syntax -> offset = (uint32)fo;
		break;
	  default:
		break;
	}
	if(sign == '-')
		syntax -> qualifier |= 020;
	if (syntax -> offset)
		syntax -> offset |= (int32)syntax -> qualifier << PL_REL_SHIFT;
	/*include sign and units-type */
	return(0);
}

static int16 validate_syntax(REL_WORDS *syntax, char input[], char list[],
							 int frame, int tot_frame)
{
	int16 j, k, m, error;
	char *ptr, sign;

	memset((char*)syntax, 0, sizeof(REL_WORDS));
	p_make_upper(input);
	sign = '+';
	k = m = 0;
	error = 0;
	while (k <= 1 && !error)
	{
		for (j = 0; j <= strlen(list); j += 3)
			/*look for syntax option & possible block # */
			if (!strncmp(&list[j], &input[m], 2))
				break;			/*break if one has been found */
		if (j < strlen(list))
		{
			syntax -> block_numbers[k] = 0;
			if (!strchr(&input[2], 'P') && !strchr(&input[2], 'p'))
			{					/* skip the mnemonic */
				ptr = strchr(input, '.');
				if (ptr)
					*ptr = 'P';
			}
			syntax -> relationships[k] = j;	/* got one */
			m += 2;
			if (!strncmp(&input[m - 2], "SA", 2))
			{
				if (input[m] == 'T') /* refer to the trim */
				{
					syntax -> block_numbers[k] = -1;
					m++;
				}
				else if (input[m] == 'P') /* refer to the page */
				{
					syntax -> block_numbers[k] = -2;
					m++;
				}
				else if (isdec(input[m]))
					syntax -> block_numbers[k] = atoi(&input[m]);
				/*if any */
			}
			else
			{
				if (isdec(input[m]))
					syntax -> block_numbers[k] = atoi(&input[m]);
				/*if any */
			}
			if (syntax -> block_numbers[k] > tot_frame
				|| (syntax -> block_numbers[k] == frame && frame))
				error = 1;
			while (isdec(input[m]))
				m++;			/* skip to non-decimal */
			k++;				/* no more than 2 */
		}
		else
		{
			if (!strchr(input, 'P') && !strchr(input, 'p'))
			{
				ptr = strchr(input, '.');
				if (ptr)
					*ptr = 'P';
			}
			k = 2;				/* no syntax option found */
		}
	}
	if (m)						/* look for sign operator if
								   syntax option found */
	{
		switch(input[m])
		{
		  case '+':
		  case '-':
			sign = input[m];
			m++;
		}
	}
	if (!error)
		error = lmt_parse_numeric(syntax, input, m, sign);
	return(error);
}

int16 lmt_xy_syntax_parser(REL_WORDS *syntax, char input[], char list[],
						   int no_default, int frame, int tot_frame)
{
	int16 i, j, mask, error;

	error = validate_syntax(syntax, input, list, frame, tot_frame);
	/*get option-string offset */
	if (error)
		return(error);
	for(i = 0; i <= 1; i++)		/* now set up the relationships */
	{
		j = syntax -> relationships[i];
		/* offset from validate syntax */
		if (j)
			j /= 3;				/*index to mask table*/
		mask = lmt_syntax_mask_table[j];
		if ((!j && no_default) || (!j && i == 1))
			mask = 0;
		if (((mask & 0x0f) < 2) && syntax -> block_numbers[i])
			error = 1;
		else if ((mask & 0x0f) > 1 && !syntax -> block_numbers[i])
			error = 1;
		syntax -> relationships[i] =
			((mask << 8) | ((syntax -> block_numbers[i]) & 0xff));
		if (!j || error)
			i++;
	}
	return(error);
}

int16 lmt_size_parser(REL_WORDS *syntax, char input[], int dflt)
/* 0 = no letter means picas 1 = no letter means points */
{
	char *ptr, sign;
	int16 error;
	int m = 0;

	memset((char *)syntax, 0, sizeof(REL_WORDS));
	if (dflt)
	{
		syntax -> qualifier = POINT;
		if (!strchr(input, 'P') && !strchr(input, 'p'))
		{
			ptr = strchr(input, '.');
			if (ptr)
				if (strchr(ptr + 1, '.'))
					*ptr = 'P';
		}
	}
	else
	{
		syntax -> qualifier = PICA;
		if (!strchr(input, 'P') && !strchr(input, 'p'))
		{
			ptr = strchr(input, '.');
			if (ptr)
				*ptr = 'P';
		}
	}
	switch(input[m])
	{
	  case '+':
	  case '-':
		sign = input[m];
		m++;
		break;
	  default:
		sign = '+';
		break;
	}
	error = lmt_parse_numeric(syntax, &input[m], 0, sign);
	syntax -> relationships[0] = (021 << 8);
	/*default is rel. to left of page origin */
	return(error);
}
