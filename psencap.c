#include <stdio.h>				/* so can use Pfd type */
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
#include "frame.h"
#include "rel_data.h"

void OutputBookMarks(void);
void OutputInfo(void);

typedef unsigned char byte;

byte Itoh[] = "\
000102030405060708090a0b0c0d0e0f\
101112131415161718191a1b1c1d1e1f\
202122232425262728292a2b2c2d2e2f\
303132333435363738393a3b3c3d3e3f\
404142434445464748494a4b4c4d4e4f\
505152535455565758595a5b5c5d5e5f\
606162636465666768696a6b6c6d6e6f\
707172737475767778797a7b7c7d7e7f\
808182838485868788898a8b8c8d8e8f\
909192939495969798999a9b9c9d9e9f\
a0a1a2a3a4a5a6a7a8a9aaabacadaeaf\
b0b1b2b3b4b5b6b7b8b9babbbcbdbebf\
c0c1c2c3c4c5c6c7c8c9cacbcccdcecf\
d0d1d2d3d4d5d6d7d8d9dadbdcdddedf\
e0e1e2e3e4e5e6e7e8e9eaebecedeeef\
f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";


static void do_showpage(void);
void nlist_fonts();
FILE *FontFile;
int export_ctr=0;

extern int exported;
extern int export_newpiece;
extern char export_name[52];
extern float Ofy;
extern float export_x;
extern float export_y;
extern float export_end_x;
extern float export_end_y;
extern float export_ii;
extern float export_jj;
extern int colorsused[4];
extern char customused[48][32];
extern char cmykcustomused[48][132];
extern char Uname[132];
extern int EpsFlag;
extern int DownLoadFont;
#if LPMfloat
extern char LPMK[];
#else
extern unsigned int LPMK;
#endif
extern int HdrW;	/*** Bug 363p:  Customer selectable page # print location in points */
extern int HdrFNamPrt;  /** Bug 363p:  User definable flag. Used to print/not print file name */
extern int HdrPNamPrt;	/** Bug 367p:  User definable flag.  Used to print/not print the name of the printer  	**/
extern char PrtNam[MAX_NAME];
extern char holdfontname[];
extern char default_color_name[];
extern Pfd colorfd;
extern int MultiPagesOddEvenFlag;
extern int MultiPagesFirstIsOdd;
extern int FirstGal, LastGal;
extern int16 nexty;
extern float ps_lay_page_width, ps_lay_page_depth;
extern float key_scale_x;
extern float key_scale_y;
extern int HelveticaFontNumber;
extern struct plate_name plate_nm[33];
extern int overflow_page_buff[];
extern int Proof;
extern int Neg;
extern int Mirror;
extern int galley_count;
extern int old_freq;
extern double old_angle;
extern char old_func[32];
extern char old_op[5];
extern int firstscreen;
extern int NumCopies;
extern int CollateFlag;
extern int PageWInch, PageDInch;
extern int IncludeScreens;

extern int16 Current_FgColor;	/* Bug 374p - et text not returning to proper color */
extern int16 Current_FgShade;   /* Bug 374p - et text not returning to proper shade */

char Type1Path[132];
char fontneeded[8192];
char fontsupplied[8192];

static int J_minx, J_miny, J_maxx, J_maxy; /* job bounding box */
static F_LIST *J_fonts;			/* first font entry for job */
static F_LIST *P_fonts;			/* first font entry for page */
static F_LIST *P2_fonts;		/* pass 2 MultiPages page first font entry */
static F_LIST *O_fonts;			/* first font entry for overflow */
static char cont_line[] = "%%+ "; /* continuation line text */
static int output_page_count;

WYSIWYG *wn;

byte getbyte( void)
{
        byte cc = getc(FontFile);

        if( feof( FontFile))
        {
                fprintf( stderr, "premature eof or error on input\n");
                return( 1);
        }

        return cc;
}

int downloadfont(char *fontname)
{
        byte cc, type;
        ulong len, cnt;

        fprintf(TrailerName,"%%!PS-AdobeFont-1.0: %s\n",fontname);
        for( ;;)
        {
                cc = getbyte();
                if( cc == 0x80)
                {
                        type = getbyte();
                        if( type == 3)
                                return(0);

                        len = 0;
                        len |= getbyte();
                        len |= ((ulong) getbyte() << 8);
                        len |= ((ulong) getbyte() << 16);
                        len |= ((ulong) getbyte() << 24);

                        if( len == 0)
                        {
                                fprintf( stderr, "type %d encountered with zero length\n", type);
                                return(1);
                        }

                        if( type == 1)
                        {
                                for( cnt = 1; cnt <= len; cnt++)
                                {
                                        cc = getbyte();
                                        putc( cc == '\r'? '\n' : cc, TrailerName);
                                }
                        }
                        else
                        if( type == 2)
                        {
                                for( cnt = 1; cnt <= len; cnt++)
                                {
                                        uint j = (uint) getbyte() << 1;

                                        putc( Itoh[j++], TrailerName);
                                        putc( Itoh[j], TrailerName);

                                        if( !(cnt % 32))
                                                putc( '\n', TrailerName);
                                }
                                putc( '\n', TrailerName);
                        }
                        else
                        {
                                fprintf( stderr, "bad type: %d (%x)\n", type, type);
                                return(1);
                        }
                }
        }
}



/**************************************************************************/

void ini_PAGE(void)
{
	output_page_count = 0;
	DidPage = 0;
	galley_count = 0;
	MasterOutputSwitch = 1;
	J_fonts = 0;
	P_fonts = 0;
	E_fonts = 0;
	P2_fonts = 0;
	O_fonts = 0;
}								/* end function */

/*************	begin page - call at start of a page section *************/

void beg_PAGE(void)
{
	char name_buff[MAX_NAME + 4], *result;
	char prtr_buff[MAX_NAME + 4];
	char nbr_buff[16];
	char basename[52];
	char ext[52];
	char *p;
    int length;
    char time_buf[32];
	int ii;
	uint32 hold_cc_mask;
	uint32 save_cc_mask;
	uint32 loop_temp;
    struct timeval tp;
    struct tm *tm;
	uint32 temp_plate_nbr;
	float temp_fx, temp_fy, temp_fx2, temp_fy3;
	float temp_scale;
	int temp_bounding_box_x;
	int temp_PageW;
	int starting_page_count;
	uint32 force_cc_mask = 0;
	int temp_pp_pg_nbr_x = 0;
    char unit_nbr_buff[16] = {0,0};

	if ( CurrentLineBcEc)
		return;					/* no output started */
	save_cc_mask = cc_mask;
	cc_mask |= active_trap_cc_mask;

	firstscreen = 1;
	old_angle = 0;
	old_freq = 0;
	prtr_buff[0] = 0; 
	strcpy(old_func,"");
	strcpy(old_op,"false");

	if ( !DidPage && !FileType && cc_mask )
	{
		galley_count++;
		if ( (galley_count < FirstGal) || (galley_count > LastGal) )
			MasterOutputSwitch = 0;
		else
			MasterOutputSwitch = 1;
	}
	if( !DidPage && cc_mask )
	{							/* if header isn't done */
		temp_fx = Fx;
		temp_fy = Fy;
		temp_fx2 = Fx2;
		temp_fy3 = Fy3;
		DidPage = 1;
#ifdef TRACE
		if (debugger_trace)
			p_info(PI_TRACE, "******BEGIN PAGE OUTPUT: plates %lo (oct), bgn cnt= %d.\n",
				   cc_mask, output_page_count);
#endif
		starting_page_count = output_page_count;
		if ( MultiPagesUp)
		{
			if ( !MultiPagesOddEvenFlag && (spot_pass == 1) )
				clear_flist(&P_fonts); /* clear list of page fonts	*/
			else if ( !MultiPagesOddEvenFlag )
				clear_flist(&P2_fonts);
/* force page start on odd page if needed or track already started pages. */
			if ( MultiPagesOddEvenFlag )
				force_cc_mask = cc_mask & ~MultiOddPage_cc_mask;
			else
			{					/* pages is on the left */
				if (spot_pass == 1)
					MultiOddPage_cc_mask = 1 & Plates;
				else
					MultiOddPage_cc_mask |= ((cc_mask | active_trap_cc_mask ) &
											 Plates);
			}
		}
		else if (in_overflow || (OverflowOrColorFlag == -1) )
			clear_flist(&O_fonts); /* clear list of overflow fonts	*/
		else
			clear_flist(&P_fonts); /* clear list of page fonts	*/
		hold_cc_mask = cc_mask;
		temp_bounding_box_x = PageW - OffR;
		if ( MultiPagesUp && FileType )	/* But if it's 2-up pages, double width: */
			temp_bounding_box_x += (PageW - OffL - OffR);
		for (ii=0; ii<MAX_CLRS; ii++)
		{
			cc_mask = ((loop_temp=(1<<ii))) & hold_cc_mask;
			if (loop_temp > hold_cc_mask)
				break;
			if ( !cc_mask)
				continue;
			if (!EpsFlag) {
			   if ( !FileType && (spot_pass == 2) ) /* galley pass 2 */
				{
				  m_fprintf("\n%%%%Page: %d\n", ++output_page_count);
				} 
			   else
			   {					/* galley pass 1 and pages */
				
				if ( !MultiPagesUp || (force_cc_mask & cc_mask) ||
					 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd )
					{
						m_fprintf("\n%%%%Page: %d\n", ++output_page_count);
						m_fprintf("\n%%PSI_Folio: %d\n", PageNo1);
						if ( ChapterPage && ChapterPageSetFlag)
							m_fprintf("%%PSI_SecondaryFolio: %d\n", ChapterPage);
					}
			   }
			}
		}						/* end for(ii=0;ii<MAX_CLRS;ii++) */
		if ( MultiPagesUp && FileType )	/* If it's 2-up pages, double width.  */
			temp_PageW = (2 * PageW) - OffL - OffR;
		else							/* It's 1-up and/or galleys, normal wid.  */
			temp_PageW = PageW;
		if ( force_cc_mask)
			cc_mask = force_cc_mask;
		else
			cc_mask = hold_cc_mask;
		if (setpage_allowed)	
		{						/* print page sizes */
			if ( !MultiPagesUp || 
				 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask )
			{
				m_fprintf("%%%%BeginFeature: *Orientation\n");
				m_fprintf("statusdict begin\n");
				if (Orient)
					m_fprintf("%d %d 0 setpage\n",
							  temp_PageW, PageH); /* Landscape */
				else
					m_fprintf("%d %d 1 setpage\n",
							  temp_PageW, PageH); /* Portrait */
				m_fprintf("end\n");
				m_fprintf("%%%%EndFeature\n");
			}
		}
		if ( !MultiPagesUp || 
			 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask )
		{
			if (!EpsFlag) {
			   m_fprintf("/ENCAPage save def\n"); /* encap start */
			   m_fprintf("%%%%BeginPageSetup\n");
			}
			m_fprintf("Penta_7.0 begin\n");
			cc_mask = hold_cc_mask;
		}
		if ( (Neg & !Mirror ) && 
			 (!MultiPagesUp || 
			  !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask ) )
			m_fprintf("0 0 translate 1 1 scale NEG\n"); /* negatives */
		else if ( Mirror && 
				  (!MultiPagesUp || 
				   !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask ) )
		{						/* Mirror */
			if (setpage_allowed && Orient)
				m_fprintf("0 %d translate 1 -1 scale\n", PageW); /* transpose*/
			else
				m_fprintf("%d 0 translate -1 1 scale\n", PageW);
			if ( Neg )
				m_fprintf(" NEG\n"); /* negative & mirror */
			else
				m_fprintf(" \n"); /* mirror */
		}
		if ( !MultiPagesUp || 
			 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask )
				m_fprintf("%d %d translate\n",
						  OffL, OffB); /* origin to LL corner of image area */
		if ( !setpage_allowed && Orient)
		{
			if ( !MultiPagesUp || 
				 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask )
			{					/* if Landscape, xlate and rotate */
				m_fprintf("%d %d translate\n", 0, PageH - OffB - OffT);
				m_fprintf("%d rotate\n", -Orient);
			}
		}
		if (ScaleFactorX && !ReportFlag &&
			(!MultiPagesUp || 
			 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask ) )
		{						/* If given, apply horiz/vert scale to page. */
			temp_scale = 1.0 - ScaleFactorY;
			if (KeyScale != -1)
				temp_scale = 1.0 - (ScaleFactorY / key_scale_y);
			if ( !Orient)
			{					/* portrait */
				m_fprintf("%f %f translate\n", /* move to top of page */
						  0., (temp_scale) * (PageH - OffT - OffB) );
				m_fprintf("%f %f scale\n", ScaleFactorX, ScaleFactorY);
			}
			else				
			{					/* landscape */
				m_fprintf("%f %f translate\n", /* move to top of page */
						  0., (temp_scale) * (PageW - OffL - OffR) );
				m_fprintf("%f %f scale\n", ScaleFactorX, ScaleFactorY);
			}
		}
		cc_mask = hold_cc_mask;
		if ( MultiPagesUp && MultiPagesOddEvenFlag )
		{
			if (FileType)		/* odd-numbered page xlate */
				m_fprintf("%f 0 translate\n", ps_lay_page_width/2);
			else
				m_fprintf("%d 0 translate\n", PageW/2);
		}
		else
			PaintOnPage = 0;



		/* If default-pg Crop Box defined at PDF menu, and doing pages, 
			and didn't draw crop box at job start, then:  */
		if (pdf_crop_dw && pdf_crop_dd &&
			FileType && !CropBoxFlag ) {

			int pgnum;
								/* Get correct crop values for even vs. odd,
									chapter page vs. default page.  */
			pgnum = FirstPage + output_page_count -1;
			pgnum = pgnum % 2;
			if ((output_page_count == 1) &&
				pdf_crop_cw && pdf_crop_cd) {
				/* Chapter page */
				cropboxw = pdf_crop_cw;
				cropboxd = pdf_crop_cd;
				switch (pgnum) {
					case 0: /* Odd */
					default:
						pdfxoff=pdf_crop_cox;
						pdfyoff=pdf_crop_coy;
						break;
					case 1: /* Even */
						pdfxoff=pdf_crop_cex;
						pdfyoff=pdf_crop_cey;
						break;

				}
			} else {
				/* Other pages */
				cropboxw = pdf_crop_dw;
				cropboxd = pdf_crop_dd;
				switch (pgnum) {
					case 0: /* Odd */
					default:
						pdfxoff=pdf_crop_dox;
						pdfyoff=pdf_crop_doy;
						break;
					case 1: /* Even */
						pdfxoff=pdf_crop_dex;
						pdfyoff=pdf_crop_dey;
						break;
				}
			}
								/* Put out crop box for this page.  */
			cbllx = pdfxoff;
			cblly = (ltrimd - (cropboxd + pdfyoff)) ;
			cburx = pdfxoff + cropboxw;
			cbury = ltrimd - pdfyoff;
			m_fprintf("[ /CropBox [ %d %d %d %d ]\n\t/PAGE pdfmark\n", cbllx, cblly, cburx, cbury);

		}		/* Done PDF crop-box for this page.  */
		if ( force_cc_mask)
			cc_mask = force_cc_mask;
		if ( !MultiPagesUp || 
			 !MultiPagesOddEvenFlag || MultiPagesFirstIsOdd || force_cc_mask )
		{
			m_fprintf("end %%end Penta_7.0\n");
			if (!EpsFlag)
			   m_fprintf("%%%%EndPageSetup\n");
			m_fprintf("Penta_7.0 begin\n");
			if ( !KeyOutputType)
				m_fprintf("%%PSI_Composite\n");
			else
			{					/* separations */
				hold_cc_mask = cc_mask;
				for (ii=0; ii<MAX_CLRS; ii++)
				{
					cc_mask = ((loop_temp=(1<<ii))) & hold_cc_mask;
					if (loop_temp > hold_cc_mask)
						break;
					if ( !cc_mask)
						continue;
					if ( plate_nm[ii+1].name[0])
					{
							m_fprintf("%%%%PlateColor: %s\n", plate_nm[ii+1].name);
					}
					else
					{
							m_fprintf("%%%%PlateColor: %d\n", ii+1);
					}
				}				/* end for(ii=0;ii<MAX_CLRS;ii++) */
				cc_mask = hold_cc_mask;
			}					/* end separations */


			if (!EpsFlag)  {
			m_fprintf("%%%%PageFonts: (atend)\n%%%%PageBoundingBox: 0 0 ");

			if ( PageWInch)
				m_fprintf("%d ", temp_bounding_box_x);
			else
				digi_print(temp_bounding_box_x * ScaleFactorX);
			if ( PageDInch)
				m_fprintf("%d\n", PageH);
			else
			{
				digi_print(PageH * ScaleFactorY);
				m_fprintf("\n");
			}
			}
			if (exported) {		/*  Export note re-starting */
				int temp_y, near_top;

				export_ctr++;
				m_fprintf("%%PSI_Export: ");
				digi_print(export_x / HorizontalBase);

				if (debugger_trace)
					printf (
					"At export continuation: export_y=%f, Imageheight=%d, SetY=%d, Ypos=%d, Ofy=%f\n",
						export_y, Imageheight, SetY, Ypos, Ofy);
				near_top = Imageheight/5;
				if (export_y)
					temp_y = export_y;
				else if ((Ypos < near_top) && (SetY < near_top))
				{
					if (Ypos > SetY)
						temp_y = Ypos;
					else
						temp_y = SetY;
				}
				else if (Ypos < near_top)
					temp_y = Ypos;
				else if (SetY < near_top)
					temp_y = SetY;
				else
					temp_y = 0;
				digi_print ((Imageheight - temp_y) / VerticalBase);

				/* Find basename */
				strcpy(basename,export_name);
				p = strrchr(basename,'.');
				if (p == (char *)NULL) {
					/* No file extension */
					m_fprintf("%s_%d\n", export_name, export_ctr);
				} else {
					strcpy(ext,p);
					p[0] = 0;
					m_fprintf("%s_%d%s\n", basename, export_ctr, ext);
				}
			}

		}						/* end if(!MultiPagesUp.... */
		cc_mask = save_cc_mask;
/* collect page bounding boxes for the end_JOB call */
		if (J_minx < 0)
			J_minx = 0;
		if (J_miny < 0)
			J_miny = 0;
		if (temp_bounding_box_x > J_maxx)
			J_maxx = temp_bounding_box_x;
		if (PageH > J_maxy)
			J_maxy = PageH;
		if ( !IncludeScreens )
			m_fprintf("currentscreen setscreen\n");

#if LPMfloat
		if ( (LPMK[LPM_PWS1] || LPMK[LPM_PWS2]) && ( !LYPrintFlag) )
#else
		if ( ((LPMK & LPM_PWS1) || (LPMK & LPM_PWS2)) && ( !LYPrintFlag) )
#endif
		{						/* gray background for PWS */
			m_fprintf("8 /Helvetica F %d %d M %d %d L %d %d L %d %d L closepath .95 setgray fill\n0 setgray\n",
					  J_minx, J_miny, J_maxx, J_miny,
					  J_maxx, J_maxy, J_minx, J_maxy);
			log_font("Helvetica", cc_mask);
		}
		Kspace[0] = 0;
		if ( !GalleySlugFlag)
		{
			if ( LYPrintFlag)
			{					/* printing layouts and reports */
				m_fprintf("%d /Courier F \n", ReportSize);
				log_font("Courier", cc_mask);
			}
			else
			{
				m_fprintf("10 /Helvetica F \n");
				log_font("Helvetica", cc_mask);
			}
			Fx = temp_fx;
			Fy = temp_fy;
			Fx2 = temp_fx2;
			Fy3 = temp_fy3;
			if ( !FileType)
				color_func(fo_line_def.SolFgColor, fo_line_def.SolFgShade);
			MultiPagesFirstIsOdd = 0;
			cc_mask = save_cc_mask;
			return;				/* omit id line if not galley */
		}
		strcpy(name_buff, SubDirName);
		if ( (result = strrchr(name_buff, '.')) )
			*result = 0;		/* null out the name extension */

		/*** Bug 363p:  Add option to print job name with header for Pages
		 ***	based on flag HdrFNamPrt defined in Psftable as HDRFNAMPRT
		 ***/
		if (( HdrFNamPrt) && (FileType))
		{
			strncat(name_buff, "/", 4);
			strncat(name_buff, LayoutName, MAX_NAME+4);
			sprintf(unit_nbr_buff, "pg %d   ", PageNo1);
		}
		else
			if ( !FileType)
			{						/* add '/jobname' to id */
				strncat(name_buff, "/", 4);
				strncat(name_buff, JobName, MAX_NAME+4);
			}
			else					/* add page number if unit */
				sprintf(unit_nbr_buff, "pg %d   ", PageNo1);

		/*** Bug 367p:  Add option to print printer name as part of header info
		 ***	based on flag HdrPNamPrt defined in Psftable as HDRPNAMPRT
		 ***/
		if( HdrPNamPrt && PrtNam[0])
		{
				strcpy(prtr_buff, "Printer: ");
				strcat(prtr_buff, PrtNam);
		}
		else
				prtr_buff[0] = 0;

		p_gettimeofday(&tp);
		tm = localtime(&tp.tv_sec);
		/*  Bug 369p add a four digit year to header line - year 2000  */
		strftime(time_buf, sizeof time_buf, "%m/%d/%Y %I:%M%p", tm);
		hold_cc_mask = cc_mask;
		for (ii=0; ii<MAX_CLRS; ii++)
		{
			cc_mask = ((loop_temp=(1<<ii))) & hold_cc_mask;
			if (loop_temp > hold_cc_mask)
				break;
			if ( !cc_mask)
				continue;
	if (!KeyOutputType)
				temp_plate_nbr = 0;
			else
				temp_plate_nbr = ii+1;
			if ( plate_nm[temp_plate_nbr].name[0])
			{	
				m_fprintf("8 /Helvetica F %d %d M (Name /%s/%s   %s     %s     Plate # %s) S \n",
						  (int)trim_mark_width / HorizontalBase,
						  ((int16 )Imageheight / VerticalBase) - header_offset,
						  TreeName, name_buff, prtr_buff,
						  time_buf, plate_nm[temp_plate_nbr].name);
			}
			else
			{
				m_fprintf("8 /Helvetica F %d %d M (Name /%s/%s   %s     %s     Plate # %d) S \n",
						  (int)trim_mark_width / HorizontalBase,
						  ((int16 )Imageheight / VerticalBase) - header_offset,
						  TreeName, name_buff, prtr_buff,
						  time_buf, temp_plate_nbr);
			}
			itoa(++starting_page_count, nbr_buff); /* look at page nbr length*/
			length = (strlen(unit_nbr_buff) +
					  strlen(nbr_buff) + 3) * 4; /* assume digit 4 pts wide */
			if ( !FileType)
			{					/* galley, use right side of sheet */
				if (HdrW > 0)	/*** Bug 363p:  customer selected page number position ***/
					temp_pp_pg_nbr_x = HdrW - length;
				else	
					if ( !Orient)
						temp_pp_pg_nbr_x = PageW - length - OffL - OffR;
					else
						temp_pp_pg_nbr_x = PageH - length - OffT- OffB;
			}
			else
			{					/* page, use right side of page */
				if (HdrW > 0)	/*** Bug 363p customer selected page number position ***/
					temp_pp_pg_nbr_x = HdrW - length;
				else
				{
					temp_pp_pg_nbr_x = ps_lay_page_width;
					if ( MultiPagesUp)
						temp_pp_pg_nbr_x /= 2;
					temp_pp_pg_nbr_x -= (length +
									 (trim_mark_width / HorizontalBase));
				}
			}
			m_fprintf("%d %d M (%s# %d) S \n",
					  temp_pp_pg_nbr_x,
					  ((int16 )Imageheight / VerticalBase) - header_offset,
					  unit_nbr_buff, starting_page_count);
		}
		cc_mask = hold_cc_mask;
		log_font("Helvetica", cc_mask);
		if  ( !fo_line_def.SolFont)
			fo_line_def.SolFont = HelveticaFontNumber;
		put_font();
		Fx = temp_fx;
		Fy = temp_fy;
		Fx2 = temp_fx2;
		Fy3 = temp_fy3;
		if ( !FileType)
			color_func(fo_line_def.SolFgColor, fo_line_def.SolFgShade);
		if (exported == 2)  /* If in middle of export piece that is suspended at
								wrong frame type, re-activate the suspension: */
			m_fprintf("%%PSI_ExportSuspend\n");
	}							/*end if(!did_Page&&(cc_msk|act_trp_cc_msk) )*/
	MultiPagesFirstIsOdd = 0;
	cc_mask = save_cc_mask;
}				/* end function beg_PAGE  */

/**********	end page - call at end of a page section *********************/

void end_PAGE(void)
{
	int ii;
	uint32 hold_cc_mask, loop_temp;
	F_LIST *vp;					/* font list-walking pointer */
	
	if ( CurrentLineBcEc)
		return;					/* no output started */
	hold_cc_mask = cc_mask;
	if(DidPage)					/* if page header is done */
	{
#ifdef TRACE
		if (debugger_trace)
			p_info(PI_TRACE, "****** END PAGE OUTPUT: plates %lo (oct), end cnt= %d.\n",
				   cc_mask, output_page_count);
#endif
		DidPage = 0;
		Oldss = -1;				/* require font define */
		if ( MultiPagesUp && !MultiPagesOddEvenFlag)
		{						/* Multi on even page */
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "2-up, skip showpage for even page.\n");
#endif
		}
		else
		{
			if (exported) {
	                        vp  = E_fonts;
       	                 	while(vp != 0)          /* for all fonts in the list */
                        	{
                                	m_fprintf("%%PSI_ExportFont: %s\n", vp->font);
                                	vp = vp->nxt;
                        	}

				/* end of page without [pq */
				m_fprintf("%%PSI_ExportEnd: ");
				if (export_end_y)			/* Floating ill/table is exported:  */
				{
											/* Width and bottom of frame:  */
					digi_print((export_end_x - export_x) / HorizontalBase);
					digi_print((Imageheight - export_end_y) / VerticalBase);
					export_newpiece = 1;	/* Ended [pp at EOP before [pq.  Set flg
												to init bound box next page.  */
				}
				else					/* All other exports:  */
				{
					digi_print((float)MaxPP_Meas / HorizontalBase);
					digi_print(Ofy);
				}
				digi_print(export_ii / HorizontalBase);
				digi_print(export_jj / VerticalBase);
				m_fprintf("\n");

			}

			if (!EpsFlag) {
			m_fprintf("\nENCAPage restore\n"); /* encapsulate end */
			do_showpage();		/* show the page */
			}
			m_fprintf("end %%end Penta_7.0\n"); /*Penta_7.0 end*/
#ifdef TRACE
			if (debugger_trace)
				p_info(PI_TRACE, "1-up or 2-up odd page, do showpage.\n");
#endif
			if (!EpsFlag) {
			   m_fprintf("%%%%PageTrailer\n"); /* time for cleanup */
			}

			if ( ( !MultiPagesUp || (MultiPagesUp && (spot_pass == 1))) &&
				 (OverflowOrColorFlag != -1) )
			{
				if (EpsFlag)
				   nlist_fonts(&P_fonts, "%%PageFonts",0); /*output page font list */
				else
				   list_fonts(&P_fonts, "%%PageFonts",0); /*output page font list */
				vp  = P_fonts;
			}
			else if (OverflowOrColorFlag == -1)
			{
				if (EpsFlag)
				   nlist_fonts(&O_fonts, "%%PageFonts",0);
				else
				   list_fonts(&O_fonts, "%%PageFonts",0);
				vp  = O_fonts;
			}
			else
			{
				if (EpsFlag)
				   nlist_fonts(&P2_fonts, "%%PageFonts",0);
				else
				   list_fonts(&P2_fonts, "%%PageFonts",0);
				vp  = P2_fonts;
			}
/* collect the page fonts into the job font list */
			while(vp != NULL)		/* we're looking for the end */
			{
				add_font(vp->font, 1, &J_fonts);
				vp = vp->nxt;		/* move to next one */
			}
#if LPMfloat
			if ( (OverflowOrColorFlag == -1) && LPMK[LPM_ImpoStrip] )
#else
			if ( (OverflowOrColorFlag == -1) && (LPMK & LPM_ImpoStrip) )
#endif
			{
				cc_mask = 1 & Plates;
				if (plate_nm[1].name[0])
					m_fprintf("%%Overflow: \n%%Plate: %s\n", plate_nm[1].name);
				else
					m_fprintf("%%Overflow: \n%%Plate: 1\n");
			}
#if LPMfloat
			else if (LPMK[LPM_ImpoStrip])
#else
			else if ((LPMK & LPM_ImpoStrip))
#endif
			{
				for (ii=0; ii<MAX_CLRS; ii++)
				{
					cc_mask = ((loop_temp=(1<<ii))) & hold_cc_mask;
					if (loop_temp > hold_cc_mask)
						break;
					if ( !cc_mask)
						continue;
/*
					if (plate_nm[ii+1].name[0])
						m_fprintf("%%Plate: %s\n", plate_nm[ii+1].name);
					else
						m_fprintf("%%Plate: %d\n", ii+1);
*/
				}
			}
			cc_mask = hold_cc_mask;
			if (!EpsFlag)
			   m_fprintf("%%%%PageEnd\n");	/* flag end of page	*/
		}						/* end else not Multi or is Multi on odd pg */
		if(((spot_pass == 1) && (cc_hit < 2)) || /* no color for pass 2? */
		   (spot_pass == 2) || !KeyOutputType) /* 2nd pass or composite? */
			MultiPagesOddEvenFlag =
				(~MultiPagesOddEvenFlag) & 1; /*switch it*/
	}							/* end if(DidPage) */
	cc_mask = hold_cc_mask;
	OverflowOrColorFlag = 0;
	if (Ktabmode && !FileType)
		nexty = 0;
	Kspace[0] = 0;
	DidPage = 0;
}								/* end function */
/************************************************************************
 ** CLEAR_FLIST	clear all entries (if any) from font list started *
 **		by passed pointer. Release the memory they occupied as *
 **		the links are dissolved. *
 ************************************************************************/
void clear_flist(F_LIST **lst)	/* clear list of fonts */
{
	F_LIST *vp = *lst;			/* list walker */
	F_LIST *np;					/* next entry pointer */
	
	while(vp != NULL)			/* walk towards the end */
	{
		if(vp->font)			/* if it looks valid... */
			p_free(vp->font);	/* free name space */
		np =  vp->nxt;			/* save location of next one */
		p_free( (char *)vp);	/* free the memory */
		vp = np;				/* and point to 'next' */
	}
	*lst = 0;					/* clear first list entry too */
}								/* end function */
/************************************************************************
 **  log_font	Add an entry to the font list.  Allocate memory	*
 **		TRUE is returned for success, FALSE for failure	*
 ************************************************************************/
int log_font(char *font, int mask)
{
	if ( MultiPagesUp && (spot_pass == 2) )
		return(add_font(font, mask, &P2_fonts));
	else if ( in_overflow)
		return(add_font(font, mask, &O_fonts));
	else
		return(add_font(font, mask, &P_fonts));
}

/************************************************************************
 **  ADD_font	Add an entry to the font list started by passed list *
 **		Memory must be allocated. *
 **		TRUE is returned for success, FALSE for failure	*
 ************************************************************************/

int add_font(char *font, int mask, F_LIST **list)
{
	F_LIST *vp = *list;			/* list-walking pointer */
	F_LIST **lp = list;			/* pointer to last link */
	
	
	while(vp != NULL)			/* we're looking for the end */
	{
		if(strcmp(vp->font, font) == 0) /* found our font? */
		{
			vp->mask |= mask;	/* add to mask */
			return(TRUE);
		}
		lp = &vp->nxt;			/* save address of last link */
		vp = vp->nxt;			/* move to next one */
	}
	
	vp = *lp = (F_LIST *) p_alloc(sizeof(F_LIST));
/* allocate & link an entry	*/
	if(vp != NULL)				/* and if it's good */
	{
		if((vp->font = p_alloc(strlen(font) +1)) ==0)
		{
			p_info(PI_ELOG, "add_flist:NO MEMORY FOR FONTNAME ENTRY\n");
			*lp = 0;			/* ignore this trial */
			p_free ( (char *)vp); /* discard the useless entry */
			return(FALSE);		/* and scream. They listening? */
		}
		strcpy(vp->font, font);
		/* fill the data in */
		vp->mask = mask;
		vp->nxt = NULL;			/* and re-terminate the list */
		return(TRUE);
	}
	else
	{
		p_info(PI_ELOG, "add_flist:NO MEMORY FOR ENTRY\n");
		return(FALSE);
	}
}								/* end function */

/************************************************************************
 **  list_fonts	output a list of fonts required to the various color 	*
 **		separation channels *
 **		ALSO - to list document resources 			**
 ************************************************************************/

/* Print to TrailerName */
void flist_fonts(F_LIST **list, char *font_set, int mode)
{
	F_LIST *vp = *list;			/* list-walking pointer */
	uint32 hold_cc_mask = cc_mask;
	uint32 hits = cc_mask;		/* map of seps hit */
	char temp_string[128];
        int result=0;
	
#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "list fonts %s:\n", font_set);
#endif
	fprintf(TrailerName,"%s:\n",font_set);
								/* if no fonts on a color. */
								/* so we keep track of hits */
	while(vp != 0)				/* for all fonts in the list */
	{
#ifdef TRACE
		if (debugger_trace)
		{
			p_info(PI_TRACE, "font entry @ %x:\n", (unsigned)vp);
			p_info(PI_TRACE, "\tmask=%x, name = %s, next = %x\n",
				   vp->mask, vp->font, (unsigned)vp->nxt);
		}
#endif

		sprintf(temp_string, "%s%s", cont_line, vp->font);
		cc_mask = vp ->mask;
		fprintf(TrailerName,"%s\n", temp_string);
		hits &= ~vp->mask;		/* clear bits for seps we hit */
		vp = vp->nxt;			/* move to next one */
	}
	if(hits)					/* some sep has NO FONTS? */
	{
		cc_mask = hits;
/*		fprintf(TrailerName,"%s(none)\n", cont_line); *//* fine ... */
        }
        cc_mask = hold_cc_mask;

        memset(fontneeded,0,sizeof(fontneeded));
        memset(fontsupplied,0,sizeof(fontsupplied));
        sprintf(fontneeded,"%%%%DocumentNeededFonts:\n");
        sprintf(fontsupplied,"%%%%DocumentSuppliedFonts:\n");
       if (DownLoadFont) {
       vp = *list;
       while (vp != 0) {
          /* Downloading Fonts */
          memset(Type1Path,0,sizeof(Type1Path));
          sprintf(Type1Path,"/Penta/%s/userdata/postscript/type1/%s", TreeName, vp->font);

          if((FontFile=fopen(Type1Path,"r"))==(FILE *)NULL)  {
		result=1;
          	p_info(PI_ELOG, "ERROR - Font '%s' is missing.\n",vp->font);
          } else {
   	     	result = downloadfont(vp->font);
             	fclose(FontFile);
          }
	  sprintf(temp_string,"%%%%+%s\n",vp->font);
	  if (result)
		strcat(fontneeded,temp_string);
	  else
		strcat(fontsupplied,temp_string);

	  vp = vp->nxt;			/* move to next one */
       }
       fprintf(TrailerName,"%s",fontneeded);
       fprintf(TrailerName,"%s",fontsupplied);

   } else {
	vp = *list;
        while (vp != 0) {
	     sprintf(temp_string,"%%%%+%s\n",vp->font);
	     strcat(fontneeded,temp_string);
	     vp = vp->nxt;			/* move to next one */

       }
       fprintf(TrailerName,"%s",fontneeded);
       fprintf(TrailerName,"%s",fontsupplied);

   }
		
}



/* No Print */
void nlist_fonts(F_LIST **list, char *font_set, int mode)
{
	F_LIST *vp = *list;			/* list-walking pointer */
	uint32 hold_cc_mask = cc_mask;
	uint32 hits = cc_mask;		/* map of seps hit */
	char temp_string[128];
	
#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "list fonts %s:\n", font_set);
#endif
	while(vp != 0)				/* for all fonts in the list */
	{
#ifdef TRACE
		if (debugger_trace)
		{
			p_info(PI_TRACE, "font entry @ %x:\n", (unsigned)vp);
			p_info(PI_TRACE, "\tmask=%x, name = %s, next = %x\n",
				   vp->mask, vp->font, (unsigned)vp->nxt);
		}
#endif
		sprintf(temp_string, "%s%s", cont_line, vp->font);
		cc_mask = vp ->mask;
	/*
		if (mode)
		   fprintf(PrologName,"%%%%IncludeResource: font %s\n", vp->font);
	*/
		hits &= ~vp->mask;		/* clear bits for seps we hit */
		vp = vp->nxt;			/* move to next one */
	}
	if(hits)					/* some sep has NO FONTS? */
	{
		cc_mask = hits;
	}
	cc_mask = hold_cc_mask;
}								/* end list_fonts */

/* Print to current stream */
void list_fonts(F_LIST **list, char *font_set, int mode)
{
	F_LIST *vp = *list;			/* list-walking pointer */
	uint32 hold_cc_mask = cc_mask;
	uint32 hits = cc_mask;		/* map of seps hit */
	char temp_string[128];
	
#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "list fonts %s:\n", font_set);
#endif
	m_fprintf("%s:\n", font_set); /* may result in a blank list */
								/* if no fonts on a color. */
								/* so we keep track of hits */
	while(vp != 0)				/* for all fonts in the list */
	{
#ifdef TRACE
		if (debugger_trace)
		{
			p_info(PI_TRACE, "font entry @ %x:\n", (unsigned)vp);
			p_info(PI_TRACE, "\tmask=%x, name = %s, next = %x\n",
				   vp->mask, vp->font, (unsigned)vp->nxt);
		}
#endif
		sprintf(temp_string, "%s%s", cont_line, vp->font);
		cc_mask = vp ->mask;
		m_fprintf("%s\n", temp_string);
	/*
		if (mode)
		   fprintf(PrologName,"%%%%IncludeResource: font %s\n", vp->font);
	*/
		hits &= ~vp->mask;		/* clear bits for seps we hit */
		vp = vp->nxt;			/* move to next one */
	}
	if(hits)					/* some sep has NO FONTS? */
	{
		cc_mask = hits;
/*		m_fprintf("%s(none)\n", cont_line); *//* fine ... */ 
	}
	cc_mask = hold_cc_mask;
}								/* end list_fonts */

/******** init at start of job *****************************************/

void ini_JOB(void)
{
	int ii;

	memset(customused,0,sizeof(customused));
	memset(cmykcustomused,0,sizeof(cmykcustomused));
	for (ii=0; ii< 48; ii++)
		colorsused[ii] = 0;
	bmctr = -1;
	uidctr = -1;
	exported=0;
	pdflinkctr=0;
	pdflinkAtSOL=0;
	clear_flist(&E_fonts);		/* clear export fontlist */
	clear_flist(&J_fonts);		/* clear job fontlist */
	clear_flist(&P_fonts);		/* clear page fontlist */
	clear_flist(&P2_fonts);		/* clear MultiPages pass 2 fontlist */
	clear_flist(&O_fonts);		/* clear overflow fontlist */
	m_clear();					/* clear list of output files */
	J_minx = J_miny = J_maxx = J_maxy = 0;	/* wipe job bounding box */

}								/* end function */

/********** do output required at start of a job (prolog) ***************/

void beg_JOB(char *new)
{

    char pres_time[32];			/* present time */
    struct timeval tp;
    struct tm *tm;

#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "BEGIN JOB OUTPUT FILE\n");
#endif

    p_gettimeofday(&tp);
    tm = localtime(&tp.tv_sec);
	/*  Bug 369p the year 2000 compliancy  */
    strftime(pres_time, sizeof pres_time, "%m/%d/%Y %I:%M%p", tm);

	fprintf(TrailerName, "%%!PS-Adobe-3.0");
        if (EpsFlag)
		fprintf(TrailerName," EPSF-3.0");
        fprintf(TrailerName,"\n");

	fprintf(TrailerName, "%%%%Creator: PubLink Inc. Version 16.52 \n");
	fprintf(TrailerName, "%%%%For: %s\n",Uname );
	if (EpsFlag)
	   fprintf(TrailerName, "%%%%Title: (%s.eps)\n", new);
	else
	   fprintf(TrailerName, "%%%%Title: (%s.tp)\n", new);
	fprintf(TrailerName, "%%%%CreationDate: %s\n", pres_time);
	fprintf(PrologName, "%%%%DocumentSuppliedProcSets: Penta 7.0 0\n");
	fprintf(PrologName, "%%%%EndComments\n");
	fprintf(PrologName, "%%%%BeginProlog\n");
	fprintf(PrologName, "%%%%BeginProcSet: Penta 7.0 0\n");
	fprintf(PrologName, "/Penta_7.0 userdict exch 100 dict dup begin put\n");
	fprintf(PrologName, " /bdf{bind def}bind def\n");
	fprintf(PrologName, " /L  {lineto}bdf    /RL {rlineto}bdf\n");
	fprintf(PrologName, " /M  {moveto}bdf    /RM {rmoveto}bdf\n");
	fprintf(PrologName, " /T  {translate}bdf\n");
	fprintf(PrologName, " /GS {gsave}bdf     /GR {grestore}bdf\n");
	fprintf(PrologName, " /S  {show}bdf\n");
	fprintf(PrologName, " /FF {findfont} bdf /MF {makefont setfont}bdf \n");
	fprintf(PrologName, " /F {findfont exch scalefont setfont} bdf\n");
	fprintf(PrologName, " /RXY {2 copy -1. mul exch -1. mul exch 5 2 roll\n");
	fprintf(PrologName, "       translate rotate translate} bdf\n");
	fprintf(PrologName, " /K {{pop pop 0 rmoveto} exch kshow} bdf\n");
	fprintf(PrologName, " /TPB {false charpath GS}bdf\n");
	fprintf(PrologName, " /TPE {setlinewidth stroke GR}bdf\n");
	fprintf(PrologName, " /scratchm matrix def\n");
	/* call ES with X-Center, Y-Center, X-Radius, Y-Radius */
	fprintf(PrologName, " /ES {newpath scratchm currentmatrix pop\n");
	fprintf(PrologName, "      translate scale 0 0 1 0 360 arc closepath\n");
	fprintf(PrologName, "      scratchm setmatrix} bdf\n");
	fprintf(PrologName, " /dot {dup mul exch dup mul add 1 exch sub}bdf\n");
	fprintf(PrologName, " /Dot {abs exch abs 2 copy add 1 gt{1 sub dup mul exch 1 sub dup mul add 1 sub}\n");
	fprintf(PrologName, " 	{dup mul exch dup mul add 1 exch sub}ifelse}bdf\n");
	fprintf(PrologName, " /square {2 copy gt {exch} if pop}bdf\n");
	fprintf(PrologName, " /line {pop}bdf\n");
	fprintf(PrologName, " /Diamond {abs exch abs add 1 exch sub}bdf\n");
	fprintf(PrologName, " /Grid {2 copy abs exch abs gt {exch} if pop 2 mul 1 exch sub 3.5 div}bdf\n");
	fprintf(PrologName, " /Lines {pop abs 2 mul 1 exch sub}bdf\n");
	fprintf(PrologName, " /Star {abs exch abs 2 copy gt {exch} if 1 sub\n");
	fprintf(PrologName, "        dup 0 eq {0.01 add}if atan 360 div}bdf\n");
	fprintf(PrologName, " /Dot2 {dup mul exch dup mul add 1 sub}bdf\n");
	fprintf(PrologName, " /OutCircleBlk {dup mul exch dup mul add 0.6 exch sub abs -0.5 mul}bdf\n");
	fprintf(PrologName, " /OutCircleWhi {dup mul exch dup mul add 0.6 exch sub abs 0.5 mul}bdf\n");
	fprintf(PrologName, " /MicroWaves {/wy exch def\n");
	fprintf(PrologName, "              180 mul cos 2 div wy dup dup dup mul mul sub mul wy add\n");
	fprintf(PrologName, "              180 mul cos}bdf\n");
	fprintf(PrologName, " /Ellipse {dup 5 mul 8 div mul exch dup mul exch add sqrt 1 exch sub}bdf\n");
	fprintf(PrologName, " /ScreenTiff {/_begin [currentdict /begin load]cvx bdf\n");
	fprintf(PrologName, "  /_end /end load def /_image /image load def\n");
	fprintf(PrologName, "  /image{ 2 index 1 eq {5 2 roll pop false 5 -2 roll imagemask}\n");
	fprintf(PrologName, "  [{1 exch sub 1 pentagray sub  mul 1 exch sub} /exec load currenttransfer /exec load]\n");
	fprintf(PrologName, "    cvx settransfer \n");
	fprintf(PrologName, "  {_begin _image _end}ifelse} bdf } bdf\n");
	fprintf(PrologName, " /ScreenEPS {/_begin [currentdict /begin load]cvx bdf\n");
	fprintf(PrologName, "  /_end /end load def /_setgray /setgray load def\n");
	fprintf(PrologName, "  /_imagemask /imagemask load def\n");
	fprintf(PrologName, "  /imagemask{ pentagray _setgray _imagemask} bdf\n");
	fprintf(PrologName, "  [{1 exch sub 1 pentagray sub  mul 1 exch sub} /exec load currenttransfer /exec load]\n");
	fprintf(PrologName, "    cvx settransfer \n");
	fprintf(PrologName, "  /image{ 2 index 1 eq {5 2 roll pop false 5 -2 roll imagemask}\n");
	fprintf(PrologName, "  {_begin _image _end}ifelse} bdf } bdf\n");
	fprintf(PrologName, " /BEGINEPSFILE {/EPSFsave save def countdictstack mark \n");
	fprintf(PrologName, "	{currentpoint}stopped newpath \n");
	fprintf(PrologName, "	70 dict begin \n");
	fprintf(PrologName, "	currentgray /pentagray exch def\n");
	fprintf(PrologName, " 0 setgray 0 setlinecap \n");
	fprintf(PrologName, "	1 setlinewidth 0 setlinejoin\n");
	fprintf(PrologName, "	10 setmiterlimit [] 0 setdash\n");
	fprintf(PrologName, "	/languagelevel where \n");
    	fprintf(PrologName, "		{pop languagelevel \n");
    	fprintf(PrologName, "		1 ne\n");
    	fprintf(PrologName, "			{false setstrokeadjust false setoverprint\n");
    	fprintf(PrologName, "		}if\n");
    	fprintf(PrologName, "	}if\n");
	fprintf(PrologName, "	/showpage {}def /copypage {} def} bdf\n");
	fprintf(PrologName, " /ENDEPSFILE   {\n");
	fprintf(PrologName, "	cleartomark \n");
	fprintf(PrologName, "	countdictstack exch sub dup 0 gt{{end}repeat}{pop}ifelse\n");
	fprintf(PrologName, "	EPSFsave restore} bdf\n");
	fprintf(PrologName, " /registermarks { gsave [] 0 setdash translate setlinewidth\n");
	fprintf(PrologName, "	newpath 1 setgray 0 0 10 0 360 arc fill\n");
	fprintf(PrologName, "	0 setgray 0 0 8 0 360 arc\n");
	fprintf(PrologName, "	dup -1 mul 0 moveto dup 0 lineto dup -1 mul 0 exch moveto 0 exch lineto\n");
	fprintf(PrologName, "	stroke 0 0 4 0 360 arc fill 1 setgray\n");
	fprintf(PrologName, "	-4 0 moveto 4 0 lineto 0 -4 moveto 0 4 lineto stroke\n");
	fprintf(PrologName, "	grestore } bdf\n");
	fprintf(PrologName, " /DNSTEP	{ 1 index exch sub 3 index 1 sub div sub exch\n");
	fprintf(PrologName, "	dup -1 roll pop 2 sub 1 roll } bdf\n");

    	fprintf(PrologName, " /CALCBLENDSCALE{ /depth exch def /width exch def\n");
    fprintf(PrologName, " width 2 div width 2 div mul depth 2 div depth 2 div mul add sqrt /tempr exch def\n");
    fprintf(PrologName, " width 2 div tempr div depth 2 div tempr div scale } bdf\n");
    fprintf(PrologName, " /RADBWBLEND { smooth 0 ne {pop smooth}if\n");
    fprintf(PrologName, " /stepnum exch def /xstart exch def /k0 exch def /k1 exch def\n");
    fprintf(PrologName, " /depth exch def /width exch def \n");
    fprintf(PrologName, " currentpoint /cy exch def /cx exch def\n");
    fprintf(PrologName, " width 2 div width 2 div mul depth 2 div depth 2 div mul add sqrt /radius exch def\n");
    fprintf(PrologName, " radius stepnum div /stepwidth exch def\n");
    fprintf(PrologName, " k0 k1 sub stepnum div /kstep exch def\n");
    fprintf(PrologName, " /bwdownstep {radius stepwidth sub abs /radius exch def\n");
    fprintf(PrologName, "     k0 kstep sub abs /k0 exch def} bind def\n");
    fprintf(PrologName, " stepnum 1 add { k0 setgray radius radius cx cy ES fill bwdownstep} repeat\n");
    fprintf(PrologName, " } bind def\n");
    fprintf(PrologName, " /RADCLRBLEND { smooth 0 ne {pop smooth}if\n");
    fprintf(PrologName, " /stepnum exch def /xstart exch def /k0 exch def /y0 exch def /m0 exch def /c0 exch def\n");
    fprintf(PrologName, " /k1 exch def /y1 exch def /m1 exch def /c1 exch def\n");
    fprintf(PrologName, " /depth exch def /width exch def \n");
    fprintf(PrologName, " currentpoint /cy exch def /cx exch def\n");
    fprintf(PrologName, " width 2 div width 2 div mul depth 2 div depth 2 div mul add sqrt /radius exch def\n");
    fprintf(PrologName, " radius stepnum div /stepwidth exch def\n");
    fprintf(PrologName, " k0 k1 sub stepnum div /kstep exch def\n");
    fprintf(PrologName, " y0 y1 sub stepnum div /ystep exch def\n");
    fprintf(PrologName, " m0 m1 sub stepnum div /mstep exch def\n");
    fprintf(PrologName, " c0 c1 sub stepnum div /cstep exch def\n");
    fprintf(PrologName, " /clrdownstep {radius stepwidth sub abs /radius exch def\n");
    fprintf(PrologName, "     k0 kstep sub /k0 exch def\n");
    fprintf(PrologName, "     c0 cstep sub /c0 exch def\n");
    fprintf(PrologName, "     y0 ystep sub /y0 exch def\n");
    fprintf(PrologName, "     m0 mstep sub /m0 exch def} bind def\n");
    fprintf(PrologName, " stepnum 1 add { c0 m0 y0 k0 setcmykcolor radius radius cx cy ES fill clrdownstep} repeat\n");
    fprintf(PrologName, " } bind def\n");
	fprintf(PrologName, " /BWBLEND { smooth 0 ne {pop smooth}if\n");
	fprintf(PrologName, "   -1 1 { newpath 1 index 5 index M\n");
	fprintf(PrologName, "	5 index 2 index sub 1 index div	2 index add 1 index\n");
	fprintf(PrologName, "	1 eq {6 index exch pop 5 -2 roll pop dup 5 2 roll} if\n");
	fprintf(PrologName, "	dup 6 index L dup 0 L 2 index 0 L\n");
	fprintf(PrologName, "	3 index setgray fill 3 -1 roll exch 3 1 roll pop\n");
	fprintf(PrologName, "	dup 1 gt { 5 3 index 5 index DNSTEP}");
	fprintf(PrologName, "	if pop } for 5 {pop} repeat} bdf\n");
	fprintf(PrologName, " /CLRBLEND { smooth 0 ne {pop smooth}if\n");
	fprintf(PrologName, "   -1 1 { newpath 1 index 11 index M\n");
	fprintf(PrologName, "	11 index 2 index sub 1 index div 2 index add 1 index\n");
	fprintf(PrologName, "	1 eq{ 12 index exch pop 11 -8 roll 4 {pop} repeat\n");
	fprintf(PrologName, "	4 copy 11 8 roll } if dup 12 index L dup 0 L 2 index 0 L\n");
	fprintf(PrologName, "	6 index 6 index 6 index 6 index setcmykcolor fill\n");
	fprintf(PrologName, "	3 -1 roll exch 3 1 roll pop dup 1 gt\n");
	fprintf(PrologName, "	{ 8 6 index 11 index DNSTEP 7 5 index 10 index DNSTEP\n");
	fprintf(PrologName, "	6 4 index 9 index DNSTEP 5 3 index 8 index DNSTEP\n");
	fprintf(PrologName, "	} if pop } for 11 {pop} repeat } bdf\n");
	fprintf(PrologName,"/PCLRBLEND { smooth 0 ne {pop smooth}if\n");
        fprintf(PrologName,"  2 index 9 index /sclr exch def /eclr exch def\n");
        fprintf(PrologName,"	2 index 9 index sub 1 index 1 sub div abs /tdn exch def\n");
        fprintf(PrologName,"	-1 1 { newpath 1 index 15 index M\n");
        fprintf(PrologName,"	15 index 2 index sub 1 index div 2 index add 1 index\n");
        fprintf(PrologName,"	1 eq{ 16 index exch pop 15 -12 roll 6 {pop} repeat\n");
        fprintf(PrologName,"	6 copy 15 12 roll } if dup 16 index L dup 0 L 2 index 0 L\n");
        fprintf(PrologName,"	%% Set first color\n");
        fprintf(PrologName,"	14 index  14 index 14 index 14 index 14 index 14 index pentacustomcolor fill\n");
        fprintf(PrologName,"	3 -1 roll exch 3 1 roll pop dup 1 gt\n");
        fprintf(PrologName,"	{\n");
        fprintf(PrologName,"        %% Reset the stack with a new tint\n");
        fprintf(PrologName,"        sclr eclr lt {8 index tdn add}{8 index tdn sub}ifelse\n");
        fprintf(PrologName,"        9 -1 roll pop 9 1 roll\n");
        fprintf(PrologName,"	} if pop } for 15 {pop} repeat } bdf\n");
	fprintf(PrologName, " /NEG { [{1 exch sub}/exec cvx currenttransfer/exec cvx]\n");
	fprintf(PrologName, "	cvx settransfer NEGA } bdf\n");
	fprintf(PrologName, " /NEGA { gsave -72000 dup moveto -72000 72000 lineto 72000\n");
	fprintf(PrologName, "	dup lineto 72000 -72000 lineto closepath 1 setgray fill grestore } bdf\n");
	fprintf(PrologName, "end\n");
	fprintf(PrologName, "%%%%EndProcSet\n");
	fprintf(PrologName, "%%%%EndProlog\n");
	fprintf(PrologName, "%%%%BeginSetup\n");
/* Start writing to actual .tp file */

	m_fprintf("Penta_7.0 begin\n");
	m_fprintf("systemdict /languagelevel where \n");
        m_fprintf("{pop languagelevel \n");
        m_fprintf("  1 ne\n");
        m_fprintf("  {false setstrokeadjust false setoverprint\n");
        m_fprintf("  }{/setoverprint {pop} bdf} ifelse\n");
        m_fprintf("}{/setoverprint {pop} bdf} ifelse\n");
	m_fprintf("\n");
	m_fprintf("systemdict /languagelevel where\n");
	m_fprintf("{pop languagelevel 2 ge\n");
	m_fprintf("  {/SMOOTH{/smooth exch def}bdf}\n");
	m_fprintf("  {/SMOOTH{{pop}/smooth 0 def}bdf}ifelse\n");
	m_fprintf("}{/SMOOTH{{pop}/smooth 0 def}bdf}ifelse\n");
	m_fprintf("\n");
	m_fprintf("systemdict /setcmykcolor known not\n");
	m_fprintf("{\n");
	m_fprintf("  /setcmykcolor \n");
	m_fprintf("  {1 sub 4 1 roll 3 {3 index add neg dup 0 lt\n");
	m_fprintf("  {pop 0} if 3 1 roll} repeat setrgbcolor pop}bdf\n");
	m_fprintf("}if\n");
	m_fprintf("\n");
        m_fprintf("systemdict /setcustomcolor where not\n");
        m_fprintf("{\n");
        m_fprintf("  /findcmykcustomcolor\n");
        m_fprintf("  {\n");
        m_fprintf("    5 packedarray\n");
        m_fprintf("  } bind def\n");
        m_fprintf("  /setcustomcolor\n");
        m_fprintf("  {\n");
        m_fprintf("    exch aload pop pop\n");
        m_fprintf("    4\n");
        m_fprintf("    {\n");
        m_fprintf("       4 index mul 4 1 roll\n");
        m_fprintf("    } repeat\n");
        m_fprintf("    5 -1 roll pop\n");
        m_fprintf("    setcmykcolor\n");
        m_fprintf("  }\n");
        m_fprintf("  def\n");
        m_fprintf("} if\n");
        m_fprintf("/_gf null def\n");
        m_fprintf("/_if null def\n");
        m_fprintf("/pentacustomcolor\n");
        m_fprintf("{\n");
        m_fprintf("/_gf exch def\n");
        m_fprintf("findcmykcustomcolor\n");
        m_fprintf("/_if exch def\n");
        m_fprintf("_if _gf 1 exch sub setcustomcolor\n");
        m_fprintf("\n");
        m_fprintf("} bdf\n");
        m_fprintf("\n");
        m_fprintf("/pdfmark where \n");
        m_fprintf("{pop}{userdict /pdfmark /cleartomark load put}ifelse\n");


	/* Find Layout Trim Depth if pages or Paper Depth if Galleys */

	if (FileType) {
		/* pages */
		wn = PsCurRec -> frame_wn;
		ltrimd= ( ( (float)lmt_off_to_abs(wn,Y_REF,TRIM_DEPTH) ) / VerticalBase); /* use layout trim depth */
		ltrimw= ( ( (float)lmt_off_to_abs(wn,X_REF,TRIM_WIDTH) ) / HorizontalBase); /* use layout trim width */
	} else {
		ltrimd=PageH; /* Use paper depth from print set-up menu */
		ltrimw=PageW; /* Use paper width from print set-up menu */
	}

	/* Values change based on crop box */
	cropboxw = ltrimw;
	cropboxd = ltrimd;
	if (pdf_crop_cw && pdf_crop_cd) {
		cropboxw = pdf_crop_cw;
		cropboxd = pdf_crop_cd;
	}
	else if (pdf_crop_dw && pdf_crop_dd) {
		cropboxw = pdf_crop_dw;
		cropboxd = pdf_crop_dd;
	}

	if (PdfActive)
	{
		m_fprintf("[/PageMode ");
		switch(pdf_docview.initdisp) {
		   case 0:
			m_fprintf("/UseNone ");
			break;
		   case 1:
			m_fprintf("/UseOutlines ");
			break;
		   case 2:
			m_fprintf("/UseThumb ");
			break;
		   case 3:
			m_fprintf("/UseBoth ");
			break;
		}

        m_fprintf("\n    /Page %d ", pdf_docview.initpage);

		switch(pdf_docview.initsize) {
		  case 0:
			/*fit depth*/
			m_fprintf("/View [/FitV 0]\n");	/* Fit pg dep to window, 0 left shift. */
			break;
		  case 1:
			/* fit width */
			m_fprintf("/View [/FitH %d]\n", cropboxd);	/* Fit pg wid to window, 0 top shift. */
			break;
		  case 2:
			/* fit zoom */
			m_fprintf("/View [/XYZ null null %5f]\n", pdf_docview.initzoom/100);
			break;
		}

        m_fprintf("    /DOCVIEW pdfmark\n");
	}			/* End  if (PdfActive)  */
		
	CropBoxFlag=0;
					/* If doing pages, and crop-box wid/dep were supplied at
						pdf menu, then we need to put out crop boxes.  */
	if ( FileType && pdf_crop_dw && pdf_crop_dd &&
					/* If cropping values for default even same as for odd, and  */
		 pdf_crop_dex == pdf_crop_dox && pdf_crop_dey == pdf_crop_doy &&
					/*   cropping values for chap don't exist, or are identical to 
						default, then output only one crop box, right here:  */
		 ((!pdf_crop_cw && !pdf_crop_cd) ||
		  (pdf_crop_cex == pdf_crop_dex && pdf_crop_cey == pdf_crop_dey &&
		   pdf_crop_cw == pdf_crop_dw && pdf_crop_cd == pdf_crop_dd)))
	{
			cbllx = pdf_crop_dex;
			cblly = (ltrimd - (cropboxd + pdf_crop_dey));
			cburx = pdf_crop_dex + cropboxw;
			cbury = ltrimd - pdf_crop_dey;
			m_fprintf("[ /CropBox [ %d %d %d %d ]\n\t/PAGES pdfmark\n",
					cbllx, cblly, cburx, cbury);
			CropBoxFlag = 1;	/* Flag: No cropbox each page.  */
	}

	if ( PsfPrologue && PsfPrologueLineCount)
	{
		getprolog();				/* Transfer user's PostScript
									   prolog, if any, to TP. */
	}
	if ( CollateFlag)
			m_fprintf("true /Collate exch def\n");
			 
    if ( NumCopies)
        m_fprintf("/#copies %d def\n", NumCopies);
 
	m_fprintf("end %%end Penta_7.0\n"); /* Penta_7.0 end */
	m_fprintf("%%%%EndSetup\n");
	if ( colorfd)
		m_fprintf("%%PSI_ColorTable %s\n", default_color_name);
	else
		m_fprintf("%%PSI_ColorTable\n");
	if ( Proof)
		m_fprintf("%%PSI_Proof\n");
	if ( !CMYK_Allowed)
		m_fprintf("%%PSI_B&W\n");
	if ( Neg)
		m_fprintf("%%PSI_NEG\n");
}								/* end function */

/**************** do output required at end of a job ****************/

void end_JOB(void)
{
	int ii, jj;
	int depth;

#ifdef TRACE
	if (debugger_trace)
		p_info(PI_TRACE, "END JOB OUTPUT FILE\n");
#endif
	MasterOutputSwitch = 1;
	if ( MultiPagesUp && overflow_page_buff_count)
	{							/* output overflow page numbers */
		PaintOnPage = 1;
		DidPage = 0;
		MultiPagesUp = 0;
		beg_PAGE();
		PaintOnPage = 1;
		if ( !Orient)
			depth = PageH - OffT - 48;
		else
			depth = PageW - OffR - 48;
		m_fprintf("24 /Helvetica F 0 %d M\n(WARNING - OVERFLOW PAGES NOT OUTPUT:) S\n",
				  depth);
		depth -= 24;
		jj = 0;
		for (ii=0; ii<overflow_page_buff_count; ii++)
		{
			if ( !jj)
			{
				m_fprintf ("%d %d M (page", 67, depth);
				if ( (ii + 1) < overflow_page_buff_count )
					m_fprintf("s: ");
				else
					m_fprintf(": ");
			}
			m_fprintf("%d", overflow_page_buff[ii]);
			jj++;
			if (jj == 8 )
			{
				jj= 0;
				depth -= 24;
				m_fprintf(") S\n %d %d M ", 67, depth);
			}
			else if ( (ii + 1) < overflow_page_buff_count )
				m_fprintf (", ");
		}
		if ( jj)
			m_fprintf (") S\n");
		end_PAGE();
	}
	if ( !output_page_count && !EpsFlag)
		error (": No data, empty output file ",tpname,0);
	else if (output_page_count || EpsFlag)
	{
		m_fprintf("\n%%%%Trailer\n"); /* time for cleanup */

		fprintf(TrailerName,"%%%%BoundingBox: %d %d ", J_minx, J_miny); 
		if ( PageWInch)
			fprintf(TrailerName,"%d ", J_maxx);
		else 
			fdigi_print(J_maxx * ScaleFactorX);
		if ( PageDInch)
			fprintf(TrailerName,"%d\n", J_maxy);
		else
		{
			fdigi_print(J_maxy * ScaleFactorY);
			fprintf(TrailerName,"\n");
		}
		if (!EpsFlag)
		   fprintf(TrailerName,"%%%%Pages: %d\n", output_page_count);

		/* Output Comments for Colors Used in the Job */
		fprintf(TrailerName,"%%%%DocumentProcessColors:");
		   if (colorsused[0])
			fprintf(TrailerName," Cyan");
		   if (colorsused[1])
			fprintf(TrailerName," Magenta");
		   if (colorsused[2])
			fprintf(TrailerName," Yellow");
		fprintf(TrailerName," Black");
	        fprintf(TrailerName,"\n");

		if (customused[0][0] != '\0') {
			fprintf(TrailerName,"%%%%DoumentCustomColors:\n");
			for (ii=0; ii< 48; ii++) {
				if (customused[ii][0] == '\0')
					break;
				fprintf(TrailerName,"%%%%+%s\n", customused[ii]);
			}
			fprintf(TrailerName,"%%%%CMYKCustomColor:\n");
			for (ii=0; ii< 48; ii++) {
				if (customused[ii][0] == '\0')
					break;
				fprintf(TrailerName,"%%%%+%s %s\n", cmykcustomused[ii],customused[ii]);
			}
		}
			
		flist_fonts(&J_fonts, "%%DocumentFonts",1); /* output list of job fonts */

		/* Output pdfmarks for info */
		if (pdf_info.title[0] != '\0') {
			OutputInfo();
		}
		/* Output pdfmarks for bookmarks */
		if (bmctr > -1) {
			OutputBookMarks();
		}
		m_fprintf("%%%%EOF\n");
		if (exported) {
			/* EOF without [pq */
			fprintf(stderr,"Warning: end of job and no [pq\n");
		}

	}
	clear_flist(&E_fonts);		/* clear export fontlist */
	clear_flist(&J_fonts);		/* clear job fontlist */
	clear_flist(&P_fonts);		/* clear page fontlist */
	clear_flist(&P2_fonts);		/* clear MultiPages pass 2 fontlist */
	clear_flist(&O_fonts);		/* clear overflow fontlist */
	
	Current_FgColor = 1;		/* initialize to black - bug374p */
	Current_FgShade = 100;		/* initialize to 100%  - bug374p */

}								/* end function */

/**************** output showpage or erasepage ****************/

static void do_showpage(void)
{
	uint32 hold_cc_mask;

	if ( !SupressBlankPages)
		m_fprintf("showpage\n"); /* show it */
	else
	{
		hold_cc_mask = cc_mask;
		cc_mask &= PaintOnPage;	/* get plates with black */
		if ( cc_mask)
			m_fprintf("showpage\n"); /* show it when there is paint */
		cc_mask = (~PaintOnPage) & hold_cc_mask; /* get only white plates */
		if ( cc_mask)
			m_fprintf("erasepage\ninitgraphics\n"); /* erase when no paint */
		cc_mask = hold_cc_mask;
	}
}

/*********** EOF ***********/
