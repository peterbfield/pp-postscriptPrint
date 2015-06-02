/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

/* check-out Psftest.c for an example of using this module */

#ifndef _PSFTABLE_H
#define _PSFTABLE_H

#define PSF_TABLE "psftable"

typedef struct
{
	char *name;
	int code;
	int punt;
	int flag;
}	PENTA_TO_PSF;

typedef struct
{
	char name[12];
	unsigned char code[256];
}	PSF_CODE;

typedef struct
{
	double scale, scalex, scaley, acoff;
	int pagew, pageh;
	int pofft, poffb, poffl, poffr;
	int hdrup;
	int platecolor_flag;
	int fmchar[10];
}	PSF_DEFAULT;

typedef struct
{
	char name[MAX_NAME];
	int cmyk, pagew, pageh, htype, noeof;
}	PSF_SENDP;

typedef struct
{
	int16 ex_font_nbr[16];
	char no_ex_in_font[16];
	char ex_font_loc[256 * 16];
}	PSF_EXCEPTION;

extern PSF_CODE *PsfCode;
extern PENTA_TO_PSF *PentaToPsf;
extern PSF_DEFAULT *PsfDefault;
extern PSF_SENDP *PsfSendp;
extern PSF_EXCEPTION *PsfExcept;
extern char *PsfPrologue;
extern int PsfPrologueLineCount;
extern int MaxPentaFont;
extern int MaxSendp;

/* Load the psftable for project 'tree/proj' into memory.
 * 'proj' must be in the form "project.prj".
 * If not providing name, assumes "psftable" for name.
 * Returns 1 if load was successful; else 0.
 * If load is unsuccessful, all allocated space is dealocated.
 * If load is successful, structures and variables are initialized
 * with the psftable information as follows:
 * PsfDefault	-  defaults portion of PAGESIZE AND DEFAULTS SECTION, + FMCHAR
 * PsfSendp		-  SENDP portion of PAGESIZE AND DEFAULTS SECTION
 * MaxSendp 	-  number of PsfSendp array entries
 * PentaToPsf	-  FONT SECTION
 * MaxPentaFont -  number of valid PentaToPsf entries (i.e., valid
 *					Penta font numbers range form 1-MaxPentaFont).
 * PsfCode		-  TRANSLATION TABLE SECTION
 * To facilitate psftable reloading, Psftable_unload() is called
 * prior to any memory allocation.
 */
extern int Psftable_load( char *tree, char *proj);
extern int Psftable_load_with_name( char *tree, char *proj, char *psf_name);

/* free up memory occupied by PsfCode, PentaToPsf, PsfDefault and PsfSendp */
extern void Psftable_unload( void);


#endif
