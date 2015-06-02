/*			DISPLAY A .TX FILE				      */
#include <stdio.h>
#include <ctype.h>
#include "p_lib.h"
#include "psjob.h"
#include "frame.h"
#include "rel_data.h"
#include "txfile.h"
#include "map.h"

static void check_tpass (unsigned char c);
static void chk_ty_depth (int inc);
static void do_below_32(unsigned char c);
static void mpu_pts (int value, char *string, int string_length);
static void output_c(unsigned char c);
static void print_design_text_line(void);
static void print_line(int word_nr, int limit);
static void save_mpu_txprint (void);

#define BUFFSIZE	8

#if i386
#define LEFT_BYTE(c)	(c & 0xff)
#define RIGHT_BYTE(c)	((c >> 8) & 0xff)
#else
#define LEFT_BYTE(c)	((c >> 8) & 0xff)
#define RIGHT_BYTE(c)	(c & 0xff)
#endif

extern int U_PrtCharDepth;
extern int U_PrtCharWid;
extern int FoFileType;
extern int tx_page_started_flag;

static int displaying;
static char uTxfileName[132];
static int tpage;
static int pass_mode;
static int char_counter;
static int mpu_text_tline;		/* index into tln_prt_buff[nl][] */
static char tln_prt_buff[200][132];
static struct mpu_txp *head;	/* pointer to first mpu_txp structure */
static struct mpu_txp *tail;	/* pointer to last mpu_txp structure */
static 	ELEMENT *ele;
static char *input_ptr;
static uint16 line_buf[BUFFSIZE];

/******************************************************************/

void mpu_textfile_print(int header_flag)
{
	int start_line;
	int stop_line;
	int next;
	int rec_no;
	int word_nr;
	uint16 *wdptr;
	Pfd uTxfilePfd;
	char line_num[12];
	int j;
	char string1[64], string2[64], string3[64], string4[64], string5[64];
	int vjtotal;
	struct tx_block txblock;
	struct tx_block *blk_ptr;
	struct tx_head *head;
	struct tx_line *lnptr;
	WYSIWYG *save_frame_wn;
	int i = 0;

	displaying = 0;
	start_line = PsCurRec -> elem -> map_data.start_line;
	stop_line = PsCurRec -> elem -> map_data.end_line;
	if ( !PsCurRec -> elem -> map_data.end_forec)
		stop_line = start_line;	/* only one line */
	ele = FRAME_DATA(CurrentFrame) ele;
	pass_mode = 0;
	char_counter = 0;
	if ( !header_flag)
	{
		tx_page_started_flag = 1;
		tpage = 1;
		memset((char *)&tln_prt_buff[0], 040, sizeof(tln_prt_buff));
		if ( MasterNameFlag)
			sprintf((char *)&tln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   MASTER (page %d)",
					LayoutName, TreeName, SubDirName, tpage);
		else
			sprintf((char *)&tln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   PAGE %d (page %d)",
					LayoutName, TreeName, SubDirName, PageNo1, tpage);
		mpu_text_tline = 2;
		return;
	}
	if ( FoFileType == LFO_FILE )
	{
		if ( !(input_ptr =
			   wn->selected_lay->frames[CurrentFrame]->rel_data.p0))
			return;				/* no text string */
		chk_ty_depth (3);
		mpu_text_tline++;
		sprintf((char *)&tln_prt_buff[mpu_text_tline++][3],
				"DESIGN TEXT  (FRAME %d)",
				CurrentFrame);
		print_design_text_line(); /* text from layout */
		if (char_counter > 6)
			mpu_text_tline++;	/* go to a new line for partial line */
		return;
	}
	if ( !PsCurRec -> elem -> map_data.start_forec)
		return;					/* no text */

/**************** print text from .txt file ************/

	sprintf(uTxfileName, "/Penta/%s/desks/%s/%s.txt", TreeName,
			SubDirName, FoName);
	uTxfilePfd = p_open(TreeName, TEXT_FILE, SubDirName, FoName, "r");
	if( !uTxfilePfd)
	{
		p_info(PI_ELOG, "While printing .txt file, Cannot open file %s\n", uTxfileName);
		return;
	}
	pass_mode = 0;
	char_counter = 0;
	p_read ( (char *)&txblock, 512, 1, uTxfilePfd, 0, SW_HEADER);
	blk_ptr = &txblock;
	head = (struct tx_head *)&txblock;
	next = head->h_fchains[FTEXT];
	rec_no = next;
	if (start_line > head->h_hst.s_hiline)
	{
		p_info(PI_WLOG, "Line %d exceeds highest line in file (%d)\nStarting at line 1.\n",
			   start_line, head->h_hst.s_hiline);
		start_line = 0;
		displaying = 1;
	}
	chk_ty_depth (3);
	mpu_text_tline++;
	sprintf((char *)&tln_prt_buff[mpu_text_tline++][3],
			"ARTICLE '%s'  (FRAME %d)",
			uTxfileName, CurrentFrame);
	chk_ty_depth (3);
	mpu_text_tline++;
	save_frame_wn = wn;
	wn = PsHead ->frame_wn;
	if ( OBJ_REF(PsCurRec -> orig_frame_nbr) )
	{							/* object frame, only print message */
		sprintf((char *)&tln_prt_buff[mpu_text_tline++][3],
				"OBJECT FRAME REFERENCE: Page (%ld)  UniqueID (%ld)",
				(OBJ_REF(PsCurRec -> orig_frame_nbr) >> 12) & MP_OB_LAY,
				OBJ_REF(PsCurRec -> orig_frame_nbr) & MP_OB_FRAME);
		chk_ty_depth (3);
		mpu_text_tline++;
		wn = save_frame_wn;
		return;
	}
	wn = save_frame_wn;
	sprintf((char *)&tln_prt_buff[mpu_text_tline++][29],
			"# LINES   # PARAS   EX. LEAD    DEPTH");
	mpu_pts (ele->map_data.vj_exld_cnt, string1, 14);
	sprintf((char *)&tln_prt_buff[mpu_text_tline++][7],
			"TRY %+4d     ON PAGE: %5d %9d %s",
			ele->map_data.vj_try, ele->map_data.vj_line_cnt,
			ele->map_data.vj_para_cnt, string1);
	vjtotal = ele->map_data.vj_line_adj + ele->map_data.vj_para_adj +
		ele->map_data.vj_exld_adj + ele->map_data.vj_depth_adj;
	mpu_pts (vjtotal, string1, 9);
	mpu_pts (ele->map_data.vj_line_adj, string2, 10);
	mpu_pts (ele->map_data.vj_para_adj, string3, 9);
	mpu_pts (ele->map_data.vj_exld_adj, string4, 9);
	mpu_pts (ele->map_data.vj_depth_adj, string5, 9);
	sprintf((char *)&tln_prt_buff[mpu_text_tline++][7],
			"ADJ %s    ADJ: %s %s %s %s",
			string1, string2, string3, string4, string5);
	pass_mode = 0;
	char_counter = 0;
/*
  On to the tx_blocks
*/
	while(next)
	{
		rec_no = next;
		word_nr = 7;		/* Word after the block data. */
		if (p_read( (char *)blk_ptr, 512, 1, uTxfilePfd, rec_no, SW_TEXTBLOCK_IN) != 1)
		{
			p_info(PI_ELOG, "Read failure in file %s, record %d during .txt print.\n",
				   uTxfileName, rec_no);
			p_close(uTxfilePfd);
			return;
		}
		if (next == blk_ptr->txb_fwd ||
			next == blk_ptr->txb_rev ||
			(blk_ptr->txb_fwd && blk_ptr->txb_fwd == blk_ptr->txb_rev))
		{
			p_info(PI_ELOG, "Block linkage error in file %s, record %d during .txt print.\n",
				   uTxfileName, rec_no);
			p_close(uTxfilePfd);
			return;
		}
		next = blk_ptr->txb_fwd;
		if (blk_ptr->txb_hln < start_line)
			continue;
/*
  On to the tx_lines
*/
		wdptr = (uint16 *)blk_ptr->txb_buff;
		while((wdptr - (uint16 *)blk_ptr->txb_buff) < (blk_ptr->txb_size - 6))
		{
			if(*wdptr == TX_EOL)
			{
				if ( i)
				{
					print_line(word_nr,i);
					word_nr += i;
					i = 0;
				}
				word_nr += (i + 8);
				i = 0;
				lnptr = (struct tx_line *)wdptr;
				if (lnptr->txl_linum >= start_line)
					displaying = 1;
				if (stop_line > 0 && lnptr->txl_linum >= stop_line)
				{
					if (displaying)
						mpu_text_tline++;
					p_close(uTxfilePfd);
					return;
				}
				if (displaying)
				{
					sprintf (line_num,"%6d ",lnptr->txl_linum);
					char_counter = 0;
					chk_ty_depth (1);
					mpu_text_tline++;
					for (j=0; j<7; j++)					
						output_c( (unsigned char )line_num[j]);
				}
				wdptr += sizeof(struct tx_line) >> 1;
			}					/* end if(*wdptr==TX_EOL) */
			else
			{					/* not at line start, step thru line */
				line_buf[i++] = *wdptr++;
				if ( i == BUFFSIZE)
				{
					print_line(word_nr,i);
					word_nr += i;
					i = 0;
				}
			}
		}						/* end while(wdptr...) */
		print_line(word_nr,i);
		word_nr += i;
		i = 0;
	}							/* end while(next) */
	p_close(uTxfilePfd);
}								/* end function */

/******************************************************************/
static void print_line(int word_nr, int limit)
{
	unsigned char left;
	unsigned char right;
	int i;
	
	if ( !limit || !displaying)
		return;
	for (i = 0; i < limit; i++)
	{
		left = (unsigned char)LEFT_BYTE(line_buf[i]);
		right = (unsigned char)RIGHT_BYTE(line_buf[i]);
		if( left <= 31)
		{
			do_below_32 (left);
			left = '\000';
		}
		else if ( left == '~')
			left = '\000';
		else if ( left >  '~')
		{
			check_tpass (left);
			left = '\000';
		}
		output_c (left);
		if( right <= 31)
		{
			do_below_32 (right);
			right = '\000';
		}
		else if ( right == '~')
			right = '\000';
		else if ( right >  '~')
		{
			check_tpass (right);
			right = '\000';
		}
		output_c (right);
	}							/* 	end for(i=0;i<limit;i++) */
}								/* end function */

/*******************************************************/
static void do_below_32(unsigned char c)
{

	switch(c)
	{
	  case 2:					/* paragraph */
		output_c('[');
		output_c('e');
		output_c('p');
		break;
	  case 3:					/* quad right */
	  case 4:					/* quad left */
	  case 5:					/* quad center */
		output_c('[');
		output_c('q');
		switch(c)
		{
		  case 3:
			output_c('r');
			break;
		  case 4:
			output_c('l');
			break;
		  case 5:
			output_c('c');
			break;
		}
		break;
	  case 6:					/* em space */
		output_c('_');
		break;
	  case 7:					/* en space */
		output_c('^');
		break;
	  case 8:					/* thin space */
		output_c('|');
		break;
	  case 9:					/* begin note */
		output_c('[');
		output_c('c');
		output_c('t');
		break;
	  case 10:					/* end note */
		output_c('[');
		output_c('x');
		output_c('c');
		break;
	  case 11:					/* + 1 relative unit */
	  case 12:					/* - 1 relative unit */
	  case 13:					/* em dash */
	  case 14:					/* en dash */
		output_c('=');
		switch(c)
		{
		  case 11:
			output_c('+');
			break;
		  case 12:
			output_c('-');
			break;
		  case 13:
			output_c('m');
			break;
		  case 14:
			output_c('n');
			break;
		}
		break;
	  case 15:					/* start style tag call */
		output_c('<');
		break;
	  case 16:					/* end style tag call */
		output_c('>');
		break;
	  case 17:					/* start style tag named variable call */
		output_c('(');
		break;
	  case 18:					/* end style tag named variable call */
		output_c(')');
		break;
	  case 19:					/* style tag universal call */
		output_c('.');
		break;
	  case 20:					/* style tag start wildcard */
		output_c(',');
		break;
	  case 21:					/* style tag branch to format */
		output_c('%');
		break;
	  case 22:					/* single key copy merge */
		output_c('#');
		break;
	}							/* end switch (c) */
	return;
}								/* end function) */

/*******************************************************/
static void output_c(unsigned char c)
{
	char *band_ptr, *cmd_ptr;

	if ( !c || pass_mode)
		return;
	chk_ty_depth (1);
	if (char_counter+2 >= U_PrtCharWid)
	{
		if (tln_prt_buff[mpu_text_tline][char_counter-1] == ' ')
			char_counter = 7;	/* break at end of line */
		else
		{						/* look for break at band or cmd key */
			tln_prt_buff[mpu_text_tline][char_counter] = 0;
			band_ptr = strrchr ( (char *)&tln_prt_buff[mpu_text_tline], ' ');
			cmd_ptr = strrchr ( (char *)&tln_prt_buff[mpu_text_tline], '[');
			if ( band_ptr && (strlen (band_ptr) > 1) &&
				 (band_ptr > &tln_prt_buff[mpu_text_tline][30]) )
			{					/* break on the last band */
				strcpy ((char *)&tln_prt_buff[mpu_text_tline+1][6], band_ptr);
				char_counter = 6 + strlen (band_ptr);
				*band_ptr = 0;	/* end new line */
			}
			else if ( cmd_ptr && (strlen (cmd_ptr) > 1) &&
					  (cmd_ptr > &tln_prt_buff[mpu_text_tline][30]) )
			{					/* break on the last command key */
				strcpy ((char *)&tln_prt_buff[mpu_text_tline+1][7], cmd_ptr);
				char_counter = 7 + strlen (cmd_ptr);
				*cmd_ptr = 0;	/* end new line */
			}
			else
				char_counter = 7; /* break at end of line */
		}
		mpu_text_tline++;
		chk_ty_depth (1);
	}
	if ( (c == 92) || (c == 40) ||
		( c == 41) )			/* for \, (, ),need to add '\' for pp */
		sprintf((char *)&tln_prt_buff[mpu_text_tline][char_counter++],"%c",92);
	sprintf((char *)&tln_prt_buff[mpu_text_tline][char_counter++], "%c", c);
}								/* end function */

/*******************************************************/
static void check_tpass (unsigned char c)
{
	switch (c)
	{
/* strip to end */
	  case 0321:				/* hex d1 - Begin Error Message */
		pass_mode |= 0x0200;
		break;
	  case 0322:				/* hex d2 - End Error Message */
		pass_mode &= ~0x0200;
		break;
/* pass on, ignore to end */
	  case 0361:				/* hex f1 - Level 1 Begin Delete */
		pass_mode |= 0x1000;
		break;
	  case 0362:				/* hex f2 - Level 1 End Delete */
		pass_mode &= ~0x1000;
		break;
	  case 0363:				/* hex f3 - Level 2 Begin Delete */
		pass_mode |= 0x2000;
		break;
	  case 0364:				/* hex f4 - Level 2 End Delete */
		pass_mode &= ~0x2000;
		break;
	  case 0365:				/* hex f5 - Begin Editor's Note */
		pass_mode |= 0x4000;
		break;
	  case 0366:				/* hex f6 - End Editor's Note */
		pass_mode &= ~0x4000;
		break;
	  case 0367:				/* hex f7 - Begin Author's Note */
		pass_mode |= 0x8000;
		break;
	  case 0370:				/* hex f8 - End Author's Note */
		pass_mode &= ~0x8000;
		break;
	}
}								/* end function */

/********************************************************/
static void chk_ty_depth (int inc)
{
	if((mpu_text_tline + inc) >= U_PrtCharDepth)
	{
		save_mpu_txprint ();
		memset((char *)&tln_prt_buff[0], 040, sizeof(tln_prt_buff));
		tpage++;
		if ( MasterNameFlag)
			sprintf((char *)&tln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   MASTER (page %d)",
					LayoutName, TreeName, SubDirName, tpage);
		else
			sprintf((char *)&tln_prt_buff[0][0],
					"LAYOUT '%s' FOR /%s/%s   PAGE %d (page %d)",
					LayoutName, TreeName, SubDirName, PageNo1, tpage);
		mpu_text_tline = 2;
		if ( FoFileType == LFO_FILE )
			sprintf((char *)&tln_prt_buff[mpu_text_tline++][3],
					"DESIGN TEXT  (FRAME %d) - CONTINUED",
					CurrentFrame);
		else
			sprintf((char *)&tln_prt_buff[mpu_text_tline++][3],
					"ARTICLE '%s'  (FRAME %d) - CONTINUED",
					uTxfileName, CurrentFrame);
		mpu_text_tline++;
	}
}								/* end function */

/******************************************************************/
void init_mpu_txprint (void)
{
	head = 0;
	tail = 0;
	tpage = 0;
}								/* end function */

/******************************************************************/
static void save_mpu_txprint (void)
{
	struct mpu_txp *current_rec;
	char *page_area;
	int i, j;

	if ( !mpu_text_tline)
		return;
    current_rec = (struct mpu_txp *) p_alloc (sizeof (struct mpu_txp));
	page_area = p_alloc (mpu_text_tline * U_PrtCharWid);
	if ( !current_rec )
	{
		p_info(PI_ELOG, "Allocate failed for txtfile print structure\n");
		return;
	}
	if ( !page_area )
	{
		p_info(PI_ELOG, "Allocate failed for txtfile print page\n");
		p_free ( (char *)current_rec);
		return;
	}
	current_rec -> next = 0;
	current_rec -> line_count = mpu_text_tline;
	if ( !head )
		head = current_rec;
    else
		tail -> next = current_rec;	/* prev struct points to this new one */
	tail = current_rec;
    current_rec -> page_saved = page_area;
	for (i=0; i<mpu_text_tline; i++)
	{
		for (j=0; j<U_PrtCharWid; j++)
		{
			*(page_area++) = tln_prt_buff[i][j]; /* save page */
		}
	}
}								/* end function */

/******************************************************************/
void output_mpu_txprint (void)
{
	struct mpu_txp *current_rec;
	char *page_area;
	int i, j, line_cnt;

	if ( mpu_text_tline)
		save_mpu_txprint ();
	if (!head)
	{
		beg_PAGE();
		end_PAGE();
		return;
	}
	do
	{
		current_rec = head;
		page_area = current_rec -> page_saved;
		memset((char *)&tln_prt_buff[0][0], 040, sizeof(tln_prt_buff));
		line_cnt = current_rec -> line_count;
		for (i=0; i<current_rec -> line_count; i++)
		{
			for (j=0; j<U_PrtCharWid; j++)
				tln_prt_buff[i][j] = *(page_area++); /* restore page */
		}
		send_it (tln_prt_buff, &line_cnt);
		head = current_rec -> next;
		p_free (current_rec -> page_saved);
		p_free ( (char *)current_rec);
	}while ( head);
}								/* end function */

/********************************************************/
static void mpu_pts (int value, char *string, int string_length)
{								/* make pi p pts right justified */
	int fract_y;
	int tmp_y;
	int pi, pt;
	int new_length;
	char ptstr[12] = {"  "};	/* 1 or 2 digits */
	char frstr[12] = {"  "};	/* only decimal and 1 digit, lead in tenths */
	char temp_string[12];

	tmp_y = value / VerticalBase;
	pi = tmp_y / 12;			/* picas */
	pt = tmp_y % 12;			/* points */
	fract_y = ((value % VerticalBase) * 100) / VerticalBase;
	if ( pt || fract_y)
	{
		if ( !pt)
		{						/* no points, only fraction */
			sprintf (frstr,"0.%d ",fract_y); /* add lead '0', trail ' ' */
			ptstr[0] = 0;
		}
		else
		{						/* point present */
			if (fract_y)
				sprintf (frstr,".%d",fract_y);
			if (pt >9)
				itoa (pt, ptstr); /* 2 digits for points*/
			else
			{					/* 1 digit for points */
				itoa (pt, ptstr);
				if (fract_y)	/* 1 digit points with fraction */
					sprintf (frstr,".%d ",fract_y);	/* add space at end */
			}					/* end else 1 digit points */
		}						/* end else points present */
	}							/* end if(pt||fract_y) */
	sprintf (temp_string, "%dP%s%s", pi, ptstr,frstr);
	memset (string, ' ', string_length);
	new_length = strlen(temp_string);
	strcpy ((char *)(string + string_length - new_length), temp_string);
}								/* end function */

/******************************************************************/
static void print_design_text_line(void)
{
	unsigned char input_char;

	char_counter = 6;
	while ((input_char = *input_ptr++))
	{
		if (input_char == 10)	/* '\n' */
		{
			if ( ! *input_ptr)
				break;
			chk_ty_depth (1);	/* output new line */
			mpu_text_tline++;
			char_counter = 6;
			continue;
		}
		output_c (input_char);
	}							/* end while(input_char=*input_ptr++) */
}								/* end function */

/******************* EOF *****************/
