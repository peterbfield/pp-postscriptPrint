#include <stdio.h>
#include <stdlib.h>
#include "p_lib.h"
#include "image.h"
#include "px_header.h"
#include "window.h"

#define MIN_CODE 9						/* Min code length */
#define MAX_CODE 12						/* Max code length */
#define CLEAR 256
#define EOD   257
#define BUF_MULT 1
#define MAX_TABLE 4094
#define MAX_STR  2048

QUEUE raw_graphics;						/* head/tail pointers to graphic Q */

typedef struct codetbl
{
	unsigned char *str;
	int len;
	struct codetbl *next;
}	CODE_TABLE, *CODE_TABLE_PTR;

CODE_TABLE_PTR sorted_len[MAX_STR];

extern int p_fseek (Pfd fd_id, long offset);
extern int cr_shapes_fr_pix (int *px2_sh_recs, PXL_SHAPE **px2_shapes, char *px, char *data);

int load_graphic (char *tree_name, char *graphic_name, char *dir_name);
char *load_graphic_file (char *tree_name, char *graphic_name, int *file_size, char *dir_name);
char *decode_file (Pfd file, int size, int *res_len);
int get_shapes (Pfd gr, int *shape_size, PXL_SHAPE **shapes, char *datafile);
char *load_pixel_file ();
int add_shapes;

char *load_graphic_file (char *tree_name, char *graphic_name, int *file_size, char *dir_name)
{
	Pfd gr;
	char *datafile = 0;
	struct stat statbuf;

	if (p_stat (tree_name, GRAPHICS, dir_name, graphic_name, &statbuf))
	{
		p_info (PI_ELOG, "No .pixel for the graphic %s.\n", graphic_name);
		return (0);
	}
	gr = p_open (tree_name, GRAPHICS, dir_name, graphic_name, "r");
	add_shapes = 0;
	datafile = load_pixel_file (gr, file_size);
	p_close (gr);
	if (datafile == 0)
	{
		p_info (PI_ELOG, "Graphic %s not properly defined\n", graphic_name);
		return (0);
	}
	else if (add_shapes)
	{
/*
 * Add shapes data to the pixel file
 */
		int iii, num, len = 0;
		char line[MAX_SHAPE_STR];
		PXL_SHAPE *shapes = (PXL_SHAPE *) (datafile + ((PXL_HDR *) datafile)->first_shape_rec * 512);

		gr = p_open (tree_name, GRAPHICS, dir_name, graphic_name, "a");
		num = shapes->num_pts;
		sprintf (line, "(%d shapes)\n", num);
		p_fputs (line, gr);
		for (iii = 0; iii < num; iii++)
		{
			sprintf (line + len, "(%d,%d)", shapes->pts[iii].x, shapes->pts[iii].y);
			len = strlen (line);
			if ((iii + 1) % SHAPES_LINE == 0)
			{
				line[len++] = '\n';
				line[len] = '\0';
				p_fputs (line, gr);
				len = 0;
			}
		}
		if (len)
		{
			line[len++] = '\n';
			line[len] = '\0';
			p_fputs (line, gr);
		}
		p_close (gr);
	}
	return (datafile);
}

char *load_pixel_file (Pfd gr, int *file_size)
{
	char *datafile = 0;
	char line[1024];
	int temp;
	int emax = 5;
	int color_mask = 0;
	int resolution = 0;
	int width = 0;
	int depth = 0;
	int trim_left = 0;
	int trim_top = 0;
	float llx = 0;
	float lly = 0;
	float urx = 0;
	float ury = 0;
	float crop_left = 0;
	float crop_top = 0;
	float crop_width = 0;
	float crop_depth = 0;
	int scalex = ZOOM_UNIT, scaley = ZOOM_UNIT;
	char *ptr;
	PXL_HDR *hdr;
	int level = 0;
	int levels[512];
	int index = 0;
	int format = 0;
	struct
	{
		int val;						/* 0-7 =  color,  -1 = push,  -2 = pop */
		int len;						/*        count        count       push index */
	}
	stack[2048];

	for (; !p_feof (gr);)
	{
		if (!p_fgets (line, 128, gr))
			continue;
		if (strncmp (line, "resolution", 10) == 0)
			sscanf (line + 10, "%d", &resolution);
		else if (strncmp (line, "size", 4) == 0)
			sscanf (line + 4, "%d %d", &width, &depth);
		else if (strncmp (line, "trim", 4) == 0)
			sscanf (line + 4, "%d %d", &trim_left, &trim_top);
		else if (strncmp (line, "mask", 4) == 0)
			sscanf (line + 4, "%d", &color_mask);
		else if (strncmp (line, "bbox", 4) == 0)
			sscanf (line + 4, "%f %f %f %f", &llx, &lly, &urx, &ury);
		else if (strncmp (line, "crop_lt", 7) == 0)
			sscanf (line + 7, "%f %f", &crop_left, &crop_top);
		else if (strncmp (line, "crop_wd", 7) == 0)
			sscanf (line + 7, "%f %f", &crop_width, &crop_depth);
		else if (strncmp (line, "scale", 5) == 0)
		{
			temp = sscanf (line + 5, "%d %d", &scalex, &scaley);
			if (temp == 1)
				scaley = scalex = scalex * 10;
		}
		else if (strncmp (line, "file_format", 11) == 0)
			sscanf (line + 11, "%d", &format);
		else if (strncmp (line, "ps_name", 7) == 0)
			continue;					/* Do nothing. ps_name is a valid entry */
		else if (strncmp (line, "ps_path", 7) == 0)
			continue;					/* Do nothing. ps_path is a valid entry */
		else if (strncmp (line, "ps_system", 9) == 0)
			continue;					/* Do nothing. ps_system is a valid entry */
		else if (strncmp (line, "ps_type", 7) == 0)
			continue;					/* Do nothing. ps_type is a valid entry */
		else if (strncmp (line, "crop_flg", 8) == 0)
			continue;					/* Do nothing. crop_flg is a valid entry */
		else if (strncmp (line, "colors", 6) == 0)
			break;
		else
		{
			p_info (PI_ELOG, "unknown graphic file entry: %s\n", line);
			if (--emax <= 0)
				return (0);
		}
	}
	crop_width = (crop_width * scalex) / ZOOM_UNIT;
	crop_depth = (crop_depth * scaley) / ZOOM_UNIT;
	if (color_mask == 0 || resolution == 0 || width == 0 || depth == 0)
		return (0);
	temp = (width + 1) / 2;				/* bytes per line */
	temp *= depth;
	temp += 511;
	temp /= 512;
	temp *= 512;
	temp += 512;
	/* add header record */
	datafile = p_alloc (temp);
	*file_size = temp;
	hdr = (PXL_HDR *) datafile;
	hdr->horiz_dpi = resolution;
	hdr->vert_dpi = resolution;
	hdr->dpl = width;
	hdr->lpg = depth;
	hdr->trim_top = trim_top;
	hdr->trim_left = trim_left;
	hdr->file_format = format;
	if (ury == 0 && lly == 0 && llx == 0 && urx == 0)	/* no BoundingBox */
	{
		crop_left = ((float)trim_left * 72) / resolution;
		crop_top = ((float)trim_top * 72) / resolution;
		crop_width = ((float)width * 72) / resolution;
		crop_depth = ((float)depth * 72) / resolution;
	}
	hdr->bbox_top = (int32) (ury * 20);
	hdr->bbox_bottom = (int32) (lly * 20);
	hdr->bbox_left = (int32) (llx * 20);
	hdr->bbox_right = (int32) (urx * 20);
	/* Round to the nearest value */
	hdr->crop_top = (int32) (crop_top * 20. + .5);
	hdr->crop_left = (int32) (crop_left * 20. + .5);
	hdr->crop_width = (int32) (crop_width * 20. + .5);
	hdr->crop_depth = (int32) (crop_depth * 20. + .5);
	hdr->source_type = 5;
	hdr->line_alignment = 1;			/* byte alignment */
	hdr->color_mask = color_mask;		/* colors in use */
	hdr->bits_per_pixel = 4;			/* four bits per pixel unless format = 1 */
	hdr->px_structure_revision = -2;
	hdr->scale = scalex;
	hdr->scaley = scaley;
	if (format == 1)
	{
		int shape_size;
		PXL_SHAPE *shapes;
		char *new;
		char *decode_str;
		int k, res_len;

		/* File is in new format */
		if (color_mask == 64)			/* Black and white */
		{
			hdr->bits_per_pixel = 1;
		}
		else
		{
			hdr->bits_per_pixel = BYTE;	/* BYTE because format = 1 */
		}
		decode_str = decode_file (gr, width * depth, &res_len);
		k = 'a';
		while (k != EOF && k != '(')
			k = p_getc (gr);
		if (k == EOF)
		{
			if (!cr_shapes_fr_pix (&shape_size, &shapes, datafile, decode_str))
				return (0);
			/* Add shapes info to the file */
			/* add_shapes = 1; */
		}
		else
			/* k == '('  */
		{
			if (!get_shapes (gr, &shape_size, &shapes, datafile))
				return (0);
		}
		*file_size = (res_len + 511) / 512 * 512 + 512;		/* Make it divisible by 512 */
		hdr->first_shape_rec = *file_size / 512;
		new = p_alloc (*file_size + shape_size);
		memcpy (new, datafile, 512);
		memcpy ((new + 512), (char *)decode_str, res_len);
		if (shape_size)
			memcpy ((new + *file_size), (char *)shapes, shape_size);
		*file_size += shape_size;
		p_free (datafile);
		datafile = new;
		p_free (decode_str);
		p_free ((char *)shapes);
		return (datafile);
	}
	for (ptr = datafile + 512;; ptr += ((width + 1) / 2))	/* each line */
	{
		int len, pos, k, c;

		pos = 0;
		index = 0;
		level = 0;
		for (;;)						/* scan each line */
		{
			k = p_getc (gr);			/* color */
			if (k == '\n' || k == EOF)
				break;					/* next line */
			if (k == '{')				/* push */
			{
				index++;
				stack[index].val = -1;
				level++;
				levels[level] = index;
				continue;
			}
			else if (k == '}')			/* pop */
				k = -2;
			else if (k < '0' || k > '7')
			{
				if (pos == 0 && k == 'e')
				{
					k = EOF;
					break;
				}
				if (k == '(')
					break;
				p_info (PI_ELOG, "graphic construction error\n");
				p_free (datafile);
				return (0);
			}
			else
				k -= '0';
			len = 0;
			for (;;)					/* each line segment */
			{
				c = p_getc (gr);
				if (c >= 'a' && c <= 'z')
				{
					len *= 26;
					len += c - 'a';
					continue;
				}
				else if (c >= 'A' && c <= 'Z')
				{
					len *= 26;
					len += c - 'A';
					break;
				}
				else
				{
					p_info (PI_ELOG, "graphic construction error\n");
					p_free (datafile);
					return (0);
				}
			}
			if (k == -2)				/* stack pop */
			{
				if (levels[level] >= 2048 || levels[level] < 0)
					continue;
				else
				{
					stack[levels[level]].len = len + 1;		/* finish push */
					index++;
					stack[index].val = -2;
					stack[index].len = levels[level];
					level--;
				}
			}
			else
			{
				index++;
				stack[index].val = k;
				stack[index].len = len + 1;
				len++;
			}
			if (level != 0)
				continue;
			for (c = 1; c <= index; c++)
			{
				k = stack[c].val;
				len = stack[c].len;
				if (k == -1)			/* push */
				{
					level++;
					levels[level] = len;
					continue;
				}
				if (k == -2)			/* pop */
				{
					levels[level]--;
					if (levels[level] > 0)
						c = len;
					else
						level--;
					continue;
				}

				if (k)
				{
					while (len--)
					{
						if (pos & 1)
							*(ptr + (pos / 2)) += k;
						else
							*(ptr + (pos / 2)) = k << 4;
						pos++;
					}
				}
				else
					pos += len;
			}
			index = 0;
			level = 0;
		}
		if (k == EOF)
			break;
		if (k == '(')					/* prepare for shape data */
		{
			int shape_size;
			PXL_SHAPE *shapes;
			char *new;

			if (!get_shapes (gr, &shape_size, &shapes, datafile))
				return (0);
			hdr->first_shape_rec = *file_size / 512;
			new = p_alloc (*file_size + shape_size);
			memcpy (new, datafile, *file_size);
			memcpy ((new + *file_size), (char *)shapes, shape_size);
			p_free (datafile);
			datafile = new;
			p_free ((char *)shapes);
			return (datafile);
		}
	}
	return (datafile);
}

int get_shapes (Pfd gr, int *shape_size, PXL_SHAPE **shapes, char *datafile)
{
	int index;
	int k;
	int num_shapes = 0;
	int num_recs = 0;
	int val = 0;
	int sign = 0;
	char line[1024];

	p_fgets (line, 128, gr);
	sscanf (line, "%d", &num_shapes);
	if (num_shapes == 0)
	{
		p_info (PI_ELOG, "graphic construction error\n");
		p_free (datafile);
		return (0);
	}
	/* how much memory? */
	num_recs = ((sizeof (PXL_SHAPE) - sizeof (((PXL_SHAPE *) * shapes)->pts)) +
				(sizeof (SHAPE_X_Y) * num_shapes)) / 512 + 1;
	*shape_size = num_recs * 512;
	*shapes = (PXL_SHAPE *) p_alloc (*shape_size);
	((PXL_SHAPE *) * shapes)->shape = 1;
	((PXL_SHAPE *) * shapes)->of_sh = 1;
	((PXL_SHAPE *) * shapes)->num_pts = num_shapes;
	((PXL_SHAPE *) * shapes)->num_recs = num_recs;
	index = 0;
	for (;;)
	{
		k = p_getc (gr);				/* color */
		if (k == '\n')
			continue;
		else if (k == EOF)
			break;						/* next line */
		else if (k == '(')
		{
			sign = 1;
			val = 0;
		}
		else if (k == ',')
		{
			((PXL_SHAPE *) * shapes)->pts[index].x = val * sign;
			sign = 1;
			val = 0;
		}
		else if (k == ')')
		{
			((PXL_SHAPE *) * shapes)->pts[index].y = val * sign;
			sign = 1;
			val = 0;
			index++;
		}
		else if (k == '-')
			sign = -1;
		else if (k >= '0' || k <= '9')
		{
			val *= 10;
			val += k - '0';
		}
		else
		{
			p_info (PI_ELOG, "graphic construction error\n");
			p_free (datafile);
			p_free ((char *)(*shapes));
			return (0);
		}
	}
	return (1);
}

static unsigned char trans_colors[BYTE] = { 0x00, 0x0f, 0x33, 0x03, 0x3c, 0x0c, 0x30, 0x3f };
int find_length (int code);
void add_table (unsigned char *str, int len);
void table_initialize ();
int code_from_table (unsigned char *str, int len);
void add_sorted (int ind, int len);

CODE_TABLE table[MAX_TABLE + 1];
int inp_bytes;
int cur_table_index;
short bits_left;
int buf_ind;
unsigned char buf[BUF_SIZE];
int int_size = sizeof (int) * BYTE;
int cur_code_len;

int code_from_table (unsigned char *str, int len)
{
	CODE_TABLE_PTR tel;
	int prom = len - 1;

	for (tel = sorted_len[len]; tel; tel = tel->next)
		if (tel->str[0] == str[0] && tel->str[prom] == str[prom] && !memcmp (str, tel->str, len))
			return (tel - table);
	return (-1);
}

void clean_table ()
{
	int i;

	for (i = 0; i <= cur_table_index; i++)
		p_free ((char *)table[i].str);
	for (i = 0; i < MAX_STR; i++)
		sorted_len[i] = NULL;
}

void add_sorted (int ind, int len)
{
	table[ind].next = sorted_len[len];
	sorted_len[len] = &(table[ind]);
}

void table_initialize ()
{
	int i;

	if (cur_table_index != 0)
	{
		for (i = EOD + 1; i <= cur_table_index; i++)
			p_free ((char *)table[i].str);
	}
	else
	{
		sorted_len[1] = NULL;
		sorted_len[0] = NULL;
		for (i = 0; i <= 255; i++)
		{
			table[i].len = 1;
			/* 
			 * We do not need one more space for NULL terminator because
			 * it can be a valid symbol in the middle of string
			 */
			table[i].str = p_alloc (sizeof (unsigned char));

			table[i].str[0] = i;
			add_sorted (i, 1);
		}
		/* 
		 * We allocate memory for EOD and CLEAR strings 
		 * in order to speed cleaning later. We do not use the contents
		 */
		table[CLEAR].len = 0;			/* Clear */
		table[CLEAR].str = p_alloc (sizeof (unsigned char));

		add_sorted (CLEAR, 0);
		table[EOD].len = 0;				/* End of Data */
		table[EOD].str = p_alloc (sizeof (unsigned char));

		add_sorted (EOD, 0);
	}
	cur_table_index = EOD;
	cur_code_len = MIN_CODE;
	for (i = 2; i < MAX_STR; i++)
		sorted_len[i] = NULL;
}

void add_table_first (int c1, int c2)
{
	int len;

	cur_table_index++;
	table[cur_table_index].len = len = table[c1].len + 1;
	table[cur_table_index].str = p_alloc (sizeof (unsigned char) * len);

	memcpy (table[cur_table_index].str, table[c1].str, len - 1);
	table[cur_table_index].str[len - 1] = table[c2].str[0];
	add_sorted (cur_table_index, len);
}

void add_table (unsigned char *str, int len)
{
	cur_table_index++;
	table[cur_table_index].len = len;
	table[cur_table_index].str = p_alloc (sizeof (unsigned char) * len);

	memcpy (table[cur_table_index].str, str, len);
	add_sorted (cur_table_index, len);
}

int find_length (int code)
{
	int len = 0;

	if (code < 0)
		len = 0;
	else if (code <= 510)
		len = MIN_CODE;
	else if (code > 510 && code <= 1022)
		len = 10;
	else if (code > 1022 && code <= 2046)
		len = 11;
	else if (code > 2046)
		len = MAX_CODE;
	return (len);
}

/* Reads a code from data */
int next_code (Pfd file)
{
	int i, shift, counter;
	int code = 0;

	/* Counter keeps the number of bits we still should take */
	counter = cur_code_len;
	for (i = 1, shift = (1L << (bits_left - i)); i <= bits_left; i++, shift >>= 1)
	{
		counter--;
		if (buf[buf_ind] & (shift))
			code |= (1L << (counter));
	}
	buf_ind++;
	if (counter > BYTE)
	{
		for (i = 1, shift = (1L << (BYTE - i)); i <= BYTE; i++, shift >>= 1)
		{
			counter--;
			if (buf[buf_ind] & (shift))
				code |= (1L << (counter));
		}
		buf_ind++;
	}
	bits_left = counter;
	for (i = 1, shift = (1L << (BYTE - i)); i <= bits_left; i++, shift >>= 1)
	{
		counter--;
		if (buf[buf_ind] & (shift))
			code |= (1L << (counter));
	}
	/* Bits left After we read the code */
	bits_left = BYTE - bits_left;
	if (bits_left == 0)
	{
		buf_ind++;
		bits_left = BYTE;
	}
	if (inp_bytes > 0 && buf_ind > inp_bytes - 6)
	{
		/* If the buffer is almost full, copy the last bytes and read from file */
		int diff = inp_bytes - buf_ind;

		for (i = 0; i < diff; i++)
			buf[i] = buf[buf_ind + i];
		inp_bytes = p_nread (file, buf + diff, BUF_SIZE - diff);
		inp_bytes += (diff);
		buf_ind = 0;
	}
	return code;
}

void put_code_to_buf (int code)
{
	unsigned char lower, upper1, upper2;
	int ind;
	int code_len = cur_code_len;

	/* splitting the code to put it in */
	/* One code can change at most 3 bytes in the buffer */
	lower = code >> (code_len - bits_left);
	buf[buf_ind] |= lower;
	buf_ind++;
	/* Now bits left keeps the number of bits used already */
	ind = (int_size + bits_left - code_len);
	if (code_len - bits_left > BYTE)
	{
		upper1 = (code << ind) >> (int_size - BYTE);	/* Should start at the last byte */
		buf[buf_ind] |= upper1;
		buf_ind++;
		ind += BYTE;
		upper2 = (code << ind) >> (int_size - BYTE);	/* Should start at the last byte */
		buf[buf_ind] |= upper2;
		bits_left = BYTE - (code_len - bits_left - BYTE);
	}
	else
	{
		upper1 = (code << ind) >> (int_size - BYTE);	/* Should start at the last byte */
		buf[buf_ind] |= upper1;
		bits_left = BYTE - (code_len - bits_left);
		if (bits_left == 0)
		{
			buf_ind++;
			bits_left = BYTE;
		}
	}
}

void encode_data (WYSIWYG * wn, Pfd file, GRAPHIC_IMAGE * image)
{
	unsigned char str[MAX_STR], *data;
	unsigned char test[MAX_STR];
	int code = 0, oldcode, str_index = -1;	/* Last used symbol index in str */
	unsigned char byte = 0, oldbyte;
	int mode, bit_order, send_flag;
	int i, j, k, buf_limit = BUF_SIZE - 6;
	int SourceBit, len, jj, limit;
	RAW *raw;
	char *in_name = (char *)((FILE_LIST *) wn->files.head)->filename;

	cur_table_index = 0;
	table_initialize ();
	for (j = 0; j < BUF_SIZE; j++)
		buf[j] = 0;
	buf_ind = 0;
	bits_left = BYTE;

	for (raw = (RAW *) raw_graphics.head; raw; raw = (RAW *) raw->que.next)
		if (strcmp (raw->px_file_name, in_name) == 0)
			break;
	if (!raw)
	{
		if (load_graphic (wn->tree_name, in_name, wn->dir_name))
		{
			p_info (PI_ELOG, "Could not find raw graphic\n");
			return;
		}
		else
			raw = (RAW *) raw_graphics.head;
	}
	data = (unsigned char *)(raw->datafile + 512);
	if (image->color_mask == 64)
	{
		/* The image is black and white -> we can save space using one byte for 8 pixels */
		mode = 0;						/* Simple mode for b/w images only */
	}
	else
	{
		/* Get bits from the source data */
		mode = 1;
	}
	/* Raw data has byte alignment */
	len = (raw->dpl + 1) / 2;			/* Bytes per line */
	limit = len * 2;
	put_code_to_buf (CLEAR);
	send_flag = 1;
	for (i = 0; i < raw->lpg; i++)		/* For each line */
	{
		SourceBit = 0xf0;
		bit_order = 0x80;
		for (jj = 0; jj < limit; jj++)
		{
			/* Converting 3bit color to 6 bit color */
			if (mode == 0 && send_flag)
			{
				send_flag = 0;
				byte = 0;
			}
			oldbyte = data[jj / 2];		/* In the old representation; pixel - 4 bits */
			oldbyte = oldbyte & SourceBit;
			if (jj % 2 == 0)
				oldbyte >>= 4;
			SourceBit >>= 4;
			if (!SourceBit)
				SourceBit = 0xf0;
			/* Transforms the colors to new format (2 bit for each of rgb components) */
			if (mode == 0)
			{
				if (oldbyte)
					byte |= bit_order;
				bit_order >>= 1;
				if (!bit_order)
				{
					bit_order = 0x80;
					send_flag = 1;
				}
				if (jj == limit - 1)
				{
					send_flag = 1;
				}
			}
			else
				byte = trans_colors[oldbyte];
			if (!send_flag)
				continue;
			/* Actual encoding */
			k = str_index + 1;
			for (j = 0; j < k; j++)
				test[j] = str[j];
			test[k] = byte;
			oldcode = code;
			if ((code = code_from_table (test, k + 1)) >= 0)
			{
				str[k] = byte;
				str_index++;
			}
			else
			{
				put_code_to_buf (oldcode);
				/* 
				 * Write the buffer to the file if it is around to be full,
				 * make sure we will have room for EOD
				 */
				if (buf_ind >= buf_limit)
				{
					p_nwrite (file, buf, buf_ind);
					buf[0] = buf[buf_ind];
					for (j = 1; j < BUF_SIZE; j++)
						buf[j] = 0;
					buf_ind = 0;
				}
				add_table (test, k + 1);
				cur_code_len = find_length (cur_table_index);
				if (cur_table_index == MAX_TABLE)
				{
					put_code_to_buf (CLEAR);
					table_initialize ();
				}
				str_index = 0;
				str[str_index] = byte;
				code = byte;
			}
		}
		data += len;
	}

	/* Save the last str if something left there */
	if (str_index >= 0)
	{
		if ((code = code_from_table (str, str_index + 1)) >= 0)
			put_code_to_buf (code);
		else
			p_info (PI_ELOG, "Error in the algorithm \n");
	}
	/* Put the rest and eod */
	put_code_to_buf (EOD);
	p_nwrite (file, buf, buf_ind + 1);
	clean_table ();
}

char *decode_file (Pfd file, int size, int *res_len)
{
	int i, prom, out_ind, out_alloc = size;
	int code, oldcode = 0;
	unsigned char test[MAX_STR];
	char *out_str = p_alloc (out_alloc);

	buf_ind = out_ind = 0;
	cur_code_len = MIN_CODE;			/* the length of code */
	bits_left = BYTE;					/* Unprocessed bits in the input buffer */
	inp_bytes = p_nread (file, buf, BUF_SIZE);
	cur_table_index = 0;
	while ((buf_ind < inp_bytes) && (code = next_code (file)) != EOD)
	{
		if (code == CLEAR)
		{
			table_initialize ();
			code = next_code (file);
			if (code == EOD)
				break;
			if (out_ind + table[code].len >= out_alloc)
			{
				out_alloc += (BUF_SIZE * BUF_MULT);
				out_str = (char *)p_realloc (out_str, out_alloc);
			}
			for (i = 0; i < table[code].len; i++)
				out_str[out_ind++] = table[code].str[i];
			oldcode = code;
		}
		else
		{
			if (code <= cur_table_index)
			{
				if (out_ind + table[code].len >= out_alloc)
				{
					out_alloc += (BUF_SIZE * BUF_MULT);
					out_str = (char *)p_realloc (out_str, out_alloc);
				}
				for (i = 0; i < table[code].len; i++)
					out_str[out_ind++] = table[code].str[i];
				add_table_first (oldcode, code);
				cur_code_len = find_length (cur_table_index + 1);
				oldcode = code;
			}
			else
			{
				if (code > cur_table_index + 1)
					continue;			/* Mistake in file */
				prom = table[oldcode].len;
				for (i = 0; i < prom; i++)
					test[i] = table[oldcode].str[i];
				test[i] = table[oldcode].str[0];
				prom++;
				if (out_ind + prom >= out_alloc)
				{
					out_alloc += (BUF_SIZE * BUF_MULT);
					out_str = (char *)p_realloc (out_str, out_alloc);
				}
				for (i = 0; i < prom; i++)
					out_str[out_ind++] = test[i];
				add_table (test, prom);
				cur_code_len = find_length (cur_table_index + 1);
				oldcode = code;
			}
		}
	}
	p_fseek (file, -inp_bytes + buf_ind);
	clean_table ();
	*res_len = out_ind;
	return out_str;
}

void encode_raw_data (FILE * file, RAW * raw)
{
	unsigned char str[MAX_STR], *data;
	unsigned char test[MAX_STR];
	int code = 0, oldcode, str_index = -1;	/* Last used symbol index in str */
	unsigned char byte = 0, oldbyte;
	int send_flag, i, j, k, buf_limit = BUF_SIZE - 6;
	int bit_order, color_mask = ((PXL_HDR *) raw->datafile)->color_mask;
	int SourceBit, jj, limit, len;
	FILE *p_WrnFd = stdout;

	cur_table_index = 0;
	table_initialize ();
	for (j = 0; j < BUF_SIZE; j++)
		buf[j] = 0;
	buf_ind = 0;
	bits_left = BYTE;
	if (!raw)
	{
		p_info (PI_ELOG, "Could not find raw graphic\n");
		return;
	}
	/* Get bits from the source data */
	data = (unsigned char *)(raw->datafile + 512);
	put_code_to_buf (CLEAR);
	send_flag = 1;
	/* Raw data has byte alignment */
	len = (raw->dpl + 1) / 2;			/* Bytes per line */
	limit = len * 2;
	for (i = 0; i < raw->lpg; i++)		/* For each line */
	{
		SourceBit = 0xf0;
		bit_order = 0x80;
		/* To show that we are thinking */
		p_info (PI_INFO, ".");
		fflush (p_WrnFd);
		for (jj = 0; jj < limit; jj++)
		{
/* Converting 3bit color to 6 bit color */
			if (color_mask == 64 && send_flag)	/* We just sent a byte or two */
			{
				send_flag = 0;
				byte = 0;
			}
			oldbyte = data[jj / 2];		/* In old representation; pixel - 4 bits */
			oldbyte = oldbyte & SourceBit;
			if (jj % 2 == 0)
				oldbyte >>= 4;
			SourceBit >>= 4;
			if (!SourceBit)
				SourceBit = 0xf0;
/* Transforms the colors to new format (2 bit for each of rgb components) */
			if (color_mask == 64)
			{
				if (oldbyte)
					byte |= bit_order;
				bit_order >>= 1;
				if (!bit_order)
				{
					bit_order = 0x80;
					send_flag = 1;
				}
				if (jj == limit - 1)
					send_flag = 1;
			}
			else
				byte = trans_colors[oldbyte];
			if (!send_flag)				/* Do not process the byte */
				continue;
			/* Actual encoding */
			k = str_index + 1;
			for (j = 0; j < k; j++)
				test[j] = str[j];
			test[k] = byte;
			oldcode = code;
			if ((code = code_from_table (test, k + 1)) >= 0)
			{
				str[k] = byte;
				str_index++;
			}
			else
			{
				put_code_to_buf (oldcode);
				/* 
				 * Write the buffer to the file if it is around to be full,
				 * make sure we will have room for EOD
				 */
				if (buf_ind >= buf_limit)
				{
					fwrite (buf, 1, buf_ind, file);
					buf[0] = buf[buf_ind];
					for (j = 1; j < BUF_SIZE; j++)
						buf[j] = 0;
					buf_ind = 0;
				}
				add_table (test, k + 1);
				cur_code_len = find_length (cur_table_index);
				if (cur_table_index == MAX_TABLE)
				{
					put_code_to_buf (CLEAR);
					table_initialize ();
				}
				str_index = 0;
				str[str_index] = byte;
				code = byte;
			}
		}
		data += len;
	}
	/* Save the last str if something left there */
	if (str_index >= 0)
	{
		if ((code = code_from_table (str, str_index + 1)) >= 0)
			put_code_to_buf (code);
		else
			p_info (PI_ELOG, "Error in the algorithm\n");
	}
	/* Put the rest and eod */
	put_code_to_buf (EOD);
	fwrite (buf, 1, buf_ind + 1, file);
	clean_table ();
	p_info (PI_INFO, "\n");
}

int encode_raw8_data (FILE * file, RAW * raw)
{
	unsigned char str[MAX_STR], *data;
	unsigned char test[MAX_STR];
	int code = 0, oldcode, str_index = -1;	/* Last used symbol index in str */
	unsigned char byte = 0, oldbyte;
	int send_flag, i, j, k, buf_limit = BUF_SIZE - 6;
	int bit_order, color_mask = ((PXL_HDR *) raw->datafile)->color_mask;
	int jj, limit, len;
	int rc = 1;
	FILE *p_WrnFd = stdout;

/*
 * The raw data for color pixels has 8 bits per pixel
 * instead of 4 as in encode_raw_data
 */
	cur_table_index = 0;
	table_initialize ();
	for (j = 0; j < BUF_SIZE; j++)
		buf[j] = 0;
	buf_ind = 0;
	bits_left = BYTE;
	if (!raw)
	{
		p_info (PI_ELOG, "Could not find raw graphic\n");
		return 0;
	}
	/* Get bits from the source data */
	data = (unsigned char *)(raw->datafile + 512);
	put_code_to_buf (CLEAR);
	send_flag = 1;
	/* 
	 * Raw data has byte alignment,
	 * so we do not need to align
	 */
	len = raw->dpl;						/* Bytes per line */
	limit = len;
	for (i = 0; i < raw->lpg; i++)		/* For each line */
	{
		bit_order = 0x80;
		/* To show that we are thinking */
		p_info (PI_INFO, ".");
		fflush (p_WrnFd);
		for (jj = 0; jj < limit; jj++)
		{
			if (color_mask == 64 && send_flag)	/* We just sent a byte or two */
			{
				send_flag = 0;
				byte = 0;
			}
			oldbyte = data[jj];			/* In the old representation; pixel - 4 bits */
			if (color_mask == 64)
			{
				if (oldbyte)
					byte |= bit_order;
				bit_order >>= 1;
				if (!bit_order)
				{
					bit_order = 0x80;
					send_flag = 1;
				}
				if (jj == limit - 1)
				{
					send_flag = 1;
				}
			}
			else
				byte = oldbyte;

			if (!send_flag)				/* Do not process the byte */
				continue;
			/* Actual encoding */
			k = str_index + 1;
			for (j = 0; j < k; j++)
				test[j] = str[j];
			test[k] = byte;
			oldcode = code;
			if ((code = code_from_table (test, k + 1)) >= 0)
			{
				str[k] = byte;
				str_index++;
			}
			else
			{
				put_code_to_buf (oldcode);
				/* 
				 * Write the buffer to the file if it is around to be full,
				 * make sure we will have room for EOD
				 */
				if (buf_ind >= buf_limit)
				{
					fwrite (buf, 1, buf_ind, file);
					buf[0] = buf[buf_ind];
					for (j = 1; j < BUF_SIZE; j++)
						buf[j] = 0;
					buf_ind = 0;
				}
				add_table (test, k + 1);
				cur_code_len = find_length (cur_table_index);
				if (cur_table_index == MAX_TABLE)
				{
					put_code_to_buf (CLEAR);
					table_initialize ();
				}
				str_index = 0;
				str[str_index] = byte;
				code = byte;
			}
		}
		data += len;
	}
	/* Save the last str if something left there */
	if (str_index >= 0)
	{
		if ((code = code_from_table (str, str_index + 1)) >= 0)
			put_code_to_buf (code);
		else
		{
			p_info (PI_ELOG, "Error in the algorithm\n");
			rc = 0;
		}
	}
	/* Put the rest and eod */
	put_code_to_buf (EOD);
	fwrite (buf, 1, buf_ind + 1, file);
	clean_table ();
	p_info (PI_INFO, "\n");
	return rc;
}

int load_graphic (char *tree_name, char *graphic_name, char *dir_name)
{
	extern int trace_misc;
	PXL_HDR *px_hdr;
	int size;
	RAW *raw;
	char *datafile;
	struct stat sbuf;

	if (trace_misc)
		p_info (PI_TRACE, "load_graphic %s\n", graphic_name);
	datafile = (char *)load_graphic_file (tree_name, graphic_name, &size, dir_name);
	if (datafile == 0)
	{
		p_info (PI_ELOG, "No .pixel for graphic %s.\n", graphic_name);
		return (-1);
	}
	px_hdr = (PXL_HDR *) datafile;
	if (trace_misc)
		p_info (PI_TRACE, "dpl = %d  lpg = %d  dpi's = %d,%d  in _process %d\n",
				px_hdr->dpl, px_hdr->lpg, px_hdr->horiz_dpi, px_hdr->vert_dpi, px_hdr->in_process_flag);

	/* 
	 * Save graphic as most reciently used graphic in Que
	 */
	raw = (RAW *) QremoveTail (&raw_graphics);	/* least recent use */
	if (raw->datafile != 0)				/* any data pointer? */
		p_free ((char *)raw->datafile);	/* yes, free it */
	QinsertHead (&raw_graphics, (QE *) raw);	/* make most recent */
	raw->datafile = datafile;
	strcpy (raw->px_file_name, graphic_name);
	p_stat (tree_name, GRAPHICS, dir_name, graphic_name, &sbuf);
	raw->st_mtim = sbuf.st_mtime;
	raw->horiz_dpi = px_hdr->horiz_dpi;
	raw->vert_dpi = px_hdr->vert_dpi;
	raw->dpl = px_hdr->dpl;
	raw->lpg = px_hdr->lpg;
	raw->trim_left = px_hdr->trim_left * 1440 / px_hdr->horiz_dpi;
	raw->trim_top = px_hdr->trim_top * 1440 / px_hdr->vert_dpi;
	raw->width = px_hdr->dpl * 1440 / px_hdr->horiz_dpi;
	raw->depth = px_hdr->lpg * 1440 / px_hdr->vert_dpi;
	raw->line_alignment = px_hdr->line_alignment;
	raw->bits_per_pixel = px_hdr->bits_per_pixel;
	raw->crop_left = px_hdr->crop_left;
	raw->crop_top = px_hdr->crop_top;
	raw->crop_width = px_hdr->crop_width;
	raw->crop_depth = px_hdr->crop_depth;
	raw->scale = px_hdr->scale;
	raw->scaley = px_hdr->scaley;
	raw->file_format = px_hdr->file_format;
	if (trace_misc)
		p_info (PI_TRACE, "graphic loaded\n");
	return (0);
}
