/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    **
     *****     **    **    **   **    **   **    *******    *******   */

#include <stdio.h>
#include "rel_data.h"
#include "frame.h"
#include "link_util.h"
#include "traces.h"
#include "lmt.f"

extern void clean_linked_frame(LAYOUT_DESC *layout);

static void build_list_elements(WYSIWYG *wn)
{
	ELEMENT *ele;
	int piece;
	LINKED_FRAME *l_frame;
	FR_LINK *p_frame;
	ELEMENT *prev_ele = NULL;

	for (l_frame = (LINKED_FRAME *)LAYOUT(wn) link_frame.head; l_frame;
		 l_frame = (LINKED_FRAME *)l_frame -> que.next)
	{
		for (p_frame = (FR_LINK *)l_frame -> list_frame.head; p_frame;
			 p_frame = (FR_LINK *)p_frame -> que.next)
		{
			piece = 0;
			for (ele = FRAME_DATA(p_frame -> frame_number) ele; ele;
				 ele = ele -> next)
			{
				ele -> map_data.frame_num = p_frame -> frame_number;
				if (!ele -> next && p_frame -> frame -> Fmap_data
					&& p_frame -> frame -> Fmap_data -> command_flags & SF)
					ele -> map_data.command_flags = SF;
				else
					ele -> map_data.command_flags = 0;
				if (!ele -> next && p_frame -> frame -> Fmap_data
					&& p_frame -> frame -> Fmap_data -> command_flags & LF)
					ele -> map_data.command_flags |= LF;
				if (TYPE_OF_FRAME(ele -> map_data.frame_num) == PL_TEXT)
					ele -> map_data.frame_id =
						(int16)ARTICLE_ID(p_frame -> frame_number);
				else
					ele -> map_data.frame_id = TEXT_ID(p_frame-> frame_number);
				ele -> map_data.piece = ++piece;
				ele -> rev = prev_ele;
				ele -> fwd = NULL;
				if (prev_ele)
					prev_ele -> fwd = ele;
				prev_ele = ele;
			}
			for (ele = FRAME_DATA(p_frame -> frame_number) ele; ele;
				 ele = ele -> next)
			{
				ele -> map_data.of_p = piece;
				if (ele -> map_data.piece == piece 
					&& p_frame -> frame -> Fmap_data
					&& p_frame -> frame -> Fmap_data -> overflow_line
					> ele -> map_data.overflow_line)
								/* Just so will see the overflow symbol */
				{
					ele -> map_data.overflow_line =
						p_frame -> frame -> Fmap_data -> overflow_line;
					ele -> map_data.overflow_forec =
						p_frame -> frame -> Fmap_data -> overflow_forec;
					ele -> map_data.overflow_fowrd =
						p_frame -> frame -> Fmap_data -> overflow_fowrd;
				}
				if (ele -> map_data.piece == piece && !p_frame -> que.next
					&& p_frame -> frame -> Fmap_data
					&& p_frame -> frame -> Fmap_data -> command_flags & EF)
					ele -> map_data.command_flags |= EF;
			}
		}
		prev_ele = NULL;
		for (p_frame = (FR_LINK *)l_frame -> list_frame.head; p_frame;
			 p_frame = (FR_LINK *)p_frame -> que.next)
			if (p_frame -> frame -> Fmap_data && p_frame -> frame -> ele &&
				p_frame -> frame -> Fmap_data -> command_flags & BF)
			{
				p_frame -> frame -> ele -> map_data.command_flags |= BF;
				break;
			}
	}
}

void trace_link_frame(WYSIWYG *wn)
{
	LINKED_FRAME *l_frame;
	FR_LINK *p_frame;

	if (trace_lmt)
	{
		int k;

		if (wn -> selected_lay && LAYOUT(wn) fl)
			p_info(PI_TRACE, "trace_link_frame layout: %s\n",
					  LAYOUT(wn) fl -> filename);
		l_frame = (LINKED_FRAME *)LAYOUT(wn) link_frame.head;
		while (l_frame)
		{
			p_info(PI_TRACE, "Article_id: %d, text id: %d\n",
					  l_frame -> article_id, l_frame -> text_id);
			
			p_frame = (FR_LINK *)l_frame -> list_frame.head;
			while (p_frame)
			{
				p_info(PI_TRACE, "Link number: %d, frame number: %d\n",
						  p_frame -> frame -> rel_data.LK_NUM,
						  p_frame -> frame_number);
				p_frame = (FR_LINK *)p_frame -> que.next;
			}
			l_frame = (LINKED_FRAME *)l_frame -> que.next;
		}
		for(k = 1; k <= LAYOUT(wn) num_frames; k++)
		{
      		ELEMENT *ele;

			if (TYPE_OF_FRAME(k) != PL_FLOW || !FRAME_DATA(k) Fmap_data)
				continue;
			p_info(PI_TRACE, "k: %d, frame_num: %d, lay_id: %d, frame_id: %d \
command: %d\n",
					  k, FRAME_DATA(k) Fmap_data -> frame_num,
					  FRAME_DATA(k) Fmap_data -> layout_id,
					  FRAME_DATA(k) Fmap_data -> frame_id,
					  FRAME_DATA(k) Fmap_data -> command_flags);
      		ele = FRAME_DATA(k) ele;
      		while(ele)
			{
				p_info(PI_TRACE, "frame_num: %d, lay_id: %d, frame_id: %d \
command: %d\n		piece %d/%d, id: %d, d10: %ld, d13: %ld, \
d16: %ld, d17: %ld\n",
						  ele -> map_data.frame_num,
						  ele -> map_data.layout_id, ele -> map_data.frame_id,
						  ele -> map_data.command_flags,
						  ele -> map_data.piece, ele -> map_data.of_p,
						  TEXT_ID(k), PREV_LAY(k), PREV_TEXT_ID(k),
						  NEXT_LAY(k), NEXT_TEXT_ID(k));
				ele = ele -> next;
			}
		}
	}
}

void lmt_page_elements(WYSIWYG *wn)
{
	LINKED_FRAME *l_frame;
	FR_LINK *p_frame, *new_p_frame;
	int i;

	clean_linked_frame(wn -> selected_lay);
	for (i = 1; i <= NEW_TOT_BLOCKS; i++)
	{
		if (!LINK_NUMBER(i))	/* No link number */
			continue;
		l_frame = (LINKED_FRAME *)LAYOUT(wn) link_frame.head;
		while (l_frame)
		{
			if (l_frame -> article_id == ARTICLE_ID(i)
				&& l_frame -> text_id == TEXT_ID(i))
				break;
			l_frame = (LINKED_FRAME *)l_frame -> que.next;
		}
		if (!l_frame)
		{						/* New article number/text id */
			l_frame = (LINKED_FRAME *)p_alloc(sizeof(LINKED_FRAME));
			l_frame -> article_id = ARTICLE_ID(i);
			l_frame -> text_id = TEXT_ID(i);
			QinsertTail(&LAYOUT(wn) link_frame, (QE *)l_frame);
		}
		p_frame = (FR_LINK *)l_frame -> list_frame.head;
		while (p_frame)
		{
			if (LINK_NUMBER(i) < p_frame -> frame -> rel_data.LK_NUM)
			{
				new_p_frame = (FR_LINK *)p_alloc(sizeof(FR_LINK));
				new_p_frame -> frame_number = i;
				new_p_frame -> frame = LAYOUT(wn) frames[i];
				QinsertBefore(&l_frame -> list_frame, (QE *)p_frame,
							  (QE *)new_p_frame);
				break;
			}
			p_frame = (FR_LINK *)p_frame -> que.next;
		}
		if (!p_frame)
		{
			new_p_frame = (FR_LINK *)p_alloc(sizeof(FR_LINK));
			new_p_frame -> frame_number = i;
			new_p_frame -> frame = LAYOUT(wn) frames[i];
			QinsertTail(&l_frame -> list_frame, (QE *)new_p_frame);
		}
		l_frame -> tot_links++;
	}
	build_list_elements(wn);
	trace_link_frame(wn);
}
