/* See end of this file for documentation...
 * Change 0 to 1 in following line to turn memory debuging on
 */
#define MEMSHELL 0


#ifndef _MEMSHELL_H
#define _MEMSHELL_H

#if MEMSHELL
extern void *_p_alloc( size_t size, char *fil, int lin);
extern void *_p_realloc(void *op, size_t size, char *fil, int lin);
extern void *_p_remalloc( void *op, size_t oldsize, size_t newsize,
		char *fil, int lin);
/* void  _p_free(void **,char *,int); */
extern void _p_free( void *cp, char *fil, int lin);
extern char *_p_strdup(char *str, char *fil, int lin);

#define MEM_None		0
#define MEM_Off			0
#define MEM_On			(1<<0)
#define MEM_Default		(1<<0)
#define MEM_Info		(1<<0)
#define MEM_List		(1<<1)
#define MEM_Abort		(1<<2)
#define MEM_AllocDups	(1<<3)
extern void p_memSetOptions( unsigned int options);
extern size_t p_memUsed( void);
extern size_t p_memMaxUsed( void);
extern void p_memListUnfreed( void);
extern size_t p_memGrowSize( void);
extern int p_memAllocCount;

#if !defined(__MSHELL__)
#define malloc(a)			_p_alloc((a),__FILE__,__LINE__)
#define realloc(a,b)		_p_realloc((a),(b),__FILE__,__LINE__)
#define free(a)				_p_free((a),__FILE__,__LINE__)
#define p_alloc(a)			_p_alloc((a),__FILE__,__LINE__)
#define p_realloc(a,b)		_p_realloc((a),(b),__FILE__,__LINE__)
#define p_remalloc(a,b,c)	_p_remalloc((a),(b),(c),__FILE__,__LINE__)
#define p_free(a)			_p_free((a),__FILE__,__LINE__)
/* #define p_free(a)		_p_free((void **)(&(a)),__FILE__,__LINE__) */
/* #define p_free(a) 		if( (a)) _p_free(a),__FILE__,__LINE__), (a) = NULL */
#define strdup(a)			_p_strdup((a),__FILE__,__LINE__)
#endif

#endif /* MEMSHELL */

#endif /* _MEMSHELL_H */

/*
To compile a module with this memory debugging package,
simply set the MEMSHELL define at the top of this file to 1.
This will cause EVERY file that includes p_lib.h to be recompiled.

To back out the memory debugging package, set the MEMSHELL define back to 0.
Again, every file that includes p_lib.h will get recompiled.


Once compiled with MEMSHELL defined, this package will print warning messages
to the p_warning() FILE handle (stdout by default) for the following types of
memory errors:

	* p_alloc is called with a size of 0
	* p_realloc is called with a size of 0
	* p_realloc is called with an old address that isn't allocated
	* p_free is called with a NULL address 
	* p_free is called with an address that isn't allocated
	* p_free is called with an address that is already freed

This is the extent of MEMSHELL's default action; however, it has additional
options that can be set using the p_memSetOptions() routine.
This routine takes one argument, which is an OR'd combination of
MEM_List, MEM_Abort and MEM_AllocDups.

MEM_List
	This will give a line-by-line listing of EVERY instance where memory
	is allocated or freed with p_alloc(), p_realloc(), p_remalloc(),
	p_free(), strdup(), malloc(), realloc(), or free().

MEM_Abort
	This will cause the module to abort immediately when a memory error
	is encountered. Core will be dumped for use with dbx.

MEM_AllocDups
	This will notify of duplicate allocations of the same address.
	This is only useful if you suspect malloc/free/realloc themselves
	are at fault. This option slows down execution considerably.

MEM_Off
	Additionally, p_memSetOptions( MEM_Off) can be invoked to turn memory
	debuging off. The MEMSHELL package is still in place, but no memory
	checking will be done. Essentially, MEM_Off makes this package work
	identically to the equivalent routines in p_lib.c.

	CAUTION: Once MEM_off is set, you cannot turn MEMSHELL back on. 
	p_memSetOptions( MEM_off) should be called at the beginning of a program.
	This is the only way to turn MEMSHELL off without recompiling the world.


There are more memory debugging routines:

p_memUsed()			returns the total amount of memory currently allocated
					by a program.

p_memMaxUsed() 		returns the most amount of memory ever allocated by a
					program so far.

p_memListUnfreed()	lists all memory allocated but not freed. This routine
					should be called before a program exits.

p_memGrowSize()		returns the number of internal info structures the
					MEMSHELL package has allocated. Each call to
					to p_alloc(), p_realloc(), p_remalloc(), p_free(),
					strdup(), malloc(), realloc(), or free() generates
					one structure. Each structure is 20 bytes.
*/
