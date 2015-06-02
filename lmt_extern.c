/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    ** 
     *****     **    **    **   **    **   **    *******    *******   */

#include "penta.h"
#include "frame.h"

/*	NOTE: the order of entries in the following lists MUST not
	change */

/*	corresponds to x and y syntax table order below.
	x and y have matching bit assignments for top/left,
	bottom/right and center */

int16 lmt_syntax_mask_table[] =
	{021, 0102, 022, 023, 020, 021, 042, 043, 040, 041,
					062, 063, 060, 061, 0100, 0101, 0122, 0142};

char lmt_x_syntax_options[] =
/*	"..   SA    LF   LG   LT   LP   RF   RG   RT   RP
					CF   CG   CT   CP   LM    RM ";*/
	".. SA LF LG LT LP RF RG RT RP CF CG CT CP LM RM ";

char lmt_y_syntax_options[] =
/*	"..   SA    TF   TG   TT   TP   BF   BG   BT   BP
					CF   CG   CT   CP   TM    BM    FL    CH";*/
	".. SA TF TG TT TP BF BG BT BP CF CG CT CP TM BM FL CH ";

char lmt_position_mask_table[] =
	{ TL,  TL,  TR,  TC,  BL,  BR,  BC,  CL,  CR,  CC,  HL,  HR,  HC};

/*	{ 17,  17,  18,  19,  33,  34,  35,  49,  50,  51,  65,  66,  67};*/
/*	{x11, x11, x12, x13, x21, x22, x23, x31, x32, x33, x41, x42, x43};*/

char lmt_position_options[] =
/*	"..    TL   TR   TC   BL   BR   BC   CL   CR   CC   HL   HR   HC";*/
	".. TL TR TC BL BR BC CL CR CC HL HR HC";

char lmt_qualifier_options[] =
	"xPIMLHQTL";

/* end of ordered lists */

/* NOTE:
	For the conversion to be accurate, these numbers must be a multiple
of both yx_convert[Y_REF] and yx_convert[X_REF].

	For example, in postscript:
		wn -> ldb = 10
		wn -> msb = 20
	and
		wn -> yx_convert[Y_REF] = lmt_unit_factors[PICA] / wn -> ldb;
		wn -> yx_convert[X_REF] = lmt_unit_factors[PICA] / wn -> msb;
	which is
		wn -> yx_convert[Y_REF] = 540
		wn -> yx_convert[X_REF] = 270

	These two numbers MUST be factor of those one listed blow for you
to get something accurate.

*/

												/* Define for frame.h */
													/* -1 = POINT */
uint16 lmt_unit_factors[] = {	0,			/* No define for this one */
								5400,				/* PICA */
								/*12150*/12201,		/* INCH_32 */
								1530,/*1537*/		/* CENTIMETER */
								1,					/* LINE_LEAD */
								2700,				/* POINT_HALF */
								1350,				/* POINT_QUARTER */
								540,				/* POINT_TENTH */
								1					/* ORG_LINE_LEAD */
							};
