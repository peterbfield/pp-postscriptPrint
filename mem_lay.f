#ifndef _MEM_LAY_F

#define _MEM_LAY_F

#include "list_control.h"
#include "llist.h"

/* Functions from ml_util.c */
int compare_index(char *p1, char *p2);
int compare_index_and_flag(char *p1, char *p2);
int compare_flag_and_type(char *p1, char *p2);
int sort_compare_index(char *p1, char *p2);
int compare_filename(char *p1, char *p2);
int compare_layout_page(char *p1, char *p2);
int compare_layout_real_page(char *p1, char *p2);

LC_LAY_REC *find_layout_filename(LLIST *llist, char *filename);
LC_LAY_REC *find_layout_page(LLIST *llist, char *page);
LC_LAY_REC *find_layout_index(LLIST *llist, int indexx);
LC_MEMB_REC *find_member_filename(LLIST *llist, char *filename);
LC_MEMB_REC *find_member_index(LLIST *llist, int indexx);
LC_MEMB_REC *find_member_type(LLIST *llist, int type);
LC_MEMB_REC *find_project_index(LLIST *llist, int indexx);
LC_MEMB_REC *find_last_member_type(LLIST *llist);
void print_layout(LLD item, int indexx);
void print_member(LLD item, int indexx);

/* Functions from pub_display.c */
void ModifyPageItem(LC_LAY_REC *Item, char *text, char *page);

#endif 
