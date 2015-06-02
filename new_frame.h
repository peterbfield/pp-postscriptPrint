#ifndef _NEW_FRAME_H

#define _NEW_FRAME_H

#include <X11/Intrinsic.h>
#include "menus.h"			/* Defines MENU_HEADER */
#include "frame.h"

#define GRAPHIC_OFFSET	10 

typedef struct {
	MENU_HEADER;
	LAYOUT_DESC *layout;
	int frame;			/* Number of the frame edited */
	Widget widgets[BOUND_CONTROLS];
	char *data;
} ANY_FRAME_MENU;

typedef struct {
	int32 start_x_machine;
	int32 start_y_machine;
	int32 end_x_machine;
	int32 end_y_machine;
	int32 diff_x_machine;
	int32 diff_y_machine;
	int start_x;
	int start_y;
	int end_x;
	int end_y;
} RB_PT;

#include "frame.f"

#endif
