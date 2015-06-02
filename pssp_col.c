/************************************************************************
 **   pssp_col.c  routines needed to process spot colors		.     	*
 ************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include "p_lib.h"
#include "psjob.h"

char *m_fprintf_buff;
int m_fprintf_size;
extern int EpsFlag;

static void m_unlink(char *fnm);

Pfd psfa[MAX_CLRS];				/* all the ps file pointers */
char tstbuf [512];			/* buffer to read prologs lines into */

/*************************************************************************
 ** Some general for() loop notes **
 **	index 0 is white, index 1 is black  **
 **	everybody else - loop for it. limit loop to MAX_CLRS, currently	**
 **			 MAX_CLRS is 32 **
 *************************************************************************/
/*************************************************************************
 **  m_fprintf	Does a series of fprintf's to the appropriate streams	**
 **		based on the variable argument list.
 *************************************************************************/
void m_fprintf(char* form, ...)
{
    va_list arg;
	uint32 temp_index, print_cc_mask;
    int i;						/* index to psfa table */

	if ( !MasterOutputSwitch || CurrentLineBcEc)
		return;
	print_cc_mask = (cc_mask | active_trap_cc_mask);
    va_start(arg, form);
	if ( !print_cc_mask )
		return;
    if( (spot_pass == 1) && psfa[0] ) /* black? */
    {
		vsprintf(m_fprintf_buff, form, arg);
		p_fputs((char *)m_fprintf_buff,psfa[0]);
		va_end(arg);
		return;
    }
    for(i=0; i< MAX_CLRS; i++)	/* look at all colors */
    {
		if((print_cc_mask & (temp_index = (1 << i))) &&
		   psfa[i])				/* avail and open?*/
		{
			vsprintf(m_fprintf_buff, form, arg);
			p_fputs((char *)m_fprintf_buff, psfa[i]);
			va_end(arg);
			va_start(arg, form);		/* rewind the list */
		}
		if (temp_index >= print_cc_mask)
			break;
    }
    va_end(arg);				/* ok, we are all done 	*/
}

/*************************************************************************
 **  m_putc	Does a series of putc's to the appropriate streams **
 **		based on the variable argument list.
 *************************************************************************/
void m_putc(unsigned char bite) 
{
    int i;						/* index to psfa table */
	uint32 temp_index, print_cc_mask;
    
	if ( !(print_cc_mask = (cc_mask | active_trap_cc_mask)) ||
		 !MasterOutputSwitch || CurrentLineBcEc)
		return;
    if( (spot_pass == 1) && psfa[0]) /* black? */
    {
		p_putc((int)bite,psfa[0]);
		return;
    }
    for(i=0; i < MAX_CLRS; i++) /* look at them all */
    {
		if((print_cc_mask & (temp_index = (1 << i))) &&
		   psfa[i])				/* avail and open?*/
			p_putc((int)bite,psfa[i]);
		if (temp_index >= print_cc_mask)
			break;
    }
}
/*************************************************************************
 **  m_fopen	Opens postscript files, stuffs its Pfd into psfa[]	**
 **		returns only map of ok's to hide Pfd and save us from  **
 **		being too smart. called with mask , of course **
 **		gives a few error messages **
 *************************************************************************/
int m_fopen(char *fnm, char * mde, int psfm)
/* char *fnm;						base path & name of file	*/
/* xchar *mde;						mode to open in */
/* int psfm;						map of ones to open */
{
    char nbuf[128];				/* name buffer */
    int i;						/* loop counter */
	int temp_index;
    int retmask = 0;			/* mask of good openings */
    

    if(psfm == 0)
		return(0);
	if ( !KeyOutputType)
	{
		if (psfa[0])
			return(1);			/* already open for composite */
		psfm = 1;
	}
    for(i=0; i < MAX_CLRS; i++) /* look at them all even black	*/
    {
		if(psfm & (temp_index = (1 << i)) )	/* of interest? */
		{
			if(i == 0)
				sprintf(nbuf,"%s",fnm);	/* build name */
			else
				sprintf(nbuf,"%s.c%02d",fnm, i+1); /* build name */
			if(psfa[i])			/* open already	*/
			{
				if ( !strcmp(mde,"a+") )
				{
					retmask |= (1 << i); /* ok to be open if a+ */
#ifdef TRACE
					if (debugger_trace)
						p_info(PI_TRACE, "m_fopen extended .TP file = %s\n", nbuf);
#endif
				}
				else
					p_info(PI_ELOG, "Output file:%s already open\n", nbuf);
			}
			else
			{
			   if (!EpsFlag) {
				if((psfa[i] = p_open(TreeName, OUTPUT_FILE, 
								((BookPrint==1) ? ParentDirName : SubDirName),
									nbuf, mde)))  /* opens OK */
				{
					retmask |= (1 << i);
#ifdef TRACE
					if (debugger_trace)
						p_info(PI_TRACE, "m_fopen .TP file = %s\n", nbuf);
#endif
				}
				else
					p_info(PI_ELOG, ":Unable to open output file %s mode%s\n",nbuf,mde);
			   } else {
				if((psfa[i] = p_open(TreeName, EPS_FILE, SubDirName,
									nbuf, mde)))  /* opens OK */
				{
					retmask |= (1 << i);
#ifdef TRACE
					if (debugger_trace)
						p_info(PI_TRACE, "m_fopen .TP file = %s\n", nbuf);
#endif
				}
				else
					p_info(PI_ELOG, ":Unable to open output file %s mode%s\n",nbuf,mde);

			   }
			}
		}
		if (temp_index >= psfm)
			break;
    }
    return(retmask);
}

/*************************************************************************
 **  m_close	closes a postscript files, removes Pfd from psfa[]	**
 **		helps to hide Pfd and save us from **
 **		being too smart. called with mask , of course **
 **		-1 is a special case for 'close them all. **
 *************************************************************************/
void m_close(int psfm)
{
    int i;
	
/* no special handling for -1: it has all bits set already */
    for(i=0; i< MAX_CLRS; i++)
    {
		if( (psfm & (1<<i)) && (psfa[i] != 0) )
		{
			p_fflush(psfa[i]);
			p_close(psfa[i]);
			psfa[i] = 0;
		}
    }
}

/*************************************************************************
 **  m_clear	cleans the psfa array, clears cc_mask & cc_hit **
 *************************************************************************/
void m_clear(void)
{
    int i;
    for(i=0; i< MAX_CLRS; i++)
		psfa[i]=0;
    cc_hit = cc_mask = 0;
}

/*************************************************************************
 **  m_tran_ov	transfers overflow files to regular line		**
 *************************************************************************/
void m_tran_ov(char *of)
{
    Pfd overfd;					/* overflow file pointer */
    int nextchar;				/* character to transfer */
    char overflow_path[128];	/* buffer of overflow path names */

	if ( !psfa[0])
			return;				/* nothing open */
	sprintf(overflow_path, "%s", of);
	if (!EpsFlag)
	overfd = p_open(TreeName, OUTPUT_FILE, 
					((BookPrint==1) ? ParentDirName : SubDirName),
					overflow_path, "r+"); /* 2ndary channel,    */
	else
	overfd = p_open(TreeName, EPS_FILE, SubDirName,
					overflow_path, "r+"); /* 2ndary channel,    */
	if(overfd)
	{
		while (1)				/* Transfer entire overflow-text */
		{						/*   workfile into TP file:	*/
			nextchar = p_getc(overfd); /* Get char as int,	*/
			if (nextchar == EOF)
				break;			/* exit on EOF,	*/
			p_putc(nextchar,psfa[0]); /* put to TP file. */
		}
		OverflowOrColorFlag = -1;
		DidPage = 1;
		cc_mask = 1 & Plates;
		do_pskmd('P',".page overflow."); /* gen showpage sequence.*/
		p_close(overfd);
		if (!EpsFlag)
		{
		if(p_unlink(TreeName, OUTPUT_FILE, 
					((BookPrint==1) ? ParentDirName : SubDirName),
					overflow_path))
			p_info(PI_ELOG, "Can't delete scratch file:%s\n",overflow_path);
		}
		else
		{
		if(p_unlink(TreeName, EPS_FILE, SubDirName, overflow_path))
			p_info(PI_ELOG, "Can't delete scratch file:%s\n",overflow_path);
		}
	}
	else
		p_info(PI_ELOG, "error re-opening overflow file:%s\n", overflow_path);
}

/*************************************************************************
 **  spot_concat - concatenate color files into the 'black' file **
 *************************************************************************/
void spot_concat(void)
{
    int i;
    Pfd outf;					/* shorthand for output file */
    int c;						/* character buffer */
    
	if ( !KeyOutputType)
		return;
	cc_hit &= (Plates & ~1);
    m_fopen(tpname, "r", cc_hit); /* open all colors except black */
    m_fopen(tpname, "r+", 1);	/* open black */
    outf = psfa[0];				/* remember black channel #	*/
    p_seek(outf,-1,2);			/* point to last character */
    if(p_getc(outf) == 4)		/* if it is 004 */
		p_seek(outf,-1,2);		/* point to it */
	else
		p_seek(outf,0,2);		/* point to eof */
    for(i=1; i < MAX_CLRS; i++) /* look at all of the possible colors */
    {							/*  except black or closed ones */
		if(psfa[i])
		{
			while((c = p_getc(psfa[i])) != EOF)
			{
				if(c != 4)		/* don't pass the PS EOF char */
					p_putc(c, outf);
				else
					break;	/* in fact, stop then */
			}
		}
    }
    m_close(-1);				/* close them all */
    m_unlink(tpname);			/* delete the extra files */
    m_fopen(tpname, "r+", 1);	/* open the black */
    outf = psfa[0];				/* remember black channel #	*/
    p_seek(outf,-1,2);			/* point to last character */
    if(p_getc(outf) == 4)		/* if it is 004 */
		p_seek(outf,-1,2);		/* point to it */
	else
		p_seek(outf,0,2);		/* point to eof */
}
/*************************************************************************
 **  m_unlink	Deletes postscript files, returning only map of ok's **
 *************************************************************************/
static void m_unlink(char *fnm)
{
    char nbuf[128];				/* name buffer */
    int i;						/* loop counter */
	int temp_index;
	int psfm;					/* map of ones to unlink */
    
	psfm = cc_hit & ~1; 
    if(psfm == 0)
		return;
	if ( !KeyOutputType)
	{
		if ( !psfa[0])
			return;				/* already done for composite */
		psfm = 1;
	}
    for(i=0; i < MAX_CLRS; i++) /* look at them all even black	*/
    {
		if(psfm & (temp_index = (1 << i)) )	/* of interest? */
		{
			if(i == 0)
				sprintf(nbuf,"%s",fnm);	/* build name	*/
			else
				sprintf(nbuf,"%s.c%02d",fnm, i+1); /* build name */
			if (!EpsFlag) {
				if(p_unlink(TreeName, OUTPUT_FILE, 
					((BookPrint==1) ? ParentDirName : SubDirName),
					nbuf))
			{
				if ( KeyOutputType)
					p_info(PI_ELOG, "Can't delete scratch file:%s\n",nbuf);
			}
			} else {
			if(p_unlink(TreeName, EPS_FILE, SubDirName, nbuf))
			{
				if ( KeyOutputType)
					p_info(PI_ELOG, "Can't delete scratch file:%s\n",nbuf);
			}
			}
		}
		if (temp_index >= psfm)
			break;
    }
}

/*********** EOF *********/
