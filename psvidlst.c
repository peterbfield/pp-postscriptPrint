#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "p_lib.h"
#include "psjob.h"
#include "msg_parse.h"
#include "lpm.h"

#if LPMfloat
extern char LPMK[];
#else
extern unsigned int LPMK;
#endif

extern int lc_parent(char *tree, char *dir, char *parent);
extern int IncludeScreens; /* 1 = set screens; 0 = use device screens */
extern int LPI; /* 0 = use colortable frequency */

static void clear_color_list();
static void store_color(int reset_flag);
static void store_white_and_black();

extern float bb_left, bb_top, bb_right, bb_bottom;
extern struct vid_box *current_vid_box;
extern int repeat_ele, repeat_rel;

int firstscreen;
int colorsused[48]; /* 0 through 3 storage for CMYK */
char customused[48][32];
char cmykcustomused[48][132];

struct plate_name plate_nm[33];

struct vid_box *vid_1st[MAX_CLRS] = { /* the first one */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

struct vid_box *vid_last[MAX_CLRS] = { /* the last one */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

struct clr_lst *clr_1st[MAX_CLRS] = { /* the first one */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
	
struct msg_parse clr_msg[] = 
{
    {"Color", ""},				/*  0  */
    {"Plate", ""},				/*  1  */
    {"Shape", ""},				/*  2  */
    {"Angle", ""},				/*  3  */
    {"Screen", ""},				/*  4  */
    {"Percent", ""},			/*  5  */
    {"Tint", ""},				/*  6  */
    {"C", ""},					/*  7  */
    {"M", ""},					/*  8  */
    {"Y", ""},					/*  9  */
    {"K", ""},					/*  10  */
    {"Pantone", ""},			/*  11  */
 	{"Separation1", ""},		/*  12  */
 	{"Separation2", ""},		/*  13  */
 	{"Separation3", ""},		/*  14  */
 	{"Separation4", ""},		/*  15  */
 	{"Separation5", ""},		/*  16  */
 	{"Separation6", ""},		/*  17  */
 	{"Separation7", ""},		/*  18  */
 	{"Separation8", ""},		/*  19  */
 	{"Separation9", ""},		/*  20  */
 	{"Separation10", ""},		/*  21  */
 	{"Separation11", ""},		/*  22  */
 	{"Separation12", ""},		/*  23  */
 	{"Separation13", ""},		/*  24  */
 	{"Separation14", ""},		/*  25  */
 	{"Separation15", ""},		/*  26  */
 	{"Separation16", ""},		/*  27  */
 	{"Separation17", ""},		/*  28  */
 	{"Separation18", ""},		/*  29  */
 	{"Separation19", ""},		/*  30  */
 	{"Separation20", ""},		/*  31  */
 	{"Separation21", ""},		/*  32  */
 	{"Separation22", ""},		/*  33  */
 	{"Separation23", ""},		/*  34  */
 	{"Separation24", ""},		/*  35  */
 	{"Separation25", ""},		/*  36  */
 	{"Separation26", ""},		/*  37  */
 	{"Separation27", ""},		/*  38  */
 	{"Separation28", ""},		/*  39  */
 	{"Separation29", ""},		/*  40  */
 	{"Separation30", ""},		/*  41  */
 	{"Separation31", ""},		/*  42  */
 	{"Separation32", ""},		/*  43  */
 	{"Plate_Name", ""},			/*  44  */
 	{"Overprint", ""},			/*  45  */
 	{"Type", ""},				/*  46  */
 	{"Name", ""}				/*  47  */
};

#define CLR_KEYWORD_COUNT 50

char default_color_name[132];
Pfd colorfd;

int old_freq;
double old_angle;
char old_func[32];
char old_op[6];

static int cur_color_id;
static int cur_plate;
static int cur_screen;
static float cur_pct;
static char cur_shape[32];
static double cur_angle;
static int cur_reverse;
static int cur_cmyk_flag;
static float cur_cmyk[4];
static int cur_tint;
static char cur_op[6];
static char cur_color_name[32];
static char cur_type[12];

/***********************************************************************
 **  CLEAR_VLIST clear list of frame bg bound box areas started  **
 **              by vid_1st. Release the memory they occupied. **
 ***********************************************************************/

void clear_vlist(void)			/* clear vid_1st list of vid_box structs */
{
    struct vid_box *vp;			/* list-walking pointer */
    int i;
    
#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "clear_vlist\n");
    if(color_trace)
		p_info(PI_TRACE, "clear_vlist (");
#endif
    for (i=0; i<MAX_CLRS; i++)
    {
		vp = vid_1st[i];
		if(vp == 0)
			continue;
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "clearing list %i (",i);
#endif
		while(vp != 0)			/* walk towards the end	*/
		{
			vid_1st[i] = vp->nxt; /* save location of next one */
#ifdef TRACE
			if(color_trace)				
				p_info(PI_TRACE, ".");	/* show a mark apiece */
#endif
			p_free((char *)vp);	/* free the memory */
			vp = vid_1st[i];	/* and point to 'next */
		}
		if(color_trace)
			p_info(PI_TRACE, ")\n");
    }							/* end for(i=0; i<MAX_CLRS; i++) */
#ifdef TRACE
	if(color_trace)
		p_info(PI_TRACE, ")\n");
#endif
}								/* end CLEAR_VLIST */
/***********************************************************************
 **  ADD_TO_VLIST  Add an entry to the vid_box list started at vid_1st.**
 **				(x,y) is upper left and (x2,y3) is lower right corner. **
 ***********************************************************************/

void add_to_vlist(float x, float y, float x2, float y3, int fg, int fg_shd,
				  int bg, int bg_shd, uint32 vlist_plates, int bf_flag)
{
    int i;
    struct vid_box *vp;			/* our new entry's pointer */
	uint32 plates_hold;
	uint32  loop_temp;
    
	if ( !KeyOutputType)
		vlist_plates = 1;
#ifdef TRACE
    if(color_trace)
	{
		p_info(PI_TRACE, "add_to_vlist: x= %.2f, y= %.2f, x2= %.2f, y3= %.2f\n",
			   x, y, x2, y3);
		p_info(PI_TRACE, "pg_deg= %d, pg_rot_x= %d, pg_rot_y= %d\n",
			   PageRotationAngle, PageRotationX, PageRotationY);
		p_info(PI_TRACE, "fg= %d %d, bg= %d %d, plates= %lo, rel= %d, ele= %d\n",
			   fg, fg_shd, bg, bg_shd, vlist_plates, CurrentFrame, CurrentEle);
	}
#endif
    for (i=0; i<MAX_CLRS-1; i++)
    {
		plates_hold = ( loop_temp = (1 << i) ) & vlist_plates;
		if(loop_temp > vlist_plates)
			break;
		if ( !plates_hold)
			continue;
		/* allocate an entry */
		vp = (struct vid_box *) p_alloc(sizeof(struct vid_box));
		if(vp != 0)				/* fill in if it's good	*/
		{
			if(vid_1st[i+1] == 0)	/* if list was empty */
			{
				vid_last[i+1] = vid_1st[i+1] = vp; /* last is first also */
				vp->prv = 0;	/* there is no previous entry*/
			}
			else
			{
				vid_last[i+1]->nxt = vp; /* old last points here */
				vp->prv = vid_last[i+1]; /* and old last is there */
				vid_last[i+1] = vp; /* while this is new last */
			}
			vp->x = x;			/* fill the data in */
			vp->y = y;
			vp->x1 = x2;
			vp->y1 = y3;
			vp->page_rotation_angle = -PageRotationAngle;
			vp->rot_x = PageRotationX / HorizontalBase;
			vp->rot_y = (Imageheight - PageRotationY) / VerticalBase;
			vp->fg_color = fg;
			vp->fg_shade = fg_shd;
			vp->bg_color = bg;
			vp->bg_shade = bg_shd;
			vp->rel_index = CurrentFrame;
			vp->ele_index = CurrentEle;
			vp->elem = PsCurRec -> elem;
			vp->cmd_bf_flag = bf_flag;
			vp->nxt  = 0;	/* and re-terminate the list  */
#ifdef TRACE
			if(color_trace)
				p_info(PI_TRACE, "entry made @ %x in color %d\n", (unsigned)vp, i);
#endif
			continue;
		}
		else					/* what, malloc failed? */
			p_info(PI_ELOG, "add_to_vlist:NO MEMORY FOR ENTRY\n");
		return;
    }							/* end for (i=0; i<MAX_CLRS-1; i++)  */
}								/* end ADD_TO_VLIST */

/***********************************************************************
 **  VLIST_INTERSECT determine if a frame bb rectangle **
 **         intersects any vid_box. **
 **		list started at vid_1st. **
 **		return 0 or pointer to structure **
 ***********************************************************************/
struct vid_box *vlist_intersect(uint32 plate_number)
{
	struct vid_box *vptr;

	if( in_overflow) 
	{
		repeat_ele = 0;
		repeat_rel = 0;
		return(0);					/* no hits */
	}
	if ( !KeyOutputType)
		plate_number = 1;
	if (current_vid_box == 0)
		vptr = vid_1st[plate_number]; /* list-walking pointer, start at head */
	else
		vptr = current_vid_box ->nxt;	/* start with next frame */
#ifdef TRACE
    if(color_trace)
		p_info(PI_TRACE, "vlist_intersect(%.2f, %.2f, %.2f, %.2f) (plate %lo)\n",
				  bb_left, bb_top, bb_right, bb_bottom, plate_number);
#endif
    while(vptr != 0)			/* walk towards the end */
    {
#ifdef TRACE
		if(color_trace)
			p_info(PI_TRACE, "compare %.2f, %.2f, %.2f, %.2f",
					  vptr->x,vptr->y,vptr->x1,vptr->y1);
#endif
		if((					/* if the passed area ...     */
				(bb_top < vptr->y1 ) && /* starts above bottom edge */
				((bb_bottom) > vptr->y) && /* ends below top edge */
				( bb_left < vptr->x1 ) && /* starts right of left edge */
				((bb_right) > vptr->x) /* ends left of right edge */
				) == TRUE)		/* they overlap! */
		{
			if ( (CurrentFrame != vptr -> rel_index) || /* rel is frame nbr */
				 (vptr ->cmd_bf_flag && (CurrentFrame == vptr -> rel_index)) )
			{					/* not current frame or bf in cur, try it */
#ifdef TRACE
				if (color_trace)
					p_info(PI_TRACE, " HIT!!\n");
#endif
				repeat_rel = vptr -> rel_index;
				repeat_ele = vptr -> ele_index;
				return(vptr);	/* so we say so */
			}
		}
#ifdef TRACE
		if(color_trace)
			p_info(PI_TRACE, " miss!\n");
#endif
		vptr = vptr -> nxt;		/* point to 'next'  */
    }
#ifdef TRACE
    if(color_trace)
		p_info(PI_TRACE, "They all miss or at end.\n");
#endif
	repeat_ele = 0;
	repeat_rel = 0;
    return(0);					/* no hits */
}								/* end VLIST_INTERSECT */

/*************************************************************************
 ** VID_COLOR - do setup of gray (& whatever) for a color by index		**
 **		MUST BE WRAPPED IN GS/GR PAIR BY CALLER			**
 *************************************************************************/
void vid_color(int color, int shade, uint32 plate_number)
{
	float temp_dens[4];
        struct clr_lst *clr;
	int ii;
    
#ifdef TRACE
	if (color_trace)
		p_info(PI_TRACE, "start vid_color(%d, %d, %lo), Black= %lo, cc_mask= %lo\n",
			   color, shade, plate_number, BlackPaint_cc_mask, cc_mask);
#endif
	if (color < 0)
	{
		BlackPaint_cc_mask &= ~(1 << (plate_number-1));	/* turn off a plate */
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "new BlackPaint_cc_mask= %lo\n", BlackPaint_cc_mask);
#endif
		return;
	}
	if ( !KeyOutputType)
		clr = find_color(color, 0); /* use color from plate 0 for composite */
	else						/* normal color */
		clr = find_color(color, plate_number); /* find the color structure */
	if(clr == 0)
	{
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "color %d on plate= %lo not defined\n",
				   color, plate_number);
#endif
		BlackPaint_cc_mask &= ~(1 << (plate_number-1));	/* turn off a plate */
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "new BlackPaint_cc_mask= %lo\n", BlackPaint_cc_mask);
#endif
		return;					/* not defined */
	}

#ifdef TRACE
		if (color_trace) {
		p_info(PI_TRACE, "color= %d, reverse= %d, freq= %d, cmyk_flag= %d\n",
			   clr->color, clr->reverse,clr->freq,clr->cmyk_flag);
		p_info(PI_TRACE, "  c= %f, m= %f, y= %f, k= %f, dens= %f, angle= %f, func= %s op=%s\n",
			   clr->cmyk[0],clr->cmyk[1],clr->cmyk[2],clr->cmyk[3],
			   clr->density,clr->angle,clr->func,clr->op);
		}

#endif

/*     if( (clr->density != 1.0) && */ /* need screen if not white or black */
/*        (clr->density != 0.0) )  */
/* always put out screen 7/20/92 -Enhancement 33P from Dee */

#ifdef TRACE
	if (color_trace)
	{
		p_info(PI_TRACE, "%s setoverprint\n",clr->op);
		p_info(PI_TRACE, "%d %f %s setscreen (note - density= %f)\n",
			   clr->freq, clr->angle + (double)Orient,
			   clr->func, clr->density);
		p_info(PI_TRACE, "%d %f %s %s\n",
			old_freq, old_angle, old_func, old_op);
	}
#endif
	/* setoverprint for composites only */
	if ( !strcmp(clr->op,"false") && (!KeyOutputType) )
		m_fprintf("false setoverprint\n");
	else if ( !strcmp(clr->op,"true") && (!KeyOutputType) )
		m_fprintf("true setoverprint\n");

	if ( IncludeScreens )
	{
		/* Use PP Keyword LPI for color frequency */
		/* If LPI = 0, use colortable frequency */

		if (LPI != 0)
			clr->freq = LPI;

		if ( (old_freq != clr->freq) || (old_angle != clr->angle)
			|| (strcmp(old_func, clr->func)) || firstscreen
			|| (strcmp(old_op, clr->op)) )
		{

			m_fprintf("%d %5.2f {%s} setscreen \n",
				  clr->freq, clr->angle + (double)Orient,
				  clr->func); /* set screen from structure */
			firstscreen = 0;
		}
		old_freq = clr->freq;
		old_angle = clr->angle;
		strcpy(old_func, clr->func);
		strcpy(old_op, clr->op);  

	}
	if ( !CMYK_Allowed)
	{							/* enhancement 59P - 7/28/92 */
		temp_dens[0] = 1.0 - (((1.0 - clr->density) * shade) / 100.);
		digi_print(temp_dens[0]); /* set density */
		m_fprintf("setgray\n");
		if ( temp_dens[0] == 1.0)
			BlackPaint_cc_mask &= ~(1 << (plate_number-1));	/* white */
		else
			BlackPaint_cc_mask |= (1 << (plate_number-1)); /* black */
#ifdef TRACE
		if (color_trace)
		{
			p_info(PI_TRACE, "CMYK_Allowed=0, %6.3f setgray (to .tp file)\n",
				   temp_dens[0]);
			p_info(PI_TRACE, "new BlackPaint_cc_mask= %lo\n", BlackPaint_cc_mask);
		}
#endif
	}
	if (clr -> cmyk_flag && !KeyOutputType && CMYK_Allowed)
	{							/* set the specified color */

		if ( !strcmp(clr->type, "process")) {
		/* Add in tint and then shade for process color only */
		   for (ii=0; ii<4; ii++)
			clr->out_cmyk[ii] = ((clr->cmyk[ii] * clr->tint) / 100.);
		   for (ii=0; ii<4; ii++)
			clr->out_cmyk[ii] = ((clr->cmyk[ii] * shade) / 100.);
		} else {
		   for (ii=0; ii<4; ii++)
			clr->out_cmyk[ii] = clr->cmyk[ii];

		}

		temp_dens[0] = clr->out_cmyk[0];
		digi_print(temp_dens[0]); /* set C */
		temp_dens[1] = clr->out_cmyk[1];
		digi_print(temp_dens[1]); /* set M */
		temp_dens[2] = clr->out_cmyk[2];
		digi_print(temp_dens[2]); /* set Y */
		temp_dens[3] = clr->out_cmyk[3];
		digi_print(temp_dens[3]); /* set K */



		if ( !strcmp(clr->type, "process")) {
			/* Add used colors to color list */
			if (temp_dens[0] != 0)
				colorsused[0]=1; /* Has Cyan */
			if (temp_dens[1] != 0)
				colorsused[1]=1; /* Has Magenta */
			if (temp_dens[2] != 0)
				colorsused[2]=1; /* Has Yellow */
			if (temp_dens[3] != 0)
				colorsused[3]=1; /* Has Black */
			m_fprintf(" setcmykcolor\n");
		} else	{
			for (ii=0; ii< 48; ii++) {
				if (strcmp(customused[ii],clr->color_name) == 0) 
					break; /* Already Loaded */
				if (customused[ii][0] ==  '\0')  {
					strcpy(customused[ii],clr->color_name); /* Found Blank Spot for new entry */
					sprintf(cmykcustomused[ii],"%.2f %.2f %.2f %.2f",temp_dens[0], temp_dens[1], temp_dens[2], temp_dens[3]);
					break;
				}
			}
			m_fprintf("%s %.2f pentacustomcolor\n",clr->color_name, ( (100 - clr->tint) + (100 - shade) ) / 100);
		}

		 if ( (temp_dens[0] == 0.0) && (temp_dens[1] == 0.0) &&
			  (temp_dens[2] == 0.0) && (temp_dens[3] == 0.0)) 
			 BlackPaint_cc_mask &= ~(1 << (plate_number-1)); /* white */
		 else
			BlackPaint_cc_mask |= (1 << (plate_number-1)); /* black */
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "%f %f %f %f set CMYK, Black= %lo\n",
				   temp_dens[0], temp_dens[1], temp_dens[2], temp_dens[3],
				   BlackPaint_cc_mask);
#endif
	}
	else if (!clr -> cmyk_flag && CMYK_Allowed)
	{							/* default to black */
		digi_print(0.0);		/* set C */
		digi_print(0.0);		/* set M */
		digi_print(0.0);		/* set Y */
		temp_dens[0] = shade / 100.;
		if( clr->density != 1.0)
		{
			digi_print(temp_dens[0]); /* set K - black */
			BlackPaint_cc_mask = 1; /* black */
		}
		else
		{
			digi_print(0.0);		/* set K - white */
			BlackPaint_cc_mask = 0;	/* white */
			temp_dens[0] = 0.0;
		}

		/* Add used colors to color list */
		if (temp_dens[0] != 0)
			colorsused[3]=1; /* Has Black */

		m_fprintf(" setcmykcolor\n");

#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "0 0 0 %f set CMYK(black), Black= %lo\n",
				   temp_dens[0], BlackPaint_cc_mask);
#endif
	}
}								/* end VID_TINT */

/*************************************************************************/
static void clear_color_list()	/* clear list of colors */
{
    struct clr_lst *tp;			/* list-walking pointer */
    struct clr_lst *tnxt;		/* list-walking pointer */
    int i;
    
#ifdef TRACE
    if (debugger_trace)
		p_info(PI_TRACE, "clear_color_list\n");
    if (color_trace)
		p_info(PI_TRACE, "clear_color_list ()\n");
#endif
    for (i=0; i<MAX_CLRS; i++)
    {
		tp = clr_1st[i];
		clr_1st[i] = 0;
		while(tp != 0)			/* walk towards the end	*/
		{
			tnxt = tp->nxt;		/* save location of next one  */
			p_free((char *)tp); /* free the memory */
#ifdef TRACE
			if (color_trace)
				p_info(PI_TRACE, "Plate #%d, %x->%d,%d,%.2f,%f,%s,%x removed\n",
					   i,(unsigned)tp, tp->color, tp->freq, tp->density, 
					   tp->angle, tp->func, (unsigned)tp->nxt);
#endif
			tp = tnxt;			/* and point to 'next' */
		}						/* end while(tp != 0) */
    }							/* end  for(i=0; i<MAX_CLRS; i++) */
}								/* end function */
/*************************************************************************
 ** ADD_COLOR	add a specified index to the list. If it exists, gripe	**
 **  and change. Don't change index 0 or 1 **
 *************************************************************************/
void add_color(void)
{
	int temp_cur_tint;
	int ii;

    struct clr_lst *tp = clr_1st[cur_plate]; /* list-walking pointer */
    struct clr_lst *lp = clr_1st[cur_plate]; /* pointer to last link */
    int	not_end = FALSE;		/* never reached the end flag */
    
    if(spot_pass != 1)
		return;					/* built on 1st color pass */
    if( (cur_plate < 0) || (cur_plate >= MAX_CLRS) )
    {
		p_info(PI_WLOG, "Attempt to define an invalid color:(%d, %d, %d, %f, %f, %d)\n",
			   cur_color_id,cur_plate,cur_screen,cur_pct,cur_angle,cur_tint);
		return;					/* not the WORST POSSIBLE case	*/
    }
    while(tp != 0)				/* we're looking for the end  */
    {
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, ".");		/* one dot per entry */
#endif
		lp = tp;				/* save address of latest entry */
		if(tp->color == cur_color_id) /* hey, we've seen this one! */
		{
			if (cur_plate)
				p_info(PI_WLOG, "Redefine a valid color: %d, %d, %d, %f, %f, %d, %d\n",
					   cur_color_id,cur_plate,cur_screen,cur_pct,
					   cur_angle,cur_reverse, cur_tint);
			not_end = TRUE;
			break;
		}
		tp = tp->nxt;			/* move to next one */
    }							/* end while(tp != 0) */
#ifdef TRACE
    if (color_trace)
		p_info(PI_TRACE, ")ptr to last found @ %x\n",(unsigned)lp);
#endif
	if(not_end == FALSE)		/* don't allocate existing one	*/
		tp = (struct clr_lst *) p_alloc(sizeof(struct clr_lst));
	if(tp != 0)
	{
		if(not_end == FALSE)
		{
			if (lp == 0)		/* if this is the first entry */
				clr_1st[cur_plate] = tp; /*   set it up */
			else				/* else there was a previous entry */
				lp->nxt = tp;	/* point prev to new allocation */
			tp->nxt = 0;		/* next allocation hasn't been done yet  */
		}
		temp_cur_tint = cur_tint;
		if ( !temp_cur_tint)
			temp_cur_tint = 100;
		tp->tint = temp_cur_tint;
		tp->color = cur_color_id; /* fill the data in */
		if(cur_screen == 0)
			tp->freq = 6;
		else
			tp->freq = cur_screen;
		tp->density = 1.0 - ( (cur_pct * cur_tint) / 10000.);
		tp->angle = cur_angle;
		strcpy(tp->func, cur_shape);
		tp->reverse = cur_reverse;
		strcpy(tp->op,cur_op);
		strcpy(tp->type,cur_type);
		strcpy(tp->color_name,cur_color_name);

		for (ii=0; ii<4; ii++) {
			tp->cmyk[ii] = cur_cmyk[ii];
		}

		tp->tint = temp_cur_tint;
/*
		for (i=0; i<4; i++)
			tp->cmyk[i] = cur_cmyk[i] = ((cur_cmyk[i] * temp_cur_tint) / 100.);
*/
		if ( !cur_plate && cur_cmyk_flag)
		{
			tp->cmyk_flag = 1;	/* only used for plate 0 if input */
			if (cur_cmyk[3] == 1)
				tp->density = 0.0; /* all black */
			else if ( !cur_cmyk[0] && !cur_cmyk[1] && !cur_cmyk[2] &&
					  !cur_cmyk[3] )
				tp->density = 1.0; /* all white */
			else if ( !cur_cmyk[0] && !cur_cmyk[1] && !cur_cmyk[2] &&
					  cur_cmyk[3] )
				tp->density = 1.0 - cur_cmyk[3]; /* all gray */
			else
				tp->density = 1.0 - (((50 * cur_cmyk[0]) + (75 * cur_cmyk[1]) +
									  (30 * cur_cmyk[2]) +
									  (100 * cur_cmyk[3]) ) / 255.);
		}
		else
			tp->cmyk_flag = 0;
#ifdef TRACE
		if (color_trace)
		{
			p_info(PI_TRACE, "add_color #%d fr:%d, dn:%3.2f, an:%3.2f, fn:%s, rv:%d\n",
				   tp->color, tp->freq, tp->density, tp->angle,
				   tp->func,tp->reverse);
			if (tp->cmyk_flag)
				p_info(PI_TRACE, "add_color CMYK Tint:%d C:%.2f M:%.2f Y:%.2f K:%.2f\n",
					   temp_cur_tint, tp->cmyk[0], tp->cmyk[1],
					   tp->cmyk[2], tp->cmyk[3]);
		}
#endif
    }
    else
    {
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "NO MEMORY FOR ENTRY\n");
#endif
    }
}								/* end function */
/*************************************************************************
 **  FIND_COLOR	find entry for & return pointer to a color entry of 	**
 **		the specified index. Return 0 if no such entry exists.	**
 *************************************************************************/
struct clr_lst *find_color(int color, uint32 plate_number)
{
    struct clr_lst *tp;			/* list-walking pointer */

	if (plate_number >= 32)
		plate_number=0;

	if (color < 0)
	{
#ifdef TRACE
		if (color_trace)
		{
			p_info(PI_TRACE, "find_color color unspecified %d, plate_number %lo.\n",
				   color, plate_number);
		}
#endif
		return(0);
	}
	if ( !KeyOutputType)
		plate_number = 0;
	tp = clr_1st[plate_number];
#ifdef TRACE
    if (color_trace)
		p_info(PI_TRACE, "find_color, look for color %d, plate %lo \n",
			   color, plate_number);
#endif
    while(tp != 0)				/* we're looking for the end  */
    {
		if(tp->color == color)	/* hey, we've seen this one! */
		{
#ifdef TRACE
			if (color_trace)
				p_info(PI_TRACE, "found it @ %x.\n",(unsigned)tp);
#endif
			return(tp);
		}
		tp = tp->nxt;			/* move to next one */
    }
#ifdef TRACE
    if (color_trace)
		p_info(PI_TRACE, "Color unspecified %d, plate_number %lo.\n",
			   color, plate_number);
#endif
	return(0);
}								/* end function */
/*************************************************************************
 **	COLOR_CHECK	passed a color index and a second choice 	**
 *************************************************************************/

int color_check(int color, int def)
{								/* color - got it? def - use if not */
    int i;
    uint32 plate;
	uint32  loop_temp;
    struct clr_lst *tp = 0;
    
	if (color < 0)
	{
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "color_check color %d unspecified ret(%d).\n",color,def);
#endif
		return(def);
	}
	if ( !KeyOutputType)
		tp = find_color(color, 0); /* plate zero for composite */
	else
	{							/* look for plate if not composite */
		for (i=0; i<MAX_CLRS-1; i++)
		{
			plate = (loop_temp= ( 1 << i )) & cc_mask;
			if (plate)
			{
				tp = find_color(color, i + 1);
				if (tp)
					break;
			}	    
			if(loop_temp >= cc_mask)
				break;
		}						/* end for (i=1; i<MAX_CLRS-1; i++) */
	}							/* end else not composite */
    if(tp == 0)					/* can't find it? */
	{
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "color_check color %d unspecified ret(%d).\n",color,def);
#endif
		return(def);			/* use second choice */
	}
    else
	{
#ifdef TRACE
		if (color_trace)
			p_info(PI_TRACE, "color_check found color %d @ %x.\n",color,(unsigned)tp);
#endif
	}
	return(color);				/* ok, use it */
}

/*************************************************************************/

void init_color(void)
{
#define CLRBUFSZ 256
    char clr_parse_buff[CLRBUFSZ];
    char clr_parse_buff_temp[CLRBUFSZ];
	char temp_name[128], temp_chap_dir[128];
	char chap_tree[64];
	char chap_dir[64];
	char *result_ptr;
	float temp;
    int length, i;
	int plate_name_input;
	int temp_plate;
	int screen_found_flag;
	int plate_zero_stored = 0;
    int endflag = 0;
	int parent_flag = 0;

	memset((char *)&plate_nm[0].name[0], 0, sizeof(struct plate_name) * 33);
    clear_vlist();              /* clear list of reverses */
    clear_color_list();			/* clear the non-standard colors */
	strcpy(clr_parse_buff, SubDirName);
	if ((result_ptr = strrchr(clr_parse_buff, '.')) != NULL)
		*result_ptr = '\0';
	colorfd = p_get_table1(TreeName, clr_parse_buff, ColorTableName,
						  default_color_name);
	if (colorfd <= 0)
	{							/* parent */
		if(lc_parent(TreeName, SubDirName, temp_name))
		{
			parent_flag++;
			p_parent_path(TreeName, clr_parse_buff, temp_name, chap_tree,
						  chap_dir);
			strcpy(temp_chap_dir, chap_dir);
			if ((result_ptr = strrchr(temp_chap_dir, '.')) != NULL)
				*result_ptr = '\0';
			colorfd = p_get_table1(chap_tree, temp_chap_dir, ColorTableName,
								   default_color_name);
		}
	}
	if (colorfd <= 0)			/* Tree/tables and .default/tables */
		colorfd = p_get_table2(TreeName, clr_parse_buff, ColorTableName,
						  default_color_name);
    if ( (colorfd <= 0) && strcmp(ColorTableName, "colortable") )
	{							/* try using colortable.ct */
		strcpy (clr_parse_buff_temp, "colortable");
		colorfd = p_get_table1(TreeName, clr_parse_buff, clr_parse_buff_temp,
							   default_color_name);
		if (colorfd <= 0)
		{
			if ( parent_flag)
				colorfd = p_get_table1(chap_tree, temp_chap_dir,
									   clr_parse_buff_temp,
									   default_color_name);
		}
		if (colorfd <= 0)		/* Tree/tables and .default/tables */
			colorfd = p_get_table2(TreeName, clr_parse_buff,
								   clr_parse_buff_temp, default_color_name);
		if (colorfd > 0)
			p_info(PI_WLOG, "Color table %s does not exist, using colortable.ct\n",
				   ColorTableName);
	}
    if ( colorfd <= 0 )
	{							/* no colortable, use black and white */
		colorfd = 0;
		store_white_and_black();
		sprintf(default_color_name, "Penta/%s/%s/???.ct",
				DEFAULT_DIR, TABLES_DIR);
		p_info(PI_WLOG, "No color table exists, only colors 0 (white) and 1 (black) are valid.\n");
		return;
	}
    spot_pass = 1;				/* 1st pass */
	cur_color_id = -1;
    while ( !endflag  )
    {
		memset(clr_parse_buff, 0, CLRBUFSZ);
		if ( p_fgets(clr_parse_buff, CLRBUFSZ,
					 colorfd) != clr_parse_buff)
			endflag++;			/* out of data */
		if ( (length = strlen (clr_parse_buff)) < 5 )
			continue;			/* not long enough for a valid string */
								/* OTOH, err if line fills buff completely: */
		if ( length >= (CLRBUFSZ-1) ) {
			p_info(PI_WLOG, "WARNING: Color table '%s' line too long, ignored after %d:\n '%s'\n",
				ColorTableName, CLRBUFSZ, clr_parse_buff);
			do {				/* Flush until EOL:  */
				*clr_parse_buff_temp = 0;
				if (p_fgets(clr_parse_buff_temp, CLRBUFSZ, colorfd)
					!= clr_parse_buff_temp) endflag++; /* out of data */
 			} while (strlen(clr_parse_buff_temp) >= (CLRBUFSZ-1));
		}
		if ( (clr_parse_buff[0] == '%') || (clr_parse_buff[0] == '!') ||
			 (clr_parse_buff[0] == '#') )
			continue;			/* skip command lines */
		strcpy (clr_parse_buff_temp, clr_parse_buff);
		if ( !strncmp(clr_parse_buff, "AI_Sep", 6) )
			continue;			/* ignore Separation Key Words */
		if ( !strncmp(clr_parse_buff, "CD_Sep", 6) )
			continue;			/* ignore Separation Key Words */
		if ( !strncmp(clr_parse_buff, "G_FG_Color", 10) )
			continue;			/* ignore default graphic color Key Words */
		if ( !strncmp(clr_parse_buff, "G_BG_Color", 10) )
			continue;			/* ignore default graphic color Key Words */
		if ( !strncmp(clr_parse_buff, "G_BW_Overprint", 14) )
			continue;			/* ignore default graphic color Key Words */
		if ( !strncmp(clr_parse_buff, "Plate_Name", 10) )
		{						/* Save Plate Name */
			plate_name_input = atoi((char *)&clr_parse_buff[11]);
			if ( (plate_name_input < 0) || (plate_name_input > 32))
				continue;		/* no number */
			memset((char *)&plate_nm[plate_name_input].name[0],
				   0, sizeof(struct plate_name));
/*
			p=(strstr((char *)&clr_parse_buff[11],"-"));
			p++;
			strncpy((char *)&plate_nm[plate_name_input], p, 30);
*/
			strncpy((char *)&plate_nm[plate_name_input],
					(char *)&clr_parse_buff[11], 30);
			plate_nm[plate_name_input].name[30] = 0;
			if ( (result_ptr = strrchr((char *)&plate_nm[plate_name_input],
									   '\n') ) )
				*result_ptr = 0; /* null out the new line */
			continue;
		}
		msg_parse(clr_parse_buff_temp, length, CLR_KEYWORD_COUNT, clr_msg);
		for (i=0; i<CLR_KEYWORD_COUNT; i++)
		{
		if ( clr_msg[i].answer[0] && (i != 72))
				break;			/* got an answer */
		}

		if (i >= 11)
			continue;			/* no valid message */
		if (i == 0)
		{						/* got a color id */
			if ( (cur_color_id > 0)  && !plate_zero_stored)
			{
				cur_plate = 0;
				store_color(0);	/* store plate zero for prev color */
			}
			plate_zero_stored = 0;
			cur_plate = -1;
			cur_screen = 72;
			cur_pct = 100;
			cur_tint = 100;
			strcpy(cur_shape, "dot");
			cur_angle = 45.;
			cur_reverse = 0;
			cur_cmyk_flag = 0;
			strcpy(cur_op, "false");
			strcpy(cur_color_name,"");
			strcpy(cur_type,"process");
			for (i=0; i<4; i++)
				cur_cmyk[i] = 0.0;
			cur_color_id = atoi(clr_msg[0].answer);
			if (cur_color_id < 0)
			{
				cur_color_id = 0;
				p_info(PI_ELOG, "ERROR - Negative color '%s' is invalid.\n",
					   clr_msg[0].answer);
				continue;
			}
			for (i=7; i<11; i++)
			{					/* look for CMYK values */
				if ( clr_msg[i].answer[0])
				{
					temp = atof(clr_msg[i].answer);
					if ( (temp < 0.0 ) || (temp > 1.0) )
						p_info(PI_ELOG, "ERROR - CMYK value on line '%s' does not lie between 0 and 1.0\n",clr_parse_buff);
					else
					{
#if LPMfloat
						if(LPMK[LPM_ColorMaster])
#else
						if((LPMK & LPM_ColorMaster))
#endif
						{
							cur_cmyk[i-7] = temp;
							cur_cmyk_flag = 1;
						}
					}
				}					
			}					/* end for(i=7;i<11;i++) */
			if ( clr_msg[6].answer[0])
			{					/* got a Tint */
				cur_tint = atoi(clr_msg[6].answer);
				if ( (cur_tint <= 0) || (cur_tint > 100) )
				{
					cur_tint = 100;
					p_info(PI_ELOG, "ERROR - color Tint must be above 0, below 100;\n");
					p_info(PI_ELOG, "     '%s' is not valid. \n",clr_msg[6].answer);
				}		     
			}
			if ( clr_msg[45].answer[0]) /* got Overprint */
			{
				strncpy(cur_op, clr_msg[45].answer, 5);
				*(cur_op+5) = 0;
				if ( strcmp(cur_op, "true") && strcmp(cur_op, "false"))
				{
					p_info(PI_ELOG, "ERROR - color op '%s' must be true or false, will use true.\n", cur_op);
					strcpy(cur_op, "true");
				}
			}
			else
				strcpy(cur_op,"false");

			if ( clr_msg[46].answer[0]) {
				strcpy(cur_type,(clr_msg[46].answer));
			} else
				strcpy(cur_type,"process");

			if ( clr_msg[47].answer[0]) {
				if (strlen(clr_msg[47].answer) > 31) {
					p_info(PI_ELOG, "ERROR - color name '%s' exceeds 31 characters.\n", clr_msg[47].answer);
					continue;
				}
				strcpy (cur_color_name,(clr_msg[47].answer));
			} else
				strcpy(cur_color_name,"");
			continue;
		}						/* end got a color id */
		if ( cur_color_id < 0)
		{
			p_info(PI_ELOG, "ERROR - color id missing before line '%s' \n",clr_parse_buff);
			continue;
		}
		if (i == 1)
		{						/* Plate being defined */
			cur_plate = atoi(clr_msg[1].answer);
			if ( (cur_plate < 0) || (cur_plate >= MAX_CLRS) )
			{
				p_info(PI_ELOG, "ERROR - Color '%d', Plate '%d' less than 0 or greater than 31 \n",cur_color_id,cur_plate);
				p_info(PI_ELOG, "     Line in error reads '%s'\n",clr_parse_buff);
				cur_plate = -1;
				continue;
			}
			continue;
		}						/* end define Plate */
		if ( cur_plate < 0)
		{
			p_info(PI_ELOG, "ERROR - color Plate is missing before line '%s' \n",
				   clr_parse_buff);
			continue;
		}
		screen_found_flag = 0;
		switch (i)
		{
		  case 2:				/* Shape */
			if ( clr_msg[2].answer[0])
				strcpy(cur_shape, clr_msg[2].answer);
			else
			{
				p_info(PI_ELOG, "ERROR - Shape '%s' is not supported.\n",
					   clr_msg[2].answer);
				strcpy(cur_shape, "dot");
			}
				
		  case 3:				/* Angle */
			if ( clr_msg[3].answer[0])
				cur_angle = atof(clr_msg[3].answer); /* got an Angle */
			
		  case 4:				/* Screen */
			if ( clr_msg[4].answer[0])
			{					/* got a Screen */
				cur_screen = atoi(clr_msg[4].answer);
				if (cur_screen < 0)
					cur_screen = 6;
			}
				
		  case 5:				/* Percent */
			if ( clr_msg[5].answer[0])
			{					/* got a Percent */
				cur_pct = atof(clr_msg[5].answer);
				if ( (cur_pct < 0.0) || (cur_pct > 100.) )
				{
					cur_pct = 0.0;
					p_info(PI_ELOG, "ERROR - color Percent must be between 0 and 100\n");
					p_info(PI_ELOG, "     '%s' is not valid. \n",clr_msg[5].answer);
				}		     
			}
			screen_found_flag++; /* set if any case 2 thru 6 found */
			break;

		  default:
			continue;
			
		}					/* end switch (i) */
		if (cur_plate && !plate_zero_stored && screen_found_flag)
		{					/* store Plate 0 first */
			temp_plate = cur_plate;
			cur_plate = 0;
			store_color(0);
			cur_plate = temp_plate;
		}
		if (screen_found_flag)
		{
			store_color(1);
			plate_zero_stored++;
		}
    }							/* end while( !endflag ) */
    p_close(colorfd);
	store_white_and_black();
	cur_cmyk_flag = 0;
	cur_tint = 100;
}								/* end of init_color */

/*************************************************************************/

static void store_color(int reset_flag)
{								/* store color pass, if any */
    if( !cur_screen || !cur_pct)
		cur_reverse = 1;
	else
		cur_reverse = 0;
#ifdef TRACE
    if(color_trace)
		p_info(PI_TRACE, "store clr= %d, plate= %d, fn= %s, ang= %3.2f, scr= %d, pct= %3.3f\n",
			   cur_color_id,cur_plate,cur_shape,cur_angle,cur_screen,cur_pct);
#endif
	add_color();
	if (reset_flag)
	{
		cur_plate = -1;    
		strcpy(cur_shape, "dot");
		cur_angle = 45.;
		cur_screen = 72;
		cur_pct = 100.0;
		cur_reverse = 0;
		strcpy(cur_op, "false");
		strcpy(cur_color_name,"");
		strcpy(cur_type,"process");
	}
}								/* end store color */

/*************************************************************************/

static void store_white_and_black()
{
	int i;
    struct clr_lst *clr;

	strcpy(cur_shape, "dot");
	cur_angle = 45.;
	cur_cmyk_flag = 0;
	cur_plate = 0;
	cur_tint = 100;
	cur_screen = 72;
	strcpy(cur_op, "false");
	strcpy(cur_color_name,"");
	strcpy(cur_type,"process");
	for (i=0; i<4; i++)
		cur_cmyk[i] = 0.0;
	if ( !(clr = find_color(0, 0)) ) /* look for white in plate 0*/
	{
		cur_pct = 0.;
		cur_color_id = 0;
		store_color(0);			/* white not defined, define it */
	}
	if ( !(clr = find_color(1, 0)) ) /* look for black in plate 0 */
	{							/* black not defined, define it */
		cur_pct = 100.;
		cur_color_id = 1;
		store_color(0);
	}
}								/* end function */

/*************************************************************************/
/*********** EOF *********/

