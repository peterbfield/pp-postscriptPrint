#ifndef _SPECS_H

#define _SPECS_H

/* Values for macros used in controls struct */
#define AUTO_FIT  0
#define ZOOM_25   25
#define ZOOM_50   50
#define ZOOM_75   75
#define ZOOM_100  100
#define ZOOM_200  200
#define ZOOM_CUSTOM 1

#define NAME_OFFSET	5			/* distance in pixel between bottom
									of layout and top of name box */

#define LABEL_MARGIN_HEIGHT 2   /* Margin height hwne you display a
									name under a layout */

#define LETTER_WIDTH  "8I16"
#define LETTER_DEPTH  "11I"
#define LEGAL_WIDTH   "8I16"
#define LEGAL_DEPTH   "14I"
#define BSIZE_WIDTH   "11I"
#define BSIZE_DEPTH   "17I"

#define DEF_CAP_HGT   "72"
#define DEF_RUL_WT    "1"
#define DEF_BOX_WT    "1"

#define LINES    1
#define PARA     2
#define EL       3
#define EX_PTS   4
#define TOP      5
#define BOTTOM   6
#define TABLE    7
#define CUSTOM  99

#define MEASURE  1
#define PT_SZ    2
#define SET_SZ   3
#define STF_LEAD   4
#define BANDS    5
#define WIDTH    6
#define MINIMUM  7
#define MAXIMUM  8

#define DEFAULT_SPECS ".default_specs"
#define UNTITLED_LAYOUT	DEFAULT_SPECS /* Name of unsaved layout */

#endif
