
#define __MSHELL__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include "p_lib.h"

#if MEMSHELL

typedef struct
{
	unsigned int op;
		#define M_alloc		(1<<0)
		#define M_free		(1<<1)
		#define M_realloc	(1<<2)
		#define M_remalloc	(1<<3)
		#define M_freed    	(1<<4)
	size_t size;
	char *addr;
	char *file;
	int line;
}	MEMHDR;

static char *ADD  = "ALLOC";
static char *ADD2 = "REALL";
static char *DEL  = "FREE ";
static char *DEL2 = "RFREE";

static size_t JJ = 0;
static size_t MemUsed = 0, MemMaxUsed = 0;
static MEMHDR *MemArray = NULL, *MemQ = NULL, *MemQend = NULL;
#define M_growsize 1000;
static int MemArraySize = 0;
static unsigned int MemOptions = MEM_On;

static void mem_list( char *type, MEMHDR *mp)
{
	fprintf( p_InfoFd, "%06u %6u %6u %07u %s %5d:%s\n",
		JJ++, MemUsed, mp->size, (unsigned)mp->addr,
		type, mp->line, mp->file);
}


static void mem_err( char *fil, int lin, char *msg, ...)
{
	char buf[512];
	va_list ap;
	
	va_start( ap, msg);
	vsprintf( buf, msg, ap);
	va_end( ap);

	p_info( PI_ELOG, "MEM_ERROR: %s/%d: %s\n", fil, lin, buf);
}


static void mem_bug( char *fil, int lin, char *msg, ...)
{
	char buf[512];
	va_list ap;
	
	va_start( ap, msg);
	vsprintf( buf, msg, ap);
	va_end( ap);

	p_info( PI_ELOG, "MEM_BUG: %s/%d: %s\n", fil, lin, buf);
}

	
static int mem_growq( void)
{
	int mq;

	mq = (MemArraySize == 0)? -1 : MemQ - MemArray;
	MemArraySize += M_growsize;
	
	MemArray = (MEMHDR*) realloc( MemArray, sizeof( MEMHDR) * MemArraySize);
	if( !MemArray)
	{
		mem_err( "mem_growq", 0,
				 "realloc(%u): failed", MemArraySize * sizeof( MEMHDR));
		return 0;
	}

	MemQ = MemArray + mq;
	MemQend = MemArray + MemArraySize-1;
	return 1;
}


void *_p_alloc( size_t size, char *fil, int lin)
{
	char *cp;

	if( size == 0)
	{
		mem_bug( fil, lin, "alloc(0): illegal");
		return NULL;
	}

	cp = malloc( size);
	if( !cp)
	{
		mem_err( fil, lin, "alloc(%u): failed", size);
		return NULL;
	}

	memset( cp, 0, size);

	if( MemOptions)
	{
		if( MemQ >= MemQend)
			if( !mem_growq())
				return NULL;
		++MemQ;

		MemQ->size = size;
		MemQ->file = fil;
		MemQ->line = lin;
		MemQ->addr = cp;
		MemQ->op = M_alloc;
		MemUsed += size;

		if( MemMaxUsed < MemUsed)
			MemMaxUsed = MemUsed;

		if( (MemOptions & MEM_List))
			mem_list( ADD, MemQ);

		if( (MemOptions & MEM_AllocDups))
		{
			MEMHDR *mp;
			for( mp = MemQ; --mp >= MemArray; )
				if( mp->addr == cp && !(mp->op & M_freed))
					mem_bug( fil, lin,
							"alloc(%u,%u): %07u already allocated at %s/%d",
							size, mp->size, cp, mp->file, mp->line);
		}
	}

	return cp;
}
	

void *_p_realloc(void *op, size_t size, char *fil, int lin)
{
	char *cp;

	if( !op)
		return _p_alloc( size, fil, lin);

	if( size == 0)
	{
		mem_err( fil, lin, "realloc(op, 0): equivalent to free(op)!!!");
		return NULL;
	}

	cp = realloc( op, size);
	if( !cp)
	{
		mem_err( fil, lin, "realloc(%07u, %u): failed", op, size);
		return NULL;
	}

	if( MemOptions)
	{
		size_t osize = 0;
		char *dp;
		MEMHDR *mp;

		if( MemQ >= MemQend)
			if( !mem_growq())
				return NULL;
		++MemQ;

		MemQ->size = size;
		MemQ->file = fil;
		MemQ->line = lin;
		MemQ->addr = cp;
		MemQ->op = M_realloc;

		dp = op != cp? op : cp;

		for( mp = MemQ; --mp >= MemArray; )
			if( mp->addr == dp && !(mp->op & M_freed))
			{
				mp->op |= M_freed;
				osize = mp->size;
				break;
			}

		MemUsed -= osize;
		if( (MemOptions & MEM_List) && mp >= MemArray)
			mem_list( DEL2, mp);

		MemUsed += size;
		if( MemMaxUsed < MemUsed)
			MemMaxUsed = MemUsed;

		if( (MemOptions & MEM_List))
			mem_list( ADD2, MemQ);

		if( mp < MemArray)
			mem_bug( fil, lin,
					"realloc(%07u, %u): old address not found", op, size);

		if( (MemOptions & MEM_AllocDups))
			for( mp = MemQ; --mp >= MemArray; )
				if( mp->addr == cp && !(mp->op & M_freed))
					mem_bug( fil, lin,
						"realloc(%07u, %u,%u): %07u already allocated at %s/%d",
						op, size, mp->size, cp, mp->file, mp->line);
	}

	return cp;
}


void *_p_remalloc( void *op, size_t oldsize, size_t newsize,
	char *fil, int lin)
{
	char *cp;

	cp = _p_realloc( op, newsize, fil, lin);
	if( !cp || !op)
		return cp;

	if( oldsize && newsize > oldsize)
		memset( cp + oldsize, 0, newsize - oldsize);

	return cp;
}


char *_p_strdup(char *str, char *fil, int lin)
{
	char *s;  

	s = _p_alloc( strlen( str) + 1, fil, lin);
	if( s)
		strcpy( s, str);

	return s;
}


/* void _p_free( void **cp, char *fil, int lin) */
void _p_free( void *cp, char *fil, int lin)
{
	/* if( !cp || !*cp) */
	if( !cp)
	{
		mem_bug( fil, lin, "free(NULL): illegal");
		return;
	}

	if( !MemOptions)
	{
		free( cp);
		/* *cp = NULL; */
		return;
	}
	else
	{
		MEMHDR *mp;

		if( MemQ >= MemQend)
			if( !mem_growq())
				return;
		++MemQ;

		MemQ->size = 0;
		MemQ->file = fil;
		MemQ->line = lin;
		MemQ->addr = cp;
		MemQ->op = M_free | M_freed;

		for( mp = MemQ; --mp >= MemArray; )
			if( mp->addr == cp)
			{
				if( !(mp->op & M_freed))
				{
					mp->op |= M_freed;
					MemQ->size = mp->size;
				}
				break;
			}

		MemUsed -= MemQ->size;
		if( (MemOptions & MEM_List))
			mem_list( DEL, MemQ);

		if( MemQ->size)
		{
			free( cp);
			/* *cp = NULL; */
			return;
		}

		if( mp < MemArray)	
			mem_bug( fil, lin, "free(%07u): address not found", cp);
		else
		{
			MEMHDR *mp1 = mp;
			int oline;
			char *ofile;

			while( --mp >= MemArray)
				if (mp->addr == cp)
					if( (mp->op & (M_alloc|M_realloc|M_remalloc)))
						break;

			if( mp == mp1 || mp < MemArray)
				oline = 0,
				ofile = "unknown";
			else
				oline = mp->line,
				ofile = mp->file;

			mem_bug( fil, lin,
					"free(%07u): already freed at %s/%d: allocated at %s/%d",
					cp, mp1->file, mp1->line, ofile, oline);
		}
	}
}
	

extern void p_memSetOptions( unsigned int options)
{
	/* Can't turn options on in midstream */
	if( !MemOptions)
		return;

	MemOptions = options;
	if( MemOptions)
		MemOptions |= MEM_On;
}


size_t p_memUsed( void)
{
	return MemUsed;
}


size_t p_memMaxUsed( void)
{
	return MemMaxUsed;
}


void p_memListUnfreed( void)
{
	MEMHDR *mp;
	size_t total = 0;

	if( !MemArray || !MemOptions)
		return;

	fprintf( p_InfoFd, "  IDX  TOTAL   SIZE    ADDR  LINE:FILE\n");
	fprintf( p_InfoFd, "----------------------------------------------------\n");

	for( mp = MemArray - 1; ++mp <= MemQ; )
		if( !(mp->op & M_freed))
			fprintf( p_InfoFd, "%5p %6u %6u %7p %5d:%s\n",
					(MEMHDR *)(mp - MemArray), total += mp->size,
					mp->size, mp->addr, mp->line, mp->file);
}

#endif

