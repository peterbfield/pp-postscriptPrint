/*   *****     **    **       *       ******     *******    *******
    **   **    **    **      ***      **   **    **         **    **
    **         **    **     ** **     **   **    **         **     **
    **         **    **    **   **    **   **    **         **     **
     *****     ********    *******    ******     *****      **     **
         **    **    **    **   **    ****       **         **     **
         **    **    **    **   **    ** **      **         **     **
    **   **    **    **    **   **    **  **     **         **    **
     *****     **    **    **   **    **   **    *******    *******   */

/* #define FILE_TRACE 1		*//* */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "p_lib.h"
#include "noproto.h"
#include "mem_lay.f"
#include "list_control.h"
#include "traces.h"

extern int lpm_checkpid( char *host, int apid);
extern int lpm_verify( void);

#ifdef FILE_TRACE
static FILE *ft;
#endif

/****************** declare all global vars here ********************/
/*	declared as extern in list_control.h */
/* names for list files, NOTE: do not change their order
   since they are indexed by the LC_LST... defines in list_control.h	*/
char *OldListControl[] ={
	"NULL",
	".facs1",
	".member1",
	".layout1",
	".output1",
	".mp_setup1"};
char *ListControl[] = {
	"NULL",
	".facs1",
	".member2",
	".layout2",
	".output1",
	".mp_setup1"};

char ListExt[] = ".list";		/* extension for list file */
char SemExt[] = ".lock";		/* extension for semaphore file */
char LcErrMsg[LC_MAXMSG];		/* all error msgs here */
int LcForPid;					/* pid of who a pid is doing work */
char LcLockedBy[LOGNAME_MAX+1];	/* = 8, no NULL */
int LcLockedByFather;			/* father (LcForPid) has lock */
char LcLockedOn[PMAXHOSTNAMELEN];
int LcLockedPid;				/* pid file is locked by */
int LcPrintErrMsgs = 1;			/* chg by lc_init() */
int LcResult;					/* result of a request */

/****************** declare all externals here **********************/

#if !OS_linux
extern char *sys_errlist[];		/* used by lc_errmsg() */
extern int sys_nerr;			/* used by lc_errmsg() */
#endif

/***************** declare all static vars here *********************/

static int LcChgStatus;			/* store status chg from
								   lc_facs_chg_status() */
static int LcId;				/* 1 -> LC_ID_MAX */
static int LcLocalResult;
static char LcMyHost[PMAXHOSTNAMELEN]; /* my host in LcAllHosts */
static int LcMyPid;				/* from getpid() */
static char LcUserName[LOGNAME_MAX+1]; /* from getlogin() */
static char LockName[LC_MAXPATH]; /* xxx/xxx/xxx/.facs.lock */
static char LstName[LC_MAXPATH]; /* xxx/xxx/xxx/.facs.list */
static char OldLstName[LC_MAXPATH];	/* xxx/xxx/xxx/.facs.list */
static char *TreeName;
static int Dir;
static char *SubDir;

/****************** declare all static funcs here ********************/
/*    The following routines are used by the list control functions and
	not by the programmer calling the list control function         */

/* build_paths(), build partial pathnames for files */
static void build_paths(int);

/* lc_create_facs_rec(), create a facs record */
static int lc_create_facs_rec(char *, int, char *, char *, int, int,
							  LC_FACS_REC *, int *);

/* lc_facs_free(), free a file from facs */
static int lc_facs_free(LC_FACS_REC *, char *, off_t *, int *);

/* scan for abs locks per pid or group */
int lc_facs_abs_lock_check (LC_FACS_REC *, char *, off_t *, int *, int);

/* lc_facs_free_by_val(), free a file by pid, id, or group_id	*/
static int lc_facs_free_by_val(LC_FACS_REC *, char *, off_t *, int *,
							   int);

/* lc_facs_lock(), lock a file with FACS */
static int lc_facs_lock(LC_FACS_REC *, char *, off_t);

/* lc_facs_search(), seach a facs list	*/
static LC_FACS_REC *lc_facs_search(LC_FACS_REC *, char *, off_t, int);

/* lc_facs_status_change(), chg the status of a file */
static int lc_facs_status_change(LC_FACS_REC *, char *, off_t);

/* lc_get_listsize(), return size-of a .list record type */
static int lc_get_listsize(int, int *);

/* lc_layout(), for .layout list NOTE: off_t is defined in type.h */
#ifdef NOT_USED_BY_ANYONE
static int lc_layout(off_t *, char *, int, void *);
#endif 

/* create .lock file */
static int lc_lock(void);

#ifdef NOT_USED_BY_ANYONE
static int lc_noop();			/* lc_noop(), no op func */
#endif 

/* lc_notify_free(), notify users who have asked to be, when a file is freed */
static void lc_notify_free(LC_FACS_REC *, char *, off_t);

/* lc_member(), for .members list file */
#ifdef NOT_USED_BY_ANYONE
static int lc_member(off_t *, char *, int, void *);

/* lc_output(), for .output list file */
static int lc_output(off_t *, char *, int, void *);
#endif 

/* lc_read_list(), read the entire list	*/
static int lc_read_list(char **Ptr, off_t *ListSize, Pfd *Fd, int, int);

/* lc_rewrite_list(), rewrite a modified list */
static int lc_rewrite_list(char *, off_t);

/* lc_set_result, set LcLocalResult */
static void lc_set_result(void);

/* lc_unlock(), deletes the .lock file */
static int lc_unlock(void);

/* perform functions on list files NOTE: do not change there order
   since they are indexed by LC_LST.... defines in list_control.h */
#ifdef NOT_USED_BY_ANYONE
static int (*lc_funcs[])() = {
	lc_noop,					/* place holder */
	lc_noop,					/* facs not needed */
	lc_member,
	lc_layout,
	lc_output};
#endif 

/************** START LIST CONTROL FUNCTIONS HERE *******************/
/*  The following routines are called by external programs.  They are
	declared in list_control.f .        */

/*********************** add a file to a list ***********************/

#ifdef NOT_USED_BY_ANYONE
int lc_add(char *treename, int dir,  char *subdir, void *List, int ListType)
{
	char ListErrMsg[LC_MAXMSG];
	int ListRecSize;
	off_t ListSize;
	char *Ptr = (char*)0;
	Pfd Fd;

	LcResult = LC_NOT_ADDED;
	LcLocalResult = 0;
	TreeName = treename;
	Dir = dir;
	SubDir = subdir;
#ifdef FILE_TRACE
	fprintf(ft,"lc_add,tree=%s,dir=%d,sub=%s,typ=%d\n",
			treename,dir,subdir,ListType);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "lc_add,tree=%s,dir=%d,sub=%s,typ=%d\n",
				  treename,dir,subdir,ListType);
#endif
	/* do size 1st since it checks on validity of ListType */
	if(lc_get_listsize(ListType, &ListRecSize))
		return(LcLocalResult);
	build_paths(ListType);
	if(lc_lock())
		return(LcLocalResult);
	if(lc_read_list(&Ptr, &ListSize, &Fd, 1))
	{
		LcLocalResult = 1;
		goto ulock1;
	}
	if(ListSize)
	{
		/* if file already exists in list */
		if((*lc_funcs[ListType])(&ListSize, Ptr, LC_ADD, List))
		{
			LcResult = LC_FILE_IN_LIST;
			LcLocalResult = 1;
			goto ulock1;
		}
	}
	p_seek(Fd, (int32)0, SEEK_END);
	if((p_write((char*)List, ListRecSize, 1, Fd, NEXT_REC, SW_NOSWAP)) < 1)
	{
		sprintf(ListErrMsg, "failed to write to %s", LstName);
		lc_errmsg(ListErrMsg, errno);
		LcResult = errno;
		LcLocalResult = 1;
		goto ulock1;
	}
	ListSize += ListRecSize;
	LcResult = LC_ADDED;
  ulock1:
	if(Ptr)
		p_free(Ptr);
	p_close(Fd);
	lc_unlock();
	if(!LcLocalResult)
	{
		LcLocalResult = LcId;
		lc_set_result();
	}
	else
		LcLocalResult = 0;
	
#ifdef FILE_TRACE
	fprintf(ft,"lc_add ret=%d,LcRes=%d\n",
			LcLocalResult,LcResult);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "lc_add ret=%d,LcRes=%d\n",
				  LcLocalResult,LcResult);
#endif
	return(LcLocalResult);
}
#endif /* NOT_USED_BY_ANYONE */

/***************** output formated error message ********************/

void lc_errmsg(char *msg, int errnum)
{
	int msglen;
	
	if(errnum < 0)
		sprintf(LcErrMsg, "%.*s",LC_MAXMSG - 6, msg);
	else if(errnum > sys_nerr)
		sprintf(LcErrMsg, "%.*s: %d",
				LC_MAXMSG - 10, msg, errnum);
	else
	{
		msglen = strlen(sys_errlist[errnum]);
		if((strlen(msg) + strlen(sys_errlist[errnum]) + 12) >  LC_MAXMSG)
			msglen = LC_MAXMSG - strlen(msg) - 12;
		sprintf(LcErrMsg, "%s: %d, %.*s",
				msg, errnum, msglen, sys_errlist[errnum]);
	}
	if(LcPrintErrMsgs)
		p_info(PI_ELOG, "LC - %s\n", LcErrMsg);
#ifdef FILE_TRACE
	fprintf(ft,"LC - %s\n", LcErrMsg);
	fflush(ft);
#endif
	return;
}

/****************** change the status on a file *********************/

#ifdef NOT_USED_BY_ANYONE
int lc_facs_chg_status(char *TreeName, int Id, int NewStatus)
{
	int result;
	
	LcChgStatus = NewStatus;
	result = lc_facs_req(TreeName, OTHER_FILE, NO_STRING, NO_STRING,
						 Id, LC_FACS_CHG_STATUS);
	return(result);
}
#endif /* NOT_USED_BY_ANYONE */

/******************* free a file by Id ******************************/

int lc_facs_free_id(char *TreeName, int Id)
{
	int result;

	result = lc_facs_req(TreeName, OTHER_FILE, NO_STRING, NO_STRING,
						 Id, LC_FACS_FREE_ID);
	return(result);
}

/****************** free all files by pid ***************************/

#ifdef NOT_USED_BY_ANYONE
int lc_facs_free_pid(char *TreeName, int Pid)
{
	int result;

	result = lc_facs_req(TreeName, OTHER_FILE, NO_STRING, NO_STRING,
						 Pid, LC_FACS_FREE_PID);
	return(result);
}
#endif /* NOT_USED_BY_ANYONE */

/***************** free all files by GroupId ************************/

int lc_facs_free_group_id(char * TreeName, int GroupId)
{
	int result;

	result = lc_facs_req(TreeName, OTHER_FILE, NO_STRING, NO_STRING,
						 GroupId, LC_FACS_FREE_GROUP_ID);
	return(result);
}

/******************* do a FACS request ******************************/

int lc_facs_req(char *treename, int FileDir, char *FileSubDir,
				char *Fname, int GroupId, int ToDo)
{
	static int Nverify = 10;
	LC_FACS_REC FacsReq;
	int CreateList;
	char ListErrMsg[LC_MAXMSG];
	int ListRecSize;
	off_t ListSize;
	int NotifyIfFree;
	off_t SaveListSize;
	char *Ptr = (char*)0;		/* ptr to entire list */
	int result = 0;
	Pfd Fd;
	int test_err = 0;

	LcLocalResult = 0;
	ListRecSize = sizeof(LC_FACS_REC);
	NotifyIfFree = 0;
	TreeName = treename;
	Dir = CONTROL_LISTS;
	SubDir = NO_STRING;
	/* init these globals each call	*/
	LcResult = LC_FACS_FAILED;
	memset(LcLockedBy, 0, sizeof(LcLockedBy));
	memset(LcLockedOn, 0, sizeof(LcLockedOn));
	LcLockedPid = 0;
	
#ifdef FILE_TRACE
	fprintf(ft,"facs_req, Pid %d,{%s/(%d)%s/%s},Group=%d,Do=%d,ForPid=%d\n",
			LcMyPid,treename,FileDir,FileSubDir,Fname,GroupId,ToDo,LcForPid);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "facs_req, tree=%s,Dir=%d,Sub=%s,Fname=%s,Group=%d,Do=%d,ForPid=%d\n",
				  treename,FileDir,FileSubDir,Fname,GroupId,ToDo,LcForPid);
#endif
	
	switch(ToDo) {
	  case LC_FACS_ABS_NOTIFY_LOCK:
	  case LC_FACS_ABS_ERROR_LOCK:
	  case LC_FACS_SOFT_SINGLE_LOCK:
	  case LC_FACS_SOFT_MULTI_LOCK:
		if( Nverify < 0)
		{
		  verifyerr:
			lc_errmsg( "LPM verify error", LC_NOSYSERR);
			LcResult = LC_LPM_VERIFY_ERR;
			return LcLocalResult;
		}
		
		if( --Nverify <= 0)
		{
			if( !lpm_verify())
			{
				Nverify = -1;
				goto verifyerr;
			}
			else
				Nverify = 10;
		}
		break;
	}
	build_paths(LC_LST_FACS);
	if(lc_lock())
		return(LcLocalResult);
	if(lc_create_facs_rec(TreeName, FileDir, FileSubDir, Fname,
						  GroupId, ToDo, &FacsReq, &CreateList))
		return(LcLocalResult);
	if(lc_read_list(&Ptr, &ListSize, &Fd, CreateList, LC_LST_FACS))
	{
		LcLocalResult = 1;
		goto ferr;
	}
	SaveListSize = ListSize;
	switch(ToDo)
	{
	  case LC_FACS_ABS_NOTIFY_LOCK:
	  case LC_FACS_ABS_ERROR_LOCK:
	  case LC_FACS_SOFT_SINGLE_LOCK:
	  case LC_FACS_SOFT_MULTI_LOCK:
		if(ListSize)
		{
			int count;

			for(count = 0; count < 5; count++) /* avoid loop */
			{
				result = lc_facs_lock(&FacsReq, Ptr, ListSize);
				if (trace_lc)
					p_info(PI_TRACE, "facs-look-up %s = %d\n",
							  FacsReq.FilePath, result);
#ifdef FILE_TRACE
				fprintf(ft,"facs-look-up %s = %d\n",
						FacsReq.FilePath, result);
				fflush(ft);
#endif
				/*
				 *	1	hard
				 *	2	self
				 *	4	sibling
				 *	8	other
				 */
				if(result & 9)
				{
					int lockable;
					int Sz;
					LC_FACS_REC *Facs;

					lockable = lpm_checkpid(LcLockedOn,LcLockedPid);
#ifdef FILE_TRACE
					fprintf(ft,"facs pid check = %d\n",ret);
#endif
					/* 0 = unlockable (pid exists),
					 * 1 = lockable (pid doesn't exit).
					 */
					if( !lockable)
						break;
					
					/* clear this pid! */
#ifdef FILE_TRACE
					fprintf(ft,"clear pid %d on %s\n",
							LcLockedPid,LcLockedOn);
#endif
					Facs = (LC_FACS_REC *)Ptr;
					Sz = 0;
					while(Sz < ListSize)
					{
						if(strncmp(Facs -> UserHostName, LcLockedOn,
								   PMAXHOSTNAMELEN) != 0)
						{
							Sz += sizeof(LC_FACS_REC);
							Facs++;
							continue;
						}
						if(Facs -> UserPid != LcLockedPid)
						{
							Sz += sizeof(LC_FACS_REC);
							Facs++;
							continue;
						}
#ifdef FILE_TRACE
						fprintf(ft,"      clearing %s\n",
								Facs -> FilePath);
#endif
						memcpy((char*)Facs, (char*)(Facs + 1),
							   ListSize - (Sz + sizeof(LC_FACS_REC)));
						ListSize -= sizeof(LC_FACS_REC);
					}				/* end for all facs recs */
					p_close(Fd);
					if(ListSize <= 0)
						p_unlink(TreeName,Dir,SubDir,LstName);
					else
						lc_rewrite_list(Ptr, ListSize);
					lc_read_list(&Ptr,&ListSize,&Fd,CreateList, LC_LST_FACS);
					continue;	/* force another loop */
				}
				break;
			}
			switch(ToDo)
			{
			  case LC_FACS_ABS_NOTIFY_LOCK:
				test_err = 9;
				if(result & 4)
					LcLockedByFather = 1;
				else
					LcLockedByFather = 0;
				break;
			  case LC_FACS_ABS_ERROR_LOCK:
				test_err = 13;
				break;
			  case LC_FACS_SOFT_SINGLE_LOCK:
				test_err = 11;
				break;
			  case LC_FACS_SOFT_MULTI_LOCK:
				test_err = 9;
				break;
			}
			if(result & test_err)
			{
				char temp[LC_MAXMSG];
				
				sprintf(temp, "/%s", treename);
				if (FileSubDir && *FileSubDir)
				{
					char *ptr;

					strcat(temp, "/");
					strcat(temp, FileSubDir);
					if ((ptr = strrchr(temp, '.')) != NULL)
						*ptr = '\0';
				}
				strcat(temp, "/");
				strcat(temp, Fname);
				if (result & 2)
					sprintf(ListErrMsg, "%s is locked by yourself!!!", temp);
				else
					sprintf(ListErrMsg, "%s is locked by %s on %s pid %d",
							temp, LcLockedBy, LcLockedOn, LcLockedPid);
				lc_errmsg(ListErrMsg, LC_NOSYSERR);
				LcResult = LC_FILE_ALREADY_LOCKED;
				LcLocalResult = 1;
				break;
			}
		}
		if(ToDo == LC_FACS_ABS_ERROR_LOCK || ToDo == LC_FACS_ABS_NOTIFY_LOCK)
			FacsReq.FacsStatus = LC_LOCKED_ABS;
		else
			FacsReq.FacsStatus = LC_LOCKED_SOFT;
		p_seek(Fd, (int32)0, SEEK_END);
		if((p_write((char*)&FacsReq, ListRecSize, 1, Fd,
					NEXT_REC, SW_FACS)) < 1)
		{
			sprintf(ListErrMsg, "failed to write to %s", LstName);
			lc_errmsg(ListErrMsg, errno);
			LcResult = errno;
			LcLocalResult = 1;
			break;
		}
		ListSize += ListRecSize;
		LcResult = LC_FILE_LOCKED;
		break;
		
	  case LC_FACS_INTEREST:
		lc_errmsg("option interest not implemented yet", LC_NOSYSERR);
		LcResult = LC_FACS_REQ_ERR;
		LcLocalResult = 1;
		break;
		
	  case LC_FACS_FREE:
		/* lc_facs_free() mods ListSize */
		if(ListSize)
			result = lc_facs_free(&FacsReq, Ptr, &ListSize, &NotifyIfFree);
		/* file not in .facs list */
		if(result || (SaveListSize <= 0))
		{
			sprintf(ListErrMsg, "file %s was not locked", FacsReq.FilePath);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			LcResult = LC_FILE_NOT_LOCKED;
			LcLocalResult = 1;
			break;
		}
		LcResult = LC_FILE_FREED;
		if(NotifyIfFree)
			lc_notify_free(&FacsReq, Ptr, ListSize);
		if(ListSize <= 0)
			break;
		p_close(Fd);
		if(lc_rewrite_list(Ptr, ListSize))
			LcLocalResult = 1;
		break;
		
	  case LC_FACS_FREE_GROUP_ID:
	  case LC_FACS_FREE_PID:
		if(lc_facs_abs_lock_check(&FacsReq, Ptr, &ListSize, 
								  &NotifyIfFree, ToDo))
		{
			LcLocalResult = 1;
			break;
		}
		/*FALLTHROUGH*/
	  case LC_FACS_FREE_ID:
		if((result = lc_facs_free_by_val(&FacsReq, Ptr, &ListSize,
										 &NotifyIfFree, ToDo)) != 0)
		{
			LcLocalResult = 1;
			break;
		}
		if(ListSize <= 0)
			break;
		p_close(Fd);
		if(lc_rewrite_list(Ptr, ListSize))
			LcLocalResult = 1;
		break;
		
	  case LC_FACS_NOTIFY_MOD:
		lc_errmsg("option notify-mod not implemented yet", LC_NOSYSERR);
		LcResult = LC_FACS_REQ_ERR;
		LcLocalResult = 1;
		break;
		
	  case LC_FACS_NOTIFY_FREE:
		lc_errmsg("option notify-free not implemented yet", LC_NOSYSERR);
		LcResult = LC_FACS_REQ_ERR;
		LcLocalResult = 1;
		break;
		
	  case LC_FACS_CHG_STATUS:
		if((result = lc_facs_status_change( &FacsReq, Ptr, ListSize)) != 0)
			LcLocalResult = 1;
		else if(lc_rewrite_list(Ptr, ListSize))
			LcLocalResult = 1;
		break;
		
	  default:
		sprintf(ListErrMsg, "illegal FACS request %d", ToDo);
		lc_errmsg(ListErrMsg, LC_NOSYSERR);
		LcResult = LC_FACS_REQ_ERR;
		LcLocalResult = 1;
		break;
	}							/* end switch(ToDo) */
	
  ferr:	;
	if(Ptr)
		p_free(Ptr);
	p_close(Fd);
	if(ListSize <= 0)
	{
		if((result = p_unlink(TreeName, Dir, SubDir, LstName)) != 0)
		{
			sprintf(ListErrMsg, "failed to delete 0 length %s", LstName);
			lc_errmsg(ListErrMsg, errno);
			LcResult = errno;
			LcLocalResult = 1;
		}
	}
	lc_unlock();
	/* NOTE: comment out next 7 statements to override facs */
	
	if(!LcLocalResult)
	{
		LcLocalResult = LcId;
		lc_set_result();
	}
	else
		LcLocalResult = 0;
	
#ifdef FILE_TRACE
	fprintf(ft,"true facs ret = %d,LcRes=%d,By=%s,On=%s,Pid=%d\n",
			LcLocalResult,LcResult,LcLockedBy,LcLockedOn,LcLockedPid);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "true facs ret = %d,LcRes=%d,By=%s,On=%s,Pid=%d\n",
				  LcLocalResult,LcResult,LcLockedBy,LcLockedOn,LcLockedPid);
#endif
	
	/* NOTE: uncomment next 2 statments to override facs */
	/*
	  LcLocalResult = LcId;
	  lc_set_result();
	  */
	
	return(LcLocalResult);
}

/******************** init list control *****************************/

int lc_init(int PrintErrMsgs)
{
	char *p;
#ifdef FILE_TRACE
	char trace_file_name[128];
#endif
	
	if(gethostname(LcMyHost, PMAXHOSTNAMELEN))
	{
		lc_errmsg("failed to get hostname", errno);
		return(errno);
	}
	LcMyPid = getpid();
	if((p = p_get_username()) == (char *)NULL)
	{
		lc_errmsg("failed to get login", errno);
		return(errno);
	}
	memset(LcUserName, 0, sizeof(LcUserName));
	strncpy(LcUserName, p, LOGNAME_MAX);
	
#ifdef FILE_TRACE
	sprintf(trace_file_name,"facs_trace_file.%d",getpid());
	ft = fopen(trace_file_name,"a+");
	fprintf(ft,"lc_init, host=%s, user=%s, pid=%d,%#x, prt=%d\n",
			LcMyHost, LcUserName, LcMyPid, LcMyPid, PrintErrMsgs);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "lc_init, host=%s, user=%s, pid=%d,%#x, prt=%d\n",
				  LcMyHost, LcUserName, LcMyPid, LcMyPid, PrintErrMsgs);
#endif
	
	LcPrintErrMsgs = PrintErrMsgs;
	LcForPid = 0;
	LcId = 1;
	LcLockedByFather = 0;
	return(0);
}

/******************** remove a file from a list *********************/

#ifdef NOT_USED_BY_ANYONE
int lc_remove(char *treename, int dir, char *subdir, void *List,  int ListType)
{
	char ListErrMsg[LC_MAXMSG];
	int ListRecSize;
	off_t ListSize;
	char *Ptr = (char *)0;
	Pfd FromFd;

	LcLocalResult = 0;
	LcResult = LC_NOT_REMOVED;
	TreeName = treename;
	Dir = dir;
	SubDir = subdir;
	
#ifdef FILE_TRACE
	fprintf(ft,"lc_remove,tree=%s,dir=%d,sub=%s,Type=%d\n",
			treename,dir,subdir,ListType);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "lc_remove,tree=%s,dir=%d,sub=%s,Type=%d\n",
				  treename,dir,subdir,ListType);
#endif
	/* do size 1st since it checks on validity of ListType */
	if(lc_get_listsize(ListType, &ListRecSize))
		return(LcLocalResult);
	build_paths(ListType);
	if(lc_lock())
		return(LcLocalResult);
	if(lc_read_list(&Ptr, &ListSize, &FromFd, 0))
	{
		LcLocalResult = 1;
		goto rmerr;
	}
	if(ListSize)
	{
		if((*lc_funcs[ListType])(&ListSize, Ptr, LC_REMOVE, List))
		{
			p_close(FromFd);
			if(ListSize <= 0)
			{
				LcResult = LC_REMOVED;
				goto rmerr;
			}
		}
		else
		{
			LcResult= LC_FILE_NOT_IN_LIST;
			LcLocalResult = 1;
			goto rmerr;
		}
		if(lc_rewrite_list(Ptr, ListSize))
			goto rmerr;
		LcResult = LC_REMOVED;
	}
	else
		LcResult= LC_FILE_NOT_IN_LIST;
  rmerr:	;
	if(Ptr)
		p_free(Ptr);
	p_close(FromFd);
	if(ListSize <= 0)
	{
		if(p_unlink(TreeName, Dir, SubDir, LstName))
		{
			sprintf(ListErrMsg, "failed to delete 0 length %s", LstName);
			lc_errmsg(ListErrMsg, errno);
			LcLocalResult = 1;
			LcResult = errno;
		}
	}
	lc_unlock();
	if(!LcLocalResult)
	{
		LcLocalResult = LcId;
		lc_set_result();
	}
	else
		LcLocalResult = 0;
	
#ifdef FILE_TRACE
	fprintf(ft,"lc_remove ret=%d,LcRes=%d\n",
			LcLocalResult,LcResult);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "lc_remove ret=%d,LcRes=%d\n",
				  LcLocalResult,LcResult);
#endif
	
	return(LcLocalResult);
}
#endif /* NOT_USED_BY_ANYONE */

/******************* return an entire list **************************/

#ifdef NOT_USED_BY_ANYONE
int lc_return_list(char *treename, int dir, char *subdir,
				   int ListType, off_t *ListSz, char **ListAddr)
{
	int ListRecSize;
	off_t ListSize;
	char *Ptr = '\0';
	Pfd Fd;

	LcLocalResult = 0;
	LcResult = LC_LIST_NOT_RETURNED;
	TreeName = treename;
	Dir = dir;
	SubDir = subdir;
	
#ifdef FILE_TRACE
	fprintf(ft,"ret_list,tree=%s,dir=%d,sub=-%s,Type=%d,Sz=%#x,Add=%#x\n",
			treename,dir,subdir,ListType,ListSz,ListAddr);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "ret_list,tree=%s,dir=%d,sub=-%s,Type=%d,Sz=%#x,Add=%#x\n",
				  treename,dir,subdir,ListType,ListSz,ListAddr);
#endif
	
	/* do size 1st since it checks on validity of ListType */
	if(lc_get_listsize(ListType, &ListRecSize))
		return(LcLocalResult);
	build_paths(ListType);
	if(lc_lock())
		return(LcLocalResult);
	if(lc_read_list(&Ptr, &ListSize, &Fd, 0))
	{
		LcLocalResult = 1;
		goto reterr;
	}
	if(ListSize)
	{
		*ListAddr = Ptr;		/* return this to caller */
		*ListSz = ListSize;
		LcResult = LC_LIST_RETURNED;
	}
  reterr:	;
	p_close(Fd);
	lc_unlock();
	if(!LcLocalResult)
	{
		LcLocalResult = LcId;
		lc_set_result();
	}
	else
		LcLocalResult = 0;
	
#ifdef FILE_TRACE
	fprintf(ft,"ret_list ret= %d,LcRes=%d\n", LcLocalResult,LcResult);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "ret_list ret= %d,LcRes=%d\n", LcLocalResult,LcResult);
#endif
	
	return(LcLocalResult);
}
#endif /* NOT_USED_BY_ANYONE */

/************** Lock a list file and read it in a queue *************/

int lc_lock_and_read_list_file(char *treename, int dir, char *subdir,
							   int ListType, LLIST *LList, int lock_file)
{
	int ListRecSize, total = 0;
	off_t ListSize;
	Pfd Fd = P_ERROR;
	char *Ptr = '\0', *ptr1, *ptr2;
	int count;

	LcLocalResult = 0;
	LcResult = LC_LIST_NOT_RETURNED;
	TreeName = treename;
	Dir = dir;
	SubDir = subdir;
	
#ifdef FILE_TRACE
	fprintf(ft,"LockAndReadListFile,tree=%s,dir=%d,sub=-%s,Type=%d, LList=%p\n",
			treename,dir,subdir,ListType, LList);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "LockAndReadListFile,tree=%s,dir=%d,sub=-%s,Type=%d, LList=%p\n",
				  treename,dir,subdir,ListType, (void *)LList);
#endif
	/* do size 1st since it checks on validity of ListType */
	if(lc_get_listsize(ListType, &ListRecSize))
		return(LcLocalResult);
	build_paths(ListType);
	if (lock_file)
		if(lc_lock())
			return(LcLocalResult);
	if(lc_read_list(&Ptr, &ListSize, &Fd, 0, 0))
	{
		LcLocalResult = 0;
		p_close(Fd);
		return(LcLocalResult);
	}
	ptr1 = ptr2 = Ptr;
	if(trace_lc)
		p_info(PI_TRACE, "listfile is in %p, total=%d, ListSize=%d, LList=%p.\n",
			Ptr, total, ListSize, (void *)LList);
	count = 0;
	while (total < ListSize)
	{
#ifdef TRACE
		if(trace_lc && (++count == 1000))	/* (Warning trace for huge lists) */
			p_info(PI_TRACE, "NOTICE: A .list file in %s/%s has >1000 entries.\n", treename, subdir);
#endif
		if (ListType == LC_LST_LAYOUTS || ListType == LC_LST_MP_SETUP)
		{
			LC_LAY_REC *LLptr;
			
			LLptr = (LC_LAY_REC *)p_alloc(sizeof(LC_LAY_REC));
			if(trace_lc)
			{
				p_info(PI_TRACE, "  LLptr=%p, list is:\n", LLptr);
				p_info(PI_TRACE, "%c %c %c\n", *ptr1, *(ptr1+1), *(ptr1+2));
				p_info(PI_TRACE, "%s\n", ptr1);
			}
			if(sscanf(ptr1, "%d %d %d %s %s %d\n",
					  &LLptr -> flag, &LLptr -> type, &LLptr -> index,
					  LLptr -> filename, LLptr -> PageNum, &LLptr -> SecPageNum) < 5)
			{
				if(trace_lc)
					p_info(PI_TRACE, "sscanf failed\n");
				p_info(PI_ELOG, "sscanf failed\n");
				LcLocalResult = 0;
				return(LcLocalResult);
			}
			if(trace_lc)
				p_info(PI_TRACE, "  type=%d, LList=%p.\n", LLptr -> type, (void *)LList);
			LLinsertTail(LList, (LLD)LLptr);
		}
		else
		{
			LC_MEMB_REC *LLptr;
			
			LLptr = (LC_MEMB_REC *)p_alloc(sizeof(LC_MEMB_REC));
			if (sscanf(ptr1, "%d %d %d %s\n",
					   &LLptr -> flag, &LLptr -> type, &LLptr -> index,
					   LLptr -> filename) != 4)
			{
				p_info(PI_ELOG, "sscanf failed\n");
				LcLocalResult = 0;
				return(LcLocalResult);
			}
			LLinsertTail(LList, (LLD)LLptr);
		}
		ptr1 = strchr(ptr1, '\n');
		*ptr1 = '\0';
		if(trace_lc)
			p_info(PI_TRACE, "Extracted '%s' from listfile into loc %d.\n", ptr2, total);
		total += strlen(ptr2) + 1;
		ptr2 = ++ptr1;
	}
	if (ListSize)
		p_free(Ptr);
	LcResult = LC_LIST_RETURNED;
	LcLocalResult = 1;
	p_close(Fd);
	return(LcLocalResult);
}

/************************ unlock list ********************************/
int lc_unlock_list_file(char *treename, int dir, char *subdir, int ListType)
{
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "unlock_list_file,tree=%s,dir=%d,sub=-%s,Type=%d,\n",
				  treename,dir,subdir,ListType);
#endif
	LcLocalResult = 0;
	LcResult = LC_FILE_NOT_LOCKED;
	TreeName = treename;
	Dir = dir;
	SubDir = subdir;
	build_paths(ListType);
	LcLocalResult = lc_unlock();
	if(!LcLocalResult)
		LcResult = LC_FILE_FREED;
	return(LcLocalResult);
}

/************** write list to disk and unlock it *********************/

int lc_write_and_unlock_list_file(char *treename, int dir, char *subdir,
								  int ListType, LLIST *LList)
{
	LcLocalResult = 0;
	LcResult = LC_FILE_NOT_LOCKED;
	
#ifdef FILE_TRACE
	fprintf(ft, "write_unlock_list_file,tree=%s,dir=%d,sub=-%s,Type=%d,\n",
			treename,dir,subdir,ListType);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "write_unlock_list_file,tree=%s,dir=%d,sub=-%s,Type=%d,\n",
				  treename,dir,subdir,ListType);
#endif
	LcLocalResult = lc_write_list_file(treename, dir, subdir, ListType, LList);
	lc_unlock();
	if(!LcLocalResult)
		LcResult = LC_FILE_FREED;
	return(LcLocalResult);
}

/************** write list to disk *********************/
int lc_write_list_file(char *treename, int dir, char *subdir,
					   int ListType, LLIST *orig_LList)
{
	int ListRecSize;
	int key;
	Pfd pfd;
	char line[120];
	LLIST *LList = NULL;

	LcLocalResult = 0;
	LcResult = LC_FILE_NOT_LOCKED;
	TreeName = treename;
	Dir = dir;
	SubDir = subdir;
	
#ifdef FILE_TRACE
	fprintf(ft, "write_list_file,tree=%s,dir=%d,sub=-%s,Type=%d,\n",
			treename,dir,subdir,ListType);
	fflush(ft);
#endif
	
#ifdef TRACE
	if(trace_lc)
		p_info(PI_TRACE, "write_list_file,tree=%s,dir=%d,sub=-%s,Type=%d,\n",
				  treename,dir,subdir,ListType);
#endif
	/* do size 1st since it checks on validity of ListType */
	/* First things first, resort list using the index before writing */
	LList = LLcopyList(orig_LList);
								/* The .mp_setup1.list is subject to duplicate
									entries, for whatever reason.  These dupes
									cause only trouble. Loop to remove them. */
	if (ListType == LC_LST_MP_SETUP)
	{
		LC_LAY_REC *LLptr1, *LLptr2;
								/* Loop just once, comparing adjacent pairs.
									List generally in order, so this works. */
		LLptr1 = (LC_LAY_REC *)LLhead(LList, &key);
		for (LLptr2 = (LC_LAY_REC *)LLnext(LList, &key);  LLptr2;
			 LLptr1 = LLptr2, LLptr2 = (LC_LAY_REC *)LLnext(LList, &key))
		{						/* Continue to compare next pair if: */
			if (LLptr1->index != LLptr2->index)
				continue;		/*   index differs, or */
			if (LLptr1->type != LLptr2->type)
				continue;		/*   type differs, or */
			if (strcmp((char *)LLptr1->PageNum, (char *)LLptr2->PageNum))
				continue;		/*   PageNum differs. */
								/* Duplicates found! Trace your intent, then
									remove the 2nd item. */
			if (trace_lc)
				{
				p_info(PI_TRACE, "Removing list item %d, %d, %d, %s, %s\n",
						  LLptr2 -> flag, LLptr2 -> type, LLptr2 -> index,
						  LLptr2 -> filename, LLptr2 -> PageNum);
				LLtrace(LList, print_layout);
				}
			LLptr2 = (LC_LAY_REC *)LLrmItemAtPos(LList, key, TRUE);
			LLptr2 = LLptr1;	/* Back up 1 step, continue. */
			key--;
		}
	}
	LLSORTED(LList) = FALSE;
	LLsort(LList, sort_compare_index);
	if(lc_get_listsize(ListType, &ListRecSize))
		return(LcLocalResult);
	build_paths(ListType);
	pfd = p_open(treename, dir, subdir, LstName, "w+");
	if (ListType == LC_LST_LAYOUTS || ListType == LC_LST_MP_SETUP)
	{
		LC_LAY_REC *LLptr;

		for (LLptr = (LC_LAY_REC *)LLhead(LList, &key);  LLptr;
			 LLptr = (LC_LAY_REC *)LLnext(LList, &key))
		{
			if (trace_lc)
				p_info(PI_TRACE, "Adding Layout %d, %d, %d, %s, %s, %d\n",
						  LLptr -> flag, LLptr -> type, LLptr -> index,
						  LLptr -> filename, LLptr -> PageNum, LLptr -> SecPageNum);
			sprintf(line, "%d %d %d %s %s %d\n",
					LLptr -> flag, LLptr -> type, LLptr -> index,
					LLptr -> filename, LLptr -> PageNum, LLptr -> SecPageNum);
			p_fputs(line, pfd);
		}
	}
	else						/* ListType == LC_MEMB_LIST */
	{
		LC_MEMB_REC *LLptr;

		for (LLptr = (LC_MEMB_REC *)LLhead(LList, &key); LLptr;
			 LLptr = (LC_MEMB_REC *)LLnext(LList, &key))
		{
			if (trace_lc)
				p_info(PI_TRACE, "Adding TextFile %d, %d, %d, %s\n",
						  LLptr -> flag, LLptr -> type, LLptr -> index,
						  LLptr -> filename);
			sprintf(line, "%d %d %d %s\n",
					LLptr -> flag, LLptr -> type, LLptr -> index,
					LLptr -> filename);
			p_fputs(line, pfd);
		}
	}
	p_close(pfd);
	LLclearList(LList);
	if(!LcLocalResult)
	{
		LcLocalResult = LcId;
		lc_set_result();
	}
	else
		LcLocalResult = 0;
	return(LcLocalResult);
}

/************** look up parent dir *********************/

int lc_parent(char *tree, char *dir, char *parent)
{
	LLIST *QueMemList;
	LC_MEMB_REC *Mem;
	int index = 0;
	
	QueMemList = LLcreateList();
	lc_lock_and_read_list_file(tree, DESK, dir, 
							   LC_LST_MEMBERS, QueMemList, FALSE);
	*parent = 0;
	for(Mem = (LC_MEMB_REC *)LLhead(QueMemList, &index); Mem;
		Mem = (LC_MEMB_REC *)LLnext(QueMemList, &index))
	{
		if(Mem -> flag == 1 && (Mem -> type == 1 || Mem -> type == 2))
			strcpy(parent, Mem -> filename);
	}
	LLclear(QueMemList);
	return(*parent);
}

/*********************************************************************/
/*	NOTE:
	The following routines are used by the list control functions and not
	by the programmer calling the list control function
	*/

/*********************************************************************/

/******************** build partial pathnames for files **************/

static void build_paths(int List)
{
	switch(List)
	{
	  case LC_LST_MEMBERS:
	  case LC_LST_LAYOUTS:
	  case LC_LST_MP_SETUP:
	  case LC_LST_FACS:
	  case LC_LST_OUTPUT:
		sprintf(OldLstName, "%s%s", OldListControl[List], ListExt);
		sprintf(LstName, "%s%s", ListControl[List], ListExt);
		sprintf(LockName, "%s%s", ListControl[List], SemExt);
		break;
	}
	return;
}

/********************* build a LC_FACS_REC ***************************/

static int lc_create_facs_rec(char *Tree, int Dir, char *SubDir, char *Fname, 
					   int GroupId, int ToDo, LC_FACS_REC *FacsReq,
					   int *CreateList)
{
	int FilesDir;
	char *FilePath;
	int i;
	int LcError = 0;
	char ListErrMsg[LC_MAXMSG];
	char *p1, *p2;

	*CreateList = 0;
	FilesDir = Dir;
	/* lock as text file				*/
	if((Dir == FO_FILE) || (Dir == MAP_FILE) || (Dir == MAGERR_FILE))
		FilesDir = TEXT_FILE;
	memset((char *)FacsReq, 0, sizeof(LC_FACS_REC));
	FacsReq -> UserPid = LcMyPid;
	FacsReq -> ForPid = LcForPid;
	FacsReq -> GroupId = GroupId;
	strncpy(FacsReq -> UserHostName, LcMyHost, PMAXHOSTNAMELEN);
	strncpy(FacsReq -> UserName, LcUserName, LOGNAME_MAX);
	switch(ToDo)
	{
	  case LC_FACS_ABS_NOTIFY_LOCK:
	  case LC_FACS_ABS_ERROR_LOCK:
	  case LC_FACS_SOFT_SINGLE_LOCK:
	  case LC_FACS_SOFT_MULTI_LOCK:
		*CreateList = 1;
		/*FALLTHROUGH*/
	  case LC_FACS_FREE:
		FacsReq -> Id = LcId;
		FilePath = p_path(Tree, FilesDir, SubDir, Fname);
		p1 = FilePath;
		for(i = 1; i <= 3; i++)
		{
			if((p2 = strchr(p1, '/')) == NULL)
				break;
			p1 = p2 + 1;
		}
		if(p1 == NULL)
		{
			sprintf(ListErrMsg, "illegal file path %s", FilePath);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			LcResult = LC_ILLEGAL_FILEPATH;
			LcError = 1;
			break;
		}
		FilePath = p1 - 1;
		strcpy(FacsReq -> FilePath, FilePath);
		break;
		
	  case LC_FACS_INTEREST:
		break;
		
	  case LC_FACS_FREE_ID:
		/* stored here*/
		FacsReq -> Id = FacsReq -> GroupId;
		FacsReq -> GroupId = 0;
		break;
		
	  case LC_FACS_FREE_GROUP_ID:
		break;
		
	  case LC_FACS_FREE_PID:
		FacsReq -> UserPid = FacsReq -> GroupId;
		FacsReq -> GroupId = 0;
		break;
		
	  case LC_FACS_NOTIFY_MOD:
		break;
		
	  case LC_FACS_NOTIFY_FREE:
		break;
		
	  case LC_FACS_CHG_STATUS:
		FacsReq -> FacsStatus = LcChgStatus;
		/* stored here */
		FacsReq -> Id = FacsReq -> GroupId;
		FacsReq -> GroupId = 0;
		break;
		
	  default:
		sprintf(ListErrMsg, "illegal FACS request %d", ToDo);
		lc_errmsg(ListErrMsg, LC_NOSYSERR);
		LcLocalResult = LC_FACS_REQ_ERR;
		LcError = 1;
	}
	return(LcError);
}

/******************** free a file in FACS ***************************/

static int lc_facs_free(LC_FACS_REC *FacsReq, char *Ptr, off_t *ListSize,
				 int *NotifyIfFree)
{
	/* Ptr = char ptr to entire list */
	/* ListSize = size of list in bytes */
	LC_FACS_REC *Facs;
	int FileNotLocked;
	int Sz;

	FileNotLocked = 1;
	*NotifyIfFree = 0;
	Facs = (LC_FACS_REC *)Ptr;
	for(Sz = 0; Sz < *ListSize; Sz += sizeof(LC_FACS_REC), Facs++)
	{
		if(strncmp(Facs -> UserHostName, FacsReq -> UserHostName,
				   PMAXHOSTNAMELEN) != 0)
			continue;
		if(strncmp(Facs -> FilePath, FacsReq -> FilePath, LC_MAXPATH) == 0)
		{
			/* found the file, if flag is set */
			if(Facs -> FacsStatus & LC_NOTIFY_FREE)
			{
				(*NotifyIfFree)++;
				continue;
			}
			/* file not locked */
			if((Facs -> FacsStatus != LC_LOCKED_ABS) &&
			   (Facs -> FacsStatus != LC_LOCKED_SOFT))
				continue;
			/* same pid */
			if(Facs -> UserPid != FacsReq -> UserPid)
				continue;
			memcpy((char*)Facs, (char*)(Facs + 1),
				   *ListSize - (Sz + sizeof(LC_FACS_REC)));
			*ListSize -= sizeof(LC_FACS_REC);
			FileNotLocked = 0;
			break;
		}						/* end if filepaths same */
	}							/* end for(all facs reqs */
	return(FileNotLocked);
}

/*********** scan facs file for abs locks per pid or group **********/

/*ARGSUSED*/
int lc_facs_abs_lock_check ( LC_FACS_REC *FacsReq, char *Ptr,
							 off_t *ListSize, int *NotifyIfFree, int ToDo)
{
	LC_FACS_REC *Facs;
	int Sz;

	if(ToDo != LC_FACS_FREE_PID && ToDo != LC_FACS_FREE_GROUP_ID)
		return(1);
	for(Sz = 0, Facs = (LC_FACS_REC *)Ptr; Sz < *ListSize;
		Sz += sizeof(LC_FACS_REC),Facs++)
	{
		if(strncmp(Facs -> UserHostName, FacsReq -> UserHostName,
				   PMAXHOSTNAMELEN) != 0)
			continue;			/* wrong tree */
		if(Facs -> UserPid != FacsReq -> UserPid)
			continue;			/* not my lock */
		if(Facs -> FacsStatus != LC_LOCKED_ABS) 
			continue;			/* not abs lock */
		if(ToDo == LC_FACS_FREE_PID)
			return(1);			/* abs lock for my pid */
		if(ToDo == LC_FACS_FREE_GROUP_ID && 
		   Facs -> GroupId == FacsReq -> GroupId)
			return(1);			/* abs lock for my group */
	}							/* end for all facs recs */
	return(0);
}
/************** free a file by pid, id, or group id *****************/

/*ARGSUSED*/
static int lc_facs_free_by_val( LC_FACS_REC *FacsReq, char *Ptr,
						 off_t *ListSize, int *NotifyIfFree, int ToDo)
{
	LC_FACS_REC *Facs;
	int BadReturn;
	int GoodReturn;
	char ListErrMsg[LC_MAXMSG];
	char MsgPtr[32];
	off_t SaveListSize;
	int SqzList;
	int Sz;

	memset(MsgPtr, 0, sizeof(MsgPtr));
	SaveListSize = *ListSize;
	switch(ToDo)
	{
	  case LC_FACS_FREE_ID:
		sprintf(MsgPtr, "Id %d", FacsReq -> Id);
		BadReturn  = LC_ID_NOT_IN_LIST;
		GoodReturn = LC_ID_FREED;
		break;
	  case LC_FACS_FREE_GROUP_ID:
		sprintf(MsgPtr, "GroupId %d", FacsReq -> GroupId);
		BadReturn = LC_GROUP_ID_NOT_IN_LIST;
		GoodReturn = LC_GROUP_FREED;
		break;
	  case LC_FACS_FREE_PID:
		sprintf(MsgPtr, "Pid %d", FacsReq -> UserPid);
		BadReturn = LC_PID_NOT_IN_LIST;
		GoodReturn = LC_PID_FREED;
		break;
	  default:
		sprintf(ListErrMsg, "illegal free request %d", ToDo);
		lc_errmsg(ListErrMsg, LC_NOSYSERR);
		LcResult = LC_FACS_REQ_ERR;
		return(1);
	}
	Facs = (LC_FACS_REC *)Ptr;
	SqzList = 0;				/* init for 1st loop iteration */
	Sz = 0;
	while(Sz < *ListSize)
	{
		if(strncmp(Facs -> UserHostName, FacsReq -> UserHostName,
				   PMAXHOSTNAMELEN) != 0)
		{
			Sz += sizeof(LC_FACS_REC);
			Facs++;
			continue;
		}
		if(Facs -> UserPid != FacsReq -> UserPid)
		{
			Sz += sizeof(LC_FACS_REC);
			Facs++;
			continue;
		}
		if((Facs -> FacsStatus != LC_LOCKED_ABS) &&
		   (Facs -> FacsStatus != LC_LOCKED_SOFT))
		{
			Sz += sizeof(LC_FACS_REC);
			Facs++;
			continue;
		}
		if(ToDo == LC_FACS_FREE_PID)
			SqzList = 1;
		else if(ToDo == LC_FACS_FREE_ID)
		{
			if(Facs -> Id == FacsReq -> Id)
				SqzList = 1;
		}
		else if(ToDo == LC_FACS_FREE_GROUP_ID)
		{
			if(Facs -> GroupId == FacsReq -> GroupId)
				SqzList = 1;
		}
		if(SqzList)
		{
			SqzList = 0;
			memcpy((char*)Facs, (char*)(Facs + 1),
				   *ListSize - (Sz + sizeof(LC_FACS_REC)));
			*ListSize -= sizeof(LC_FACS_REC);
		}
		else
		{
			Sz += sizeof(LC_FACS_REC);
			Facs++;
		}
	}							/* end for all facs recs */
	if(SaveListSize == *ListSize)
	{
		sprintf(ListErrMsg, "%s has no files in facs", MsgPtr);
		lc_errmsg(ListErrMsg, LC_NOSYSERR);
		LcResult = BadReturn;
		return(1);
	}
	LcResult = GoodReturn;
	return(0);
}

/****************** process a FACS lock request *********************/

static int lc_facs_lock(LC_FACS_REC *FacsReq, char *Ptr, off_t ListSize)
{
	
	/* Ptr = ptr to the whole .facs file */
	/* ListSize = size of Ptr in bytes	*/
	LC_FACS_REC *Facs;
	int FileAlreadyLocked;
	int Sz;
	FileAlreadyLocked = 0;
	Facs = (LC_FACS_REC *)Ptr;
	for(Sz = 0; Sz < ListSize; Sz += sizeof(LC_FACS_REC), Facs++)
	{
		if(strncmp(Facs -> FilePath, FacsReq -> FilePath, LC_MAXPATH))
			continue;
		
		if(strncmp(Facs -> UserHostName, FacsReq -> UserHostName,
				   PMAXHOSTNAMELEN))
		{
			FileAlreadyLocked |= 8;	/* by other */
			strncpy(LcLockedBy, Facs -> UserName, LOGNAME_MAX);
			strncpy(LcLockedOn,Facs -> UserHostName,PMAXHOSTNAMELEN);
			LcLockedPid = Facs -> UserPid;
			break;
		}
		
		if(Facs -> FacsStatus == LC_LOCKED_ABS)
		{
			FileAlreadyLocked |= 1;
			strncpy(LcLockedBy, Facs -> UserName, LOGNAME_MAX);
			strncpy(LcLockedOn, Facs -> UserHostName,PMAXHOSTNAMELEN);
			LcLockedPid = Facs -> UserPid;
			break;
		}
		else if(Facs -> FacsStatus == LC_LOCKED_SOFT)
		{
			if(Facs -> UserPid == FacsReq -> UserPid)
				FileAlreadyLocked |= 2;	/* by self */
			else if(Facs -> UserPid == FacsReq -> ForPid)
				FileAlreadyLocked |= 4;	/* by sibling */
			else
			{
				FileAlreadyLocked |= 8;	/* by other */
				strncpy(LcLockedBy, Facs -> UserName, LOGNAME_MAX);
				strncpy(LcLockedOn,	Facs -> UserHostName,PMAXHOSTNAMELEN);
				LcLockedPid = Facs -> UserPid;
				break;
			}
		}
	}							/* end for(all facs reqs) */
	return(FileAlreadyLocked);
}

/*************** search a facs list for a status ********************/

static LC_FACS_REC *lc_facs_search(LC_FACS_REC *FacsReq, char *Ptr,
							off_t ListSize, int SearchFor)
{
	/* Ptr = char ptr to entire list */
	/* ListSize = size fo list in bytes	*/
	LC_FACS_REC *Facs, *FacsRet = 0, *FacsRem;
	int FoundRec;
	int Sz;

	FoundRec = 0;
	Facs = (LC_FACS_REC *)Ptr;
	FacsRem = (LC_FACS_REC *)0;
	for(Sz = 0; Sz < ListSize; Sz += sizeof(LC_FACS_REC), Facs++)
	{
		if(strncmp(Facs -> UserHostName, FacsReq -> UserHostName,
				   PMAXHOSTNAMELEN) != 0)
			continue;
		if(strncmp(Facs -> FilePath, FacsReq -> FilePath,
				   LC_MAXPATH) != 0)
			continue;
		if((SearchFor == LC_LOCKED_ABS) && 
		   (Facs -> FacsStatus == LC_LOCKED_ABS) && Facs -> ForPid)
		{
			FacsRem = Facs;
			FoundRec = 1;
			continue;
		}
		FoundRec = 1;
		break;
	}
	if(FoundRec)
	{
		if(FacsRem)
			Facs = FacsRem;
		if(Facs -> FacsStatus == SearchFor)
			FacsRet = Facs;
	}
	if(!FoundRec)
		FacsRet = (LC_FACS_REC *)0;
	return(FacsRet);
}

/****************** change the status of a file *********************/

static int lc_facs_status_change(LC_FACS_REC *FacsReq, char *Ptr,
						  off_t ListSize)
{
	/* Ptr = char ptr to entire list */
	/* ListSize = size of list in bytes	*/
	LC_FACS_REC *Facs, *Facs1;
	int FoundRec;
	char ListErrMsg[LC_MAXMSG];
	int StatusNotChanged;
	int Sz;

	FoundRec = 0;
	StatusNotChanged = 1;
	LcResult = LC_STATUS_NOT_CHANGED;
	Facs = (LC_FACS_REC *)Ptr;
	for(Sz = 0; Sz < ListSize; Sz += sizeof(LC_FACS_REC), Facs++)
	{
		if(strncmp(Facs -> UserHostName, FacsReq -> UserHostName,
				   PMAXHOSTNAMELEN) != 0)
			continue;
		if(Facs -> UserPid != FacsReq -> UserPid)
			continue;
		if(Facs -> Id != FacsReq -> Id)
			continue;
		FoundRec = 1;
		break;					/* found the record */
	}							/* end for entire list */
	if(FoundRec)
	{
		switch(FacsReq -> FacsStatus)
		{
		  case LC_FACS_ABS_NOTIFY_LOCK:
		  case LC_FACS_ABS_ERROR_LOCK:
			/* needs work */
			/* case LC_FACS_SOFT_SINGLE_LOCK: */
			/* case LC_FACS_SOFT_MULTI_LOCK: */
			if((Facs1 = lc_facs_search(Facs, Ptr, ListSize,
									   LC_LOCKED_ABS)) != NULL)
			{
				LcResult = LC_FILE_ALREADY_LOCKED;
				strncpy(LcLockedBy, Facs1 -> UserName, LOGNAME_MAX);
				strncpy(LcLockedOn, Facs1 -> UserHostName, PMAXHOSTNAMELEN);
				if(Facs1 -> ForPid)
				{
					LcLockedPid = Facs1 -> ForPid;
					sprintf(ListErrMsg, 
							"%s already locked by %s on %s for pid %d",
							Facs1 -> FilePath, LcLockedBy, LcLockedOn,
							LcLockedPid);
					lc_errmsg(ListErrMsg, LC_NOSYSERR);
				}
				else
				{
					LcLockedPid = Facs1 -> UserPid;
					sprintf(ListErrMsg, "%s already locked by %s on %s pid %d",
							Facs1 -> FilePath, LcLockedBy, LcLockedOn,
							LcLockedPid);
					lc_errmsg(ListErrMsg, LC_NOSYSERR);
				}
			}
			else
			{
				Facs -> FacsStatus = LC_LOCKED_ABS;
				StatusNotChanged = 0;
			}
			break;
			
		  case LC_FACS_INTEREST:
			if(Facs -> FacsStatus != LC_INTEREST)
			{
				Facs -> FacsStatus = LC_INTEREST;
				StatusNotChanged = 0;
			}
			break;
			
		  case LC_FACS_NOTIFY_MOD:
			if(Facs -> FacsStatus != LC_NOTIFY_MOD)
			{
				Facs -> FacsStatus = LC_NOTIFY_MOD;
				StatusNotChanged = 0;
			}
			break;
			
		  case LC_FACS_NOTIFY_FREE:
			if(Facs -> FacsStatus != LC_NOTIFY_FREE)
			{
				Facs -> FacsStatus = LC_NOTIFY_FREE;
				StatusNotChanged = 0;
			}
			break;
			
		  case LC_FACS_FREE:
		  case LC_FACS_FREE_ID:
		  case LC_FACS_FREE_GROUP_ID:
		  case LC_FACS_FREE_PID:
		  case LC_FACS_CHG_STATUS:
		  default:
			LcResult = LC_ILLEGAL_STATUS_REQ;
			break;
		}						/* end switch(FacsStatus) */
	}
	else
		LcResult = LC_ID_NOT_IN_LIST;
	if(!StatusNotChanged)
		LcResult = LC_STATUS_CHANGED;
	return(StatusNotChanged);
}

/******************* return the size of a list **********************/

static int lc_get_listsize(int List, int *ListSize)
{
	int Error = 0;
	char ListErrMsg[LC_MAXMSG];

	switch(List)
	{
	  case LC_LST_FACS:
		*ListSize = sizeof(LC_FACS_REC);
		break;
		
	  case LC_LST_MEMBERS:
		*ListSize = sizeof(LC_MEMB_REC);
		break;
		
	  case LC_LST_LAYOUTS:
	  case LC_LST_MP_SETUP:
		*ListSize = sizeof(LC_LAY_REC);
		break;
		
	  case LC_LST_OUTPUT:
		*ListSize = sizeof(LC_OUT_REC);
		break;
		
	  default:
		sprintf(ListErrMsg, "List type %d is undefined", List);
		lc_errmsg(ListErrMsg, LC_NOSYSERR);
		LcResult = LC_LIST_ERR;
		Error = 1;
	}
	return(Error);
}

/******************* process a layout list **************************/

#ifdef NOT_USED_BY_ANYONE
int lc_layout(off_t *Size, char *Ptr, int ToDo, void *List)
/* Size of layout list, off_t defined in types.h */
/* start addr of list defined in list_control.h */
/* rec to process */
{
	int found = 0;
	char ListErrMsg[LC_MAXMSG];
	int Sz;
	LC_LAY_REC *LayRec, *LayList;

	LayRec = (LC_LAY_REC *)List;
	LayList = (LC_LAY_REC *)Ptr;
	for(Sz = 0; Sz < (*Size); Sz += sizeof(LC_LAY_REC), LayList++)
	{
		if(strncmp(LayRec -> filename, LayList -> filename, LC_MAXPATH) == 0)
		{
			found++;
			break;
		}
	}
	switch(ToDo)
	{
	  case LC_REMOVE:
		if(found)
		{
			memcpy((char*)LayList, (char*)(LayList + 1),
				   *Size - (Sz + sizeof(LC_LAY_REC)));
			*Size -= sizeof(LC_LAY_REC);
			return(1);
		}
		else
		{
			sprintf(ListErrMsg, "did not find %s in %s",
					LayRec -> filename, LstName);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			return(0);
		}
		
	  case LC_ADD:
		if(found)
		{
			sprintf(ListErrMsg, "%s already exists in %s",
					LstName, LayRec -> filename);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			return(1);
		}
		else
			return(0);
		
	  default:
		break;
	}							/* end switch(ToDo) */
	return(0);
}
#endif 

/**************** try to create the .lock file ***********************/

static int lc_lock()
{
	int Error;
	int Fd;
	char *FullPath;
	char ListErrMsg[LC_MAXMSG];
	int tries;

	Fd = -1;
	Error = 0;
	tries = 0;
	/* add rest of path */
	FullPath = p_path (TreeName, Dir, SubDir, LockName);
	for (;;)
	{
		errno = 0;
		Fd = open (FullPath, O_WRONLY | O_CREAT | O_EXCL, 0666);
		if (Fd != -1 || errno != EEXIST)
			break;
		if (++tries > 1 && !(tries % 5))
		{
			sprintf (ListErrMsg, "trying to create `%s' .lock file", TreeName);
			lc_errmsg (ListErrMsg, errno);
		}

		sleep (LC_NAPTIME);
	}

	if (Fd == -1)
	{
		sprintf (ListErrMsg, "failed to create `%s' .lock file", TreeName);
		lc_errmsg (ListErrMsg, errno);
		Error = 1;
		LcResult = LC_NO_LOCK;
	}
	else if (close (Fd) == -1)
	{
		sprintf (ListErrMsg, "failed to close `%s' .lock file", TreeName);
		lc_errmsg (ListErrMsg, errno);
		Error = 1;
		LcResult = LC_NO_LOCK;
	}

	return (Error);
}

/******************* process a member list **************************/

#ifdef NOT_USED_BY_ANYONE
int lc_member(off_t *Size, char *Ptr, int ToDo, void *List)
/* Size of memb list, off_t defined in types.h */
/* start addr of list defined in list_control.h */
/* rec to process */
{
	int found = 0;
	char ListErrMsg[LC_MAXMSG];
	int Sz;
	LC_MEMB_REC *MembRec, *MembList;

	MembRec = (LC_MEMB_REC *)List;
	MembList = (LC_MEMB_REC *)Ptr;
	for(Sz = 0; Sz < (*Size); Sz += sizeof(LC_MEMB_REC), MembList++)
	{
		if(strncmp(MembRec -> filename, MembList -> filename, AX_NAME) == 0)
		{
			found++;
			break;
		}
	}
	switch(ToDo)
	{
	  case LC_REMOVE:
		if(found)
		{
			memcpy((char*)MembList, (char*)(MembList + 1),
				   *Size - (Sz + sizeof(LC_MEMB_REC)));
			*Size -= sizeof(LC_MEMB_REC);
			return(1);
		}
		else
		{
			sprintf(ListErrMsg, "did not find %s in %s", 
					MembRec -> filename, LstName);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			return(0);
		}
		
	  case LC_ADD:
		if(found)
		{
			sprintf(ListErrMsg, "%s already exists in %s",
					LstName, MembRec -> filename);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			return(1);
		}
		else
			return(0);
		
	  default:
		break;
	}							/* end switch(ToDo) */
	return(0);
}

/*************** place holder in (*lc_funcs[])() ********************/

static int lc_noop()
{
	return(0);
}
#endif 

/****************** notify users of a freed file ********************/

/*ARGSUSED*/
static void lc_notify_free(LC_FACS_REC *FacsReq, char *Ptr,
						   off_t ListSize)
{
	/* future
	   1 - go through list notify user of freed file
	   2 - remove from list
	   */
	return;
}

/******************* process an output list *************************/

#ifdef NOT_USED_BY_ANYONE
int lc_output(off_t *Size, char *Ptr, int ToDo, void *List)
/* Size of output list, off_t defined in types.h */
/* start addr of list defined in list_control.h */
/* rec to process */
{
	int found = 0;
	char ListErrMsg[LC_MAXMSG];
	int Sz;
	LC_OUT_REC *OutRec, *OutList;

	OutRec = (LC_OUT_REC *)List;
	OutList = (LC_OUT_REC *)Ptr;
	for(Sz = 0; Sz < (*Size); Sz += sizeof(LC_OUT_REC), OutList++)
	{
		if(strncmp(OutRec -> OutputPath, OutList -> OutputPath,
				   LC_MAXPATH) == 0)
		{
			found++;
			break;
		}
	}
	switch(ToDo)
	{
	  case LC_REMOVE:
		if(found)
		{
			memcpy((char*)OutList, (char*)(OutList + 1),
				   *Size - (Sz + sizeof(LC_OUT_REC)));
			*Size -= sizeof(LC_OUT_REC);
			return(1);
		}
		else
		{
			sprintf(ListErrMsg, "did not find %s in %s",
					OutRec -> OutputPath, LstName);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			return(0);
		}

	  case LC_ADD:
		if(found)
		{
			sprintf(ListErrMsg, "%s already exists in %s",
					LstName, OutRec -> OutputPath);
			lc_errmsg(ListErrMsg, LC_NOSYSERR);
			return(1);
		}
		else
			return(0);
		
	  default:
		break;
	}							/* end switch(ToDo) */
	return(0);
}
#endif 

/*************** read entire list into allocated memory *************/

static int lc_read_list(char **Ptr, off_t *ListSize, Pfd *Fd, int CreateList, int ListType)
{
	struct stat stats;
	int Error;
	char ListErrMsg[LC_MAXMSG];
	Pfd LocalFd;

	*Fd = 0;
	if (*Ptr)
	{
		p_free(*Ptr);
		*Ptr = '\0';
	}
	if (p_stat(TreeName, Dir, SubDir, LstName, &stats) != 0 &&
		p_stat(TreeName, Dir, SubDir, OldLstName, &stats) == 0)
	{
		Pfd old;
		Pfd new;
		char old_line[128];
		char new_line[128];

		/*
		 * old revision of layout or member list exists,
		 * do auto convert to new style,
		 * ie: add {0 0 } to start of each line for flag and type
		 */
		p_info(PI_INFO, "converting list file\n");
		old = p_open(TreeName, Dir, SubDir, OldLstName, "r");
		new = p_open(TreeName, Dir, SubDir, LstName, "w");
		if(old != P_ERROR && new != P_ERROR)
		{
			while(p_fgets(old_line,128,old))
			{
				sprintf(new_line,"0 0 %s",old_line);
				p_fputs(new_line,new);
			}
			p_info(PI_INFO, "conversion complete\n");
		}
		else
			p_info(PI_ELOG, "conversion failed\n");
		p_close(old);
		p_close(new);
	}
	Error = 0;
	if(p_stat(TreeName, Dir, SubDir, LstName, &stats))
	{
		/* file not exist, and not called by lc_return_list, then create list*/
		if((errno == ENOENT) && CreateList)
		{						/* create the list file */
			if((LocalFd = p_open(TreeName, Dir, SubDir, LstName, "w"))
			   == P_ERROR)
			{
				sprintf(ListErrMsg, "failed to create %s", LstName);
				lc_errmsg(ListErrMsg, errno);
				Error = 1;
				LcResult = errno;
				goto rdlst;
			}
			p_close(LocalFd);
			stats.st_size = 0;
		}
		else
		{
			sprintf(ListErrMsg, "stat failed on %s", LstName);
			lc_errmsg(ListErrMsg, errno);
			Error = 1;
			LcResult = errno;
			goto rdlst;
		}
	}
	if(((*Fd) = p_open(TreeName, Dir, SubDir, LstName, "r+")) == P_ERROR)
	{
		sprintf(ListErrMsg, "failed to open %s", LstName);
		lc_errmsg(ListErrMsg, errno);
		Error = 1;
		LcResult = errno;
		goto rdlst;
	}
	if(stats.st_size == 0)
	{
		*ListSize = 0;
		goto rdlst;
	}
	if(((*Ptr) = p_alloc(stats.st_size+1)) == NULL)
	{
		sprintf(ListErrMsg, "failed to alloc for %s", LstName);
		lc_errmsg(ListErrMsg, errno);
		Error = 1;
		LcResult = errno;
		goto rdlst;
	}
	if((p_read(*Ptr, stats.st_size, 1, *Fd, 0, 
			((ListType==LC_LST_FACS) ?  SW_FACS : SW_NOSWAP))) < 1)
	{
		sprintf(ListErrMsg, "failed to read %s", LstName);
		lc_errmsg(ListErrMsg, errno);
		Error = 1;
		LcResult = errno;
		goto rdlst;
	}
/*	*(*Ptr + stats.st_size) = 0;	*//* Null-term rec in case it's scanned  */
	*ListSize = stats.st_size;
  rdlst:	;
	return(Error);
}

/**************** rewrite a modified list file to disk ***************/
/*****  Note: This is ONLY called for the .facs?.list file, no other *.list files. *****/
static int lc_rewrite_list(char *Ptr, off_t ListSize)
{
	int Error;
	char ListErrMsg[LC_MAXMSG];
	char TmpFile[LC_MAXPATH];
	Pfd ToFd;

	Error = 0;
	if(ListSize <= 0)
		return(Error);
	sprintf(TmpFile, "%s.tmp", LstName);
	if((ToFd = p_open(TreeName, Dir, SubDir, TmpFile, "w+")) == P_ERROR)
	{
		sprintf(ListErrMsg, "failed to open %s", TmpFile);
		lc_errmsg(ListErrMsg, errno);
		LcResult = errno;
		Error = 1;
		goto rewlst;
	}
	if((p_write(Ptr, ListSize, 1, ToFd, 0, SW_FACS)) < 1)
	{
		sprintf(ListErrMsg, "failed to write to %s", TmpFile);
		lc_errmsg(ListErrMsg, errno);
		LcResult = errno;
		Error = 1;
		p_close(ToFd);
		goto rewlst;
	}
	p_close(ToFd);
	if(p_unlink(TreeName, Dir, SubDir, LstName))
	{
		sprintf(ListErrMsg, "failed to delete %s", LstName);
		lc_errmsg(ListErrMsg, errno);
		LcResult = errno;
		Error = 1;
		goto rewlst;
	}
	if(p_rename(TreeName, Dir, SubDir, TmpFile,
				Dir, SubDir, LstName))
	{
		sprintf(ListErrMsg, "failed to rename %s to %s",
				TmpFile, LstName);
		lc_errmsg(ListErrMsg, errno);
		sprintf(ListErrMsg, "%s exists as %s", LstName, TmpFile);
		lc_errmsg(ListErrMsg, LC_NOSYSERR);
		LcResult = errno;
		Error = 1;
	}
  rewlst:	;
	return(Error);
}

/********************* set the return value ***************************/

static void lc_set_result()
{
	LcId++;
	if(LcId > LC_MAX_ID)
		LcId = 1;
	return;
}

/******************** delete the .lock file **************************/

static int lc_unlock()
{
	char ListErrMsg[LC_MAXMSG];

	if(p_unlink(TreeName, Dir, SubDir, LockName))
	{
		sprintf(ListErrMsg, "failed to delete .lock %s", LockName);
		lc_errmsg(ListErrMsg, errno);
		LcResult = errno;
		LcLocalResult = 1;
	}
	return(0);
}

/********************* create a list file ****************************/

int lc_create_list_file(char *treename, int dir, char *subdir, int ListType)
{
	Pfd LocalFd;

	/* Create LstName */
	build_paths(ListType);
	if((LocalFd = p_open(treename, dir, subdir, LstName, "w")) != P_ERROR)
	{
		p_close(LocalFd);
		return (P_SUCCESS);
	}
	else
		return(P_ERROR);
}
