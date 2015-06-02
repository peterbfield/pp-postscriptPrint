#ifndef _PSJOB_H
#define _PSJOB_H

#include "window.h"
#include "line_def.h"
#include "Psftable.h"

typedef struct f_list {
	char * font;				/* Postscript's font handle */
	int mask;					/* color seps needing the font */
	struct	f_list *nxt;		/* pointer to next font */
}F_LIST;

#define MAX_CLRS 32

struct vid_box {				/* video box list structure */
    float x, y, x1, y1;			/* location of diag corners of bounding box */
	float  rot_x, rot_y;		/* location of center of rotation */
	int16 page_rotation_angle;	/* angle of rotation */
    struct vid_box *nxt;		/* the next one */
    struct vid_box *prv;		/* the previous one */
	ELEMENT *elem;
    int fg_color, bg_color;		/* foreground and background */
    int fg_shade, bg_shade;
	int16 rel_index, ele_index;
	int cmd_bf_flag;			/* one if from cmd bf */
};

struct clr_lst {				/* color descr. list structure */
    int	color;					/* which color is this? */
	int reverse;				/* 1 = reverse (white type) */
    int	freq;					/* frequency of pattern */
	int cmyk_flag;				/* 1 if cmyk is input */
	float cmyk[4];				/* 4 cmyk values */
	float out_cmyk[4];				/* 4 cmyk values */
    float density;				/* in PS style: 1.0 = white */
    float tint;
    double angle;				/* rotation angle */
    char func[32];				/* PS function to use */
	char type[12];
	char op[6];					/* Overprint flag */
	char color_name[32];		/* Color Name */
    struct clr_lst *nxt;		/* the next one of these */
};


struct ps_pieces
{
	int frame_nbr;
	int orig_frame_nbr;
	int ele_nbr;
	ELEMENT *elem;
	ELEMENT *orig_elem;			/* ele for frame with object */
	int frame_type;				/* 0 = end file
								   1 = new page
								   2 = end page
								   3 = crop marks
								   4 = box rule
								   6 = overflow
								   7 = text
								   8 = graphic */
	WYSIWYG *frame_wn;			/* wn for this piece */
	WYSIWYG *text_wn;			/* wn for text content of this piece, if dif-
									ferent from *frame_wn (eg for OBJ_REF) */
	struct ps_pieces *next;		/* pointer to next ps_pieces structure */
};

struct obj_pg_wn
{
	int page_number[11];
	WYSIWYG *wns[11];
};

struct wid_val					/* Converted widths from WIDTH.xx */
{
    int16 fontnum;				/* Font # of this entry. */
    int16 setsize;				/* Set size of this entry. */
    int16 roundoffs[256];		/* Width remainders, from WIDTH.xx */
};

struct plate_name
{
	char name[32];
} plate_nm[33];

struct kmd80_stuff				/* [bf storage */
{
	int active_flag;
 	int start_depth;
 	int16 corner_radius;
 	int16 trapping_offset;
 	int bg_color;
 	int bg_shade;
	int upr_left_x;
	int16 width;
	int16 depth;
	int16 original_depth;
	uint32 active_trap_cc_mask;
	int active_trap_color;
	int16 active_trap_stroke_width;
	int16  active_trap_shade;
	int blend_flag;
	int16 blend_angle;
	int blend_end_color;
	int blend_end_shade;
	int16 anchr_amts[20];
	int anchr_clrs[20];
	int anchr_shades[20];
	int anchr_cnt;
};

struct mpu_file
{
	struct mpu_file *next;		/* pointer to next mpu_ structure */
	char filename[128];			/* pointer to saved filename */
};

struct mpu_lp
{
	struct mpu_lp *next;		/* pointer to next mpu_txp structure */
	int16 line_count;			/* how many lines saved */
	char* page_saved;			/* pointer to saved page */
};

struct mpu_txp
{
	struct mpu_txp *next;		/* pointer to next mpu_txp structure */
	int16 line_count;			/* how many lines saved */
	char* page_saved;			/* pointer to saved page */
};

int bmlevel;
int bmctr;
int bmtop;
int uidctr;
int uidtop;
int pdflinkctr;
int pdflinkAtSOL;

int PdfActive;					/* 1=PDFstyle given & used at print menu.
								   0=Not.   */
int pdf_note_disp;
int pdf_note_length;
int pdf_note_depth;
int pdf_note_color;
int pdf_note_pos;
char pdf_note_ocolor[20];

int CropBoxFlag;
int cropboxd;
int cropboxw;
int pdf_output_lev;
int pdf_crop_dox;
int pdf_crop_doy;
int pdf_crop_dex;
int pdf_crop_dey;
int pdf_crop_cox;
int pdf_crop_coy;
int pdf_crop_cex;
int pdf_crop_cey;
int pdf_crop_cw;
int pdf_crop_cd;
int pdf_crop_dw;
int pdf_crop_dd;
int pdf_bead;

float ltrimd, ltrimw;
int pdfxoff, pdfyoff;

int cbllx, cblly, cburx, cbury;

typedef struct 
{
	char title[128];
	char id[34];
	char rid[34];
	int disp;
	int view;
	char dest[256];
} pdf_bookmarks;
pdf_bookmarks *pdf_bookmark;

typedef struct
{
	char ndest[256];
	char destarg[64];
	int desttype;
	int weight;
	int dashlen;
	int dashgap;
	int radius;
	int btype;
	int bcolor;
	char ocolor[20];
} pdf_links;
pdf_links pdf_link[500];

typedef struct
{
	char ndest[256];
	int weight;
	int dashlen;
	int dashgap;
	int radius;
	int btype;
	int bcolor;
	char ocolor[20];
} job_links;
job_links job_link;

typedef struct 
{
	char title[8192];
	char subj[257];
	char keyw[1025];
	char auth[129];
	char creator[34];
	char date1[26];
	char date2[26];
} pdf_Info;
pdf_Info pdf_info;

typedef struct
{
	int initpage;
	int initdisp;
	int initsize;
	float initzoom;
} pdf_Docview;
pdf_Docview pdf_docview;

typedef struct
{
	char id[34];
	int usage;
} uidtables;
uidtables *uidtable;

struct fn_global {		/* Per-page storage of global-footnote info.  */
	int id;				/* User's footnote ID, assigned in HandJ  */
	int reference;		/* [fm ref# assigned to foot for current page  */
};
						/* Storage of info on up to 10 global footnotes
							on current page, zeroed at S.o.P.:  */
struct fn_global fnglobals[10];
int num_fnglobals;		/* Bump at each new global fn on the page.  */
 
#include "psjob.f"

extern Pfd Fofd;
extern char JobName[];          /* unit or galley name, no extensions */
extern char SubDirName[];		/* sub_dir name + extension .prj */
extern char ParentDirName[];	/* parent's sub_dir name + extension .prj */
extern char NextDirName[];		/* sub_dir name found in parent's member.list  */
extern char TreeName[];
extern char tpname[];			/* Name of TP file for this unit. */
extern char prologname[];
extern FILE *PrologName;
extern FILE *TrailerName;
extern char LayoutName[];
extern char PageName[];
extern char FirstProjofBook[];
extern char LastProjofBook[];
extern unsigned char Line[];	/* Characters to be set on line. */
extern int FirstPage;
extern int LastPage;
extern int BookPrint;
extern int ParentIndex;
extern int LastParentIndex;
extern char ColorTableName[];
extern int16 HorizontalBase;
extern int16 VerticalBase;
extern char FoName[];
extern int FileType;			/* 0 = galley, 1 = layout */
extern uint32 cc_mask;			/* current color mask */
extern uint32 cc_hit;			/* all possible pass 2 colors */
extern uint32 active_trap_cc_mask;
extern uint32 Plates;
extern LDEF fo_line_def;
extern int debugger_trace;
extern int color_trace;
extern int text_trace;
extern int spot_pass;			/* spot color pass (1 or 2) */
extern int in_overflow;
extern int MasterNameFlag;
extern int parse_trace;
extern int crop_flag;
extern int was_text;
extern int utype;
extern int usw;
extern int CircleFlag;
extern int16 anchor_amounts[];
extern int anchor_colors[];
extern int AnchorCount;
extern float RoundCornerRadiusTL;
extern int ChapterPage;
extern int ChapterPageSetFlag;
extern float ScaleFactorX;
extern float ScaleFactorY;
extern int KeyTrimFlags;
extern int KeyScale;
extern int KeyScaleX, KeyScaleY;
extern int Reports;
extern int ReportFlag;
extern int ReportSize;
extern int SupressBlankPages;
extern int FootnoteFrameFlag;
extern int GalleySlugFlag;
extern int MultiPagesUp;
extern int MultiPagesOddEvenFlag;
extern uint32 MultiOddPage_cc_mask;
extern int setpage_allowed;
extern int obj_wn_count;
extern int overflow_page_buff_count;
extern int page_number[];
extern int vj_pending;
extern int not_lnum;
extern int last_good_line;
extern int blend_flag;
extern int16 blend_angle;
extern int16 header_offset;
extern int16 khar_wid;
extern int16 ackhar;
extern int SetX;
extern int SetY;
extern int16 jbl;
extern int16 foreads[];
extern int16 foreadsTOP;
extern int16 holdforec, holdfowrd;
extern int16 forec;				/* current FO record, cnt from 1 */
extern int16 fowrd;				/* current FO word, count from */
extern int16 Holdfont, Holdss, Holdps;
extern int16 Oldfont, Oldss, Oldps;
extern int16 FontBase;
extern float Hold_ob, Old_ob;
extern float Kspace[];			/* Space adjustment between chars.*/
extern float Rotate;
extern float Obdegree;
extern int16 PageRotationAngle;
extern int16 PageRotationX;
extern int16 PageRotationY;
extern char *m_fprintf_buff;
extern int m_fprintf_size;
extern int Linesub;
extern int LYPrintFlag;
extern int KeyOutputType;
extern int DidPage;
extern int Jrule;
extern int NoFont;
extern int ForceLineNbrFont;
extern float Rwei;
extern int Widindex;			/* Index of current width set among 10. */
extern int RoundoffBucket;		/* Sum remainders until space band */
extern struct wid_val Widval[10]; /* Hold most-recently-used 10 fonts. */
extern struct obj_pg_wn ObjPageWn;
extern struct ps_pieces *PsCurRec;
extern struct ps_pieces *PsHead; /* pointer to first ps_pieces structure */
extern struct ps_pieces *PsTail; /* pointer to last ps_pieces struct */
extern WYSIWYG *wn;
extern int PageNo1, PageNo2, PageNo3;
extern int OffT, OffB;			/* Top, Bottom margin offset. */
extern int OffL, OffR;			/* Left, Right margin offset. */
extern int Orient;				/* 0=Portrait   90=Landscape */
extern int PageH;				/* Page Height in Points. */
extern int PageW;				/* Page Height in Points. */
extern int Imageheight;			/* Pg height less margins in VerticalBase */
extern int PsfNameIndex;		/* Penta font number (index to PentaToPsf)*/
extern int PsfCodeIndex;		/* Index to the PsfCode translation table */
extern int MasterOutputSwitch;
extern int Ktabmode;
extern uint32 BlackPaint_cc_mask;
extern uint32 PaintOnPage;
extern int DontPaintFlag;
extern int CurrentFrame;
extern int CurrentEle;
extern int Xpos, Ypos, Xmark;
extern float Fx, Fy;
extern float Fx2, Fy3;
extern float Ofx;
extern float trim_mark_width, trim_mark_depth;
extern int EditTraceFlag;
extern int EditTraceStarted;
extern int EditTrace_beg_x1;
extern int EditTrace_beg_x2;
extern int ET_FakeTabLineFlag;
extern int ET_InsertStartDelay1;
extern int ET_InsertEndDelay1;
extern int ET_InsertStartDelay2;
extern int ET_InsertEndDelay2;
extern int ET_DeleteDelay1;
extern int ET_DeleteDelay2;
extern int ET_LineStartedFlag;
extern int BcEcFlag;
extern int CurrentLineBcEc;
extern int PrevLineBcEc;
extern int BcEcExtraLeadNeeded;
extern int BcEcExtraLead;
extern int FlashOn;
extern int OverflowOrColorFlag;
extern int CMYK_Allowed;
extern int FgClr;
extern int FgShd;
extern int BgClr;
extern int BgShd;
extern int BgBlendStartClr;
extern int BgBlendStartShd;
extern int BgBlendEndClr;
extern int BgBlendEndShd;
extern int BlendAngle;
extern float DegAngle, RadAngle;
extern float SinAngle, CosAngle;
extern int NewBlendColor;
extern int NewBgColor;
extern int BlendEndColor;
extern float BfClipFx, BfClipFy, BfClipFx2, BfClipFy3;
extern int FgColor;
extern int FgShade;
extern int BgColor;
extern int BgShade;
extern float BfBgShadeStart;
extern float BlendEndShade;
extern int CornerRadiusBfFlag;	/* 1=bf with no blend, 2=bf with blend
								   3=bf with blend, clip bottom
								   4=bf with blend, clip top
								   5=bf with blend, clip top and bottom */
extern int DefaultFrameFgColor;
extern int DefaultFrameFgShade;
extern int ActiveTrapColor;
extern int ActiveTrapShade;
extern int16 ActiveTrapStrokeWidth;
extern int Kmd_PP_Started;
extern int MaxPP_Meas;
extern F_LIST *E_fonts;

/**** EOF ****/

#endif
