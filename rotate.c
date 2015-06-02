/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

/***********************************************************************
 *
 *	  This routine is used to get the rotated position of any point
 *	on a page in relations to a rotation origin point on the page
 *	and the degree of rotation from that origin point.  It uses an
 *	array of sine values which are the top half of a fraction whose
 *	bottom is 1000.  This means that the resultant value from the
 *	calculation can be rounded by adding 512 and the actual value
 *	can be obtained be shifting left 10 bits.  Note, do all math
 *	with 32 bit values, then shift the result 10 places into a 16
 *	bit value to avoid problems with negative numbers.
 * 
 **********************************************************************/

#include <stdio.h>
#include <X11/Xlib.h>
#include "rel_data.h"
#include "rotation.h"
#include "frame.h"
#include "interfer.h"
#include "lmt.f"

static void do_rot(WYSIWYG *, ROT *, PPOINT *, PPOINT *);

/* Table of sin values for angles from 0.0 to 90.0 */
uint16 sin_cos[901] = {
0, 1, 3, 5, 7, 8, 10, 12, 14, 16,                   /* 0 to 9 */
17, 19, 21, 23, 25, 26, 28, 30, 32, 33, 
35, 37, 39, 41, 42, 44, 46, 48, 50, 51, 
53, 55, 57, 58, 60, 62, 64, 66, 67, 69, 
71, 73, 74, 76, 78, 80, 82, 83, 85, 87, 
89, 91, 92, 94, 96, 98, 99, 101, 103, 105, 
107, 108, 110, 112, 114, 115, 117, 119, 121, 123, 
124, 126, 128, 130, 131, 133, 135, 137, 138, 140, 
142, 144, 146, 147, 149, 151, 153, 154, 156, 158, 
160, 161, 163, 165, 167, 169, 170, 172, 174, 176, 
177, 179, 181, 183, 184, 186, 188, 190, 191, 193, /* 100 to 109 */
195, 197, 198, 200, 202, 204, 205, 207, 209, 211, 
212, 214, 216, 218, 219, 221, 223, 225, 226, 228, 
230, 232, 233, 235, 237, 239, 240, 242, 244, 245, 
247, 249, 251, 252, 254, 256, 258, 259, 261, 263, 
265, 266, 268, 270, 271, 273, 275, 277, 278, 280, 
282, 283, 285, 287, 289, 290, 292, 294, 295, 297, 
299, 301, 302, 304, 306, 307, 309, 311, 313, 314, 
316, 318, 319, 321, 323, 324, 326, 328, 330, 331, 
333, 335, 336, 338, 340, 341, 343, 345, 346, 348, 
350, 351, 353, 355, 356, 358, 360, 361, 363, 365, /* 200 to 209 */
366, 368, 370, 371, 373, 375, 376, 378, 380, 381, 
383, 385, 386, 388, 390, 391, 393, 395, 396, 398, 
400, 401, 403, 405, 406, 408, 409, 411, 413, 414, 
416, 418, 419, 421, 423, 424, 426, 427, 429, 431, 
432, 434, 435, 437, 439, 440, 442, 444, 445, 447, 
448, 450, 452, 453, 455, 456, 458, 460, 461, 463, 
464, 466, 468, 469, 471, 472, 474, 475, 477, 479, 
480, 482, 483, 485, 487, 488, 490, 491, 493, 494, 
496, 498, 499, 501, 502, 504, 505, 507, 508, 510, 
512, 513, 515, 516, 518, 519, 521, 522, 524, 525, /* 300 to 309 */
527, 528, 530, 531, 533, 535, 536, 538, 539, 541, 
542, 544, 545, 547, 548, 550, 551, 553, 554, 556, 
557, 559, 560, 562, 563, 565, 566, 568, 569, 571, 
572, 574, 575, 577, 578, 579, 581, 582, 584, 585, 
587, 588, 590, 591, 593, 594, 596, 597, 598, 600, 
601, 603, 604, 606, 607, 609, 610, 611, 613, 614, 
616, 617, 619, 620, 621, 623, 624, 626, 627, 629, 
630, 631, 633, 634, 636, 637, 638, 640, 641, 643, 
644, 645, 647, 648, 649, 651, 652, 654, 655, 656, 
658, 659, 660, 662, 663, 665, 666, 667, 669, 670, /* 400 to 409 */
671, 673, 674, 675, 677, 678, 679, 681, 682, 683, 
685, 686, 687, 689, 690, 691, 693, 694, 695, 697, 
698, 699, 700, 702, 703, 704, 706, 707, 708, 710, 
711, 712, 713, 715, 716, 717, 719, 720, 721, 722, 
724, 725, 726, 727, 729, 730, 731, 732, 734, 735, 
736, 737, 739, 740, 741, 742, 744, 745, 746, 747, 
748, 750, 751, 752, 753, 754, 756, 757, 758, 759, 
760, 762, 763, 764, 765, 766, 768, 769, 770, 771, 
772, 773, 775, 776, 777, 778, 779, 780, 782, 783, 
784, 785, 786, 787, 789, 790, 791, 792, 793, 794, /* 500 to 509 */
795, 796, 798, 799, 800, 801, 802, 803, 804, 805, 
806, 808, 809, 810, 811, 812, 813, 814, 815, 816, 
817, 818, 819, 821, 822, 823, 824, 825, 826, 827, 
828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 
838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 
848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 
858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 
868, 869, 870, 871, 872, 873, 874, 874, 875, 876, 
877, 878, 879, 880, 881, 882, 883, 884, 885, 885, 
886, 887, 888, 889, 890, 891, 892, 892, 893, 894, /* 600 to 609 */
895, 896, 897, 898, 899, 899, 900, 901, 902, 903, 
904, 904, 905, 906, 907, 908, 909, 909, 910, 911, 
912, 913, 914, 914, 915, 916, 917, 918, 918, 919, 
920, 921, 921, 922, 923, 924, 925, 925, 926, 927, 
928, 928, 929, 930, 931, 931, 932, 933, 934, 934, 
935, 936, 936, 937, 938, 939, 939, 940, 941, 941, 
942, 943, 943, 944, 945, 946, 946, 947, 948, 948, 
949, 950, 950, 951, 952, 952, 953, 954, 954, 955, 
955, 956, 957, 957, 958, 959, 959, 960, 961, 961, 
962, 962, 963, 964, 964, 965, 965, 966, 967, 967, /* 700 to 709 */
968, 968, 969, 969, 970, 971, 971, 972, 972, 973, 
973, 974, 974, 975, 976, 976, 977, 977, 978, 978, 
979, 979, 980, 980, 981, 981, 982, 982, 983, 983, 
984, 984, 985, 985, 986, 986, 987, 987, 988, 988, 
989, 989, 990, 990, 990, 991, 991, 992, 992, 993, 
993, 994, 994, 994, 995, 995, 996, 996, 996, 997, 
997, 998, 998, 998, 999, 999, 1000, 1000, 1000, 1001, 
1001, 1001, 1002, 1002, 1003, 1003, 1003, 1004, 1004, 1004, 
1005, 1005, 1005, 1006, 1006, 1006, 1007, 1007, 1007, 1008, 
1008, 1008, 1009, 1009, 1009, 1009, 1010, 1010, 1010, 1011, /* 800 tp 809 */
1011, 1011, 1011, 1012, 1012, 1012, 1013, 1013, 1013, 1013, 
1014, 1014, 1014, 1014, 1015, 1015, 1015, 1015, 1015, 1016, 
1016, 1016, 1016, 1017, 1017, 1017, 1017, 1017, 1018, 1018, 
1018, 1018, 1018, 1018, 1019, 1019, 1019, 1019, 1019, 1019, 
1020, 1020, 1020, 1020, 1020, 1020, 1020, 1021, 1021, 1021, 
1021, 1021, 1021, 1021, 1021, 1022, 1022, 1022, 1022, 1022, 
1022, 1022, 1022, 1022, 1022, 1023, 1023, 1023, 1023, 1023, 
1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 
1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1024/* 890 tp 900 */
};

static void get_rot(WYSIWYG *wn, ROT *rot, PPOINT *x, PPOINT *y)
{
	float org_x, org_y;
	float h,v;
	float c,s,t, temp_x, temp_y;

	org_x = (float)(rot -> origin_x * wn -> ldb);
	org_y = (float)(rot -> origin_y * wn -> msb);
	h = (float)((*x - rot -> origin_x) * wn -> ldb);
	v = (float)((*y - rot -> origin_y) * wn -> msb);
	if(rot -> degree < QUAT_CIRCLE)
	{
		s = (float)(long)sin_cos[rot -> degree];
		c = (float)(long)sin_cos[QUAT_CIRCLE - rot -> degree];
	}
	else if(rot -> degree == QUAT_CIRCLE)
	{
		*x = org_x - v;
		*y = org_y + h;
		return;
	}
	else if(rot -> degree < HALF_CIRCLE)
	{
		s = (float)(long)sin_cos[HALF_CIRCLE - rot -> degree];
		c = (float)(long)-sin_cos[rot -> degree - QUAT_CIRCLE];
	}
	else if(rot -> degree == HALF_CIRCLE)
	{
		*x = org_x - h;
		*y = org_y - v;
		return;
	}
	else if(rot -> degree < THQU_CIRCLE)
	{
		s = (float)(long)-sin_cos[rot -> degree - HALF_CIRCLE];
		c = (float)(long)-sin_cos[THQU_CIRCLE - rot -> degree];
	}
	else if(rot -> degree == THQU_CIRCLE)
	{
		*x = org_x + v;
		*y = org_y - h;
		return;
	}
	else
	{
		s = (float)(long)-sin_cos[FULL_CIRCLE - rot -> degree];
		c = (float)(long)sin_cos[rot -> degree - THQU_CIRCLE];
	}
	t = ((h * c) - (v * s)) / 1024.;
	temp_x = t + org_x;
	*x = (int32)((temp_x < 0.) ? (temp_x - .5) : (temp_x + .5));
	t = ((h * s) + (v * c)) / 1024.;
	temp_y = t + org_y;
	*y = (int32)((temp_y < 0.) ? (temp_y - .5) : (temp_y + .5));
}

static void do_rot(WYSIWYG *wn, ROT *rot, PPOINT *x, PPOINT *y)
{
	get_rot(wn, rot, x, y);
	if (*x >= 0) *x = (*x + (wn -> ldb >> 1)) / wn -> ldb;
	else         *x = (*x - (wn -> ldb >> 1)) / wn -> ldb;
	if (*y >= 0) *y = (*y + (wn -> msb >> 1)) / wn -> msb;
	else         *y = (*y + (wn -> msb >> 1)) / wn -> msb;
}

void do_mouse_rot(WYSIWYG *wn, ROT *rot, PPOINT *x, PPOINT *y)
{
	if (rot -> degree == 0)
		return;
	get_rot(wn, rot, x, y);
	*x = (*x + (wn -> ldb >> 1)) / wn -> ldb;
	*y = (*y + (wn -> msb >> 1)) / wn -> msb;
}

int set_rot_origin(WYSIWYG *wn, int ref, ROT *rotation)
{
	PPOINT t, b, l, r, tb, lr;

	t = FRAME_DATA(ref) top;
	b = FRAME_DATA(ref) bottom;
	tb = FRAME_DATA(ref) y_center;
	l = FRAME_DATA(ref) left; 
	r = FRAME_DATA(ref) right; 
	lr = FRAME_DATA(ref) x_center;
	switch(ROT_LOCKPOINT(ref))
	{
	  case TL:
		rotation -> origin_y = t;
		rotation -> origin_x = l; 
		break;
	  case TC:
		rotation -> origin_y = t;
		rotation -> origin_x = lr;
		break;
	  case TR:
		rotation -> origin_y = t;
		rotation -> origin_x = r;
		break;
	  case CL:
		rotation -> origin_y = tb;
		rotation -> origin_x = l;
		break;
	  case CC:
		rotation -> origin_y =  tb;
		rotation -> origin_x = lr;
		break;
	  case CR:
		rotation -> origin_y = tb;
		rotation -> origin_x = r;
		break;
	  case BL:
		rotation -> origin_y = b;
		rotation -> origin_x = l;
		break;
	  case BC:
		rotation -> origin_y = b;
		rotation -> origin_x = lr;
		break;
	  case BR:
		rotation -> origin_y = b;
		rotation -> origin_x = r;
		break;
	  default:
		return(P_FAILURE);
	}
	return(P_SUCCESS);
}

void lmt_set_rot_frame(WYSIWYG *wn, int ref)
{
	ROT rotation;
	int i;

	FRAME_DATA(ref) rot_top_left.x = FRAME_DATA(ref) left;
	FRAME_DATA(ref) rot_top_left.y = FRAME_DATA(ref) top;
	FRAME_DATA(ref) rot_bottom_left.x = FRAME_DATA(ref) left;
	FRAME_DATA(ref) rot_bottom_left.y = FRAME_DATA(ref) bottom;
	FRAME_DATA(ref) rot_y_center_left.x = FRAME_DATA(ref) left;
	FRAME_DATA(ref) rot_y_center_left.y = FRAME_DATA(ref) y_center;
	FRAME_DATA(ref) rot_top_right.x = FRAME_DATA(ref) right;
	FRAME_DATA(ref) rot_top_right.y = FRAME_DATA(ref) top;
	FRAME_DATA(ref) rot_bottom_right.x = FRAME_DATA(ref) right;
	FRAME_DATA(ref) rot_bottom_right.y = FRAME_DATA(ref) bottom;
	FRAME_DATA(ref) rot_y_center_right.x = FRAME_DATA(ref) right;
	FRAME_DATA(ref) rot_y_center_right.y = FRAME_DATA(ref) y_center;
		FRAME_DATA(ref) rot_top_x_center.x = FRAME_DATA(ref) x_center;
	FRAME_DATA(ref) rot_top_x_center.y = FRAME_DATA(ref) top;
	FRAME_DATA(ref) rot_bottom_x_center.x = FRAME_DATA(ref) x_center;
	FRAME_DATA(ref) rot_bottom_x_center.y = FRAME_DATA(ref) bottom;
	FRAME_DATA(ref) rot_y_center_x_center.x = FRAME_DATA(ref) x_center;
	FRAME_DATA(ref) rot_y_center_x_center.y = FRAME_DATA(ref) y_center;
	if (!(rotation.degree = REL_DATA(ref) i13))
		return;
	if (set_rot_origin(wn, ref, &rotation) == P_FAILURE)
		return;
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_top_left.x,
		   &FRAME_DATA(ref) rot_top_left.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_bottom_left.x,
		   &FRAME_DATA(ref) rot_bottom_left.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_y_center_left.x,
		   &FRAME_DATA(ref) rot_y_center_left.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_top_right.x,
		   &FRAME_DATA(ref) rot_top_right.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_bottom_right.x,
		   &FRAME_DATA(ref) rot_bottom_right.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_y_center_right.x,
		   &FRAME_DATA(ref) rot_y_center_right.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_top_x_center.x,
		   &FRAME_DATA(ref) rot_top_x_center.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_bottom_x_center.x,
		   &FRAME_DATA(ref) rot_bottom_x_center.y);
	do_rot(wn, &rotation, &FRAME_DATA(ref) rot_y_center_x_center.x,
		   &FRAME_DATA(ref) rot_y_center_x_center.y);
	if (FRAME_DATA(ref) out_lst_pts &&
		FRAME_DATA(ref) out_lst_pts != FRAME_DATA(ref) list_points)
		p_free((char *)FRAME_DATA(ref) out_lst_pts);
	FRAME_DATA(ref) out_lst_pts =
		(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) *
								  FRAME_DATA(ref) n_points);
	for (i = 0; i < FRAME_DATA(ref) n_points; i++)
	{
		FRAME_DATA(ref) out_lst_pts[i].x = FRAME_DATA(ref) list_points[i].x;
		FRAME_DATA(ref) out_lst_pts[i].y = FRAME_DATA(ref) list_points[i].y;
		do_rot(wn, &rotation, &FRAME_DATA(ref) out_lst_pts[i].x,
			   &FRAME_DATA(ref) out_lst_pts[i].y); 
	}
}

ELEMENT *lmt_set_rot_ele(WYSIWYG *wn, int rot, int ref, ELEMENT *rot_ele)
{
	ELEMENT *new_ele;
	ROT rotation;
	int i;
	PPOINT t = 0, b = 0, l = 0, r = 0;
    int crop_left, crop_top;
    FRAME *frame = wn -> selected_lay -> frames[ref];

	new_ele = (ELEMENT *)p_alloc(sizeof(ELEMENT));
	memcpy((char *)new_ele, (char *)rot_ele, sizeof(ELEMENT));
	new_ele -> out_lst_pts = new_ele -> list_points =
		(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * new_ele-> n_points);
	memcpy((char *)new_ele -> list_points, (char *)rot_ele -> list_points,
		   sizeof(DRAW_POINT_X_Y) * new_ele -> n_points);
	if (!(rotation.degree = REL_DATA(ref) i13))
		return(new_ele);
	if (rot != ref)
		rotation.degree = FULL_CIRCLE - rotation.degree;
	if (set_rot_origin(wn, ref, &rotation) == P_FAILURE)
		return(new_ele);
	for (i = 0; i < new_ele -> n_points; i++)
	{
		if (TYPE_OF_FRAME(ref) == PL_GRAPHIC && WRAP_GRAPHIC(ref))
		{
/* Do scaling */
			crop_left = (lmt_off_to_abs(wn, X_REF, CROP_LEFT(ref))*ZOOM_GR(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT ;
			crop_top = (lmt_off_to_abs(wn, Y_REF, CROP_TOP(ref))*ZOOM_GRY(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT; 
			new_ele -> list_points[i].x = ((new_ele -> list_points[i].x - frame ->left)*ZOOM_GR(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT +
	frame -> left - crop_left;
			new_ele -> list_points[i].y = ((new_ele -> list_points[i].y - frame -> top)*ZOOM_GRY(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT +
	frame -> top - crop_top;
		}
		do_rot(wn, &rotation, &new_ele -> list_points[i].x,
			   &new_ele -> list_points[i].y); 
		if (!i)
		{
			l = new_ele -> list_points[0].x;
			r = new_ele -> list_points[0].x;
			t = new_ele -> list_points[0].y;
			b = new_ele -> list_points[0].y;
		}
		else
		{
		    if (new_ele -> list_points[i].x < l)
        		l = new_ele -> list_points[i].x;
    		else if (new_ele -> list_points[i].x > r)
        		r = new_ele -> list_points[i].x;
    		if (new_ele -> list_points[i].y < t)
        		t = new_ele -> list_points[i].y;
    		else if (new_ele -> list_points[i].y > b)
        		b = new_ele -> list_points[i].y;
		}
	}
	new_ele -> rot_rect_left = l;
	new_ele -> rot_rect_right = r;
	new_ele -> rot_rect_top = t;
	new_ele -> rot_rect_bottom = b;
	new_ele -> rot_rect_x_center =
		(new_ele -> rot_rect_left + new_ele -> rot_rect_right) >> 1;
	new_ele -> rot_rect_y_center =
		(new_ele -> rot_rect_top + new_ele -> rot_rect_bottom) >> 1;
	return(new_ele);
}

/* This function should be called for wrapped graphics only */
ELEMENT *lmt_set_scale_ele(WYSIWYG *wn, int rot, int ref, ELEMENT *rot_ele)
{
	ELEMENT *new_ele;
	int i;
	PPOINT t = 0, b = 0, l = 0, r = 0;
    int crop_left, crop_top;
    FRAME *frame = wn -> selected_lay -> frames[ref];

	new_ele = (ELEMENT *)p_alloc(sizeof(ELEMENT));
	memcpy((char *)new_ele, (char *)rot_ele, sizeof(ELEMENT));
	new_ele -> out_lst_pts = new_ele -> list_points =
		(DRAW_POINT_X_Y *)p_alloc(sizeof(DRAW_POINT_X_Y) * new_ele-> n_points);
	memcpy((char *)new_ele -> list_points, (char *)rot_ele -> list_points,
		   sizeof(DRAW_POINT_X_Y) * new_ele -> n_points);
	for (i = 0; i < new_ele -> n_points; i++)
	{
		{
/* Do scaling */
			crop_left = (lmt_off_to_abs(wn, X_REF, CROP_LEFT(ref))*ZOOM_GR(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT ;
			crop_top = (lmt_off_to_abs(wn, Y_REF, CROP_TOP(ref))*ZOOM_GRY(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT; 
			new_ele -> list_points[i].x = ((new_ele -> list_points[i].x - frame ->left)*ZOOM_GR(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT +
	frame -> left - crop_left;
			new_ele -> list_points[i].y = ((new_ele -> list_points[i].y - frame -> top)*ZOOM_GRY(ref) + HALF_ZOOM_UNIT)/ZOOM_UNIT +
	frame -> top - crop_top;
		}
		if (!i)
		{
			l = new_ele -> list_points[0].x;
			r = new_ele -> list_points[0].x;
			t = new_ele -> list_points[0].y;
			b = new_ele -> list_points[0].y;
		}
		else
		{
		    if (new_ele -> list_points[i].x < l)
        		l = new_ele -> list_points[i].x;
    		else if (new_ele -> list_points[i].x > r)
        		r = new_ele -> list_points[i].x;
    		if (new_ele -> list_points[i].y < t)
        		t = new_ele -> list_points[i].y;
    		else if (new_ele -> list_points[i].y > b)
        		b = new_ele -> list_points[i].y;
		}
	}
	new_ele -> rot_rect_left = l;
	new_ele -> rot_rect_right = r;
	new_ele -> rot_rect_top = t;
	new_ele -> rot_rect_bottom = b;
	new_ele -> rot_rect_x_center =
		(new_ele -> rot_rect_left + new_ele -> rot_rect_right) >> 1;
	new_ele -> rot_rect_y_center =
		(new_ele -> rot_rect_top + new_ele -> rot_rect_bottom) >> 1;
	return(new_ele);
}

void do_overflow_rot(WYSIWYG *wn, POINT_X_Y list[], int ref)
{
	ROT rotation;

	if (!(rotation.degree = REL_DATA(ref) i13) ||
		(set_rot_origin(wn, ref, &rotation) == P_FAILURE))
		return;
	do_rot(wn, &rotation, &list[0].x, &list[0].y);
	do_rot(wn, &rotation, &list[1].x, &list[1].y);
	do_rot(wn, &rotation, &list[2].x, &list[2].y);
}

void do_trap_rot(WYSIWYG *wn, DRAW_POINT_X_Y list[],int n_points, int ref)
{
	ROT rotation;
	int i;

	if (!(rotation.degree = REL_DATA(ref) i13) ||
		(set_rot_origin(wn, ref, &rotation) == P_FAILURE))
		return;
	for(i=0;i<n_points;i++)
		do_rot(wn, &rotation, &list[i].x,&list[i].y);
}

#include "window.h"
int rotate_wrt_scale(WYSIWYG *wn, int frame_num, DRAW_POINT_X_Y inp, DRAW_POINT_X_Y *outp)
{
/* Find coordinates of the point inp making correction based on x&y scaling factors */
    ROT rotation;
    int crop_left, crop_top;
    int minx, miny, maxx, maxy;
    FRAME *frame = wn -> selected_lay -> frames[frame_num];
/* Rotate to frame frame_num , degree = - ROT_DEGREE */
    set_rot_origin(wn, frame_num, &rotation);
    rotation.degree = FULL_CIRCLE - ROT_DEGREE(frame_num) ;
    if (rotation.degree >= FULL_CIRCLE) rotation.degree -= FULL_CIRCLE;
    memcpy(outp, &inp, sizeof(DRAW_POINT_X_Y));
    do_rot(wn, &rotation, &(outp -> x), &(outp -> y));
/* Do scaling */
    crop_left = (lmt_off_to_abs(wn, X_REF, CROP_LEFT(frame_num))*ZOOM_GR(frame_num) + HALF_ZOOM_UNIT)/ZOOM_UNIT ;
    crop_top = (lmt_off_to_abs(wn, Y_REF, CROP_TOP(frame_num))*ZOOM_GRY(frame_num) + HALF_ZOOM_UNIT)/ZOOM_UNIT;
    outp -> x = ((outp -> x - frame ->left)*ZOOM_GR(frame_num) + HALF_ZOOM_UNIT)/ZOOM_UNIT +
	frame -> left - crop_left;
    outp -> y = ((outp -> y - frame -> top)*ZOOM_GRY(frame_num) + HALF_ZOOM_UNIT)/ZOOM_UNIT +
	frame -> top - crop_top;

/* If the point is out of frame (cropping) make a correction */
    minx = frame ->prev_left + crop_left;
    miny =  frame ->prev_top + crop_top;
    maxx = minx + lmt_off_to_abs(wn,X_REF,WIDTH_OFFSET(frame_num));
    maxy = miny + lmt_off_to_abs(wn,Y_REF,DEPTH_OFFSET(frame_num));
    if (outp -> x > maxx)
	outp -> x = maxx;
    if (outp -> y > maxy)
	outp -> y = maxy;
    if (outp -> x < minx)
	outp -> x = minx;
    if (outp -> y < miny)
	outp -> y = miny;
/* Rotate back */
    rotation. degree = ROT_DEGREE(frame_num) ;
    do_rot(wn, &rotation, &(outp -> x), &(outp -> y));
    return TRUE;
}

/* Rotate all points in pts by -ROT_DEGREE(frame) */
void rotate_all_pts(WYSIWYG *wn, int frame, XPoint *pts, int npts)
{
                           PPOINT x,y;
                           ROT rotation;
			   int i;
			   int frameAngle = ROT_DEGREE(frame);
			   /* Rotate the frame backwards */
			   set_rot_origin(wn, frame, &rotation);
			   rotation.degree = (FULL_CIRCLE - frameAngle);
			   if (rotation.degree >= FULL_CIRCLE) rotation.degree -= FULL_CIRCLE;
			   for (i=0; i<npts; i++)
			   {
/* Convert pixels to units in order to be able to use do_rot */
			     x = pix_h(wn,pts[i]. x);
			     y = pix_v(wn,pts[i]. y);
			     do_rot(wn, &rotation, &(x), &(y));
			     pts[i]. x = h_pix(wn,x);
			     pts[i]. y = v_pix(wn,y);
			   }
}

/* Rotate (x1,y1) and (x2, y2) by ROT_DEGREE(frame) */
void rotate_line(WYSIWYG *wn, int frame, PPOINT *x1, PPOINT *y1, PPOINT *x2, PPOINT *y2)
{
			   ROT rotation;
			   int frameAngle = ROT_DEGREE(frame);
			   /* Rotate the frame backwards */
			   set_rot_origin(wn, frame, &rotation);
			   rotation.degree = frameAngle;
/* Convert pixels to units in order to be able to use do_rot */
			   *x1 = pix_h(wn, *x1);
			   *x2 = pix_h(wn, *x2);
			   *y1 = pix_v(wn, *y1);
			   *y2 = pix_v(wn, *y2);
			   do_rot(wn,&rotation,x1,y1);
			   do_rot(wn,&rotation,x2,y2);
			   *x1 = h_pix(wn, *x1);
			   *x2 = h_pix(wn, *x2);
			   *y1 = v_pix(wn, *y1);
			   *y2 = v_pix(wn, *y2);
}




