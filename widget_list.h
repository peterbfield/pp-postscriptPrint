#ifndef WIDGET_LIST_H
#define WIDGET_LIST_H

/* All values must agree w/ widget_list.uil */
#define wig_f_weight                            3

#define wig_p_form				0
#define wig_p_scroll			1
#define wig_p_draw				2
#define wig_p_rulers			3
#define wig_p_top_rule			4
#define wig_p_left_rule			5
#define wig_p_tool_select_frame	6
#define wig_p_menu_bar			7
#define wig_p_position			8
#define wig_p_name_list			9

#define wig_max_widget	9
#define MAX_WIDGETS (wig_max_widget + 1)

/* Values correspond to the entries found in frame_io.c */
#define wig_p_pt_sz			 1			/* Specs menu */
#define wig_p_ld			 2			/* Specs menu */
#define wig_f_graphic_nm     4
#define wig_frame_locks      6                                  /* Lockpoint widget in the frame menu */
#define wig_p_horz           7
#define wig_p_vert           8
#define wig_p_width          9
#define wig_p_depth         10
#define wig_p_g_top			11			/* Gutters menu */
#define wig_p_g_bottom		12			/* Gutters menu */
#define wig_p_g_left		13			/* Gutters menu */
#define wig_p_g_right		14			/* Gutters menu */
#define wig_p_layout_window 15			/* Layout menu */
#define wig_frame_tl_lock   16                  /* TL lockpoint button in the frame menu (the smallest)*/
#define wig_p_specs_window  29			/* Specs menu */
#define wig_p_edit_frame	30			/* Open frame menu */
#define wig_p_layer_bg      37
#define wig_p_cap_hgt       38			/* Specs menu */
#define wig_p_text_style    41			/* Specs menu */
#define wig_f_rot_deg       43
/* 42 to 45 in use in widget_list.uil */
#define wig_p_overlay       46			/* Specs menu */
#define wig_p_group         47			/* Specs menu */
#define wig_p_num_col		59			/* Specs menu */
#define wig_p_illus_style	61			/* Specs menu */
/*#define 					62			NOT USED */
/*#define 					63			NOT USED */
/*#define 					64			NOT USED */
/*#define 					65			NOT USED */
#define wig_p_custom_width	66			/* Specs menu */
#define wig_p_custom_depth	67			/* Specs menu */
#define wig_p_page_width	68			/* Specs menu */
#define wig_p_page_depth	69			/* Specs menu */
#define wig_p_page_horz		70			/* Specs menu */
#define wig_p_page_vert		71			/* Specs menu */
#define wig_p_margin_top	72			/* Specs menu */
#define wig_p_margin_bottom	73			/* Specs menu */
#define wig_p_margin_left	74			/* Specs menu */
#define wig_p_margin_right	75			/* Specs menu */
#define wig_p_col_width		76			/* Specs menu */
#define wig_p_row_depth		77			/* Specs menu */
#define wig_p_col_gutter	78			/* Specs menu */
#define wig_p_row_gutter	79			/* Specs menu */
#define wig_p_num_row		80			/* Specs menu */
#define wig_p_frame_type	81			/* Specs menu */
/* 84 to 95 used by rotational lock */
/*#define					96				NOT USED */
#define wig_p_text			97			/* Specs menu */
#define wig_p_graphic		98			/* Specs menu */
#define wig_p_rule_box		99			/* Specs menu */
#define wig_p_design		303			/* Specs menu */
#define wig_p_trim_type    105			/* Specs menu */
#define wig_p_letter       106			/* Specs menu */
#define wig_p_legal        107			/* Specs menu */
#define wig_p_Bsize        108			/* Specs menu */
#define wig_p_custom       109			/* Specs menu */
#define wig_p_st_sz        111			/* Specs menu */
#define wig_p_zoom_text      112		/* Custom display menu */
#define wig_p_zoom_opt  	 113		/* Layout menu */
#define wig_p_auto_fit		 114		/* Layout menu */
#define wig_p_zoom_100		 115		/* Layout menu */
#define wig_p_zoom_75		 116		/* Layout menu */
#define wig_p_zoom_50		 117		/* Layout menu */
#define wig_p_zoom_25		 118		/* Layout menu */
#define wig_p_zoom_custom	 119		/* Layout menu */
#define wig_p_text_disp		 120		/* Layout menu */
#define wig_p_text_frm		 121		/* Layout menu */
#define wig_p_text_label	 122		/* Layout menu */
#define wig_p_text_wys		 123		/* Layout menu */
#define wig_p_text_f_wys	 124		/* Layout menu */
#define wig_p_graph_disp	 125		/* Layout menu */
#define wig_p_graph_frm		 126		/* Layout menu */
#define wig_p_graph_label	 127		/* Layout menu */
#define wig_p_graph_wys		 128		/* Layout menu */
#define wig_p_graph_f_wys	 129		/* Layout menu */
#define wig_p_h_rul_org		 130		/* Rulers menu */
#define wig_p_h_rul_trim	 131		/* Rulers menu */
#define wig_p_h_rul_page	 132		/* Rulers menu */
#define wig_p_h_rul_margin	 133		/* Rulers menu */
#define wig_p_h_rul_frame	 134		/* Rulers menu */
#define wig_p_h_rul_frame_t	 135		/* Rulers menu */
#define wig_p_v_rul_org		 136		/* Rulers menu */
#define wig_p_v_rul_trim	 137		/* Rulers menu */
#define wig_p_v_rul_page	 138		/* Rulers menu */
#define wig_p_v_rul_margin	 139		/* Rulers menu */
#define wig_p_v_rul_frame	 140		/* Rulers menu */
#define wig_p_v_rul_frame_t	 141		/* Rulers menu */
#define wig_p_h_rul_units	 142		/* Rulers menu */
#define wig_p_h_rul_pica	 143		/* Rulers menu */
#define wig_p_h_rul_in_10	 144		/* Rulers menu */
#define wig_p_h_rul_in_32	 145		/* Rulers menu */
#define wig_p_h_rul_cm		 146		/* Rulers menu */
#define wig_p_v_rul_units	 147		/* Rulers menu */
#define wig_p_v_rul_pica	 148		/* Rulers menu */
#define wig_p_v_rul_in_10	 149		/* Rulers menu */
#define wig_p_v_rul_in_32	 150		/* Rulers menu */
#define wig_p_v_rul_cm		 151		/* Rulers menu */
#define wig_p_v_rul_ln		 152		/* Rulers menu */
#define wig_p_v_rul_ln_t	 153		/* Rulers menu */
#define wig_p_rul_window	 154		/* Rulers menu */
#define wig_p_col_gd_cols	 155		/* Column guides menu */
#define wig_p_col_gd_wid	 156		/* Column guides menu */
#define wig_p_col_gd_gut_w	 157		/* Column guides menu */
#define wig_p_col_gd_rows	 158		/* Column guides menu */
#define wig_p_col_gd_dep	 159		/* Column guides menu */
#define wig_p_col_gd_gut_d	 160		/* Column guides menu */
#define wig_p_col_gd_orig	 161		/* Column guides menu */
#define wig_p_col_gd_trim	 162		/* Column guides menu */
#define wig_p_col_gd_page	 163		/* Column guides menu */
#define wig_p_col_gd_margin	 164		/* Column guides menu */
#define wig_p_col_gd_window  165		/* Column guides menu */
#define wig_p_snap_from      166		/* Snaps menu */
#define wig_p_snap_fr_form   167		/* Snaps menu */
#define wig_p_snap_fr_edge   168		/* Snaps menu */
/*#define						 169			NOT USED */
/*#define						 170			NOT USED */
#define wig_p_snap_to_v		 171		/* Snaps menu */
#define wig_p_snap_trim_v    172		/* Snaps menu */
#define wig_p_snap_page_v    173		/* Snaps menu */
#define wig_p_snap_margin_v  174		/* Snaps menu */
#define wig_p_snap_fr_edge_v 175		/* Snaps menu */
#define wig_p_snap_crosshr1_v 176		/* Snaps menu */
#define wig_p_snap_crosshr2_v 177		/* Snaps menu */
#define wig_p_snap_col_gd_v   178		/* Snaps menu */
#define wig_p_snap_leading_v  179		/* Snaps menu */
#define wig_p_snap_to_h		  180		/* Snaps menu */
#define wig_p_snap_trim_h     181		/* Snaps menu */
#define wig_p_snap_page_h     182		/* Snaps menu */
#define wig_p_snap_margin_h   183		/* Snaps menu */
#define wig_p_snap_fr_edge_h  184		/* Snaps menu */
#define wig_p_snap_crosshr1_h 185		/* Snaps menu */
#define wig_p_snap_crosshr2_h 186		/* Snaps menu */
#define wig_p_snap_col_gd_h   187		/* Snaps menu */
#define wig_p_snaps_window    188		/* Snaps menu */
#define wig_p_disp_opts       189		/* Layout menu */
#define wig_p_disp_col_gd     190		/* Layout menu */
#define wig_p_disp_cross_hr1  191		/* Layout menu */
#define wig_p_disp_cross_hr2  192		/* Layout menu */
#define wig_p_disp_rulers     193		/* Layout menu */
#define wig_p_disp_trim       194		/* Layout menu */
#define wig_p_disp_page       195		/* Layout menu */
#define wig_p_disp_margin     196		/* Layout menu */
#define wig_p_closed_box      197		/* Layout menu */
#define wig_p_track_mouse     198		/* Layout menu */
#define wig_p_auto_size       199		/* Layout menu */
#define wig_p_rule_weight_text 200		/* R/B weight pref. menu */
#define wig_p_box_weight_text  201		/* R/B weight pref. menu */
#define wig_p_custom_zoom_menu 202		/* Custom display menu */
#define wig_p_box_menu         203		/* R/B weight pref. menu */
#define wig_p_gutters_menu     204		/* Gutters menu */
#define wig_p_lay_hort         205		/* Layout menu */
#define wig_p_lay_vert         206		/* Layout menu */
#define wig_p_lay_width        207		/* Layout menu */
#define wig_p_lay_depth        208		/* Layout menu */
#define wig_p_lay_locks        209		/* Layout menu */
				/* 210 to 221 */		/* Layout menu */
#define wig_p_disp_tints       222		/* Layout menu */
#define wig_p_disp_links       223		/* Layout menu */
#define wig_p_lay_hort_l       224		/* Layout menu */
#define wig_p_lay_vert_l       225		/* Layout menu */
#define wig_p_lay_width_l      226		/* Layout menu */
#define wig_p_lay_depth_l      227		/* Layout menu */
#define wig_p_custom_width_l   228		/* Specs menu */
#define wig_p_custom_depth_l   229		/* Specs menu */
/*#define						   230		NOT USED */
/*#define						   231		NOT USED */
#define wig_p_default_button   232
#define wig_p_stf_window       233
#define wig_p_vjt_window       234
#define wig_p_vjt_option	   235		/* Layout menu */
#define wig_p_vjt_lines		   236
#define wig_p_vjt_paragraph	   237
#define wig_p_vjt_extra_lead   238
#define wig_p_vjt_expand_pts   239
#define wig_p_vjt_top		   240
#define wig_p_vjt_bottom	   241
#define wig_p_vjt_custom	   242
#define wig_p_vjt_reset		   243
#define wig_p_stf_option	   244
#define wig_p_stf_measure	   245
#define wig_p_stf_pt_sz		   246
#define wig_p_stf_set_sz	   247
#define wig_p_stf_leading	   248
#define wig_p_stf_bands		   249
#define wig_p_stf_width		   250
#define wig_p_stf_minimum	   251
#define wig_p_stf_maximum	   252
#define wig_p_stf_custom	   253
#define wig_p_stf_reset		   254
#define wig_p_list_dots        255
#define wig_p_stf_cust_menu    256
#define wig_p_vjt_cust_menu    257
#define wig_p_vjt_stat_lines1  258
#define wig_p_vjt_stat_lines2  259
#define wig_p_vjt_stat_para1   260
#define wig_p_vjt_stat_para2   261
#define wig_p_vjt_stat_el1     262
#define wig_p_vjt_stat_el2     263
#define wig_p_vjt_stat_expts1  264
#define wig_p_vjt_stat_expts2  265
#define wig_p_vjt_stat_top1    266
#define wig_p_vjt_stat_top2    267
#define wig_p_vjt_stat_bottom1 268
#define wig_p_vjt_stat_bottom2 269
#define wig_p_vjt_cust_option  270
#define wig_p_vjt_cust_lines   271
#define wig_p_vjt_cust_para    272
#define wig_p_vjt_cust_el      273
#define wig_p_vjt_cust_ex_pts  274
#define wig_p_vjt_cust_top     275
#define wig_p_vjt_cust_bottom  276
#define wig_p_vjt_cust_table   277
#define wig_p_vjt_lines_fixed  278
#define wig_p_vjt_lines_max    279
#define wig_p_vjt_para_fixed   280
#define wig_p_vjt_para_max     281
#define wig_p_vjt_el_fixed     282
#define wig_p_vjt_el_max       283
#define wig_p_vjt_ex_pts_fixed 284
#define wig_p_vjt_ex_pts_max   285
#define wig_p_vjt_top_fixed    286
#define wig_p_vjt_top_max      287
#define wig_p_vjt_bottom_fixed 288
#define wig_p_vjt_bottom_max   289
#define wig_p_vjt_table_select 290
#define wig_f_design_text      291
#define wig_f_fg_attribute     293
#define wig_f_open_pg_text     294		/* Open page menu */
#define wig_f_open_pg_list     295		/* Open page menu */
#define wig_f_open_pg_menu     296		/* Open page menu */
#define wig_f_facing_pages     297		/* Layout menu */
#define wig_f_step_repeat	   298		/* Step/repeat menu */
#define wig_f_step_repeat_t1   299		/* Step/repeat text 1 */
#define wig_f_step_repeat_t2   300		/* Step/repeat text 2 */
#define wig_f_step_repeat_t3   301		/* Step/repeat text 3 */
#define wig_f_step_repeat_t4   302		/* Step/repeat text 4 */
#define wig_f_outline		   315
#define wig_f_out_color		   316
#define wig_f_out_weight	   317
#define wig_position		   318		/* Layout position frame */
#define wig_rotation		   319		/* Layout position frame */
#define wig_l_old_angle		   320		/* Layout position frame */
#define wig_l_var_angle		   321		/* Layout position frame */
#define wig_l_new_angle		   322		/* Layout position frame */
#define wig_reflow			   323		/* Reflow menu */
#define wig_reflow_vj		   324		/* Reflow menu clear vj */
#define wig_reflow_locks	   325		/* Reflow menu clear locks */
#define wig_reflow_both		   326		/* Reflow menu clear both */
#define wig_rel_frame_num	   327
#define wig_f_mp1			   328
#define wig_f_mp_text		   329
#define wig_f_mp_CL			   330
#define wig_f_mp_SN			   331
#define wig_f_mp_AR			   332
#define wig_f_mp2			   333
#define wig_f_mp_qnone		   334
#define wig_f_mp_qleft		   335
#define wig_f_mp_qcenter	   336
#define wig_f_mp_qright		   337
#define wig_f_typeset		   338
#define wig_p_page_style	   339
#define wig_p_try_table		   340
#define wig_p_w_baseline	   341
#define wig_p_cap_height	   342
#define wig_p_layer_fg  	   343
#define wig_f_out_trap 		   344				/* TMS */
#define wig_f_rad_top_left	   345
#define wig_f_rad_top_right	   346		/* In use if we go to diff. radius for each corner. */
#define wig_f_rad_bottom_left  347		/* In use if we go to diff. radius for each corner. */
#define wig_f_rad_bottom_right 348		/* In use if we go to diff. radius for each corner. */
#define wig_rounded			   349		/* Layout position frame */
#define wig_l_rounded		   350		/* Layout position frame */
#define wig_p_gr_crop_top      351              /* TMS */
#define wig_p_gr_crop_left     352              /* TMS */
#define wig_p_gr_crop_bottom   353              /* TMS */
#define wig_p_gr_crop_right    354              /* TMS */
#define wig_p_gr_zoomx_gr      355              /* TMS */
#define wig_f_fg_shade         361
#define wig_f_bg_shade         362
#define wig_f_shade            363
#define wig_p_gr_zoomy_gr      364              /* TMS */
#define wig_reflow_lockf       365
#define wig_reflow_entire      366
#define wig_reflow_fromtoend   367
#define wig_reflow_fromtostart 368
#define wig_reflow_fromsel     369
#define wig_reflow_text        370
#define wig_blend_color_start  371
#define wig_blend_color_end    372
#define wig_blend_angle        373
#define wig_dash_on            374
#define wig_dashwid            375
#define wig_gapwid             376
#define wig_odash_on            377
#define wig_odashwid            378
#define wig_ogapwid             379
#define wig_blend_shade_start   380
#define wig_blend_shade_end     381

#define wig_l_hort_text         wig_p_lay_hort
#define wig_l_vert_text         wig_p_lay_vert
#define wig_l_width_text        wig_p_lay_width
#define wig_l_depth_text        wig_p_lay_depth

/* Start widgets for Color/Palette Menus */
#define wig_col_menu      0
#define wig_col_file      1
#define	wig_col_select    2
#define	wig_col_move      3
#define	wig_col_copy      4
#define	wig_col_create    5
#define	wig_col_open      6
#define	wig_col_exit      7
#define wig_col_como      8
#define	wig_col_def       9
#define	wig_col_treepr   10
#define wig_col_ok       11
#define	wig_col_cancel   12
#define wig_col_deftog   13
#define wig_col_cmtog    14
#define wig_col_coltab   15
#define	wig_col_selcol   16
#define	wig_col_add      17
#define	wig_col_change   18
#define	wig_col_delete   19
#define	wig_col_texit    20
#define	wig_col_tcancel  21
#define	wig_col_tsave    22
#define	wig_col_tsaveas  23
#define	wig_col_tgraph   24
#define wig_col_tcolor	 25
#define wig_col_label    26
#define	wig_col_edit     27
#define	wig_col_spot     28
#define	wig_col_process  29
#define	wig_col_overprint  30
#define	wig_col_spot1    31
#define	wig_col_spot2    32
#define	wig_col_spot3    33
#define	wig_col_spot4    34
#define	wig_col_spot5    35
#define	wig_col_spot6    36
#define	wig_col_colname  37
#define wig_col_tint     38
#define wig_col_scalec   39
#define	wig_col_scalem   40
#define	wig_col_scaley   41
#define	wig_col_scalek   42
#define	wig_col_esave	 43
#define	wig_col_ecancel	 44
#define	wig_col_eexit	 45
#define	wig_col_eseparat 46
#define	wig_col_epalettes 47
#define wig_col_ecolor	 48
#define wig_col_spots    49
#define wig_col_textc    50
#define wig_col_textm	 51
#define	wig_col_texty	 52
#define	wig_col_textk	 53
/* Separation Menu */
#define wig_col_smenu			54
#define wig_col_scompshape		55
#define wig_col_scompangle		56
#define wig_col_scompscreen		57
#define wig_col_scomppct		58
#define wig_col_scomplbltext	59
#define wig_col_splate1shape	60
#define wig_col_splate1angle	61
#define wig_col_splate1screen	62
#define wig_col_splate1pct		63
#define wig_col_splate1label	64
#define wig_col_splate2shape	65
#define wig_col_splate2angle	66
#define wig_col_splate2screen	67
#define wig_col_splate2pct		68
#define wig_col_splate2label	69
#define wig_col_splate3shape	70
#define wig_col_splate3angle	71
#define wig_col_splate3screen	72
#define wig_col_splate3pct		73
#define wig_col_splate3label	74
#define wig_col_splate4shape	75
#define wig_col_splate4angle	76
#define wig_col_splate4screen	77
#define wig_col_splate4pct		78
#define wig_col_splate4label	79
#define wig_col_splate5shape	80
#define wig_col_splate5angle	81
#define wig_col_splate5screen	82
#define wig_col_splate5pct		83
#define wig_col_splate5label	84
#define wig_col_splate6shape	85
#define wig_col_splate6angle	86
#define wig_col_splate6screen	87
#define wig_col_splate6pct		88
#define wig_col_splate6label	89
#define wig_col_sok				90
#define wig_col_scancel			91
/* Color Graphics Menu */
#define wig_col_gmenu			92
#define wig_col_gfgcolortext	93
#define wig_col_gbgcolortext	94
#define wig_col_goptmenu1		95
#define wig_col_goptmenu2		96
#define wig_col_goptmenu3		97
#define wig_col_goptmenu4		98
#define wig_col_goptmenu5		99
#define wig_col_goptmenu6		100
#define wig_col_gscrltxt1		101
#define wig_col_gscrltxt2		102
#define wig_col_gscrltxt3		103
#define wig_col_gscrltxt4		104
#define wig_col_gscrltxt5		105
#define wig_col_gscrltxt6		106
#define wig_col_gok				112
#define wig_col_gcancel			113
#define wig_col_adddial 114
#define wig_col_addtext	115
#define wig_col_addbut	116
#define wig_col_addcancel 117
#define	wig_col_credial    118
#define	wig_col_cretext	   119
#define	wig_col_crebut	   120
#define	wig_col_crecancel  121
#define wig_col_crelabel   122
#define wig_col_conok      123
#define wig_col_concancel  124
#define wig_col_confirm    125
#define wig_col_paltake    126
#define wig_col_ptext      127
#define wig_col_pselect    128
#define wig_col_popen      129
#define wig_col_pcexit     130
#define wig_col_pcolors    131
#define wig_col_pcol_sel   132
#define wig_col_pfilter    133
#define wig_col_pok        134
#define wig_col_pccancel   135
#define wig_col_esaveas    136
#define wig_col_supdate    137
#define wig_col_bwgraphics	138
/* If more widgets added - adjust COLOR_CONTROL_ITEMS */
#define COLOR_CONTROL_ITEMS  139

#define	wig_col_palmenu		0
#define	wig_col_palfile		1
#define	wig_col_palsel		2
#define	wig_col_palopen		3
#define	wig_col_palexit		4
#define	wig_col_editpal		5
#define	wig_col_selpalcol	6
#define	wig_col_filtext		7
#define	wig_col_pcolname	8
#define wig_col_pscalec		9
#define	wig_col_pscalem		10
#define	wig_col_pscaley		11
#define	wig_col_pscalek		12
#define	wig_col_pecolor		13
#define	wig_col_ptextc		14
#define	wig_col_ptextm		15
#define	wig_col_ptexty		16
#define	wig_col_ptextk		17
#define wig_col_psave		18
#define wig_col_pdelete		19
#define wig_col_pcancel		20
#define wig_col_pexit		21
#define wig_col_cr_palmenu	22
#define wig_col_cr_palfile	23
#define wig_col_fromname	24
#define wig_col_cr_palsel	25
#define wig_col_crok		26
#define wig_col_crcancel	27
#define wig_col_crexit		28
#define wig_col_fromtog		29
#define wig_col_paldelete	30
#define	wig_col_pconfirm	31
#define	wig_col_pconok		32
#define	wig_col_pconcancel	33
/* If more widgets added - adjust PALETTE_CONTROL_ITEMS */
#define PALETTE_CONTROL_ITEMS 34

#define wig_col_gpbcyan			200		/* More color graphics menu defs */
#define wig_col_gpbmagenta		201
#define wig_col_gpbyellow		202
#define wig_col_gpbblack		203
#define wig_col_gpbcustom		204
#define wig_col_gpbccyan		205
#define wig_col_gpbcmagenta		206
#define wig_col_gpbcyellow		207
#define wig_col_gpbcblack		208
#define wig_col_gpbunused		209
#define LAST_LABEL                      wig_col_gpbunused 
#define NUM_LABELS                      10 /* Should be the number of items from wig_col_gpbcyan */
#define TOTAL_LABELS                    60 /* 6 * NUM_LABELS because we have 6 submenus in Color Gr */
/* End widgets for Color/Palette Menus */


/* Values correspond to the entries found in pub_display.c */
#define wig_p_pub_window     0
#define wig_p_from_text		 1
#define wig_p_to_text		 2
#define wig_p_print_window  16
#define wig_p_galley_window 25
#define wig_p_specs			wig_p_specs_window
#define wig_p_page			31
#define wig_p_cp_layout     32
#define wig_p_group_create  33
#define wig_p_paginate		35
#define wig_p_display_type  36
#define wig_p_icons         37
#define wig_p_name          38
#define wig_p_layout_list   39
#define wig_p_breaking		44
#define wig_p_chapter		45
#define wig_p_illustration	46
#define wig_p_footnote		47
#define wig_p_sidenote		48
#define wig_p_vert_spacing	49
#define wig_p_setup_pages	50
#define wig_p_setup_text	51
#define wig_p_setup_tables	52
#define wig_p_setup_chapter	53
#define wig_p_print_specs  	54
#define wig_p_copy          55
#define wig_p_log_file		56
#define wig_p_export        57
#define wig_p_export_confirm 58
#define wig_p_export_cancel 999
#define wig_p_export_ok		9
#define wig_p_export_button	8
#define wig_p_colors        59
#define wig_p_palcreate     60
#define wig_p_paledit       61
#define wig_p_flotables     62
/* Change PUB_CONTROL_ITEMS to last value + 1 */

/* If this list increases, modify PUB_CONTROL_ITEMS in new_specs.h */
			
/* Define for layout menu bar */

									/* No layouts are opened */
#define BEGIN_NO_LAYOUT		1
#define END_NO_LAYOUT		15

									/* No frames are selected */

#define FileCascade				0
#define EditCascade				1
#define OptionsCascade			2
#define DisplayCascade			3
#define GuidesCascade			4
#define PreferencesCascade		5
#define SaveLayout				6
#define CloseLayout				7
#define IconifyLayout			8
#define PrevPage				9
#define NextPage				10
#define FacingPage				11
#define TurnPage				12
#define ContinuedFrom			13
#define ContinuedTo				14
#define OpenFlow				15
#define DeleteBearoff			16
#define MoveTop					17
#define MoveBottom				18
#define MoveForward				19
#define MoveBackward			20
#define ChangeFrame				21
#define StepRepeat				22
#define ReFlow					23
#define FitUnderset				24
#define FitOverflow				25
#define VerticalJustification	26
#define DeleteFrame				27
#define ClearText				28
#define EditFrame				29
#define ClearCS                                 30            
#define CroptoFit                               31
#define ScaletoFit                              32
/* Should set MAX_MENU_BAR in X11_info.h to last +1 */

#define BEGIN_NO_FRAME		13
#define END_NO_FRAME		32

#define WID_CORRECTION 20
#define HEI_CORRECTION 11

/* Defines for crop-to-fit and scale-to-fit specs widgets */
#define Scale2Fit    0
#define ScaleWidth   1
#define ScaleDepth   2
#define Crop2Fit     3
#define CropWidth    4
#define CropDepth    5
/* Set GR_CHANGE in menus.h to CropDepth+1 !!! */

#define ScaleOK      7
#define ScaleCancel  8
#define CropOK       9
#define CropCancel   10


#endif
