#ifndef _ROTATION_H

#define _ROTATION_H

typedef struct {
	PPOINT origin_x;	/* horizontal pixel location of rotation */
	PPOINT origin_y;	/* vertical pixel location of rotation */
	int16 degree;		/* degree of rotation -- must be 0 <= d < 360 */
	} ROT;

#endif
