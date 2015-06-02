#ifndef _QUE_H

#define _QUE_H

#include "penta.h"

/* queue element */

typedef struct Qe {
	struct Qe *next;		/* pointer to next element in queue */
	struct Qe *prev;		/* pointer to previous element in queue */
}QE;

/* queue descriptor */

typedef struct struct_queue {
	QE *head;				/* pointer to first element in the queue */
	QE *tail;				/* pointer to last element in the queue */
	BOOLEAN Sorted;			/* flag indicating if Que is sorted */
}QUEUE;

#define qNULL (QE *)0

QE *QremoveHead(QUEUE *);
QE *QremoveTail(QUEUE *);
QE *QremoveElement(QUEUE *, QE *);
void QinsertTail(QUEUE *, QE *);
void QinsertHead(QUEUE *, QE *);
void QinsertAfter(QUEUE *, QE *, QE *);
void QinsertBefore(QUEUE *, QE *, QE *);
void Qclear(QUEUE *);
void Qsort(QUEUE *queue, int (*compare)());

#endif
