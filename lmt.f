#ifndef _LMT_F

#define _LMT_F

extern  int lmt_BuildElements(ELEMENT **head,
								ELEMENT **neg_head, ELEMENT *old_ele,
								ELEMENT *new_ele, int neg_flag);
extern	long lmt_lock_lead(WYSIWYG *wn, int i, long Ti);
extern  long lmt_frame_off_to_abs(WYSIWYG *wn, int frame,
										int type, unsigned long value);
extern uint32 lmt_abs_to_off(WYSIWYG *wn, int type,
									uint32 units, int32 machine_unit);
extern  long lmt_off_to_abs(WYSIWYG *wn, int type, unsigned long value);
extern  long lmt_rel_to_abs(WYSIWYG *wn, int abs_index,
									int rel_index, RELATIVES *rel_data);
extern  int lmt_interface(WYSIWYG *wn);
extern	void lmt_BuildRectangle(DRAW_POINT_X_Y *list_pt, ELEMENT *ele,
				long top, long bottom, long left, long right);
extern	void lmt_page_elements(WYSIWYG *);
extern	void lmt_set_rot_frame(WYSIWYG *, int);
extern	ELEMENT *lmt_set_rot_ele(WYSIWYG *, int, int, ELEMENT *);
extern ELEMENT *lmt_set_scale_ele(WYSIWYG *wn, int rot, int ref, ELEMENT *rot_ele);
extern  ELEMENT *lmt_MergeElements(ELEMENT *head);
extern  void lmt_compute_x(WYSIWYG *, int);
extern  void lmt_compute_y(WYSIWYG *, int);
extern  void lmt_compute_param_x(WYSIWYG *wn, int i,
				int32 *anchor_x, int32 *width);
extern  void lmt_compute_param_y(WYSIWYG *wn, int i,
				int32 *anchor_y, int32 *depth);
extern  void lmt_IsARule(WYSIWYG *, int, int16 *, int32 *, int32 *);
extern  int lmt_IsTwoDiffEle(ELEMENT *ele_1, ELEMENT *ele_2);

#endif
