#ifndef _LINK_UTIL_H

#define _LINK_UTIL_H

typedef struct
	{
	QE que;
	FRAME *frame;
	int frame_number;
	} FR_LINK;

typedef struct linked_frame
	{
	QE que;
	QUEUE list_frame;
	int tot_links;
	int16 text_id;
	int16 article_id;
	} LINKED_FRAME;

#define NO_MATCHING_WN	0
#define SAME_WN_NEW_LAY	1
#define NEW_WN			2

#endif
