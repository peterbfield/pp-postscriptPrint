#include <stdio.h>                              /* so can use Pfd type */
#include <ctype.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include "penta.h"
#include "noproto.h"
#include "p_lib.h"
#include "traces.h"
#include "psjob.h"
#include "lpm.h"
#include "prototypes.h"


int point_size;
int leading_size;

void addchar(p,c)
   char                 **p;
   char                 c;
{
   if (*p)
   {
      **p = c;
      (*p)++;
      **p = '\0';
   }

}

void addstring(p,cptr,l)
   char                 **p,*cptr;
   int                  l;
{
   char         *c = cptr;

   if (*p == '\0')
        return;

   while (l--)
   {
      addchar(p,*c);
      c++;
   }
}

void ProcessLink(nargs)
int16 nargs;
{
int ii, iint;
int bmargctr=1;
int16 pdfkwrd;
int16 pdfkarg;
int16 pdfchar;
char pdfword[32];
char pdfstring[128];
char c1;
char c2;
float fcolor;

		point_size = fo_line_def.SolPointSize / 20;

		pdflinkctr++;
                /* initialize values */
		strcpy(pdf_link[pdflinkctr].ocolor,job_link.ocolor);
		pdf_link[pdflinkctr].ndest[0] = '\0';
		/* Can be set globally through pdf menu */
		pdf_link[pdflinkctr].btype = job_link.btype;
		pdf_link[pdflinkctr].bcolor = job_link.bcolor;
		pdf_link[pdflinkctr].weight = job_link.weight;
		pdf_link[pdflinkctr].dashlen = job_link.dashlen;
		pdf_link[pdflinkctr].dashgap = job_link.dashgap;
		pdf_link[pdflinkctr].radius = job_link.radius;

		m_fprintf("currentpoint\n");
		memset(pdfstring,0,sizeof(pdfstring));
		sprintf(pdfstring,"/lSy%d exch def /lSx%d exch def\n",pdflinkctr,pdflinkctr);
		m_fprintf(pdfstring);

		while (bmargctr < nargs) {
			pdfkwrd = foget(); bmargctr++;

			switch (pdfkwrd) {
						/* Only one of these six alternates will exist:  */
			case 5:                              /* dest  */
			case 20:							/* file  */
			case 21:							/* macfile  */
			case 22:							/* unixfile  */
			case 23:							/* winfile  */
			case 24:							/* uri  */
				pdf_link[pdflinkctr].desttype = pdfkwrd;
				pdfkarg = foget(); bmargctr++;
				pdf_link[pdflinkctr].ndest[0] = '\0';
				for (ii=1; ii<= pdfkarg; ii++) {
					pdfchar=foget(); bmargctr++;
					c1 = (pdfchar >> 8) & 0xFF;
					c2 =  pdfchar       & 0xFF;
					sprintf(pdfword,"%c%c", c1, c2);
					strcat(pdf_link[pdflinkctr].ndest,pdfword);
				}
				break;
			case 25:							/* params  */
				pdfkarg = foget(); bmargctr++;
				pdf_link[pdflinkctr].destarg[0] = '\0';
				for (ii=1; ii<= pdfkarg; ii++) {
					pdfchar=foget(); bmargctr++;
					c1 = (pdfchar >> 8) & 0xFF;
					c2 =  pdfchar       & 0xFF;
					sprintf(pdfword,"%c%c", c1, c2);
					strcat(pdf_link[pdflinkctr].destarg,pdfword);
				}
				break;
			case 11:                              /* btype */
				pdf_link[pdflinkctr].btype = foget(); bmargctr++;
				break;
			case 10:                              /* bcolor */
				pdf_link[pdflinkctr].ocolor[0] = '\0';
				for (ii=1; ii <= 3; ii++) {
					iint = foget(); bmargctr++;
					fcolor = iint/255;
					sprintf(pdfword," %.2f",fcolor);
					strcat(pdf_link[pdflinkctr].ocolor,pdfword);
				}
				break;
			default:
				fprintf(stderr,"Warning: Invalid keyword %d found\n", pdfkwrd);
				break;

			}		/* end  switch (pdfkwrd)  */
		}		/* end  while (bmargctr < nargs)  */
}


void ProcessEnd() 
{
struct URLm {
	char match[10];
	int mlen;
} URLmatches[8] = {			/* Any pk,link strings that start with any of these
								prefixes should gen PDF /Action /URI command,
								rather than normal /Dest command:  */
	{"www.",		4},		/* Web link (default form of http://www.)  */
	{"http://",		7},		/* Hypertext link.  */
	{"https://",	8},		/* Secure hypertext link.  */
	{"ftp://",		6},		/* ftp link.  */
	{"mms://",		6},		/* MS Media Services link.  */
	{"rtsp://",		7},		/* Real-time server link.  */
	{"mailto:",		7},		/* E-mail link.  */
	{"",			0}};	/* (NOTE: Last list entry MUST always be mlen=0)  */
struct URLm *URLmptr;
char pdfstring[512];
int ii;

if (pdflinkctr < 1)			/* If [pk,end] is unmatched with an earlier [pk,link...], */
	return;					/*  it's probably top of 1st page of a subset of the unit.
								Just return, preceding piece of link won't be marked.  */

m_fprintf("currentpoint\n");
m_fprintf("/lEy%d exch def /lEx%d exch def\n", pdflinkctr, pdflinkctr);
m_fprintf("[ /Rect [ lSx%d 2 sub lSy%d 4 sub lEx%d 2 add lEy%d %d add ]\n",
	pdflinkctr, pdflinkctr, pdflinkctr, pdflinkctr, point_size);

/* Output Links */
strcpy (pdfstring, pdf_link[pdflinkctr].ndest);

/* BRANCH on which of six alternate DEST types were given.  */
switch (pdf_link[pdflinkctr].desttype) {
	
  case 5:						/* The arg was: dest=....   This was the original and
									still supports alternate actions based on how
									the string is prefixed.  */
	URLmptr = URLmatches;		/* Point to URLmatches list  */
			
	do {						/* Loop to see if pdfstring has a listed prefix:  */
		if (!strncasecmp (pdfstring, URLmptr->match, URLmptr->mlen))
			break;				/* Yes, break & act on it.  */
	} while ((++URLmptr)->mlen);
	/* printf("Out at string %s.\n",  URLmptr->match); */
	if (URLmptr->mlen)			/* We matched a URI prefix, so gen the action:  */
		m_fprintf ("\t/Action << /Subtype /URI /URI\n\t (%s) >> \n", pdfstring);
	
	else if (!strncasecmp (pdfstring, "file:", 5))
	{
		ii = 5;					/* Normally strip "file:".
									But if formal URL syntax was used,  */
		if (!strncasecmp (pdfstring, "file://", 7))
			ii = 7;				/*  strip full "file://".  */
		m_fprintf ("\t/Action /Launch /File (%s)\n", pdfstring+ii);
	}
	else						/* Normal link to destination within this doc.  */
		m_fprintf("\t/Dest /%s\n", pdf_link[pdflinkctr].ndest);
	break;

  case 20:						/* The arg was: file=  */
	m_fprintf ("\t/Action /Launch /File (%s)\n", pdfstring);
	break;
  case 21:						/* The arg was: macfile=  */
	m_fprintf ("\t/Action /Launch /MacFile (%s)\n", pdfstring);
	break;
  case 22:						/* The arg was: unixfile=  */
	m_fprintf ("\t/Action /Launch /UnixFile (%s)\n", pdfstring);
	break;
  case 23:						/* The arg was: winfile=  */
  								/* This puts out PDF-format string directly; distiller
									accepts it fine.  */
	m_fprintf ("\t/A <</S/Launch/Win<</F(%s)", pdfstring);
	if (*(pdf_link[pdflinkctr].destarg))
		m_fprintf ("/P(%s)", pdf_link[pdflinkctr].destarg);
	m_fprintf (">>>>\n", pdfstring);
	break;
  case 24:						/* The arg was: uri=  */
	m_fprintf ("\t/Action << /Subtype /URI /URI\n\t (%s) >> \n", pdfstring);
}

switch (pdf_link[pdflinkctr].btype) {
	case 1: /* solid line */
		m_fprintf("\t/Border [ %d %d %d ]\n",
			pdf_link[pdflinkctr].radius, pdf_link[pdflinkctr].radius,
			pdf_link[pdflinkctr].weight);
		break;
	case 2: /* dashed line */
		m_fprintf("\t/Border [ %d %d %d [%d %d] ]\n",
			pdf_link[pdflinkctr].radius, pdf_link[pdflinkctr].radius,
			pdf_link[pdflinkctr].weight, pdf_link[pdflinkctr].dashlen,
			pdf_link[pdflinkctr].dashgap);
		break;
	case 3: /* none */
	default:
		m_fprintf("\t/Border [ 0 0 0 ]\n");
		break;
}

if (pdf_link[pdflinkctr].ocolor[0])
	m_fprintf("\t/Color [ %s ]\n", pdf_link[pdflinkctr].ocolor);

m_fprintf("\t/Subtype /Link\n\t/ANN\n pdfmark\n");

pdflinkctr--;
}


void ProcessNote(nargs)
int16 nargs;
{
int ii;
int bmargctr=1;
int16 pdfkwrd;
int16 pdfkarg;
int16 pdfchar;
char pdfword[32];
char c1;
char c2;
char pdf_note_text[8192];
char pdf_note_title[32];
float fcolor;
char pdfstring[8208];
char *pdfptr;
char *titleptr;


                /* initialize values */
		memset(pdf_note_text,0,sizeof(pdf_note_text));
		memset(pdf_note_title,0,sizeof(pdf_note_title));

		m_fprintf("currentpoint\n");
		m_fprintf("/nSy exch def /nSx exch def\n");

                while (bmargctr < nargs) {
                pdfkwrd = foget(); bmargctr++;

		
                switch (pdfkwrd) {
                   case 1:                              /* title  */
                        pdfkarg = foget(); bmargctr++;
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_note_title,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_note_title,pdfword);
                        }
                        break;
                   case 7:                              /* text  */
                        pdfkarg = foget(); bmargctr++;
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_note_text,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_note_text,pdfword);
                        }
                        break;
                   case 2:                              /* disp */
                        pdf_note_disp = foget(); bmargctr++;
                        break;
                   case 8:                              /* pos */
                        pdf_note_pos = foget(); bmargctr++; 
                        break;
                   case 9:                              /* size */
	                        pdf_note_length = foget(); bmargctr++; 
				pdf_note_depth=foget(); bmargctr++;
				
                        break;
                   case 10:                              /* bg */
			pdf_note_ocolor[0] = '\0';
                        for (ii=1; ii <= 3; ii++) {
                           pdf_note_color = foget(); bmargctr++;
			   fcolor = pdf_note_color/255;
			   memset(pdfword,0,sizeof(pdfword));
			   sprintf(pdfword," %.2f",fcolor);
			   strcat(pdf_note_ocolor,pdfword);
			}
			break;
                   default:
                        fprintf(stderr,"Warning: Invalid keyword %d found\n", pdfkwrd);
                        break;
                }

	} /* end while */

	/* Output Note Now */
	m_fprintf("[ /Contents (");
	m_fprintf(pdf_note_text);
	m_fprintf(")\n");

	memset(pdfstring,0,sizeof(pdfstring));
	if (pdf_note_pos == 2) {
	/* top right */
		sprintf(pdfstring,"\t/Rect [ nSx %d sub nSy nSx nSy %d sub ]\n", pdf_note_length, pdf_note_depth);
	} else if (pdf_note_pos == 3) {
	/* bottom left */
		sprintf(pdfstring,"\t/Rect [ nSx nSy %d add nSx %d add nSy ]\n", pdf_note_length, pdf_note_depth);
	} else if (pdf_note_pos == 4) {
	/* bottom right */
		sprintf(pdfstring,"\t/Rect [ nSx %d sub nSy %d add nSx nSy ]\n", pdf_note_length, pdf_note_depth);
	} else if (pdf_note_pos == 5) {
	/* center */
		sprintf(pdfstring,"\t/Rect [ nSx %d sub nSy %d add nSx %d add nSy %d sub ]\n", pdf_note_length/2, pdf_note_depth/2, pdf_note_depth/2,pdf_note_depth/2);
	} else {
	/* default - top left */
		sprintf(pdfstring,"\t/Rect [ nSx nSy nSx %d add nSy %d sub ]\n", pdf_note_length, pdf_note_depth);
	}
	m_fprintf(pdfstring);

	if (pdf_note_title[0] != '\0') {
	   memset(pdfstring,0,sizeof(pdfstring));
	   sprintf(pdfstring,"\t /Title (%s)\n", pdf_note_title);
	   titleptr=pdfstring;
	   pdfptr=TranslateTable(titleptr);
	   m_fprintf(pdfptr);
	}

	if (pdf_note_disp == 1)
	   m_fprintf("\t /Open true\n");
	else
	   m_fprintf("\t /Open false\n");

	if (pdf_note_ocolor != '\0') {
	   memset(pdfstring,0,sizeof(pdfstring));
	   sprintf(pdfstring,"\t/Color [ %s ]\n", pdf_note_ocolor);
	   m_fprintf(pdfstring);
	}

	m_fprintf("\t/ANN\npdfmark\n");

	/* Add Border Later */
}
	



void ProcessBookMarks(nargs)
int16 nargs;
{
int ii;
int bmargctr=1;
int adduid;
int16 pdfkwrd;
int16 pdfkarg;
int16 pdfchar;
char viewstring[40];
char *op;
int dpos=0;
char dposstring[24];
char titlestring[8193];

                if (++bmctr >= bmtop)	/* Time to alloc static mem for bookmarks?  */
				{						/* Yes. Hunks of 500, never shrink between jobs. */
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "Adding 500 bookmarks mem\n");
#endif
					if (bmtop == 0)		/* (note: bmctr starts at -1 each job)  */
						pdf_bookmark = (pdf_bookmarks *)p_alloc (500*sizeof(pdf_bookmarks));
					else
						pdf_bookmark = (pdf_bookmarks *)p_remalloc ((char *)pdf_bookmark,
							 bmtop     *sizeof(pdf_bookmarks),
							(bmtop+500)*sizeof(pdf_bookmarks));
					bmtop += 500;		/* 0, 500, 1000, 1500, etc.  */
				}

                /* initialize values */
                pdf_bookmark[bmctr].title[0] = '\0';
                pdf_bookmark[bmctr].disp = 0;
                pdf_bookmark[bmctr].id[0] = '\0';
                pdf_bookmark[bmctr].rid[0] = '\0';
                pdf_bookmark[bmctr].dest[0] = '\0';
                pdf_bookmark[bmctr].view = 0;

                while (bmargctr < nargs) {
                pdfkwrd = foget(); bmargctr++;

                switch (pdfkwrd) {
                   case 1:                              /* title  */
                        pdfkarg = foget(); bmargctr++;
						op = titlestring;
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           *op++ = (pdfchar >> 8) & 0xFF;
                           *op++ = pdfchar & 0xFF;
                        }
						*op = 0;			/* null-term  */
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "  Title=%s.\n", titlestring);
#endif
						strcpy (pdf_bookmark[bmctr].title, 
								TranslateTable(titlestring));
                        break;
                   case 2:                              /* disp */
                        pdf_bookmark[bmctr].disp = foget(); bmargctr++;
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "  Disp=%d.\n", pdf_bookmark[bmctr].disp);
#endif
                        break;
                   case 3:                              /* id */
                        /* Unique ID */
                        pdfkarg = foget(); bmargctr++;
						op = pdf_bookmark[bmctr].id;
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           *op++ = (pdfchar >> 8) & 0xFF;
                           *op++ = pdfchar & 0xFF;
                        }
						*op = 0;			/* null-term  */
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "  Id=%s.\n", pdf_bookmark[bmctr].id);
#endif

                        /* Look up in ID table */
                        adduid=1;
                        for (ii = 0; ii <= uidctr; ii++) {
                           if (strcmp(uidtable[ii].id,pdf_bookmark[bmctr].id) == 0) {
                                /* Already there */
                                fprintf(stderr,"Warning: BookMark ID %s already exists\n", pdf_bookmark[bmctr].id);
                                adduid=0;
                                break;
                           }
                        }
						if (adduid) {
							if (++uidctr >= uidtop)	/* Time to alloc static mem for uids? */
							{	/* Yes. Hunks of 500, never shrink between jobs.  */
								/*   (note: uidctr starts at -1 each job)  */
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "Adding 500 uids mem\n");
#endif
								if (uidtop == 0)
									uidtable = (uidtables *)p_alloc (500*sizeof(uidtables));
								else
									uidtable = (uidtables *)p_remalloc ((char *)uidtable,
										 uidtop     *sizeof(uidtables),
										(uidtop+500)*sizeof(uidtables));
								uidtop += 500;		/* 0, 500, 1000, 1500, etc.  */
							}
							strcpy(uidtable[uidctr].id,pdf_bookmark[bmctr].id);
							uidtable[uidctr].usage=1;
						}
                        adduid=0;
                        break;
                  case 4:                              /* rid */
                        pdfkarg = foget(); bmargctr++;
						op = pdf_bookmark[bmctr].rid;
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           *op++ = (pdfchar >> 8) & 0xFF;
                           *op++ = pdfchar & 0xFF;
                        }
						*op = 0;			/* null-term  */
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "  Rid=%s.\n", pdf_bookmark[bmctr].rid);
#endif

                        /* Points to another bookmark - find parent and increment */
                        adduid=1;
                        for (ii = 0; ii <= uidctr; ii++) {
                           if (strcmp(uidtable[ii].id,pdf_bookmark[bmctr].rid) == 0) {
                                uidtable[ii].usage++;
                                adduid=0;
                                break;
                           }
                        }
                        if (adduid) {
                           fprintf(stderr,"Warning: BookMark RID %s cannot reference defined ID\n", pdf_bookmark[bmctr].
rid);
                        }
                        adduid=0;
                        break;
                   case 5:                              /* dest */
                        pdfkarg = foget(); bmargctr++;
						op = pdf_bookmark[bmctr].dest;
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           *op++ = (pdfchar >> 8) & 0xFF;
                           *op++ = pdfchar & 0xFF;
                        }
						*op = 0;			/* null-term  */
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "  Dest=%s.\n", pdf_bookmark[bmctr].dest);
#endif
                        break;
		   case 6:
                        pdf_bookmark[bmctr].view = foget(); bmargctr++;
#ifdef TRACE
              if (debugger_trace)
                  p_info(PI_TRACE, "  View=%d.\n", pdf_bookmark[bmctr].view);
#endif
                        break;
                   case 19:
                        dpos=foget(); bmargctr++;
                        break;

                   default:
                        fprintf(stderr,"Warning: Invalid keyword %d found\n", pdfkwrd);
                        break;
                } /* end switch */

                } /* End while bmargctr */



	leading_size=fo_line_def.SolPointSize / 20;
        memset(dposstring,0,sizeof(dposstring));
        memset(viewstring,0,sizeof(viewstring));
        switch (pdf_bookmark[bmctr].view) {
                     case 1:
                              sprintf(viewstring,"/View [/Fit]");
                              break;
                     case 2:
                              if (dpos == -1) {
                                m_fprintf("currentpoint /vSy exch def /vSx exch def\n");
                                sprintf(dposstring,"vSy %d add " , leading_size);
                              } else {
                                sprintf(dposstring,"%d", cropboxd);
                              }
                              sprintf(viewstring,"/View [/FitH %s]",dposstring);
                              break;
                     case 3:
                              if (dpos == -1) {
                                m_fprintf("currentpoint /vSy exch def /vSx exch def\n");
                                sprintf(dposstring,"vSy %d add " , leading_size);
                              } else {
                                sprintf(dposstring,"%d", cropboxd);
                              }
                              sprintf(viewstring,"/View [/FitV %s]",dposstring);
                              break;
                     default:
                              if (dpos == -1) {
                                m_fprintf("currentpoint /vSy exch def /vSx exch def\n");
                                sprintf(dposstring,"vSx vSy %d add ", leading_size);
                              } else {
                                sprintf(dposstring,"null null");
                              }
                              sprintf(viewstring,"/View [/XYZ %s 0]",dposstring);
                              break;
        }


                       /* Output named designation */
                        if (pdf_bookmark[bmctr].dest[0] == '\0') {
			   if (pdf_bookmark[bmctr].view != 0) {
                                m_fprintf("[/Dest /BM%d %s /DEST pdfmark\n",bmctr,viewstring);
			   } else {
                                m_fprintf("[/Dest /BM%d /View [/XYZ null null 0] /DEST pdfmark\n",bmctr);
			   }
                        } else {
                                /* Named destination must be defined elsewhere */
                        }
}



void ProcessNDest(nargs)
int16 nargs;
{
int ii;
int bmargctr=1;
int16 pdfkwrd;
int16 pdfkarg;
int16 pdfchar;
char pdfword[2];
char pdftitle[256];
int16 pdfzoom=0;
char viewstring[512];
char c1;
char c2;
int dpos=0;
char dposstring[512];

        while (bmargctr < nargs) {
                pdfkwrd = foget(); bmargctr++;

                switch (pdfkwrd) {
                   case 1:                              /* name  */
                        pdfkarg = foget(); bmargctr++;
                        pdftitle[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdftitle,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdftitle,pdfword);
                        }
                        break;
		   case 6:
			pdfzoom = foget(); bmargctr++;
			viewstring[0] = '\0';
			break;
		   case 19: 
			dpos=foget(); bmargctr++;
			break;
		   default:
                        fprintf(stderr,"Warning: Invalid keyword %d found\n", pdfkwrd);
			break;
		}

	}

	leading_size=fo_line_def.SolPointSize / 20 ;
	memset(dposstring,0,sizeof(dposstring));
	memset(viewstring,0,sizeof(viewstring));
        switch (pdfzoom) {
                     case 1:
                              sprintf(viewstring,"/View [/Fit]");
                              break;
                     case 2:
			      if (dpos == -1) {
				m_fprintf("currentpoint /vSy exch def /vSx exch def\n");
				sprintf(dposstring,"vSy %d add " , leading_size);
			      } else {
				sprintf(dposstring,"%d", cropboxd);
			      }
                              sprintf(viewstring,"/View [/FitH %s]",dposstring);
                              break;
                     case 3:
			      if (dpos == -1) {
				m_fprintf("currentpoint /vSy exch def /vSx exch def\n");
				sprintf(dposstring,"vSy %d add " , leading_size);
			      } else {
				sprintf(dposstring,"%d", cropboxd);
			      }
                              sprintf(viewstring,"/View [/FitV %s]",dposstring);
                              break;
                     default:
			      if (dpos == -1) {
				m_fprintf("currentpoint /vSy exch def /vSx exch def\n");
				sprintf(dposstring,"vSx vSy %d add ",  leading_size);
			      } else {
				sprintf(dposstring,"null null");
			      }
                              sprintf(viewstring,"/View [/XYZ %s .%d]",dposstring,pdfzoom);
                              break;
        }

	if (viewstring[0] != '\0')
        	m_fprintf("[/Dest /%s %s /DEST pdfmark\n",pdftitle,viewstring);
	else
        	m_fprintf("[/Dest /%s /View [/XYZ null null 0] /DEST pdfmark\n",pdftitle);
}

void ProcessInfo(nargs)
int16 nargs;
{
int ii;
int bmargctr=1;
int16 pdfkwrd;
int16 pdfkarg;
int16 pdfchar;
char pdfword[2];

char c1;
char c2;


        while (bmargctr < nargs) {
                pdfkwrd = foget(); bmargctr++;

		/* No creator in pk command yet */
                switch (pdfkwrd) {
                   case 1:                              /* name  */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.title[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.title,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.title,pdfword);
                        }
                        break;
		   case 13:				/* subject */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.subj[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.subj,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.subj,pdfword);
                        }
                        break;
		   case 14:				/* keyword */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.keyw[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.keyw,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.keyw,pdfword);
                        }
                        break;
		   case 15:				/* author */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.auth[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.auth,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.auth,pdfword);
                        }
                        break;
		   case 16:				/* date 1 */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.date1[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.date1,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.date1,pdfword);
                        }
                        break;
		   case 17:				/* date 2 */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.date2[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.date2,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.date2,pdfword);
                        }
                        break;
		   case 18:				/* creator */
                        pdfkarg = foget(); bmargctr++;
                        pdf_info.creator[0] = '\0';
                        for (ii=1; ii<= pdfkarg; ii++) {
                           pdfchar=foget(); bmargctr++;
                           c2 = pdfchar & 0xFF;
                           c1 = (pdfchar >> 8) & 0xFF;
                           sprintf(pdfword,"%c",c1);
                           strcat(pdf_info.creator,pdfword);
                           sprintf(pdfword,"%c",c2);
                           strcat(pdf_info.creator,pdfword);
                        }
                        break;
		   default:
                        fprintf(stderr,"Warning: Invalid keyword %d found\n", pdfkwrd);
			break;
		}

	}
}


void ProcessPdfMarks(nargs) 
int16 nargs;
{

int16 pdfsubcmd;

	pdfsubcmd = foget();

	switch (pdfsubcmd) {
		case 1: 
			ProcessBookMarks(nargs); /* bmark */
			break;
		case 2:
			ProcessEnd();
			break;
		case 3:
			ProcessInfo(nargs);	/* info */
			break;
		case 4:
			ProcessLink(nargs); 	/* link */
			break;
		case 5:
			ProcessNDest(nargs);	/* ndest */
			break;
		case 6:
			ProcessNote(nargs);	/* note */
			break;
		default:
			break;
	}
}

int FindNumChildren(pbookmark)
char *pbookmark;
{
   int jj=0;
   int res;

   res=1;
   for (jj = 0; jj <= uidctr; jj++) {
      if (strcmp(pbookmark,uidtable[jj].id) == 0)  {
         return(uidtable[jj].usage);
	 break;
      }
   }
   return(res);
}

void OutputChildren(bookmark)
char *bookmark;
{
   int jj=0;
   int numchildren=0;
   char *bmptr;


   for (jj = 0; jj <= bmctr; jj++) {
        /* Look for everything with RID of current ID */
	if ( (strcmp(pdf_bookmark[jj].rid, bookmark) == 0) ){
	   if (bmlevel < pdf_output_lev) {
		numchildren=FindNumChildren(pdf_bookmark[jj].id);
		bmlevel++;
		m_fprintf("\t[/Count ");
                if (pdf_bookmark[jj].disp != 1)
                        m_fprintf("-");
		bmptr = pdf_bookmark[jj].title;
		if (pdf_bookmark[jj].dest[0] == '\0')
		   m_fprintf("%d /Dest /BM%d /Title (%s) /OUT pdfmark\n", 
			numchildren-1, jj, bmptr);
		else
		   m_fprintf("%d /Dest /%s /Title (%s) /OUT pdfmark\n", 
			numchildren-1, pdf_bookmark[jj].dest, bmptr);
		if (numchildren != 1 && (pdf_bookmark[jj].id[0] != '\0') ) {
			OutputChildren(pdf_bookmark[jj].id);
			bmlevel--;
		} else {
			bmlevel--;
		}
		
	   } else {
		fprintf(stderr,"Warning: Bookmark levels exceed maximum %d\n", pdf_output_lev);
	   } 
	} 
   }
}

void OutputInfo()
{
	m_fprintf("[");
	if (pdf_info.auth[0] != '\0')
		m_fprintf(" /Author (%s)\n", pdf_info.auth);
	if (pdf_info.creator[0] != '\0')
		m_fprintf(" /Creator (%s)\n", pdf_info.creator);
	if (pdf_info.date1[0] != '\0')
		m_fprintf(" /CreationDate (%s)\n", pdf_info.date1);
	if (pdf_info.title[0] != '\0')
		m_fprintf(" /Title (%s)\n", pdf_info.title);
	if (pdf_info.subj[0] != '\0')
		m_fprintf(" /Subject (%s)\n", pdf_info.subj);
	if (pdf_info.keyw[0] != '\0')
		m_fprintf(" /Keywords (%s)\n", pdf_info.keyw);
	if (pdf_info.date2[0] != '\0')
		m_fprintf(" /ModDate (%s)\n", pdf_info.date2);
	m_fprintf("/DOCINFO pdfmark\n");
}

void OutputBookMarks()
{
   int ii=0;
   int numchildren=0;
   char *bmptr;
#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "OutputBookMarks: BMctr=%d.\n", bmctr);
#endif
   bmlevel=1;
	/* pdf_output_lev set to 9 or changed in pdf menu */
   for (ii=0; ii <= bmctr; ii++) {
#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "  BookMark %d: title=(%s) id=(%s) rid=(%s).\n",
			ii, pdf_bookmark[ii].title, pdf_bookmark[ii].id, pdf_bookmark[ii].rid);
#endif
	/* Sort for levels */
	if (pdf_bookmark[ii].rid[0] == '\0')  {
		bmlevel=1; /* Keep track of levels output */
		/* Level 1: Find number of Children */
		numchildren = FindNumChildren(pdf_bookmark[ii].id);

		m_fprintf("[/Count ");
		if (pdf_bookmark[ii].disp != 1)
			m_fprintf("-");

		bmptr = pdf_bookmark[ii].title;
		if (pdf_bookmark[ii].dest[0] == '\0') 
		   m_fprintf("%d /Dest /BM%d /Title (%s) /OUT pdfmark\n", numchildren-1, ii, bmptr);
		else
		   m_fprintf("%d /Dest /%s /Title (%s) /OUT pdfmark\n", numchildren-1, pdf_bookmark[ii].dest, bmptr);

		if ( (pdf_bookmark[ii].id[0] != '\0') ) {
			OutputChildren(pdf_bookmark[ii].id);
		}
	}
   }
}
