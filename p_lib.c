#ifndef CODECENTER
#define ALLOC_TRACE 0x00		/* set to 0x20 */
#else
#define ALLOC_TRACE 0x00		/* Always 0x00 for CodeCenter */
#endif
#define TEST_MEMORY	0			/* if set, add test_mem.c to your source */

#include <stdio.h>				/* Need to be declared before malloc.h */

#ifdef LIB_MALLOC
#include <malloc.h>				/* The better malloc utilities */
#endif 

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>				/* Used for p_get_username */
#include <errno.h>
#include <stdarg.h>
#include "p_lib.h"
#include "traces.h"
  
#ifdef TEST_MEMORY
extern void add_1_entry(int lineno, int32 size, uint32 address);
extern void realloc_1_entry(int lineno, int32 old_size, uint32 old_address,
							int32 new_size, uint32 new_address);
extern void free_1_entry(int lineno, int32 size, uint32 address);
#endif
extern int unlink(const char *path);
extern uid_t getuid();

char p_ErrMsg[1024];
char p_WrnMsg[1024];
char p_InfoMsg[1024];
int p_LogCnt = 0;
int p_LogMaxCnt = 50;
int p_BackupVersion;

FILE *p_LogFd = NULL;

#define UNUSED		0
#define INUSE_FILE	1
#define INUSE_STDIO	2
#define INUSE_DIR	-1

#define MAX_OPEN_FILES 64

#define MOUNT_PT	"/Penta"
#define MAX_PATH_CHARS MAX_NAME * P_LEVELS

#if !MEMSHELL
static void p_print_mallinfo();
#endif

int alloc_call_size = 0;
int alloc_call_count = 0;

static struct P_Files
{
	int in_use;					/* UNUSED = not in use
								   INUSE_FILE = in use for a file
								   INUSE_DIR  = in use for a directory
								   INUSE_STDIO = inuse by stdin,stdout,stderr*/
	DIR *dirp;					/* UNIX directory stream */
	FILE *fd;					/* UNIX file descriptor */
	int pos;					/* current file position */
	char last_op;				/* flag as to last operation done on file */
	char filename[MAXPATHLEN];
	int buf_style;				/* style of buffering used by this file */
	int records_per_chunk;		/* records per chunk */
	int record_size;			/* record size */
	int chunk_size;				/* chunk size */
	int max_chunk;				/* max chunk written */
	int max_rec;				/* max record written */
	int num_chunks;				/* number of chunk pointers allocated */
	char **chunks;				/* list of pointers to chunks of the file */
} PFILES[MAX_OPEN_FILES];


void subgraphic_path(char *tree_name, char *sub_dir, char *fname, char *pathname);
/***************** utilities used herein ****************************/

long p_page_atoi(char *ptr)
{
	return(atol(ptr));
}

/********************************************************************/

void reverse(s)
char s[];
{
	int c,i,j;

	for(i=0, j=strlen(s)-1; i<j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

char *itoa(n, s)
char s[];
int n;
{
	int i, sign;

	if((sign = n) < 0)
		n = -n;
	i = 0;
	do{
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
	return(s);
}

/********************************************************************/

#if !MEMSHELL
void *p_alloc(int size)
{
	char *char_ptr;

	if (size == 0)
	{
		p_info(PI_ELOG, "\nWARNING: call to p_alloc(0)!!\n\n");
		if (trace_error)
			abort();
		return (NULL);
	}
#if ALLOC_TRACE
	alloc_call_size += size;
	size += ALLOC_TRACE;
#endif
	if ((char_ptr = malloc(size)) == NULL) 
	{
		p_info(PI_ELOG, "\n\nERROR: Failed to allocate %d bytes", size);
		p_print_mallinfo();
		return (NULL);
	}
	memset(char_ptr,0,size);
#if ALLOC_TRACE
	*(int *)char_ptr = size;
	char_ptr += ALLOC_TRACE;
#ifdef TRACE
	if (trace_alloc)
	    p_info(PI_TRACE, "p_alloc %4d bytes at loc 0x%6x\n", 
				  size - ALLOC_TRACE, char_ptr);
#endif 
#ifdef TEST_MEMORY
	if (trace_memory)
		add_1_entry(0, size - ALLOC_TRACE, (uint32)char_ptr);
#endif
#endif 
	alloc_call_count++;
	return(char_ptr);
}								/* end p_alloc() */
#endif /* MEMSHELL */

/********************************************************************/

void p_close(Pfd fd_id)
{
	int temp;
	
	if (fd_id == P_ERROR)
	{
		if(trace_8)
			p_info(PI_TRACE, "Trying to close a file with an illegal id\n");
		return;
	}
	if (trace_8) 
		p_info(PI_TRACE, "p_close ==> fd_id: %d, filename: %s\n", 
				  fd_id, PFILES[fd_id].filename);
	switch (PFILES[fd_id].buf_style)
	{
	  case P_READ_FROM_MEMORY:
		break;					/* don't close, just clean up */
	  case P_WRITE_TO_MEMORY:
		break;					/* don't close, just clean up */
	  case P_DISK_MEMORY_BUFFERS:
	  case P_NO_BUFFERING:
	  default:
		{
			if ((fd_id > P_ERROR) && (fd_id < MAX_OPEN_FILES) &&
				(PFILES[fd_id].in_use == INUSE_FILE)) 
			{
				if (fclose(PFILES[fd_id].fd) != 0)
					p_info(PI_ELOG, "Error closing %s (fd_id %d)", 
							PFILES[fd_id].filename, fd_id);
			}
		}
	}							/* end switch buf_style */
	PFILES[fd_id].in_use = UNUSED;
	PFILES[fd_id].fd     = 0;
	PFILES[fd_id].dirp   = 0;
	PFILES[fd_id].filename[0] = '\0';
	PFILES[fd_id].buf_style = P_NO_BUFFERING;
	if(PFILES[fd_id].chunks)
	{
		for(temp = 0; temp < PFILES[fd_id].num_chunks; temp++)
		{
			char *chunk;

			chunk = *(PFILES[fd_id].chunks + temp);
			if(chunk)
				p_free(chunk);
		}
	}
	if( PFILES[fd_id].chunks)
	{
		p_free((char *)PFILES[fd_id].chunks);
		PFILES[fd_id].chunks = 0;
	}
}								/* end p_close() */

/********************************************************************/

int p_closedir(Pfd fd_id)
{
	int return_val=0;

#ifdef TRACE
	if (trace_8) 
		p_info(PI_TRACE, "P_closedir ==> fd_id: %d, filename: %s\n", 
				  fd_id, PFILES[fd_id].filename);
#endif 
	if ((fd_id >= 0) && (fd_id < MAX_OPEN_FILES) && 
		(PFILES[fd_id].in_use == INUSE_DIR))
	{
		return_val=closedir(PFILES[fd_id].dirp);
		PFILES[fd_id].in_use = UNUSED;
		PFILES[fd_id].fd     = 0;
		PFILES[fd_id].dirp   = 0;
		PFILES[fd_id].pos    = 0;
		PFILES[fd_id].filename[0] = 0;
		PFILES[fd_id].buf_style = P_NO_BUFFERING;
	}
	return(return_val);
}								/*end p_closedir() */

/********************************************************************/

int p_copy(char *from_tree, int from_dir, char *from_sub_dir,
		   char *from_fname, char *to_tree,
		   int to_dir, char *to_sub_dir, char *to_fname)
{
	Pfd from_fd, to_fd;
	struct stat s_buffer;
	char *r_buffer;

	if (p_stat(from_tree, from_dir, from_sub_dir, from_fname, &s_buffer) == -1)
		return (P_ERROR);
	if ((from_fd = p_open(from_tree, from_dir, 
						  from_sub_dir, from_fname, "r")) == P_ERROR)
		return (P_ERROR);
	if (!(r_buffer = p_alloc(s_buffer.st_size)))
		return (P_ERROR);
	if (p_read(r_buffer, s_buffer.st_size, 
			   1, from_fd, NEXT_REC, SW_NOSWAP) == P_ERROR)
	{
		p_free(r_buffer);
		return (P_ERROR);
	}
	p_close(from_fd);
	if ((to_fd = p_open(to_tree, to_dir,to_sub_dir, to_fname, "w")) == P_ERROR)
	{
		p_free(r_buffer);
		return (P_ERROR);
	}
	if (p_write(r_buffer, s_buffer.st_size, 1, to_fd, NEXT_REC, SW_NOSWAP)
		== P_ERROR)
	{
		p_free(r_buffer);
		return (P_ERROR);
	}
	p_free(r_buffer);
	p_close(to_fd);
	return(1);
}

/********************************************************************/
/* p_error prints the message provided in the parameter list, as 
   well as the system error message corresponding to errno (if errno is set).
   */

void p_error(char *fmt, ...)
{
    FILE *p_ErrFd = stderr;
    va_list args;
    char msg[256];
    int eno = errno;


	/* Get User error message */
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
	
    /* Print user error message and system error message */
    if (p_ErrFd)
        if( errno)
            fprintf( p_ErrFd, "%s: %s (%d)\n", msg, strerror(eno), eno);
        else
            fprintf( p_ErrFd, "%s\n", msg);
    else
        if( errno)
            sprintf( p_ErrMsg, "%s: %s (%d)", msg, strerror(eno), eno);
        else
            strcpy( p_ErrMsg, msg);
    errno = 0;
}								/* end p_error() */

/********************************************************************/
/* written this way since feof() is implemented 
   as a macro
   returns non zero when EOF was previously been detected
   */

int p_feof(Pfd fd)
{
	int result;
	result = feof(PFILES[fd].fd);
	return(result);
}

/********************************************************************/

char *p_readline(Line,Size,Fd) char *Line; int Size; FILE *Fd; {

        int     I=0;

        if (feof(Fd))
            return (char *)0;

        while (I<Size) {
            Line[I] = fgetc(Fd);
            if (feof(Fd)) {
                Line[I] = 0;
                return Line;
            }
            if (Line[I] == '\n') {	/* Line feed */
                I++;
                break;
            }
            if (Line[I] == '\r') {	/* Carriage return */
                Line[I] = '\n';
                I++;
                break;
            }
            I++;
        }

        Line[I]=0;

        return Line;

}

char *p_grfgets(char *ptr, int length, Pfd fd_id)
{
	if(fd_id == P_ERROR)
		return(0);
	PFILES[fd_id].last_op = 'g'; /* get string */
	return(p_readline(ptr, length, PFILES[fd_id].fd));
}

char *p_fgets(char *ptr, int length, Pfd fd_id)
{
	if(fd_id == P_ERROR)
		return(0);
	PFILES[fd_id].last_op = 'g'; /* get string */
	return(fgets(ptr, length, PFILES[fd_id].fd));
}

/********************************************************************/

int p_filetype(char *tree_name, int dir, char *sub_dir,  char *fname, int type)
{
	struct stat stat_buffer;

	if (p_stat(tree_name, dir, sub_dir, fname, &stat_buffer)) 
		return(FALSE);
	return((stat_buffer.st_mode&S_IFMT) == type);
}								/* end p_filetype() */

/********************************************************************/

void p_fflush(Pfd fd_id)
{
	if(fd_id == P_ERROR)
		return;
	if(PFILES[fd_id]. buf_style == P_WRITE_TO_MEMORY)
	{
		Pfd fd;
		char *chunk;
		int i;
		int no_errors = 1;
		char name_tmp[132];

		sprintf(name_tmp,"%s.tmp",PFILES[fd_id].filename);
		
		if((fd = p_open(NO_STRING, OTHER_FILE, NO_STRING, 
						name_tmp,"w+")) == P_ERROR)
		{
			p_info(PI_ELOG, "unable to create file to flush it\n");
			return;
		}
		for(i = 0; i <= PFILES[fd_id].max_chunk; i++)
		{
			chunk = *(PFILES[fd_id].chunks + i);
			if(chunk != 0)
			{
				if(p_write(chunk,PFILES[fd_id].chunk_size,1,fd,i,SW_NOSWAP)
				   == P_ERROR) no_errors = 0;
				p_free(chunk);
				*(PFILES[fd_id].chunks + i) = 0;
			}
		}
		p_close(fd);
		if(no_errors)
		{
			unlink(PFILES[fd_id].filename);
			rename(name_tmp,PFILES[fd_id].filename);
		}
	}
	else
		fflush(PFILES[fd_id].fd);
}

/********************************************************************/

int p_fputs(char *ptr, Pfd fd_id)
{
	if(fd_id == P_ERROR)
		return(EOF);
	PFILES[fd_id].last_op = 'p'; /* put string */
	return(fputs(ptr, PFILES[fd_id].fd));
}

int p_fprintf (Pfd fd_id, char *fmt, ...)
{
	va_list args;
	int rc;

	if (fd_id == P_ERROR)
		return (EOF);
	PFILES[fd_id].last_op = 'p';		/* put string */
	va_start (args, fmt);
	rc = vfprintf (PFILES[fd_id].fd, fmt, args);
	va_end (args);
	return rc;
}

int p_nread(Pfd fd_id, unsigned char *buf, int size)
{
	if(fd_id == P_ERROR)
		return(EOF);
	PFILES[fd_id].last_op = 'g'; 
	return(fread(buf,1,size,PFILES[fd_id].fd));
}

int p_fseek(Pfd fd_id, long offset)
{
	if(fd_id == P_ERROR)
		return(EOF);	
	PFILES[fd_id].last_op = 'p'; 
	return(fseek(PFILES[fd_id].fd, offset, SEEK_CUR));
}

int p_nwrite(Pfd fd_id, unsigned char *buf, int size)
{
	if(fd_id == P_ERROR)
		return(EOF);
	PFILES[fd_id].last_op = 'p'; 
	return(fwrite(buf,1,size,PFILES[fd_id].fd));
}

/********************************************************************/

#if !MEMSHELL
void p_free(char *ptr)
{
#if ALLOC_TRACE
	int size;
	if(ptr == 0)
		return;
	ptr -= ALLOC_TRACE;
	size = *(int *)ptr;
	alloc_call_size -= (size - ALLOC_TRACE);
#ifdef TRACE
	if (trace_alloc)
		p_info(PI_TRACE, "p_free %4d bytes at loc 0x%6x\n",
				  size - ALLOC_TRACE,ptr + ALLOC_TRACE);
#endif TRACE
#ifdef TEST_MEMORY
	if (trace_memory)
	{
		free_1_entry(0, size - ALLOC_TRACE, (uint32)(ptr + ALLOC_TRACE));
		memset(ptr, 0x55, size);
	}
#endif
#endif 
	if(ptr == NULL)
		return;
	free(ptr);
	alloc_call_count--;
}								/* end p_free() */
#endif /* MEMSHELL */


/********************************************************************/

char *p_get_username()
{
	struct passwd *pwent;
	uid_t uid;
	
	/* Oddly enough, there are no error conditions for this call */
	uid=getuid();
	if ((pwent=getpwuid((int)uid)) == NULL)
	{
		p_info(PI_ELOG, "ERROR accessing passwd entry for %d", (int)uid);
		return (NULL);
	}
	/* Got pw entry ok, so return the name */
	return(pwent->pw_name);
}								/* end p_get_username() */

/********************************************************************/

/********************************************************************* 
  NOTE for p_getc() coded like this since getc is a macro and can not
  be passed as an arg to the function return()
  **********************************************************************/

int p_getc(Pfd fd_id)
{
	int val;

	val = getc(PFILES[fd_id].fd);
	PFILES[fd_id].last_op = 'g';
	return(val);
}

/********************************************************************/

void p_init(int store_stdio)
{
	memset(&PFILES, 0, sizeof(PFILES));
	if(store_stdio)
	{
		PFILES[1].fd = stdin;
		PFILES[1].in_use = INUSE_STDIO;
		PFILES[1].last_op = 'o';
		PFILES[1].pos = 0;
		memset(PFILES[1].filename, 0, MAXPATHLEN);
		PFILES[2].fd = stdout;
		PFILES[2].in_use = INUSE_STDIO;
		PFILES[2].last_op = 'o';
		PFILES[2].pos = 0;
		memset(PFILES[1].filename, 0, MAXPATHLEN);
		PFILES[3].fd = stderr;
		PFILES[3].in_use = INUSE_STDIO;
		PFILES[3].last_op = 'o';
		PFILES[3].pos = 0;
		memset(PFILES[1].filename, 0, MAXPATHLEN);
	}
}

/********************************************************************/

#ifdef TRACE
void p_lib_stat()
{
	int i;
	if (!trace_8)
		return;
	for(i = 0; i < MAX_OPEN_FILES; i++)
	{
		switch(PFILES[i].in_use)
		{
		  case INUSE_FILE:
			p_info(PI_TRACE, "Plib file %3d fd %x dirp %x pos %d name %s\n",
					  i, PFILES[i].fd, PFILES[i].dirp, PFILES[i].pos,
					  PFILES[i].filename);
			break;
			
		  case INUSE_DIR:
			p_info(PI_TRACE, "Plib dir  %3d fd %x dirp %x pos %d name %s\n",
					  i, PFILES[i].fd, PFILES[i].dirp, PFILES[i].pos,
					  PFILES[i].filename);
			break;
			
		  case INUSE_STDIO:
			p_info(PI_TRACE, "Plib stdio %3d fd %x\n",
					  i, PFILES[i].fd);
			
		  default:
			break;
		}
	}
	return;
}
#endif

/********************************************************************/

#ifdef NOT_USED_BY_ANYONE
void p_make_lower(char *name)
{
	while (*name)
		*name++ = (char)tolower((int)*name);
}								/* end p_make_lower() */
#endif 

/********************************************************************/

void p_make_upper(char *name)
{
	while (*name)
	{	
		*name = (char)toupper((int)*name);
		name++;
	}
}								/* end p_make_upper() */


/********************************************************************/

int p_mkdir(char *tree_name, int dir, char *sub_dir, char *fname, int mode)
{
	int return_val;
	char *pathname;

	if (mode==0)
		mode=S_IRWXU|S_IRWXG|S_IRWXO;
#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "in p_mkdir(tree:%s dir:%d sub_dir:%s fname:%s, mode:%o)\n",
				  tree_name ? tree_name : "NULL", dir, 
				  sub_dir ? sub_dir : "NULL", fname ? fname : "NULL", mode);
#endif
	pathname = p_path(tree_name, dir, sub_dir, fname);
	if ((return_val = mkdir(pathname, mode)) != 0) 
		p_info(PI_ELOG, "Cannot create directory: %s", pathname);
	return(return_val);
}								/* end p_mkdir() */

/********************************************************************/
Pfd p_open_buffered(char *tree_name, int dir, char *sub_dir,
					char *fname, char *type, int buf_style, 
					int record_size, int records_per_chunk)
{
	Pfd fd_id;
	int i;
	char *pathname;

	pathname = p_path(tree_name, dir, sub_dir, fname);
	/*
	 *				find an open slot for handling this file
	 */
	fd_id = P_ERROR;
	for(i = 0; i < MAX_OPEN_FILES; i++)
	{
		if(PFILES[i].in_use == UNUSED && i != P_ERROR)
		{
			fd_id = (Pfd)i;
			break;
		}
	}
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "p_open_buffered(): fd_id: %d, pathname: %s\n", 
				  fd_id, pathname);
#endif
	if(fd_id == P_ERROR)
		return(fd_id);			/* no file slots available */
	strcpy(PFILES[fd_id].filename, pathname);
	PFILES[fd_id].in_use = INUSE_FILE;
	PFILES[fd_id].last_op = 'o'; /* open */
	PFILES[fd_id].pos    = 0;
	switch (buf_style)
	{
	  case P_READ_FROM_MEMORY:
		{
			Pfd fd;
			struct stat s_buffer;
			int size;

			if (p_stat(tree_name, dir, sub_dir, fname, &s_buffer) == -1)
				break;
			size = s_buffer.st_size;
			if(size == 0)
				break;
			if ((fd = p_open(tree_name,dir,sub_dir,fname,"r"))==P_ERROR)
				break;	
			PFILES[fd_id].chunks = (char **)p_alloc(sizeof(char *)* 50);
			PFILES[fd_id].num_chunks = 50;
			*PFILES[fd_id].chunks = p_alloc(size);	/* alloc chunk 0 */
			p_read(*PFILES[fd_id].chunks, size, 1, fd, 0, SW_NOSWAP);
			p_close(fd);
			PFILES[fd_id].buf_style = P_READ_FROM_MEMORY;
			PFILES[fd_id].record_size = size;
			PFILES[fd_id].records_per_chunk = 1;
			PFILES[fd_id].chunk_size = size;
			PFILES[fd_id].max_chunk = 0;
			return(fd_id);
		}
	  case P_WRITE_TO_MEMORY:
		PFILES[fd_id].buf_style = P_WRITE_TO_MEMORY;
		PFILES[fd_id].record_size = record_size;
		PFILES[fd_id].records_per_chunk = records_per_chunk;
		PFILES[fd_id].chunk_size = record_size * records_per_chunk;
		PFILES[fd_id].max_chunk = 0;
		PFILES[fd_id].num_chunks = 0;
		PFILES[fd_id].chunks = 0;
		return(fd_id);
	  case P_DISK_MEMORY_BUFFERS:
		{
			struct stat s_buffer;
			int size = 0;

			if (p_stat(tree_name, dir, sub_dir, fname, &s_buffer) != -1)
				size = s_buffer.st_size;
			if((PFILES[fd_id].fd = fopen(pathname,type)) == NULL)
				break;
			PFILES[fd_id].buf_style = P_DISK_MEMORY_BUFFERS;
			PFILES[fd_id].last_op = 'o'; /* open */
			PFILES[fd_id].pos = 0;
			PFILES[fd_id].record_size = record_size;
			PFILES[fd_id].records_per_chunk = records_per_chunk;
			PFILES[fd_id].chunk_size = record_size * records_per_chunk;
			PFILES[fd_id].max_chunk = 0;
			PFILES[fd_id].num_chunks = 0;
			PFILES[fd_id].chunks = 0;
			PFILES[fd_id].max_rec = (size / record_size) - 1;
			return(fd_id);
		}
	  case P_NO_BUFFERING:
	  default:
		{
			if((PFILES[fd_id].fd = fopen(pathname,type)) == NULL)
				break;
			PFILES[fd_id].buf_style = P_NO_BUFFERING;
			PFILES[fd_id].chunk_size = 0;
			PFILES[fd_id].record_size = 0;
			PFILES[fd_id].records_per_chunk = 0;
			return(fd_id);
		}
	}							/* end of switch: break from switch == error */
	
	if(PFILES[fd_id].fd != P_ERROR)
		fclose(PFILES[fd_id].fd);
	PFILES[fd_id].in_use = 0;
	return(P_ERROR);
}								/* end p_open_buffered() */

/********************************************************************/

Pfd p_open(char *tree_name, int dir, char *sub_dir, char *fname, char *type)
{
	Pfd fd_id;
	int i;
	char *pathname;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "in p_open(tree:%s dir:%d sub_dir:%s fname:%s, type:%s)\n",
				  tree_name ? tree_name : "NULL", dir,  
				  sub_dir ? sub_dir : "NULL",
				  fname ? fname : "NULL", type ? type : "NULL");
#endif
	pathname = p_path(tree_name, dir, sub_dir, fname);
	fd_id = P_ERROR;
	/* See if file has already been opened */
	for(i = 0; i < MAX_OPEN_FILES; i++)
	{
		/* Do not use P_ERROR even if it is available */
		if(PFILES[i].in_use == UNUSED && i != P_ERROR)
		{
			fd_id = (Pfd)i;
			break;
		}
	}
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "p_open(): fd_id: %d, pathname: %s\n", fd_id, pathname);
#endif
	/* PFILES structure filled, return error */
	if(fd_id == P_ERROR)
		return(fd_id);
	strcpy(PFILES[fd_id].filename, pathname);
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "p_open %s, %d, %s, %s, -> fopen %s\n", 
				  tree_name ? tree_name : "NULL", dir,  
				  sub_dir ? sub_dir : "NULL",
				  fname ? fname : "NULL", type ? type : "NULL", 
				  pathname ? pathname : "NULL", type ? type : "NULL");
#endif
	if((PFILES[fd_id].fd = fopen(pathname,type)) == NULL)
	{
#ifdef TRACE
		if(trace_8)
			p_info(PI_TRACE, "fopen(%s, %s) FAILED", pathname, type);
#endif
		return(P_ERROR);
	}
	PFILES[fd_id].in_use = INUSE_FILE;
	PFILES[fd_id].last_op = 'o'; /* open */
	PFILES[fd_id].pos = 0;
	PFILES[fd_id].buf_style = P_NO_BUFFERING;
	return(fd_id);
}								/* end p_open() */

/********************************************************************/

Pfd p_opendir(char *tree_name, int dir, char *sub_dir, char *fname)
{
	Pfd fd_id;
	int i;
	char *pathname;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "in p_opendir(tree:%s dir:%d sub_dir:%s fname:%s)\n",
				  tree_name ? tree_name : "NULL", dir,
				  sub_dir ? sub_dir : "NULL", fname ? fname : "NULL");
#endif
	pathname = p_path(tree_name, dir, sub_dir, fname);
	fd_id = P_ERROR;
	for(i = 0; i < MAX_OPEN_FILES; i++) 
	{
		if(PFILES[i].in_use == UNUSED && i != P_ERROR) 
		{
			fd_id = i;
			break;
		}
	}
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "p_opendir fd_id: %d, fname: %s\n", fd_id, fname);
#endif 
	if(fd_id == P_ERROR)
		return(fd_id);
	strcpy(PFILES[fd_id].filename, pathname);
	if((PFILES[fd_id].dirp = opendir(pathname)) == NULL) 
	{
#ifdef TRACE
		if (trace_8)
			p_info(PI_TRACE, "Could not open %s", pathname);
#endif
		return(P_ERROR);
	}
	PFILES[fd_id].in_use = INUSE_DIR;
	PFILES[fd_id].pos    = 0;
	PFILES[fd_id].buf_style = P_NO_BUFFERING;
	return(fd_id);
}								/* end p_opendir()*/

Pfd p_get_table1(char *tree, char *subdir, char *table_name, char *return_path)
{
	Pfd fd_id;
	char dir_name[MAX_NAME];

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "p_get_table1 tree: %s, subdir: %s, table: %s \n",
				  tree, subdir, table_name);
#endif
	if (!(subdir && *subdir))
		return(P_ERROR);
	if (strcmp(subdir, TABLES_DIR)) {
		sprintf(dir_name, "%s%s", subdir, PROJECT_DIR_EXT);
		fd_id = p_open(tree, COLOR_TABLE, dir_name, table_name, "r");
	}
	else
		fd_id = p_open(tree, COLOR_TABLE, NULL, table_name, "r");
	if ( (fd_id != P_ERROR) && return_path)
		strcpy(return_path,PFILES[fd_id].filename);
	return(fd_id);
}

Pfd p_get_table2(char *tree, char *subdir, char *table_name, char *return_path)
{
	Pfd fd_id;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "p_get_table2 tree: %s, subdir: %s, table: %s \n",
				  tree, subdir, table_name);
#endif
	if (!(subdir && *subdir))
		return(P_ERROR);
	fd_id = p_open(tree, TABLES_FILE, COLOR_TABLE_EXT, table_name, "r");
	if (fd_id && return_path)
		strcpy(return_path,PFILES[fd_id].filename);
	return(fd_id);
}

int p_get_userdata_name (char *tree, char *path)
{
	Pfd fd_id;

#ifdef TRACE
	if (trace_8)
		p_info (PI_TRACE, "get_userdata_name tree: %s, path: %s\n",
				tree, path);
#endif

	fd_id = p_open (tree, USERDATA, NO_STRING, DATA_PATH, "r");
	if (fd_id == P_ERROR)
		return (-1);
	path = p_fgets (path, MAX_NAME, fd_id);
	p_close (fd_id);
	if (path == NULL)
		return (-1);
	P_CLEAN (path);
	return (0);
}

int p_get_data_name(char *tree, char *subdir, char path[], int search_tree)
{
	Pfd fd_id;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "get_data_name tree: %s, subdir: %s, \
path: %s search:%d\n",
				  tree,subdir,path,search_tree);
#endif
	/* Open the data file, if found */
	if (!(subdir && *subdir))
		return(-1);
	if((fd_id=p_open(tree,DOTDATAFILE,subdir,NO_STRING,"r")) == P_ERROR)
	{
		if(!search_tree)
			return(-1);
		if((fd_id = p_open(tree, USERDATA, NO_STRING, DATA_PATH, "r")) 
		   == P_ERROR)
			return(-1);
	}
	if(p_fgets(path,MAX_NAME,fd_id) == NULL)
	{
		p_close(fd_id);
		return(-1);
	}
	p_close(fd_id);
	P_CLEAN(path);
	return(0);
}

/*********************************************************************/

int p_set_data_name(char *tree, char *subdir,char *path)
{
	Pfd fd_id;

	if (trace_8)
		p_info(PI_TRACE, "set_data_name tree: %s, subdir: %s, path: %s\n",
				  tree,subdir,path);
	if (!(subdir && *subdir))
		return(-1);
	if ((fd_id=p_open(tree,DOTDATAFILE,subdir,NO_STRING,"w+")) == P_ERROR)
		return(-1);
	if(p_fputs(path,fd_id) == EOF)
	{
		p_close(fd_id);
		return(-1);
	}
	p_close(fd_id);
	return(0);
}

/*
  input:                                      |   output:
  tree    dir                   parent        |   tree    dir
  --------------------------------------------|-------------------
  maple   abc.prj/mp_data.prj   abc.prj       |   maple   abc.prj
  maple   mp_data.prj           abc.prj       |   maple   abc.prj
  maple   mp_data.prj           /abc/xyz.prj  |   abc     xyz.prj
  */

int p_parent_path(char  *in_tree, char  *in_dir, char *parent,
				  char *out_tree, char *out_dir)
{
	char *po;
	char *pi;

	if(in_tree != out_tree)
		strcpy(out_tree, in_tree);
	if(in_dir != out_dir)
		strcpy(out_dir, in_dir);
	if(*parent == '.')			/* ../dir */
	{
		if((pi = strchr(parent, '/')) == 0)
			return(1);			/* unknown parent construction */
		pi++;					/* point to first '/' + 1 */
		if((po = strrchr(out_dir, '/')) != NULL)
			po++;				/* point to last '/' + 1 */
		else
			po = out_dir;		/*  or beginning of string */
		strcpy(po,pi);	/* update dir name */
	}
	else if(*parent == '/')		/* /tree/dir */
	{
		if((pi = strchr(parent+1,'/')) == 0)
			return(1);			/* unknown parent construction */
		strncpy(out_tree,parent+1,pi - parent);
		*(out_tree + (pi - parent - 1)) = 0;
		pi++;					/* point to first '/' + 1 */
		strcpy(out_dir,pi);
	}
	else						/* dir */
	{
		strcpy(out_dir,parent);
	}
	if(strcmp(out_dir + strlen(out_dir) - 4, ".prj"))
		strcat(out_dir,".prj");
	return(0);
}

/*
  input:                                |   output:
  tree     dir           child          |   tree    dir
  --------------------------------------|---------------------------
  maple    mp_data.prj   abc.prj        |   maple   mp_data.prj/abc.prj
  maple    mp_data.prj   /abc/xyz.prj   |   abc     xyz.prj
  maple    mp_data.prj   ../xxx.prj     |   maple   xxx.prj
  */

int p_child_path(char  *in_tree, char  *in_dir, char *child,
				 char *out_tree, char *out_dir)
{
	char *po;
	char *pi;

	if(in_tree != out_tree)
		strcpy(out_tree, in_tree);
	if(in_dir != out_dir)
		strcpy(out_dir, in_dir);
	if(*child == '.')			/* ../dir */
	{
		if((pi = strchr(child, '/')) == 0)
			return(1);			/* unknown child construction */
		pi++;					/* point to first '/' + 1 */
		
		if((po = strrchr(out_dir, '/')) != NULL)
			po++;				/* point to last '/' + 1 */
		else
			po = out_dir;		/*  or beginning of string */
		
		strcpy(po,pi);	/* update dir name */
	}
	else if(*child == '/')		/* /tree/dir */
	{
		if((pi = strchr(child+1,'/')) == 0)
			return(1);			/* unknown child construction */
		strncpy(out_tree,child+1,pi - child);
		*(out_tree + (pi - child - 1)) = 0;
		pi++;					/* point to first '/' + 1 */
		strcpy(out_dir,pi);
	}
	else						/* dir */
	{
		strcat(out_dir,"/");
		strcat(out_dir,child);
	}
	if(strcmp(out_dir + strlen(out_dir) - 4, ".prj"))
		strcat(out_dir,".prj");
	return(0);
}

/*ARGSUSED*/
void subgraphic_path(char *tree_name, char *sub_dir, char *fname, char *pathname) {
	char treecopy[MAX_PATH_CHARS];
	char *ptr, subdir1[MAX_NAME], subdir2[MAX_NAME];
	int len;
/* We are dealing with subgraphic directories! */
/* Copy & Parse the directory */

	strcpy(treecopy, tree_name + 1);
	len = strlen(treecopy);
	if (treecopy[len-1] == DIR_SEP)
		treecopy[len-1] = '\0'; /* Trim the last Dir_SEP */
	ptr = strchr(treecopy, DIR_SEP);
	if (ptr) { /* There is a subdir */
		strncpy(subdir1, treecopy, ptr - treecopy);
		subdir1[ptr - treecopy] = '\0';
		strcpy(subdir2, ptr+1);
	}
	else {
		strcpy(subdir1, treecopy);
		subdir2[0] = '\0';
	}
	/* Create a pathname */
	sprintf(pathname, "%s/%s/%s/%s%s/%s/%s%s", MOUNT_PT,subdir1, JOBS_DIR, subdir2,
			PROJECT_DIR_EXT, GRAPHICS_DIR, fname, PIX_EXT);
}

/********************************************************************/
/* form for a file path
   /Penta/tree_name/dir/[sub_dir[.xxx]]/[.drawer]/fname[.xx[x]]
   
   tree_name = mounted directory
   dir = see switch of defines (p_lib.h) below
   sub_dir = file path, in some case ex. width file may be
   the .20 extension
   [.xxx]	= .gal for galleys, .prj for projects
   .drawer = only for files contained in a drawer ex. .fo
   fname = filaname
   [xx.[x]] = files will have 2 or 3 char extesion
   */

char *p_path(char *tree_name, int directory, char *sub_dir, char *filename)
{
	char *fname;
	char psi_name[MAXPATHLEN];
	static char pathname[MAXPATHLEN];

	fname = filename;
	/* Set the directory where all penta files are located */
	sprintf(psi_name, "%s/%s", MOUNT_PT, tree_name);
	switch (directory)
	{
	  case TEXT_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
					  psi_name, JOBS_DIR, sub_dir, fname, TEXT_EXT);
		break;
	  case MPIL_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, MPIL_EXT);
		break;
	  case PIL_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, PIL_EXT);
		break;
	  case NEW_TEXT_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
					  psi_name, JOBS_DIR, sub_dir, fname, NEW_TEXT_EXT);
		break;
		
	  case LOG_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s", 
				psi_name, JOBS_DIR, sub_dir, fname, LOGFILE_EXT);
		break;
	  case LAYOUT_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, LAYOUT_EXT);
		break;
	  case VARIABLES_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, VARIABLES_EXT);
		break;
	  case TRACK_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
					  psi_name, JOBS_DIR, sub_dir, fname, TRACK_EXT);
		break;
	  case LIST_FILE:
		sprintf(pathname, "%s/%s/%s/%s", psi_name, JOBS_DIR, sub_dir, fname);
		break;
	  case FO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, FO_EXT);
		break;
	  case PREV_FO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, PREV_FO_EXT);
		break;
	  case OLD_FO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR,  fname, OLD_FO_EXT);
		break;
	  case NEW_FO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, NEW_FO_EXT);
		break;
	  case MAP_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, MAP_EXT);
		break;
	  case MAGERR_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, MAGERR_EXT);
		break;
	  case OUTPUT_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, OUTPUT_EXT);
		break;
	  case EPS_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, EPS_EXT);
		break;
	  case PM_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, PM_FILE_EXT);
		break;
	  case FORMAT_FILE:
		sprintf(pathname, "%s/%s/%s/%s",
				psi_name, JOBS_DIR, sub_dir, FORMAT_FILE_NAME);
		break;
	  case GENTAG_FILE:
		sprintf(pathname, "%s/%s/%s/%s",
				psi_name, JOBS_DIR, sub_dir, GENTAG_FILE_NAME);
		break;
	  case USERDATA:	
		/* in this case sub_dir contains the .data name (ex. postscript) */
	    {
			struct stat buffer;

			if(sub_dir && *sub_dir)
				sprintf(pathname, "%s/%s/%s/%s",
						psi_name, USER_DIR, sub_dir, fname);
			else
				sprintf(pathname, "%s/%s/%s", psi_name, USER_DIR, fname);
			if(stat(pathname,&buffer) != 0)
			{
				if(sub_dir && *sub_dir)
					sprintf(pathname, "%s/%s/%s/%s/%s",
							MOUNT_PT, DEFAULT_DIR, USER_DIR, sub_dir, fname);
				else
					sprintf(pathname, "%s/%s/%s/%s",
							MOUNT_PT, DEFAULT_DIR, USER_DIR, fname);
			}
			break;
		}
	  case USERDATA2:
		/* USERDATA2 reverts to DEFAULT_DIR if the
		 *   requested data **DIRECTORY** doesen't exist,
		 * whereas USERDATA reverts to DEFAULT_DIR if the
		 *   requested data **FILE** doesn't exist.
		 * Use USERDATA2 to create a new file in an existing
		 * non-default data directory.
		 */
	    {
			struct stat buffer;

			if(sub_dir && *sub_dir)
				sprintf(pathname, "%s/%s/%s/",
						psi_name, USER_DIR, sub_dir);
			else
				sprintf(pathname, "%s/%s/", psi_name, USER_DIR);
			if(stat(pathname,&buffer) != 0)
			{
				if(sub_dir && *sub_dir)
					sprintf(pathname, "%s/%s/%s/%s/",
							MOUNT_PT, DEFAULT_DIR, USER_DIR, sub_dir);
				else
					sprintf(pathname, "%s/%s/%s/",
							MOUNT_PT, DEFAULT_DIR, USER_DIR);
			}
			strcat(pathname, fname);
			break;
		}
	  case PENTADATA:
		/* in this case sub_dir contains the file extension */
		if(sub_dir && *sub_dir)
			sprintf(pathname, "%s/%s/%s.%s", 
					psi_name, PDATA_DIR, fname, sub_dir);
		else
			sprintf(pathname, "%s/%s/%s", psi_name, PDATA_DIR, fname);
		break;
	  case CONTROL_LISTS:
		/* sub_dir may contain the list  extension */
		if(sub_dir && *sub_dir)
			sprintf(pathname, "%s/%s/%s.%s", 
					psi_name, LISTS_DIR, fname, sub_dir);
		else	
			sprintf(pathname, "%s/%s/%s", psi_name, LISTS_DIR, fname);
		break;
	  case GRAPHICS:
	  case GRAPHICS_FILE_OPEN:/* Opens file even if it does not exist */
		{
			struct stat buffer;
			if (tree_name[0] == DIR_SEP) 
			{
/* We are dealing with subgraphic directories! */
/* Copy & Parse the directory */
				subgraphic_path(tree_name, sub_dir, fname, pathname);
				if(directory !=GRAPHICS_FILE_OPEN && stat(pathname,&buffer) != 0)
				{
					sprintf(pathname, "%s/%s/%s/%s%s", MOUNT_PT, DEFAULT_DIR,
							GRAPHICS_DIR, fname, PIX_EXT);
				}
			}
			else if (!strcmp(tree_name, ".")) 
			{
				sprintf(pathname, "%s/%s%s", tree_name, fname, PIX_EXT);
			}
			else if (fname && *fname == '\0') 
			{
/* We need a graphic directory, not a graphic file */
				if (sub_dir && *sub_dir)
					sprintf(pathname, "%s/%s/%s%s/%s", psi_name, JOBS_DIR, sub_dir,PROJECT_DIR_EXT,GRAPHICS_DIR);
				else
					sprintf(pathname, "%s/%s/", 
							psi_name, GRAPHICS_DIR);
			}
			else {
				if (sub_dir && *sub_dir)
					sprintf(pathname, "%s/%s/%s/%s/%s%s", 
							psi_name, JOBS_DIR, sub_dir, GRAPHICS_DIR, fname, PIX_EXT);
				else
					sprintf(pathname, "%s/%s/%s%s", 
							psi_name, GRAPHICS_DIR, fname, PIX_EXT);
				if(directory !=GRAPHICS_FILE_OPEN &&  stat(pathname, &buffer) != 0)
				{
					sprintf(pathname, "%s/%s/%s%s", 
							psi_name, GRAPHICS_DIR, fname, PIX_EXT);
					if(stat(pathname, &buffer) != 0)
						sprintf(pathname, "%s/%s/%s/%s%s",MOUNT_PT,DEFAULT_DIR,
								GRAPHICS_DIR, fname, PIX_EXT);
				}
			}
			break;
		}
	  case COLTABLE:
		if(sub_dir && *sub_dir)
			sprintf(pathname, "%s/%s/%s/%s",
					psi_name, JOBS_DIR, sub_dir, fname);
		else
			sprintf(pathname, "%s/%s/%s", MOUNT_PT, DEFAULT_DIR, TABLES_DIR);
		break;
	  case COLOR_PALETTE:
		if(fname && *fname)
			sprintf(pathname, "%s/%s/%s/%s%s",
					MOUNT_PT, DEFAULT_DIR, USER_DIR, fname, COLOR_PALETTE_EXT);
		else
			sprintf(pathname, "%s/%s/%s",
					MOUNT_PT, DEFAULT_DIR, USER_DIR);
		break;
	  case DESK:
		if(sub_dir && *sub_dir)
			sprintf(pathname, "%s/%s/%s/%s",
					psi_name, JOBS_DIR, sub_dir, fname);
		else
			sprintf(pathname, "%s/%s/%s", psi_name, JOBS_DIR, fname);
		break;
	  case DRAWER:
		if(sub_dir && *sub_dir)
			sprintf(pathname, "%s/%s/%s/%s/%s",
					psi_name, JOBS_DIR, sub_dir, fname, DRAWER_DIR);
		else
			sprintf(pathname, "%s/%s/%s/%s",  
					psi_name, JOBS_DIR, fname, DRAWER_DIR);
		break;
	  case SERVERS:
		sprintf(pathname, "%s/", MOUNT_PT);
		break;
	  case TABLES_FILE:
		{
			struct stat buffer;

			if(sub_dir && *sub_dir)
				sprintf(pathname, "%s/%s/%s%s", 
						psi_name, TABLES_DIR, fname, sub_dir);
			else
				sprintf(pathname, "%s/%s/%s", psi_name, TABLES_DIR, fname);
			if(stat(pathname,&buffer) != 0)
			{
				if(sub_dir && *sub_dir)
					sprintf(pathname, "%s/%s/%s/%s%s", MOUNT_PT, DEFAULT_DIR,
							TABLES_DIR, fname, sub_dir);
				else
					sprintf(pathname, "%s/%s/%s/%s", 
							MOUNT_PT, DEFAULT_DIR, TABLES_DIR, fname);
			}
			break;
		}
	  case DOTDATAFILE:
		sprintf(pathname, "%s/%s/%s/%s",psi_name,JOBS_DIR, sub_dir, DATA_PATH);
		break;
		
	  case COLOR_TABLE:
		if (sub_dir && *sub_dir)
			sprintf(pathname, "%s/%s/%s/%s%s",
					psi_name, JOBS_DIR, sub_dir, fname, COLOR_TABLE_EXT);
		else
			sprintf(pathname, "%s/%s/%s/%s%s",
					MOUNT_PT, DEFAULT_DIR, TABLES_DIR,fname, COLOR_TABLE_EXT);
		break;
	  case MP_DATA_FILE:
		sprintf(pathname, "%s/%s/%s/%s",
				psi_name, JOBS_DIR, sub_dir, MP_DATA_FILE_NAME);
		break;
	  case MP_DESIGN_FILE:
		sprintf(pathname, "%s/%s/%s/%s",
				psi_name, JOBS_DIR, sub_dir, MP_DESIGN_FILE_NAME);
		break;
	  case IL_STYLE_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, ILS_EXT);
		break;
	  case BR_STYLE_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s", 
				psi_name, JOBS_DIR, sub_dir, fname, BRS_EXT);
		break;
	  case CH_STYLE_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, CHS_EXT);
		break;
	  case FN_STYLE_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, FNS_EXT);
		break;
	  case SN_STYLE_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, SNS_EXT);
		break;
	  case VS_STYLE_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, VSS_EXT);
		break;
	  case GLOBAL_STF_FILE:
		if (!fname || STREQ(fname, NO_STRING))
			sprintf(pathname, "%s/%s", psi_name, TABLES_DIR);
		else
			sprintf(pathname, "%s/%s/%s%s", 
					psi_name, TABLES_DIR, fname, STF_EXT);
		break;
	  case LOCAL_STF_FILE:
		if (!fname || STREQ(fname, NO_STRING))
			sprintf(pathname, "%s/%s/%s", psi_name, JOBS_DIR, sub_dir);
		else
			sprintf(pathname, "%s/%s/%s/%s%s",
					psi_name, JOBS_DIR, sub_dir, fname, STF_EXT);
		break;
	  case GLOBAL_VJT_FILE:
		if (!fname || STREQ(fname, NO_STRING))
			sprintf(pathname, "%s/%s", psi_name, TABLES_DIR);
		else
			sprintf(pathname, "%s/%s/%s%s", 
					psi_name, TABLES_DIR, fname, VJT_EXT);
		break;
	  case LOCAL_VJT_FILE:
		if (!fname || STREQ(fname, NO_STRING))
			sprintf(pathname, "%s/%s/%s", psi_name, JOBS_DIR, sub_dir);
		else
			sprintf(pathname, "%s/%s/%s/%s%s",
					psi_name, JOBS_DIR, sub_dir, fname, VJT_EXT);
		break;
	  case LFO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, LFO_EXT);
		break;
	  case PREV_LFO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, PREV_LFO_EXT);
		break;
	  case OLD_LFO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, OLD_LFO_EXT);
		break;
	  case NEW_LFO_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, NEW_LFO_EXT);
		break;
	  case LMAP_FILE:
		sprintf(pathname, "%s/%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, LMAP_EXT);
		break;
	  case ILL_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, ILL_EXT);
		break;
	  case PEX_FILE_TEMP:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, PEX_TEMP_EXT);
		break;
	  case PEX_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s", 
				psi_name, JOBS_DIR, sub_dir, fname, PEX_EXT);
		break;
	  case LPEX_FILE:
		sprintf(pathname, "%s/%s/%s/%s%s",
				psi_name, JOBS_DIR, sub_dir, fname, LPEX_EXT);
		break;
	  case FORMAT_FILE2:
		sprintf(pathname, "%s/%s/%s/%s",
				psi_name, JOBS_DIR, sub_dir, fname);
		break;
	  case GENTAG_FILE2:
		sprintf(pathname, "%s/%s/%s/%s",
				psi_name, JOBS_DIR, sub_dir, fname);
		break;
 	  case PELEM_FILE:
 		sprintf(pathname, "%s/%s/%s/%s%s",
 				psi_name, JOBS_DIR, sub_dir, fname, PELEM_EXT);
  		break;
	  case LAYOUT_BACKUP:
		sprintf(pathname, "%s/%s/%s/%s%s%s%d",
				psi_name, JOBS_DIR, sub_dir, fname, LAYOUT_EXT,
				BACKUP_EXT, p_BackupVersion);
		break;
	  case TEXT_BACKUP:
		sprintf(pathname, "%s/%s/%s/%s%s%s%d",
				psi_name, JOBS_DIR, sub_dir, fname, TEXT_EXT,
				BACKUP_EXT, p_BackupVersion);
		break;
	  case FO_BACKUP:
		sprintf(pathname, "%s/%s/%s/%s/%s%s%s%d",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, FO_EXT,
				BACKUP_EXT, p_BackupVersion);
		break;
	  case MAP_BACKUP:
		sprintf(pathname, "%s/%s/%s/%s/%s%s%s%d",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, MAP_EXT,
				BACKUP_EXT, p_BackupVersion);
		break;
	  case LFO_BACKUP:
		sprintf(pathname, "%s/%s/%s/%s/%s%s%s%d",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, LFO_EXT,
				BACKUP_EXT, p_BackupVersion);
		break;
	  case LMAP_BACKUP:
		sprintf(pathname, "%s/%s/%s/%s/%s%s%s%d",
				psi_name, JOBS_DIR, sub_dir, DRAWER_DIR, fname, LMAP_EXT,
				BACKUP_EXT, p_BackupVersion);
		break;
	  case OTHER_FILE:
	  default:					/* Unknown pathname header */
		sprintf(pathname,"%s",fname);
		break;
	}
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "p_path(): returning %s\n",pathname);
#endif 
	/* Check if last character is a '/',  if so, then delete it */
	if (pathname[strlen(pathname)-1] == '/')
		pathname[strlen(pathname)-1] = '\0';
#ifdef TRACE
	if (trace_8)
		p_info (PI_TRACE, "p_path %s\n",pathname);
#endif 
	return(&pathname[0]);
}								/* end p_path() */

/********************************************************************/
/* Current size of malloc'ed bytes */

#if !MEMSHELL
static void p_print_mallinfo()
{
#if defined(TRACE) && defined(LIB_MALLOC)
	struct mallinfo info;

	if (trace_8)
	{
		info = mallinfo();
		p_info(PI_TRACE, "MallocInfo:\n\t%s %d,\n\t%s %d,\n\t%s %d,\n\t%s %d,\n\t%s %d,\n\t%s %d,\n\t%s %d,\n\t%s %d,\n\t%s %d\n",
				  "Total Space in Arena", info.arena,
				  "Number of Ordinary Blocks", info.ordblks, 
				  "Number of Small Blocks", info.smblks,
				  "Number of Holding Blocks", info.hblks,
				  "Space in Holding Block Headers", info.hblkhd,
				  "Space in Small Blocks", info.usmblks, 
				  "Space in Free Small Blocks", info.fsmblks,
				  "Space in ordinary blocks inuse", info.uordblks, 
				  "Space in free ordinary blocks", info.fordblks);
	}
#endif 
}
#endif /* MEMSHELL */


/********************************************************************/

/********************************************************************* 
  NOTE for p_put() coded like this since putc is a macro and can not
  be passed as an arg to the function return()
  **********************************************************************/
int p_putc(int c, Pfd fd_id)
{
	int val;
	
	/*LINTED*/
	val = putc(c, PFILES[fd_id].fd);
	PFILES[fd_id].last_op = 'p';
	return(val);
}						/* end p_putc() */

/********************************************************************/

/*	p_read mimmicks the system call "read", except it adds two
	fields to its parameter list, recnum and swap_type, which are used 
	for random access files and byte swapping respectively.
	
	Parameter list description:
 	buffer		- buffer for data
	reclength	- byte length of each record in file
	numrecs		- number of records to read
	fd_id     	- penta file ID
	recnum    	- record number to be read (NEX_REC = next)
	swap_type	- index to list of record byte/int structures
	*/

/*ARGSUSED*/
int p_read(char *buffer, int reclength, int numrecs, Pfd fd_id, int recnum,
		   int16 swap_type)
{
	int temp;
	int result;

#ifdef TRACE
	if(trace_8) 
		p_info(PI_TRACE, "p_read(buffer: %X, reclength: %d, numrecs: %d, fd_id: %d, recnum: %d, fd: %X)\n",
				  buffer,reclength,numrecs,fd_id,recnum,PFILES[fd_id].fd);
#endif 
	
	if(fd_id == P_ERROR)
		return(0);
	switch (PFILES[fd_id].buf_style)
	{
	  case P_READ_FROM_MEMORY:
		{
			int pos,length;
			pos = recnum * reclength;
			length = numrecs * reclength;
			if(pos < 0 || (pos + length) > PFILES[fd_id].record_size)
				return(0);
			memcpy(buffer, (*PFILES[fd_id].chunks) + pos, length);
#if defined i386

#if OS_linux
			if (swap_type && swap_type != SW_FACS)
				byte_swap (buffer, length, swap_type);
#else
			if (swap_type)
				byte_swap (buffer, length, swap_type);
#endif

#endif 
			return(numrecs);
		}
	  case P_WRITE_TO_MEMORY:
	  case P_DISK_MEMORY_BUFFERS:
		{
			char *chunk;
			int piece;
			int max;
			int rec;
			int nrecs;

			rec = recnum;
			nrecs = numrecs;
			if(reclength != PFILES[fd_id].record_size)
			{
				p_info(PI_ELOG, "Unmatched record sizes during write\n");
				return(0);
			}
			max = (rec + nrecs) / PFILES[fd_id].records_per_chunk;
			if(max >= PFILES[fd_id].num_chunks)
			{
				char **new_chunks;

				max += 50;
				new_chunks = (char **)p_alloc(sizeof(char *) * max);
				if(PFILES[fd_id].chunks != 0)
				{
					memcpy( (char *)new_chunks, (char *)PFILES[fd_id].chunks,
							sizeof(char *) * PFILES[fd_id].num_chunks);
					p_free((char *)PFILES[fd_id].chunks);
				}
				PFILES[fd_id].chunks = new_chunks;
				PFILES[fd_id].num_chunks = max;
			}
			while(nrecs > 0)
			{
				chunk = *(PFILES[fd_id].chunks + 
						  (rec / PFILES[fd_id].records_per_chunk));
				piece = rec % PFILES[fd_id].records_per_chunk;
				
				if (chunk == 0)		/* If this chunk unallocated (has no ptr) */
				{					/* (allowed only for P_DISK_MEMORY_BUFFERS) */
					if(PFILES[fd_id].buf_style == P_WRITE_TO_MEMORY)
					{
						p_info(PI_ELOG, "can't read undefined memory file\n");
						return(0);
					}
					max = rec / PFILES[fd_id].records_per_chunk;
					temp = PFILES[fd_id].chunk_size * max;
					if (fseek(PFILES[fd_id].fd, (long)temp, 0))
					{
						p_info(PI_ELOG, "Bad call (#1) to fseek(%s, %d, 0)",
								PFILES[fd_id].filename, temp);
						return(0);
					}
					chunk = (char *)p_alloc(PFILES[fd_id].chunk_size);
					result = fread(chunk, PFILES[fd_id].record_size,
								PFILES[fd_id].records_per_chunk,
								PFILES[fd_id].fd);
					PFILES[fd_id].last_op = 'o'; /* read */
					*(PFILES[fd_id].chunks + max) = chunk;
					if(PFILES[fd_id].max_chunk < max)
						PFILES[fd_id].max_chunk = max;
				}
				memcpy(buffer, chunk + (reclength * piece), reclength);
#if defined i386
#if OS_linux
				if (swap_type && swap_type != SW_FACS)
					byte_swap (buffer, reclength, swap_type);
#else
				if (swap_type)
					byte_swap (buffer, reclength, swap_type);
#endif
#endif 
				buffer += reclength;
				nrecs--;
				rec++;
			}
			if((recnum + numrecs -1) <= PFILES[fd_id].max_rec)
				return(numrecs);
			return(0);
		}
	  case P_NO_BUFFERING:
	  default:
			if(recnum == NEXT_REC)
				temp = PFILES[fd_id].pos;
			else
				temp = reclength * recnum;
			if(PFILES[fd_id].last_op != 'r' || PFILES[fd_id].pos !=temp)
			{
				if(trace_8)
					p_info(PI_TRACE, "seeking fd_id %d to %d\n", fd_id, temp);
				if (fseek(PFILES[fd_id].fd, (long)temp, 0))
				{
					p_info(PI_ELOG, "Bad call (#2) to fseek(%s, %d, 0)",
							PFILES[fd_id].filename, temp);
					return(0);
				}
				PFILES[fd_id].pos = temp;
			}
			if(trace_8)
				p_info(PI_TRACE, "reading  ");
			temp = fread(buffer,reclength,numrecs,PFILES[fd_id].fd);
			PFILES[fd_id].last_op = 'r'; /* read */
			if (temp == numrecs)
				PFILES[fd_id].pos += temp * reclength;
			else
				PFILES[fd_id].pos = NEXT_REC; /* Force seek next time */
#if defined i386
#if OS_linux
			if (swap_type && swap_type != SW_FACS)
				byte_swap (buffer, reclength*numrecs, swap_type);
#else
			if (swap_type)
				byte_swap (buffer, reclength*numrecs, swap_type);

#endif

#endif 
	}							/* end switch buf_style */
#ifdef TRACE
	if (trace_data)
	{
		int i;

		for (i = 0; i < (reclength * numrecs); i += 16)
		{
			p_info(PI_TRACE, "%5.2X:  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X ",
					  i, (unsigned char)buffer[i], (unsigned char)buffer[i+1],
					  (unsigned char)buffer[i + 2], (unsigned char)buffer[i+3],
					  (unsigned char)buffer[i + 4], (unsigned char)buffer[i+5],
					  (unsigned char)buffer[i + 6], (unsigned char)buffer[i+7],
					  (unsigned char)buffer[i + 8], (unsigned char)buffer[i+9],
					  (unsigned char)buffer[i+10], (unsigned char)buffer[i+11],
					  (unsigned char)buffer[i+12], (unsigned char)buffer[i+13],
					  (unsigned char)buffer[i+14],(unsigned char)buffer[i+15]);
			p_info(PI_TRACE, "* %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c *\n",
					  isprint(buffer[i     ]) ? (buffer[i     ]) : '.',
					  isprint(buffer[i +  1]) ? (buffer[i +  1]) : '.',
					  isprint(buffer[i +  2]) ? (buffer[i +  2]) : '.',
					  isprint(buffer[i +  3]) ? (buffer[i +  3]) : '.',
					  isprint(buffer[i +  4]) ? (buffer[i +  4]) : '.',
					  isprint(buffer[i +  5]) ? (buffer[i +  5]) : '.',
					  isprint(buffer[i +  6]) ? (buffer[i +  6]) : '.',
					  isprint(buffer[i +  7]) ? (buffer[i +  7]) : '.',
					  isprint(buffer[i +  8]) ? (buffer[i +  8]) : '.', 
					  isprint(buffer[i +  9]) ? (buffer[i +  9]) : '.',
					  isprint(buffer[i + 10]) ? (buffer[i + 10]) : '.', 
					  isprint(buffer[i + 11]) ? (buffer[i + 11]) : '.',
					  isprint(buffer[i + 12]) ? (buffer[i + 12]) : '.', 
					  isprint(buffer[i + 13]) ? (buffer[i + 13]) : '.',
					  isprint(buffer[i + 14]) ? (buffer[i + 14]) : '.', 
					  isprint(buffer[i + 15]) ? (buffer[i + 15]) : '.');
		}
	}
#endif
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "Number of record read: %d\n",temp);
#endif 
	return(temp);
}								/* end p_read() */

/********************************************************************/

char *p_readdir(Pfd fd_id)
{
	struct dirent *ptr;

	if(PFILES[fd_id].in_use >= 0) 
		return(NULL);
	/* Read entries until we get one which is not '.' or '..' */
	for(;;)
	{
		int len;

		if ((ptr = readdir(PFILES[fd_id].dirp)) == NULL) 
		{
			p_closedir(fd_id);
			return(NULL);
		}
		/* Check for an inode of 0, which is bogus */
		if (ptr -> d_ino == (unsigned long) 0)
			continue;
		if ((len = strlen(ptr->d_name))<= 2)
		{
			if (*(ptr->d_name) == (char)'.')
			{
				if (len == 1)
					continue;
				if (*(ptr->d_name+1) == (char)'.') 
					continue;
			}
		}
		return(ptr->d_name);
	}
}								/* end p_readdir() */

/********************************************************************/

#if !MEMSHELL
void *p_realloc(char *oldptr, int newsize)
{
	char *char_ptr;

	if (oldptr)
	{
#if ALLOC_TRACE
		int oldsize;

		oldptr -= ALLOC_TRACE;
		oldsize = *(int *)oldptr;
		newsize += ALLOC_TRACE;
		alloc_call_size += newsize;
		alloc_call_size -= oldsize;
		char_ptr = (char *)realloc(oldptr, newsize);
		oldptr += ALLOC_TRACE;
		*(int *)char_ptr = newsize;
		char_ptr += ALLOC_TRACE;
#ifdef TRACE
		if (trace_alloc)
			p_info(PI_TRACE, "p_realloc %4d bytes at loc 0x%6x to %4d bytes at loc 0x%6x\n",
					  oldsize - ALLOC_TRACE, oldptr, newsize - ALLOC_TRACE,
					  char_ptr);
#endif TRACE
#ifdef TEST_MEMORY
		if (trace_memory)
			realloc_1_entry(0, oldsize - ALLOC_TRACE, (uint32)oldptr,
							newsize - ALLOC_TRACE, (uint32)char_ptr);
#endif
#else 
		if ((char_ptr = realloc(oldptr, newsize)) == NULL)
		{
			p_info(PI_TRACE, "\n\nERROR: Failed to reallocate %d bytes",
					newsize);
			p_print_mallinfo();
			return (NULL);
		}
#endif 
		return(char_ptr);
	}
	else
		return(p_alloc(newsize));
}								/* end p_realloc() */
#endif /* MEMSHELL */

/********************************************************************/

#if !MEMSHELL
void *p_remalloc( char *op, int oldsize, int newsize)
{
	char *cp;

	cp = p_realloc( op, newsize);
	if( !cp || !op)
		return cp;
	if( oldsize && newsize > oldsize)
		memset( cp + oldsize, 0, newsize - oldsize);
	return cp;
}
#endif

/********************************************************************/

int p_rename(char *tree_name, int olddir, char *oldsub_dir, char *oldname,
			 int newdir, char *newsub_dir,	char *newname)
{
	char old_path[160];	/* 128 -> 160 in case of 64-ch graphic name. */
	char new_path[160];
	int ret_val;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "p_rename\n");
#endif
	strcpy(old_path,p_path(tree_name, olddir, oldsub_dir, oldname));
	strcpy(new_path,p_path(tree_name, newdir, newsub_dir, newname));
	ret_val = rename(old_path,new_path);
#ifdef TRACE
	if(trace_8) 
		p_info(PI_TRACE, "%d = p_rename %s -> %s\n",ret_val,old_path,new_path);
#endif
	return(ret_val);
}								/* end p_rename() */

/********************************************************************
 *  Does rename of one backup version to the next.
 *  "olddir" must be one of: <LAYOUT,LFO,LMAP,TEXT,FO,MAP>_BACKUP.
 *  Using that type, this routine renames the file numbered 
 *    p_BackupVersion to p_backupVersion+1.
 *******************************************************************/

int p_rename_backup (char *tree_name, int olddir, char *oldsub_dir, char *oldname)
{
	char old_path[160];	/* 128 -> 160 in case of 64-ch graphic name. */
	char new_path[160];
	int ret_val;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "p_rename_backup\n");
#endif
	strcpy (old_path,p_path(tree_name, olddir, oldsub_dir, oldname));
	ret_val = p_BackupVersion++;
	strcpy (new_path,p_path(tree_name, olddir, oldsub_dir, oldname));
	p_BackupVersion = ret_val;		/* (restore caller's version#)  */

	ret_val = rename (old_path,new_path);
#ifdef TRACE
	if(trace_8) 
		p_info(PI_TRACE, "%d = p_rename_backup %s -> %s\n",ret_val,old_path,new_path);
#endif
	return(ret_val);
}								/* end p_rename_backup() */

/********************************************************************/

int p_seek(Pfd fd_id, int32 offset, int mode)
{
	int result;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "seeking, fd_id: %d, offset: %ld, mode: %d\n",
				  fd_id, offset, mode);
#endif
	
	result = fseek(PFILES[fd_id].fd, offset, mode);
	if (!result)
	{							/* Success */
		PFILES[fd_id].last_op = 's'; /* seek */
		switch (mode)
		{
		  case SEEK_SET:
			PFILES[fd_id].pos = offset;
			break;
		  case SEEK_CUR:
			PFILES[fd_id].pos += offset;
			break;
		  case SEEK_END:
			if (fgetpos(PFILES[fd_id].fd,
						(fpos_t *)&PFILES[fd_id].pos))
			{
				p_info(PI_ELOG, "*** fgetpos failed\n");
				return(-1);
			}
			break;
		}
	}
	else
		PFILES[fd_id].pos = NEXT_REC; /* Force seek next time */
	return(result);
}								/* end p_seek() */

/********************************************************************/

int p_stat(char *tree_name, int dir, char *sub_dir, char *fname,
		   struct stat *buffer)
{
	char *pathname;
	int stat_result;
	int save_errno;

	save_errno = errno;
#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "p_stat\n");
#endif
	pathname = p_path(tree_name, dir, sub_dir, fname);
	if ((stat_result = stat(pathname, buffer)) != 0)
	{
#ifdef TRACE
		if (trace_8)
			p_info(PI_TRACE, "stat %s failed",pathname);
#endif
		return(stat_result);
	}
#ifdef TRACE
	if(trace_8) 
		p_info(PI_TRACE, "stat %s: size %d, mode %o (octal)\n",
				  pathname, buffer->st_size, buffer->st_mode);
#endif 
	errno = save_errno;
	return(stat_result);
}								/* end p_stat() */

/********************************************************************/

int p_ungetc(int c, Pfd fd)
{
	int val;

	val = ungetc(c, PFILES[fd].fd);
	PFILES[fd].last_op = 'u';
	return(val);
}

/********************************************************************/

int p_unlink(char *tree_name, int dir, char *sub_dir, char *fname)
{
	int return_val;
	char *pathname;

#ifdef TRACE
	if (trace_8)
		p_info(PI_TRACE, "unlink\n");
#endif

	pathname = p_path(tree_name, dir, sub_dir, fname);
	return_val = unlink(pathname);
	if (trace_8 && return_val)
		p_info(PI_TRACE, "Cannot remove (unlink) file %s", pathname);
	return(return_val);
}								/* end p_link() */

/*
 * Print `printf'-like formated string to stderr,
 * and optionally to a log file.
 * level = PI_ERRNO -- emulate p_error
 *         otherwise - emulate p_warning.
 * level & PI_LOG -- also print to p_LogFd
 * See p_lib.h for valid values for level.
 * Currently, only PI_ERRNO and PI_LOG have significance.
 * Eventually, this can be enhanced for other logging options.
 * p_LogFd must already be opened for output to occur.
 * By default, only 50 lines will be printed to p_LogFd.
 * This can be changed by changing p_LogMaxCnt.
 * If p_LogMaxCnt is <= 0, log indefinately.
 */
void p_info (int level, char *msg, ...)
{
FILE *p_WrnFd = stdout;
FILE *p_InfoFd = stderr;

	va_list ap;
	char buf[1024];
	int eno = errno;
	errno = 0;

	/* Get User error message */
	va_start (ap, msg);
	if (!(level & PI_NOFMT))
		vsprintf (buf, msg, ap);
	else
		strcpy (p_InfoMsg, msg);
	va_end (ap);

	if (!(level & PI_NOFMT))
	{
		if (!(level & PI_ERRNO))
			eno = 0;

		/* Print user error message and system error message */
		if( eno)
			sprintf (p_InfoMsg, "%s: %s (%d)", buf, strerror (eno), eno);
		else
			strcpy (p_InfoMsg, buf);

		if ((level & PI_ERRNO))
			strcat (p_InfoMsg, "\n");
	}

	if ((level & PI_TRACE) && p_WrnFd)
		fputs (p_InfoMsg, p_WrnFd);
	else if (p_InfoFd)
		fputs (p_InfoMsg, p_InfoFd);

	if ((level & PI_LOG))
		if (p_LogFd)
			if (p_LogMaxCnt <= 0 || p_LogCnt++ < p_LogMaxCnt)
				fputs (p_InfoMsg, p_LogFd);
}

/********************************************************************/
/* p_warning prints the message specified by the user to stderr
 */

void p_warning(char *fmt, ...)
{
    va_list args;
    FILE *p_WrnFd = stdout;

	/* Get User error message */
    va_start(args, fmt);
    if( p_WrnFd)
        vfprintf(p_WrnFd, fmt, args);
    else
        vsprintf(p_WrnMsg, fmt, args);
    va_end(args);
}								/* end p_warning() */

/********************************************************************/

/*	p_write mimmicks the system call "write", except it adds two 
	fields to its parameter list, recnum and swap_type, which are used 
	for random access files	and byte swapping respectively.
	
 	Parameter list description:
	buffer		- buffer for data
	reclength	- byte length of each record in file
	numrecs		- number of records to read
	fd_id     	- penta file ID
	recnum    	- record number to be read (NEX_REC = next)
	swap_type	- index to list of record byte/int structures
	*/

/*ARGSUSED*/
int p_write(char *buffer, int reclength, int numrecs, Pfd fd_id, 
			int recnum, int16 swap_type)
{
	int temp;
	int result;


#ifdef TRACE
	if(trace_8)
	{
		p_info(PI_TRACE, "p_write,buffer: %X, reclength: %d, numrecs: %d, fd_id: %d, recnum: %d, fd: %X)\n",
				  buffer,reclength,numrecs,fd_id,recnum,PFILES[fd_id].fd);
	}
#endif
	if(fd_id == P_ERROR)
		return(0);
	switch (PFILES[fd_id].buf_style)
	{
	  case P_READ_FROM_MEMORY:
		{
			p_info(PI_ELOG, "ERROR can't write %s, opened for memory reading\n",
					  PFILES[fd_id].filename);
			return(P_ERROR);
		}
	  case P_DISK_MEMORY_BUFFERS:		/* (Writes to mem THEN to disk) */
	  case P_WRITE_TO_MEMORY:
		{
			char *chunk;
			int piece;
			int max;
			int rec;
			int nrecs;

			rec = recnum;
			nrecs = numrecs;
			if(reclength != PFILES[fd_id].record_size)
			{
				p_info(PI_ELOG, "Unmatched record sizes during write\n");
				return(0);
			}
			max = (rec + nrecs) / PFILES[fd_id].records_per_chunk;
			if(max >= PFILES[fd_id].num_chunks)
			{
				char **new_chunks;

				max += 50;
				new_chunks = (char **)p_alloc(sizeof(char *) * max);
				if(PFILES[fd_id].chunks != 0)
				{
					memcpy( (char *)new_chunks, (char *)PFILES[fd_id].chunks,
							sizeof(char *) * PFILES[fd_id].num_chunks);
					p_free((char *)PFILES[fd_id].chunks);
				}
				PFILES[fd_id].chunks = new_chunks;
				PFILES[fd_id].num_chunks = max;
			}
			while(nrecs > 0)
			{
				char *bptr;

				chunk = *(PFILES[fd_id].chunks +  
						  (rec / PFILES[fd_id].records_per_chunk));
				piece = rec % PFILES[fd_id].records_per_chunk;
				if(chunk == 0)
				{
					max = rec / PFILES[fd_id].records_per_chunk;
					chunk = (char *)p_alloc(PFILES[fd_id].chunk_size);
					if(PFILES[fd_id].buf_style == P_DISK_MEMORY_BUFFERS)
					{
						temp = PFILES[fd_id].chunk_size * max;
						if (fseek(PFILES[fd_id].fd, (long)temp, 0))
						{
							p_info(PI_ELOG, "Bad call (#4) to fseek(%s, %d, 0)",
									PFILES[fd_id].filename, temp);
							return(0);
						}
						result = fread(chunk, PFILES[fd_id].record_size,
									PFILES[fd_id].records_per_chunk,
									PFILES[fd_id].fd);
						PFILES[fd_id].last_op = 'o'; /* read */
					}
					*(PFILES[fd_id].chunks + max) = chunk;
					if(PFILES[fd_id].max_chunk < max)
						PFILES[fd_id].max_chunk = max;
				}
				bptr = chunk + (reclength * piece);
				memcpy(bptr, buffer, reclength);	/* Write to mem buf */
#if defined i386
#if OS_linux
				if (swap_type && swap_type != SW_FACS)
					byte_swap (bptr, reclength, swap_type);
#else
				if (swap_type)
					byte_swap (bptr, reclength, swap_type);
#endif
#endif 
				if (PFILES[fd_id].buf_style == P_DISK_MEMORY_BUFFERS)
				{
					temp = reclength * rec;
					if (fseek(PFILES[fd_id].fd, (long)temp, 0))
					{
						p_info(PI_ELOG, "Bad call (#3) to fseek(%s, %d, 0)",
								PFILES[fd_id].filename, temp);
						return(0);
					}
					temp = fwrite(bptr,reclength,1,PFILES[fd_id].fd);
				}
				if(PFILES[fd_id].max_rec < rec)
					PFILES[fd_id].max_rec = rec;
				buffer += reclength;
				rec++;
				nrecs--;
			}
			return(numrecs);
		}
	  case P_NO_BUFFERING:
	  default:
		{
			if(recnum == NEXT_REC)
				temp = PFILES[fd_id].pos;
			else
				temp = reclength * recnum;
			if(PFILES[fd_id].last_op != 'w' || PFILES[fd_id].pos !=temp)
			{
				if(trace_8)
					p_info(PI_TRACE, "seeking fd_id %d to %d\n", fd_id, temp);
				if (fseek(PFILES[fd_id].fd, (long)temp, 0))
				{
					p_info(PI_ELOG, "Bad call (#5) to fseek(%s, %d, 0)",
							PFILES[fd_id].filename, temp);
					return(0);
				}
				PFILES[fd_id].pos = temp;
			}
#if defined i386
#if OS_linux
			if (swap_type && swap_type != SW_FACS)
				buffer = byte_swap_copy (buffer, reclength * numrecs, swap_type);
#else
			if (swap_type)
				buffer = byte_swap_copy (buffer, reclength * numrecs, swap_type);

#endif

#endif 
			temp = fwrite(buffer,reclength,numrecs,PFILES[fd_id].fd);
			PFILES[fd_id].last_op = 'w'; /* write */
			if (temp == numrecs)
				PFILES[fd_id].pos += temp * reclength;
			else
				PFILES[fd_id].pos = NEXT_REC; /* Force seek next time */
		}						/*  */
	}								/* end switch buf_style */
#ifdef TRACE
	if (trace_data)
	{
		int i;

		for (i = 0; i < (reclength * numrecs); i += 16)
		{
			p_info(PI_TRACE, "%5.2X:  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X  %2.2X%2.2X ",
					  i, (unsigned char)buffer[i], (unsigned char)buffer[i +1],
					  (unsigned char)buffer[i + 2], (unsigned char)buffer[i +3],
					  (unsigned char)buffer[i + 4], (unsigned char)buffer[i +5],
					  (unsigned char)buffer[i + 6], (unsigned char)buffer[i +7],
					  (unsigned char)buffer[i + 8], (unsigned char)buffer[i +9],
					  (unsigned char)buffer[i+10], (unsigned char)buffer[i+11],
					  (unsigned char)buffer[i+12], (unsigned char)buffer[i+13],
					  (unsigned char)buffer[i+14],(unsigned char)buffer[i+15]);
			p_info(PI_TRACE, "* %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c *\n",
					  isprint(buffer[i     ]) ? (buffer[i     ]) : '.',
					  isprint(buffer[i +  1]) ? (buffer[i +  1]) : '.',
					  isprint(buffer[i +  2]) ? (buffer[i +  2]) : '.',
					  isprint(buffer[i +  3]) ? (buffer[i +  3]) : '.',
					  isprint(buffer[i +  4]) ? (buffer[i +  4]) : '.',
					  isprint(buffer[i +  5]) ? (buffer[i +  5]) : '.',
					  isprint(buffer[i +  6]) ? (buffer[i +  6]) : '.',
					  isprint(buffer[i +  7]) ? (buffer[i +  7]) : '.',
					  isprint(buffer[i +  8]) ? (buffer[i +  8]) : '.', 
					  isprint(buffer[i +  9]) ? (buffer[i +  9]) : '.',
					  isprint(buffer[i + 10]) ? (buffer[i + 10]) : '.', 
					  isprint(buffer[i + 11]) ? (buffer[i + 11]) : '.',
					  isprint(buffer[i + 12]) ? (buffer[i + 12]) : '.', 
					  isprint(buffer[i + 13]) ? (buffer[i + 13]) : '.',
					  isprint(buffer[i + 14]) ? (buffer[i + 14]) : '.', 
					  isprint(buffer[i + 15]) ? (buffer[i + 15]) : '.');
		}
	}
#endif
#ifdef TRACE
	if(trace_8)
		p_info(PI_TRACE, "Number of record written: %d\n",temp);
#endif 
	return(temp);
}								/* end p_write() */


/*
 * Parse [buf] into fields.
 * Pointers to start of each field are returned in [tok].
 * [cnt] is the max nr of fields to parse. It cannot be greater
 *     than the size of [tok]. If [cnt] is less the the number
 *     of fields encountered, tok[cnt] is set to NULL.
 * [squeeze] == 1 if sequences of consecutive delimiter chars
 *     should be treated as a single delimiter.
 *           == 0 if sequences of consecutive delimiter chars
 *     should be treated as marking empty fields.
 * [delim] contains the chars to be used as field delimiters.
 *     If null, it is set to " \t\n\f\r".
 * Return the number of fields found.
 * NOTE: A similar function, 'strfield' is used by several programs,
 *     and each program has its own separate copy. Eventually,
 *     they should all be changed to use this function.
 */
int p_strfld (char *buf, char **tok, int cnt, int squeeze, char *delim)
{
	char *cps;
	char **tps;

	tps = tok;
	if (!tps)
		return 0;
	*tps = NULL;

	if (cnt <= 0)
		return 0;

	cps = buf;
	if (!cps || !*cps)
		return 0;

	if (!delim || !*delim)
		delim = " \t\n\f\r";

	/* get rid of initial delimiter
	 * or initial consecutive delimiters
	 */
	if (squeeze)
		while (*cps && strchr (delim, *cps))
			cps++;
	else if (strchr (delim, *cps))
		cps++;

	for (;;)
	{
		*tps++ = cps;
		--cnt;
			
		while (*cps && !strchr (delim, *cps))
			cps++;
		if (!*cps)
			break;
		*cps++ = 0;	/* found a delimiter, null it */
		if (squeeze)	/* Bypass all consecutive delimiters.  */
			while (*cps && strchr (delim, *cps))
				cps++;
		if (!*cps)		/* Above was (were) trailing delimiters. */
			break;
		if (cnt <= 0)
			break;
	}

	if (cnt >= 0)
		*tps = NULL;	/* Null the token ptr past count.  */
	return tps - tok;
}
