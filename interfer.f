#ifndef _INTERFER_F

#define _INTERFER_F

#include "menu_frame.h"

extern	short lmt_size_parser(REL_WORDS *, char *, int);
extern	void lmt_syntax_to_ascii(REL_WORDS *, char *, char *);
extern	void lmt_syntax_to_ascii_pts(REL_WORDS *, char *);
extern	short lmt_xy_syntax_parser(REL_WORDS *, char *, char *, int,
				int, int);

#endif
