#ifndef _MAP_H

#define _MAP_H

#define MAP_FLOW_MASK	0x0007
#define BF	0x0001				/* Start at first line */
#define EF	0x0002				/* End at last line */
#define SF	0x0004				/* Suspend flow */
#define LF	0x0008				/* Previous frame has a suspend flow */
#define MAP_ERROR_MASK	0x0070
#define WE	0x0010				/* Widow error */
#define OE	0x0020				/* Orphan error */
#define BE	0x0040				/* Break error */

#define MAP_ALL_TEXT_FILE	0	/* Scan map all the text file */
#define MAP_LAYOUT			-1	/* Map layout only */

typedef struct map_data
	{
	QE que;						/* pointer to next & previous items */
	uint16	start_line;			/* start line num in .fo	*/
	int16	start_forec;		/* start rec in .fo			*/
	int16	start_fowrd;		/* start word in .fo		*/
	uint16	end_line;			/* end line num in .fo		*/
	int16	end_forec;			/* end rec in .fo			*/
	int16	end_fowrd;			/* end word in .fo			*/
	int16	underflow_depth;	/* total lead remainline in frame	*/
	uint16	overflow_line;		/* overflow line num in .fo	*/
	int16	overflow_forec;		/* overflow rec in .fo		*/
	int16	overflow_fowrd;		/* overflow word in .fo		*/
	int32	overflow_depth;		/* total lead in overflow	*/
	int16	tab_split;			/* Is frame split across facing pages?
								   0=No(normal)  1=Left half  2=Right half */
	int32	layout_id;			/* layout number of this data */
	int16	frame_id;			/* st or ff# per block type	*/
	int16	frame_num;			/* Frame number inside layout */
	int16	piece;				/* element number in this frame */
	int16	of_p;				/* number of elements in this frame */
	uint16	command_flags;		/* Possible value: BF, EF, SF	*/
	uint16	error_flags;		/* Possible value: WE, OE, BE	*/
	int16	prev_lay_id;		/* layout_id & frame_id of prev in flow. */
	int16	prev_frame_id;
	int16	next_lay_id;		/* layout_id & frame_id of next in flow. */
	int16	next_frame_id;
							/* Next 5 internal to H&J, not stored on disk: */
	int16	hnj_index;			/* Index 1-n of lines in .map file.
									Generated during map_load(). */
	int16	hnj_start_index;	/* For BF entry: hnj_index of start of flow. */
	int16	hnj_end_index;		/* For BF entry: hnj_index of end of flow. */
	uint16	hnj_start_line;		/* For BF entry: Line# of start of flow. */
	uint16	hnj_end_line;		/* For BF entry: Line# of end of flow. */

	int		vj_top;				/* vertical justification data */
	int		vj_line_cnt;
	int		vj_line_adj;
	int		vj_para_cnt;
	int		vj_para_adj;
	int		vj_exld_cnt;
	int		vj_exld_adj;
	int		vj_vbnd_cnt;
	int		vj_vbnd_adj;
	int		vj_mode;			/* vertical just mode 0x0000h0ta */
								/* where:
									a = (click line) auto adjust value
										1 = adjust at lines
										2 = adjust at paragraphs
										3 = adjust at extra lead
										4 = adjust at vert bands
										5 = adjust at top
										6 = adjust at center
									t = top line adjust modes
										0x00 = no expansion on top line
										0x10 = treat as normal line
											   for feathering and
											   extra lead adjust
										0x20 = undefined, no expansion
										0x30 = also treat as first
											   line of paragraph
									h = special hold flags
										0x4000 = force interface not to 
											clear frame if it changes size	*/
	int		vj_try;
	int		vj_depth_adj;
	} MAP_DATA;

#endif
