#include <stdio.h>
#include "p_lib.h"
#include "psjob.h"

static void drawvrule2(int16 ruleind);
static void eoln8(void);
static void gtabs(int16 ntabs);
static void tscan(void);
static void vjscan(void);
static void tab_scan_to_split(void);
static void tab_scan_to_cmd_et(void);

int Ktabmode;
int ktabindex;
int holdktabindex;
int instrad;
int16 maxstradlevel;			/* Greatest level of straddles */
int16 sindex;					/* straddle index */
int16 ntabs;             	    /* numb  of tabs/cols */
int16 holdforec2, holdfowrd2;
int16 holdr1, holdw1;
int16 maxslead;					/* Max leading of strads in level */
int16 starty;					/* Top Y-pos of top straddle level */
int16 thisy;					/* Top Y-pos of this straddle lev */
int16 nexty;					/* Ending Y-pos of this stradl lev */
int16 stradcols;				/* # of columns to straddle. */
int16 vdepth, vband, vbandct;	/* Depth from VM; Amounts for VB. */
int16 vlead;					/* LD of current line in VJ area. */
int16 tab_max_y;				/* Deepest Ypos of table line */
int16 tab_min_y;				/* Shallowest Ypos of table line. */
int16 strad_vert_align;		    /* Vert alignment of mult straddls */
int16 strad_vert_space;		    /* Vert alignment of mult straddls */
int psend;

extern int tab_offset;			/* horiz displacement for tabular */
extern int tab_split_index_start;
extern int tab_split_index_end;
extern int flagGlyph;			/* 1= stack() has a glyph ready to print.  */

static int i,j;
static int k1, k2;
static int tab_line_BcEc;
static int16 hold_lead;

struct onetab					/* See entry kmd22 for definitions */
{
    int16 twid, ttype, tx, ty1, ty2;
    int16 tsumlead, tdepth, quad;
} ktab[110];

struct onepwtab					/* See kmd68 define page colums. */
{								/* Driven by dpage for .pw files. */
    int16 twid, ttype, tx, ty1;
    int16 ty2,  tsumlead,  tdepth;
} kpwtab[110];

struct onestrad					/* See entry kmd22 for definitions */
{
    int16 sx, sy1, sy2, sbeg, send;
    int16 salign, slead, level;
} stab[110];

int PSTabGutterRuleColor[100];
int PSTabGutterRuleShade[100];

/***********************************************************************
 **  KMD14()    Define tabular columns **
 ***********************************************************************/
int kmd14()
{
    ntabs = foget() / 2;		/* Number of tabular columns. */
    gtabs(ntabs);
	return(0);
}
/***********************************************************************
 **  KMD15()    Define Stradle. **
 ***********************************************************************/
int kmd15()
{
    stradcols = foget();		/* # of columns to straddle */
    foget();					/* Total measure of straddle head */
    instrad = 1;
    sindex++;
    tab_offset = stab[sindex].sx;
    Ypos = stab[sindex].sy1;
	return(0);
}
/***********************************************************************
 **  KMD22()    Begin tabular mode **
 ***********************************************************************/
int kmd22()
{
	uint32 save_cc_mask;


	tab_line_BcEc = CurrentLineBcEc;
    ntabs = foget();			/* # of width/type pairs */
    for (k1=0; k1<110; k1++)	/* Clear array of existing data. */
    {
		ktab[k1].twid = 0;		/* Width of column or gutter */
		ktab[k1].ttype = 0;		/* Column type: 0=gutter  1=top-
								   aligned text  2=bottom  3=center
								   10+=down rule: rule weight+10 */
		ktab[k1].tx = 0;		/* X-pos of start of column */
		ktab[k1].ty1 = 0;		/* Abs Y-pos of col from page top */
		ktab[k1].ty2 = 0;		/* Ending abs Y-pos from page top */
		ktab[k1].tsumlead  = 0;	/* (unused) */
		ktab[k1].tdepth = 0;	/* Total column depth, if text col */
		ktab[k1].quad = 3;		/* Default quad of V-rul: centered */
		stab[k1].sx = 0;		/* X-pos of start of straddle */
		stab[k1].sy1 = 0;		/* Y-pos of start, from page top */
		stab[k1].sy2 = 0;		/* Y-pos of end, from page top */
		stab[k1].sbeg = 999;	/* ktab[] index of first column
								   that's straddled.*/
		stab[k1].send = -999;	/* ktab[] index, +1, of last column
								   that's straddled. */
		stab[k1].salign = 0;	/* Straddle align: 1=Tp 2=Bt 3=Cen */
		stab[k1].slead = 0;	    /* Total depth of lines in straddl */
		stab[k1].level = 1;		/* Level of straddle from top. */
    }
    sindex = -1;       		    /* Init: No straddles exist yet. */
    maxstradlevel = 1;		    /* In top level of straddles. */
    gtabs(ntabs);      		    /* Input column width/type pairs. */
    tab_offset = 0;
    for (k1=0; k1<ntabs; k1++)	/* Loop to set starting x- and y- */
    {							/* positions of each tab column. */
		ktab[k1].tx  = tab_offset;
		ktab[k1].ty1 = Ypos;
		tab_offset  += ktab[k1].twid;
    }
    tab_offset = 0;
    ktabindex  = 0;
    while((ktab[ktabindex].ttype != 1) &&
		  (ktab[ktabindex].ttype != 2) &&
		  (ktab[ktabindex].ttype != 3))
		ktabindex++;			/* Bump to index of 1st text col */
    tab_offset = ktab[ktabindex].tx;
    holdktabindex = ktabindex;
    Ktabmode  = 1;
    instrad   = 0;
    stradcols = 0;
    tscan();					/* Scan whole tab line thru [ET, building tab
								   depths and vertical alignments. */
	if ( tab_line_BcEc)
	{							/* no output of tab */
		PrevLineBcEc = tab_line_BcEc;
		CurrentLineBcEc = tab_line_BcEc;
		return(0);
	}
	if ( BcEcFlag)
	{
		PrevLineBcEc = CurrentLineBcEc;
		CurrentLineBcEc = tab_line_BcEc;
		if ( PrevLineBcEc && !CurrentLineBcEc)
		{						/* turn on output, force out new parameters */
			BcEcExtraLeadNeeded = BcEcExtraLead;
			FgColor = -2;		/* will force out current color */
			Holdfont = 0;
		}
	}
    tab_max_y = 0;				/* Y-pos of deepest text baseline in table. */
    tab_min_y = 15999;			/* Y-pos of highest text baseline in table. */
    for(i=0;i<ntabs;i++)		/* Loop to set those */
    {
		if((ktab[i].ty1 > 0) && (ktab[i].ty1 < tab_min_y))
			tab_min_y = ktab[i].ty1;
		if (ktab[i].ty2 > tab_max_y)
			tab_max_y = ktab[i].ty2;
		if((stab[i].sy1 > 0) && (stab[i].sy1 < tab_min_y))
			tab_min_y = stab[i].sy1;
		if (stab[i].sy2 > tab_max_y)
			tab_max_y = stab[i].sy2;
    }
    if( (tab_max_y >= Imageheight) && !FileType) /*If tab line will exceed */
    {							/* page depth and in galley, do page-feed. */
		save_cc_mask = cc_mask;
		if (spot_pass == 1)
			cc_mask = 1 & Plates;
		else
			cc_mask = cc_hit;
		if (in_overflow)
			OverflowOrColorFlag = -1;
		else if ( FileType)
		{						/* layout */
			if (spot_pass == 1)
				OverflowOrColorFlag = 1;
			else
				OverflowOrColorFlag = cc_hit;
		}
		do_pskmd('P',"KMD22 begin tab mode.");
		cc_mask = save_cc_mask;
		for (i=0;i<ntabs;i++)
		{
			if (ktab[i].ty1)
				ktab[i].ty1 += Ypos - tab_min_y;
			if (ktab[i].ty2)
				ktab[i].ty2 += Ypos - tab_min_y;
			if (stab[i].sy1)
				stab[i].sy1 += Ypos - tab_min_y;
			if (stab[i].sy2)
				stab[i].sy2 += Ypos - tab_min_y;
		}
    }
#ifdef TRACE
    if (debugger_trace)
    {
		p_info (PI_TRACE, "ntabs=%d  sindex=%d \n", ntabs, sindex);
		p_info (PI_TRACE, "tab_max_y=%d  Imageheight=%d\n", tab_max_y, Imageheight);
		p_info (PI_TRACE, "twid  ttype   tx   ty1   ty2  tdepth\n");
		for (i=0;i<ntabs;i++)
			p_info (PI_TRACE, "%5d %5d %5d %5d %5d %5d\n",
					ktab[i].twid, ktab[i].ttype, ktab[i].tx, ktab[i].ty1,
					ktab[i].ty2, ktab[i].tdepth);
		p_info (PI_TRACE, "\n  sx    sy1  sy2  sbeg  send  saline slead level\n");
		for (i=0;i<=sindex;i++)
			p_info (PI_TRACE, "%5d %5d %5d %5d %5d %5d %5d %5d\n",
					stab[i].sx, stab[i].sy1, stab[i].sy2, stab[i].sbeg,
					stab[i].send, stab[i].salign, stab[i].slead,stab[i].level);
    }
#endif
    ktabindex = holdktabindex;
    /* Set starting Y-pos of 1st line of col 1:   */
    if (sindex >= 0 && stab[0].sbeg == ktabindex)
		Ypos = stab[0].sy1;		/* Y-pos of straddle if
								   it covers col. 1.   */
    else
		Ypos = ktab[ktabindex].ty1;	/* Else text-col pos. */

	if ( tab_split_index_start)
	{							/* scan to start of split */
		tab_scan_to_split();
		return(0);
	}
    sindex = -1;
    nexty = ktab[0].ty2;
	ET_FakeTabLineFlag++;
	return(0);
}

/***********************************************************************
 **  KMD23()    Next tabular column **
 ***********************************************************************/
int kmd23()
{
    if(instrad)
		instrad = 0;
    else
		do
		{
			ktabindex++;
		} while ( (ktabindex <= ntabs) && (ktab[ktabindex].ttype != 1) &&
				  (ktab[ktabindex].ttype != 2) &&(ktab[ktabindex].ttype != 3));
	if ( !tab_split_index_start && tab_split_index_end && 
		 (ktabindex >=tab_split_index_end))
	{							/* skip to cmd et */
		tab_scan_to_cmd_et();
		kmd24();
		return(0);
	}
    tab_offset = ktab[ktabindex].tx;
    Ypos = ktab[ktabindex].ty1;
	if ( ET_InsertEndDelay1)
	{
		ET_InsertStartDelay1 = 0;
		ET_InsertEndDelay1 = 0;
	}
	if ( ET_InsertEndDelay2)
	{
		ET_InsertStartDelay2 = 0;
		ET_InsertEndDelay2 = 0;
	}
	ET_FakeTabLineFlag++;
	return(0);
}

/***********************************************************************
 **  KMD24()  End tabular mode, set leading to maximum depth of table. **
 ***********************************************************************/
int kmd24()
{
#ifdef TRACE
	if(debugger_trace)
		p_info(PI_TRACE, "In kmd24: Ypos=%d, nexty=%d\n", Ypos,nexty);
#endif
    Ypos = nexty;
    tab_offset = 0;
    Ktabmode = 0;
    sindex = -1;
    instrad = 0;
	ET_FakeTabLineFlag++;
	return(0);
}

/***********************************************************************
 **  KMD30()  Vertical Depth.                                          **
 ***********************************************************************/
int kmd30()
{
    vdepth = foget();
    vjscan();
	return(0);
}

/***********************************************************************
 **  KMD31()  Vertical Band.                                           **
 ***********************************************************************/
int kmd31()
{
    i = vband / vbandct;		/* Portion of excess depth in
								   vert measure for this band */
    camy(i,"vertical band");	/* Put out the leading now. */
    vband -= i;					/* For next VB: depth remain, */
    vbandct--;					/*   number of bands left. */
	return(0);
}

/***********************************************************************
 **  KMD32()  Vertical Justify. **
 ***********************************************************************/
int kmd32()
{
    vband = 0;
	return(0);
}

/***********************************************************************
 **  KMD68()    Define page columns. **
 ***********************************************************************/
int kmd68()
{
	int16 pwcols;

    for(i=0;i<110;i++)
    {
		kpwtab[i].twid = 0;
		kpwtab[i].ttype = 0;
		kpwtab[i].tx = 0;
		kpwtab[i].ty1 = 0;
		kpwtab[i].ty2 = 0;
    }
    pwcols = foget() / 2;		/* Number page columns. */
    for(i=0; i<pwcols; i++)
    {
		kpwtab[i].twid = foget(); /* Column width. */
		kpwtab[i].ttype = foget(); /* Column type. */
    }
	return(0);
}
/***********************************************************************
 **  KMD72()   Straddle alignment.  Tells how to vertically align **
 **		the shallower of two adjacent straddle heads against **
 **		the deeper. **
 ***********************************************************************/
int kmd72()
{
    i = foget();				/* There are 1 or 2 args: */
    strad_vert_align = foget();	/* 1=Top  2=Bot  3=Center */
    strad_vert_space = (i==1) ? 2 : foget(); /*1=Put extra space bottom.
											   2=Divide evenly (default)
											   3=Put extra space top. */
	return(0);
}
/***********************************************************************
 **  GTABS(ntabs)   Get Column (Tab) widths and types **
 ***********************************************************************/
static void gtabs(int16 ntabs)
{
    for(i=0; i<ntabs; i++)
    {
		ktab[i].twid  = foget(); /* Column (Tab) width. */
		ktab[i].ttype = foget(); /* Column (Tab) type. */
    }
}
/***********************************************************************
 ** DRAWVRULE2(ruleind)      Draw Vertical rule. **
 ***********************************************************************/
static void drawvrule2(int16 ruleind)
{
    float line_bredth;
	int i;

    line_bredth = (float)ktab[ruleind].ttype - 10.0; /* Get rule weight from user. */
    line_bredth = (line_bredth * HorizontalBase) / Jrule;
    Fx  = ktab[ruleind].tx + 
		fo_line_def.SolMarginAdjust; /* X-pos of rule centr (adjusted below) */
    if( FileType)
		Fx += Xmark;
    i = ktab[ruleind].quad;
    if(i == 2)
		Fx += (ktab[ruleind].twid - line_bredth); /* quad right.*/
    else if (i == 3)
		Fx += ( (ktab[ruleind].twid / 2) - (line_bredth/2)); /* quad center.*/
    Fx2 = Fx + line_bredth;
    Fy = ktab[ruleind].ty1;		/* Y-pos of rule top. */
    Fy3 = ktab[ruleind].ty2;	/* Y-pos of rule bottom. */
    Xpos = Fx;
    Ypos = Fy;
    do_pskmd('L',"drawvrule");
	Ofx = -4000;				/* force Move command output */
    do_pskmd('M',"drawvrule after");
}

/***********************************************************************
 ** SKMD8()  tabular data scan, EOL, get num of chars in next line. **
 ***********************************************************************/
int skmd8()
{
	char err_buf[128];
	int err;
	int16 nchar;
	LDEF fo_line_def;
	
    while ((nchar=foget()) <= 0)
    {
		if (nchar < 0)		    /* error - end of file before [et */
		{
			psend = 1;
			return(0);
		}
		fowrd = 256;		    /* End of this FO rec. Get next */
    }
	if ( (err = read_line_def( &fo_line_def, foget)) )
	{
		sprintf(err_buf, "PP ERROR - Unable to read line start after line %d,  forec %d %d, error %d in skmd8\n",
				last_good_line, forec, fowrd, err);
		stop(err_buf, 0,0);
	}
	if ( Kmd_PP_Started &&
		((fo_line_def.SolMeasure+fo_line_def.SolMarginAdjust) > MaxPP_Meas))
		MaxPP_Meas = fo_line_def.SolMeasure + fo_line_def.SolMarginAdjust;
	DontPaintFlag = fo_line_def.MiscLineFlags & 0x10;
	hold_lead = fo_line_def.BolLeading +  fo_line_def.BolExpandableExtraLead +
		fo_line_def.BolFlexibleExtraLead + fo_line_def.BolRigidExtraLead +
			fo_line_def.BolDroppableExtraLead + 
				fo_line_def.BolFlexibleExtraLeadTop + 
					fo_line_def.BolDroppableExtraLeadTop; /* Accum leading */
#ifdef TRACE
	if(debugger_trace)
		p_info(PI_TRACE, "In skmd8, hold_lead=%d to add to tdepth.\n", hold_lead);
#endif
	if(instrad)
		stab[sindex].slead += hold_lead;
	else
		ktab[ktabindex].tdepth += hold_lead;
	if ( !(fo_line_def.MiscLineFlags & 0x100) )
		tab_line_BcEc = 0;		/* output the entire tab swath */
	return(0);
}								/* end skmds8 */
/***********************************************************************
 ** SKMD13() data scan - extra lead. **
 ***********************************************************************/
int skmd13()
{
    i = foget();
    if (instrad)
		stab[sindex].slead += i;
    else
		ktab[ktabindex].tdepth += i;
	return(0);
}
/***********************************************************************
 ** SKMD14()   tabular data scan - define tabular columns. **
 ***********************************************************************/
int skmd14()
{
    j = foget();				/* skip command & data */
    for(i=1; i<=j/2; i++)
    {
		foget();
		foget();
    }
	return(0);
}
/***********************************************************************
 ** SKMD15()   tabular data scan - define stradle. **
 ***********************************************************************/
int skmd15()
{
	int16 j;

    stradcols = foget();		/* # of columns to straddle */
    foget();					/* Total measure of straddle head */
    sindex++;					/* Move to next straddle slot */
    stab[sindex].sx   = ktab[ktabindex].tx; /* Start X-pos of strad */
    stab[sindex].sy1  = ktab[ktabindex].ty1; /* Start Y-pos of strad */
    stab[sindex].sbeg = ktabindex; /* This head straddles columns */
								/*   beginning at ktabindex, */
    stab[sindex].send = ktabindex + stradcols; /* and ending here. */
    for (i=sindex-1; i>=0; i--)	/* Scan all previous levels for a */
		if (stab[i].send-1 >= ktabindex) /* head which also straddles this */
		{						/* column.  If found, this head */
			j = stab[i].level + 1; /* is 1 level deeper than it. */
			stab[sindex].level = j;
			if (j>maxstradlevel)
				maxstradlevel = j; /* Keep max level. */
			break;
		}
    instrad = 1;			    /* Set in-a-straddle-head flag. */
	return(0);
}

/***********************************************************************
 ** SKMD19()   tabular data scan - top align, overrides [DT alignment. **
 ***********************************************************************/
int skmd19()
{
    if (instrad)
		stab[sindex].salign   = 1;
    else
		ktab[ktabindex].ttype = 1;
	return(0);
}

/***********************************************************************
 ** SKMD20()   tabular data scan - bottom alignment. **
 ***********************************************************************/
int skmd20()
{
    if (instrad)
		stab[sindex].salign   = 2;
    else
		ktab[ktabindex].ttype = 2;
	return(0);
}

/***********************************************************************
 ** SKMD21()   tabular data scan - center alignment. **
 ***********************************************************************/
int skmd21()
{
    if (instrad)
		stab[sindex].salign   = 3;
    else
		ktab[ktabindex].ttype = 3;
	return(0);
}

/***********************************************************************
 ** SKMD22()   tabular data scan - begin tabular. **
 ***********************************************************************/
int skmd22()
{
    j = foget();				/* Skip command and data */
    for (i=1;i<=j;i++)
    {
		foget();
		foget();
    }
	return(0);
}

/***********************************************************************
 ** SKMD23()   tabular data scan - next tabular column **
 ***********************************************************************/
int skmd23()
{
    if (instrad)				/* In straddle, [nt exits it. */
    {
		k2 = stab[sindex].slead; /* Get depth of this straddle */
		for (k1=stab[sindex].sbeg ; k1<stab[sindex].send ; k1++)
		{						/* For all columns beneath it
								   (including rule gutters), */
			ktab[k1].ty1 += k2;	/* move top y-position down
								   by straddle depth. */
		}
		ktab[ktabindex].tsumlead = 0; /* Reset cum. lead(??) */
		instrad = 0;			/* Exit the straddle. */
    }
    else
		do
		{
			ktabindex++;
		} while((ktabindex <= ntabs) && (ktab[ktabindex].ttype != 1) &&
				(ktab[ktabindex].ttype != 2) && (ktab[ktabindex].ttype != 3));
	return(0);
}
/***********************************************************************
 **  SKMD24()  tabular data scan - end tabular mode **
 ***********************************************************************/
int skmd24()
{
    int16 level;
	int rule_color, new_rule_color;
	int rule_shade;
	uint32 plates_hold, BlackPaint_cc_mask_sav;
	uint32 loop_temp;
	uint32 current_color_cc_mask;
	uint32 temp_clr_mask = 0;
	int prev_rule_color = -2;

    ktab[ktabindex].tdepth -= hold_lead; /* Remove lead of [ET FO line (it is
											total table depth) */
/*      If any straddle-heads were keyed, straighten them out.
			All top-level straddles must be same depth; all straddles in the
			level below them must match in depth; etc.   I believe six levels
			are the stated maximum, although there's no limit here.
			Change start Y-positions of all basic tabs to the bottom of the
			lowest straddle above them. */
    if(sindex > -1)    		    /* If any straddles exist: */
    {
		starty = stab[0].sy1;	/* Top Ypos of all straddles. */
		nexty = starty;		    /* Bottom Ypos of prev straddle. */
		level = 1;	       	    /* Starting at straddle level 1, */
		do {		       	    /* loop for each straddle level. */
			thisy = nexty;		/* Top Ypos of this straddle. */
			maxslead = 0;		/* Find max straddle leading among */
			for(k1=0; k1<=sindex; k1++)
			{					/* all straddles at this level. */
				if(stab[k1].level != level)
					continue;
				if(stab[k1].slead > maxslead)
					maxslead = stab[k1].slead; /* Keep greatest leading  */
			}
			nexty += maxslead;	/* Now we know bottom Y-pos for */
								/* this straddle level. */
								/* Perform vertical alignment for */
			for(k1=0; k1<=sindex; k1++)
			{					/* straddles in this level: */
				if(stab[k1].level != level) continue;
				k2 = maxslead-stab[k1].slead; /* Lead diff for shallow col */
				stab[k1].sy1 = thisy; /* Assume top alignment */
				if(stab[k1].salign == 2) /* Bottom alignment */
					stab[k1].sy1 += k2;
				else if(stab[k1].salign == 3) /* Center alignment */
				stab[k1].sy2 = nexty; /* Bottom Y-pos for all this level */
			}
			for(k1=0; k1<=sindex; k1++)	/* Based on straddle bottom level, */
			{
				if(stab[k1].level > level)
					stab[k1].sy1 = nexty; /* set top of all lower strads, */
			}
			for(k1=0; k1<=sindex; k1++)	/* and top of basic tab columns */
			{					/* that fall below these strads. */
				if(stab[k1].level != level)
					continue;
				for(k2=stab[k1].sbeg; k2<stab[k1].send; k2++)
					ktab[k2].ty1 = nexty;
			}
		}  while (++level <= maxstradlevel); /* Loop if further level(s) .*/
    }							/* end if (sindex > -1) */
								/* End of Straddle-head twiddling.
								   Ajust basic tab columns,
								   draw vertical rules between them. */
    nexty = -15999;				/* get greatest tab-column Y-pos: */
    for(k1=0; k1<ntabs; k1++)	/* For each tab column, */
    {
		k2 = ktab[k1].ty1 + ktab[k1].tdepth;/* Y-pos is top Y-pos + leading*/
#ifdef TRACE
		if(debugger_trace)
			p_info(PI_TRACE, "In skmd24, tab %d: ty1=%d + tdepth=%d = nexty=%d\n",
				k1, ktab[k1].ty1, ktab[k1].tdepth, k2);
#endif
		if (k2 > nexty)
			nexty = k2;			/* Remember greatest of these. */
    }
    for(k1=0; k1<ntabs; k1++)	/* Set max Y-pos of table line */
    {							/* into each column. */
		ktab[k1].ty2 = nexty;
								/* Quad of col, in case it's rule: */
		if (k1 == 0)
			i = 1;				/* 1st column: Quad rule left. */
		else if (k1 == ntabs-1)
			i = 2;				/* Last column: Quad rule right. */
		else
			i = 3;				/* All middle cols: Quad rule cntr */
		ktab[k1].quad = i;		/* Store the quad-direction. */
    }
	if ( !DidPage)
		beg_PAGE();
	stack_print();
	Kspace[0] = 0;
	Linesub = -1;
	flagGlyph = 0;
	was_text = 0;
	if (spot_pass == 1)
		cc_mask = 1 & Plates;
	else
		cc_mask = cc_hit;
	if (FlashOn)
	{
		m_fprintf("GS\n");		/* save current color before v-rules */
		for(k1=0; k1<ntabs; k1++) /* Draw all vert rules pieces */
		{
			if(ktab[k1].ttype > 9) /* for each rule-gutter. */
			{
				rule_color = PSTabGutterRuleColor[k1];
				rule_shade = PSTabGutterRuleShade[k1];
				if (rule_color <0)
				{
					rule_color = DefaultFrameFgColor;
					rule_shade = DefaultFrameFgShade;
				}
				temp_clr_mask = 0;
				set_pass_2_color(rule_color, &temp_clr_mask, 1);
				temp_clr_mask &= Plates;
				if(spot_pass == 1)
				{				/* doing the black pass */
					current_color_cc_mask = temp_clr_mask & 1;
					new_rule_color = color_check(rule_color, -1);
					if (new_rule_color < 0)
						current_color_cc_mask = 0;
					else
					{
						new_rule_color = rule_color;
						current_color_cc_mask = 1 & Plates;
					}
					if (KeyOutputType) /* pass 2 only if not composite */
						cc_hit |= (temp_clr_mask & ~1);
				}				/* end if(spot_pass==1) */
				else
				{				/* pass 2 */
					new_rule_color = rule_color;
					current_color_cc_mask = temp_clr_mask & ~1;
				}
				if ( current_color_cc_mask )
				{				/* output the colors for this mask */
					BlackPaint_cc_mask_sav = BlackPaint_cc_mask;
					for (i=0; i<MAX_CLRS-1; i++)
					{			/* output the color and v-rule */
						plates_hold = ( loop_temp = (1 << i) ) &
							current_color_cc_mask;
						if(loop_temp > current_color_cc_mask)
							break;
						if ( !plates_hold)
							continue;
						cc_mask = loop_temp;
						if ( prev_rule_color != rule_color)
						{
							BlackPaint_cc_mask = 0;
							vid_color(new_rule_color, rule_shade,
									  i+1); /* output color */
						}
						drawvrule2(k1);
						PaintOnPage |= BlackPaint_cc_mask;
					}			/* end for(i=0;i<MAX_CLRS-1;i++)  */
					BlackPaint_cc_mask = BlackPaint_cc_mask_sav;
				}				/* done with output of all the colors */
				prev_rule_color = -2;
			}					/* end of this gutter-rule */
		}						/* done with all vertical rules */
		if (spot_pass == 1)
			cc_mask = 1 & Plates;
		else
			cc_mask = cc_hit;
		m_fprintf("GR\n");		/* restore current color,from before v-rules */
	}							/* end if(FlashOn) */
	color_func(fo_line_def.SolFgColor, fo_line_def.SolFgShade);
    for(k1=0; k1<ntabs; k1++)	/* Perform bottom- and center- */
    {							/* alignment of text columns. */
		if((ktab[k1].ttype == 2) || (ktab[k1].ttype == 3))
		{						/* Space left over in column is: */
			k2 = ktab[k1].ty2 - ktab[k1].ty1 /* expanded depth top-to-bottom */
				- ktab[k1].tdepth; /* less leading of the text. */
			if(k2 > 0)			/* If there is any space left, */
			{					/* add to start-Y as follows: */
				if(ktab[k1].ttype == 2)
					ktab[k1].ty1 += k2;	/* Add all for Bottom-aligned. */
				else
					ktab[k1].ty1 += k2 / 2; /* Add half for Center-aligned. */
			}
		}
    }							/*  End for. */
	return(0);
}								/*  End SKMD24. */
/***********************************************************************
 **  SKMD77()  tabular data scan - skip graphic **
 ***********************************************************************/
int skmd77()
{
	int16 ii;
	int16 fo_count = 0;

	ii = foget();
	switch (ii)
	{
	  case 0:					/* old style [mg graphic before 6/94 */
		for (ii=0; ii<7; ii++)	/* get args, last of these is name count */
			fo_count = foget();
		break;
	  case 1:					/* new style [mg after 6/94 */
		fo_count = foget();	/* get name length */
		break;
	  case 2:					/* [gm */
		for (ii=0; ii<17; ii++)
			fo_count = foget();
	}							/* end switch(ii) */
	if ((fo_count > 57) || (fo_count <= 0))
	{
		p_info(PI_ELOG, "Invalid GRAPHIC name length %d (1 to 57 valid).\n", fo_count);
		return(0);
	}
	while(fo_count > 0)
	{
		foget();				/* skip over the name */
		fo_count--;
	}
	return(0);
}

/***********************************************************************
 ** TSCAN()         Called from kmd22() **
 ** This routine scans the fo file following a begin tabular command **
 ** to set tabular text position data and draw vertical rules **
 ***********************************************************************/
/*
  The following array shows how many arguments follow each possible FO command.
  0	means there are no arguments.
  1-89	is the argument count.
  -1	means arg 1 contains number of remaining arguments.
  -2	means arg 2 contains number of remaining arguments.
  -8	means arg 8 contains number of remaining arguments.
  99	is special for command 42 (pagination mark).  Argument 1 is key:
  -12 or -13 mean 26 args follow.
  -21 means 1 arg follows.
  all other values mean no more args.
  98	is special for command 73 (footnote mark).  Argument 1 is key:
  3 or 4 means 27 args follow.
  all other values mean 1 arg follows.
  */
int16 foreads[100] =			/* Allows for up to 100 FO commands.  */
{	  2 ,1, 1, 1, 2, 1, 5, 0, 1, 1,	/*  1-10  Used also by other routines */
      0, 0, 1,-1, 2, 1, 1, 1, 0, 0,	/* 11-20  that skip thru FO:  */
      0,-1, 0, 0, 7, 1, 0, 0, 1, 1,	/* 21-30     vjscan() Vertical Just.*/
      0, 0, 0, 1, 3, 1, 0, 1, 1, 1,	/* 31-40 */
      1,99, 2, 2, 2, 1, 1, 1, 0, 1,	/* 41-50 */
      0, 0, 1, 1, 1, 1, 0, 0, 0, 5,	/* 51-60 */
      2, 1, 1,-2,-1, 1, 0,-1, 5, 5,	/* 61-70 */
      1,-1,98, 7,-1, 2, 0, 5, 2,11,	/* 71-80 */
      5,-1,-7, 4};					/* 81-84 */
int16 foreadsTOP = 84;	/* NOTE: When add new fo cmd above, bump this. */

void tscan(void)
{
    int16 fochar,kcmd,icmd,j,argcount;
    static int16 nscmd = 15;
    static int16 scmd[] = { 1,2,8,13,14,15,19,20,21,22,23,24,47,72,77 };
    int skmd8(),skmd13(),skmd14(),skmd15(),skmd19(),skmd22(),skmd23(),
    skmd24(),skmd20(),skmd21(),kmd1(),kmd2(),kmd72(),kmd73(),skmd77();
    static int (*skmds[])() =
    { kmd1, kmd2, skmd8, skmd13, skmd14, skmd15, skmd19, skmd20, skmd21,
          skmd22, skmd23, skmd24, skmd13, kmd72, skmd77 };

    /* store FO file position of the BT command. */
    holdr1 = forec;				/* fo record */
    holdw1 = fowrd;				/* fo word */
    maxslead = 0;
    psend = 0;
    do							/* GO THRU FO FILE KEYING ON TAB, LEAD CMDS  */
    {
		if((fochar = foget()) >= 0)	/* If it's a character, */
		{						/* throw it away */
			foget();			/* and its width. */
			continue;
		}
		kcmd = -fochar;		    /* It's a command. Get command #, */
		for(icmd=0;icmd<nscmd;icmd++)
		{						/* compare command # against */
			if(kcmd == scmd[icmd])  /* special process list. */
			{
				(*skmds[icmd])(); /* Call special process handler. */
				goto quit;		/* Continue to bottom of do-loop. */
			}
		}
		if(kcmd > foreadsTOP)
			continue;			/* Illegal command#: Loop. */
		argcount = foreads[kcmd-1];	/* For all other commands, get
									   the argument count constant. */
		if (argcount == 0)
			continue;			/* No arguments?  Loop. */
								/* Pos arg count is default value */
		if (argcount < 0)		/* Neg arg count means: */
		{
			for (j=-1; j>argcount; j--) /*  skip (argcount-1) args, then  */
				foget();
			argcount = foget();	/* ARGCOUNTth arg contains # of */
		}						/* remaining args (join default) */
		else if (argcount == 99) /* Special arg list */
		{
			j = foget();
			switch (j)
			{
			  case -12:
			  case -13:
				argcount = 26;
				break;
			  case -21:
				argcount = 1;
				break;
			  default:
				continue;	    /* 1st arg was only one. Loop. */
			}
		}
		else if (argcount == 98) /* Special arg list */
		{
			j = foget();
			argcount = 1;
			if (j==3 || j==4)
				argcount = 27;
		}						/* End of if/else sequence */
		for (j=1; j<=argcount; j++) /* Loop to ignore string of args  */
			foget();
      quit:
		continue;
    } while((fochar!=-24)&&(!psend)); /*  -24 = End Tab */
    if(psend)
		skmd24();				/* restore .fo to previous pos */
    foread(holdr1);				/* Restore fo record and fo word  */
    fowrd = holdw1;
}								/* end tscan */

/***********************************************************************
 **  VJSCAN    To scan the FO file until command -32 (Vert Just) **
 ***********************************************************************/
static int16 end2;

static void vjscan(void)
{
    int16 fochar,kcmd,j,argcount,not_in_tab;
    int16 vcumlead;
/* Constant int16 list "foreads[100]" and "foreadsTOP" are defined externally above tscan. */
    holdforec2 = forec;			/* Store current fo file rec pos */
    holdfowrd2 = fowrd;			/* Store current fo file word pos */
    vcumlead = 0;				/* Init lead of VJ area to 0. */
    vlead = fo_line_def.BolLeading;	/* Leading of [VM line, will be */
								/* added to vcumlead in loop. */
    vband    = 0;
    vbandct  = 0;
    end2     = 0;
    not_in_tab = 1;				/* Start out not in tab mode. */
    
/***********************************************************************
 ** DO -- Proceed thru FO file keying onf VB, VJ and EOL commands.     **
 ***********************************************************************/
    do
    {
		if((fochar = foget()) >= 0)	/* If it's a character, throw it away */
		{
			foget();			/* as well as its width. */
			continue;			/* To bottom of do-loop. */
		}
		if (fochar == -8)		/* End of line: */
		{
			if (not_in_tab)
				vcumlead += vlead; /* Bump VJ depth by line lead */
			eoln8();   		    /* read in start of next line */
		}						/* (saving its leading.) */
		else if (fochar == -22)	/* Begin tab mode. */
			not_in_tab = 0;
		else if (fochar == -24)	/* End tab mode. */
			not_in_tab = 1;
		else if (fochar == -31)	/* Vertical band */
			vbandct++;
		kcmd = -fochar;		    /* It's a command. Get command #, */
		if(kcmd > foreadsTOP)
			continue;			/* Illegal command#: Loop. */
		argcount = foreads[kcmd-1];	/* For all other commands, get
									   the argument count constant. */
		if (argcount == 0) continue; /* No arguments?  Loop. */
								/* Pos arg count is default value */
		if (argcount < 0)	    /* Neg arg count means: */
		{
			for (j=-1; j>argcount; j--) /*  skip (argcount-1) args, then  */
				foget();
			argcount = foget();	/* ARGCOUNTth arg contains # of */
		}						/* remaining args (join default) */
		else if (argcount == 99) /* Special arg list */
		{
			j = foget();
			switch (j)
			{
			  case -12:
			  case -13:
				argcount = 26;
				break;
			  case -21:
				argcount = 1;
				break;
			  default:
				continue;	    /* 1st arg was only one. Loop. */
			}
		}
		else if (argcount == 98)	    /* Special arg list */
		{
			j = foget();
			argcount = 1;
			if (j==3 || j==4)
				argcount = 27;
		}						/* End of if/else sequence */
		for (j=1; j<=argcount; j++)	/* Loop to ignore string of args: */
			foget();		    /* Read & throw away each FO char */
								/* END OF VJSCAN DO-WHILE LOOP: */
    } while ( (fochar != -32) && (!end2) ); /* -32 = Vertical Justify */
    if ( (vcumlead >= vdepth) || (end2) )
    {
		vbandct = 0;
		vband   = 0;
    }
								/* Calculate expansion depth, to  */
								/*  happen at the vertical bands. */
    if (vbandct > 0)
		vband = vdepth - vcumlead;
    foread(holdforec2);			/* Restore fo record and fo word */
    fowrd = holdfowrd2;
} /* end vjscan */

/***********************************************************************
 **  EOLN8()   Called from vjscan, end of line, -8. **
 ***********************************************************************/
static void eoln8(void)
{
	char err_buf[128];
	int err;
	int16 nchar;
	LDEF temp_fo_line_def;

    while ((nchar = foget()) == 0) /* Null ends record, */
		fowrd = 256;		    /* Start new record and retry. */
    if(nchar<0)					/* error - end of file before [et */
    {
		end2= 1;
		return;
    }
	if ( (err = read_line_def( &temp_fo_line_def, foget)) )
	{
		sprintf(err_buf, "PP ERROR - Unable to read line start after line %d,  forec %d %d, error %d in eoln8\n",
				last_good_line, forec, fowrd, err);
		stop(err_buf, 0,0);
	}
	if ( Kmd_PP_Started &&
		((temp_fo_line_def.SolMeasure+temp_fo_line_def.SolMarginAdjust)
		 > MaxPP_Meas) )
		MaxPP_Meas = temp_fo_line_def.SolMeasure +
					 temp_fo_line_def.SolMarginAdjust;
	DontPaintFlag = fo_line_def.MiscLineFlags & 0x10;
	vlead = fo_line_def.BolLeading + 
		fo_line_def.BolExpandableExtraLead +
			fo_line_def.BolFlexibleExtraLead +
				fo_line_def.BolRigidExtraLead +
					fo_line_def.BolDroppableExtraLead + 
						fo_line_def.BolFlexibleExtraLeadTop + 
							fo_line_def.BolDroppableExtraLeadTop;
}								/* end eoln8 */

/************  TAB_SCAN_TO_SPLIT() ************************************/
static void tab_scan_to_split(void)
{								/* skip everything until first col or 
								   gutter after split */
    int16 fochar,kcmd,icmd,j,argcount;
    int16 scmd[] = { 8, 15, 23, 24};
    const int16 nscmd = sizeof (scmd) / sizeof (int16);
    int skmd8_split(), skmd15_split(), skmd23_split(), skmd24_split();

    static int (*skmds[])() = { skmd8_split, skmd15_split, skmd23_split, 
								skmd24_split };

    /* store FO file position of the BT command. */
    psend = 0;
    do							/* GO THRU FO FILE KEYING ON TAB CMDS */
    {
		if((fochar = foget()) >= 0)	/* If it's a character, */
		{						/* throw it away */
			foget();			/* and its width. */
			continue;
		}
		kcmd = -fochar;		    /* It's a command. Get command #, */
		for(icmd=0;icmd<nscmd;icmd++)
		{						/* compare command # against */
			if(kcmd == scmd[icmd])  /* special process list. */
			{
				(*skmds[icmd])(); /* Call special process handler. */
				goto quit;		/* Continue to bottom of do-loop. */
			}
		}
		if(kcmd > foreadsTOP)
			continue;			/* Illegal command#: Loop. */
		argcount = foreads[kcmd-1];	/* For all other commands, get
									   the argument count constant. */
		if (argcount == 0)
			continue;			/* No arguments?  Loop. */
								/* Pos arg count is default value */
		if (argcount < 0)		/* Neg arg count means: */
		{
			for (j=-1; j>argcount; j--) /*  skip (argcount-1) args, then  */
				foget();
			argcount = foget();	/* ARGCOUNTth arg contains # of */
		}						/* remaining args (join default) */
		else if (argcount == 99) /* Special arg list */
		{
			j = foget();
			switch (j)
			{
			  case -12:
			  case -13:
				argcount = 26;
				break;
			  case -21:
				argcount = 1;
				break;
			  default:
				continue;	    /* 1st arg was only one. Loop. */
			}
		}
		else if (argcount == 98) /* Special arg list */
		{
			j = foget();
			argcount = 1;
			if (j==3 || j==4)
				argcount = 27;
		}						/* End of if/else sequence */
		for (j=1; j<=argcount; j++) /* Loop to ignore string of args  */
			foget();
      quit:
		continue;
    } while((fochar!=-24) && ( !psend) && (ktabindex < tab_split_index_start));
}

int skmd8_split()
{								/* end of line */
	char err_buf[128];
	int err;
	int16 nchar;
	LDEF fo_line_def;
	
    while ((nchar=foget()) <= 0)
    {
		if (nchar < 0)		    /* error - end of file before [et */
		{
			psend = 1;
			return(0);
		}
		fowrd = 256;		    /* End of this FO rec. Get next */
    }
	if ( (err = read_line_def( &fo_line_def, foget)) )
	{
		sprintf(err_buf, "PP ERROR - Unable to read line start after line %d,  forec %d %d, error %d in skmd8_split\n",
				last_good_line, forec, fowrd, err);
		stop(err_buf, 0,0);
	}
	return(0);
}

int skmd15_split()
{								/* straddle head */
    stradcols = foget();		/* # of columns to straddle */
    foget();					/* Total measure of straddle head */
    instrad = 1;
	return(0);
}

int skmd23_split()
{								/* next tab */
	int ii;

    if(instrad)
		instrad = 0;
    else
		do
		{
			ktabindex++;
		} while ( (ktabindex <= ntabs) && (ktab[ktabindex].ttype != 1) &&
				  (ktab[ktabindex].ttype != 2) &&(ktab[ktabindex].ttype != 3));
	if (ktabindex < tab_split_index_start)
		return(0);				/* not at the split yet */
    tab_offset = ktab[tab_split_index_start].tx; /* at split, this is start col */
	for (ii=tab_split_index_start; ii <=ntabs; ii++)
		ktab[ii].tx -= tab_offset; /* adjust offsets above split */
    tab_offset = ktab[ktabindex].tx; /* at split, this is starting column */
    Ypos = ktab[ktabindex].ty1;
	if ( ET_InsertEndDelay1)
	{
		ET_InsertStartDelay1 = 0;
		ET_InsertEndDelay1 = 0;
	}
	if ( ET_InsertEndDelay2)
	{
		ET_InsertStartDelay2 = 0;
		ET_InsertEndDelay2 = 0;
	}
	ET_FakeTabLineFlag++;
	return(0);
}

int skmd24_split()
{								/* end tab */
	psend = 1;
	return(0);
}

/************  TAB_SCAN_TO_CMD_ET() ************************************/
static void tab_scan_to_cmd_et(void)
{								/* skip to command et */
    int16 fochar,kcmd,icmd,j,argcount;
    int16 scmd[] = { 8, 15, 23, 24};
    const int16 nscmd = sizeof (scmd) / sizeof (int16);
    int skmd8_et(), skmd15_et(), skmd23_et(), skmd24_et();

    static int (*skmds[])() = { skmd8_et, skmd15_et, skmd23_et, 
								skmd24_et };

    /* store FO file position of the BT command. */
    psend = 0;
    do							/* GO THRU FO FILE KEYING ON TAB CMDS */
    {
		if((fochar = foget()) >= 0)	/* If it's a character, */
		{						/* throw it away */
			foget();			/* and its width. */
			continue;
		}
		kcmd = -fochar;		    /* It's a command. Get command #, */
		for(icmd=0;icmd<nscmd;icmd++)
		{						/* compare command # against */
			if(kcmd == scmd[icmd])  /* special process list. */
			{
				(*skmds[icmd])(); /* Call special process handler. */
				goto quit;		/* Continue to bottom of do-loop. */
			}
		}
		if(kcmd > foreadsTOP)
			continue;			/* Illegal command#: Loop. */
		argcount = foreads[kcmd-1];	/* For all other commands, get
									   the argument count constant. */
		if (argcount == 0)
			continue;			/* No arguments?  Loop. */
								/* Pos arg count is default value */
		if (argcount < 0)		/* Neg arg count means: */
		{
			for (j=-1; j>argcount; j--) /*  skip (argcount-1) args, then  */
				foget();
			argcount = foget();	/* ARGCOUNTth arg contains # of */
		}						/* remaining args (join default) */
		else if (argcount == 99) /* Special arg list */
		{
			j = foget();
			switch (j)
			{
			  case -12:
			  case -13:
				argcount = 26;
				break;
			  case -21:
				argcount = 1;
				break;
			  default:
				continue;	    /* 1st arg was only one. Loop. */
			}
		}
		else if (argcount == 98) /* Special arg list */
		{
			j = foget();
			argcount = 1;
			if (j==3 || j==4)
				argcount = 27;
		}						/* End of if/else sequence */
		for (j=1; j<=argcount; j++) /* Loop to ignore string of args  */
			foget();
      quit:
		continue;
    } while((fochar!=-24) && !psend);
    tab_offset = 0;
}

int skmd8_et()
{								/* end of line */
	char err_buf[128];
	int err;
	int16 nchar;
	LDEF fo_line_def;
	
    while ((nchar=foget()) <= 0)
    {
		if (nchar < 0)		    /* error - end of file before [et */
		{
			psend = 1;
			return(0);
		}
		fowrd = 256;		    /* End of this FO rec. Get next */
    }
	if ( (err = read_line_def( &fo_line_def, foget)) )
	{
		sprintf(err_buf, "PP ERROR - Unable to read line start after line %d,  forec %d %d, error %d in skmd8_et\n",
				last_good_line, forec, fowrd, err);
		stop(err_buf, 0,0);
	}
	return(0);
}

int skmd15_et()
{								/* straddle head */
    stradcols = foget();		/* # of columns to straddle */
    foget();					/* Total measure of straddle head */
    instrad = 0;				/* not in straddle at [et */
	return(0);
}

int skmd23_et()
{								/* next tab */
    if(instrad)
		instrad = 0;
    else
		do
		{
			ktabindex++;
		} while ( (ktabindex <= ntabs) && (ktab[ktabindex].ttype != 1) &&
				  (ktab[ktabindex].ttype != 2) &&(ktab[ktabindex].ttype != 3));
    Ypos = ktab[ktabindex].ty1;
	if ( ET_InsertEndDelay1)
	{
		ET_InsertStartDelay1 = 0;
		ET_InsertEndDelay1 = 0;
	}
	if ( ET_InsertEndDelay2)
	{
		ET_InsertStartDelay2 = 0;
		ET_InsertEndDelay2 = 0;
	}
	return(0);
}

int skmd24_et()
{								/* end tab */
	psend = 1;
	return(0);
}

/************  (EOF)  *************************************************/
