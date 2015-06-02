#ifndef _SPECS_F

#define _SPECS_F

#include "menu_frame.h"			/* CNTRLS */

/* Build_Layout.c extern functions */
int build_layout_page(WYSIWYG *wn, char *data, Widget parent);
int build_layout_frames(WYSIWYG *wn, char *data, Widget parent);

/* Pub_Display.c extern functions */
void OH_map_widget(Widget widget);
void OH_unmap_widget(Widget widget);
int OH_open_proj(char *tree_name, char *pathname, char *subdir, 
				 								int dir, int filetype);
void NothingChanged(Widget parent);
void CB_DeleteLayout(Widget widget, XtPointer *tag,
											XtPointer *callback_data);

/* Functions defined in lay_display.c */
void set_layout_drawing_area(WYSIWYG *wn, BOOLEAN Resize);
int lay_display(WYSIWYG *inwn, FILE_LIST *infl, 
						BOOLEAN New_layout, int display, WYSIWYG *wn);
void lmt_recompute_layout(WYSIWYG *wn);
int frame_save(ANY_MENU *menu, int CloseMenu);
void DisplayPageSelections(Widget w, LLIST *list1,
											char *filter, int fl_flag);
void CB_RaiseUnitMenu(Widget widget, XtPointer *tag,
											XtPointer *callback_data);
void SetString(char *data, CNTRLS controls, char *new_text);
void OH_set_position_info (WYSIWYG *wn);
void OH_set_crosshair_toggle (WYSIWYG *wn, int which);

/* Functions defined in save_as.c */
void OH_OpenSaveAsMenu(ANY_PUB_MENU *unit_menu, WYSIWYG *wn, 
							Widget parent, char *unitname, short type);

#endif
