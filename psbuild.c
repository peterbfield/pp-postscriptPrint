#include <stdio.h>
#include <stdlib.h>
#include "p_lib.h"
#include "frame.h"
#include "list_control.h"
#include "llist.h"
#include "mem_lay.f"
#include "rel_data.h"
#include "psjob.h"

extern void build_pspage(LC_LAY_REC *existing_entry);
extern WYSIWYG *get_page(char *TreeName, char *DirPrj, int PageNbr);

static int alloc_next_ps_piece(void);
static void trace_dp(void);

extern int next_dot_page_page;
extern LLIST *QueLayList;
extern LLIST *QueMemList;
extern LLIST *QueMemListParent;
extern int16 yx_convert[];

char LayoutName[MAX_NAME + 4];
int obj_wn_count;
int num_dot_page_recs;
int next_is_new_proj;
struct ps_pieces *PsCurRec;
struct ps_pieces *PsHead;		/* pointer to first ps_pieces structure */
struct ps_pieces *PsTail;		/* pointer to last ps_pieces structure */
struct obj_pg_wn ObjPageWn;
LC_LAY_REC *existing_ent;

/********************************************************************/

int psbuild(int page)
{
	LC_LAY_REC *next_entry, lay_rec;
	LAYOUT_DESC *lay_desc;
	char ascii_page[13];
    int lay_index, ii;
	
	itoa(page, ascii_page);
#ifdef TRACE
	if(parse_trace)
		p_info(PI_TRACE, "psbuild(job -%s, page -%s)\n", JobName,ascii_page);
#endif
	num_dot_page_recs = 0;		/* new layout, no blocks yet */
	next_dot_page_page = -1;
	if (BookPrint)
		strcpy(SubDirName, NextDirName);
	if ( MasterNameFlag)
	{							/* get a master layout */
		existing_ent = find_layout_filename(QueLayList, PageName);
		if (!existing_ent)
			return(1);
	}
	else
	{					/* Get layout name that corresponds to this page  */
		if (BookPrint && next_is_new_proj)
		{				/* But first, since this is first page of a new child
							project in whole-book output, clear and re-fill the
							layout and member lists for the new project.  */
			if (QueLayList)
				LLclearItems(QueLayList);
			else
				QueLayList = LLcreateList();
   			if (lc_lock_and_read_list_file(TreeName, DESK, NextDirName,
						LC_LST_LAYOUTS, QueLayList, FALSE) == 0)
   			{
				p_info(PI_ELOG, "OUTPUT failed to get layout list for %s/%s\n",
		   			TreeName, NextDirName);
				return(1);
   			}
			LLSORTED(QueLayList) = FALSE;
			LLsort(QueLayList, compare_layout_real_page); /* sort lays by pg nbr */

    		if (QueMemList)
				LLclearItems(QueMemList);
    		else
				QueMemList = LLcreateList();
    		if (lc_lock_and_read_list_file(TreeName, DESK, NextDirName,
						LC_LST_MEMBERS, QueMemList, FALSE) == 0)
    		{
				p_info(PI_ELOG, "OUTPUT failed to get members list for %s/%s\n",
			   		TreeName, NextDirName);
				return(1);
    		}
		}				/* (end new child project setup)  */
        memset((char *)&lay_rec,0,sizeof(lay_rec));
        strcpy(lay_rec.PageNum, ascii_page);
        if((lay_index = LLfindItem(QueLayList, (LLD)&lay_rec,
                                   compare_layout_real_page)) == LL_ERROR )
        {
            p_info(PI_ELOG, "psbuild, failed to find page %s\n",
                   lay_rec.PageNum);
            return(1);
        }
        if((existing_ent = (LC_LAY_REC *)LLget(QueLayList,lay_index))
           == NULL)
        {
            p_info(PI_ELOG, "psbuild, failed to return lay rec %d\n", lay_index);
            return(2);
        }
	}
/* allocate the memory for the wn structure for the page */
	wn = (WYSIWYG *)p_alloc(sizeof(WYSIWYG));
	lay_desc = (LAYOUT_DESC *)p_alloc(sizeof(LAYOUT_DESC));
	QinsertTail(&wn -> layout_list, (QE *)lay_desc);
	wn -> selected_lay = lay_desc;
	wn -> yx_convert[0] = yx_convert[0] = 5400 / VerticalBase;
	wn -> yx_convert[1] = yx_convert[1] = 5400 / HorizontalBase;
	wn -> msb = (hmu)(HorizontalBase);
	wn -> ldb = (vmu)(VerticalBase);
	wn -> mb = wn -> msb * 100;
	wn -> mr = wn -> mb >> 1;
	wn -> lb = wn -> ldb * 100;
	wn -> lr = wn -> lb >> 1;
	wn -> izt = 100;
#ifdef TRACE
	if(parse_trace)
		p_info(PI_TRACE, "psbuild yx[0]=%d, yx[1]=%d, msb=%ld, ldb=%ld\n",
			   wn->yx_convert[0], wn->yx_convert[1], wn->msb, wn->ldb);
#endif
    strcpy(wn -> tree_name, TreeName);
    strcpy(wn -> dir_name, SubDirName);
	
/* read in the layout */
	if(frame_rd(wn, LAYOUT_FILE, existing_ent -> filename))
	{
		p_info(PI_ELOG, "psbuild failed to read layout %s\n",
			   existing_ent -> filename);
		return(3);
	}
	memset(LayoutName, 0, sizeof(LayoutName));
	strcpy(LayoutName, existing_ent -> filename);
	NEW_TOT_BLOCKS = TOT_BLOCKS; /* set number of frames */
/* Make interface believe that the graphic is loaded */
	for (ii = 1; ii <= NEW_TOT_BLOCKS; ii++)
		if (TYPE_OF_FRAME(ii) == PL_GRAPHIC)
			FRAME_DATA(ii) gr = (GRAPHIC_IMAGE *)-1;
	lmt_interface(wn);
/* Reset to zero so we do not try to free it ... */
	for (ii = 1; ii <= NEW_TOT_BLOCKS; ii++)
		if (TYPE_OF_FRAME(ii) == PL_GRAPHIC)
			FRAME_DATA(ii) gr = NULL;
	lmt_page_elements(wn);		/* create element list */
	scan_map(existing_ent -> index); /* scan in map data */
	PageNo1 = p_page_atoi(existing_ent -> PageNum);
	PageNo2 =0;
	PageNo3 = 0;
	if ((existing_ent -> SecPageNum) > 0)
	{
		ChapterPage = existing_ent -> SecPageNum;
		ChapterPageSetFlag++;
	}
	else if ( ChapterPage && !ChapterPageSetFlag)
	{
		ChapterPage -= PageNo1;
		ChapterPageSetFlag++;
	}
	build_pspage(existing_ent);
	next_is_new_proj = 0;		/* (assume putting out pgs in single project) */
	if ( !MasterNameFlag)
	{
			 /* This logic requires that all flag=0 page entries be consecutive
				in the .layout2.list.  At end of list, or when flag!=0, 
				next_dot_page_page will be left at -1 indicating end of unit. */
		if((next_entry = (LC_LAY_REC *)LLnext(QueLayList, &lay_index)))
			if(next_entry -> flag == 0)	
				next_dot_page_page = p_page_atoi(next_entry -> PageNum);

			/* If did last page, check for whole-book output (multiple chapters).
				If so, find next project name in parent's member list, open 
				the Layout list there, get to its first page number.  */
		if (next_dot_page_page == -1)
		{
			while (QueMemListParent && (ParentIndex < LastParentIndex))
			{
				LC_MEMB_REC *mem_rec;
				LLIST *QueLayListTmp;
				char *nmptr;

				if ((mem_rec = find_project_index(QueMemListParent, ++ParentIndex))
					== NULL)
				{
					p_info(PI_ELOG, "psbuild could not find child project %d\n",
			   			ParentIndex);
					return(0);
				}
				if ((nmptr = strchr(mem_rec->filename, '/')))
					++nmptr;
				else
					nmptr = mem_rec->filename;
				strcpy (NextDirName, nmptr);	/* Next project dir for pages */
				QueLayListTmp = LLcreateList();
    			if(lc_lock_and_read_list_file(TreeName, DESK, NextDirName,
								  LC_LST_LAYOUTS, QueLayListTmp, FALSE) == 0)
    			{
					p_info(PI_ELOG, "OUTPUT failed to get layout list for %s/%s\n",
			   			TreeName,NextDirName);
					return(1);
    			}
				LLSORTED(QueLayListTmp) = FALSE;
												/* Sort lays by pg nbr:  */
				LLsort(QueLayListTmp, compare_layout_real_page);
				next_entry = (LC_LAY_REC *)LLhead (QueLayListTmp, &lay_index);
				while (next_entry && next_entry->flag==0 &&
					   (atoi(next_entry->PageNum) < 1))
					next_entry = (LC_LAY_REC *)LLnext(QueLayListTmp, &lay_index);
				if (!next_entry || next_entry -> flag > 0)	
				{
					p_info(PI_ELOG, "OUTPUT: layout list for %s/%s has no MP pages.\n",
			   			TreeName,NextDirName);
					continue;
				}
				next_dot_page_page = p_page_atoi(next_entry -> PageNum);
				next_is_new_proj = 1;	/* Flag: Change to new proj at start
											of next page.  */
				LLclear(QueLayListTmp);
				break;
			}
		}
#ifdef TRACE
	if(parse_trace)
		p_info(PI_TRACE, "psbuild, ndpp = %d\n",
			   next_dot_page_page);
#endif
	}	
	return(0);
}				/* end psbuild() */

/*********************************************************************/
/*
 * Filter all .lay recs in this page into PsCurRecs of type 'struct ps_pieces'.
 */
void build_pspage(LC_LAY_REC *existing_entry)
{
	WYSIWYG *save_frame_wn;
	ELEMENT *ele, *ele_obj;
	int ii, jj, kk, ij;
	int obj_frame, obj_lay;

	crop_flag = 0;
	PsHead = 0;
	PsTail = 0;
	if (alloc_next_ps_piece())
		return;
	PsCurRec -> frame_type = 1; /* new page */
	PsCurRec -> ele_nbr = 0;
	PsCurRec -> elem = 0;
	PsCurRec -> orig_elem = 0;
	PsCurRec -> frame_nbr = 0;
	PsCurRec -> orig_frame_nbr = 0;
	PsCurRec -> frame_wn = wn;
	PsCurRec -> text_wn = wn;
	ObjPageWn.page_number[0] = PageNo1;
	ObjPageWn.wns[0] = wn;
	obj_wn_count = 1;
	if ( (KeyTrimFlags & 3) && !LYPrintFlag )
    {							/* set up crop marks */
		if (alloc_next_ps_piece())
			return;
		PsCurRec -> frame_type = 3; /* crop marks */
		PsCurRec -> ele_nbr = 0;
		PsCurRec -> elem = 0;
		PsCurRec -> orig_elem = 0;
		PsCurRec -> frame_nbr = 0;
		PsCurRec -> orig_frame_nbr = 0;
		PsCurRec -> frame_wn = wn;
		PsCurRec -> text_wn = wn;
	}
	for(ii = 1; ii <= LAYOUT(wn) num_frames; ii++)
	{
		switch (TYPE_OF_FRAME(ii))
		{
		  case PL_GRAPHIC:
			if(GRAPHIC_NAME(ii) == 0)
			{
#ifdef TRACE
				if(parse_trace)
					p_info(PI_TRACE, "PS_BUILD no graphic name in %s\n",
						   existing_entry -> filename);
#endif
				break;
			}
			if (alloc_next_ps_piece())
				return;
			PsCurRec -> frame_type = 8; /* graphic */
			PsCurRec -> ele_nbr = 1;
			PsCurRec -> elem = FRAME_DATA(ii) ele;
			PsCurRec -> orig_elem = FRAME_DATA(ii) ele;
			PsCurRec -> frame_nbr = ii;
			PsCurRec -> orig_frame_nbr = ii;
			PsCurRec -> frame_wn = wn;
			PsCurRec -> text_wn = wn;
			break;

		  case PL_RBX:
			for(jj = 1, ele = FRAME_DATA(ii) ele; ele; jj++, ele = ele -> next)
			{
				if (alloc_next_ps_piece())
					return;
				PsCurRec -> frame_type = 4; /* rule/box */
				PsCurRec -> ele_nbr = jj;
				PsCurRec -> elem = ele;
				PsCurRec -> orig_elem = ele;
				PsCurRec -> frame_nbr = ii;
				PsCurRec -> orig_frame_nbr = ii;
				PsCurRec -> frame_wn = wn;
				PsCurRec -> text_wn = wn;
			}					/* end for all elements in frame */
			break;

		  case PL_TEXT:
		  case PL_FLOW:
			for(jj = 1, ele = FRAME_DATA(ii) ele; ele; jj++, ele = ele -> next)
			{
				if (alloc_next_ps_piece())
					return;
				PsCurRec -> frame_type = 7;			/* Text frame  */
				PsCurRec -> ele_nbr = jj;
				PsCurRec -> elem = ele;
				PsCurRec -> orig_elem = ele;
				PsCurRec -> frame_nbr = ii;
				PsCurRec -> orig_frame_nbr = ii;
				PsCurRec -> frame_wn = wn;
				PsCurRec -> text_wn = wn;
				save_frame_wn = wn;
				if ( OBJ_REF(ii) )
				{				/* this frame is an object */
					obj_frame = OBJ_REF(ii) & MP_OB_FRAME; /* this is UID */
					obj_lay = (OBJ_REF(ii) >> 12) & MP_OB_LAY;
					for ( ij=0; ij < obj_wn_count; ij++)
					{			/* look for page already set up */
						if ( obj_lay == ObjPageWn.page_number[ij] )
							break;
					}
					if ( ij < obj_wn_count)
						wn = ObjPageWn.wns[ij]; /* already read the page */
					else if ( obj_wn_count >= 11 )
					{			/* error, lacking room for more object */
						p_info(PI_WLOG, "Warning - no memory for Object: page %d, frame %d.\n",
							   obj_lay, obj_frame);
						OBJ_REF(ii) = 0; /* kill the object reference */
						ARTICLE_ID(ii) = 0;	/* make sure no text */
						break;
					}
					else
					{
						if ( !(wn = get_page(TreeName, SubDirName, obj_lay)) )
						{		/* no such page */
							p_info(PI_WLOG, "Warning - missing Object for page %d.\n",
								   obj_lay);
							wn = save_frame_wn;
							OBJ_REF(ii) = 0; /* kill the object reference */
							ARTICLE_ID(ii) = 0;	/* make sure no text */
							break;
						}
						ObjPageWn.page_number[obj_wn_count] = obj_lay;
						ObjPageWn.wns[obj_wn_count] = wn;
						obj_wn_count++;
					}

/* convert from UID to frame number */
					for (kk=1; kk<=LAYOUT(wn) num_frames; kk++)
					{
						if ( FRAME_DATA(kk) frame_uid == obj_frame)
						{
							obj_frame = kk;
							break;
						}
					}
					if ( kk > LAYOUT(wn) num_frames )
					{			/* missing frame */
						wn = save_frame_wn;
						OBJ_REF(ii) = 0; /* kill the object reference */
						ARTICLE_ID(ii) = 0;	/* make sure no text */
						break;
					}
					for(kk = 1, ele_obj = FRAME_DATA(obj_frame) ele;
						ele_obj; kk++, ele_obj = ele_obj -> next)
					{			/* set up all the elements in object frame */
						if ( kk != 1)
						{		/* first piece already allocated */
							if (alloc_next_ps_piece())
								return;
						}
						PsCurRec -> frame_type = 7; /* text */
						PsCurRec -> ele_nbr = kk;
						PsCurRec -> elem = ele_obj;
						PsCurRec -> orig_elem = ele;
						PsCurRec -> frame_nbr = obj_frame;
						PsCurRec -> orig_frame_nbr = ii;
						PsCurRec -> frame_wn = save_frame_wn;
						PsCurRec -> text_wn = wn;
					}			/* end for(kk=1,ele_obj=FRAME...le_obj->next */
					if ( kk == 1 )
					{			/* error, never got any elements */
						p_info(PI_WLOG, "Warning - missing Object: page %d, frame %d\n",
							   obj_lay, obj_frame);
						wn = save_frame_wn;
						OBJ_REF(ii) = 0; /* kill the object reference */
						ARTICLE_ID(ii) = 0;	/* make sure no text */
					}
				}				/* end if(OBJ_REF(ii)) */
				else if(ele ->map_data.overflow_line)
				{
					if (alloc_next_ps_piece())
						return;
					PsCurRec -> frame_type = 6; /* overflow */
					PsCurRec -> ele_nbr = jj;
					PsCurRec -> elem = ele;
					PsCurRec -> orig_elem = ele;
					PsCurRec -> frame_nbr = ii;
					PsCurRec -> orig_frame_nbr = ii;
					PsCurRec -> frame_wn = wn;
					PsCurRec -> text_wn = wn;
				}				/* end if overflow line */
				wn = save_frame_wn;
			}					/* end for all elements in frame */
			break;
		}						/* end switch frame type */
	}							/* end for all frames in layout */
	if (alloc_next_ps_piece())
		return;
	PsCurRec -> frame_type = 2; /* end page */
	PsCurRec -> ele_nbr = 0;
	PsCurRec -> elem = 0;
	PsCurRec -> orig_elem = 0;
	PsCurRec -> frame_nbr = 0;
	PsCurRec -> frame_wn = wn;
	PsCurRec -> text_wn = wn;
	PsCurRec -> next = 0;
#ifdef TRACE
	if(parse_trace)
		trace_dp();
#endif
}								/* end function */

/*********************************************************************/
/* called by others to get a filename */

char *get_ps_fname(int article_id)
{
	LC_MEMB_REC *mem_rec;
	
	if (article_id == 0)
	{
#ifdef TRACE
		if (parse_trace)
			p_info(PI_TRACE, "get_ps_fname could not find file for article %d\n",
				   article_id);
#endif
		return((char *)NULL);
	}
	QueMemList -> Sorted = 0; /* Set to false, to force sort of QueMemList before
								 find. Experience shows it can have out-of-order 
								 entries here, causing find to fail.  */
	if((mem_rec = find_member_index(QueMemList, article_id)) == NULL)
	{
		p_info(PI_ELOG, "get_ps_fname could not find file for article %d\n",
			   article_id);
		return((char *)NULL);
	}
	return(mem_rec -> filename);
}

/*********************************************************************/

void scan_map(int layout_num)
{
	ELEMENT *ele;
	LC_MEMB_REC *memb;
	MAP_DATA *map_data, *next_map_data;
	QUEUE Qmap;
	int key;
	int ii;
	int previous_article_id = 0;
	
#ifdef TRACE
	if(parse_trace)
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
				if (((int)TYPE_OF_FRAME(ii) == PL_TEXT ||
					 map_data -> layout_id == layout_num) &&  /* (FLOW frame) */
					(map_data -> frame_num == ele -> map_data.frame_num) &&
					(map_data -> frame_id == ele -> map_data.frame_id) &&
					(map_data -> piece == ele -> map_data.piece))
				{
						memcpy((char *)&ele -> map_data,
							   (char *)map_data, sizeof(MAP_DATA));
						QremoveElement(&Qmap, (QE *)map_data);
						p_free((char *)map_data);
						break;
				}
				map_data = next_map_data;
			}				/* end while(map_data) */
		}					/* end for all elements of frame */
	}						/* end for all frames in layout */
	Qclear(&Qmap);
}

/*********************************************************************/

static void trace_dp(void)
{
	struct ps_pieces *head;

	p_info(PI_TRACE, "PSBUILD page file elements:\n");
	if (!PsHead)
	{
		p_info(PI_TRACE, "NOTHING BUILT\n");
		return;
	}
	head = PsHead;
	while ( head)
	{
		trace_idtape(head);
		head = head -> next;
	}							/* end while(head) */
	return;
}

/*********************************************************************/

void set_layout_process_flag(LAYOUT_DESC *layout)
{								/* dummy function for link */
}

/*********************************************************************/

static int alloc_next_ps_piece(void)
{
    PsCurRec = (struct ps_pieces *) p_alloc (sizeof (struct ps_pieces));
	if ( !PsCurRec )
	{
		p_info(PI_ELOG, "Allocate failed for ps_pieces structure\n");
		return(1);
	}
	PsCurRec -> next = 0;
	if ( !PsHead )
		PsHead = PsCurRec;
    else
		PsTail -> next = PsCurRec; /* prev struct points to this new one */
	PsTail = PsCurRec;
	num_dot_page_recs++;
	return(0);
}								/* end function */

/************** EOF ****************/
