/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include <unistd.h>
#include "p_lib.h"
#include "que.h"

QUEUE Menus = {qNULL, qNULL};

#ifdef TRACE
static int QT = 0;

void setQT()
{
	QT=1;
}

void clearQT()
{
	QT=0;
}

void qtrace(char * text,QUEUE *queue)
{
	int k;
	QE *ele;

	p_info(PI_TRACE, "qtrace %s %x %x\n",text,queue -> head,queue -> tail);
	ele = queue -> head;
	k = 25;
	for(;;)
	{
		if(ele == qNULL)
			return;
		p_info(PI_TRACE, "%x %x %x\n",ele,ele -> next,ele -> prev);
		if(k-- <= 0)
		{
			sleep(3);
			k = 25;
		}
		ele = ele -> next;
	}
}
#endif

QE *QremoveHead(QUEUE *queue)
{
	QE *ele;
	if(queue -> head == qNULL)
	{
#ifdef TRACE
		if(QT)
			qtrace("remove head",queue);
#endif
		return(qNULL);
	}
	ele = queue -> head;
	if((queue -> head = ele -> next) != NULL)
		queue -> head -> prev = qNULL;
	else
		queue -> tail = qNULL;	/* Queue is empty */
	ele -> next = qNULL;
#ifdef TRACE
	if(QT)
		qtrace("remove head",queue);
#endif
	return(ele);
}

QE *QremoveTail(QUEUE *queue)
{
	QE *ele;
	if(queue -> tail == qNULL)
	{
#ifdef TRACE
		if(QT)
			qtrace("remove tail",queue);
#endif
		return(qNULL);
	}
	ele = queue -> tail;
	if((queue -> tail = ele -> prev) != NULL)
		queue -> tail -> next = qNULL;
	else
		queue -> head = qNULL;	/* Queue is empty */
	ele -> prev = qNULL;
#ifdef TRACE
	if(QT)
		qtrace("remove tail",queue);
#endif
	return(ele);
}

QE *QremoveElement(QUEUE *queue, QE *ele)
{
	if(ele -> next == qNULL)
		queue -> tail = ele -> prev;
	else
		ele -> next -> prev = ele -> prev;
	if(ele -> prev == qNULL)
		queue -> head = ele -> next;
	else
		ele -> prev -> next = ele -> next;
	ele -> next = qNULL;
	ele -> prev = qNULL;
#ifdef TRACE
	if(QT)
		qtrace("remove element",queue);
#endif
	return(ele);
}

void QinsertTail(QUEUE *queue, QE *ptr)
{
	if(queue -> tail == qNULL)
	{
		queue -> head = ptr;
		queue -> tail = ptr;
		ptr -> next = qNULL;
		ptr -> prev = qNULL;
	}
	else
	{
		ptr -> next = qNULL;
		ptr -> prev = queue -> tail;
		queue -> tail -> next = ptr;
		queue -> tail = ptr;
	}
#ifdef TRACE
	if(QT)
		qtrace("insert tail",queue);
#endif
	queue -> Sorted = False;
}

void QinsertHead(QUEUE *queue, QE *ptr)
{
	if(queue -> head == qNULL)
	{
		queue -> head = ptr;
		queue -> tail = ptr;
		ptr -> next = qNULL;
		ptr -> prev = qNULL;
	}
	else
	{
		ptr -> prev = qNULL;
		ptr -> next = queue -> head;
		queue -> head -> prev = ptr;
		queue -> head = ptr;
	}
#ifdef TRACE
	if(QT)
		qtrace("insert head",queue);
#endif
	queue -> Sorted = False;
}

void QinsertAfter(QUEUE *queue, QE *ele, QE *ptr)
{
	if(ele -> next == qNULL)
	{
		QinsertTail(queue,ptr);
		return;
	}
	ptr -> next = ele -> next;
	ele -> next = ptr;
	ptr -> next -> prev = ptr;
	ptr -> prev = ele;
}

void QinsertBefore(QUEUE *queue, QE *ele, QE *ptr)
{
	if(ele -> prev == qNULL)
	{
		QinsertHead(queue,ptr);
		return;
	}
	ele -> prev -> next = ptr;
	ptr -> next = ele;
	ptr -> prev = ele -> prev;
	ele -> prev = ptr;
}

void Qclear(QUEUE *queue)
{
	QE *this;
	QE *next;

	this = next = queue -> head;
	while(next)
	{
		this = next;
		next = this -> next;
		p_free((char *)this);
	}
	queue -> head = qNULL;
	queue -> tail = qNULL;
}

/* Implement a bubble sort on Queue */
void Qsort(QUEUE *queue, int (*compare)())
{
	QE *ptr1, *ptr2, *lowest;

	/* Do nothing, if already sorted */
	if (queue -> Sorted) 
		return;
	ptr1 = queue -> head;
	while(ptr1)
	{
		lowest = ptr1;
		for (ptr2 = ptr1;
			 ptr2;
			 ptr2 = ptr2 -> next)
		{
			/* Maintain lowest throughout queue */
			if ((*(compare))(ptr2, lowest) < 0)
				lowest = ptr2;
		}
		/* Has lowest changed */
		if (lowest != ptr1)
		{
			/* Move lowest to ptr1 position */
			lowest = QremoveElement(queue, lowest);
			QinsertBefore(queue, ptr1, lowest);
		} else
			ptr1 = ptr1 -> next;
	}
	queue -> Sorted = True;
}
