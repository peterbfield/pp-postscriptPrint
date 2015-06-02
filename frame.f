#ifndef _FRAME_F

#define _FRAME_F

extern void pw_drag_frame_selected(WYSIWYG *wn, RB_PT *rb);
extern void mu_anchor_starting_corner_of_box(WYSIWYG *wn, XEvent *event,
						ANY_MENU *menu, RB_PT *rb);
extern void mu_move_box_as_mouse_moves(WYSIWYG *wn, XEvent *event, 
						ANY_MENU *menu,
						RB_PT *rb, int *ref_x, int *ref_y);
extern void mu_draw_box_as_mouse_moves(WYSIWYG *wn, XEvent *event, 
					ANY_MENU *menu, RB_PT *rb, int Checked, int nature);
extern void mu_anchor_ending_corner_of_box(WYSIWYG *wn, XEvent *event, 
						RB_PT *rb, int nature);
extern void free_form(WYSIWYG *, WYSIWYG *, QUEUE *,
						RB_PT *rb_pt_layout, int);
extern void mu_get_syntax(int type, short relation,
						uint32 offset, char *data);
extern void mu_setup_rb(int left, int top,
						int right, int bottom, RB_PT *rb);
extern void mu_draw_rule_as_mouse_moves(WYSIWYG *wn, XEvent *event, 
						ANY_MENU *menu, RB_PT *rb,
						int x_weight, int y_weight, int Checked);
extern void mu_move_crosshair(WYSIWYG *wn, CROSSHAIRS *ch, int , int y);
extern void mu_display_tracking(ANY_MENU *menu,
						char *data_left, char *data_top,
						char *data_width, char *data_depth);
extern void pw_put_image(WYSIWYG *wn, LINE_IMAGE *line_image,
						int xpin, int ypin, int frame, ELEMENT *ele,
                        int top, int bot);

#include "pw_misc.f"

#endif
