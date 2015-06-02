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
#include "p_lib.h"
#include "llist.h"
#include "traces.h"

#define LL_INCREMENT 20

void LLrmItem(LLIST *llist, LLD item, BOOLEAN MaintainOrder, int (*compare)())
{
	int i, index=LL_ERROR;

	if (compare)
		index = LLfindItem(llist, item, compare);
	/* Make sure we found the correct one before we delete it */
	if (llist -> data[index] != item)
	{
		index = LL_ERROR;
		for (i = 0; i < llist -> count; i++)
			if (llist -> data[i] == item)
				index = i;
	}
	LLrmItemAtPos(llist, index, MaintainOrder);
}

void LLclearItems(LLIST *llist)
{
	int i;

	if (llist && llist -> count)
	{
		for (i = 0; i < llist -> count; i++)
			if (llist -> data[i])
				p_free(llist -> data[i]);
		llist -> count = 0;
		llist -> size = 0;
		llist -> Sorted = FALSE;
		p_free((char *)llist -> data);
		llist -> data = NULL;
	}
}

void LLclearList(LLIST *llist)
{
	if (llist)
	{
		if (llist -> data)
			p_free((char *)llist -> data);
		p_free((char *)llist);
	}
}

void LLclear(LLIST *llist)
{
	LLclearItems(llist);
	LLclearList(llist);
}

/* Linked List Removal Routines */
LLD LLrmItemAtPos(LLIST *llist, int position, BOOLEAN MaintainOrder)
{
	LLD user_data;
	
	if (trace_misc)
		p_info(PI_TRACE, "in LLrmItemAtPos(%#x, %d, %d) - cnt %d)\n",
				  llist, position, MaintainOrder, llist -> count);
	
	/* Do any elements exist in list */
	if (llist -> count == 0) return(NULL);
	
	/* Save ptr to data at position */
	user_data = llist -> data[position];
	
	/* Is item in our list? */
	if (position > llist -> count - 1 || position < 0)
	{
		if (trace_misc)
			p_info(PI_TRACE, "in LLrmItemAtPos(%#x, **BAD POSITION** %d) \
- cnt %d)\n", llist, position, llist -> count);
		return(NULL);
	}
	
	/* Is Item at tail of list? */
	if (position == llist -> count - 1)
	{
		llist -> data[--llist -> count] = NULL;
		return(user_data);
	}		
	
	/* If any elements exist after ours, bump `em down */
	if (--llist -> count > position)
	{
		if (MaintainOrder)
			/* Bump each entry down one */
		{
			memcpy((char *)&llist -> data[position],
				   (char *)&llist -> data[position+1], 
				   (llist -> count-position) * sizeof(LLD));
		}
		else
		{
			llist -> data[position] = llist -> data[llist -> count];
			llist -> Sorted = False;
		}	
	}
	/* Clear out element which was at end of list,
	   but now bumped down */
	llist -> data[llist -> count] = NULL;
	
	return(user_data);
}

LLIST *LLcreateList()
{
	LLIST *llist;
	
	llist = (LLIST *)p_alloc(sizeof(LLIST));
	llist -> data = NULL;
	llist -> size = 0;
	llist -> count = 0;
	llist -> Sorted = False;
	return(llist);
}

LLIST *LLcopyList(LLIST *orig_llist)
{
	LLIST *llist;
	
	llist = LLcreateList();
	if (orig_llist -> size)
	{
		llist -> data = (LLD *)p_alloc(sizeof(LLD) * orig_llist -> size);
		memcpy((char *)llist -> data, (char *)orig_llist -> data,
			  sizeof(LLD) * orig_llist -> count);
		llist -> count = orig_llist -> count;
		llist -> size = orig_llist -> size;
	}
	return(llist);
}

/* Linked list Insertion Routines */
LLIST *LLinsertItemAtPos(LLIST *llist, int position, 
						 LLD item, BOOLEAN MaintainOrder)
{
	if (llist == NULL)
		llist = LLcreateList();
	
	/* First, guarantee our LLIST is ready for new data */
	if (llist -> data == NULL)
	{
		llist -> data = (LLD *)p_alloc(sizeof(LLD) * LL_INCREMENT);
		llist -> size = LL_INCREMENT;
	}
	else if (llist -> size == llist -> count)
	{
		llist -> data = (LLD *)p_remalloc((char *)llist -> data,
								 sizeof(LLD) * llist -> size,
								 sizeof(LLD) * (LL_INCREMENT + llist -> size));
		llist -> size += LL_INCREMENT;
	}
	
	
	/* Is position legal? */
	if (position > llist -> count || position < 0)
	{
		if (trace_misc)
			p_info(PI_TRACE, "in LLinsertItemAtPos(%#x, **BAD POSITION** %d) \
- cnt %d)\n", llist, position, llist -> count);
		return(llist);
	}
	
	/* If any elements exist after ours, bump `em up */
	if (position < llist -> count)
	{
		if (MaintainOrder)
			/* Bump each entry up one */
		{
			memcpy((char *)&llist -> data[position+1],
				   (char *)&llist -> data[position], 
				   (llist -> count-position) * sizeof(LLD));
		}
		else
		{
			llist -> data[llist -> count] = llist -> data[position];
			llist -> Sorted = False;
		}	
	}
	/* Place element in list */
	llist -> data[position] = item;
	llist -> count++;
	return(llist);
}

LLD LLhead(LLIST *llist, int *key)
{
	*key = 0;
	if (llist && llist -> count > *key && llist -> data[*key])
		return (llist -> data[*key]);
	else 
		return (NULL);
}

LLD LLtail(LLIST *llist, int *key)
{
	*key = llist -> count - 1;
	if (llist && *key >= 0 && llist -> data[*key])
		return (llist -> data[*key]);
	else 
		return (NULL);
}

LLD LLprev(LLIST *llist, int *key)
{
	(*key)--;
	if (llist && *key >= 0 && *key < llist -> count && llist -> data[*key])
		return (llist -> data[*key]);
	else 
		return (NULL);
}

LLD LLnext(LLIST *llist, int *key)
{
	(*key)++;
	if (llist && *key >= 0 && *key < llist -> count && llist -> data[*key])
		return (llist -> data[*key]);
	else 
		return (NULL);
}

LLD LLget(LLIST *llist, int key)
{
	if (llist && key >= 0 && key < llist -> count && llist -> data[key])
		return (llist -> data[key]);
	else 
		return (NULL);
}

int LLfindItem(LLIST *llist, LLD item, int (*compare)())
{
	LLD *result;
	int index = LL_ERROR;
	
	if (!compare)
	{
		if (trace_misc)
			p_info(PI_TRACE, "in LLfindItem(%#x, %#x, NULL) - No compare \
function given\n", llist, item);
		return (index);
	}
	
	if (llist -> count > 0) 
	{							/* Make sure list exists */
		LLsort(llist, compare);	/* Make sure it is sorted, as well */
		result = (LLD *)bsearch((char *)&item, (char *)llist -> data,
								llist -> count, sizeof(LLD), compare);
		if (result)
			index = ((int)result - (int)llist -> data) / sizeof(LLD);
	}
	return(index);
}

void LLsort(LLIST *llist, int (*compare)())
{
	if (llist -> Sorted) 
		return;
	if (trace_lc)
		p_info(PI_TRACE, "Sorting list %#x, size: %d\n", llist, llist -> count);
	qsort((char *)llist -> data, llist -> count, sizeof(LLD), compare);
	llist -> Sorted = True;
}

void LLtrace(LLIST *llist, void (*trace_fnc)())
{
	LLD item;
	int index;

	for (item = LLhead(llist, &index); item; item = LLnext(llist, &index))
		(*(trace_fnc))(item, index);
}
