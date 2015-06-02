#ifndef MENU_FRAME_H

#define MENU_FRAME_H

#include "penta.h"

#define SHAPE	0
#define BEAROFF	1

typedef struct {
	int16 block_numbers[2];
	int16 relationships[2];
	int16 qualifier;
	int16 fraction;
	uint32 offset;
	} REL_WORDS;

typedef struct {
	int type;			/* type of widget */
	int length;			/* length of data in data (a field name in menu) */
	int offset;			/* start index into data */
	int off_rel;		/* index into RELATIVES */
	int (*func)();		/* Call a function that converts to and 
							from visual representation to internal format */
	char *ptr_rw;		/* Pointer to keyword in reserved_words[]. Pointers
							loaded in at runtime by frame_wr(). */
	} CNTRLS;

#endif
