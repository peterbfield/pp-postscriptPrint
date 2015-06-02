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
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "window.h"
#include "link_util.h"
#include "list_control.h"
#include "frame.h"
#include "llist.h"
#include "traces.h"
#include "rel_data.h"
#include "map.f"
#include "mem_lay.f"

extern int compare_filename(char *p1, char *p2);

void build_map_line(char line[], MAP_DATA *data);

typedef struct map_key
{
	char *keyword;
	char **answer;
} MAP_KEY;

static int trace_map = 0;
static int first_call = 1;

static MAP_KEY keys[] =
{
	{"FL", 0},					/*  0  first line */
	{"LL", 0},					/*  1  last line */
	{"OL", 0},					/*  2  overflow line */
	{"LY", 0},					/*  3  layout */
	{"ID", 0},					/*  4  layout ID */
	{"FR", 0},					/*  5  frame */
	{"PO", 0},					/*  6  piece of */
	{"BF", 0},					/*  7  begin flow */
	{"EF", 0},					/*  8  end flow */
	{"SF", 0},					/*  9  suspend flow */
	{"PL", 0},					/* 10  previous layout */
	{"NL", 0},					/* 11  next layout */
	{"WE", 0},					/* 12  widow error */
	{"OE", 0},					/* 13  orphan error */
	{"BE", 0},					/* 14  break error */
	{"VJT", 0},					/* 15  vertical top adjust */
	{"VCL", 0},					/* 16  vertical line count */
	{"VJL", 0},					/* 17  vertical line adjust */
	{"VCP", 0},					/* 18  vertical paragraph count */
	{"VJP", 0},					/* 19  vertical paragraph adjust */
	{"VCE", 0},					/* 20  vertical extra lead count */
	{"VJE", 0},					/* 21  vertical extra lead adjust */
	{"VCB", 0},					/* 22  vertical band count */
	{"VJB", 0},					/* 23  vertical band adjust */
	{"VJM", 0},					/* 24  last vert action performed */
	{"VJD", 0},					/* 25  column depth adjustment */
	{"VST", 0},					/* 26  try table index */
	{"LF", 0},					/* 27  previous frame has a suspend flow */
	{"TAB", 0},					/* 28  Frame split across facing pages */
};

static void validate_map_file(WYSIWYG *wn, char *filename, QUEUE *map_data,
							  int layout_text_flag)
{
	MAP_DATA *data, *prev_data = NULL;
	BOOLEAN changes_made = FALSE;

	for(data = (MAP_DATA *)map_data -> head; 
		data; data = (MAP_DATA *)data -> que.next)
	{
		if (prev_data && !prev_data -> next_lay_id && data -> prev_lay_id)
		{
			changes_made = TRUE;
			prev_data -> next_lay_id = data -> layout_id;
			prev_data -> next_frame_id = data -> frame_id;
		}
		if (prev_data && prev_data -> next_lay_id && !data -> prev_lay_id)
		{
			changes_made = TRUE;
			data -> prev_lay_id = prev_data -> layout_id;
			data -> prev_frame_id = prev_data -> frame_id;
		}
		if (data -> command_flags & EF)
			prev_data = NULL;
		else
			prev_data = data;
	}
	if (changes_made)
		map_dump(wn, filename, map_data, layout_text_flag);
}

int map_load(WYSIWYG *wn, char *filename, QUEUE *map_data,
			 int layout_text_flag)
{
	char *ptrs[100];
	char line[256];
	char *start = NULL;
	char **answer = NULL;
	char *ptr;
	int i;
	int num;
	int cnt;
	int scan_mode;
	Pfd fd;
	MAP_DATA *data;
	int hnj_index;
	int map_file_name = MAP_FILE;

	Qclear(map_data);
	num = sizeof(keys)/sizeof(MAP_KEY);
	if (layout_text_flag)
		map_file_name = LMAP_FILE;
	fd = p_open(wn -> tree_name, map_file_name, wn -> dir_name, filename, "r");
	if (fd == P_ERROR)
		return(P_FAILURE);
	hnj_index = 0;
	while(p_fgets(line, sizeof(line), fd))
	{
		ptr = &line[0];
		scan_mode = 0;
		cnt = 0;
		for(i = 0; i < num; i++)
			keys[i].answer = 0;
		while(*ptr)
		{
			switch (*ptr)
			{
			  case ',':
			  case ' ':
			  case '\t':
			  case '\n':
			  case '\f':
			  case '\r':
				if(scan_mode)
				{
					*ptr = '\0';
					scan_mode = 0;
				}
				break;
			  case ':' :
				  *ptr = '\0';
				for(i = 0; i < num; i++)
				{
					if(strcmp(start, keys[i].keyword) == 0)
						keys[i].answer = answer;
				}
				scan_mode = 0;
				break;
			  default:
				if(scan_mode == 0) /* scanning for start */
				{
					start = ptr;
					answer = &ptrs[cnt];
					ptrs[cnt++] = ptr;
					scan_mode = 1;
				}
				break;
			}
			ptr++;
		}
		data = (MAP_DATA *)p_alloc(sizeof(MAP_DATA));
		QinsertTail(map_data, (QE *)data);
		data -> hnj_index = ++hnj_index;
		if ((answer = keys[0].answer) != NULL) /* FL */
		{
			data -> start_line = atoi(*(++answer));	
			data -> start_forec = atoi(*(++answer));
			data -> start_fowrd = atoi(*(++answer));
		}
		if ((answer = keys[1].answer) != NULL) /* LL */
		{
			data -> end_line = atoi(*(++answer));		
			data -> end_forec = atoi(*(++answer));
			data -> end_fowrd = atoi(*(++answer));	
			data -> underflow_depth = atoi(*(++answer));	
		}
		if ((answer = keys[2].answer) != NULL) /* OL */
		{
			data -> overflow_line = atoi(*(++answer));	
			data -> overflow_forec = atoi(*(++answer));	
			data -> overflow_fowrd = atoi(*(++answer));	
			data -> overflow_depth = atoi(*(++answer));
		}
		if ((answer = keys[28].answer) != NULL) /* TAB */
			data -> tab_split = atoi(*(++answer));	
		if ((answer = keys[3].answer) != NULL) /* LY */
			data -> layout_id = atoi(*(++answer));		
		if ((answer = keys[4].answer) != NULL) /* ID */
			data -> frame_id = atoi(*(++answer));		
		if ((answer = keys[5].answer) != NULL) /* BO */
			data -> frame_num = atoi(*(++answer));
		if ((answer = keys[6].answer) != NULL) /* PO */
		{
			data -> piece = atoi(*(++answer));	
			data -> of_p = atoi(*(++answer));		
		}
		if ((answer = keys[10].answer) != NULL)	/* PL */
		{
			data -> prev_lay_id = atoi(*(++answer));	
			data -> prev_frame_id = atoi(*(++answer));	
		}
		if ((answer = keys[11].answer) != NULL)	/* NL */
		{
			data -> next_lay_id = atoi(*(++answer));	
			data -> next_frame_id = atoi(*(++answer));	
		}
		if ((answer = keys[7].answer) != NULL) /* BF */
			data -> command_flags	|= BF;
		if ((answer = keys[8].answer) != NULL) /* EF */
			data -> command_flags	|= EF;
		if ((answer = keys[9].answer) != NULL) /* SF */
			data -> command_flags	|= SF;
		if ((answer = keys[27].answer) != NULL)	/* LF */
			data -> command_flags	|= LF;
		if ((answer = keys[12].answer) != NULL)	/* WE */
			data -> error_flags		|= WE;
		if ((answer = keys[13].answer) != NULL)	/* OE */
			data -> error_flags		|= OE;
		if ((answer = keys[14].answer) != NULL)	/* BE */
			data -> error_flags		|= BE;
		if ((answer = keys[15].answer) != NULL)	/* VJT*/
			data -> vj_top = atoi(*(++answer));
		if ((answer = keys[16].answer) != NULL)	/* VCL*/
			data -> vj_line_cnt	= atoi(*(++answer));
		if ((answer = keys[17].answer) != NULL)	/* VJL*/
			data -> vj_line_adj	= atoi(*(++answer));
		if ((answer = keys[18].answer) != NULL)	/* VCP*/
			data -> vj_para_cnt	= atoi(*(++answer));
		if ((answer = keys[19].answer) != NULL)	/* VJP*/
			data -> vj_para_adj	= atoi(*(++answer));
		if ((answer = keys[20].answer) != NULL)	/* VCE*/
			data -> vj_exld_cnt	= atoi(*(++answer));
		if ((answer = keys[21].answer) != NULL)	/* VJE*/
			data -> vj_exld_adj	= atoi(*(++answer));
		if ((answer = keys[22].answer) != NULL)	/* VCB*/
			data -> vj_vbnd_cnt	= atoi(*(++answer));
		if ((answer = keys[23].answer) != NULL)	/* VJB*/
			data -> vj_vbnd_adj	= atoi(*(++answer));
		if ((answer = keys[24].answer) != NULL)	/* VJM*/
			data -> vj_mode	= atoi(*(++answer));
		if ((answer = keys[25].answer) != NULL)	/* VJD*/
			data -> vj_depth_adj = atoi(*(++answer));
		if ((answer = keys[26].answer) != NULL)	/* VST*/
			data -> vj_try = atoi(*(++answer));
	}
	p_close(fd);
	if (!map_data -> head)
		return(P_FAILURE);
	validate_map_file(wn, filename, map_data, layout_text_flag);
	return(P_SUCCESS);
}

static void map_print_fd(Pfd fd, QUEUE *map_data)
{
	char line[256];
	MAP_DATA *data;

	for(data = (MAP_DATA *)map_data -> head; 
		data; data = (MAP_DATA *)data -> que.next)
	{
		build_map_line(line, data);
		p_fputs(line, fd);
		if (trace_ff || trace_map)
			p_info(PI_TRACE, line);
	}
	if (trace_ff || trace_map)
		p_info(PI_TRACE, "\n");
}

int map_dump(WYSIWYG *wn, char *filename, QUEUE *map_data,
			 int layout_text_flag)
{
	Pfd fd;
	int key;
	LC_MEMB_REC member, *mem;
	int map_file_name = MAP_FILE;

	if(first_call)
	{
		struct stat sbuf;

		if(p_stat(DEFAULT_DIR,PENTADATA,NO_STRING,"trace_map",&sbuf))
			trace_map = 0;
		else
			trace_map = 1;
		first_call = 0;
	}
	if (layout_text_flag)
		map_file_name = LMAP_FILE;
	if (trace_ff || trace_map)
		p_info(PI_TRACE, "Dumping for %s\n", filename);
	if (!map_data -> head)
	{
		LLIST *LLmemList;

		p_unlink(wn -> tree_name, map_file_name,
				 wn -> dir_name, filename);
		if (trace_ff || trace_map)
			p_info(PI_TRACE, "File deleted\n\n");
		LLmemList = LLcreateList();
		if (lc_lock_and_read_list_file(wn -> tree_name, DESK,
									   wn -> dir_name, LC_LST_MEMBERS,
									   LLmemList, TRUE) <= 0)
		{						/* Error, list could not be opened */
			p_info(PI_ELOG, "Could not open Layout List File - (error %d)\n",
					  LcResult);
			LLclear(LLmemList);
			return(P_FAILURE);
		}
		strcpy(member.filename, filename);
		if ((key = LLfindItem(LLmemList, (LLD)&member, compare_filename)) !=
			LL_ERROR && (mem = (LC_MEMB_REC *)LLget(LLmemList, key)) != NULL)
		{
			/* Make sure MP does not use it for something else */
/*			if (mem -> flag == 0 && mem -> type == 0)
			{
*/
				mem = LLrmItemAtPos(LLmemList, key, TRUE);
				if(mem)
					p_free((char *)mem);
/*			}
*/
			lc_write_and_unlock_list_file(wn -> tree_name, DESK,
										  wn -> dir_name, LC_LST_MEMBERS,
										  LLmemList);
			LLclear(LLmemList);
			return(P_SUCCESS);
		}
		lc_write_and_unlock_list_file(wn -> tree_name, DESK,
									  wn-> dir_name,LC_LST_MEMBERS, LLmemList);
		LLclear(LLmemList);
		return(P_FAILURE);
	}
	fd = p_open(wn -> tree_name, map_file_name, wn -> dir_name, filename,"w+");
	if (fd == P_ERROR)
	{
		p_info(PI_ELOG, "Could not open %s\n", filename);
		return(P_FAILURE);
	}
	map_print_fd(fd, map_data);
	p_close(fd);
	return(P_SUCCESS);
}

void build_map_line(char line[], MAP_DATA *data)
{
	char item[80];

	line[0] = 0;
	if(data -> start_line)		/* FL */
	{
		sprintf(item, "%s: %d %d %d ", keys[0].keyword, data -> start_line,
				data -> start_forec, data -> start_fowrd);
		strcat(line, item);
	}
	if(data -> end_line)		/* LL */
	{
		sprintf(item, "%s: %d %d %d %d ", keys[1].keyword, data -> end_line,
				data -> end_forec, data -> end_fowrd, data -> underflow_depth);
		strcat(line, item);
	}
	if(data -> overflow_line)	/* OL */
	{
		sprintf(item, "%s: %d %d %d %ld ", keys[2].keyword, 
				data -> overflow_line, data -> overflow_forec,
				data -> overflow_fowrd, data -> overflow_depth);
		strcat(line, item);
	}
	if(data -> tab_split)		/* TAB */
	{
		sprintf(item, "%s: %d ", keys[28].keyword, data -> tab_split);
		strcat(line, item);
	}
	if(data -> layout_id)		/* LY */
	{
		sprintf(item, "%s: %ld ", keys[3].keyword, data -> layout_id);
		strcat(line, item);
	}
	if(data -> frame_id)		/* ID */
	{
		sprintf(item, "%s: %d ", keys[4].keyword, data -> frame_id);
		strcat(line, item);
	}
	if(data -> frame_num)		/* FR */
	{
		sprintf(item, "%s: %d ", keys[5].keyword, data -> frame_num);
		strcat(line, item);
	}
	if(data -> piece)			/* PO */
	{
		sprintf(item, "%s: %d %d ", keys[6].keyword, data -> piece, 
				data -> of_p);
		strcat(line, item);
	}
	if(data -> prev_lay_id)		/* PL */
	{
		sprintf(item, "%s: %d %d ", keys[10].keyword, data -> prev_lay_id,
				data -> prev_frame_id);
		strcat(line, item);
	}
	if(data -> next_lay_id)		/* NL */
	{
		sprintf(item, "%s: %d %d ", keys[11].keyword, data -> next_lay_id,
				data -> next_frame_id);
		strcat(line, item);
	}
	if(data -> command_flags & BF) /* BF */
	{
		sprintf(item, "%s: ", keys[7].keyword);
		strcat(line, item);
	}
	if(data -> command_flags & EF) /* EF */
	{
		sprintf(item, "%s: ", keys[8].keyword);
		strcat(line, item);
	}
	if(data -> command_flags & SF) /* SF */
	{
		sprintf(item, "%s: ", keys[9].keyword);
		strcat(line, item);
	}
	if(data -> command_flags & LF) /* LF */
	{
		sprintf(item, "%s: ", keys[27].keyword);
		strcat(line, item);
	}
	if(data -> error_flags & WE) /* WE */
	{
		sprintf(item, "%s: ", keys[12].keyword);
		strcat(line, item);
	}
	if(data -> error_flags & OE) /* OE */
	{
		sprintf(item, "%s: ", keys[13].keyword);
		strcat(line, item);
	}
	if(data -> error_flags & BE) /* BE */
	{
		sprintf(item, "%s: ", keys[14].keyword);
		strcat(line, item);
	}
	if( data -> vj_top)			/* VJT */
	{
		sprintf(item, "%s: %d ", keys[15].keyword, data -> vj_top);
		strcat(line, item);
	}
	if(data -> vj_line_cnt)		/* VCL */
	{
		sprintf(item, "%s: %d ", keys[16].keyword, data -> vj_line_cnt);
		strcat(line, item);
		if( data -> vj_line_adj) /* VJL */
		{
			sprintf(item, "%s: %d ", keys[17].keyword, data -> vj_line_adj);
			strcat(line, item);
		}
	}
	if( data -> vj_para_cnt)	/* VCP */
	{
		sprintf(item, "%s: %d ", keys[18].keyword, data -> vj_para_cnt);
		strcat(line, item);
		if( data -> vj_para_adj) /* VJP */
		{
			sprintf(item, "%s: %d ", keys[19].keyword, data -> vj_para_adj);
			strcat(line, item);
		}
	}
	if( data -> vj_exld_cnt)	/* VCE */
	{
		sprintf(item, "%s: %d ", keys[20].keyword, data -> vj_exld_cnt);
		strcat(line, item);
		if( data -> vj_exld_adj) /* VJE */
		{
			sprintf(item, "%s: %d ", keys[21].keyword, data -> vj_exld_adj);
			strcat(line, item);
		}
	}
	if( data -> vj_vbnd_cnt)	/* VCB */
	{
		sprintf(item, "%s: %d ", keys[22].keyword, data -> vj_vbnd_cnt);
		strcat(line, item);
		if( data -> vj_vbnd_adj) /* VJB */
		{
			sprintf(item, "%s: %d ", keys[23].keyword, data -> vj_vbnd_adj);
			strcat(line, item);
		}
	}
	if( data -> vj_mode)		/* VJM */
	{
		sprintf(item, "%s: %d ", keys[24].keyword, data -> vj_mode);
		strcat(line, item);
	}
	if( data -> vj_depth_adj)	/* VJD */
	{
		sprintf(item, "%s: %d ", keys[25].keyword, data -> vj_depth_adj);
		strcat(line, item);
	}							/*  */
	if( data -> vj_try)			/* VST */
	{
		sprintf(item, "%s: %d ", keys[26].keyword, data -> vj_try);
		strcat(line, item);
	}
	strcat(line, "\n");
}
