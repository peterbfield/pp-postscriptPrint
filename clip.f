#ifndef _CLIP_F

#define _CLIP_F

extern  void lmt_AddIntersection(long x,long y, CLIP_POINTS *old_pt,
				CLIP_POINTS *new_pt, int type_old,int type_new);
extern  void lmt_BuildIntersections(CLIP_POINTS *start_old,
				CLIP_POINTS *start_new);
extern  CLIP_POINTS *lmt_BuildClipPoints(DRAW_POINT_X_Y *lp, int n_pts);
extern  LIST_PT *lmt_AddPtToList(LIST_PT *, CLIP_POINTS *cur);

#endif
