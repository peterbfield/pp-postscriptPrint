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
#include <stdlib.h>
#include <string.h>
#include "window.h"
#include "menus.h"
#include "menu_frame.h"
#include "frame.h"
#include "locks.h"
#include "rel_data.h"
#include "specs.h"
#include "traces.h"
#include "widget_list.h"
#include "list_control.h"

#define FRAME_STRING	"FRAME#"

#define LLAY_WIDTH 66
#define LLAY_DEPTH 67
#define LINE_SIZE  120

void frame_add_1_frame_struct(WYSIWYG *);
void frame_expand(WYSIWYG *);

static int read_layout(WYSIWYG *, Pfd, char *);
BOOLEAN Autosizing = FALSE;
int MFSyntaxPts(WYSIWYG *, CNTRLS *, int, char *, int);
int MFGlobalData(WYSIWYG *, CNTRLS *, int, char *, int);
int MFSyntaxPicas(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyIntChar(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyStrPtr(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyIntShort(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyFloZoom(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyFloShort(WYSIWYG *, CNTRLS *, int, char *, int);
int MFFlag(WYSIWYG *, CNTRLS *, int, char *, int);
int MFMpFlag(WYSIWYG *, CNTRLS *, int, char *, int);
int MFStoreIntChar(WYSIWYG *, CNTRLS *, int, char *, int);
int MFHorizontal(WYSIWYG *, CNTRLS *, int, char *, int);
int MFVertical(WYSIWYG *, CNTRLS *, int, char *, int);
int MFWidth(WYSIWYG *, CNTRLS *, int, char *, int);
int MFDepth(WYSIWYG *, CNTRLS *, int, char *, int);
int MFVjDepth(WYSIWYG *, CNTRLS *, int, char *, int);
int MFSTFIntLong(WYSIWYG *, CNTRLS *, int, char *, int);
int MFMiscID(WYSIWYG *, CNTRLS *, int, char *, int);
int MF20IntShort(WYSIWYG *, CNTRLS *, int, char *, int);
int MFLinkNumber(WYSIWYG *, CNTRLS *, int, char *, int);
int MFPolyDots(WYSIWYG *, CNTRLS *, int, char *, int);
int MFIntShort(WYSIWYG *, CNTRLS *, int, char *, int);
int MFSynPicas(WYSIWYG *, CNTRLS *, int, char *, int);
int MFPrefSynPts(WYSIWYG *, CNTRLS *, int, char *, int);
int MFLayText1(WYSIWYG *, CNTRLS *, int, char *, int);
int MFLayText2(WYSIWYG *, CNTRLS *, int, char *, int);
int MFLayText3(WYSIWYG *, CNTRLS *, int, char *, int);
int MFLayText4(WYSIWYG *, CNTRLS *, int, char *, int);
int MFnonframeHort(WYSIWYG *, CNTRLS *, int, char *, int);
int MFnonframeVert(WYSIWYG *, CNTRLS *, int, char *, int);
int MFMpLong(WYSIWYG *, CNTRLS *, int, char *, int);
int MFObjRef(WYSIWYG *, CNTRLS *, int, char *, int);
int MFFrameUID(WYSIWYG *, CNTRLS *, int, char *, int);
int MFFrameMaxUID(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyFloDeg(WYSIWYG *, CNTRLS *, int, char *, int);
int MFAnyBlDeg(WYSIWYG *, CNTRLS *, int, char *, int);


int zoom_revision; /* 0 if we have to multiply scaling values by 10 */

CNTRLS controls[] = {
/*     type,  length,offset,off_rel,(*func()), *ptr_rw   (see menus.h and menu_frame.h)  */
	{ MHWnothing, 2,   0,  0, MFAnyIntChar,  0},/*  0 Type of frame */
	{ MHWtext,   10,   4,  0, MFSyntaxPts,   0},/*  1 Point size */
	{ MHWtext,   10, 284,  1, MFSyntaxPts,   0},/*  2 Leading */
	{ MHWtext,   10,   4,  1, MFSyntaxPts,   0},/*  3 Weight */
	{ MHWtextPtr, 4, 204,  2, MFAnyStrPtr,   0},/*  4 Graphic name */
	{ MHWtoggle,  1,   4,  2, MFStoreIntChar,0},/*  5 Wrap graphic */
	{ MHWoption,  1,   3,  1, MFStoreIntChar,0},/*  6 Lockpoint */
	{ MHWtext,   14,  27,  2, MFHorizontal,  0},/*  7 Horizontal */
	{ MHWtext,   14,  42,  3, MFVertical,    0},/*  8 Vertical */
	{ MHWtext,   14,  57,  4, MFWidth,       0},/*  9 Width */
	{ MHWtext,   14,  72,  5, MFDepth,       0},/* 10 Depth */
	{ MHWtext,   10,  87,  6, MFPrefSynPts,  0},/* 11 Top gutter */
	{ MHWtext,   10,  98,  7, MFPrefSynPts,  0},/* 12 Bottom gutter */
	{ MHWtext,   10, 109,  8, MFPrefSynPts,  0},/* 13 Left gutter */
	{ MHWtext,   10, 120,  9, MFPrefSynPts,  0},/* 14 Right gutter */
	{ MHWwindow,  0,   0,  0,  0,            0},/* 15 Layout window */
	{ MHWoptionButton, TL,  6, 0, 0,         0},/* 16 Top left */
	{ MHWoptionButton, TC,  6, 0, 0,         0},/* 17 Top center */
	{ MHWoptionButton, TR,  6, 0, 0,         0},/* 18 Top right */
	{ MHWoptionButton, BL,  6, 0, 0,         0},/* 19 Bottom left */
	{ MHWoptionButton, BC,  6, 0, 0,         0},/* 20 Bottom center */
	{ MHWoptionButton, BR,  6, 0, 0,         0},/* 21 Bottom right */
	{ MHWoptionButton, CL,  6, 0, 0,         0},/* 22 Center left */
	{ MHWoptionButton, CC,  6, 0, 0,         0},/* 23 Center center */
	{ MHWoptionButton, CR,  6, 0, 0,         0},/* 24 Center right */
	{ MHWoptionButton, HL,  6, 0, 0,         0},/* 25 Height left */
	{ MHWoptionButton, HC,  6, 0, 0,         0},/* 26 Height center */
	{ MHWoptionButton, HR,  6, 0, 0,         0},/* 27 Height right */
	{ MHWcascade, 0,   0,  0, 0,             0},/* 28 Redraw */
	{ MHWwindow,  0,   0,  0, 0,             0},/* 29 Main Window */
	{ MHWwindow,  0,   0,  0, 0,             0},/* 30 Edit Frame Window*/
	{ MHWtext,   10, 160, 10, MFSyntaxPicas, 0},/* 31 Gr. top gutter */
	{ MHWtext,   10, 171, 11, MFSyntaxPicas, 0},/* 32 Gr. bottom gut. */
	{ MHWtext,   10, 182, 12, MFSyntaxPicas, 0},/* 33 Gr. left gutter */
	{ MHWtext,   10, 193, 13, MFSyntaxPicas, 0},/* 34 Gr. right gut. */
	{ MHWundef,   3, 256,  8, MFMiscID,      0},/* 35 Misc.ID */
	{ MHWundef,   3, 160, 19, MFAnyIntShort, 0},/* 36 Article number */
	{ MHWtoggle,  1,  24,  6, MFFlag,        0},/* 37 Layer_Bg */
	{ MHWtext,    5, 164, 14, MFAnyFloShort, 0},/* 38 Cap Height */
	{ MHWundef,   3, 145,  9, MFAnyIntChar,  0},/* 39 # of part in VJ */
	{ MHWundef,  10, 149, 14, MFVjDepth,     0},/* 40 VJ relationship */
	{ MHWtextPtr, 4, 280,  1, MFAnyStrPtr,   0},/* 41 Text style name */
	{ MHWoption,  1, 131,  6, MFStoreIntChar,0},/* 42 Rotational lock */
	{ MHWtext,    5, 132, 13, MFAnyFloDeg, 0},/* 43 Rotatio. degree */
	{ MHWtext,    5, 138, 16, MFAnyIntShort, 0},/* 44 Foregrnd color */
	{ MHWtext,    5,  16, 17, MFAnyIntShort, 0},/* 45 Backgrnd color */
	{ MHWtoggle,  1,  25, 12, MFGlobalData,  0},/* 46 Overlay */
	{ MHWundef,   3, 180,  0, MFSTFIntLong,  0},/* 47 STF Style adj. */
	{ MHWundef,   3, 184,  1, MFSTFIntLong,  0},/* 48 STF V. sp. adj. */
	{ MHWundefint,1, 247,  1, MFFlag,        0},/* 49 Shape flag */
	{ MHWundef,   5, 189,  9, MFAnyIntShort, 0},/* 50 STF measure */
	{ MHWundef,   5, 195, 10, MFAnyFloShort, 0},/* 51 STF Point size */
	{ MHWundef,   5, 201, 11, MFAnyFloShort, 0},/* 52 STF Set size */
	{ MHWundef,   5, 207, 12, MFAnyFloShort, 0},/* 53 STF Leading */
	{ MHWundef,   5, 213,  7, MFAnyIntChar,  0},/* 54 STF Interword */
	{ MHWundef,   5, 219,  8, MFAnyIntChar,  0},/* 55 STF Space band */
	{ MHWundef,   3, 225,  2, MFSTFIntLong,  0},/* 56 STF V. sp. ovr. */
	{ MHWnothing, 3, 170,  3, MFAnyIntChar,  0},/* 57 Breaking rule */
	{ MHWundef,   1,  15,  2, MFAnyIntChar,  0},/* 58 Rule or box */
	{ MHWtext,    2,  26,  1, MFAnyIntShort, 0},/* 59 Original # cols */
	{ MHWundef,   3,  29,  2, MFAnyIntShort, 0},/* 60 Total # of fram */
	{ MHWtextPtr, 4,  16,  0, MFAnyStrPtr,   0},/* 61 Illustration style name */
	{ MHWtoggle,  1,  37,  0, MFFlag,        0},/* 62 Trim mark flag */
	{ MHWtoggle,  1,  38,  1, MFFlag,        0},/* 63 Crop mark flag */
	{ MHWtext,   10,  39,  6, MF20IntShort,  0},/* 64 Mark weight */
	{ MHWtext,   10,  50,  7, MFAnyIntShort, 0},/* 65 Mark length */
	{ MHWtext,   10,  61,  2, MFSyntaxPicas, 0},/* 66 Trim width */
	{ MHWtext,   10,  72,  3, MFSyntaxPicas, 0},/* 67 Trim depth */
	{ MHWtext,   10,  83,  4, MFSyntaxPicas, 0},/* 68 Page width */
	{ MHWtext,   10,  94,  5, MFSyntaxPicas, 0},/* 69 Page depth */
	{ MHWtext,   10, 105, 17, MFSyntaxPicas, 0},/* 70 X page origin */
	{ MHWtext,   10, 116, 18, MFSyntaxPicas, 0},/* 71 Y page origin */
	{ MHWtext,   10, 127, 19, MFSyntaxPicas, 0},/* 72 Top margin */
	{ MHWtext,   10, 138, 20, MFSyntaxPicas, 0},/* 73 Bottom margin */
	{ MHWtext,   10, 149, 10, MFSyntaxPicas, 0},/* 74 Left margin */
	{ MHWtext,   10, 174, 11, MFSyntaxPicas, 0},/* 75 Right margin */
	{ MHWtext,   10, 185, 12, MFSyntaxPicas, 0},/* 76 Org. col. width */
	{ MHWtext,   10, 196, 13, MFSyntaxPicas, 0},/* 77 Org. col. depth */
	{ MHWtext,   10, 207, 14, MFSyntaxPicas, 0},/* 78 Org. gut. width */
	{ MHWtext,   10, 218, 16, MFSyntaxPicas, 0},/* 79 Org. gut. depth */
	{ MHWtext,    2, 229,  8, MFAnyIntShort, 0},/* 80 Original # rows */
	{ MHWoption,  1, 232,  1, MFStoreIntChar,0},/* 81 Frame Type */
	{ MHWoption,  1, 233,  4, MFStoreIntChar,0},/* 82 Orientation */
	{ MHWoption,  1, 234,  5, MFStoreIntChar,0},/* 83 Positioning */
	{ MHWoptionButton, TL, 42, 0, 0,         0},/* 84 Top left */
	{ MHWoptionButton, TC, 42, 0, 0,         0},/* 85 Top center */
	{ MHWoptionButton, TR, 42, 0, 0,         0},/* 86 Top right */
	{ MHWoptionButton, BL, 42, 0, 0,         0},/* 87 Bottom left */
	{ MHWoptionButton, BC, 42, 0, 0,         0},/* 88 Bottom center */
	{ MHWoptionButton, BR, 42, 0, 0,         0},/* 89 Bottom right */
	{ MHWoptionButton, CL, 42, 0, 0,         0},/* 90 Center left */
	{ MHWoptionButton, CC, 42, 0, 0,         0},/* 91 Center center */
	{ MHWoptionButton, CR, 42, 0, 0,         0},/* 92 Center right */
	{ MHWoptionButton, HL, 42, 0, 0,         0},/* 93 Height left */
	{ MHWoptionButton, HC, 42, 0, 0,         0},/* 94 Height center */
	{ MHWoptionButton, HR, 42, 0, 0,         0},/* 95 Height right */
	{ MHWtoggle,  1,  25,  2,  MFFlag,       0},/* 96 Overlay */
	{ MHWradioButton,  PL_FLOW,   81, 0, 0,  0},/* 97 Flow Frame */
	{ MHWradioButton,  PL_GRAPHIC,81, 0, 0,  0},/* 98 Graphic Frame */
	{ MHWradioButton,  PL_RBX,    81, 0, 0,  0},/* 99 Rule/Box Frame */
	{ MHWradioButton,  PORT,      82, 0, 0,  0},/* 100 Portrait */
	{ MHWradioButton,  LAND,      82, 0, 0,  0},/* 101 Landscape */
	{ MHWradioButton,  ABS,       83, 0, 0,  0},/* 102 Absolute */
	{ MHWradioButton,  REL,       83, 0, 0,  0},/* 103 Relative */
	{ MHWundef,   3, 235, 15, MFMiscID,      0},/* 104 High ID */
	{ MHWoption,  1, 239,  6, MFStoreIntChar,0},/* 105 Trim type */
	{ MHWradioButton,  PL_LETTER, 105,0, 0,  0},/* 106 Letter Trim */
	{ MHWradioButton,  PL_LEGAL,  105,0, 0,  0},/* 107 Legal Trim */
	{ MHWradioButton,  PL_BSIZE,  105,0, 0,  0},/* 108 Bsize Trim */
	{ MHWradioButton,  PL_CUSTOM, 105,0, 0,  0},/* 109 Custom Trim */
	{ MHWundef,  LINK_STRING_LENGTH, 1037, 15, MFLinkNumber,  0},/* 110 Link number */
	{ MHWtext,   10, 266, 15, MFSyntaxPts,   0},/* 111 Set size */
	{ MHWtext,    5, 304,  4,  MFGlobalData, 0},/* 112 Zoom Value */
	{ MHWoption,  1, 310,  3,  MFGlobalData, 0},/* 113 Zoom Option */
	{ MHWradioButton, AUTO_FIT,   113,0, 0,  0},/* 114 Auto Fit */
	{ MHWradioButton, ZOOM_100,   113,0, 0,  0},/* 115 100% zoom */
	{ MHWradioButton, ZOOM_75,    113,0, 0,  0},/* 116  75% zoom */
	{ MHWradioButton, ZOOM_50,    113,0, 0,  0},/* 117  50% zoom */
	{ MHWradioButton, ZOOM_25,    113,0, 0,  0},/* 118  25% zoom */
	{ MHWradioButton, ZOOM_CUSTOM,113,0, 0,  0},/* 119 custom zoom */
	{ MHWoption,  1, 311,  1,  MFGlobalData, 0},/* 120 Text Display */
	{ MHWradioButton, FRAME_OPT,  120,0, 0,  0},/* 121 frame */
	{ MHWradioButton, LABEL_OPT,  120,0, 0,  0},/* 122 label */
	{ MHWradioButton, WYS_OPT,    120,0, 0,  0},/* 123 wysiwyg */
	{ MHWradioButton, F_WYS_OPT,  120,0, 0,  0},/* 124 framed wysiwyg */
	{ MHWoption,  1, 312,  2,  MFGlobalData, 0},/* 125 Graphic Disp. */
	{ MHWradioButton, FRAME_OPT,  125,0, 0,  0},/* 126 frame */
	{ MHWradioButton, LABEL_OPT,  125,0, 0,  0},/* 127 label */
	{ MHWradioButton, WYS_OPT,    125,0, 0,  0},/* 128 wysiwyg */
	{ MHWradioButton, F_WYS_OPT,  125,0, 0,  0},/* 129 framed wysiwyg */
	{ MHWoption,  1, 363,  8,  MFGlobalData, 0},/* 130 Horz Ruler Org */
	{ MHWradioButton,      TRIM,  130,0, 0,  0},/* 131 horz trim */
	{ MHWradioButton,      PAGE,  130,0, 0,  0},/* 132 horz page */
	{ MHWradioButton,    MARGIN,  130,0, 0,  0},/* 133 horz margin */
	{ MHWradioButton,    FRAMES,  130,0, 0,  0},/* 134 horz frame */
	{ MHWtext,   10, 364, -1, MFnonframeHort,0},/* 135 Horz frame text */
	{ MHWoption,  1, 375,  9,  MFGlobalData, 0},/* 136 Vert Ruler Org */
	{ MHWradioButton,      TRIM,  136,0, 0,  0},/* 137 vert trim */
	{ MHWradioButton,      PAGE,  136,0, 0,  0},/* 138 vert page */
	{ MHWradioButton,    MARGIN,  136,0, 0,  0},/* 139 vert margin */
	{ MHWradioButton,    FRAMES,  136,0, 0,  0},/* 140 vert frame */
	{ MHWtext,   10, 376, -1, MFnonframeVert,0},/* 141 Vert frame text */
	{ MHWoption,  1, 387, 10,  MFGlobalData, 0},/* 142 Horz Ruler Units */
	{ MHWradioButton,      PICA,  142,0, 0,  0},/* 143 horz pica */
	{ MHWradioButton,   INCH_10,  142,0, 0,  0},/* 144 horz inches (dec) */
	{ MHWradioButton,   INCH_32,  142,0, 0,  0},/* 145 horz inches (frac) */
	{ MHWradioButton,CENTIMETER,  142,0, 0,  0},/* 146 horz cm */
	{ MHWoption,  1, 388, 11,  MFGlobalData, 0},/* 147 Vert Ruler Units */
	{ MHWradioButton,      PICA,  147,0, 0,  0},/* 148 vert pica */
	{ MHWradioButton,   INCH_10,  147,0, 0,  0},/* 149 vert inches (dec) */
	{ MHWradioButton,   INCH_32,  147,0, 0,  0},/* 150 vert inches (frac) */
	{ MHWradioButton,CENTIMETER,  147,0, 0,  0},/* 151 vert cm */
	{ MHWradioButton, LINE_LEAD,  147,0, 0,  0},/* 152 vert lines */
	{ MHWtext,   10, 389, -1,  MFPrefSynPts, 0},/* 153 Vert ruler lines */
	{ MHWwindow,  0,   0,  0,            0,  0},/* 154 Rulers Window */
	{ MHWtext,    3, 400,  2,    MFIntShort, 0},/* 155 Column # Cols */
	{ MHWtext,   10, 404,  0,    MFSynPicas, 0},/* 156 Column width */
	{ MHWtext,   10, 415,  1,    MFSynPicas, 0},/* 157 Column gutt width */
	{ MHWtext,    3, 426,  1,    MFIntShort, 0},/* 158 Column # Rows */
	{ MHWtext,   10, 430,  2,    MFSynPicas, 0},/* 159 Column depth */
	{ MHWtext,   10, 441,  3,    MFSynPicas, 0},/* 160 Column gutt depth */
	{ MHWoption,  1, 452,  0,    MFIntShort, 0},/* 161 Column Guide Origin */
	{ MHWradioButton,   CG_TRIM,  161,0, 0,  0},/* 162 align trim */
	{ MHWradioButton,   CG_PAGE,  161,0, 0,  0},/* 163 align page */
	{ MHWradioButton, CG_MARGIN,  161,0, 0,  0},/* 164 align margin */
	{ MHWwindow,  0,   0,  0,            0,  0},/* 165 ColGuide Window */
	{ MHWoption,  1, 453,  5,  MFGlobalData, 0},/* 166 Snap FROM opt. */
	{ MHWradioButton, FREE_FORM,  166,0, 0,  0},/* 167 free form snaps */
	{ MHWradioButton,FRAME_EDGES, 166,0, 0,  0},/* 168 frame edge snaps */
	{ MHWundef,   0,   0,      0,        0,  0},/* 169 UNUSED */
	{ MHWundef,   0,   0,      0,        0,  0},/* 170 UNUSED */
	{ MHWoption,  2, 296,  7,  MFGlobalData, 0},/* 171 Snap TO Vert Opt. */
	{ MHWtoggleOpt,        TRIM,  171,0, 0,  0},/* 172 Trim V toggle */
	{ MHWtoggleOpt,        PAGE,  171,0, 0,  0},/* 173 Page V toggle */
	{ MHWtoggleOpt,      MARGIN,  171,0, 0,  0},/* 174 Margin V toggle */
	{ MHWtoggleOpt, FRAME_EDGES,  171,0, 0,  0},/* 175 Frame Edg V toggle */
	{ MHWtoggleOpt,CROSSHAIR1_H_V,171,0, 0,  0},/* 176 1st Crosshair V */
	{ MHWtoggleOpt,CROSSHAIR2_H_V,171,0, 0,  0},/* 177 2nd Crosshair V */
	{ MHWtoggleOpt,COLUMN_GUIDES, 171,0, 0,  0},/* 178 ColGuides V toggle */
	{ MHWtoggleOpt,LEADING_GUIDES,171,0, 0,  0},/* 179 Leading V toggle */
	{ MHWoption,  1, 455,  6,  MFGlobalData, 0},/* 180 Snap TO Horz opt. */
	{ MHWtoggleOpt,        TRIM,  180,0, 0,  0},/* 181 Trim H toggle */
	{ MHWtoggleOpt,        PAGE,  180,0, 0,  0},/* 182 Page H toggle */
	{ MHWtoggleOpt,      MARGIN,  180,0, 0,  0},/* 183 Margin H toggle */
	{ MHWtoggleOpt, FRAME_EDGES,  180,0, 0,  0},/* 184 Frame Edg H toggle */
	{ MHWtoggleOpt,CROSSHAIR1_H_V,180,0, 0,  0},/* 185 1st Crosshair H */
	{ MHWtoggleOpt,CROSSHAIR2_H_V,180,0, 0,  0},/* 186 2nd Crosshair H */
	{ MHWtoggleOpt,COLUMN_GUIDES, 180,0, 0,  0},/* 187 ColGuides H toggle */
	{ MHWwindow,  0,   0,  0,            0,  0},/* 188 Snap Window */
	{ MHWoption,  2, 456,  0,  MFGlobalData, 0},/* 189 Display option */
	{ MHWtoggleOpt, COLUMN_GUIDES,189,0, 0,  0},/* 190 Display ColGuides */
	{ MHWtoggleOpt,CROSSHAIR1_H_V,189,0, 0,  0},/* 191 Display 1st CH */
	{ MHWtoggleOpt,CROSSHAIR2_H_V,189,0, 0,  0},/* 192 Display 2nd CH */
	{ MHWtoggleOpt,        RULERS,189,0, 0,  0},/* 193 Display Rulers */
	{ MHWtoggleOpt,          TRIM,189,0, 0,  0},/* 194 Display Trim */
	{ MHWtoggleOpt,          PAGE,189,0, 0,  0},/* 195 Display Page */
	{ MHWtoggleOpt,        MARGIN,189,0, 0,  0},/* 196 Display Margin */
	{ MHWtoggleOpt,      BOX_FILL,189,0, 0,  0},/* 197 Rule/Box Fill */
	{ MHWtoggleOpt,SAVE_TRACK_MOUSE,189,0,0, 0},/* 198 Track Mouse */
	{ MHWtoggleOpt,   AUTO_SZ_SEL,189,0, 0,  0},/* 199 Auto Menu Size */
	{ MHWtext,   10, 458,  0,  MFPrefSynPts, 0},/* 200 Rule weight */
	{ MHWtext,   10, 469,  1,  MFPrefSynPts, 0},/* 201 Box weight */
	{ MHWwindow,  0,   0,  0,           0,   0},/* 202 Custom Zoom Menu */
	{ MHWwindow,  0,   0,  0,           0,   0},/* 203 Rule/Box Pref Menu */
	{ MHWwindow,  0,   0,  0,           0,   0},/* 204 Gutters Pref Menu */
	{ MHWtext,   14, 480,  2,  MFLayText1,   0},/* 205 Lay Frame Hort pos */
	{ MHWtext,   14, 495,  3,  MFLayText2,   0},/* 206 Lay Frame Vert pos */
	{ MHWtext,   14, 510,  4,  MFLayText3,   0},/* 207 Lay Frame Width pos */
	{ MHWtext,   14, 525,  5,  MFLayText4,   0},/* 208 Lay Frame Depth pos */
	{ MHWoption,  1, 277,  1, MFStoreIntChar,0},/* 209 Lay Frame Lockpoint */
	{ MHWoptionButton, TL,209, 0, 0,         0},/* 210 Top left */
	{ MHWoptionButton, TC,209, 0, 0,         0},/* 211 Top center */
	{ MHWoptionButton, TR,209, 0, 0,         0},/* 212 Top right */
	{ MHWoptionButton, BL,209, 0, 0,         0},/* 213 Bottom left */
	{ MHWoptionButton, BC,209, 0, 0,         0},/* 214 Bottom center */
	{ MHWoptionButton, BR,209, 0, 0,         0},/* 215 Bottom right */
	{ MHWoptionButton, CL,209, 0, 0,         0},/* 216 Center left */
	{ MHWoptionButton, CC,209, 0, 0,         0},/* 217 Center center */
	{ MHWoptionButton, CR,209, 0, 0,         0},/* 218 Center right */
	{ MHWoptionButton, HL,209, 0, 0,         0},/* 219 Height left */
	{ MHWoptionButton, HC,209, 0, 0,         0},/* 220 Height center */
	{ MHWoptionButton, HR,209, 0, 0,         0},/* 221 Height right */
	{ MHWtoggleOpt,         TINTS,189,0, 0,  0},/* 222 Display Frame Tints */
	{ MHWtoggleOpt,         LINKS,189,0, 0,  0},/* 223 Display Frame Links */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 224 Lay Frame Hort Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 225 Lay Frame Hort Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 226 Lay Frame Hort Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 227 Lay Frame Hort Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 228 Cust Trim Width Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 229 Cust Trim Depth Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 230 Reg Mark Width Label */
	{ MHWlabel,  0,  490,  0,           0,   0},/* 231 Reg Mark Depth Label */
	{ MHWpushButton,0,490, 0,           0,   0},/* 232 Default Pushbutton */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 233 STF Menu */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 234 VJT Menu */
	{ MHWnothing, 1, 000, 00, MFStoreIntChar,0},/* 235 Vert Just Opt */
					/* If the above line is one day used, you need
						to set '000' and '00' to an appropriate value
						and change MHWnothing to MHWoption. */
	{ MHWradioButton,    LINES, 235, 0, 0,   0},/* 236 VJ Lines */
	{ MHWradioButton,     PARA, 235, 0, 0,   0},/* 237 VJ Paragaph */
	{ MHWradioButton,       EL, 235, 0, 0,   0},/* 238 VJ Extra Lead */
	{ MHWradioButton,   EX_PTS, 235, 0, 0,   0},/* 239 VJ Expand Pts */
	{ MHWradioButton,      TOP, 235, 0, 0,   0},/* 240 VJ Top */
	{ MHWradioButton,   BOTTOM, 235, 0, 0,   0},/* 241 VJ Bottom */
	{ MHWradioButton,   CUSTOM, 235, 0, 0,   0},/* 242 VJ Custom */
	{ MHWpushButton, 0, 491, 0,         0,   0},/* 243 VJ Reset */
	{ MHWnothing, 1, 000, 00, MFStoreIntChar,0},/* 244 Set To Fit Opt */
					/* If the above line is one day used, you need
						to set '000' and '00' to an appropriate value
						and change MHWnothing to MHWoption. */
	{ MHWradioButton,  MEASURE, 244, 0, 0,   0},/* 245 STF Measure */
	{ MHWradioButton,    PT_SZ, 244, 0, 0,   0},/* 246 STF Point Size */
	{ MHWradioButton,   SET_SZ, 244, 0, 0,   0},/* 247 STF Set Size */
	{ MHWradioButton, STF_LEAD, 244, 0, 0,   0},/* 248 STF Leading */
	{ MHWradioButton,    BANDS, 244, 0, 0,   0},/* 249 STF Bands */
	{ MHWradioButton,    WIDTH, 244, 0, 0,   0},/* 250 STF Width */
	{ MHWradioButton,  MINIMUM, 244, 0, 0,   0},/* 251 STF Minimum */
	{ MHWradioButton,  MAXIMUM, 244, 0, 0,   0},/* 252 STF Maximum */
	{ MHWradioButton,   CUSTOM, 244, 0, 0,   0},/* 253 STF Custom */
	{ MHWpushButton, 0, 492, 0,         0,   0},/* 254 STF Reset */
	{ MHWpolyDots, 0, SHAPE, 0, MFPolyDots,  0},/* 255 List of dots */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 256 STF Custom Menu */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 257 VJT Custom Menu */
	{ MHWlabel,  6,  542,  0,           0,   0},/* 258 VJT Stat Lines # */
	{ MHWlabel,  7,  549,  0,           0,   0},/* 259 VJT Stat Lines Add */
	{ MHWlabel,  6,  557,  0,           0,   0},/* 260 VJT Stat Para # */
	{ MHWlabel,  7,  564,  0,           0,   0},/* 261 VJT Stat Para Add */
	{ MHWlabel,  6,  572,  0,           0,   0},/* 262 VJT Stat EL # */
	{ MHWlabel,  7,  579,  0,           0,   0},/* 263 VJT Stat EL Add */
	{ MHWlabel,  6,  587,  0,           0,   0},/* 264 VJT Stat Ex Pts # */
	{ MHWlabel,  7,  594,  0,           0,   0},/* 265 VJT Stat Ex Pts Add */
	{ MHWlabel,  6,  602,  0,           0,   0},/* 266 VJT Stat Top # */
	{ MHWlabel,  7,  609,  0,           0,   0},/* 267 VJT Stat Top Add */
	{ MHWlabel,  6,  617,  0,           0,   0},/* 268 VJT Stat Bottom # */
	{ MHWlabel,  7,  624,  0,           0,   0},/* 269 VJT Stat Bottom Add */
	{ MHWoption, 1,  632,  0,           0,   0},/* 270 VJT Cust Select Opt */
	{ MHWradioButton,  LINES, 270, 0,   0,   0},/* 271 VJT Cust Lines */
	{ MHWradioButton,   PARA, 270, 0,   0,   0},/* 272 VJT Cust Para */
	{ MHWradioButton,     EL, 270, 0,   0,   0},/* 273 VJT Cust EL */
	{ MHWradioButton, EX_PTS, 270, 0,   0,   0},/* 274 VJT Cust Exp Pts */
	{ MHWradioButton,    TOP, 270, 0,   0,   0},/* 275 VJT Cust Top */
	{ MHWradioButton, BOTTOM, 270, 0,   0,   0},/* 276 VJT Cust Bottom */
	{ MHWradioButton,  TABLE, 270, 0,   0,   0},/* 277 VJT Cust Table */
	{ MHWtext,   6,  633,  0,           0,   0},/* 278 VJT Lines Fixed */
	{ MHWtext,   6,  640,  0,           0,   0},/* 279 VJT Lines Max */
	{ MHWtext,   6,  647,  0,           0,   0},/* 280 VJT Para Fixed */
	{ MHWtext,   6,  654,  0,           0,   0},/* 281 VJT Para Max */
	{ MHWtext,   6,  661,  0,           0,   0},/* 282 VJT EL % Fixed */
	{ MHWtext,   6,  668,  0,           0,   0},/* 283 VJT EL % Max */
	{ MHWtext,   6,  675,  0,           0,   0},/* 284 VJT Exp Pts Fixed */
	{ MHWtext,   6,  682,  0,           0,   0},/* 285 VJT Exp Pts Max */
	{ MHWtext,   6,  689,  0,           0,   0},/* 286 VJT Top Fixed */
	{ MHWtext,   6,  696,  0,           0,   0},/* 287 VJT Top Max */
	{ MHWtext,   6,  703,  0,           0,   0},/* 288 VJT Bottom Fixed */
	{ MHWtext,   6,  710,  0,           0,   0},/* 289 VJT Bottom Max */
	{ MHWtext,  24,  717,  0,           0,   0},/* 290 VJT Table selection */
	{ MHWtextPtr,4,  300,  0, MFAnyStrPtr,   0},/* 291 A Text String PTR */
	{ MHWpolyDots, 0, BEAROFF, 0, MFPolyDots,0},/* 292 List of bearoff dots */
	{ MHWtoggle, 1,   23,  7,      MFFlag,   0},/* 293 FG Attribute */
	{ MHWtext,   0,    0,  0,           0,   0},/* 294 Open Page Text */
	{ MHWselection,0,  0,  0,           0,   0},/* 295 Open Page List */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 296 Open Page Menu */
	{ MHWtoggleOpt,  FACING_PAGES,189,0, 0,  0},/* 297 Display Facing pages */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 298 Step/repeat menu */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 299 Step/repeat text 1 */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 300 Step/repeat text 2 */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 301 Step/repeat text 3 */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 302 Step/repeat text 4 */
	{ MHWradioButton,  PL_TEXT,  81, 0, 0,   0},/* 303 Design Frame */
	{ MHWtoggle, 1,   27,  6,           0,   0},/* 304 Apply lock */
	{ MHWtoggle, 1,   28, 11,           0,   0},/* 305 Apply gutters */
	{ MHWtoggle, 1,   29,  3,           0,   0},/* 306 Apply rule weight */
	{ MHWtoggle, 1,   30, 46,           0,   0},/* 307 Apply overlay */
	{ MHWtoggle, 1,   31, 37,           0,   0},/* 308 Apply layer */
	{ MHWtoggle, 1,   32,293,           0,   0},/* 309 Apply transparent */
	{ MHWtoggle, 1,   33, 44,           0,   0},/* 310 Apply fg color */
	{ MHWtoggle, 1,   34, 45,           0,   0},/* 311 Apply bg color */
	{ MHWtoggle, 1,   35, 42,           0,   0},/* 312 Apply rot. lock */
	{ MHWtoggle, 1,   36, 43,           0,   0},/* 313 Apply rot. degree */
	{ MHWtoggle, 1,   37,315,           0,   0},/* 314 Apply outline */
	{ MHWtoggle,  1,  26,  0, MFFlag,        0},/* 315 Outline */
	{ MHWtext,    5, 240, 18, MFAnyIntShort, 0},/* 316 Outline color */
	{ MHWtext,   10, 229, 18, MFSyntaxPts,   0},/* 317 Outline weight */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 318 Position menu */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 319 Rotation menu */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 320 Old angle */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 321 Variation */
	{ MHWtext,   5,  298, 13, MFAnyFloDeg, 0},/* 322 New angle */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 323 Reflow */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 324 Reflow clear vj */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 325 Reflow clear locks */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 326 Reflow clear both */
	{ MHWtext,   5,  216,  9, MFAnyIntShort, 0},/* 327 Relative frame # */
	{ MHWoption,  1, 214,  8, MFStoreIntChar,0},/* 328 MP design-text type: text/CL/SN/AR */
	{ MHWradioButton, DESIGN_TEXT, 328, 0, 0,0},/* 329 --Reg. design text */
	{ MHWradioButton, CONT_LINE, 328, 0, 0,  0},/* 330 --Continued line */
	{ MHWradioButton, SIDE_NOTE, 328, 0, 0,  0},/* 331 --Side note */
	{ MHWradioButton, AREA_REF, 328,  0, 0,  0},/* 332 --Area reference */
	{ MHWoption,        1, 213, 0, MFMpFlag, 0},/* 333 MP flags: */
	{ MHWradioButton, NO_QUAD, 333,   0, 0,  0},/* 334 --Ignore quad */
	{ MHWradioButton, FQUAD_LEFT, 333, 0, 0, 0},/* 335 --Quad left */
	{ MHWradioButton, FQUAD_RIGHT, 333, 0,0, 0},/* 336 --Quad center */
	{ MHWradioButton, FQUAD_CENTER, 333, 0,0,0},/* 337 --Quad right */
	{ MHWtoggle,        1, 215, 1, MFMpFlag, 0},/* 338 Typeset flag */
	{ MHWtextPtr,4, 248, 2, MFAnyStrPtr,     0},/* 339 Page style name */
	{ MHWtextPtr,4, 252, 3, MFAnyStrPtr,     0},/* 340 Try table style name */
	{ MHWtoggleOpt,W_BASELINE, 171,   0, 0,  0},/* 341 W baseline V toggle */
	{ MHWtoggleOpt,W_CAP_HEIGHT, 171, 0, 0,  0},/* 342 W cap height V toggle */
	{ MHWtoggle,  1, 743,  8, MFFlag,        0},/* 343 Layer_Fg */
	{ MHWtext,   10, 745, 19, MFSyntaxPts,   0},/* 344 Outline trap TMS */
	{ MHWtext,   10, 756, 21, MFSyntaxPts,   0},/* 345 Top left rounded */
	{ MHWtext,   10, 767, 22, MFSyntaxPts,   0},/* 346 Top right rounded */
	{ MHWtext,   10, 778, 23, MFSyntaxPts,   0},/* 347 Bottom left rounded */
	{ MHWtext,   10, 789, 24, MFSyntaxPts,   0},/* 348 Bottom right rounded */
	{ MHWwindow, 0,    0,  0,           0,   0},/* 349 Rounded menu */
	{ MHWtext,   10, 800, 21, MFSyntaxPts,   0},/* 350 Rounded text */
    { MHWtext,   10, 811, 14, MFDepth, 		 0},/* 351 Crop_Top */
    { MHWtext,   10, 822, 16, MFWidth, 		 0},/* 352 Crop_Left */
    { MHWtext,   10, 833, 15, MFDepth, 		 0},/* 353 Crop_Bottom */
    { MHWtext,   10, 844, 20, MFWidth, 		 0},/* 354 Crop_Right */
    { MHWtext,    5, 855, 11, MFAnyFloZoom,  0},/* 355 Zoom_Graphic X direction*/
    { MHWtext,    1, 861, 14, MFAnyIntShort, 0},/* 356 Crop_Flag */
	{ MHWnothing,23, 863, 25, MFObjRef, 	 0},/* 357 Object_Ref */
	{ MHWundef,  11, 887,  0, MFMpLong,		 0},/* 358 MP frame type */
					/* Above line not used for any display purpose in a frame,
						therefore the offset is 000.  If it's used, must assign
						proper offset and length.						*/
	{ MHWnothing,11, 899, 0, MFFrameUID, 0},/* 359 Frame Unique ID */
	{ MHWnothing,11, 911, 0, MFFrameMaxUID, 0},/* 360 Frame Max Unique ID */
	{ MHWtext,    5, 923, 21, MFAnyIntShort, 0},/* 361 Foreground Shade*/
	{ MHWtext,    5, 929, 22, MFAnyIntShort, 0},/* 362 Background Shade*/
	{ MHWtext,    5, 935, 23, MFAnyIntShort, 0},/* 363 Out Shade       */
    { MHWtext,    5, 941, 24, MFAnyFloZoom,  0},/* 364 Zoom_Graphic Y direction*/
	{ MHWwindow, 0,    947,  0,           0,   0},/* 365 Reflow lock frames*/
	{ MHWwindow, 0,    947,  0,           0,   0},/* 366 Reflow entire flow */
	{ MHWwindow, 0,    947,  0,           0,   0},/* 367 Reflow from here to end */
	{ MHWwindow, 0,    947,  0,           0,   0},/* 368 Reflow from here to start */
	{ MHWwindow, 0,    947,  0,           0,   0},/* 369 Reflow from here to */
	{ MHWwindow, 0,    947,  0,           0,   0},/* 370 Reflow text widget */
	{ MHWtext,   5,    947, 25, MFAnyIntShort, 0},/* 371 Blend Color Start */
	{ MHWtext,   5,    953, 26, MFAnyIntShort, 0},/* 372 Blend Color End */
	{ MHWtext,   5,    959, 27, MFAnyBlDeg, 0},/* 373 Blend Angle */
	{ MHWtoggle, 1,    965, 14, MFStoreIntChar,  0},/* 374 Dashed toggle  */
	{ MHWtext,   10,   967, 26, MFSyntaxPicas, 0},/* 375 Dash Width */
	{ MHWtext,   10,   978, 27, MFSyntaxPicas, 0},/* 376 Gap Width  */
	{ MHWtoggle, 1,    989, 15, MFStoreIntChar,  0},/* 377 Outline Dash toggle */
	{ MHWtext,   10,   991, 28, MFSyntaxPicas, 0},/* 378 Outline Dash Width */
	{ MHWtext,   10,  1002, 29, MFSyntaxPicas, 0},/* 379 Outline Gap Width */
	{ MHWtext,    5,  1013, 30, MFAnyIntShort, 0},/* 380 Start Blend Color Shade*/
	{ MHWtext,    5,  1019, 31, MFAnyIntShort, 0},/* 381 End Blend Color Shade*/
	{ MHWundef,  11,  1025, 10, MFMpLong,	   0},/* 382 MP Ill/table placement keys*/
	{ MHWnothing,0,   1068,  0,           0,   0},/* 383 Nothing */
};
/*     type,  length,offset,off_rel,(*func()), *ptr_rw   (see menus.h and menu_frame.h)  */
/* You should define CONTROL_SIZE in menus.h as offset+length+1 of the last(!) controls item */
/* 
 * If you add a line, you MUST adjust #define BOUND_CONTROLS in menus.h
 * Also, please update controls_struct.doc, as this is the current list of
 * data held in the above structure.
 */

/*  0         5        10        15      19 */
char spec_frame_field[]   = {
	1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,	/*  20 -  39 */
	0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,	/*  40 -  59 */
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	/*  60 -  79 */
	1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,	/* 100 - 119 */
	1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,	/* 120 - 139 */
	0,1,1,0,0,0,0,1,0,0,0,0,0,1,0,1,1,1,1,1,	/* 140 - 159 */
	1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,        /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 300 - 319 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,	/* 320 - 339 */
	1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,	/* 340 - 359 */
	1,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,	/* 360 - 379 */
	1,1,0,0						/* 380 - 382 */
	};
char empty_frame_field[]   = {
	1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,	/*  20 -  39 */
	0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,	/*  40 -  59 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  60 -  79 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 100 - 119 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 120 - 139 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 140 - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,    /* 300 - 319 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 320 - 339 */
	0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,	/* 340 - 359 */
	0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,	/* 360 - 379 */
	1,1,0,0						/* 380 - 382 */
	};
char free_form_frame_field[]   = {
	1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,	/*  20 -  39 */
	1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,	/*  40 -  59 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  60 -  79 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,	/* 100 - 119 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 120 - 139 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 140 - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,    /* 300 - 319 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,	/* 320 - 339 */
	1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,	/* 340 - 359 */
	0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,	/* 360 - 379 */
	1,1,1,0						/* 380 - 382 */
	};
char text_frame_field[]    = {
	1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,	/*  20 -  30 */
	1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,	/*  40 -  59 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  60 -  79 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,	/* 100 - 119 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 120 - 139 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 140 - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,    /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,    /* 300 - 319 */
	0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,1,	/* 320 - 339 */
	1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,	/* 340 - 359 */
	0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,	/* 360 - 379 */
	1,1,0,0						/* 380 - 382 */
	};
char misc_frame_field[]    = {
	1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,	/*  20 -  39 */
	1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,	/*  40 -  59 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  60 -  79 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,	/* 100 - 119 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 120 - 139 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 140 - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,    /* 300 - 319 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,	/* 320 - 339 */
	1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,0,1,	/* 340 - 359 */
	0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,	/* 360 - 379 */
	1,1,0,0						/* 380 - 382 */
	};
char graphic_frame_field[] = {
	1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,0,1,	/*  20 -  39 */
	1,0,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,	/*  40 -  59 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  60 -  79 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 100 - 119 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 120 - 139 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 140 - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,    /* 300 - 319 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 320 - 339 */
	0,0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,	/* 340 - 359 */
	0,1,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,	/* 360 - 379 */
	1,1,0,0						/* 380 - 382 */
	};
char rb_frame_field[]		= {
	1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,	/*   0 -  19 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,	/*  20 -  39 */
	1,0,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,1,0,	/*  40 -  59 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  60 -  79 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/*  80 -  99 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 100 - 119 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 120 - 139 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 140 - 159 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 160 - 179 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 180 - 199 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 200 - 219 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 220 - 239 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,	/* 240 - 259 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 260 - 279 */
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 280 - 299 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,    /* 300 - 319 */
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 320 - 339 */
	0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,	/* 340 - 359 */
	0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,	/* 360 - 379 */
	1,1,0,0						/* 380 - 382 */
};

static int *rw_index, size_index;

typedef struct {
	int index;
	char r_word[13];
	} RESERVED_WORDS;

static RESERVED_WORDS reserved_words[] =
	{
/* Length 2:
   ========= */
	  {2, "LD"},
	  {1, "PS"},
	{111, "SS"},

/* Length 3:
   ========= */
	 {39, "#VJ"},
	 {83, "POS"},

/* Length 4:
   ========= */
	  {7, "HORZ"},
	  {8, "VERT"},
	{291, "TEXT"},
	  {0, "TYPE"},
	{112, "ZOOM"},

/* Length 5:
   ========= */
	 {73, "B_MAR"},
	 {10, "DEPTH"},
	 {37, "LAYER"},
	 {74, "L_MAR"},
	 {82, "ORIEN"},
	 {75, "R_MAR"},
	 {49, "SHAPE"},
	 {41, "STYLE"},
	 {72, "T_MAR"},
	  {9, "WIDTH"},

/* Length 6:
   ========= */
	{201, "BOX_WE"},
	 {-1, FRAME_STRING},
	{125, "G_DISP"},
	 {58, "R_OR_B"},
	 {60, "TOT#FR"},
	{120, "T_DISP"},

/* Length 7:
   ========= */
	 {81, "FR_TYPE"},
	  {6, "LOCK_PT"},
	{360, "MAX_UID"},
	 {35, "MISC_ID"},
	{358, "MP_FTYP"},
	{328, "MP_TYPE"},
	{357, "OBJ_REF"},
	 {59, "ORG#COL"},
	 {80, "ORG#ROW"},
	{315, "OUTLINE"},
	 {46, "OVERLAY"},
	 {69, "PAGE_DE"},
	 {68, "PAGE_WI"},
	{200, "RULE_WE"},
	 {67, "TRIM_DE"},
	 {66, "TRIM_WI"},
	  {5, "WRAP_GR"},
	{355, "ZOOM_GR"},
	  {374, "DASH_ON"},
	  {375, "DASHWID"},
	  {376, "GAP_WID"},
	  {377, "ODASHON"},
	  {378, "ODASHWD"},
	  {379, "OGAPWID"},

/* Length 8:
   ========= */
	 {36, "ARTICLE#"},
	 {45, "BG_COLOR"},
	{351, "CROP_TOP"},
	{343, "FG_LAYER"},
	 {12, "B_GUTTER"},
	 {44, "FG_COLOR"},
	 {13, "L_GUTTER"},
	 {64, "MARKS_WE"},
	{333, "MP_FLAGS"},
	{382, "MP_MISC1"},
	{344, "OUT_TRAP"},
	 {14, "R_GUTTER"},
	{349, "ROUNDED1"},
	{350, "ROUNDED2"},
	 {11, "T_GUTTER"},
	 {70, "X_PG_ORG"},
	 {71, "Y_PG_ORG"},
	{113, "ZOOM_OPT"},
	{361, "FG_SHADE"},
	{362, "BG_SHADE"},
	{364, "ZOOM_GRY"},

/* Length 9:
   ========= */
	{155, "COLGD_COL"},
	{159, "COLGD_DEP"},
	{161, "COLGD_ORG"},
	{158, "COLGD_ROW"},
	{156, "COLGD_WID"},
	{356, "CROP_FLAG"},
	{352, "CROP_LEFT"},
	{189, "DISP_OPTS"},
	{293, "FG_ATTRIB"},
	{359, "FRAME_UID"},
	{135, "H_RUL_FRM"},
	{130, "H_RUL_ORG"},
	{142, "H_RUL_UNT"},
	 {65, "MARKS_LGH"},
	{316, "OUT_COLOR"},
	{255, "POLY_DOTS"},
	  {3, "RB_WEIGHT"},
	{327, "REL_FRAME"},
	{166, "SNAP_FROM"},
	{180, "SNAP_TO_H"},
	{171, "SNAP_TO_V"},
	{105, "TRIM_TYPE"},
	{340, "TRY_TABLE"},
	{104, "USED_FFID"},
	{141, "V_RUL_FRM"},
	{153, "V_RUL_LIN"},
	{136, "V_RUL_ORG"},
	{147, "V_RUL_UNT"},
	{363, "OUT_SHADE"},
	{372, "BLEND_END"},

/* Length 10:
   ========= */
	{347, "BL_ROUNDED"},
	{348, "BR_ROUNDED"},
	 {38, "CAP_HEIGHT"},
	 {63, "CROP_MARKS"},
	{354, "CROP_RIGHT"},
	{338, "MP_TYPESET"},
	 {77, "ORG_COL_DE"},
	 {76, "ORG_COL_WI"},
	 {78, "ORG_GUT_WI"},
	 {79, "ORG_GUT_DE"},
	{317, "OUT_WEIGHT"},
	{339, "PAGE_STYLE"},
	 {43, "ROT_DEGREE"},
	{345, "TL_ROUNDED"},
	{346, "TR_ROUNDED"},
	 {62, "TRIM_MARKS"},

/* Length 11:
   ========= */
	 {57, "BREAK_RULES"},
	 {32, "B_GR_GUTTER"},
	{160, "COLGD_GUT_D"},
	{157, "COLGD_GUT_W"},
	{353, "CROP_BOTTOM"},
	 {33, "L_GR_GUTTER"},
	{110, "LINK_NUMBER"},
	 {42, "ROT_LOCK_PT"},
	 {34, "R_GR_GUTTER"},
	 {31, "T_GR_GUTTER"},
	 {40, "VJ_RELATION"},
	{371, "BLEND_START"},
	{373, "BLEND_ANGLE"},

/* Length 12:
   ========= */
	{292, "BEAROFF_DOTS"},
	  {4, "GRAPHIC_NAME"},
	 {61, "ILLUSTRATION"},
	 {380,"BLEND_SSHADE"},
	 {381,"BLEND_ESHADE"},
	};

#if TRACE_FRAME
static void trace_frame(WYSIWYG *wn)
{
	int i, j;
	
	for (i = 0; i < wn -> num_frames; i++)
	{
		p_info(PI_TRACE, "Frame number: %d\n", i);
		for (j = 0; j < 33; j++)
		{
			p_info(PI_TRACE, "c%d: %X ", j, (int)*(&(REL_DATA(i) c0) + j));
			if (!(j | 10))
				p_info(PI_TRACE, "\n");
		}
		p_info(PI_TRACE, "\n");
		for (j = 0; j < 21; j++)
		{
			p_info(PI_TRACE, "i%d: %X ", j, *(&(REL_DATA(i) i0) + j));
			if (!(j | 10))
				p_info(PI_TRACE, "\n");
		}
		p_info(PI_TRACE, "\n");
		for (j = 0; j < 26; j++)
		{
			p_info(PI_TRACE, "d%d: %lX ", j, *(&(REL_DATA(i) d0) + j));
			if (!(j | 10))
				p_info(PI_TRACE, "\n");
		}
		p_info(PI_TRACE, "\n");
		for (j = 0; j < 10; j++)
		{
			p_info(PI_TRACE, "t%d: %lX ", j, *(&(REL_DATA(i) t0) + j));
			if (!(j | 10))
				p_info(PI_TRACE, "\n");
		}
		p_info(PI_TRACE, "\n");
	}
}
#endif

int get_lay_wd(WYSIWYG *wn, LC_LAY_REC *Mem, char *lay_width, char *lay_depth)
{
	char line[LINE_SIZE], *ptr_value;
	Pfd lay_file;
	int len;
	int num;
	int index, i;
	int wid_ind = 0, dep_ind = 0;
	int found = 0;

	if (!Mem)
		return False;
	lay_file = p_open(wn -> tree_name, LAYOUT_FILE, wn -> dir_name, Mem -> filename, "r");
	if (!lay_file)
		return False;
	len = strlen(FRAME_STRING);
	index = (sizeof(reserved_words) / sizeof(RESERVED_WORDS));
	for (;;)
	{
		memset(line, 0, sizeof(line));
		if (!p_fgets(line, sizeof(line), lay_file))
		{
			p_close(lay_file);
			return False;
		}
		if (!strncmp(FRAME_STRING, line, len))
			break;
   	}
/* Found FRAME_STRING, check that it's frame 0 and search for TRIM_WI, TRIM_DE */
	ptr_value = strchr(line, ':');
	if (ptr_value)
		ptr_value += 1;
	else
	{
		p_close(lay_file);
		return False;
	}
	num = atoi(ptr_value);
	if (num != 0)
	{
		p_close(lay_file);
		return False;
	}
/* Find indexes in reserved_words */
	for (i=0; i< index; i++)
	{
		if (reserved_words[i].index == LLAY_WIDTH)
			wid_ind = i;
		if (reserved_words[i].index == LLAY_DEPTH)
			dep_ind = i;
/* Already found both ?*/
		if (dep_ind != 0 && wid_ind != 0)
			break;
	}

	while(found < 2)
	{
		memset(line, 0, sizeof(line));
		if (!p_fgets(line, sizeof(line), lay_file))
		{
			p_close(lay_file);
			return False;
		}
		if (!strncmp(reserved_words[wid_ind].r_word, line, len))
		{
		   ptr_value = strchr(line, ':');
		   if (ptr_value)
		      ptr_value ++;
		   else
		   {
		      p_close(lay_file);
		      return False;
		   }
		   while(ptr_value - line < LINE_SIZE && *ptr_value == ' ')
		      ptr_value++;
		   found++;
		   strcpy(lay_width, ptr_value);
		}
		else if (!strncmp(reserved_words[dep_ind].r_word, line, len))
		{
		   ptr_value = strchr(line, ':');
		   if (ptr_value)
		      ptr_value ++;
		   else
		   {
		      p_close(lay_file);
		      return False;
		   }
		   while(ptr_value - line < LINE_SIZE && *ptr_value == ' ')
		      ptr_value++;
		   found++;
		   strcpy(lay_depth, ptr_value);
		}
   	}
	p_close(lay_file);
	return True;
}



static int read_layout(WYSIWYG *wn, Pfd fd, char *fname)
{
	char line[100], *ptr_rw, *ptr_value;
	int i, k, frame_number = -1, num_poly, read_data = 0;
	
	for (;;)
	{
		char *data = NULL;
		char *scan;
		int length = 0;
		
		memset(line, 0, sizeof(line));
		if (!p_fgets(line, sizeof(line), fd))
			return(read_data);
		P_CLEAN(line);
		
		/* If line is longer than amount read, keep reading */
		while (line[sizeof(line)-2] != '\0')
		{
			if (data)
			{
				length += sizeof(line);
				data = p_remalloc(data, length - sizeof(line), length);
			}
			else
			{
				length = sizeof(line);
				data = p_alloc(length);
			}
			strcat(data, line);
			memset(line, 0, sizeof(line));
			if (!p_fgets(line, sizeof(line), fd))
			{
				p_free((char *)data);
				return(read_data);
			}
			P_CLEAN(line);
		}
		if (data)
		{
			length += sizeof(line);
			data = p_remalloc(data, length - sizeof(line), length);
			strcat(data, line);
		}
		else
			data = STRDUP(line);
		for(scan = data; scan && *scan; scan++)
		{
			if(*scan == '~')
				*scan = '\n';
		}
		read_data = 1;
		if (!data[0])			/* Blank line == end of frame */
		{
			if (frame_number > 0 && TYPE_OF_FRAME(frame_number) == PL_GRAPHIC) {
				if (zoom_revision == 0) {
/* Multiply gr frame scale factors by 100 if zoom_revision == 0 */
					ZOOM_GR(frame_number) *= 10;
					ZOOM_GRY(frame_number) *= 10;
				}
/* Now the zoom values are correct, put them in the strings */
				(*(controls[wig_p_gr_zoomx_gr].func))
					(wn, &controls[wig_p_gr_zoomx_gr], 
					 frame_number, NULL, 1);
				(*(controls[wig_p_gr_zoomy_gr].func))
					(wn, &controls[wig_p_gr_zoomy_gr], 
					 frame_number, NULL, 1);
			}
				
			p_free((char *)data);
			return(1);
		}
		ptr_value = strchr(data, ':');
		if (!ptr_value)
		{
			if (trace_error)
				p_info(PI_TRACE, "%s: Invalid structure for line \"%s\" in \
file. Skipping it\n", fname, data);
			p_free((char *)data);
			continue;
		}
		*ptr_value++ = '\0';
		ptr_rw = data;
		while (*ptr_rw == ' ') ptr_rw++;
		while (*ptr_value == ' ') ptr_value++;
		k = strlen(ptr_rw);
		if (k > size_index)
		{
			if (trace_error)
				p_info(PI_TRACE, "%s: Invalid line \"%s\" in file. \
Skipping it\n", fname, data);
			p_free((char *)data);
			continue;
		}
		for (i = rw_index[k]; i < rw_index[k + 1] && i != -1; i++)
			if (!strcmp(ptr_rw, reserved_words[i].r_word))
				break;
		if (i < rw_index[k + 1])
		{
			if (reserved_words[i].index == -1
				&& !strcmp(reserved_words[i].r_word, FRAME_STRING))
			{					/* FRAME# */
				frame_number = atoi(ptr_value);
				p_free((char *)data);
				continue;
			}
			if (frame_number == -1)
			{
				if (trace_error)
					p_info(PI_TRACE, "%s: We do not have a frame number \
yet!!! Ignoring line \"%s\" in file\n", fname, data);
				p_free((char *)data);
				continue;
			}
			if (reserved_words[i].index != -1
			    && controls[reserved_words[i].index].func
			    && frame_number != -1) 
			{
				switch (controls[reserved_words[i].index].type)
				{
				  case MHWpolyDots:
					num_poly = atoi(ptr_value);
					if (num_poly)
					{
						DRAW_POINT_X_Y *pts;
						
						if (controls[reserved_words[i].index].offset
							== SHAPE)
						{
							FRAME_FLAGS(frame_number) |= POLYGON_SHAPE;
							FRAME_DATA(frame_number) num_shapes = num_poly;
							pts = FRAME_DATA(frame_number) shape_pts =
								(DRAW_POINT_X_Y *)
									p_alloc(sizeof(DRAW_POINT_X_Y) * num_poly);
						}
						else
						{
							FRAME_FLAGS(frame_number) |= BEAROFF_SHAPE;
							FRAME_DATA(frame_number) num_bearoffs = num_poly;
							pts = FRAME_DATA(frame_number) bearoff_pts =
								(DRAW_POINT_X_Y *)
									p_alloc(sizeof(DRAW_POINT_X_Y) * num_poly);
						}
						for (k = 0; k < num_poly; k++)
						{
							ptr_value = strchr(ptr_value, ';');
							if (!ptr_value)
							{
								p_fgets(line, sizeof(line) - 1, fd);
								ptr_value = line;
							}
							else
								ptr_value++;
							pts[k].x = atoi(ptr_value);
							ptr_value = strchr(ptr_value, ',') + 1;
							pts[k].y = atoi(ptr_value);
						}
					}
					break;
				  case MHWtextPtr:
					(*(controls[reserved_words[i].index].func))
						(wn,
						 &controls[reserved_words[i].index], 
						 frame_number, ptr_value, 0);
					break;
				  case MHWoption:
					if (controls[reserved_words[i].index].length == 2)
					{
						uint16 value;
						value = (uint16)atoi(ptr_value);
						if (reserved_words[i].index == 189 && !Autosizing)
/* Set autosizing to false */
							value &= ~AUTO_SZ_SEL;
						(*(controls[reserved_words[i].index].func))
							(wn, &controls[reserved_words[i].index], 
							 frame_number, &value, 0);
					}
					else
					{
						u_char value;
						value = (u_char)atoi(ptr_value);
						(*(controls[reserved_words[i].index].func))
							(wn, &controls[reserved_words[i].index], 
							 frame_number, &value, 0);
					}
					break; 
				  case MHWoptionButton:
				  case MHWundefint:
				  case MHWtoggle:
					{
						u_char value;
						
						value = (u_char)atoi(ptr_value);
						(*(controls[reserved_words[i].index].func))
							(wn, &controls[reserved_words[i].index], 
							 frame_number, &value, 0);
					}
					break;
				  default:
					if (reserved_words[i].index == wig_p_gr_zoomx_gr) {
/* Set zoomy=zoomx if zoomy will not be specified   */
						zoom_revision = 0;
						ZOOM_GR(frame_number) = atoi(ptr_value);
						ZOOM_GRY(frame_number) = atoi(ptr_value);
					}
					else if (reserved_words[i].index == wig_p_gr_zoomy_gr) {
						zoom_revision = 1;
						ZOOM_GRY(frame_number) = atoi(ptr_value);
					}
					else
						(*(controls[reserved_words[i].index].func))
							(wn, &controls[reserved_words[i].index], 
							 frame_number, ptr_value, 0);
					break;
				}
			}
		}
		else if (trace_error)
			p_info(PI_TRACE, "%s: Invalid reserved word \"%s\". \
Skipping the line\n", fname, data);
		p_free(data);
	}
}

int frame_rd(WYSIWYG *wn, int dir_type, char *filename)
{
	Pfd fd;
	
#ifdef TRACE
	if (trace_debugger)
		p_info(PI_TRACE, "frame_rd %d %s\n", dir_type, filename);
#endif
	if ((fd = p_open(wn -> tree_name, dir_type, wn -> dir_name,
					 filename, "r")) == P_ERROR)
	{
		if (trace_error)
			p_info(PI_TRACE, "ERROR opening %s", filename);
		return(1);
	}
	/* clean up existing frame data from menu */
	if (LAYOUT(wn) frames)
		p_free((char *)LAYOUT(wn) frames);
	if (LAYOUT(wn) f_chunks.head)
		Qclear(&LAYOUT(wn) f_chunks);
	LAYOUT(wn) frames = NULL;
	LAYOUT(wn) max_frames = 0;
	LAYOUT(wn) num_frames = -1;
	if (!rw_index)
	{
		int index, i, length, prev_length = 0;
		
		index = (sizeof(reserved_words) / sizeof(RESERVED_WORDS)) - 1;
		size_index = strlen(reserved_words[index].r_word) + 2;
		/* This pointer is never freed because we will need it as long as
		   designmaster lives */
		rw_index = (int *)p_alloc(size_index * sizeof(int));
		length = strlen(reserved_words[0].r_word);
		for (i = 0; i < length; i++)
			rw_index[i] = -1;
		for (i = 0; i <= index; i++)
		{
			length = strlen(reserved_words[i].r_word);
			if (prev_length < length)
			{
				rw_index[length] = i;
				prev_length = length;
			}
		}
		rw_index[size_index - 1] = index + 1;
		size_index -= 2;		/* Set to the real length for testing */
	}
	do
	{
		int kkk;
		frame_add_1_frame_struct(wn);
		kkk = LAYOUT(wn) num_frames;
/* Set it in advance, will be overwritten if specified */
		BLEND_SSHADE(kkk)=BLEND_ESHADE(kkk)=
		  FG_SHADE(kkk)=BG_SHADE(kkk)=OUT_SHADE(kkk) = 100;
	} while(read_layout(wn, fd, filename));
	p_free((char *)LAYOUT(wn) frames[LAYOUT(wn) num_frames] -> ele);
	LAYOUT(wn) num_frames--;
	p_close(fd);
	return(0);
}

void frame_expand(WYSIWYG *wn)
{
	int size;
	int i;
	FR_CHUNK *fc;
	
	size = (LAYOUT(wn) max_frames + FPC) * sizeof(FRAME *);
	if (LAYOUT(wn) frames)
		LAYOUT(wn) frames =
			(FRAME **)p_remalloc((char *)LAYOUT(wn) frames,
								 (LAYOUT(wn) max_frames) * sizeof(FRAME *),
								 size);
	else
		LAYOUT(wn) frames = (FRAME **)p_alloc(size);
	fc = (FR_CHUNK *)p_alloc(sizeof(FR_CHUNK));
	for(i = 0; i < FPC; i++)
		LAYOUT(wn) frames[LAYOUT(wn) max_frames + i] = &fc -> frames[i];
	QinsertTail(&LAYOUT(wn) f_chunks, (QE *)fc);
	LAYOUT(wn) max_frames += FPC;
}

void frame_add_1_frame_struct(WYSIWYG *wn)
{
	LAYOUT(wn) num_frames++;
	if (LAYOUT(wn) num_frames >= LAYOUT(wn) max_frames)
		frame_expand(wn);
	(*(LAYOUT(wn) frames + LAYOUT(wn) num_frames)) -> ele =
		(ELEMENT *)p_alloc(sizeof(ELEMENT));
}

int frame_wr(WYSIWYG *wn, int dir_type, char *filename)
{
	int i, j;
	int new_max_uid, save_max_uid;
	char data[80], temp [128];
	char *frame_field;
	Pfd fd;
	
#ifdef TRACE
	if (trace_debugger)
		p_info(PI_TRACE, "frame_wr %d %s\n", dir_type, filename);
#endif
	if ((fd = p_open(wn -> tree_name, dir_type, 
					 wn -> dir_name, filename, "w+")) == P_ERROR) 
	{
		p_info(PI_ELOG, "in frame_wr(): **ERROR** opening %s", filename);
		return(1);
	}
	if (!controls[0].ptr_rw)
	{
		for (i = 0;
			 i < sizeof(reserved_words) / (sizeof(RESERVED_WORDS)); i++)
		{
			if (reserved_words[i].index == -1)
				continue;
			controls[reserved_words[i].index].ptr_rw
				= reserved_words[i].r_word;
		}
#if 1
	  {{
	  int maxoffset = 0;
	  int index = 0;
	  for (j = 0; j < BOUND_CONTROLS; j++)
	  {
		  int offset;

		  if (controls[j].type == MHWtoggleOpt)
			  offset = controls[j].offset + 5;
		  else
			  offset = controls[j].offset + controls[j].length;
		  if ( offset > maxoffset)
		  {
			  maxoffset = offset;
			  index = j;
		  }
	  }

	  if ( maxoffset >= CONTROL_SIZE)
		  p_info (PI_ELOG,
	"\n\n\t\t****YOU ARE GOING TO CRASH!!!****\nCONTROL_SIZE is only %d\nIt must be %d (index = %d)\n\n",
				CONTROL_SIZE, maxoffset+1, index);
	  }}
#endif
	}
	save_max_uid = FRAME_DATA(0) max_uid; /* from frame 0 */
	new_max_uid = save_max_uid;
	if (new_max_uid > 4095)
		p_info(PI_ELOG, "In frame_wr, '%s' MAX_UID went above max (4095)\n",
				  filename);
	for (i = 1; i <= LAYOUT(wn) num_frames; i++)
	{							/* get new max_uid count needed for frame 0 */
		if ( !( FRAME_DATA(i) frame_uid ) )
			new_max_uid += 1;
	}
	for (i = 0; i <= LAYOUT(wn) num_frames; i++)
	{
		if ( i  && !( FRAME_DATA(i) frame_uid ) )
		{						/* frame not #0 and has no uid */
			FRAME_DATA(i) frame_uid = save_max_uid + 1;
			save_max_uid += 1;
		}
		else if ( !i )
			FRAME_DATA(0) max_uid = new_max_uid; /* frame 0 */
		switch (TYPE_OF_FRAME(i))
		{
		  case PL_SPECS:		/* Specification frame */
			frame_field = spec_frame_field;
			break;
		  case PL_TEXT:			/* Text frame */
			frame_field = text_frame_field;
			break;
		  case PL_GRAPHIC:		/* Graphic frame */
			frame_field = graphic_frame_field;
			break;
		  case PL_MISC:			/* Misc. frame */
			frame_field = misc_frame_field;
			break;
		  case PL_RBX:			/* Rule/Box frame */
			frame_field = rb_frame_field;
			break;
		  case PL_EMPTY:		/* Empty frame */
			frame_field = empty_frame_field;
			break;
		  case PL_FLOW:			/* Free form frame */
			frame_field = free_form_frame_field;
			break;
		  default:
			continue;
		}
		sprintf(temp, "%s: %d\n", FRAME_STRING, i);
		p_fputs(temp, fd);
		for (j = 0; j < BOUND_CONTROLS; j++)
		{
			if (controls[j].func && frame_field[j])
			{
				int val;
				memset(data, 0, sizeof(data));
				if (controls[j].type == MHWpolyDots)
				{
					int num_pts = 0;
					
					if (FRAME_FLAGS(i) & POLYGON_SHAPE
						&& controls[j].offset == SHAPE)
						num_pts = FRAME_DATA(i) num_shapes; 
					else if (FRAME_FLAGS(i) & BEAROFF_SHAPE
							 && controls[j].offset == BEAROFF)
						num_pts = FRAME_DATA(i) num_bearoffs; 
					if (num_pts)
					{
						char *str;
						
						sprintf(data, "%s: %d", controls[j].ptr_rw, num_pts);
						p_fputs(data, fd);
						str = p_alloc(16 * num_pts);
						memset(str, 0, sizeof(str));
						(*(controls[j].func))(wn, &controls[j], i, str, 1);
						p_fputs(str, fd);
						p_fputs("\n", fd);
						p_free(str);
					}
					continue;
				}
				switch (controls[j].type)
				{
				  case MHWtextPtr:
					{
						char **str1, *str2, *scan;
						
						if ((str1 = (&REL_DATA(i) p0
									 + controls[j].off_rel)) && *str1)
						{
							str2 = p_alloc(strlen(*str1) + 
										   strlen(controls[j].ptr_rw) + 4);
							for(scan = *str1; scan && *scan; scan++)
							{
								if(*scan == '\n')
									*scan = '~';
							}
							sprintf(str2, "%s: %s\n", 
									controls[j].ptr_rw, *str1);
							for(scan = *str1; scan && *scan; scan++)
							{
								if(*scan == '~')
									*scan = '\n';
							}
						}
						else
						{
							str2 = p_alloc(strlen(controls[j].ptr_rw)+4);
							sprintf(str2, "%s: \n", controls[j].ptr_rw);
						}
						p_fputs(str2, fd);
						p_free(str2);
					}
					break;
				  case MHWoption:
					(*(controls[j].func))(wn, &controls[j], i, data, 1);
					if (controls[j].length == 2)
						sprintf(temp, "%s: %d\n",
								controls[j].ptr_rw, *(uint16 *)data);
					else
						sprintf(temp, "%s: %d\n",
								controls[j].ptr_rw, (u_int)(u_char)data[0]);
					p_fputs(temp, fd);
					break;
				  case MHWoptionButton:
				  case MHWundefint:
				  case MHWtoggle:
					(*(controls[j].func))(wn, &controls[j], i, data, 1);
					sprintf(temp, "%s: %d\n",
							controls[j].ptr_rw, (u_int)(u_char)data[0]);
					p_fputs(temp, fd);
					break;
				  default:
					(*(controls[j].func))(wn, &controls[j], i, data, 1);
					if (j == wig_p_gr_zoomx_gr || j == wig_p_gr_zoomy_gr) {
/* Multuply by 10 */
						val = (int)(atof(data) * 10.);
						itoa(val, data);
					}
					sprintf(temp, "%s: %s\n", controls[j].ptr_rw, data);
					p_fputs(temp, fd);
					break;
				}
			}
		}
		p_fputs("\n", fd);
	}
	p_close(fd);
	return(0);
}
