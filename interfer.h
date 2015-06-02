#ifndef _INTERFER_H

#define _INTERFER_H

#include "penta.h"

#define SORT_X 1
#define SORT_Y 2

typedef struct struct_clip_points CLIP_POINTS;

struct struct_clip_points
{
	DRAW_POINT_X_Y pt;
	uint32 hard : 1;
	uint32 intersection : 1;
	uint32 used : 1;
	uint32 inside : 2;
			/*
				00 : not evaluated
				01 : inside
				10 : outside
				11 : not used
			*/
	uint32 unused : 27;
	CLIP_POINTS *prev;
	CLIP_POINTS *next;
	CLIP_POINTS *twin;
};

typedef struct struct_edges
{
	DRAW_POINT_X_Y pt1;
	DRAW_POINT_X_Y pt2;
	float delta;
	float b;
	PPOINT result;
	struct struct_edges *next;
	struct struct_edges *prev;
	int invalid;
} EDGES;

#define NUM_PTS     10

typedef struct struct_list_pt
	{
	DRAW_POINT_X_Y *points;
	PPOINT min_x;
	PPOINT max_x;
	PPOINT min_y;
	PPOINT max_y;
	int pts_used;
	int total_pts;
	} LIST_PT;

#include "interfer.f"

#endif
