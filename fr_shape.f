#ifndef _ELE_SHAPE_F

#define _ELE_SHAPE_F

extern void shp_add_pixel(PPOINT, PPOINT, PPOINT, WORK *);

extern void shp_process_coordinate(PPOINT, PPOINT,
								   PPOINT, PPOINT, WORK *);

extern void shp_create_shape(long, long, WORK *);

#endif
