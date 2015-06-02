#ifndef _P_LIB_H

#define _P_LIB_H

#include <sys/types.h>		/* Defines types used in sys/stat.h */
#include <sys/stat.h>		/* Defines struct stat */
#include <stdio.h>
#include <string.h>			/* defines routine strcmp() */
#include "penta.h"
#include "byte_swap.h"		/* Needed by anyone who calls p_read/p_write */

/* Log file return code */

#define CANT_OPEN_LOG	-3
#define LOG_EMPTY		-2
#define NO_LOG			-1
#define LOG_DISPLAYED	1
#define LOG_EXISTS		2
/* p_open_buffered buffering style id's */

#define P_NO_BUFFERING			0
#define P_READ_FROM_MEMORY		1
#define P_WRITE_TO_MEMORY		2
#define P_DISK_MEMORY_BUFFERS	3

#ifndef P_SUCCESS
#define P_SUCCESS 1
#endif
#ifndef P_FAILURE
#define P_FAILURE 0
#endif

#define NO_PAGE_NUMBER	"0"		/* Page number is zero */

/* Bad return from p_open, p_opendir, ... */

#define P_ERROR 0

#define STREQ(a,b) (strcmp(a,b) == 0)

/* Add one for NULL */
#define STRDUP(a) (strcpy(p_alloc(strlen(a)+1),a))

#define P_CLEAN(a) \
	{ \
	char *aaa;				\
	aaa = strchr(a, '\015');\
	if (aaa)				\
		*aaa = 0;			\
	aaa = strchr(a, '\012');\
	if (aaa)				\
		*aaa = 0;			\
	}

typedef int Pfd;
								/* define where stdin,stdout,stderr
									are stored in PFILES in p_lib.c */
#define P_STDIN		1
#define P_STDOUT	2
#define P_STDERR	3

								/* do not change order of next 3
									defines since MAX_SUBDIRPATH uses
									MAX_NAME and MAX_SUBDIRLEVELS */
#define MAX_NAME			32
#define MAX_SUBDIRLEVELS	1
#define MAX_SUBDIRPATH		(MAX_NAME * MAX_SUBDIRLEVELS)
#define PMAXHOSTNAMELEN		64

#define P_LEVELS			3		/* Tree, SubDir, FileName */
#define SZ_PAGE_STRING		16

/* Macros used in calling p_read p_write */

#define REC_SIZE		512
#define NEXT_REC		-1

#define DIR_SEP			'/'
#define NO_STRING		""

/* defines for the directory argument in some p_lib.c funcs		
	most notably p_path() */

#define OTHER_FILE			 0
#define TEXT_FILE			 1
#define NEW_TEXT_FILE		 2
#define LAYOUT_FILE			 3
#define LIST_FILE			 4
#define FO_FILE				 5
#define MAGERR_FILE			 8
#define OUTPUT_FILE			 9
#define FORMAT_FILE			10
#define USERDATA			11
#define PENTADATA			12
#define CONTROL_LISTS		13
#define GRAPHICS			14
#define GRAPHICS_FILE		15
#define DESK				17
#define DRAWER				18
#define SERVERS				19
#define DOTDATAFILE			20
#define MAP_FILE			21
#define COLOR_TABLE			22
#define GENTAG_FILE			23
#define MP_DATA_FILE		24
#define PM_FILE				25
#define GLOBAL_STF_FILE		26
#define LOCAL_STF_FILE		27
#define GLOBAL_VJT_FILE		28
#define LOCAL_VJT_FILE		29
#define LFO_FILE			30
#define LMAP_FILE			31
#define TABLES_FILE			32
#define NEW_FO_FILE			33
#define NEW_LFO_FILE		34
#define ILL_FILE			35
#define MP_DESIGN_FILE		36
#define MASTER_FILE			37
#define PIL_FILE			38
#define MPIL_FILE			39
#define IL_STYLE_FILE		41
#define BR_STYLE_FILE		42
#define CH_STYLE_FILE		43
#define FN_STYLE_FILE		44
#define SN_STYLE_FILE		45
#define VS_STYLE_FILE		46
#define LOG_FILE			47
#define PEX_FILE_TEMP		48
#define PEX_FILE			49
#define LPEX_FILE			50
#define PREV_FO_FILE		51
#define OLD_FO_FILE			52
#define PREV_LFO_FILE		53
#define OLD_LFO_FILE		54
#define GRAPHICS_FILE_OPEN	55  /* Used to create a file anyway, without stat test */
#define FORMAT_FILE2		56
#define GENTAG_FILE2		57
#define USERDATA2			58
#define COLTABLE            59
#define COLOR_PALETTE       60
#define PELEM_FILE			61
#define LAYOUT_BACKUP		62	/* These 6 append ".vX" to file, where X is the */
#define LFO_BACKUP			63	/*  number found in global p_BackupVersion.  */
#define LMAP_BACKUP			64
#define TEXT_BACKUP			65
#define FO_BACKUP			66
#define MAP_BACKUP			67
#define EPS_FILE			68
#define VARIABLES_FILE		69
#define TRACK_FILE			70

/* defines for some filenames that can be used for p_lib calls */

#define DICTIONARY_INDEX_FILE	"diction"	/* in userdata */
#define DICTION_ASCII			"diction.ascii"	/* userdata/ascii */
#define FORMAT_FILE_NAME		".format"	/* in project dir */
#define FORMAT_FILE2_NAME		".format2"	/* in project dir */
#define GENTAG_FILE_NAME		".gentag"	/* in project dir */
#define GENTAG_FILE2_NAME		".gentag2"	/* in project dir */
#define GENTAG_ASCII			".gentag.ascii"
#define STANDARDS_FILE			"standards"	/* userdata */
#define UNIV_FORMAT_FILE_NAME	"uniformat"	/* in userdata/postscript*/
#define UNIV_FORMAT_FILE2_NAME	"uniformat2" /* in userdata/postscript*/
#define UNIV_GENTAG_FILE_NAME	"universal" /* in userdata */
#define UNIV_GENTAG_FILE2_NAME	"universal2" /* in userdata */
#define MP_DATA_FILE_NAME		"mp_data"	/* in pproject dir */
#define MP_DESIGN_FILE_NAME		"mp_design"	/* in pproject dir */


/* define default name for color table. */

#define DEF_COLOR_TABLE	"colortable"

/* defines for the directory names used by some p_lib.c funcs
	most notably p_path() */

#define ASCII_DIR		"ascii"
#define DEFAULT_DIR		".default"
#define DRAWER_DIR		".drawer"
#define GRAPHICS_DIR	"graphics"
#define JOBS_DIR		"desks"
#define LISTS_DIR		"control_lists"
#define PDATA_DIR		"pentadata"
#define TABLES_DIR		"tables"
#define USER_DIR		"userdata"

/* defines for file extensions used in some p_lib.c funcs */
/* defines for list files in list_control.h */

#define COLOR_PALETTE_EXT	".cl"
#define COLOR_TABLE_EXT		".ct"
#define OLD_FO_EXT			".fo3"
#define PREV_FO_EXT			".fo4"
#define FO_EXT				".fo"
#define OLD_LFO_EXT			".lfo3"
#define PREV_LFO_EXT		".lfo4"
#define LFO_EXT				".lfo"
#define NEW_FO_EXT			".nfo"
#define NEW_LFO_EXT			".nlfo"
#define GRAPHIC_SHAPE_EXT	".gs"
#define LAYOUT_EXT			".lay"
#define LOGFILE_EXT			".log"
#define MAGERR_EXT			".me"
#define MAP_EXT				".map"
#define LMAP_EXT			".lmap"
#define OUTPUT_EXT			".tp"
#define EPS_EXT				".eps"
#define PM_FILE_EXT			".pm"
#define STF_EXT				".stf"
#define TEXT_EXT			".txt"
#define NEW_TEXT_EXT		".nf"
#define VJT_EXT				".vjt"
#define ILL_EXT				".ilp"
#define ILS_EXT				".ils"
#define BRS_EXT				".brs"
#define CHS_EXT				".chs"
#define FNS_EXT				".fns"
#define SNS_EXT				".sns"
#define VSS_EXT				".vss"
#define PEX_TEMP_EXT		".temp_pex"
#define PEX_EXT				".pex"
#define LPEX_EXT			".lpex"
#define PIL_EXT				".pil"
#define MPIL_EXT			".mpil"
#define PIX_EXT				".pixel"
#define PELEM_EXT			".pelem"
#define BACKUP_EXT			".v"	/* + number in global p_BackupVersion  */
#define VARIABLES_EXT		".vars"
#define TRACK_EXT			".track"

/* defines for certain directory extensions */

#define GALLEY_DIR_EXT		".gal"
#define PROJECT_DIR_EXT		".prj"

/* define for the filename that contains the userdata subdir name */

#define DATA_PATH		".data"

/* Seek relative to the beginning of the file. */

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

/* Seek relative to the current position in the file. */

#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif

/* Seek relative to the end of the file. */

#ifndef SEEK_END
#define SEEK_END	2
#endif

/* valid values for first arg ('level') to p_info() */
#define PI_ERROR	(1<<0)
#define PI_ERRNO	(1<<1)
#define PI_WARN		(1<<2)
#define PI_INFO		(1<<3)
#define PI_TRACE	(1<<4)
#define PI_ABORT	(1<<5)
#define PI_NOFMT	(1<<19)
#define PI_LOG		(1<<20)
#define PI_ELOG		(PI_LOG | PI_ERROR)
#define PI_ENLOG	(PI_LOG | PI_ERRNO)
#define PI_WLOG		(PI_LOG | PI_WARN)
#define PI_ILOG		(PI_LOG | PI_INFO)
#define PI_TLOG		(PI_LOG | PI_TRACE)
#define PI_ALOG		(PI_LOG | PI_ABORT)

/****************** prototypes for the p_lib.c functions *************/

extern long p_page_atoi(char *ptr);
extern void *p_alloc(int size);
extern void p_close(Pfd fd_id);
extern int p_closedir(Pfd fd_id);
extern int p_copy(char *from_tree, int from_dir, char *from_sub_dir,
		   char *from_fname, char *to_tree,
		   int to_dir, char *to_sub_dir, char *to_fname);
extern void p_info (int level, char *msg, ...);
extern void p_error(char *, ...);
extern void p_warning(char *, ...);
extern char *itoa(int,char *);
extern int p_feof(Pfd fd);
extern char *p_fgets(char *ptr, int length, Pfd fd_id);
extern char *p_grfgets(char *ptr, int length, Pfd fd_id);
extern int p_filetype(char *tree_name, int dir, char *sub_dir,
											char *fname, int type);
extern void p_fflush(Pfd fd_id);
extern int p_fputs(char *ptr, Pfd fd_id);
extern int p_fprintf (Pfd fd_id, char *format, ...);
extern int p_strfld (char *buf, char **tok, int cnt, int squeeze, char *delim);
extern int p_fseek(Pfd fd_id, long offset);
extern void p_free(char *ptr);
extern int p_get_userdata_name (char *tree, char *path);
extern int p_get_data_name(char *tree, char *subdir, char *path,
													int search_tree);
extern int p_set_data_name(char *tree, char *subdir, char *path);
extern char *p_get_username(void);
extern int p_getc(Pfd fd_id);
extern void p_init(int store_stdio);
extern void p_lib_stat(void);
extern void p_make_lower(char *name);
extern void p_make_upper(char *name);
extern int p_mkdir(char *tree_name, int dir, char *sub_dir,
												char *fname, int mode);
extern Pfd p_open(char *tree_name, int dir, char *sub_dir, char *fname, 
														char *type);
extern Pfd p_opendir(char *tree_name, int dir, char *sub_dir,
														char *fname);
extern Pfd p_open_buffered(char *tree_name, int dir, char *sub_dir,
						   char *fname, char *type, int buf_style,
						   int record_size, int records_per_chunk);
extern int p_nread(Pfd fd_id, unsigned char *buf, int size);
extern int p_nwrite(Pfd fd_id, unsigned char *buf, int size);
extern char *p_path(char *tree_name, int dir, char *sub_dir,
														char *fname);
extern int p_putc(int c, Pfd fd_id);
extern int p_read(char *buffer, int reclength, int numrecs, Pfd fd_id,
												int recnum, int16 bs);
extern char *p_readdir(Pfd fd_id);
extern void *p_realloc(char *ptr, int size);
extern void *p_remalloc(char *oldptr, int oldsize, int newsize);
extern int p_rename(char *tree_name, int olddir, char *oldsub_dir,
			char *oldname, int newdir, char *newsub_dir,char *newname);
extern int p_rename_backup (char *tree_name, int olddir, char *oldsub_dir,
			char *oldname);
extern int p_seek(Pfd, int32, int);
extern int p_stat(char *tree_name, int dir, char *sub_dir, char *fname,
												struct stat *buffer);
extern int p_unlink(char *tree_name, int dir,
											char *sub_dir, char *fname);
extern int p_ungetc(int c, Pfd fd);
extern int p_write(char *buffer, int reclength, int numrecs, Pfd fd_id,
												int recnum, int16 bs);
extern int p_parent_path(char *in_tree, char *in_dir, char *parent,
				 						char *out_tree, char *out_dir);
extern int p_child_path(char *in_tree, char *in_dir, char *child,
				 						char *out_tree, char *out_dir);
extern Pfd p_get_table1(char *tree, char *subdir, char *table_name,
					   char *return_path);
extern Pfd p_get_table2(char *tree, char *subdir, char *table_name,
					   char *return_path);

extern char p_InfoMsg[1024];
extern FILE *p_InfoFd;
extern FILE *p_LogFd;
extern int p_LogCnt;
extern int p_LogMaxCnt;

#include "memshell.h"

#endif
