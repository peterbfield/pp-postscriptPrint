#ifndef _LIST_CONTROL_F

#define _LIST_CONTROL_F

#include "llist.h"


void lc_errmsg(char *, int);

						/* lc_errmsg()
							process an error msg
						   char * = ptr to msg, w/o \n
						   int    = errno or -1 if other 
						   will print msg as:
						   LC - your msg: errno, sys err msg
						   in lc_utils.c						*/

int lc_facs_free_id(char *, int);

						/* lc_facs_free_id()
						   	free a file based on the unique id sent
						   	back to caller from original request
						   char *	= TreeName
						   int	= unique id
						   returns > 0 if ok
						   returns 0 if not ok
								see LcResult for reason			*/

int lc_facs_free_group_id(char *, int);

						/* lc_facs_free_group_id()
						  	 free all files by window number sent
						  	 by caller with original request
						   char *	= TreeName
						   int		= window number
						   returns > 0 if ok
						   returns 0 if not o
								reason in LcResult				*/

int lc_facs_req(char *, int, char *, char *, int, int);
						/* lc_facs_req()
						   char *	= dest tree
						   int		= files dir
						   char *	= files subdir
						   char *	= filename
						   int		= GroupId
						   int		= what to do (defines in 
									  list_control.h)
						   returns > 0 (unique id) if ok
						   returns 0 if not ok
								see LcResult for reason			*/

int lc_init(int);

						/* lc_init()
						   int	= print or not print err msgs
								  1 = print err msgs
						   gets hostname, pid, username for
						   list control funcs
						   called by program at startup before
						   caller sets LcForPid
						   returns 0 if ok
						   errno if not ok
						   in lc_utils.c						*/

int lc_lock_and_read_list_file(char *treename, int dir, char *subdir,
							int ListType, LLIST *llist, int lock_file);

int lc_unlock_list_file(char *treename, int dir, char *subdir,
														int ListType);

int lc_write_and_unlock_list_file(char *treename, int dir, char *subdir,
											int ListType, LLIST *llist);

int lc_write_list_file(char *treename, int dir, char *subdir,
											int ListType, LLIST *llist);

int lc_parent(char *tree, char *dir, char *parent);

int lc_create_list_file(char *treename, int dir, char *subdir,
														int ListType);

#endif
