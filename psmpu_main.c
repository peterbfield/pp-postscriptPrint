#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include "window.h"
#include "list_control.h"
#include "mp_styles.h"
#include "traces.h"
#include "psjob.h"

static int add_mpu_name (char *filename, int index);
static int clear_mpu_que(struct mpu_file *mpu_head);
static void zero_mpu_head_tail(void);

extern int16 mpu_next_line;
extern int mpu_page;

struct mpu_file *br_head;		/* pointer to first mpu_br structure */
struct mpu_file *br_tail;		/* pointer to last mpu_br structure */
struct mpu_file *ch_head;		/* pointer to first mpu_ch structure */
struct mpu_file *ch_tail;		/* pointer to last mpu_ch structure */
struct mpu_file *fn_head;		/* pointer to first mpu_fn structure */
struct mpu_file *fn_tail;		/* pointer to last mpu_fn structure */
struct mpu_file *il_head;		/* pointer to first mpu_il structure */
struct mpu_file *il_tail;		/* pointer to last mpu_il structure */
struct mpu_file *sn_head;		/* pointer to first mpu_sn structure */
struct mpu_file *sn_tail;		/* pointer to last mpu_sn structure */
struct mpu_file *vs_head;		/* pointer to first mpu_vs structure */
struct mpu_file *vs_tail;		/* pointer to last mpu_vs structure */
char uMemberName[132];
char uLayoutName[132];
char U_TreeName[132];
char U_DirName[132];
char U_DataName[132];
char uBR_UnitName[64];
char uCH_UnitName[64];
char uFN_UnitName[64];
char uIL_UnitName[64];
char uSN_UnitName[64];
char uVS_UnitName[64];
char uBR_PageName[64];
char uIL_PageName[64];
char uVS_PageName[64];
char uBR_FrameName[64];
BR_STYLE uBR_style;
CH_STYLE uCH_style;
FN_STYLE uFN_style;
IL_STYLE uIL_style;
SN_STYLE uSN_style;
VS_STYLE uVS_style;
char uReportType[32];
char uExt[][4] = {"brs", "chs", "fns", "ils", "sns", "vss"};
int U_PrtCharDepth = 71;
int U_PrtCharWid = 80;
int ReportFlag = 0;
WYSIWYG *u_wn = 0;

/******************************************************************/

void psmpu_main(void)
{
	char *dir_name_ptr, *dir_ext_name_ptr;
	int i;
	Pfd dir_fd;

	sprintf(uMemberName,"/Penta/%s/desks/%s/%s",
			TreeName, SubDirName, ".member2.list");
	sprintf(uLayoutName,"/Penta/%s/desks/%s/%s",
			TreeName, SubDirName, ".layout2.list");
	sprintf(U_DataName,"/Penta/%s/desks/%s", TreeName, SubDirName);
	strcpy (U_TreeName, TreeName);
	strcpy (U_DirName, SubDirName);

/*----------------  Process Data Files  -----------*/

    u_wn = (WYSIWYG *)p_alloc(sizeof(WYSIWYG));
    u_wn -> selected_lay = (LAYOUT_DESC *)p_alloc(sizeof(LAYOUT_DESC));
    QinsertTail(&u_wn -> layout_list, (QE *)u_wn -> selected_lay);
    u_wn -> msb = 20;
    u_wn -> ldb = 10;
    u_wn -> yx_convert[0] = 5400 / u_wn -> ldb;
    u_wn -> yx_convert[1] = 5400 / u_wn -> msb;
    strcpy(u_wn -> tree_name, TreeName);
    strcpy(u_wn -> dir_name, SubDirName);
	mpu_page = 1;
	strcpy (uReportType, " - ALL DIRECTORY FILES ");
	zero_mpu_head_tail();
	if ( (LYPrintFlag & 1) && (Reports <= 0) )
	{							/* try to print data before pages processed */
		if ((dir_fd=p_opendir(TreeName, DESK, SubDirName, NO_STRING)) ==
			P_ERROR)
		{
			p_info(PI_ELOG, "ERROR - could not open directory Tree '%s', Dir '%s' for reports.\n",
				   TreeName,SubDirName);
			return;
		}
		while ((dir_name_ptr = p_readdir(dir_fd)) )
		{						/* look at all the directory names */
			if ( *dir_name_ptr == '.')
				continue;		/* ignore files starting with '.' */
			if ( !(dir_ext_name_ptr = strrchr(dir_name_ptr, '.')) )
				continue;		/* ignore files with no .extension */
			if ( strlen(dir_ext_name_ptr) != 4)
				continue;		/* only test for match if '.ccc' */
			*dir_ext_name_ptr++ = 0; /* null at extension, point past '.' */
			for (i=0; i<6; i++)
			{					/* look for extension match */
				if ( !strcmp(dir_ext_name_ptr, (char *)&uExt[i][0]) )
					break;		/* got a match */
			}
			if ( i >= 6)
				continue;		/* no match */
			if (strlen(dir_name_ptr) > 126)
			{
				p_info(PI_ELOG, "ERROR - Report filename '%s' in Tree '%s', Dir '%s' is too long.\n",
				   TreeName,SubDirName, dir_name_ptr);
				continue;
			}
			add_mpu_name (dir_name_ptr, i);	/* Save Useful File Name */
		}						/* end while(dir_name=p_readdir(dir_fd)) */
		mpu_data_print_br ();
		mpu_data_print_ch ();
		mpu_data_print_fn ();
		mpu_data_print_sn ();
		mpu_data_print_vs ();
	}                           /* end if(LYPrintFlag&1) */
	if ( (LYPrintFlag & 2) && (Reports <= 0) )
		mpu_data_print_il ();
	clean_wysiwyg(u_wn);
	return_mpu_que();
	if ( LYPrintFlag & 4)
		beg_PAGE();
}								/* end main */

/*******************************************************/
void send_it( char buff[200][132], int *next_line)
{
    int i, j, y_val, imageheight_in_pts;

	if ( !*next_line)
		return;
	ReportFlag = 1;
	cc_mask = 1;
    for(i = 0; i < *next_line; i++)
        buff[i][U_PrtCharWid-1] = 0;
	beg_PAGE();
	imageheight_in_pts = Imageheight / VerticalBase;
	y_val = imageheight_in_pts - 10;
    for(i=0; i<*next_line; i++)
	{
		for (j=0; j<U_PrtCharWid-1; j++)
		{						/* only print non-blank lines */
			if ( !buff[i][j])
				break;			/* at end of line, no printing */
			if ( buff[i][j] == ' ')
				continue;		/* still finding blanks, keep going */
			m_fprintf("0 %d M\n(%s)S\n", y_val, &buff[i]);
			break;
		}
		y_val -= ReportSize;
		if (y_val < 0)
		{
			if ( (i + 1) >= *next_line)
				break;
			end_PAGE();
			beg_PAGE();
			y_val = (imageheight_in_pts - 10);
		}
	}							/* end for(i=0;i<*next_line;i++) */
	end_PAGE();
	ReportFlag = 0;
	*next_line = 0;
}								/* end function */

/*******************************************************/
void return_mpu_que(void)
{
	clear_mpu_que(br_head);
	clear_mpu_que(ch_head);
	clear_mpu_que(fn_head);
	clear_mpu_que(il_head);
	clear_mpu_que(sn_head);
	clear_mpu_que(vs_head);
	zero_mpu_head_tail();
}								/* end function */

/*******************************************************/
static int clear_mpu_que(struct mpu_file *mpu_head)
{
	struct mpu_file *current_rec;
	struct mpu_file *next_rec;

	if ( !(current_rec = mpu_head) )
		return(0);
	while (current_rec)
	{
		next_rec = current_rec -> next;
		p_free ( (char *)current_rec);
		current_rec = next_rec;
	}
	return(0);
}								/* end function */

/*******************************************************/
static void zero_mpu_head_tail(void)
{
		br_head = 0;
		br_tail = 0;
		ch_head = 0;
		ch_tail = 0;
		fn_head = 0;
		fn_tail = 0;
		il_head = 0;
		il_tail = 0;
		sn_head = 0;
		sn_tail = 0;
		vs_head = 0;
		vs_tail = 0;
}								/* end function */

/******************************************************************/
static int add_mpu_name (char *filename, int index)
{
	struct mpu_file *current_rec;

	current_rec = (struct mpu_file *)p_alloc(sizeof(struct mpu_file));
	if ( !current_rec )
	{
		p_info(PI_ELOG, "Allocate failed for report filename save structure\n");
		return(1);
	}
	current_rec -> next = 0;
	memcpy (current_rec -> filename, filename,
			sizeof (current_rec -> filename) );
	switch (index)
	{							/* insert name record into que */
	  case 0:
		if ( !br_head )
			br_head = current_rec;
		else
			br_tail -> next = current_rec; /* prev struct points to
											  this new one */
		br_tail = current_rec;
		break;

	  case 1:
		if ( !ch_head )
			ch_head = current_rec;
		else
			ch_tail -> next = current_rec;
		ch_tail = current_rec;
		break;

	  case 2:
		if ( !fn_head )
			fn_head = current_rec;
		else
			fn_tail -> next = current_rec;
		fn_tail = current_rec;
		break;

	  case 3:
		if ( !il_head )
			il_head = current_rec;
		else
			il_tail -> next = current_rec;
		il_tail = current_rec;
		break;

	  case 4:
		if ( !sn_head )
			sn_head = current_rec;
		else
			sn_tail -> next = current_rec;
		sn_tail = current_rec;
		break;

	  case 5:
		if ( !vs_head )
			vs_head = current_rec;
		else
			vs_tail -> next = current_rec;
		vs_tail = current_rec;
		break;
	}							/* end switch(i) */
	return(0);
}								/* end function */

/******************************************************************/
void init_mpu_in_use_print(void)
{

	if ( (REL_DATA(0) p2) && *(REL_DATA(0) p2) )
		strcpy(uBR_PageName, REL_DATA(0) p2);
	else
		strcpy(uBR_PageName, uBR_UnitName);
	add_mpu_name (uCH_UnitName, 1);
	add_mpu_name (uFN_UnitName, 2);
	if ( (REL_DATA(0) p0) && *(REL_DATA(0) p0) )
		strcpy(uIL_PageName, REL_DATA(0) p0);
	else
		strcpy(uIL_PageName, uIL_UnitName);
	add_mpu_name (uIL_PageName, 3);
	add_mpu_name (uSN_UnitName, 4);
	if ( (REL_DATA(0) p3) && *(REL_DATA(0) p3) )
		strcpy(uVS_PageName, REL_DATA(0) p3);
	else
		strcpy(uVS_PageName, uVS_UnitName);
	add_mpu_name (uVS_PageName, 5);
}								/* end function */

/******************************************************************/
int verify_and_add_mpu_name (int index)
{
	struct mpu_file *cur_rec, *nxt_rec;
	int result;

	switch (index)
	{							/* only brs varies with frame at this time */
	  case 0:
		if ( (REL_DATA(CurrentFrame) p2) && *(REL_DATA(CurrentFrame) p2) )
			strcpy(uBR_FrameName, REL_DATA(CurrentFrame) p2);
		else
			strcpy(uBR_FrameName, uBR_PageName);
		if ( (cur_rec = br_head) )
		{						/* see if a match */
			nxt_rec = cur_rec;
			while (nxt_rec)
			{
				nxt_rec = cur_rec -> next;
				if ( !strcmp (cur_rec -> filename, uBR_FrameName) )
					return (0);	/* match, do not add to list */
			}					/* end while(nxt_rec) */
		}						/* end if(cur_rec = br_head) */
	}							/* end switch(index) */
	result = add_mpu_name (uBR_FrameName, 0);
	return(result);
}								/* end function */

/*************************** end of file *****************************/
