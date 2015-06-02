/*    *****     **    **       *       ******     *******    *******
     **   **    **    **      ***      **   **    **         **    **
     **         **    **     ** **     **   **    **         **     **
     **         **    **    **   **    **   **    **         **     **
      *****     ********    *******    ******     *****      **     **
          **    **    **    **   **    ****       **         **     **
          **    **    **    **   **    ** **      **         **     **
     **   **    **    **    **   **    **  **     **         **    **
      *****     **    **    **   **    **   **    *******    *******  */

#include "window.h"
#include "link_util.h"
#include "color.h"
#include "traces.h"

extern void FreeXmString();		/* Do not put args inside, 
								   because it uses Xm header file */

static int not_MasterPage = 1;

void set_MasterPage(int new_value)
{
	not_MasterPage = new_value;
}

void clean_file_list(fl)
FILE_LIST *fl;
{
#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "fl -> Qmap_data Q\n");
#endif
	Qclear(&fl -> Qmap_data);
	if (fl -> label)
		FreeXmString(fl -> label, __FILE__, __LINE__);
	p_free((char *)fl);
}

static void clean_files_que(QUEUE *files)
{
	FILE_LIST *file;
	
#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "files Q\n");
#endif
	while ((file = (FILE_LIST *)QremoveHead(files)) != NULL)
		clean_file_list(file);
}

void clean_line_image(li)
LINE_IMAGE *li;
{
	COLOR_IMAGE *image, *next_image;
	
	Qclear(&li -> misc_images);
	image = (COLOR_IMAGE *)li -> color_image.head;
	while (image)
	{
		next_image = (COLOR_IMAGE *)image -> que.next;
		p_free((char *)image -> dots);
		p_free((char *)image);
		image = next_image;
	}
	p_free((char *)li);
}

void clean_items_que(QUEUE *items)
{
	LINE_IMAGE *li, *next_li;
	
	li = (LINE_IMAGE *)items -> head;
	while (li)
	{
		next_li = (LINE_IMAGE *)li -> que.next;
#ifdef TRACE
		if(trace_alloc)
			p_info(PI_TRACE, "items Q\n");
#endif
		clean_line_image(li);
		li = next_li;
	}
	
	items->head = items->tail = NULL;
}

void clean_draw_point_x_y(DRAW_POINT_X_Y *ele_list, int num_pts)
{
	int i;
	DRAW_POINT_X_Y *list;

	if(ele_list != NULL)
	{
		for (i = 0, list = ele_list; i < num_pts; i++, list++)
			if (list -> arc)
				p_free((char *)list -> arc);

		p_free((char *)ele_list);
	}
}

void clean_ele(ELEMENT *ele)
{
	if(ele -> out_lst_pts != NULL &&
	   ele -> list_points != ele -> out_lst_pts)
	{
#ifdef TRACE
		if(trace_alloc)
			p_info(PI_TRACE, "old_lst_pts\n");
#endif
		clean_draw_point_x_y(ele -> out_lst_pts, ele -> n_points);
	}
	if(ele -> list_points != NULL)
	{
#ifdef TRACE
		if(trace_alloc)
			p_info(PI_TRACE, "list_points\n");
#endif
		clean_draw_point_x_y(ele -> list_points, ele -> n_points);
	}
	if(ele -> trap_points != NULL)
	{
#ifdef TRACE
		if(trace_alloc)
			p_info(PI_TRACE, "trap_points\n");
#endif
		clean_draw_point_x_y(ele -> trap_points, ele -> trap_pts);
	}

	clean_items_que(&ele -> items);
	clean_items_que(&ele -> oitems);
#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "ele\n");
#endif
	Qclear(&ele -> shape);
	p_free((char *)ele);
}

void clean_ele_list(ELEMENT *ele)
{
	ELEMENT *next;

	while (ele)
	{
		next = ele -> next;
		clean_ele(ele);
		ele = next;
	}
}

void clean_frame(FRAME *frame)
{
	clean_ele_list(frame -> ele);
	frame -> ele = NULL;
	clean_ele_list(frame -> neg_ele);
	frame -> neg_ele = NULL;
	if (frame -> Fmap_data)
	{
		p_free((char *)frame -> Fmap_data);
		frame -> Fmap_data = NULL;
	}
	if (frame -> gr)
	{
		p_free((char *)frame -> gr);
		frame -> gr = NULL;
	}
	if (frame -> out_lst_pts &&
		frame -> out_lst_pts != frame -> list_points)
	{
		p_free((char *)frame -> out_lst_pts);
		frame -> out_lst_pts = NULL;
	}
	if (frame -> list_points)
	{
		p_free((char *)frame -> list_points);
		frame -> list_points = NULL;
		frame -> out_lst_pts = NULL;
	}
	if (frame -> bearoff_lst)
	{
		p_free((char *)frame -> bearoff_lst);
		frame -> bearoff_lst = NULL;
	}
	if (frame -> shape_pts)
	{
		p_free((char *)frame -> shape_pts);
		frame -> shape_pts = NULL;
	}
	if (frame -> bearoff_pts)
	{
		p_free((char *)frame -> bearoff_pts);
		frame -> bearoff_pts = NULL;
	}
	if(not_MasterPage)
	{
		if (frame -> rel_data.p0)
		{
			p_free((char *)frame -> rel_data.p0);
			frame -> rel_data.p0 = NULL;
		}
		if (frame -> rel_data.p1)
		{
			p_free((char *)frame -> rel_data.p1);
			frame -> rel_data.p1 = NULL;
		}
		if (frame -> rel_data.p2)
		{
			p_free((char *)frame -> rel_data.p2);
			frame -> rel_data.p2 = NULL;
		}
		if (frame -> rel_data.p3)
		{
			p_free((char *)frame -> rel_data.p3);
			frame -> rel_data.p3 = NULL;
		}
	}
	if (frame -> w_boxes)
	{
		p_free((char *)frame -> w_boxes);
		frame -> w_boxes = NULL;
	}
}

void clean_linked_frame(layout)
LAYOUT_DESC *layout;
{
	LINKED_FRAME *l_frame;
	
	if (layout -> link_frame.head)
	{
		l_frame = (LINKED_FRAME *)layout -> link_frame.head;
		while (l_frame)
		{
			Qclear(&l_frame -> list_frame);
			l_frame = (LINKED_FRAME *)l_frame -> que.next;
		}
		Qclear(&layout -> link_frame);
	}
}

static void clean_color(wn)
WYSIWYG *wn;
{
	int i;
	
	if (wn -> LLcolor && wn -> LLcolor->count)
	{
		for (i = 0; i < wn -> LLcolor->count; i++)
			if (wn -> LLcolor -> data[i])
				p_free(wn -> LLcolor -> data[i]);
		wn -> LLcolor->count = 0;
		wn -> LLcolor->size = 0;
		wn -> LLcolor->Sorted = FALSE;
		p_free((char *)wn -> LLcolor -> data);
		p_free((char *)wn -> LLcolor);
		wn -> LLcolor = NULL;
	}
}

void clean_layout(LAYOUT_DESC *layout)
{
	int i;

	if (layout -> menu_info)
		p_free((char *)layout -> menu_info);
	if (layout -> fl)
		clean_file_list(layout -> fl);
	clean_items_que(&layout -> items);
	if(layout -> f_chunks.head)
	{
		for(i = 0; i <= layout -> num_frames; i++)
			clean_frame(layout -> frames[i]);
#ifdef TRACE
		if(trace_alloc)
			p_info(PI_TRACE, "chuncks Q\n");
#endif
		Qclear(&layout -> f_chunks);
	}
	if(layout -> frames)
	{
#ifdef TRACE
		if(trace_alloc)
			p_info(PI_TRACE, "frames\n");
#endif
		p_free((char *)layout -> frames);
	}
	clean_linked_frame(layout);
	p_free((char *)layout);
}

void clean_wysiwyg(WYSIWYG *wn)
{
	LAYOUT_DESC *layout, *lay_next;

#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "hor_guidelines\n");
#endif
	Qclear(&CUR_LOCKS(wn).hor_guidelines);
#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "ver_guidelines\n");
#endif
	Qclear(&CUR_LOCKS(wn).ver_guidelines);
#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "locked_files\n");
#endif
	Qclear(&wn -> locked_files);
#ifdef TRACE
	if(trace_alloc)
		p_info(PI_TRACE, "crosshairs\n");
#endif
	Qclear(&CUR_LOCKS(wn).crosshairs);
	if (wn -> files.head != qNULL)
		clean_files_que(&wn -> files);
	layout = (LAYOUT_DESC *)wn -> layout_list.head;
	while (layout)
	{
		lay_next = (LAYOUT_DESC *)layout -> que.next;
		QremoveElement(&wn -> layout_list, (QE *)layout);
		clean_layout(layout);
		layout = lay_next;
	}
	clean_color(wn);
	if (wn -> X11_vars)
		p_free((char *)wn -> X11_vars);
	p_free((char *)wn);
}
