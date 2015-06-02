/*    *****     **    **       *       ******     *******    *******
 *   **   **    **    **      ***      **   **    **         **    **
 *   **         **    **     ** **     **   **    **         **     **
 *   **         **    **    **   **    **   **    **         **     **
 *    *****     ********    *******    ******     *****      **     **
 *        **    **    **    **   **    ****       **         **     **
 *        **    **    **    **   **    ** **      **         **     **
 *   **   **    **    **    **   **    **  **     **         **    ** 
 *    *****     **    **    **   **    **   **    *******    *******  */

#include "penta.h"

#define FO_STATUS_LENGTH sizeof(LDEF)/2

typedef struct
{
	int16 FoStatusLength;
	int16 Quad;
	int16 LeftIndent;
	int16 RightIndent;
	int16 Bands;
	int16 BandSpace;
	int16 UnusedMeasure;
	int16 LsPerChar;
	int16 LsTotal;
	uint16 LineNum;
	int16 BolLeading;
	int16 BolFlexibleExtraLead;		/* el-,0 */
	int16 BolExpandableExtraLead;	/* el-,1 */
	int16 BolDroppableExtraLead;	/* el-,2 */
	int16 BolRigidExtraLead;		/* el-,3 */
	int16 MiscLineFlags;		/* & 0x000f = number of [vb's in line
								   & 0x0010: 0= allow paint; 1= supress paint
								   				from [bp (0) and [kp (1)
								   & 0x0020: 0= no [fm, 1= [fm in the line
								   & 0x0040: 1= level 1 add started (editrace)
								   & 0x0080: 2= level 2 add started (editrace)
								   & 0x0100: 0= [bc, output of line allowed
								   			 1= [ec, inhibit output of line
								   & 0x0200: 1= There is a [pp this line
								   & 0x0400: 1= Apply UnusedMeasure as additional
								   				 LsTotal to LS, not to bands as usual  */
	int16 SolMeasure;
	int16 SolFont;
	int16 SolSlant;			  /*	0100	flash
									0002	bold
									0001	slant	*/
	int16 SolPointSize;
	int16 SolSetSize;
	int16 SolLeading;
	int16 SolCwAdj;
	int16 SolBaseLine;
	int16 SolMarginAdjust;
	int16 SolFgColor;
	int16 SolFgShade;
	int16 BolDroppableExtraLeadTop;	/* el-,0 , 1] */
	int16 BolFlexibleExtraLeadTop;	/* el-,2 , 1] */
	} LDEF;
