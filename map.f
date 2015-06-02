#ifndef _MAP_F

#define _MAP_F

int map_load(WYSIWYG *wn, char *filename, QUEUE *map_data,
												int layout_text_flag);
int map_dump(WYSIWYG *wn, char *filename, QUEUE *map_data,
												int layout_text_flag);
void scan_map(WYSIWYG *wn, int aticle_to_map);
void build_new_map_file(WYSIWYG *wn, FILE_LIST *fl_text,
													int perm_to_write);

#endif
