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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "window.h"
#include "rel_data.h"
#include "traces.h"
#include "lpm.h"
#include "list_control.h"
#include "mem_lay.f"
#include "lmt.f"
#include "frame.h"
#include "llist.h"
#include "map.f"

typedef struct m_list {
	char *member_name;
	int lock_id;				/* id number when file is locked */
	struct	m_list *nxt;		/* pointer to next member */
}M_LIST;

extern int frame_rd(WYSIWYG *wn, int dir_type, char *filename);

static char UserdataPath[132];
static void clear_mlist(M_LIST **lst);
static int add_member(char *member, M_LIST **list);
static void get_page_scan_map(WYSIWYG *wn, int layout_num, char *LayoutName,
							  LLIST *QueMemList);
static int get_page_debugger_trace = 0;

WYSIWYG *get_page(char *TreeName, char *DirPrj, int PageNbr)
{

	WYSIWYG *wn;
	LAYOUT_DESC *lay_desc;
	LC_LAY_REC *existing_entry, lay_rec;
	LC_MEMB_REC *mem_rec;
	Pfd sysfd;
    int16 stds_buff[240];
	int index;
	int article_id;
    int lay_index, i;
	int lay_page_no;
	int16 HorizontalBase, VerticalBase;
	int16 yx_convert[2] = {1350,100};
	LLIST *QueLayList = NULL;
	LLIST *QueMemList = NULL;
	LC_LAY_REC *Lay = 0;
	M_LIST *MemList = 0;		/* galley text members */
	M_LIST *DesignList = 0;		/* design text members */
	M_LIST *MemListFirst = 0;	/* galley text members */
	M_LIST *DesignListFirst = 0; /* design text members */

#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "start get_page for page # %d\n",PageNbr);
#endif
	wn = 0;
	lc_init(1);

/* get the layout list */

	QueLayList = LLcreateList();
	if(lc_lock_and_read_list_file(TreeName, DESK, DirPrj,
								  LC_LST_LAYOUTS, QueLayList, FALSE) <= 0)
	{
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "get_page Could not read layout list\n");
#endif
		return (0);			/* Could not read layout list */
	}
	LLSORTED(QueLayList) = FALSE;
	LLsort(QueLayList, compare_layout_real_page); /*put names in page order,
													use PageNum string
													for sort */
	if ( !p_get_data_name ( TreeName, DirPrj, UserdataPath, 0 ) )
	{
		sysfd = p_open(TreeName, USERDATA, UserdataPath, "standards", "r");
		p_read((char *)stds_buff, 240, 1, sysfd, 0, BS16);
		p_close(sysfd);
		HorizontalBase = stds_buff[35];
		VerticalBase = stds_buff[74];
	}
	else
	{
		HorizontalBase = 20;
		VerticalBase = 10;
	}
	yx_convert[0] = 5400 / VerticalBase;
	yx_convert[1] = 5400 / HorizontalBase;
	wn = (WYSIWYG *)p_alloc(sizeof(WYSIWYG));
	wn -> yx_convert[0] = yx_convert[0];
	wn -> yx_convert[1] = yx_convert[1];
	wn -> msb = (hmu)(HorizontalBase);
	wn -> ldb = (vmu)(VerticalBase);
	wn -> mb = wn -> msb * 100;
	wn -> mr = wn -> mb >> 1;
	wn -> lb = wn -> ldb * 100;
	wn -> lr = wn -> lb >> 1;
	wn -> izt = 100;
    strcpy(wn -> tree_name, TreeName);
    strcpy(wn -> dir_name, DirPrj);
	clean_wysiwyg(wn);
	QueMemList = LLcreateList();
	if( lc_lock_and_read_list_file(TreeName, DESK, DirPrj, LC_LST_MEMBERS,
								   QueMemList, FALSE) <= 0)
	{
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "get_page Could not read member list\n");
#endif
		return (0);				/* Could not read member list */
	}
#ifdef TRACE
	if (get_page_debugger_trace)
		LLtrace(QueMemList, print_member);
#endif
	for(Lay = LLhead(QueLayList, &index);
		Lay;
		Lay = LLnext(QueLayList, &index))
	{
		if ( Lay -> flag)
		{						/* not a page layout */
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "NO PAGE - filename '%s \n",Lay->filename);
#endif
			continue;			/* not a page layout */
		}
#ifdef TRACE
		if (get_page_debugger_trace)
		{
			LLtrace(QueLayList, print_layout);
			p_info(PI_TRACE, "make  filename '%s', page '%s' \n",
					Lay->filename, Lay->PageNum);
		}
#endif
		lay_page_no = p_page_atoi(Lay -> PageNum);
		if ( PageNbr != lay_page_no) 
		{
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "No match. Want page %d, is page %d\n",
						  PageNbr,lay_page_no);
#endif
			continue;
		}
		break;					/* found the needed page */
	}							/* end layout for page */
#ifdef TRACE
	if (get_page_debugger_trace)
		p_info(PI_TRACE, "get_page found page %d\n", PageNbr);
#endif
	memset((char *)&lay_rec,0,sizeof(lay_rec));
	strcpy(lay_rec.PageNum, Lay -> PageNum);
	if((lay_index = LLfindItem(QueLayList, (LLD)&lay_rec,
							   compare_layout_real_page)) == LL_ERROR )
	{
#ifdef TRACE
		if (get_page_debugger_trace)
			p_info(PI_TRACE, "get_page failed to find page\n");
#endif
		return (0);				/* Failed to find page. */
	}
	if((existing_entry = (LC_LAY_REC *)LLget(QueLayList, lay_index))
	   == NULL)
	{
#ifdef TRACE
		if (get_page_debugger_trace)
			p_info(PI_TRACE, "get_page failed to find layout record\n");
#endif
		return (0);				/* Failed to find layout record. */
	}
	
/* allocate the memory for the wn structure for the page */

	wn = (WYSIWYG *)p_alloc(sizeof(WYSIWYG));
	lay_desc = (LAYOUT_DESC *)p_alloc(sizeof(LAYOUT_DESC));
	QinsertTail(&wn -> layout_list, (QE *)lay_desc);
	wn -> selected_lay = lay_desc;
	wn -> yx_convert[0] = yx_convert[0];
	wn -> yx_convert[1] = yx_convert[1];
	wn -> msb = (hmu)(HorizontalBase);
	wn -> ldb = (vmu)(VerticalBase);
	wn -> mb = wn -> msb * 100;
	wn -> mr = wn -> mb >> 1;
	wn -> lb = wn -> ldb * 100;
	wn -> lr = wn -> lb >> 1;
	wn -> izt = 100;
	strcpy(wn -> tree_name, TreeName);
	strcpy(wn -> dir_name, DirPrj);

/* read in the layout */

	if(frame_rd(wn, LAYOUT_FILE, existing_entry -> filename))
	{
#ifdef TRACE
		if (get_page_debugger_trace)
			p_info(PI_TRACE, "get_page->frame_rd failed to read layout\n");
#endif
		return (0);			/* Failed to read the layout. */
	}
	NEW_TOT_BLOCKS = TOT_BLOCKS; /* set number of frames */
/* Make interface believe that the graphic is loaded */
	for (i=1; i<=NEW_TOT_BLOCKS; i++)
		if (TYPE_OF_FRAME(i) == PL_GRAPHIC)
			FRAME_DATA(i) gr = (GRAPHIC_IMAGE *)-1;
	lmt_interface(wn);
	lmt_page_elements(wn);		/* create element list */
	get_page_scan_map(wn, existing_entry -> index,
					  existing_entry -> filename,
					  QueMemList); /* scan in map data */
/* Reset to zero so we do not try to free it, get members names ... */
	for (i=1; i<=NEW_TOT_BLOCKS; i++)
	{
		switch (TYPE_OF_FRAME(i))
		{
		  case PL_GRAPHIC:
			FRAME_DATA(i) gr = NULL;
			break;
			
		  case PL_TEXT:			/* design text file */
			if ( (REL_DATA(i) HNJ_T[0]) == 0 )
			{
#ifdef TRACE
				if (get_page_debugger_trace)
					p_info(PI_TRACE, "design text, no filename\n",
							  existing_entry->filename);
#endif
				break;
			}
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "DESIGN filename '%s'\n",
						  existing_entry -> filename);
#endif
			add_member(existing_entry -> filename, &DesignList);
			if ( !DesignListFirst)
				DesignListFirst = DesignList;
			break;
			
		  case PL_FLOW:			/* galley file */
			if ( (article_id = ARTICLE_ID(i)) == 0)
			{
#ifdef TRACE
				if (get_page_debugger_trace)
					p_info(PI_TRACE, "frame %d, galley text with, no article\n", 
							  i);
#endif
				break;
			}
			if((mem_rec = find_member_index(QueMemList, article_id))
			   == NULL)
			{
#ifdef TRACE
				if (get_page_debugger_trace)
					p_info(PI_TRACE, "galley text, MP data, no index\n");
#endif
				return (0);		/* Could not find file for article. */
			}
			if ( mem_rec -> flag )
			{					/* not a galley record, is MP data */
#ifdef TRACE
				if (get_page_debugger_trace)
					p_info(PI_TRACE, "galley text, MP data, no filename\n");
#endif
				break;
			}
			
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "GALLEY filename '%s'\n", mem_rec -> filename);
#endif
			add_member(mem_rec -> filename, &MemList);
			if ( !MemListFirst)
				MemListFirst = MemList;
			break;
		}						/* end switch frame type */
	}							/* end for all frames in layout, now we have
								   the complete member list */
#ifdef TRACE
	if (get_page_debugger_trace)
		p_info(PI_TRACE, "ending get_page\n");
#endif
	clear_mlist(&DesignList);
	clear_mlist(&MemList);
	if ( QueLayList)
		LLclear(QueLayList);
	if ( QueMemList)
		LLclear(QueMemList);
	if ( !wn)
		return (0)				/* never found the page */;
	return (wn);
}								/* end function */

/***********************************************************************/

void clear_mlist(M_LIST **lst)	/* clear list of members */
{
	M_LIST *vp = *lst;			/* list walker */
	M_LIST *np;					/* next entry pointer */
	
	while(vp != NULL)			/* walk towards the end */
	{
		if(vp->member_name)		/* if it looks valid... */
			p_free(vp->member_name); /* free name space */
		np =  vp->nxt;			/* save location of next one */
		p_free( (char *)vp);	/* free the memory */
		vp = np;				/* and point to 'next' */
	}
	*lst = 0;					/* clear first list entry too */
}								/* end function */

/***********************************************************************/

int add_member(char *member, M_LIST **list)
{
	M_LIST *vp = *list;			/* list-walking pointer */
	M_LIST **lp = list;			/* pointer to last link */
	
	
	while(vp != NULL)			/* we're looking for the end */
	{
		if(strcmp(vp->member_name, member) == 0) /* found our member? */
		{
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "Found member '%s' in member list\n", member);
#endif
			return(TRUE);
		}
		lp = &vp->nxt;			/* save address of last link */
		vp = vp->nxt;			/* move to next one */
	}
	vp = *lp = (M_LIST *) p_alloc(sizeof(M_LIST));
/* allocate & link an entry	*/
	if(vp != NULL)				/* and if it's good */
	{
		if((vp->member_name = p_alloc(strlen(member) +1)) ==0)
		{
			*lp = 0;			/* ignore this trial */
			p_free ( (char *)vp); /* discard the useless entry */
			p_info(PI_ELOG, "add-member: No memory to add file '%s' to list\n",
				  member);
			return(-1);			/* No memory to add file name to list. */
		}
/* fill the data in */
		strcpy(vp->member_name, member);
		vp->lock_id = 0;		/* not locked at this time */
		vp->nxt = NULL;			/* and re-terminate the list */
#ifdef TRACE
			if (get_page_debugger_trace)
				p_info(PI_TRACE, "Added member '%s' to end of member list\n", member);
#endif
	}
	else
	{
		p_info(PI_ELOG, "add-member: No memory to add file '%s' in member list\n",
				  member);
		return(-2);
	}
	return(0);
}								/* end function */

/*********************************************************************/

static void get_page_scan_map(WYSIWYG *wn, int layout_num, char *LayoutName,
							  LLIST *QueMemList)
{
	ELEMENT *ele;
	LC_MEMB_REC *memb;
	MAP_DATA *map_data, *next_map_data;
	QUEUE Qmap;
	int key;
	int ii;
	int previous_article_id = 0;
	
#ifdef TRACE
	if(get_page_debugger_trace)
		p_info(PI_TRACE, "scan_map, layout num = %d\n", layout_num);
#endif
	
	Qmap.head = Qmap.tail = NULL;
	for(ii = 1; ii <= LAYOUT(wn) num_frames; ii++)
	{						/* article id needed */
		if ((int)TYPE_OF_FRAME(ii) == PL_TEXT)
		{					/* text from layout */
			previous_article_id = 0;
			Qclear(&Qmap);
			if (map_load(wn, LayoutName, &Qmap, 1) == P_FAILURE)
				continue;
		}
		else
		{
			if (((int)TYPE_OF_FRAME(ii) != PL_FLOW))
				continue;
			for(memb = (LC_MEMB_REC *)LLhead(QueMemList, &key);
				memb;
				memb = (LC_MEMB_REC *)LLnext(QueMemList, &key))
				if ((memb -> index == (int)ARTICLE_ID(ii)) &&
					(memb -> flag == 0))	/* (flag!=0 is not a member article) */
					break;
			if (!memb ||(memb -> index != (int)ARTICLE_ID(ii)))
				continue;
			if(previous_article_id != memb -> index)
			{					/* normal frame */
				previous_article_id = memb -> index;
				Qclear(&Qmap);
				map_load(wn, memb -> filename, &Qmap, 0);
			}
		}
		for(ele = FRAME_DATA(ii) ele; ele; ele = ele -> next)
		{
			map_data = (MAP_DATA *)Qmap.head;
			while(map_data)
			{
				next_map_data = (MAP_DATA *)map_data -> que.next;
				if (((int)TYPE_OF_FRAME(ii) == PL_TEXT))
				{			/* text from layout */
					if((map_data -> frame_num ==
						ele -> map_data.frame_num) &&
					   (map_data -> frame_id ==
						ele -> map_data.frame_id) &&
					   (map_data -> piece == ele -> map_data.piece))
					{
						memcpy((char *)&ele -> map_data,
							   (char *)map_data, sizeof(MAP_DATA));
						QremoveElement(&Qmap, (QE *)map_data);
						p_free((char *)map_data);
						break;
					}
				}
				else
				{				/* PL_FLOW text */
					if((map_data -> layout_id == layout_num) &&
					   (map_data -> frame_num ==
						ele -> map_data.frame_num) &&
					   (map_data -> frame_id ==
						ele -> map_data.frame_id) &&
					   (map_data -> piece == ele -> map_data.piece))
					{
						memcpy((char *)&ele -> map_data,
							   (char *)map_data, sizeof(MAP_DATA));
						QremoveElement(&Qmap, (QE *)map_data);
						p_free((char *)map_data);
						break;
					}
				}
				map_data = next_map_data;
			}				/* end while(map_data) */
		}					/* end for all elements of frame */
	}						/* end for all frames in layout */
	Qclear(&Qmap);
}

/***********************************************************************/
