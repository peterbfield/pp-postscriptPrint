#ifndef _BYTE_SWAP_H
#define _BYTE_SWAP_H 1

/* byte_swap.h
	All i/o to non-ascii disk files should be done using the p_lib.c routines
	p_read and p_write, which call byte-swapping filters that are invoked if
	the program has been compiled to run on an x86 machine, since short and
	long int's on that machine are stored in memory in byte-swapped format.

	All calls to p_read/p_write should pass one of the following constants as
	the sixth argument:
*/

						/*  Use this arg to p_read/p_write if no byte-swapping
							at all should be done on the PC:	*/
#define SW_NOSWAP				0
						/*  The first active arg is for data recs which are
							all 16-bit words.  Prior to DTP in the PC, almost
							all calls to p_read/p_write throughout DTP soft-
							ware had argument BS16, formerly defined as a 
							null pointer.  Now, the byte-swap routine has no 
							guiding struct for this; it just does 2-byte 
							swaps through the length of the record:		*/
#define BS16					1
#define DEFAULT_BS				1
#define SW_INT16S				1
						/* The second active arg does 4-byte swaps thru the rec: */
#define BS32					2
#define SW_INT32S				2
						/*  All others index into byte_swap.c struct sw_master,
							each entry of which specifies:  a byte-swapping 
							function to call, and a guiding struct showing 
							the record's pattern of longs/shorts/bytes:	*/
#define	SW_HEADER				10
#define	SW_TEXTBLOCK_IN			11
#define	SW_TEXTBLOCK_OUT		12
#define	SW_TEXTHBLOCK			13
#define SW_FORMAT				14
#define SW_GENTAG				15
#define SW_PM_GLOBAL			16
#define SW_PM_TEXT				17
#define SW_PM_INSERT			18
#define SW_PM_STRING			19
#define SW_PM_STR2				20
#define SW_PSD_SETUP			21
#define SW_PSD_PRINT			22
#define SW_FACS					23
#define SW_DICT_INDEX			24
#define SW_KERNDATA				25
#define SW_WIDTH				26
#define SW_SHAPE_SPEC_HEADER	27
#define SW_PX_HEADER			28
#define SW_STYLETAG				29
#define SW_PM_NEWLAYOUT			30
#define SW_PM_STYLEREC			31
						/*  When adding i/o calls for new record types, add:
							1. a constant here to end of list;
							2. in byte_swap.c, an entry to struct sw_master;
							3. in byte_swap.c, a new pattern-structure of type 
								sw_array, pointed to by the sw_master entry.
						*/

#if defined i386 || defined _BYTE_SWAP_C
extern void byte_swap (void* buf, int buflen, int swtype);
extern void* byte_swap_copy (void* buf, int buflen, int swtype);
#else
#define byte_swap(buf, buflen, swtype)
#define byte_swap_copy(buf, buflen, swtype)	buf
#endif

#endif


