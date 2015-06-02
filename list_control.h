							/* insre inclusion only once	*/
#ifndef _LIST_CONTROL_H
#define _LIST_CONTROL_H

#include <sys/param.h>
#ifndef PMAXHOSTNAMELEN
	#define PMAXHOSTNAMELEN 64
#endif
#include <limits.h>

#define LOGNAME_MAX 8

/* Change the next two Rev# lines at each major release of the software.
   This number us used for the lpm/key REVxx checking. Stored encrypted.
   (Note: These are defined in this header file, because it's used in several
   important programs, but not all programs (as e.g. lpm.h is).)  */
#define REVNUMMajor 16
#define REVNUMMinor    52
#define REVNUMEncrypt (10 - REVNUMMajor*3)

/***************** defines for error returns ************************/
/* These values are returned in global variabel LcResult
   LcResult may also be set to system variable errno (>= 0)
*/

#define LC_NO_LOCK			-1		/* failed to create .lock file	*/
#define LC_UNLOCK_ERR		-2		/* failed to delete .lock file	*/
#define LC_MISC_ERR			-3		/* miscellaneous errors			*/
#define LC_LIST_ERR			-4		/* wrong list type				*/
#define LC_ILLEGAL_FILEPATH -5		/* filepath not valid			*/
#define LC_NOT_ADDED		-6		/* did not add to list			*/
#define LC_ADDED			-7		/* added file to list			*/
#define LC_NOT_REMOVED		-8		/* failed to remove from list	*/
#define LC_REMOVED			-9		/* file removed from list		*/
#define LC_LIST_NOT_RETURNED -10	/* failed to return entire list	*/
#define LC_LIST_RETURNED	-11		/* returned entire list			*/
#define LC_FACS_REQ_ERR		-12		/* illegal FACS request			*/
#define LC_FACS_FAILED		-13		/* FACS request failed, reason
									   printed on screen			*/
#define LC_FILE_LOCKED 		-14		/* FACS lock was successful		*/
#define LC_FILE_ALREADY_LOCKED -15	/* file already locked by FACS	*/
#define LC_FILE_NOT_LOCKED -16		/* file not locked, so not freed */
#define LC_FILE_FREED		-17		/* file freed from lock			*/
#define LC_FILE_IN_LIST		-18		/* file already in list			*/
#define LC_FILE_NOT_IN_LIST -19		/* file not in list				*/
#define LC_PID_FREED		-20		/* files for pid are freed		*/
#define LC_ID_FREED			-21		/* file for Id is freed			*/
#define LC_GROUP_FREED		-22		/* fiels in a group are freed	*/
#define LC_PID_NOT_IN_LIST	-23		/* pid not found				*/
#define LC_ID_NOT_IN_LIST	-24		/* Id not found					*/
#define LC_GROUP_ID_NOT_IN_LIST -25	/* GroupId not found			*/
#define LC_STATUS_CHANGED	-26		/* status of a file was changed	*/
#define LC_STATUS_NOT_CHANGED	-27	/* status not changed			*/
#define LC_ILLEGAL_STATUS_REQ	-28	/* see lc_facs_status_chnage()	*/
#define LC_LPM_VERIFY_ERR	-29		/* lpm_verify() failed */

/********defines for type of facs_req() calls (ToDo arg) *************/

#define LC_FACS_SOFT_SINGLE_LOCK	1
#define LC_FACS_SOFT_MULTI_LOCK		2
#define LC_FACS_INTEREST			3
#define LC_FACS_FREE				4
#define LC_FACS_FREE_ID				5
#define LC_FACS_FREE_GROUP_ID		6
#define LC_FACS_FREE_PID			7
#define LC_FACS_NOTIFY_MOD			8
#define LC_FACS_NOTIFY_FREE			9
#define LC_FACS_CHG_STATUS			10
#define LC_FACS_ABS_NOTIFY_LOCK		11
#define LC_FACS_ABS_ERROR_LOCK		12

/******************** miscellaneous defines *************************/

#define LC_MAX_ID		30000	/* unique id for facs call			*/
#define LC_MAX_HOSTS	10		/* see lc_utils.c for use			*/
#define LC_MAXPATH		256 	/* use this instead of PATH_MAX in 
							       limits.h (1023)					*/
#define LC_MAXMSG		256		/* for messages  					*/
#define LC_NOSYSERR		-1		/* for messages						*/
#define LC_REMOVE		1		/* remove from a list				*/
#define LC_ADD			2		/* add to a list					*/

/**************** defines for creation of .lock file ****************/

#define LC_MAXTRIES		9	/* number of times to try and open .lock */
#define LC_NAPTIME		2	/* sleep(LC_NAPTIME) between LC_MAXTRIES*/

							
/************** defines for type of list file to lock ***************/
/*	NOTE: these values are indicies into *ListControl[] 
		  (defined in lc_utils.c), do not change their values		
*/
#define LC_LST_FACS			1	/* list access control		*/
#define LC_LST_MEMBERS		2	/* list members list		*/
#define LC_LST_LAYOUTS		3	/* list of layout names		*/
#define LC_LST_OUTPUT		4	/* list of output files		*/
#define LC_LST_MP_SETUP		5	/* list of masterpage items	*/


/*************** defines for LC_FACS_REC.FacsStatus *****************/

#define LC_LOCKED_ABS	0x1		/* file is locked absolutely		*/
#define LC_LOCKED_SOFT	0x2		/* soft lock						*/
#define LC_INTEREST		0x4		/* file is unlocked but not free	*/
#define LC_NOTIFY_MOD	0x8		/* notify if file is modified		*/
#define LC_NOTIFY_FREE	0x10	/* notify if file is free			*/

/********************************************************************/
/************** structures to define each type of list **************/

typedef struct					/* define the facs request			*/
	{
	int FacsStatus;				/* see defines this file, set by
								   lc_facs_req()					*/
	int UserPid;				/* pid of file owner				*/
	int ForPid;					/* pid UserPid is doing work for	*/
	int ModifyPid;				/* notify this pid if file is
								   modified, set by lc_facs_req when
								   LC_FACS_NOTIFY_MOD is passed	*/
	int FreePid;				/* notify this pid when file is 
								   free, set by lc_facs_req() when
								   LC_FACS_NOTIFY_FREE is passed	*/
	int Id;						/* returned by lc_facs_req() when
								   request is successful			*/
	int GroupId;				/* set by caller					*/	
	char FilePath[LC_MAXPATH];	/* from p_path(), w/o /penta/tree	*/
	char UserHostName[PMAXHOSTNAMELEN];	/* = 64, defined in
										   sys/param.h, set in 
										   lc_facs_req()			*/
	char UserName[LOGNAME_MAX];	/* = 8, defined in limits.h, set 
								   by lc_facs_req()					*/
	}LC_FACS_REC;

typedef struct 
{
  int flag;						/* flag word 0 = normal 1 = MPage */
  int type;						/* sub definition of MPage data */
  int index;					/* index in list of file */
  char filename[MAX_NAME];		/* MAX_NAME in p_lib.h */
  void *fl;
} LC_ANY_REC;

typedef struct					/* define the members list 			*/
{
  int flag;						/* flag word 0 = normal 1 = MPage */
  int type;						/* flag = 0, + also indicates the
											   order of MPage useage
								   flag = 1, 1 = member of entry
											 2 = dependent on entry
											 3 = unit entry 
												 index = chapter order*/
  int index;					/* index in list of file */
  char filename[MAX_NAME];		/* MAX_NAME in p_lib.h */
  void *fl;
} LC_MEMB_REC;

#define DM		0
#define MPAGE	1
#define MASTER	2

#define MOD_DM				0x1
#define MOD_MPAGE			0x2
#define MOD_BOTH			(MOD_DM | MOD_MPAGE)		/* = 0x3 */
#define MOD_CHANGED_PAGE	0x4

typedef struct					/* define the layout list			*/
{	
  int flag;						/* flag = DM = normal;
										  MPAGE = MPage;
										  MASTER = Master for DM and MP;
								*/
  int type;			/* flag = 0
								          0, MOD_DM = modified by DM
										  MOD_MPAGE = modified by MPage
										  MOD_BOTH = modified by both
										  MOD_CHANGED_PAGE = Contents differ from
															previous MPage run.
					   flag = 1
											 -16 breaking style name
											 -15 illustration style name
								             -14 vert just style name
								             -13 sidenote style name
								             -12 footnote style name
								             -11 chapter style name
								             -4  LogicRev, Rev# (0/1/2)
								             -3  LastPage, ending page#
										     -2  Page_name_template
										     -1  FirstPage, page#
											  0  special page, page#
											  1  default_even;
											  2  default_odd;
											  3  chapter_even;
											  4  chapter_odd;
											  5  chp_even_1;
											  6  chp_odd_1;
											  7  chp_even_2;
											  8  chp_odd_2;
											  9  chp_even_3;
											 10  chp_odd_3;
											 11  chp_even_4;
											 12  chp_odd_4;
											 13  blank_even;
											 14  blank_odd;		*/
  int index;					/* index in list of file */
  char filename[MAX_NAME];		/* MAX_NAME in p_lib.h */
  void *fl;
  char PageNum[SZ_PAGE_STRING];	/* SZ_PAGE_STRING in p_lib.h	*/
								/* If flag=1 & type=-4:  
									PageNum=1 for rev7 illustration-placement logic.
									PageNum=0 for pre-Rev7.						*/
  int SecPageNum;				/* For flag=0 & type=2:
  									Secondary page number (if applicable).  */
} LC_LAY_REC;

typedef struct					/* define the output list			*/
	{
	char OutputPath[LC_MAXPATH];
	}LC_OUT_REC;

/********************************************************************/
/*************** declared in lc_utils.c *****************************/

extern char LcErrMsg[];			/* msg to/not to be printed		*/
extern int LcForPid;			/* pid doing work for			*/
extern char LcLockedBy[];		/* username who has file locked	*/
extern int LcLockedByFather;	/* file requested is locked by
								   father						*/
extern char LcLockedOn[];		/* host of who has file locked	*/
extern int LcLockedPid;			/* pid file locked by			*/
extern int LcPrintErrMsgs;		/* init in lc_utils to 1, chged
								   by lc_init()					*/
extern int LcResult;			/* results of a request			*/
extern char *ListControl[];		/* extensions to list files		*/
extern char ListExt[];			/* extension for list file		*/
extern char SemExt[];			/* extension for semaphore file	*/

#include "list_control.f"

#endif
