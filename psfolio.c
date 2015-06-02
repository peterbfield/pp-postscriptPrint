#include <stdio.h>
#include "p_lib.h"
#include "psjob.h"

static void cnum(int num, int type);
static void fhnum(int16 level, int16 isep, int number, int16 igt,
				  int zero_type);
static int power(int16 x, int16 n);
static void sfhnum(void);

static int16 idata[]= {0,1,3,7,050,010,011,013,017,060,020,021,023,027};
static int16 iray[] = {0,9,22,24,12,3,4,13,41,54,56,44,35,36,45};
static int16 ifont[27];
static int block_number[3];
static int16 numeral_type;
static int16 sep1, sep2, sepblock;
static int16 fwidth, fadjust;


/* ----------KFOLIO(ival)   Called by kmd42 (pagination). ---------- */

void kfolio(int ival)
{
    int16 igt, level, last_level, isep;
	int ii;
    int number, jj, ztype;

    fwidth = 0;
    igt = -ival;
    fadjust = 0;
    switch(ival)
    {
      case -1:					/* Regular Arabic Numbers. */
      case -2:					/* Shift   Arabic Numbers. */
      case -3:					/* Super Shift Arabic Numbers. */
      case_1:					/* output current folio */
		sfhnum();
		ztype = page_number[0] & 1;
		last_level = 0;
		fhnum(1, 0, page_number[0], igt, ztype);
		if(page_number[1])
		{
			last_level = 1;
			fhnum(2, sep1, page_number[1], igt, ztype);
			if(page_number[2])
			{
				last_level = 2;
				fhnum(3, sep2, page_number[2], igt, ztype);
			}
		}
		ztype = block_number[0] & 1;
		isep = sepblock;
		level = last_level + 1;
		for(ii=0;ii<last_level;ii++)
			if(page_number[ii]!=block_number[ii])
				level = 1;
		while(1)
		{
			number = block_number[level-1];
			if(number==0)
				break;
			fhnum(level, isep, number, igt, ztype);
			if(level==1)
				isep = sep1;
			else if (level==2)
				isep = sep2;
			else
				break;
			break;
			level++;
		}
		fhnum(0, 0, 0, 0, 0);
		break;
		
      case -4:					/* Lower Case Roman Numerals. */
      case -5:					/* Upper Case Roman Numerals. */
      case_4:
		sfhnum();
		fhnum(0, 0, page_number[0], igt, 0);
		break;
		
      case -7:					/* Now in an equation */
      case -8:					/* Exiting equation. */
		break;
		
      case -10:					/* begin non-expandable area */
		break;
		
      case -11:					/* end non-expandable area */
		break;
		
      case -12:					/* read in character widths */
      case -13:
		for(ii=1;ii<=26;ii++)
			ifont[ii] = foget();
		
      case -9:					/* adjust folio <fh9>,<fh12> and <fh13>. */
		fadjust = 1;
		igt = igt - 8;
		if((igt-4)<0)
			goto case_1;
		goto case_4;
		
      case -15:					/* adjustment for issue number  */
		fadjust = 1;
      case -14:					/* Punch issue number */
		sfhnum();
		fhnum(1, 0, 0, igt, 1);
		break;
		
      case -16:					/* Init chapter page # to 1. */
      case -21:					/* Init chapter page # to */
		sfhnum();				/* user's value. */
		jj = ival+16 ? foget() : 1;
		ChapterPage = jj - page_number[0];
		ChapterPageSetFlag++;	/* Flag: Sec folio is now in offset form.  */
		break;					/* Save either as offset from
								   current page number. */
		
      case -17:					/* Adjust chapter page num. */
		fadjust = 1;
      case -18:					/* Put out chapter page num. */
		sfhnum();				/* Retrieve current pg#. */
		fhnum (1, 0, page_number[0]+ChapterPage, igt, 0);
		break;
		
      case -6:					/* UNSUPPORTED:  Incremental footnote mark  */
      case -19:					/* Text-related pg# adjust */
      case -20:					/* Text-related page number */
		break;
		
      case 32:					/* ??? */
		foget();
		break;
    }
} /* end kfolio */

/* ---------- FHNUM(level,isep,number,igt,zero_type) punches
				out a number according to the required style ---------- */
static void fhnum(int16 level, int16 isep, int number, int16 igt,
				  int zero_type)
{
    int16 ntype,twd;
    int jj;
	int16 ii = 0;

    if(igt<=0)					/* [fh number */
    {
        if(fadjust)
			goto adjust;
        return;
    }
    ntype = 9-igt;				/* 9 - -ival */
    if(igt<14)
    {
        if(level>0)
		{
            jj = power(010, level-1);
            ntype = (numeral_type & (ii*7))/jj;
            if(isep)			/* punch separator out */
			{
                twd = fo_line_def.SolSetSize/3;
                if(!fadjust)
				{
                    khar_wid = twd;
                    ackhar = isep;
					stack(((int16)PsfCode[PsfCodeIndex].code[isep]) & 0xff);
				}
                fwidth += twd;
			}
		}
    }
    switch (ntype)
    {
      case  1:					/*Normal numeric folio*/
      case -5:					/*Issue number */
      case -9:					/*Chapter folio */
		if(level-1)
			number -= zero_type;
		if(!fadjust)
			cnum(number, zero_type);
		
      case -6:					/* adjust for width of number punched out */
      case -8:					/* Adjust chapter folio*/
		for(ii=0;ii<=3;ii++)
		{
			jj = 1 - zero_type;
			if(ii)
				jj = power(10, ii);
			if(number>=jj) 
				fwidth += fo_line_def.SolSetSize/2;
		}
		break;
		
      case 2:
      case 3:					/* subsidiary letter */
		if(number < 0)
			break;
		if(number > 26)
			number = 26;
		number += (3-ntype)*32;
		twd = fo_line_def.SolSetSize;
		fwidth += twd;
		if(!fadjust)
		{
			khar_wid = twd;
			ackhar = number;
			stack(((int16)PsfCode[PsfCodeIndex].code[number]) & 0xff);
		}
		break;
		
      case 4:
      case 5:					/* roman numerals */
		{
            int16 shift,iix,kx,lx,mx,nx,jx,jjx,itest;
            shift = 6 - ntype;	/* Uppeer  caps? */
            ii = shift * 7;
            iix = 7;
            kx = 100;
            lx = number;
            for(mx=1;mx<=3;mx++)
			{
                nx = lx/kx;
                lx = lx - nx*kx;
                kx = kx/10;
                ii  -= 2;
                iix -= 2;
                itest = 040;
                for(jx=0;jx<=5;jx++)
				{
					if(idata[nx] & itest)
					{
						jjx=0;
						if((jx<=2) && (jx>0))
							jjx=3-jx;
						twd = ifont[iray[iix+jjx]];
						if(!fadjust)
						{
							khar_wid = twd;
							ackhar = (iray[ii+jjx]);
							stack(((int16)PsfCode[PsfCodeIndex].code[iray[ii+jjx]]) & 0xff);
						}
						fwidth += twd;
					}
					itest /= 2;
				}
			}
		}
    }
    
    if((igt>3)&&(fadjust))
    {
      adjust:
		switch(fo_line_def.Quad & 7)
		{
		  case 1:
		  case 4:
		  case 5:
		  case 6:
		  case 7:
			fo_line_def.RightIndent -= fwidth;
			break;
			
		  case 3:
			fwidth /= 2;
			fo_line_def.RightIndent -= fwidth;
			fo_line_def.LeftIndent -= fwidth;
			Xpos  -= fwidth;
			if (cc_mask || active_trap_cc_mask)
				Kspace[Linesub+1] -= ((float) fwidth / (float)HorizontalBase);
			break;
			
		  case 2:
			fo_line_def.LeftIndent -= fwidth;
			Xpos -= fwidth;
			if (cc_mask || active_trap_cc_mask)
				Kspace[Linesub+1] -= ((float) fwidth / (float)HorizontalBase);
			break;
		}
    }							/* if */
}

/* ---------- POWER(x,n) returns a power of a number ---------- */
static int power(int16 x, int16 n)
{
    int16 cnt=0;
    int res=1;
    for(cnt=1;cnt<=n;cnt++)
		res *= x;
    return(res);
}

/* ---------- CNUM(num,type) punches a number ---------- */
void cnum(int num, int type)
{
    int16 ii;
    int16 num1,ndigits,twd;
    int16 temp[10];
    twd     = fo_line_def.SolSetSize/2;
    num1    = num;
    ndigits = 0;
    do{
		ndigits++;
    } while ((num1/=10));
    num1 = num;
    for(ii=ndigits;ii>0;ii--,num/=10)
		temp[ii] = (num % 10) + 65;
    if((num1 == 0) && (type == 0))
		ndigits--;
    for(ii=1;ii<=ndigits;ii++)
    {
		khar_wid = twd;
		ackhar   = temp[ii];
		stack(((int16)PsfCode[PsfCodeIndex].code[temp[ii]]) & 0xff);
    }
}

/* ---------- SFHNUM() ---------- */
static void sfhnum(void)
{
	int i;
    
    switch(FileType)
    {
      case 0:					/* Processing galley */
        page_number[0] = 1;
        page_number[1] = page_number[2] = 0;
        break;
		
      case 1:					/* Processing layout */
		page_number[0] = PageNo1;
		page_number[1] = PageNo2;
		page_number[2] = PageNo3;
    }
    
    for(i=0;i<3;i++)
		block_number[i] = 0;
    sep1 = sep2 = sepblock = 27;
    numeral_type = ((1*8 + 1)*8 +1);
}

/* ----------EOF ----------*/
