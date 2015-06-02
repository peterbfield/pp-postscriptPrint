#ifndef _LOCKS_F

#define _LOCKS_F

extern int lock_a_rectangle(WYSIWYG *, int, int, int, int,
														LOCKS_REPORT *);

extern int lock_a_point(WYSIWYG *, int, int, LOCKS_REPORT *);

extern void mu_post_rb_analyse_lr(WYSIWYG *wn,
						int32 *x_machine, int32 *y_machine,
						int x, int y, LOCKS_REPORT *locks_report);

extern void mu_analyse_locks_report(WYSIWYG *wn, int in_x, int in_y,
						RB_PT *rb, int *out_x, int *out_y,
						LOCKS_REPORT *locks_report,
						char *data_x, char *data_y,
						int flag_h, int flag_v);

extern void mu_post_ch_analyse_lr(WYSIWYG *wn, CROSSHAIRS *ch,
						int x, int y, LOCKS_REPORT *locks_report);

extern void mu_lock_crosshair(WYSIWYG *wn, XEvent *event,
						int *out_x, int *out_y,
						LOCKS_REPORT *locks_report, ANY_MENU *menu);

extern void mu_analyse_a_rectangle(WYSIWYG *wn, RB_PT *rb,
						LOCKS_REPORT *locks_report,
						char *data_left, char *data_top);

#endif
