#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "p_lib.h"
#include "psjob.h"
#include "msg_parse.h"
#include "traces.h"
#include "lpm.h"
#include "list_control.h"

extern int read (int fildes, char *buffer, unsigned nbyte);
extern int write (int fildes, char *buffer, unsigned nbyte);

static int job_setup(void);
static void psmain(void);
static void set_traces(char *string);
static void get_prt_name(void);		/* Bug 367p Function to retrieve printer name from .psh file	*/


extern Pfd wdfd;
extern int y_max;
extern int frame_fg_attribute;	/* 0 = opaque, 1 = clear */
extern int was_overflow;
extern int delay_records_flag;
extern int16 underscore_min;
extern int PageW_key_flag;		/* if non-zero, keyword PageW in input */
extern int PageD_key_flag;		/* if non-zero, keyword PageD in input */
extern int Pofft_key_flag;		/* if non-zero, keyword Pofft in input */
extern int Poffb_key_flag;		/* if non-zero, keyword Poffb in input */
extern int Poffl_key_flag;		/* if non-zero, keyword Poffl in input */
extern int Poffr_key_flag;		/* if non-zero, keyword Poffr in input */
extern int galley_count;
extern uint16 last_lnum;
extern int16 repeatno;
extern int psend;
extern int overflow_page_buff[256];
extern struct kmd80_stuff bf_kmd80[40];
extern int StrikeThruRuleStartFlag;

extern int HdrPNamPrt; /*** Bug 367p customer selectable header print file print  ***/
extern char GlyphName[];

char PrtNam[MAX_NAME+4];	/* Bug 367p actual printer name for hdr **/
char shell_msg[1028];

struct msg_parse msg[] = 
{
    {"PipeIn", ""},				/*  0  */
    {"PipeOut", ""},			/*  1  */
    {"Galley", ""},				/*  2  */
    {"Unit", ""},				/*  3  */
    {"Dir", ""},				/*  4  */
    {"Queue", ""},				/*  5  */
    {"Tree", ""},				/*  6  */
    {"Traces", ""},				/*  7  */
    {"FirstPage", ""},			/*  8  */
    {"LastPage", ""},			/*  9  */
    {"LineNbrs", ""},			/* 10  */
    {"ColorTable", ""},			/* 11  */
    {"Slug",""},				/* 12  */
 	{"Psf",""},					/* 13  */
	{"Orient",""},				/* 14  */
	{"Scale",""},				/* 15  */
	{"Scalex",""},				/* 16  */
	{"Scaley",""},				/* 17  */
 	{"fp",""},					/* 18  */
 	{"lp",""},					/* 19  */
 	{"Output",""},				/* 20  */
	{"FirstMaster",""},			/* 21  */
	{"LastMaster",""},			/* 22  */
	{"Header",""},				/* 23  */
 	{"Multi",""},				/* 24  */
 	{"PageW",""},				/* 25  */
 	{"PageD",""},				/* 26  */
 	{"Pofft",""},				/* 27  */
 	{"Poffb",""},				/* 28  */
 	{"Poffl",""},				/* 29  */
 	{"Poffr",""},				/* 30  */
 	{"Cmyk",""},				/* 31  */
 	{"Hdrup",""},				/* 32  */
 	{"Layout",""},				/* 33  */
 	{"LYP",""},					/* 34  */
 	{"Report",""},				/* 35  */
 	{"Register",""},			/* 36  */
 	{"TrimMarks",""},			/* 37  */
 	{"Register_Wt",""},			/* 38  */
 	{"Register_Length",""},		/* 39  */
 	{"Trim_Wt",""},				/* 40  */
 	{"Trim_Length",""},			/* 41  */
	{"FirstGal",""},			/* 42  */
	{"LastGal",""},				/* 43  */
	{"src",""},					/* 44  */
	{"Proof",""},				/* 45  */
	{"Plates",""},				/* 46  */
	{"Blank",""},				/* 47  */
	{"Neg",""},					/* 48  */
	{"Mirror",""},				/* 49  */
	{"BcEc",""},				/* 50  */
	{"EditTrace",""},			/* 51  */
	{"CurInsOffset",""},		/* 52  */
	{"PreInsOffset",""},		/* 53  */
 	{"CurDelOffset",""},		/* 54  */
 	{"PreDelOffset",""},		/* 55  */
 	{"CurInsClr",""},			/* 56  */
 	{"PreInsClr",""},			/* 57  */
 	{"CurDelClr",""},			/* 58  */
 	{"PreDelClr",""},			/* 59  */
 	{"CurInsWeight",""},		/* 60  */
 	{"PreInsWeight",""},		/* 61  */
 	{"Folio2",""},				/* 62  */
	{"IncludeScreens",""},		/* 63  */
	{"LPI",""},					/* 64  */
	{"Copies",""},				/* 65  */
	{"ClipPath",""},			/* 66  */
	{"Collate",""},				/* 67  */
	{"nolock",""},				/* 68  */
	{"PageWInch",""},			/* 69  */
	{"PageDInch",""},			/* 70  */
	{"DoExtractBreaks",""},		/* 71  */
	{"Trim_Reg_Gap",""},			/* 72  */
	{"Eps",""},			/* 73  */
	{"Uname",""},			/* 74  */
	{"DLF",""},			/* 75  */
	{"Resolve",""},			/* 76  */
	{"PdfLkType",""},		/* 77 */
	{"PdfLkWeight",""},		/* 78 */
	{"PdfLkColor",""},		/* 79 */
	{"PdfLkDashlen",""},		/* 80 */
	{"PdfLkDashgap",""},		/* 81 */
	{"PdfLkRadius",""},		/* 82 */
	{"PdfTitle",""},		/* 83 */
	{"PdfAuthor",""},		/* 84 */
	{"PdfSubject",""},		/* 85 */
	{"PdfKeywds",""},		/* 86 */
	{"PdfCreator",""},		/* 87 */
	{"PdfCreDate",""},		/* 88 */
	{"PdfModDate",""},		/* 89 */
	{"PdfPage",""},		/* 90 */
	{"PdfDisplay",""},	/* 91 */
	{"PdfSize",""},		/* 92 */
	{"PdfZoom",""},		/* 93 */
	{"PdfNtDisplay",""},		/* 94 */
	{"PdfNtWidth",""},		/* 95 */
	{"PdfNtDepth",""},		/* 96 */
	{"PdfNtBColor",""},		/* 97 */
	{"PdfBmarkLev",""},		/* 98 */
	{"PdfCrpDOX",""},		/* 99 */
	{"PdfCrpDOY",""},		/* 100 */
	{"PdfCrpDEX",""},		/* 101 */
	{"PdfCrpDEY",""},		/* 102 */
	{"PdfCrpCOX",""},		/* 103 */
	{"PdfCrpCOY",""},		/* 104 */
	{"PdfCrpCEX",""},		/* 105 */
	{"PdfCrpCEY",""},		/* 106 */
	{"PdfCrpCW",""},		/* 107 */
	{"PdfCrpCD",""},		/* 108 */
	{"PdfCrpDW",""},		/* 109 */
	{"PdfCrpDD",""},		/* 110 */
	{"PdfBead",""},			/* 111 */
	{"",""},		/* 112 */
	{"",""},		/* 113 */
	{"",""},		/* 114 */
	{"",""},		/* 115 */
	{"",""},		/* 116 */
	{"",""},		/* 117 */
	{"",""},		/* 118 */
	{"",""},		/* 119 */
	{"Distill",""},			/* 120 */
	{"LineStat",""},		/* 121 */
	{"",""}			/* (last, leave unused)  */
};
#define KEYWORD_COUNT 122

char *nme;
char overflow_path[16];
char ColorTableName[MAX_NAME];
char PsfTableName[MAX_NAME];
char PrinterName[MAX_NAME];
char PageName[MAX_NAME];
char FirstProjofBook[MAX_NAME+4];
char LastProjofBook[MAX_NAME+4];
int BookPrint;
int MasterNameFlag;
int lnumbers;
int suppresslp;
int NoFont;
int GalleySlugFlag;
int KeyScale;					/* -1 for scale to fit */
int KeyScaleX, KeyScaleY;
int KeyOutputType;				/* 0 = composite, 1 = separate */
int GraphicsPresentFlag;
int MultiPagesUp;				/* Non zero is 2 up */
int MultiPagesOddEvenFlag;		/* 0= left side (even), 1= right side (odd) */
int MultiPagesFirstIsOdd;		/* 0 = first page is even, 1 = first is odd */
uint32 MultiOddPage_cc_mask;	/* cc_hit at end of odd page */
int err;
static char *parse_buff;
char UserdataPath[132];
unsigned char Line[2000];		/* Characters to be set on line. */
float Kspace[2000];				/* Space adjustment between chars. */
int flagGlyph;					/* 1=stack() has a glyph ready to print.  */

char tpname[132];				/* Name of TP file for this unit. */
char prologname[132];
char trailername[132];
FILE *PrologName;
FILE *TrailerName;
char logname[132];
int Linesub;					/* Index to be used with line. */
int prologue_output_flag;
int CMYK_Allowed;
#if LPMfloat
char LPMK[LPM_MaxOption];
#else
unsigned int LPMK;
#endif
int setpage_allowed;
int LYPrintFlag;				/* hex
								   1   = MasterPage Data Bases
								   2   = MasterPage Illustrations
								   4   = Draw Layouts
								   8   = Print Layout specs and frame data
								   10  = Text File */
int Reports;					/* -1 = all files in directory and all used
								   0 = only those files in directory (default)
								   1 = only those files used */
int KeyTrimFlags;				/* 1 = trim, 2 = register, 3 = both */
int KeyTrimWeight;				/* in 1/20 points */
int KeyTrimLength;
int KeyRegisterLength;			/* in points */
double KeyRegisterWt;
int KeyTrimRegGap;				/* in points */
int EpsFlag;
int DownLoadFont;
int ResolveOPI;
char Uname[132];
int color_trace;
int text_trace;
int FirstGal, LastGal;
char Psf_path_name[132];
int HelveticaFontNumber;
int Jrule;						/* word 96 in standards */
int Proof;						/* 1= use .pixel file, not .ps graphic file */
int IncludeScreens;				/* 1 = set screens, 1 is default */
int LPI;						/* 0 = use colortable */
uint32 Plates;					/* use as mask for cc_mask if Output=1 */
int ChapterPage;
int ChapterPageSetFlag;			/* 1 after chaptr page adjusted to 1st page */
int WidthOfNumeric;				/* from kmd64 -13 */
int page_number[3];				/* for folios */
int SupressBlankPages;			/* Key Word 'Blank' to supress blank pages */

uint32 BlackPaint_cc_mask;		/* 1 - black paint, 0 = white */
uint32 PaintOnPage;				/* 1 bit per plate - each 1 means non-white
								   paint on the plate. */
int DontPaintFlag;				/* from kmd64, -10 and MiscLineFlags &0x10
								   1= white paint, 0 = normal paint */
int Neg;						/* 1 if negative output - keyword Neg */
int Mirror;						/* 1 if mirror output - keyword Mirror */
int FootnoteFrameFlag;			/* 1 if frame is a footnote */
int BcEcFlag;					/* BcEc mode from Key Word input */
int CurrentLineBcEc;			/* 0 = do output normally, 1 = kill output */
int PrevLineBcEc;
int BcEcExtraLeadNeeded;		/* Extra BcEc lead to add on this line */
int BcEcExtraLead;				/* Extra leading between [ec & [bc */
int EditTraceFlag;				/* 0 = none, 1 = current edits, 2 = previous */
int CurInsOffset;
int PreInsOffset;
int CurDelOffset;
int PreDelOffset;
int CurInsClr;
int PreInsClr;
int CurDelClr;
int PreDelClr;
float CurInsWeight;
float PreInsWeight;
int galley_count;
int MasterOutputSwitch;
int ClipPathFlag;
int NumCopies;
int CollateFlag;
int LockFlag;
int ExitError;
int HNJ_LockFailureFlag;
int PageWInch, PageDInch;
int DoExtractBreaks;			/* 1=Output page breaks in pp/pq extracts.  0=Don't. */
int DistillFlag;				/* 1=Distill to PDF  0=Don't.  */
int LineStats;					/* 1+=Print line stats after line #s.  0=Don't.  
									5+=Print the stats using val as the Pt-size.  */

int16 Current_FgColor;	/*  Fix for BUG 594q used to reset color after underscore  */
int16 Current_FgShade;	/*  Fix for BUG 594q used to reset shade after underscore  */
int16 LastLinemaxErr;			/* Prevent mult err msgs on Line[] overflow.  */ 

static Pfd source_pfd;

/***********************************************************************
 **  POST_PRINT.C mainline.	**
 ***********************************************************************/
void main(int argc, char *argv[])
{
    int num_read, i;
    int one_shot;
    int pipe_in, pipe_out;
    int result;
	char tp_extend[18];
	char *new_Psf_path_ptr;
    char *parse_buff_ptr;

	umask(0);
    set_sigs();					/* get all traces out at end */
    p_init(0);
	lc_init(1);
	memset ( (char *)&ObjPageWn.page_number[0], 0,
			 sizeof (struct obj_pg_wn) );
	obj_wn_count = 0;
    one_shot = 0;

	Current_FgColor = 1;			/*  BUG 594q Initialize current color to black     */
	Current_FgShade = 100;			/*  BUG 594q Initialize current shade to 100%   */
    msg_parse_args(argc, argv, KEYWORD_COUNT, msg);
    pipe_in = atoi(msg[0].answer);
    pipe_out = atoi(msg[1].answer);
    if((pipe_in == 0) && (pipe_out ==0))
		one_shot = 1;			/* don't read from pipe */
    p_info(PI_INFO, "Postprint Startup.\n"); /* log in first message. */
    strcpy (overflow_path,"overflow.pos1"); /* Build name of overflow-text
											   temp file, using ipc suffix.*/
	bmtop = 0;					/* Init dynamic PDF bookmark mem array.  */
	uidtop = 0;					/* Init dynamic PDF bookmark-idrid mem array.  */

/***************************** start of big loop ***************************/
    for(;;)
    {
		if (p_LogFd)
		{
			fclose (p_LogFd);
			p_LogFd = NULL;
		}
		if (TrailerName)
		{
		        fclose(TrailerName);
			TrailerName = NULL;
		}
		if (PrologName)
		{
		        fclose(PrologName);
			PrologName = NULL;
		}

		p_LogCnt = 0;
		if (Fofd)
			p_close(Fofd);
		FoName[0] = 0;
		parse_buff = p_alloc(2048);
		if ( !m_fprintf_size)
		{
			m_fprintf_size = 4096;
			m_fprintf_buff = p_alloc(m_fprintf_size);
		}
		if( !one_shot)
		{						/* read from the pipe */
			num_read = read(pipe_in, parse_buff, 2048);
			switch (num_read)
			{
			  case -1:
				p_info(PI_ELOG, "pipe read failed\n");
				exit(1);
			  case 0:
				p_info(PI_ELOG, "pipe_in EOF\n");
				exit(1);
			  default:
				break;
			}					/* end switch(num_read) */
#ifdef TRACE
			if(debugger_trace)
				p_info(PI_TRACE, "pipe num_read= %d, '%s' \n",num_read,parse_buff);
#endif
			msg_parse(parse_buff, num_read, KEYWORD_COUNT, msg);
        }						/* end of pipe read */
		else
		{						/* read from arg & argv or from file */
			if ((one_shot == 1) && !source_pfd)
			{
			    if ( msg[44].answer[0])
				{				/* KEYWORD 'source', read from file */
					one_shot = 2;
					if((source_pfd = p_open(NO_STRING, OTHER_FILE, NO_STRING,
											msg[44].answer, "r")) == 0)
					{			
						p_info(PI_ELOG, "Failed to open source '%s'\n",msg[44].answer);
						exit(2);
					}
				}
			}
			if ( source_pfd)
			{					/* get next job from source file */
				if (p_fgets(parse_buff, 2048, source_pfd) == 0)
				{
					p_info(PI_ELOG, "source file empty\n");
					exit(3);
				}
				parse_buff_ptr = parse_buff;
				while ( *parse_buff_ptr)
				{
					if ( *parse_buff_ptr == '\012')
					{
						*parse_buff_ptr = 0;
						break;
					}
					parse_buff_ptr++;
				}
				msg_parse (parse_buff, (int)(parse_buff_ptr - parse_buff + 1),
						   KEYWORD_COUNT, msg);
			}
		}
#if LPMfloat
		for (i = 0; i < LPM_MaxOption; i++)
			LPMK[i] = lpm_option(i);
#else
		LPMK = lpm_query(LPM_Options,LPM_CurrRev);
#endif
        result = job_setup();
        p_free(parse_buff);
        if(result)
        {						/* only if setting traces */
			if(!one_shot)
			{
				if((num_read = write(pipe_out, "0", 2))== -1)
				{
					p_info(PI_ELOG, "write to pipe failed\n");
					exit(1);
				}
				else
					continue;	/* traces only, get next pipe message */
			}
			exit(1);
        }
        strcpy(tpname, JobName); /* TP filename. */
		if ( FileType)
		{						/* layout */
			tp_extend[0] = 0;
			if (MasterNameFlag)
				sprintf (tp_extend, ".%s", PageName);
			if ( LYPrintFlag)
				strcat (tp_extend, ".lyp");
			strcat (tpname, tp_extend);
			if ( (FirstPage >= 0) && ( !MasterNameFlag) )
			{
				if (LastPage != FirstPage)
					sprintf(tp_extend, ".%d_%d",FirstPage,LastPage);
				else			/* if equal only use one page number */
					sprintf(tp_extend, ".%d",FirstPage);
				strcat(tpname,tp_extend);
			}
			else if (BookPrint)
			{
				if (LastParentIndex > ParentIndex)
					sprintf(tp_extend, ".u%d_u%d",ParentIndex,LastParentIndex);
				else
					sprintf(tp_extend, ".u%d",ParentIndex);
				strcat(tpname,tp_extend);
			}
		}
		if (EpsFlag)
		   strcpy (logname, p_path(TreeName, EPS_FILE, SubDirName, tpname));
		else if (BookPrint)
		   strcpy (logname, p_path(TreeName, OUTPUT_FILE, ParentDirName, tpname));
		else
		   strcpy (logname, p_path(TreeName, OUTPUT_FILE, SubDirName, tpname));
		strcpy (prologname, logname);
		strcpy (trailername, logname);

		strcat (logname, ".log");
		p_LogFd = fopen (logname, "w+");

		strcat (prologname, ".plog");
		PrologName = fopen(prologname, "w+");

		strcat (trailername,".ptrl");
		TrailerName = fopen(trailername, "w+");

		new_Psf_path_ptr = p_path(TreeName, USERDATA, UserdataPath,
								  PsfTableName);
		if ( strcmp( Psf_path_name, new_Psf_path_ptr) )
		{						/* no match, need a new name and Psftable */
			strcpy(Psf_path_name, new_Psf_path_ptr);
			if ( !Psftable_load_with_name(TreeName, SubDirName,
										  PsfTableName) )
			{					/* bad psftable load, abort */
				p_info(PI_ELOG, "Unable to open file '%s'",Psf_path_name);
				exit(101);
			}
			PsfNameIndex = 0;
			PsfCodeIndex = 0;
			HelveticaFontNumber = 0;
			for (i=0; i<MaxPentaFont; i++)
			{					/* find Helvetica */
				if ( PentaToPsf[i].name)
				{
					if ( !strcmp(PentaToPsf[i].name, "Helvetica") )
						HelveticaFontNumber  = i;
				}
			}
		}						/* psftable loaded */

		if (HdrPNamPrt)
			get_prt_name();  /*  Bug 367p get actual printer name from .psh file */
		else
			PrtNam[0] = 0;
	
		getpgsize();			/* Parse header lines */

		if(lnumbers & 1) {		/* Print line numbers?  */
			if (!FileType) {	/* In galley mode:  */
				OffL += 40;		/* =15pts space at left + 25 for big line number  */
				if (LineStats)
					OffL += 30;	/* =5pts at left + 25 line# + 40 for band & ls  */
			}
			else				/* In page mode:  */
				OffL += 10;		/* just 10 pts of room.  */
		}
/***********************************************************************
 **  Call the PostScript mainline processor.			      **
 ***********************************************************************/
		
		psmain();
		
/***********************************************************************
 **  Wrap it all up.						      **
 ***********************************************************************/
		if (EpsFlag)
		   strcpy (logname, p_path(TreeName, EPS_FILE, SubDirName, tpname));
		else if (BookPrint)
		   strcpy (logname, p_path(TreeName, OUTPUT_FILE, ParentDirName, tpname));
		else
		   strcpy (logname, p_path(TreeName, OUTPUT_FILE, SubDirName, tpname));
		if (PrinterName[0])
		{						/* output to the printer queue */
			fflush (p_LogFd);
			if( !GraphicsPresentFlag)
				result = 0;
			else
				result = 1;

			sprintf(shell_msg,"/usr/local/bin/%s.psh %s %d %d %s %s %d %d %d %d",
                PrinterName, logname, result, FileType, TreeName,
				SubDirName, FirstPage, LastPage, HNJ_LockFailureFlag,
				DistillFlag);
			system(shell_msg);
		}

		/* If user asked for "Distill to PDF" (and there's no H&J lock), then
			test to find the newest .tp, .eps or .ps in the .drawer, and see
			if it's newer than any .pdf file there.
			If so, this means the printer script did NOT create a .pdf, so
			distill the .tp to .pdf using ghostscript's ps2pdf script here.
		*/
		if (DistillFlag && !HNJ_LockFailureFlag)
		{
			struct stat tpstat, psstat, pdfstat;
			int tpind, psind, pdfind, buildpdf;
			char *dotptr;

			strcpy (prologname, logname);
			strcat (prologname, ".pdf");	/* Get stat of *.tp.pdf file:  */
			pdfind = stat(prologname, &pdfstat);
			if (pdfind)						/* If that doesn't exit, alt try:  */
			{
				strcpy (prologname, logname);
				dotptr = strrchr (prologname, '.');
				*dotptr = 0;				/* Strip whatever suffix we have,  */
				strcat (prologname, ".pdf");/*  replace with .pdf.  */
											/* Get stat of that pdf file:  */
				pdfind = stat(prologname, &pdfstat);
			}

			tpind = stat(logname, &tpstat);	/* Get stat of .tp (or .eps) file  */
			psind = 1;						/* (assume unused)  */
			if (!EpsFlag)					/* If we started with .tp file,  */
			{								/*  also get stat of .ps file:  */
				strcpy (prologname, logname);
				strcpy (&prologname[strlen(logname)-3], ".ps");
				psind = stat(prologname, &psstat);
			}

			buildpdf = 0;				/* Assume no need to build a .pdf  */
			if (!tpind &&				/* If .tp (or .eps) exists, and  */
				(psind ||				/*  .ps does not, or .tp is newer, then:  */
				 tpstat.st_mtime > psstat.st_mtime))
			{
				strcpy (prologname, logname);	/* (use .tp name)  */
										/*  if there's no .pdf or it's older,  */
				if (pdfind || tpstat.st_mtime > pdfstat.st_mtime)
					buildpdf = 1;		/*  set flag to distill the .tp file.  */
			}
			else 
			if (!psind &&				/* If .ps exists, and  */
				(tpind ||				/*  .tp does not, or .ps is newer, then:  */
				 tpstat.st_mtime <= psstat.st_mtime))
			{
				strcpy (logname, prologname);	/* (use .ps name)  */
										/*  if there's no .pdf or it's older,  */
				if (pdfind || psstat.st_mtime > pdfstat.st_mtime)
					buildpdf = 1;		/*  set flag to distill the .ps file.  */
			}
			else						/* Neither exist, forget it.   */
				;

			if (buildpdf)				/* If we decided to distill a .pdf, then  */
			{							/* build basename by stripping suffix,  */
				*(dotptr = strrchr (prologname, '.')) = 0;
										/* build little script calling ps2pdf,  */
			  	sprintf (shell_msg,
				"PATH=/usr/local/bin/ghostscript:$PATH; export PATH; ps2pdf  %s %s.pdf",
				logname, prologname);

				p_info(PI_INFO, "Executing distillation to PDF from Postprint\n");
				system(shell_msg);		/* and send it off, wait for done.  */
			}
		}					/* end if (DistillFlag && !HNJ_LockFailureFlag)  */

		p_free(m_fprintf_buff);
		m_fprintf_size = 0;
		if(!one_shot)
		{
			if((num_read =write(pipe_out, "0", 2))== -1)
			{
				p_info(PI_ERROR, "write to pipe failed\n");
				exit(1);
			}
			else
				p_info(PI_INFO, "Post completed for %s \n",tpname);
		}
		else
		{
			if (!source_pfd)
			{
				p_info(PI_INFO, "Post completed for %s, program exit. \n",JobName);
				break;
			}
			else
				p_info(PI_INFO, "Post completed for %s \n",JobName);
		}
    }							/* end of for(;;) */
    exit(ExitError);
}								/* end of PSMAIN */

/**********************************************************************/
void stop(char *s1, char *s2, int eror)
{
    char msgbuff[256];
    
    if (eror > 0 )
    	sprintf (msgbuff, "PostPrint Fatal ERROR : %d ", err);
	else
	    sprintf (msgbuff, "PostPrint Fatal ERROR : ");

    if (s1 && (*s1 > ' '))
		strcat (msgbuff, s1);
    if (s2 && (*s2 > ' '))
		strcat (msgbuff, s2);
    strcat(msgbuff, "\n");
    p_info(PI_ELOG, msgbuff);
    exit(1);
}

/***********************************************************************
 **  PostScript mainline processor.				      **
 ***********************************************************************/
static void psmain(void)
{
    int color_done;			/* done doing colors */
	int i;
	
	ini_JOB();					/* init job stuff		*/
    ini_PAGE();					/* init %%Page: comment state */
	Fofd = 0;
    GraphicsPresentFlag = 0;
    NoFont = 0;
	width_open(); /* Open WIDTH for H&Js char-width round-off errors*/
	FontBase = 1000;
#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "Starting job:%s\n",JobName);
#endif
    cc_hit = 0;
	cc_mask = (1 & Plates); /* hit nothing, set all black */
	if ( !cc_mask) 
		m_fopen(tpname, "w+", 1); /* delete prev version of .tp */
    spot_pass = 1;				/* 1st pass */
    color_done = 0;				/* not done yet */
	PageRotationAngle = 0;
	frame_fg_attribute = 0;		/* default is opaque */
	prologue_output_flag = 0;
	PageNo1 = 1;
	underscore_min = 10;
	overflow_page_buff_count = 0;
	memset ( (char *)overflow_page_buff, 0, sizeof (overflow_page_buff) );
	ExitError = 0;
	HNJ_LockFailureFlag = 0;
    while(color_done != 1 )		/* until we get all colors */
    {
		FootnoteFrameFlag = 0;
		BlackPaint_cc_mask = cc_mask;
		PaintOnPage = 0;
		blend_flag = 0;
		BlendAngle = 0;
		BlendEndColor = 0;
		in_overflow = 0;
		was_overflow = 0;
		Linesub = -1;			/* clear text/escapement buffer	*/
		flagGlyph = 0;
		*GlyphName = 0;
		for(i=0; i<1999; i++)
		{
			Line[i] = 0;
			Kspace[i] = 0;
		}
		for(i=0; i<40; i++)
			bf_kmd80[i].active_flag = 0;
		fowrd =  256;
		Xmark =  0;
		y_max =  0;
		Oldfont = -1;
		Holdfont =  0;
		usw = 0;
		StrikeThruRuleStartFlag = 0;
		FlashOn = 1;
		CurrentFrame = 0;
		CurrentEle = 0;
		memset( (char *)&fo_line_def.FoStatusLength, 0, sizeof(LDEF) );
		DontPaintFlag = fo_line_def.MiscLineFlags & 0x10;
		not_lnum = 1;			/* Default: Put text font, not line-num font*/
		last_lnum = 0;
		jbl = 0;
		repeatno = 0;
		Obdegree = 0;
		Old_ob = 0;
		Hold_ob = 0;
		Holdss = 0;
		Holdps = 0;
		Oldss = 0;
		Oldps = 0;
		Ktabmode = 0;
		psend = 0;
		WidthOfNumeric = 0;
		memset ( page_number, 0, sizeof(page_number) );
		CurrentLineBcEc = 0;
		PrevLineBcEc = 0;
		BcEcExtraLeadNeeded = 0;
		BcEcExtraLead = 0;
		LastLinemaxErr = 0;
		cc_hit &= (Plates & ~1); /* only use plates to be output */
#ifdef TRACE
		if(debugger_trace)
		{
			p_info(PI_TRACE, "spot color pass %d, cc_mask: %lo , cc_hit: %lo\n",
				   spot_pass, cc_mask, cc_hit);
			p_info(PI_TRACE, "processing Tree: '%s', Dir: '%s',file: '%s', type= %d\n",
				   TreeName, SubDirName, JobName, FileType);
		}
#endif
		switch(FileType)
		{
		  case 0:				/* Galley output */
			if (spot_pass != 2)
				cc_hit = 0;
			if((err = dtext()) )
				error("processing FO file, err = ",JobName,err);
			if( (cc_hit < 2) ||	/* picked up no color(s) */
			   (spot_pass != 1) || /* or on second pass thru file */
				( !KeyOutputType) )	/* or composite */
				color_done = 1;	/* we are done */
			else
			{
				spot_pass = 2;	/* mark as 2nd pass thru file */
				cc_hit &= (Plates & ~1); /* mask off black */
				cc_mask = 0;	/* not the black we start with */
				galley_count = 0;
				MasterOutputSwitch = 1;
				if(Fofd)		/* if still open */
					foread(1);	/* rewind open FO file */
			}
			break;
			
		  case 1:				/* DesignMaster */
			if((err = dxy()) )
				p_info(PI_ELOG, "ERROR --processing Tree: '%s', Dir: '%s', Unit: %s\n",
					   TreeName,SubDirName,JobName);
			color_done = 1;		/* color loop inside dxy */
			break;
		}						/* end switch(FileType) */
		m_close(-1);			/* close all output */
    }							/* end while(color_done!=1) */
    if (wdfd)
    {
		p_close (wdfd);
		wdfd = 0;
    }
    if (Fofd)
    {
		p_close(Fofd);
		Fofd = 0;
		FoName[0] = 0;
    }
   if (TrailerName) {
		fclose(TrailerName);
		TrailerName = NULL;
    }
    if (PrologName)
    {
		fclose(PrologName);
		PrologName = NULL;

		if (EpsFlag)
		   strcpy (logname, p_path(TreeName, EPS_FILE, SubDirName, tpname));
		else if (BookPrint)
		   strcpy (logname, p_path(TreeName, OUTPUT_FILE, ParentDirName, tpname));
		else
		   strcpy (logname, p_path(TreeName, OUTPUT_FILE, SubDirName, tpname));

	  	sprintf(shell_msg,"cat %s >> %s \n mv %s %s \n cat %s >> %s \n mv %s %s",
			prologname, trailername,
			trailername, prologname,
			logname, prologname,
			prologname, logname);
		system(shell_msg);
    }
#ifdef TRACE
    if(debugger_trace)
		p_info(PI_TRACE, "Finished job, Tree: '%s', Dir: '%s', Name: '%s'.\n",
			   TreeName,SubDirName,JobName);
#endif
}
/**********************************************************************/
static void set_traces(char *string)
{
    while(*string)
    {
		int c;
		c = *string++;
		switch (c)
		{
		  case 'a':
			color_trace = 1;
			debugger_trace = 1;
			trace_lmt = 1;
			trace_8 = 1;
			parse_trace = 1;
			text_trace = 1;
			break;
		  case 'c':
			color_trace = 1;
			break;
		  case 'd':
			debugger_trace = 1;
			break;
		  case 'e':
			trace_lmt = 1;
			break;
		  case 'l':
			trace_8 = 1;
			break;
		  case 'p':
			parse_trace = 1;
			break;
		  case 't':
			text_trace = 1;
			break;
		  case '0':
			color_trace = 0;
			debugger_trace = 0;
			trace_lmt = 0;
			trace_8 = 0;
			parse_trace = 0;
			text_trace = 0;
			break;
		  default:
			p_info(PI_WLOG, "bad field %c\n",c);
		}						/* end switch */
    }							/* end while */
}								/* end function */

/**********************************************************************/
static int job_setup(void)
{
    int length, i, c;    
    int16 stds_buff[240];
    Pfd sysfd;
	int temp_reg_length = 0;
	int temp_trim = 0;

	float fcolor;
	int ii;
	char pdfword[8192];
	char pdftmp[8192];
	char pdfc[5];
	char *p;

	PdfActive = 0;				/* Assume no PDF style given, thus 
									no PDF output desired.  */
	/* Clear PDF values - Set defaults for menu items left blank  */
	/*   (all assuming that PDF output is active).  */
	pdflinkctr = 0;
	pdflinkAtSOL = 0;

	job_link.btype=3; /* none */
	job_link.weight=1;
	job_link.bcolor=000000000; /* White */
	strcpy(job_link.ocolor,"0 0 0");
	job_link.dashlen=4;
	job_link.dashgap=4;
	job_link.radius=0;

	pdf_info.title[0] = '\0';
	pdf_info.auth[0] = '\0';
	pdf_info.subj[0] = '\0';
	pdf_info.keyw[0] = '\0';
	pdf_info.creator[0] = '\0';
	pdf_info.date1[0] = '\0';
	pdf_info.date2[0] = '\0';

	pdf_docview.initpage=1;
	pdf_docview.initdisp=2; /* page only */
	pdf_docview.initsize=2; /* fit zoom */
	pdf_docview.initzoom=0; /* full screen */

	pdf_note_disp=0;
	pdf_note_length=60;
	pdf_note_depth=60;
	pdf_note_color=255255255;
	strcpy(pdf_note_ocolor,"1 1 1");

	pdf_output_lev = 9; /* all levels */
	pdf_crop_cw = pdf_crop_cd = 0;	/* No crop box. */
	pdf_crop_dw = pdf_crop_dd = 0;
	pdf_crop_cox = pdf_crop_coy = pdf_crop_cex = pdf_crop_cey = 0;
	pdf_crop_dox = pdf_crop_doy = pdf_crop_dex = pdf_crop_dey = 0;

    set_traces(msg[7].answer);
#ifdef TRACE
    if(debugger_trace)
    {
		for (i=0; i<KEYWORD_COUNT; i++)
			p_info(PI_TRACE, "i= %d, Keyword %s, Value= %s \n",
				   i,msg[i].keyword,msg[i].answer);
    }
#endif
	if (msg[68].answer[0])
		LockFlag = 0;
	else
		LockFlag = 1;
	FirstGal = 0;
	LastGal = 1000000;
    if ( !msg[6].answer[0])
    {							/* no Tree name present */
		if( !msg[7].answer[0])
		{						/* no traces change */
			p_info(PI_ELOG, "ERROR - 'Tree' name missing, program exit \n");
			exit(1);
		}
		return(1);				/* change traces only */
    }
    length = strlen(msg[6].answer);
    if( length >= MAX_NAME) 
    {
		p_info(PI_ELOG, "ERROR- 'Tree' name '%s' exceeds %d characters, program exit.\n",
			   msg[6].answer, MAX_NAME - 1);
		exit(1);
    }
    strcpy(TreeName, msg[6].answer);
    if ( !msg[4].answer[0])
    {							/* sub-dir name missing */
		if( msg[2].answer[0] )
		{						/* need subdir name if it is a galley */
			p_info(PI_ELOG, "ERROR - 'Dir' name is missing, program exit \n");
			exit(1);
		}
		else
		{						/* create SubDirName if not present for unit */
			if( !msg[3].answer[0] )
			{					/* need subdir name if it is a galley */
				p_info(PI_ELOG, "ERROR - 'Dir' name is missing, program exit \n");
				exit(1);
			}
			else				/* use unit name for project */
				sprintf(SubDirName,"%s.prj",msg[3].answer);
		}
    }
    else
    {
		length = strlen(msg[4].answer);
		if( length >= MAX_NAME) 
		{
			p_info(PI_ELOG, "ERROR - 'Dir' name '%s' exceeds %d characters, program exit.\n",
				   msg[4].answer, MAX_NAME - 1);
			exit(1);
		}
		strcpy(SubDirName, msg[4].answer);
    }
    if ( p_get_data_name ( TreeName, SubDirName, UserdataPath, 0 ) )
    {
		p_info(PI_ELOG, "ERROR - missing file '/Penta/%s/desks/%s/.data', program exit.\n",
			   TreeName, SubDirName);
		exit(1);
    }
#ifdef TRACE
    if (debugger_trace)
		p_info(PI_TRACE, "path to userdata is '%s' \n",UserdataPath);
#endif
    if ((sysfd = p_open(TreeName,USERDATA,UserdataPath,"standards","r")) == 0)
    {
		p_info(PI_ELOG, "ERROR - file 'standards' is missing, program exit.\n");
		exit(1);
    }
    if ( !(p_read((char *)stds_buff, 240, 1, sysfd, 0, BS16) ) )
    {
		p_info(PI_ELOG, "ERROR - cannot read file 'standards', program exit.\n");
		exit(1);
    }
    p_close(sysfd);
    HorizontalBase = stds_buff[35];
    VerticalBase = stds_buff[74];
    Jrule = (int)stds_buff[95];
	if ( !Jrule)
		Jrule = 10;
	BookPrint = 0;
    if ( msg[2].answer[0])
    {							/* galley name */
		length = strlen(msg[2].answer);
		if( length >= MAX_NAME) 
		{
			p_info(PI_ELOG, "ERROR - 'Galley' name '%s' exceeds %d characters, program exit. \n",
				   msg[2].answer, MAX_NAME - 1);
			exit(1);
		}
		strcpy(JobName, msg[2].answer); /* we have a galley name */
		FileType = 0;			/* Galley output */
		FirstPage = 0;
    }							/* end galley name */
    else if ( msg[3].answer)
    {							/* use the unit name */
		length = strlen(msg[3].answer);
		if( length >= MAX_NAME) 
		{
			p_info(PI_ELOG, "ERROR - 'Unit' name '%s' exceeds %d characters, program exit.\n",
				   msg[3].answer, MAX_NAME);
			exit(1);
		}
		FileType = 1;			/* must be a unit name */
		strcpy(JobName, msg[3].answer);

		FirstPage = -1;			/* (assume no start/end pgs given)  */
		LastPage = -1;
		if (strstr(msg[8].answer, ".prj"))
		{						/* Child-project name is passed instead of first pg#.
									Store project name, set doing-book flag:  */
			strcpy (FirstProjofBook, msg[8].answer);
			strcpy (LastProjofBook, FirstProjofBook);
			BookPrint = 1;
			if (strstr(msg[9].answer, ".prj"))
				strcpy (LastProjofBook, msg[9].answer);
		}
		else if (msg[8].answer[0])	/* Regular starting page #  */
		{
			FirstPage = atol(msg[8].answer);
			if (msg[9].answer[0])		/* Regular ending page #  */
				LastPage = atol(msg[9].answer);
		}
		if ( msg[18].answer[0])
		{						/* only use for debugging */
			FirstPage = atol(msg[18].answer);
			if ( !msg[19].answer[0])
				LastPage = -1;
			else
				LastPage = atol(msg[19].answer);
		}
		if ( msg[33].answer[0])	/* Layout */
			strcpy (msg[21].answer, msg[33].answer);
		if ( msg[21].answer[0])	/* FirstMaster */
		{
			length = strlen(msg[21].answer);
			if( length >= MAX_NAME) 
			{
				p_info(PI_ELOG, "ERROR - 'FirstMaster' name '%s' exceeds %d characters, program exit. \n",
					   msg[21].answer, MAX_NAME - 1);
				exit(1);
			}
			strcpy(PageName, msg[21].answer); /* we have a layout name */
			MasterNameFlag = 1;
			FirstPage = 0;
			LastPage = 0;
		}
		else
		{
			MasterNameFlag = 0;
			PageName[0] = 0;
		}
		if(psinit())
			exit(1);
    }							/* end unit name */
    else
    {
		p_info (PI_ELOG, "ERROR - missing 'Galley' or 'Unit' name, program exit. \n");
		exit(1);
    }
	if (msg[5].answer[0])
	{							/* printer name */
		length = strlen(msg[5].answer);
		if( length >= MAX_NAME) 
		{
			p_info(PI_ELOG, "ERROR - 'PrinterName' '%s' exceeds %d characters, program exit. \n",
				   msg[5].answer, MAX_NAME - 1);
			exit(1);
		}
		strcpy(PrinterName, msg[5].answer);
    }
    else
		strcpy(PrinterName, "\0\0");

    lnumbers = atoi(msg[10].answer);
    suppresslp=0;

    if ( msg[11].answer[0])
    {
		length = strlen(msg[11].answer);
		if( length >= MAX_NAME) 
		{
			p_info(PI_ELOG, "ERROR - 'ColorTable' name '%s' exceeds %d characters, program exit. \n",
				   msg[11].answer, MAX_NAME - 1);
			exit(1);
		}
		strcpy(ColorTableName, msg[11].answer);
    }
    else
		strcpy(ColorTableName, "colortable");
    init_color();
    p_info(PI_INFO, "Postprint starting Tree: '%s', Dir: '%s', Job: '%s' \n",TreeName,SubDirName,JobName);
    if ( msg[12].answer[0])
		GalleySlugFlag = atoi(msg[12].answer); /* use keyword with 0 or 1 */
    else
    {							/* no keyword, use defaults */
		if ( FileType)
			GalleySlugFlag = 0;	/* default is no print slug if Unit */
		else
			GalleySlugFlag = 1;	/* default is print slug if Galley */
    }
	if ( msg[13].answer[0])
		strcpy(PsfTableName, msg[13].answer); /* name of PSFTABLE */
	else
		strcpy(PsfTableName, "psftable");
	Orient = 0;					/* portrait is default */
	if ( msg[14].answer[0])
	{							/* p or P = portrait, l or L = landscape */
		switch (msg[14].answer[0])
		{
		  case 'P':
		  case 'p':
			Orient = 0;			/* portrait */
			break;
		  case 'L':
		  case 'l':
			Orient = 90;		/* landscape */
			break;
		  default:
			p_info(PI_ELOG, "ERROR - Orient '%s' invalid, defaulting to portrait.\n",
				   msg[14].answer);
			break;
		}
	}
	if ( msg[15].answer[0])
	{							/* KeyScale */
		if ( (msg[15].answer[0] & 0xdf) == 'X')
			KeyScale = -1;		/* defined as 'X' */
		else
			KeyScale = atoi(msg[15].answer);
	}
	else
		KeyScale = -2;			/* not defined */
	if ( msg[16].answer[0])		/* KeyScaleX */
		KeyScaleX = atoi(msg[16].answer);
	else
		KeyScaleX = -1;
	if ( msg[17].answer[0])		/* KeyScaleY */
		KeyScaleY = atoi(msg[17].answer);
	else
		KeyScaleY = -1;
	KeyOutputType = 0;			/* default is composite */
	if ( msg[20].answer[0] == '1')
	{
#if LPMfloat
		if ( LPMK[LPM_PWS1] || LPMK[LPM_PWS2] )
#else
		if ( (LPMK & LPM_PWS1) || (LPMK & LPM_PWS2) )
#endif
			p_info(PI_ELOG, "ERROR - Color separation not allowed on a Personal Workstation.\n");
		else
#if ! LPMfloat
		if((LPMK & LPM_SpotColor))
#endif
			KeyOutputType = 1;	/* color separation */
	}
	if ( msg[23].answer[0])		/* Header */
		setpage_allowed =
			atoi(msg[23].answer); /* 1 means dump page width and height */
	else
		setpage_allowed = -1;
	MultiPagesUp = 0;			/* default is 1 up */
	MultiPagesOddEvenFlag = 0;
	MultiOddPage_cc_mask = 0;
	if ( msg[24].answer[0])		/* Multi - for 2 pages up */
	{							/* non-zero means 2-up */
		MultiPagesUp = atoi(msg[24].answer);
		if ( !FileType || (FirstPage < 0) ||
			 ( ( FirstPage >= 0) && !(FirstPage & 1) ) )
			MultiPagesOddEvenFlag = 0;
		else
			MultiPagesOddEvenFlag = 1; /* starting Unit on odd page */
	}
	if ( msg[25].answer[0])		/* PageW */
	{
		PageW = atoi(msg[25].answer);
		PageW_key_flag = 1;
	}
	else
	{
		PageW = -1;
		PageW_key_flag = 0;
	}
	if ( msg[26].answer[0])		/* PageD */
	{
		PageH = atoi(msg[26].answer);
		PageD_key_flag = 1;
	}
	else
	{
		PageH = -1;
		PageD_key_flag = 0;
	}
	if ( msg[27].answer[0])		/* Pofft */
	{
		OffT = atoi(msg[27].answer);
		Pofft_key_flag = 1;
	}
	else
	{
		OffT = -1;
		Pofft_key_flag = 0;
	}
	if ( msg[28].answer[0])		/* Poffb */
	{
		OffB = atoi(msg[28].answer);
		Poffb_key_flag = 1;
	}
	else
	{
		OffB = -1;
		Poffb_key_flag = 0;
	}
	if ( msg[29].answer[0])		/* Poffl */
	{
		OffL = atoi(msg[29].answer);
		Poffl_key_flag = 1;
	}
	else
	{
		OffL = -1;
		Poffl_key_flag = 0;
	}
	if ( msg[30].answer[0])		/* Poffr */
	{
		OffR = atoi(msg[30].answer);
		Poffr_key_flag = 1;
	}
	else
	{
		OffR = -1;
		Poffr_key_flag = 0;
	}
	if ( msg[31].answer[0])		/* Cmyk */
		CMYK_Allowed = atoi(msg[31].answer);
	else
		CMYK_Allowed = -1;
	if ( msg[32].answer[0])		/* Hdrup */
		header_offset = atoi(msg[32].answer); /* Pts to raise slug line */
	else
		header_offset = -32760;	/* -32760 means no Hdrup keyword */
	LYPrintFlag = 0;
	if ( msg[34].answer[0])		/* LYP -  layout print */
	{
		if ( !FileType)
		{
			p_info(PI_ELOG, "ERROR - Cannot do LYP (layout print) for galley, program exit.\n");
			exit(1);

		}
/* for LayoutPrint, kill other functions */
		for (i=0; i<10; i++)
		{
			if ( !msg[34].answer[i])
				break;
			if ( msg[34].answer[i] == 'a')
			{					/* a means all reports */
				LYPrintFlag = -1;
				break;
			}
			c = (msg[34].answer[i] - '0') & 0xf;
			if ( (c <= 0) || (c > 9) )
				continue;
			LYPrintFlag |= 1 << (c -1 );
		}
	}							/* end if(msg[34].answer[0]) */
	if (LYPrintFlag)
	{
		delay_records_flag = -1; /* no sorting for reporting */
		GalleySlugFlag = 0;
		KeyOutputType = 0;
		MultiPagesUp = 0;
		lnumbers = 0;
		if ( msg[35].answer[0])
		{						/* Report directory, used, or both */
			Reports = atoi( msg[35].answer);
			switch ( Reports)
			{
			  case -1:			/* all files in directory and all used */
			  case 0:			/* only those files in directory (default) */
			  case 1:			/* only those files used */
				break;
			  default:
				p_info(PI_ELOG, "ERROR - 'Report' Key Word option %s not available. Defaulting to reporting on data base in directory.\n", msg[35].answer);
				Reports = 0;
				break;
 			}						/* end switch(Reports) */
		}
		else
			Reports = 0;		/* default */
	}
	else
		delay_records_flag = 1;	/* normal postsript delay */
	if ( msg[37].answer[0] == '1')
		KeyTrimFlags = 1;		/* trim marks */
	else
		KeyTrimFlags = 0;
	if ( msg[36].answer[0] == '1')
		KeyTrimFlags |= 2;		/* register marks */
	if ( (KeyTrimFlags & 1) && !LYPrintFlag )
    {							/* set up trim marks */
		if ( msg[41].answer[0])
			temp_trim = atoi(msg[41].answer);
		if ( !temp_trim)
			temp_trim = 24;		/* default */
		trim_mark_width = temp_trim * HorizontalBase;
		trim_mark_depth = temp_trim * VerticalBase;
		KeyTrimLength = temp_trim;
		KeyTrimWeight = 0;
		if ( msg[40].answer[0])
			KeyTrimWeight = (int )(atof(msg[40].answer) * 20); /* in 1/20 pt */
		if ( KeyTrimWeight < 2)
			KeyTrimWeight = 2;
	}
	else
	{
		trim_mark_width = 0;
		trim_mark_depth = 0;
		KeyTrimWeight = 0;
		KeyTrimLength = 0;
	}
	if ( (KeyTrimFlags & 2) && !LYPrintFlag )
    {							/* set up register marks */
		if ( msg[39].answer[0])
			temp_reg_length = atoi(msg[39].answer);
		if ( !temp_reg_length)
			temp_reg_length = 12;		/* default */
		KeyRegisterLength = temp_reg_length; /* in points */
		KeyRegisterWt = 0;
		if ( msg[38].answer[0])
			KeyRegisterWt = atof(msg[38].answer);
		if ( KeyRegisterWt < 0.1)
			KeyRegisterWt = .1;
		if ( (2 * temp_reg_length) > temp_trim)
		{
			trim_mark_width = temp_reg_length * HorizontalBase * 2;
			trim_mark_depth = temp_reg_length * VerticalBase * 2;
		}
	}
	else
	{
		KeyRegisterLength = 0;
		KeyRegisterWt = 0;
	}
	if ( (KeyTrimFlags &3) && !LYPrintFlag )
	{							/* add 9 point offset */
		if (msg[72].answer[0])	/* ie. gap between trim-line & tick */
			KeyTrimRegGap = atoi(msg[72].answer);
		else
			KeyTrimRegGap = 9;
		trim_mark_width += (KeyTrimRegGap * HorizontalBase);
		trim_mark_depth += (KeyTrimRegGap * VerticalBase);
	}
	if (msg[73].answer[0])	
		EpsFlag = atoi(msg[73].answer);
	else
		EpsFlag=0;

	if (msg[74].answer[0])	
		strcpy(Uname,msg[74].answer);
	else
		strcpy(Uname,"penta");

	if (msg[75].answer[0])	
		DownLoadFont=1;
	else
		DownLoadFont=0;

	if (msg[76].answer[0])	
		ResolveOPI=1;
	else
		ResolveOPI=0;

	if (msg[77].answer[0])
		job_link.btype=atoi(msg[77].answer);

	if (msg[78].answer[0])
		job_link.weight=atoi(msg[78].answer);

	if (msg[79].answer[0]) {
		job_link.bcolor=atoi(msg[79].answer);
		strcpy(pdftmp,msg[79].answer);
		/* split into chunks of 3 digits */
		job_link.ocolor[0] = '\0';
		p=pdftmp;
                for (ii=1; ii <= 3; ii++) {
		     strncpy(pdfc,p,3);
		     fcolor=(atoi(pdfc)/255);
                     memset(pdfword,0,sizeof(pdfword));
                     sprintf(pdfword," %.2f",fcolor);
                     strcat(job_link.ocolor,pdfword);
		     p+=3;
                 }
	}

	if (msg[80].answer[0])
		job_link.dashlen=atoi(msg[80].answer);

	if (msg[81].answer[0])
		job_link.dashgap=atoi(msg[81].answer);

	if (msg[82].answer[0])
		job_link.radius=atoi(msg[82].answer);

	if (msg[83].answer[0])
		strcpy(pdf_info.title,msg[83].answer);

	if (msg[84].answer[0])
		strcpy(pdf_info.auth,msg[84].answer);

	if (msg[85].answer[0])
		strcpy(pdf_info.subj,msg[85].answer);

	if (msg[86].answer[0])
		strcpy(pdf_info.keyw,msg[86].answer);

	if (msg[87].answer[0])
		strcpy(pdf_info.creator,msg[87].answer);

	if (msg[88].answer[0])
		strcpy(pdf_info.date1,msg[88].answer);

	if (msg[89].answer[0])
		strcpy(pdf_info.date2,msg[89].answer);

	if (msg[90].answer[0])
		pdf_docview.initpage = atoi(msg[90].answer);

	if (msg[91].answer[0])	/* This is the switch, set by DM print menu:  */
	{						/*   There was a PDF style used in this request.  */
		PdfActive = 1;		/* PDF output is now active.  */
		pdf_docview.initdisp = atoi(msg[91].answer);
	}

	if (msg[92].answer[0])
		pdf_docview.initsize = atoi(msg[92].answer);

	if (msg[93].answer[0])
		pdf_docview.initzoom = atoi(msg[93].answer);

	if (msg[94].answer[0])
		pdf_note_disp = atoi(msg[94].answer);

	if (msg[95].answer[0])
		pdf_note_length = atoi(msg[95].answer);

	if (msg[96].answer[0])
		pdf_note_depth = atoi(msg[96].answer);

	if (msg[97].answer[0]) {
                pdf_note_color=atoi(msg[97].answer);
                strcpy(pdftmp,msg[97].answer);
                /* split into chunks of 3 digits */
                pdf_note_ocolor[0] = '\0';
                p=pdftmp;
                for (ii=1; ii <= 3; ii++) {
                     strncpy(pdfc,p,3);
                     fcolor=(atoi(pdfc)/255);
                     memset(pdfword,0,sizeof(pdfword));
                     sprintf(pdfword," %.2f",fcolor);
                     strcat(pdf_note_ocolor,pdfword);
                     p+=3;
                 }
	}

	if (msg[98].answer[0])
		pdf_output_lev = atoi(msg[98].answer);

	if (msg[99].answer[0])
		pdf_crop_dox = atoi(msg[99].answer);
	if (msg[100].answer[0])
		pdf_crop_doy = atoi(msg[100].answer);
	if (msg[101].answer[0])
		pdf_crop_dex = atoi(msg[101].answer);
	if (msg[102].answer[0])
		pdf_crop_dey = atoi(msg[102].answer);
	if (msg[103].answer[0])
		pdf_crop_cox = atoi(msg[103].answer);
	if (msg[104].answer[0])
		pdf_crop_coy = atoi(msg[104].answer);
	if (msg[105].answer[0])
		pdf_crop_cex = atoi(msg[105].answer);
	if (msg[106].answer[0])
		pdf_crop_cey = atoi(msg[106].answer);
	if (msg[107].answer[0])
		pdf_crop_cw = atoi(msg[107].answer);
	if (msg[108].answer[0])
		pdf_crop_cd = atoi(msg[108].answer);
	if (msg[109].answer[0])
		pdf_crop_dw = atoi(msg[109].answer);
	if (msg[110].answer[0])
		pdf_crop_dd = atoi(msg[110].answer);
	if (msg[111].answer[0])
		pdf_bead = atoi(msg[111].answer);
	else
		pdf_bead = 0;


	MultiPagesFirstIsOdd = MultiPagesOddEvenFlag;
	if ( !FileType)
	{							/* if galley, see if start & end galley */
		if ( msg[42].answer[0])
		{
			FirstGal = atoi(msg[42].answer);
			if (FirstGal < 0)
			{
				p_info(PI_ELOG, "ERROR - First galley number (%s) less than 0, defaulting to all galleys.\n",
						   msg[42].answer);
				FirstGal = 0;
				LastGal = 1000000;
			}
			else if ( msg[43].answer[0])
			{
				LastGal = atoi(msg[43].answer);
				if (LastGal < FirstGal)
				{
					p_info(PI_ELOG, "ERROR - Last galley number (%s) less than first galley number (%s), defaulting to all galleys.\n",
						   msg[43].answer, msg[42].answer);
					FirstGal = 0;
					LastGal = 1000000;
				}
			}
			else
			{
				FirstGal = 0;
				LastGal = 1000000;
			}
		}						/* end if(msg[42].answer[0])  */
		if (!FirstGal && !LastGal)
			LastGal = 1000000;
	}							/* end if(!FileType) */
	if ( msg[45].answer[0])
		Proof = atoi(msg[45].answer);
	else
		Proof = 0;
	if ( KeyOutputType && msg[46].answer[0])
		Plates = (uint32 )atoi(msg[46].answer);
	else
		Plates = 0xffffffff;
	if ( FileType && msg[47].answer[0] && !LYPrintFlag)
		SupressBlankPages = atoi(msg[47].answer);
	else
		SupressBlankPages = 0;
	if ( msg[48].answer[0])
		Neg = atoi(msg[48].answer);
	else
		Neg = 0;
	if ( msg[49].answer[0])
		Mirror = atoi(msg[49].answer);
	else
		Mirror = 0;
	if ( msg[50].answer[0] && !FileType) /* only allowed in galleys */
		BcEcFlag = atoi(msg[50].answer); /* non-zero: use BcEc */
	else
		BcEcFlag = 0;			/* Ignore BcEc for this job */
	if ( msg[51].answer[0])
	{
		EditTraceFlag = atoi(msg[51].answer);
		if ( (EditTraceFlag < 0) || (EditTraceFlag > 3) )
		{
			p_info(PI_ELOG, "ERROR - EditTrace '%s' invalid, defaulting to none.\n",
				   msg[51].answer);
			EditTraceFlag = 0;
		}
	}
	else
		EditTraceFlag = 0;
	EditTraceStarted = 0;
	if ( EditTraceFlag)
	{							/* only read parameters if needed. */
		if ( msg[52].answer[0])
			CurInsOffset = (int )(atof(msg[52].answer) * VerticalBase);
		else
			CurInsOffset = 1.25 * VerticalBase;
		if ( msg[53].answer[0])
			PreInsOffset = (int )(atof(msg[53].answer) * VerticalBase);
		else
			PreInsOffset = 2.75 * VerticalBase;
		if ( msg[54].answer[0])
			CurDelOffset = (int )(atof(msg[54].answer) * VerticalBase);
		else
			CurDelOffset = 2.25 * VerticalBase;
		if ( msg[55].answer[0])
			PreDelOffset = (int )(atof(msg[55].answer) * VerticalBase);
		else
			PreDelOffset = 3.75 * VerticalBase;
		if ( msg[56].answer[0])
		{
			CurInsClr = atoi(msg[56].answer);
			if(CurInsClr < -1)
				CurInsClr = -1;
		}
		else
			CurInsClr = 1;
		if ( msg[57].answer[0])
		{
			PreInsClr = atoi(msg[57].answer);
			if(PreInsClr < -1)
				PreInsClr = -1;
		}
		else
			PreInsClr = 1;
		if ( msg[58].answer[0])
		{
			CurDelClr = atoi(msg[58].answer);
			if(CurDelClr < -1)
				CurDelClr = -1;
		}
		else
			CurDelClr = 1;
		if ( msg[59].answer[0])
		{
			PreDelClr = atoi(msg[59].answer);
			if(PreDelClr < -1)
				PreDelClr = -1;
		}
		else
			PreDelClr = 1;
		if ( msg[60].answer[0])
		{
			CurInsWeight = (atof(msg[60].answer)  * VerticalBase);
			if(CurInsWeight <= 0)
				CurInsWeight = 1.; /* 1/10 point in VerticalBase units */
		}
		else
			CurInsWeight = .1 * VerticalBase;
		if ( msg[61].answer[0])
		{
			PreInsWeight = atof(msg[61].answer) * VerticalBase;
			if(PreInsWeight <= 0)
				PreInsWeight = 1.; /* 1/10 point in VerticalBase units */
		}
		else
			PreInsWeight = .1 * VerticalBase;
	}							/* end if(EditTraceFlag) */
	if ( msg[62].answer[0])
	{
		ChapterPage = atoi(msg[62].answer);
		ChapterPageSetFlag = 0;
	}
	else
	{
		ChapterPage = 0;
		ChapterPageSetFlag = 1;
	}
	if ( msg[63].answer[0])
		IncludeScreens = atoi(msg[63].answer);
	else
		IncludeScreens = 1; /* If no keyword, set IncludeScreens to 1 */

	if (msg[64].answer[0])
		LPI = atoi(msg[64].answer);
	else
		LPI = 0; /* Use ColorTable */

    if ( msg[65].answer[0]) /* Number of Copies */
        NumCopies = atoi(msg[65].answer);
    else
        NumCopies = 1;
    if ( msg[66].answer[0]) /* ClipPaths */
		ClipPathFlag = atoi(msg[66].answer);
	else
		ClipPathFlag = 0;
	if ( msg[67].answer[0]) /* Collate */
		CollateFlag = atoi(msg[67].answer);
	else
		CollateFlag = 0;
	if ( msg[69].answer[0]) /* Page width in inches */
		PageWInch = atoi(msg[69].answer);
	else
		PageWInch = 0;
	if ( msg[70].answer[0]) /* Page Depth in inches */
		PageDInch = atoi(msg[70].answer);
	else
		PageDInch = 0;
	if ( msg[71].answer[0]) /* Switch: Do galley breaks in pp/pq extracts */
		DoExtractBreaks = atoi(msg[71].answer);
	else
		DoExtractBreaks = 1;
	if ( msg[120].answer[0]) /* 1= Distill to PDF (done by .psh script)
								0= don't   */
		DistillFlag = atoi(msg[120].answer);
	else
		DistillFlag = 0;
	if ( msg[121].answer[0] && lnumbers)
							/* += Put out line stats w/ line# (galley md only)
								5+=Do so, and use val as their pt-size.
								0= don't   */
		LineStats = atoi(msg[121].answer);
	else
		LineStats = 0;
	return(0);
}								/* end function */
/**********************************************************************/
/***  Function to read printer name from .psh file  Bug 367p  *********/
/**********************************************************************/
static void get_prt_name(void)
{
	int i;
	int j;
	int k;
	int m;
	int hit;
	char PrtFile[70];
	char psh_buff[256];
	Pfd psh_fd;

	hit = 0;
	PrtNam[0] = 0;
	strcpy (PrtFile, "/usr/local/bin/");
	strcat (PrtFile, PrinterName);
	strcat (PrtFile, ".psh");

	psh_fd = p_open(NO_STRING, OTHER_FILE, NO_STRING, PrtFile, "r"); 
	if (!psh_fd)
	{
		printf ("\n%%%%ERROR -- pp Unable to open printer script file '%s'.\n", PrtFile);
		return;
	}
	for (;;)		/* loop thru all lines in .psh file searching for printer name */
	{
		if (p_fgets (psh_buff, 256, psh_fd) == psh_buff)	/* process a valid line */
		{
			m = 0;
			for (i = 0; i < 257; ++i)
			{
				if ((psh_buff[i] == '\n') || (psh_buff[i] == '#'))  
					break;
				if (!(strncmp (&psh_buff[i], "lp ", 3)))
				{
					for (j = i + 3; j < 257; j++)
					{
						if (psh_buff[j] == '\n')
						{
							strcpy( PrtNam, "default");
							hit = 1;
							break;
						}
						if (!(strncmp (&psh_buff[j], "-d", 2)))
						{
							for (k = j + 2; k < 257; ++k)
							{
								PrtNam[m++] = psh_buff[k];
								if (psh_buff[k] == ' ')
								{
									PrtNam[--m] = 0;
									hit = 1;
									break;
								}
							}
						}
					if (hit)
						break;
					}
				}
				if (hit)
					break;
			}
		}
		else
			break;						/* error, assume EOF */
	}
	p_close (psh_fd);
}	/**********************************end of function  get_prt_name*****************/ 

/*********end file************/

