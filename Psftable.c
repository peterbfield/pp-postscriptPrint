/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "p_lib.h"
#include "Psftable.h"

PSF_CODE *PsfCode = NULL;
PENTA_TO_PSF *PentaToPsf = NULL;
PSF_DEFAULT *PsfDefault = NULL;
PSF_SENDP *PsfSendp = NULL;
PSF_EXCEPTION *PsfExcept = NULL;
char *PsfPrologue = NULL;
int PsfPrologueLineCount = 0;
int MaxPentaFont = 0;
int MaxSendp = 0;

int HdrW = 0;		/*** Bug 363p customer selectable position to print page number  ***/
int HdrFNamPrt = 0; /*** Bug 363p customer selectable file name print  ***/
int HdrPNamPrt = 0;	/*** Bug 367p customer selectable printer name as part of header  ***/

static Pfd fp;
static PENTA_TO_PSF *pp;
static PSF_CODE *tp;
static char buf[512];
static char *tok[30];
static char *bp, *cp, *psftable;
static unsigned char *dp;
static int numpsfcodes;

static int load_pagesize_and_defaults(void)
{
	int cnt;

	PsfDefault = (PSF_DEFAULT *) p_alloc( sizeof( PSF_DEFAULT));
	if( !PsfDefault)
		return (1);
	for (cnt=0; cnt<=9; cnt++)
		PsfDefault->fmchar[cnt] = cnt + 65;	/* defaults to Penta loc 0->9 */

	while( p_fgets( bp = buf, sizeof buf, fp))
	{
		if( bp[0] == '/')
		{
			if( !memcmp( bp+1, "END=", 3))
				break;
			continue;
		}
		if( bp[0] == '%')
			continue;
		if( strlen( bp) < 8)
			continue;
		
		if( bp[0] == 'S')
		{
			if( !memcmp( bp+1, "ENDP=", 5))
			{
				PSF_SENDP *sp;
				
				cnt = p_strfld( bp+6, tok, 6, 1, " \t\n");
				if( cnt <= 0)
					continue;
				
				if( !PsfSendp)
				{
					PsfSendp = (PSF_SENDP *) p_alloc( sizeof( PSF_SENDP));
					if( !PsfSendp)
						return(1);
					MaxSendp = 1;
				}
				else
				{
					PsfSendp = (PSF_SENDP *) 
						p_remalloc( (char*)PsfSendp,
									sizeof( PSF_SENDP) * MaxSendp,
									sizeof( PSF_SENDP) * (MaxSendp+1));
					if( !PsfSendp)
						return(1);
					MaxSendp++;
				}
				
				sp = &PsfSendp[MaxSendp - 1];
				switch( cnt)
				{
				  case 6: sp->noeof = atoi( tok[5]);  /* fallthru */
				  case 5: sp->htype = atoi( tok[4]);  /* fallthru */
				  case 4: sp->pageh = atoi( tok[3]);  /* fallthru */
				  case 3: sp->pagew = atoi( tok[2]);  /* fallthru */
				  case 2: sp->cmyk  = atoi( tok[1]);  /* fallthru */
				  case 1:
					memcpy( sp->name, tok[0], MIN( MAX_NAME-1,
												   strlen( tok[0])));  break;
				}
			}
			else if( !memcmp( bp+1, "CALE=", 5))
				PsfDefault->scale = atof( bp+6);
			else if( !memcmp( bp+1, "CALEX=", 6))
				PsfDefault->scalex = atof( bp+7);
			else if( !memcmp( bp+1, "CALEY=", 6))
				PsfDefault->scale = atof( bp+7);
		}
		else if( bp[0] == 'H')
		{
			if( !memcmp( bp+1, "DRUP=", 5))
				PsfDefault->hdrup = atoi( bp+6);

		/***  Bug 363p:  let user select where page # will print and give them the
		 *** 		the option of printing the filename for pages  
		 ***		HdrW position in points for sheet # to print
		 ***		HdrFNamPrt flag indicating whether or not to print file name 
		 ***			for pages.  1 = yes.
		 ***/
			else if(!memcmp( bp+1, "DRW=",4))		
				HdrW = atoi(bp+5);
			else if(!memcmp( bp+1, "DRFNAMPRT=",10))
				HdrFNamPrt = atoi(bp+11);
	
		/***  Bug 367p:  Let user select whether or not to print printer name as part
		 ***			of header.  HdrPNamPrt is flag.  HDRPNAMPRT is variable
		 ***			defined in psftable.  1 = yes
		 ***/
			else if(!memcmp( bp+1, "DRPNAMPRT=", 10))
				HdrPNamPrt = atoi(bp+11);

		}
		else if( bp[0] == 'A' && !memcmp( bp+1, "COFF=", 5))
			PsfDefault->acoff = atof( bp+6);
		else if( bp[0] == 'P')
		{
			if( !memcmp( bp+1, "AGEW=", 5))
				PsfDefault->pagew = atoi( bp+6);
			else if( !memcmp( bp+1, "AGEH=", 5))
				PsfDefault->pageh = atoi( bp+6);
			else if( !memcmp( bp+1, "OFFT=", 5))
				PsfDefault->pofft = atoi( bp+6);
			else if( !memcmp( bp+1, "OFFB=", 5))
				PsfDefault->poffb = atoi( bp+6);
			else if( !memcmp( bp+1, "OFFL=", 5))
				PsfDefault->poffl = atoi( bp+6);
			else if( !memcmp( bp+1, "OFFR=", 5))
				PsfDefault->poffr = atoi( bp+6);
			else if( !memcmp( bp+1, "LATECOLOR=", 10))
				PsfDefault->platecolor_flag = atoi( bp+11);
		}
	}							/* end while */
	return(0);
}								/* end function */

static int load_fmchar_section(void)
{
	int j, cnt;
	int k = 0;

	if( !PsfDefault)
		return (1);				/* fmchars are in PsfDefault */
	while( p_fgets( bp = buf, sizeof buf, fp))
	{
		if( bp[0] == '/')
			break;
		if( bp[0] == '%')
			continue;
		cnt = p_strfld( bp, tok, 4, 1, " \t\n");
		if( cnt < 1)
			continue;
		if ( k == 10)
			break;
		cp = tok[0];
		if( !isdigit( *cp))
			continue;
		j = atoi( cp);
		if( j <= 0)
			continue;
		PsfDefault->fmchar[k++] = j;
	}							/* end while */
	return(0);
}								/* end function */

static int load_font_section(void)
{
	int j, k, cnt;
	int numfonts = 0;

	while( p_fgets( bp = buf, sizeof buf, fp))
	{
		if( bp[0] == '/')
			break;
		if( bp[0] == '%')
			continue;
		
		cnt = p_strfld( bp, tok, 4, 1, " \t\n");
		if( cnt < 3)
			continue;
		
		cp = tok[0];
		if( !isdigit( *cp))
			continue;
		j = atoi( cp);
		if( j <= 0)
			continue;
		
		if( j > MaxPentaFont)
			MaxPentaFont = j;
		
		if( j >= numfonts)
		{
			if( numfonts == 0)
			{
				numfonts = 71;
				if ( j > numfonts)
					numfonts = j;
				else
					MaxPentaFont = numfonts;
				
				PentaToPsf = (PENTA_TO_PSF *)
					p_alloc( sizeof( PENTA_TO_PSF) * (numfonts + 1));
				if( !PentaToPsf)
					return(1);
			}
			else
			{
				k = numfonts;
				if ( j > k)
				{
					k = j + 10;
					MaxPentaFont = k;
				}
				PentaToPsf = (PENTA_TO_PSF *) 
					p_remalloc( (char*)PentaToPsf,
								sizeof( PENTA_TO_PSF) * (numfonts + 1),
								sizeof( PENTA_TO_PSF) * (k+1));
				if( !PentaToPsf)
					return(1);
				numfonts = k;
			}
		}
		
		pp = &PentaToPsf[j];
		
		if( cnt >= 4)
		{
			cp = tok[3];
			if( isdigit( *cp))
				pp->punt = atoi( cp);
		}
		else
			pp->punt = 0;
		
		cp = tok[2];
		
		if( !PsfCode)
		{
			PsfCode = (PSF_CODE *) p_alloc( sizeof( PSF_CODE));
			if( !PsfCode)
				return(1);
			memcpy( PsfCode[numpsfcodes++].name, cp, 11);
		}
		else
		{
			for( j = 0, tp = PsfCode; j < numpsfcodes; j++, tp++)
				if( !strcmp( tp->name, cp))
				{
					pp->code = j;
					break;
				}
			
			if( j >= numpsfcodes)
			{
				PsfCode = (PSF_CODE *)
					p_remalloc( (char*)PsfCode,
								sizeof( PSF_CODE) * numpsfcodes,
								sizeof( PSF_CODE) * (numpsfcodes+1));
				if( !PsfCode)
					return(1);
				memcpy( PsfCode[numpsfcodes].name, cp, 11);
				pp->code = numpsfcodes++;
			}
		}
		
		cp = tok[1];
		pp->name = (char *) p_alloc( strlen( cp) + 1);
		if( !pp->name)
			return(1);
		
		strcpy( pp->name, cp);
	}
	
	if( MaxPentaFont == 0)
	{
		p_info( PI_ELOG, "%s Contains no 'FONT SECTION' info\n", psftable);
		return(1);
	}							/* end while */
	return(0);
}								/* end function */

static int load_translation_table(void)
{
	int j, cnt;

	cp = bp + strlen( bp) - 1;
	*cp-- = 0;
	while( isspace( *cp))
		*cp-- = 0;
	
	cp = bp+24;
	
	for( j = 0, tp = PsfCode; j < numpsfcodes; j++, tp++)
		if( !strcmp( tp->name, cp))
		{
			pp->code = j;
			break;
		}
	if( j >= numpsfcodes)
		return(1);
	
	dp = tp->code;
	
	while( p_fgets( bp = buf, sizeof buf, fp))
	{
		if( bp[0] == '/')
			break;
		if( bp[0] == '%')
			continue;
		
		cnt = p_strfld( bp, tok, 2, 1, " \t\n");
		if( cnt < 2)
			continue;
		
		cp = tok[0];
		if( !isdigit( *cp))
			continue;
		j = atoi( cp);
		if( j < 0 || j > 255)
			continue;
		
		cp = tok[1];
		if( !isdigit( *cp))
			continue;
		
		dp[j] = atoi( cp);
	}							/* end while */
	return(0);
}								/* end function */

static void load_base_accent_excepts(void)
{
	int font;
	int penta_char_position;
	int j, cnt;
	int fnt_ndx = 0;

	while( p_fgets( bp = buf, sizeof buf, fp))
	{
		if( bp[0] == '/')
			break;
		if( bp[0] == '%')
			continue;
		
		cnt = p_strfld( bp, tok, 29, 1, " \t\n");
		if( cnt < 1)
			continue;			/* ignore line with only font number */

		if( !PsfExcept)
		{
			PsfExcept = (PSF_EXCEPTION *) p_alloc( sizeof( PSF_EXCEPTION));
			if( !PsfExcept)
				return;
		}

		font = atoi( tok[0]);
		if (font <= 0)
			continue;			/* ignore line if crazy font */
		for (j=0; j<16; j++)
		{
			if ( (PsfExcept -> ex_font_nbr[j] == (int16)font) ||
				 !PsfExcept -> ex_font_nbr[j])
			{
				fnt_ndx = j * 256;
				if ( !PsfExcept -> ex_font_nbr[j])
					PsfExcept -> ex_font_nbr[j] = (int16)font;
				break;
			}
		}
		if ( j >= 16)
		{
			p_info( PI_ELOG, "%s Contains more than 16 'ACCENT_EXCEPTION' fonts.\n",
					   psftable);
			return;
		}
		for (j=1; j<cnt; j++)
		{
			penta_char_position = atoi( tok[j]);
			if ( (penta_char_position <= 0) || (penta_char_position > 255) ) 
			{
				p_info( PI_ELOG, "%s, font %d contains invalid 'ACCENT_EXCEPTION' Penta font location %d.\n",
					   psftable, font, penta_char_position);
				continue;
			}
			PsfExcept -> ex_font_loc[penta_char_position + fnt_ndx] = 'a';
		}
	}							/* end while */
}								/* end function */

static void	load_prologue_section()
{
	int line_size, temp_prologue_length;
	char *prologue_end = 0;		/* points to null at end of last line */
	int prologue_size = 0;

	if ( PsfPrologue)
	{
		p_info( PI_ELOG, "%s conntains a duplicate (invalid) 'START-OF-TEXT SECTION'.\n",
				   psftable);
		goto error;
	}
	while( p_fgets( bp = buf, sizeof buf, fp))
	{
		if( !memcmp( bp, "//", 2))
			return;
		line_size = strlen(buf);
		if ( !PsfPrologue)
		{
			PsfPrologue = p_alloc(1024);
			if( !PsfPrologue)
			{
				p_info( PI_ELOG, "%s unable to allocate memory for  'START-OF-TEXT SECTION'.\n",
						   psftable);
				goto error;
			}
			prologue_size = 1024;
			prologue_end = PsfPrologue;
		}
		if ( (prologue_end - PsfPrologue + line_size) >= prologue_size)
		{
			temp_prologue_length = prologue_end - PsfPrologue;
			PsfPrologue = p_remalloc( (char*)PsfPrologue, prologue_size,
									  prologue_size + 1024);
			if( !PsfPrologue)
			{
				p_info( PI_ELOG, "%s unable to allocate memory for  'START-OF-TEXT SECTION'.\n",
						   psftable);
				goto error;
			}
			prologue_size += 1024;
			prologue_end = PsfPrologue + temp_prologue_length;
		}
		strcpy(prologue_end, buf);
		prologue_end += line_size;
		PsfPrologueLineCount += 1;
	}							/* end while(p_fgets(bp=buf,sizeof buf,fp)) */
	return;
error:
	if( fp)
		p_close( fp);
	Psftable_unload();
}								/* end function */


int Psftable_load( char *tree, char *proj)
{								/* psftable name defaults to PSF_TABLE */
	int result;
	
	result = Psftable_load_with_name( tree, proj, PSF_TABLE);
	return(result);
}

int Psftable_load_with_name( char *tree, char *proj, char *psf_name)
{
	int result, j, k, fnt_ndx;
	int pagesize_found = 0;
	int fonts_found = 0;
	int translation_table_found = 0;
	
	Psftable_unload();
	fp = 0;
	pp = NULL;
	numpsfcodes = 0;

	if (tree && *tree && proj && *proj)
	{
		if( p_get_data_name( tree, proj, buf, 0) != 0)
		{
			p_info( PI_ELOG, "Couldn't get data name for project '%s/%s'\n",
					   tree, proj);
			goto error;
		}

		cp = p_path( tree, USERDATA, buf, psf_name);
		if( !cp)
		{
			p_info( PI_ELOG, "Couldn't get data path for project '%s/%s'\n",
					   tree, proj);
			goto error;
		}

		fp = p_open( tree, USERDATA, buf, psf_name, "r");
		if( !fp)
		{
			p_info( PI_ELOG, "Couldn't open file '%s'\n", cp);
			goto error;
		}
	}
	else
	{
		fp = p_open("", OTHER_FILE,"", psf_name, "r");
		if( !fp)
		{
			p_info( PI_ELOG, "Couldn't open file '%s'\n", psf_name);
			goto error;
		}

	}

	psftable = psf_name;
	
	while( (bp = p_fgets( buf, sizeof buf, fp)) != NULL)
	{
		if( !memcmp( bp, "PAGESIZE AND DEFAULTS SECTION", 29))
		{
			if ( pagesize_found)
			{
				p_info( PI_ELOG, "%s has more than one 'PAGESIZE AND DEFAULTS SECTION'\n",
						   psf_name);
				goto error;
			}
			result = load_pagesize_and_defaults();
			if ( result)
			{
				p_info( PI_ELOG, "%s has error in 'PAGESIZE AND DEFAULTS SECTION'\n",
						   psf_name);
				goto error;
			}
			pagesize_found = 1;
		}						/* end pagesize section */	
		else if( !memcmp( bp, "F", 1))
		{
			if ( fonts_found)
			{
				p_info( PI_ELOG, "%s has more than one 'FONT SECTION'\n",
						   psf_name);
				goto error;
			}
			result = load_font_section();
			if ( result)
			{
				p_info( PI_ELOG, "%s has error in 'FONT SECTION'\n", psf_name);
				goto error;
			}
			fonts_found = 1;
		}						/* end font section */	
		else if( !memcmp( bp, "BASELINE_ACCENT_EXCEPTION", 25))
			load_base_accent_excepts();
		else if( !memcmp( bp, "TRANSLATION TABLE NAME", 22))
		{
			result = load_translation_table();
			if ( result)
				continue;		/* table not referenced, not loaded */
			translation_table_found = 1;
		}						/* end translation_table section */	
		else if( !memcmp( bp, "ST", 2))
			load_prologue_section();
		else if( !memcmp( bp, "FM", 2))
			load_fmchar_section();
	}							/* end while line != NULL */
	if ( !pagesize_found)
	{
		p_info( PI_ELOG, "%s has no 'PAGESIZE AND DEFAULTS SECTION'\n", psf_name);
		goto error;
	}
	if ( !fonts_found)
	{
		p_info( PI_ELOG, "%s has no 'FONT SECTION'\n", psf_name);
		goto error;
	}
	if ( !translation_table_found)
	{
		p_info( PI_ELOG, "%s has no 'TRANSLATION TABLE NAME'\n", psf_name);
		goto error;
	}
	p_close( fp);
	if( PsfExcept)
	{							/* check for no exceptions in font */
		for (j=0; j<16; j++)
		{
			result = 0;
			fnt_ndx = 256 * j;
			if ( PsfExcept -> ex_font_nbr[j] )
			{
				for ( k=0; k<256; k++)
				{
					if ( PsfExcept -> ex_font_loc[k + fnt_ndx])
					{
						result = 1;
						break;
					}
				}
				if (!result)
					PsfExcept -> no_ex_in_font[j] = 1;
			}					/* end if font number */
			else
				break;
		}						/* end for(j=0;j<16;j++) */
	}							/* end check for no exceptions */
	
	return 1;

error:
	if( fp)
		p_close( fp);
	Psftable_unload();

	return 0;
}

void Psftable_unload( void)
{
	if( PsfCode)
	{
		p_free( (char*)PsfCode);
		PsfCode = NULL;
	}

	if( PentaToPsf)
	{
		PENTA_TO_PSF *pps;
		pps = &PentaToPsf[MaxPentaFont+1];
		while( --pps > PentaToPsf)
			if( pps->name)
				p_free( (char*)pps->name);
		p_free( (char*)PentaToPsf);
		PentaToPsf = NULL;
		MaxPentaFont = 0;
	}

	if( PsfDefault)
	{
		p_free( (char*)PsfDefault);
		PsfDefault = NULL;
	}

	if( PsfSendp)
	{
		p_free( (char*)PsfSendp);
		PsfSendp = NULL;
		MaxSendp = 0;
	}

	if (PsfExcept)
	{
		p_free( (char*)PsfExcept);
		PsfExcept = NULL;
	}

	if ( PsfPrologue)
	{
		p_free( PsfPrologue);
		PsfPrologue = NULL;
		PsfPrologueLineCount = 0;
	}
}
