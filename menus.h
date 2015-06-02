#ifndef _MENUS_H

#define _MENUS_H

#include "window.h"

/* Set's size of Publication Menu Icons */
#define PUB_button_width	80
#define PUB_button_depth	115
#define CROP_BIT_ZOOM 0x0010

/*
 *		widget type defines for control section of menu handlers
 */
#define MHWwindow		0	/*	n/a		n/a		n/a		*/
#define MHWtext			1	/*	type	length	offset	*/
#define MHWoption		2	/*	type	length	offset	*/
#define MHWoptionButton	3	/*	type	value	parent	*/
#define MHWtoggle		4	/*	type	length	offset	*/
#define MHWcascade		5	/*	type	length	offset	*/
#define MHWradioButton  6	/*	type	value	parent	*/
#define MHWselection	7	/*	type	length	offset	*/
#define MHWpushButton	8	/*	type	length	offset	*/
#define MHWlabel		9	/*	type	length	offset	*/
#define MHWtoggleOpt   10	/*	type	length	offset	*/
#define MHWpolyDots    11	/*	type	length	offset	*/
#define MHWtextPtr	   12	/*	type	length	offset	*/
#define MHWnothing	   13	/*	type	length	offset	*/
#define MHWundef	MHWnothing
#define MHWundefint	MHWnothing + 1

/*
 * defines for file type selected from system menu
 */
#define EDIT       0
#define DESIGN     1
#define GRAPHIC    2

#define FRAMED    0x1
#define LABELED   0x2
#define WYSIWYGED 0x4

#define FRAME_OPT  FRAMED
#define LABEL_OPT  LABELED | FRAMED
#define WYS_OPT    WYSIWYGED
#define F_WYS_OPT  WYSIWYGED | FRAMED

/*
 *		defines for type field in all menus
 */
#define MHTapplyFrame	-1
#define MHTsystem		1
#define MHTlocks		2
#define MHTtextFrame	3
#define MHTmiscFrame	4
#define MHTgraphicFrame	5
#define MHTrbFrame		6
#define MHTemptyFrame	7
#define MHTspecsFrame	8
#define MHTgalley		9
#define MHTpub			10
#define MHTlayout		11
#define MHTcreateUnit	12
#define MHTsaveAs		13
#define MHTcopyLayout	14
#define MHTgraphics		15
#define MHTstf			16
#define MHTvjt			17
#define MHTflowFrame	18
#define MHTgroupCopy	19
#define MHTsaveReplace	20
#define MHTsaveLayout	21
#define MHTpaginateMenu	22
#define MHTbreakingMenu	23
#define MHTchapterMenu	24
#define MHTillustrationMenu	25
#define MHTfootnoteMenu	26
#define MHTsidenoteMenu	27
#define MHTvert_spacingMenu	28
#define MHTSetupPages	29
#define MHTSetupText	30
#define MHTSetupTables	31
#define MHTSetupChapter	32
#define MHTManagePages	33
#define MHTcolorFrame   34
#define MHTpaletteFrame 35

/*
 *		defines of calltypes used for penta menu callbacks
 */
#define CB_M_createWidget	1
#define MHtextChanged		2
#define MHtextPtrChanged	3
#define MHtoggleChanged		4
#define MHoptionChanged		5
#define MHdatumChanged      6
#define MHselectionChanged	7
#define MHselectionMade		8
#define MHselectionAdded    9

#define MHupdateDisplay		10
#define MHexitRequest		11
#define MHcancelRequest		12
#define MHmenuDestroyed		13
#define MHmenuMap			14
#define MHokRequest			15
#define MHopenRequest		16
#define MHcloseRequest		17
#define MHaddRequest		18
#define MHdeleteRequest		19
#define MHupdateRequest		20
#define MHremoveRequest		21
#define MHnextRequest		22
#define MHprevRequest		23
#define MHiconifyRequest	24
#define MHsubiconifyRequest	25
#define MHeditRequest       26
#define MHdefaultAction     27
#define MHbuttonSelected    28
#define MHunmap				29
#define MHreal_destroy		30
#define P_ALPHA 0
#define P_DIGIT 1
#define P_COLOR 2
#define P_PERCENT 3
#define P_INT6POS 4 /* Integer from 0 till 999999 */
#define P_PERCENT0 5
#define P_BLANGLE  6
#define P_SEP 7

/*
 *			all callbacks have the following format:
 *		(menu,widget,widget_number,chstr,value,calltype)
 */
extern QUEUE Menus;

#define BOUND_CONTROLS 384  /* Always one greater than list in frame_io */
#define CONTROL_SIZE 1069
#define GR_CHANGE    6

#define LINK_STRING_LENGTH 30
#define LONG_STRING        64

#define MENU_HEADER \
    QE que;			/* pointer to next & previous items */	\
    short type;			/* type of display item */ \
    WYSIWYG *wn;		/* window this menu refers to */ \
    void *this_menu; \
    int (*func)()
     
typedef struct {
	MENU_HEADER;
} ANY_MENU;

typedef struct {
	MENU_HEADER;
	ANY_MENU *parent_menu;
} ANY_AUX_MENU;

#define GALLEY_CONTROL_ITEMS 14

typedef struct {
	MENU_HEADER;
	FILE_LIST *unit_fl;
	char *data;
	void *widgets[GALLEY_CONTROL_ITEMS];
} ANY_GALLEY_MENU;

#endif
