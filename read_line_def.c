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
#include <string.h>
#include "line_def.h"

int read_line_def(LDEF *fo_line_def, int16 (*foget) () )
{
	int16 status_length;
	int16 flow = 0;

	status_length = foget();
	if ( status_length < 27 )
		return( -1);			/* 27 is the smallest fo */
	if ( status_length > (FO_STATUS_LENGTH + 10) )
		return( -2);			/* probably out of synch */
	fo_line_def -> FoStatusLength = status_length;
	if ( status_length < FO_STATUS_LENGTH)
	{							/*set default values for missing status words*/
		if ( (status_length + 2) <= FO_STATUS_LENGTH)
		{
			fo_line_def -> BolDroppableExtraLeadTop = 0;
			fo_line_def -> BolFlexibleExtraLeadTop = 0;
		}
	}
	while ( ++flow < status_length)
	{
		switch (flow)
		{
		  case 1:
			fo_line_def -> Quad = foget();
			break;
		  case 2:
			fo_line_def -> LeftIndent = foget();
			break;
		  case 3:
			fo_line_def -> RightIndent = foget();
			break;
		  case 4:
			fo_line_def -> Bands = foget();
			break;
		  case 5:
			fo_line_def -> BandSpace = foget();
			break;
		  case 6:
			fo_line_def -> UnusedMeasure = foget();
			break;
		  case 7:
			fo_line_def -> LsPerChar = foget();
			break;
		  case 8:
			fo_line_def -> LsTotal = foget();
			break;
		  case 9:
			fo_line_def -> LineNum = foget();
			break;
		  case 10:
			fo_line_def -> BolLeading = foget();
			break;
		  case 11:
			fo_line_def -> BolFlexibleExtraLead = foget();
			break;
		  case 12:
			fo_line_def -> BolExpandableExtraLead = foget();
			break;
		  case 13:
			fo_line_def -> BolDroppableExtraLead = foget();
			break;
		  case 14:
			fo_line_def -> BolRigidExtraLead = foget();
			break;
		  case 15:
			fo_line_def -> MiscLineFlags = foget();
			break;
		  case 16:
			fo_line_def -> SolMeasure = foget();
			break;
		  case 17:
			fo_line_def -> SolFont = foget();
			break;
		  case 18:
			fo_line_def -> SolSlant = foget();
			break;
		  case 19:
			fo_line_def -> SolPointSize = foget();
			break;
		  case 20:
			fo_line_def -> SolSetSize = foget();
			break;
		  case 21:
			fo_line_def -> SolLeading = foget();
			break;
		  case 22:
			fo_line_def -> SolCwAdj = foget();
			break;
		  case 23:
			fo_line_def -> SolBaseLine = foget();
			break;
		  case 24:
			fo_line_def -> SolMarginAdjust = foget();
			break;
		  case 25:
			fo_line_def -> SolFgColor = foget();
			break;
		  case 26:
			fo_line_def -> SolFgShade = foget();
			break;
		  case 27:
			fo_line_def -> BolDroppableExtraLeadTop = foget();
			break;
		  case 28:
			fo_line_def -> BolFlexibleExtraLeadTop = foget();
			break;
		  default:
			foget();			/* skip extras */
			break;

		}						/* end switch(flow) */
	}							/* end 	while(++flow<status_length) */
	return(0);
}
