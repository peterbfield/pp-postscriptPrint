/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include <string.h>
#include "p_lib.h"
#include "list_control.h"
#include "llist.h"
#include "mem_lay.f"
#include "traces.h"

void print_member(LLD item, int index)
{
	p_info(PI_TRACE, "Trace item %d -- Index: %d, filename: %s\n",
			  index, ((LC_MEMB_REC *)item)->index, 
			  ((LC_MEMB_REC *)item)->filename);
}

void print_layout(LLD item, int index)
{
	p_info(PI_TRACE, "Trace item %d -- Flag: %d, Type: %d, Index: %d, \
filename: %s, PageNum: %s, SecPageNum: %d\n",
			  index, ((LC_LAY_REC *)item)->flag, ((LC_LAY_REC *)item)->type,
			  ((LC_LAY_REC *)item)->index, ((LC_LAY_REC *)item)->filename,
			  ((LC_LAY_REC *)item)->PageNum, ((LC_LAY_REC *)item)->SecPageNum);
}

int sort_compare_index(char *p1, char *p2)
{
	int i1 = (*(LC_ANY_REC **)p1)->index;
	int i2 = (*(LC_ANY_REC **)p2)->index;
	
	if (i1 < i2)
		return(-1);
	else if (i1 == i2)
	{
		int t1 = (*(LC_ANY_REC **)p1)->type;
		int t2 = (*(LC_ANY_REC **)p2)->type;
		
		if (t1 < t2)
			return(-1);
		else if (t1 > t2)
			return(1);
		else
		{
			char *s1 = (*(LC_LAY_REC **)p1)->PageNum;
			char *s2 = (*(LC_LAY_REC **)p2)->PageNum;
			int len1 = strlen(s1);
			int len2 = strlen(s2);
			
			return(len1 < len2 ? -1 :
				   (len1 > len2 ? 1 : strcmp(s1, s2)));
		}
	}
	else
		return(1);
}

int compare_index_and_flag(char *p1, char *p2)
{
	int i1 = (*(LC_ANY_REC **)p1)->index;
	int i2 = (*(LC_ANY_REC **)p2)->index;
	
	if (i1 == i2)
	{
		int f1 = (*(LC_ANY_REC **)p1)->flag;
		int f2 = (*(LC_ANY_REC **)p2)->flag;
		
		return (f1 < f2 ? -1 : (f1 == f2 ? 0 : 1) );
	}
	return (i1 < i2 ? -1 : (i1 == i2 ? 0 : 1) );
}

int compare_flag_and_type(char *p1, char *p2)
{
	int f1 = (*(LC_ANY_REC **)p1)->flag;
	int f2 = (*(LC_ANY_REC **)p2)->flag;
	
	if (f1 == f2)
	{
		int t1 = (*(LC_ANY_REC **)p1)->type;
		int t2 = (*(LC_ANY_REC **)p2)->type;
		
		return (t1 < t2 ? -1 : (t1 == t2 ? 0 : 1) );
	}
	return (f1 < f2 ? -1 : 1 );
}

int compare_index(char *p1, char *p2)
{
	int i1 = (*(LC_ANY_REC **)p1)->index;
	int i2 = (*(LC_ANY_REC **)p2)->index;
	
	return (i1 < i2 ? -1 : (i1 == i2 ? 0 : 1) );
}

int compare_filename(char *p1, char *p2)
{
	return (strcmp( (*(LC_ANY_REC **)p1)->filename,
					(*(LC_ANY_REC **)p2)->filename ));
}

int compare_layout_page(char *p1, char *p2)
{
	char *s1 = (*(LC_LAY_REC **)p1)->PageNum;
	char *s2 = (*(LC_LAY_REC **)p2)->PageNum;
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	int flag1 = (*(LC_LAY_REC **)p1)->flag;
	int flag2 = (*(LC_LAY_REC **)p2)->flag;
	int result;
	
	result = flag1 < flag2 ? 1 : (flag1 == flag2 ? 0 : -1);
	if(result == 0)
		result = len1 < len2 ? -1 : (len1 == len2 ? 0 : 1);
	if (result == 0)
	{
		if (strcmp(s1, NO_PAGE_NUMBER) || strcmp(s2, NO_PAGE_NUMBER))
			return (strcmp(s1, s2));
		else
			return(strcmp((*(LC_LAY_REC **)p1)->filename,
						  (*(LC_LAY_REC **)p2)->filename));
	}
	else
		return(result);
}

int compare_layout_real_page(char *p1, char *p2)
{
	char *s1 = (*(LC_LAY_REC **)p1)->PageNum;
	char *s2 = (*(LC_LAY_REC **)p2)->PageNum;
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	int flag1 = (*(LC_LAY_REC **)p1)->flag;
	int flag2 = (*(LC_LAY_REC **)p2)->flag;
	int result;
	
	result = flag1 < flag2 ? -1 : (flag1 == flag2 ? 0 : 1);
	if(result == 0)
		result = len1 < len2 ? -1 : (len1 == len2 ? 0 : 1);
	if (result == 0)
	{
		if (strcmp(s1, NO_PAGE_NUMBER) || strcmp(s2, NO_PAGE_NUMBER))
			return (strcmp(s1, s2));
		else
			return(strcmp((*(LC_LAY_REC **)p1)->filename,
						  (*(LC_LAY_REC **)p2)->filename));
	}
	else
		return(result);
}

LC_LAY_REC *find_layout_filename(LLIST *llist, char *filename)
{
	LC_LAY_REC layout1, *layout2;
	int key;
	
	strcpy(layout1.filename, filename);
	if ((key = LLfindItem(llist, (LLD)&layout1, compare_filename)) !=
		LL_ERROR && (layout2 = (LC_LAY_REC *)LLget(llist, key)) != NULL)
		return(layout2);
	else
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_layout_filename(%#x, %s) NOT FOUND\n",
					  llist, filename);
			LLtrace(llist, print_layout);
		}
		return(NULL);
	}
}

LC_LAY_REC *find_layout_page(LLIST *llist, char *page)
{
	LC_LAY_REC layout1, *layout2;
	int key;
	
	layout1.flag = DM;
	strcpy(layout1.PageNum, page);
	if ((key = LLfindItem(llist, (LLD)&layout1, compare_layout_page)) !=
		LL_ERROR && (layout2 = (LC_LAY_REC *)LLget(llist, key)) != NULL)
		return(layout2);
	else
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_layout_page(%#x, %s) NOT FOUND\n",
					  llist, page);
			LLtrace(llist, print_layout);
		}
		return(NULL);
	}
}

LC_LAY_REC *find_layout_index(LLIST *llist, int index)
{
	LC_LAY_REC layout1, *layout2;
	int key;
	
	layout1.index = index;
	if ((key = LLfindItem(llist, (LLD)&layout1, compare_index)) != LL_ERROR &&
		(layout2 = (LC_LAY_REC *)LLget(llist, key)) != NULL)
	{
		return(layout2);
	}
	else
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_layout_index(%#x, %d) NOT FOUND\n",
					  llist, index);
			LLtrace(llist, print_layout);
		}
		return(NULL);
	}
}

LC_MEMB_REC *find_member_filename(LLIST *llist, char *filename)
{
	LC_MEMB_REC member1, *member2;
	int key;
	
	strcpy(member1.filename, filename);
						/* Find filename in .member.list:  */
	if ((key = LLfindItem(llist, (LLD)&member1, compare_filename)) == LL_ERROR
		&& (!strstr(filename, ".prj")))
	{					/* Not found, but try with .prj suffix on filename:  */
		sprintf(member1.filename, "%s.prj", filename);
		key = LLfindItem(llist, (LLD)&member1, compare_filename);
	}
	if ((key == LL_ERROR) && strncmp(filename, "../", 3))
	{					/* Neither found, but try ../ prefix on filename.prj:  */
		char filenametemp[MAX_NAME+4];

		sprintf(filenametemp, "../%s", member1.filename);
		strcpy(member1.filename, filenametemp);
		key = LLfindItem(llist, (LLD)&member1, compare_filename);
	}
						/* If we found it, get pntr to full rec:  */
	if (key != LL_ERROR && (member2 = (LC_MEMB_REC *)LLget(llist, key)) != NULL)
		return(member2);
	else				/* Could not find it, return NULL.  */
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_member_filename(%#x, %s) NOT FOUND\n",
					  llist, filename);
			LLtrace(llist, print_member);
		}
		return(NULL);
	}
}

LC_MEMB_REC *find_member_index(LLIST *llist, int index)
{
	LC_MEMB_REC member1, *member2;
	int key;
	
	member1.flag = 0;			/* Look for member-textfile entries.  */
	member1.index = index;
	if ((key = LLfindItem(llist, (LLD)&member1, compare_index_and_flag)) !=
		LL_ERROR && (member2 = (LC_MEMB_REC *)LLget(llist, key)) != NULL)
		return(member2);
	else
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_member_index(%#x, %d) NOT FOUND\n",
					  llist, index);
			LLtrace(llist, print_member);
		}
		return(NULL);
	}
}

LC_MEMB_REC *find_member_type(LLIST *llist, int type)
{
	LC_MEMB_REC member1, *member2;
	int key;
	
	member1.flag = 0;			/* Look for member-textfile entries,  */
	member1.type = type;		/* of type (=pag-member-sequence) passed.  */
	if ((key = LLfindItem(llist, (LLD)&member1, compare_flag_and_type)) !=
		LL_ERROR && (member2 = (LC_MEMB_REC *)LLget(llist, key)) != NULL)
		return(member2);
	else
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_member_type(%#x, %d) NOT FOUND\n",
					  llist, type);
			LLtrace(llist, print_member);
		}
		return(NULL);
	}
}

LC_MEMB_REC *find_project_index(LLIST *llist, int index)
{
	LC_MEMB_REC member1, *member2;
	int key;
	
	member1.flag = 1;			/* Look at project entries, both members
									and parents.  */
	member1.index = index;
	if ((key = LLfindItem(llist, (LLD)&member1, compare_index_and_flag)) !=
		LL_ERROR && (member2 = (LC_MEMB_REC *)LLget(llist, key)) != NULL)
		return(member2);
	else
	{
		if (trace_lc)
		{
			p_info(PI_TRACE, "find_project_index(%#x, %d) NOT FOUND\n",
					  llist, index);
			LLtrace(llist, print_member);
		}
		return(NULL);
	}
}

int find_last_index(LLIST *llist)
{
	LC_ANY_REC *item;
	int index=0, key;
	
	for (item = (LC_ANY_REC *)LLhead(llist, &key); item;
		 item = (LC_ANY_REC *)LLnext(llist, &key))
		if (item->index > index)
			index = item->index;
	return(index);
}

LC_MEMB_REC *find_last_member_type(LLIST *llist)
{
	LC_MEMB_REC *item;
	LC_MEMB_REC *member = NULL;
	int type=0, key;
	
	for (item = (LC_MEMB_REC *)LLhead(llist, &key); item;
		 item = (LC_MEMB_REC *)LLnext(llist, &key))
		if (item->flag == 0 && item->type > type)
		{
			type = item->type;
			member = item;
		}
	return(member);
}
