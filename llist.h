/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#ifndef _LLIST_H

#define _LLIST_H

#include "penta.h"

typedef void * LLD;

/* llist descriptor */
typedef struct {
	int size;							/* size of array */
	int count;							/* number of used elements in array */
	BOOLEAN Sorted;						/* flag indicating if list is sorted */
	LLD *data;							/* Array of data elements */
} LLIST;

/* Linked List macros */
#define LL_ERROR -1
#define LLSIZE(llist)      ((llist)->count)
#define LLSORTED(llist)    ((llist)->Sorted)

/* Linked List Removal Routines */
LLD  LLrmItemAtPos(LLIST *llist, int position, BOOLEAN MaintainOrder);
void LLrmItem(LLIST *llist, LLD item, BOOLEAN MaintainOrder, int (*compare)());
#define LLrmHead(llist, Order)  LLrmItemAtPos(llist, 0, Order)
#define LLrmTail(llist)         LLrmItemAtPos(llist, (llist)->count-1, False)
void LLclearItems(LLIST *);
void LLclearList(LLIST *);
void LLclear(LLIST *);


/* Linked list Insertion Routines */
LLIST *LLinsertItemAtPos(LLIST *llist, int position, 
					   LLD item, BOOLEAN MaintainOrder);
#define LLinsertHead(llist, item, Order)  \
        LLinsertItemAtPos(llist, 0, item, Order)
#define LLinsertTail(llist, item) \
        LLinsertItemAtPos(llist, (llist)->count, item, False)

/* Linked list access routines */
LLD LLhead(LLIST *llist, int *key);
LLD LLtail(LLIST *llist, int *key);
LLD LLprev(LLIST *llist, int *key);
LLD LLnext(LLIST *llist, int *key);
LLD LLget(LLIST *llist, int key);

/* Linked list misc routines */
LLIST *LLcreateList(void);
LLIST *LLcopyList(LLIST *orig_llist);
int LLfindItem(LLIST *llist, LLD item, int (*compare)());
void LLsort(LLIST *llist, int (*compare)());
void LLtrace(LLIST *llist, void (*trace_fnc)());

#endif 
