#ifndef _PENTA_H

#define _PENTA_H

typedef long int32;
typedef short int16;
typedef unsigned long uint32;
typedef unsigned long UINT32;
typedef unsigned short uint16;
typedef unsigned short UINT16;
typedef unsigned char uchar;

typedef int32 hmu;					/* horizontal machine units */
typedef int32 vmu;					/* vertical machine units */

typedef int32 x_coord;				/* horizontal pixels */
typedef int32 y_coord;				/* vertical pixels */

typedef int BOOLEAN;

typedef int32 PPOINT;

#define D_LINE	0					/* Draw a line */
#define D_ARC	1					/* Draw an arc */

typedef struct
	{
	PPOINT x;
	PPOINT y;
	} POINT_X_Y;

typedef struct _draw_point_x_y DRAW_POINT_X_Y;

typedef struct {
	uint32 width;
	uint32 depth;
	PPOINT left;
	PPOINT top;
	int start_angle;
	int end_angle;
	DRAW_POINT_X_Y *end_arc;
	} P_ARC;

struct _draw_point_x_y
	{
	int draw_type;					/* D_LINE, D_ARC */
	PPOINT x;
	PPOINT y;
	int off_x;
	P_ARC *arc;
	};

#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif
#ifndef True
#define True	1
#endif
#ifndef False
#define False	0
#endif

#endif
