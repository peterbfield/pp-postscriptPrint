#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#if OS_sunos
#define PORTMAP
#endif
#include "noproto.h"
#include "socket.h"
#include "p_lib.h"
#include "lpm.h"
#include "lpm_server.h"
#include <rpc/pmap_clnt.h>

#define getmypid()		(MyPid? MyPid : (MyPid = getpid()))
#define setstring(dst,src) \
		if ((src) && *(src)) \
		{ \
			if (strlen (src) > sizeof (dst) - 1) \
				return 0; \
			strcpy ((dst), (src)); \
		} \
		else \
			*(dst) = 0

#define lpm_call_int(type) \
		{ \
		int rc = 0; \
		return lpm_call ((type), (char*) &arg, (char*) &rc)? rc:0; \
		}


#define lpm_call_agg(type,aggregate) \
		{ \
		static aggregate rc; \
		memset ((char *)&rc, 0, sizeof rc); \
		return lpm_call ((type),  (char*) &arg, (char*) &rc)? &rc : NULL; \
		}

typedef struct Parm { bool_t (*ixdr) (), (*oxdr) (); } Parm;

char lpm_errmsg[LPM_MAXMESG];

static Parm LpmParm[] =
{
	{ 0,			0 },
	{ xdr_int,		xdr_int },			/* lpm_init */
	{ xdr_que,		xdr_int },			/* lpm_que */
	{ xdr_done,		xdr_int },			/* lpm_done */
	{ xdr_int,		xdr_xmesg },		/* lpm_getmsg */
	{ xdr_msg,		xdr_int },			/* lpm_sendmsg */
	{ xdr_ckpid,	xdr_int },			/* lpm_checkpid */
	{ xdr_qry,		xdr_u_int },		/* lpm_query */
	{ xdr_int,		xdr_int },			/* lpm_bye */
	{ xdr_pset,		xdr_int },			/* lpm_preset */
	{ xdr_int,		xdr_u_long },		/* lpm_verify */
	{ xdr_xhost,	xdr_pidq },			/* lpm_getpidq */
	{ xdr_xhost,	xdr_reqq },			/* lpm_getreqq */
	{ xdr_xhost,	xdr_msgq },			/* lpm_getmsgq */
	{ xdr_xhost,	xdr_hostq },		/* lpm_gethostq */
	{ xdr_xhost,	xdr_keys },			/* lpm_getkeys */
};

static CLIENT *lpm_cl;
xname MyHname;
ulong MyHaddr;
static ulong lpm_nom;
static int MyPid = 0;
static enum clnt_stat LpmRpcError = RPC_SUCCESS;


#if 0
void dbug( char *msg, ...)
{
	char buf1[2048];
	va_list ap;
	
	va_start( ap, msg);
	vsprintf( buf1, msg, ap);
	va_end( ap);
	
	fprintf( stderr, "lpmclnt: %s\n", buf1);
}
#endif

static int lpm_clnt_create (void)
{
	struct sockaddr_in saddr;
#if 0
	struct timeval tv;
#endif

	if (!*MyHname)
		gethostname (MyHname, sizeof MyHname - 1);

	if (lpm_cl)
		clnt_destroy (lpm_cl);

	lpm_cl = clnt_create (MyHname, LPMPROG, LPMVERS, "udp");
	if (!lpm_cl)
	{
		strcpy (lpm_errmsg, clnt_spcreateerror (MyHname));
		return 0;
	}

#if 0
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	clnt_control (lpm_cl, CLSET_TIMEOUT, (char *)&tv);
	clnt_control (lpm_cl, CLSET_RETRY_TIMEOUT, (char *)&tv);
#endif
	clnt_control (lpm_cl, CLGET_SERVER_ADDR, (char *)&saddr);
	MyHaddr = ntohl (saddr.sin_addr.s_addr);

	errno = 0;
	return 1;
}

static int lpm_call (int type, char *ip, char *op)
{
	struct timeval tv;
	Parm *pm;

	if (type < LPMINIT || type > LPMMAXPROC)
		return 0;

	pm = &LpmParm[type];

	if (!lpm_cl)
		if (!lpm_clnt_create ())
			return 0;
	
	tv.tv_sec = 25;
	tv.tv_usec = 0;

	LpmRpcError = clnt_call (lpm_cl, type, (xdrproc_t)pm->ixdr, ip, (xdrproc_t)pm->oxdr, op, tv);
	if (LpmRpcError != RPC_SUCCESS)
	{

		strcpy (lpm_errmsg, clnt_sperrno (LpmRpcError));

		switch (LpmRpcError) {
		default:
			return 0;
		case RPC_CANTSEND:				/* failure in sending call */
		case RPC_CANTRECV:				/* failure in receiving result */
		case RPC_TIMEDOUT:				/* call timed out */
			break;
		}

		lpm_clnt_create ();
		if (!lpm_cl)
			return 0;

		if (clnt_call (lpm_cl, type, (xdrproc_t)pm->ixdr, ip, (xdrproc_t)pm->oxdr, op, tv) != RPC_SUCCESS)
			return 0;
	}

	return 1;
}

int lpm_init (void)
{
	int arg = getmypid ();

	lpm_call_int (LPMINIT);
}

int lpm_que (char *prog, char *msg)
{
	LpmQue arg;

	setstring (arg.name, prog);
	setstring (arg.mesg, msg);
	arg.pid = getmypid ();
	arg.pri = 5;

	lpm_call_int (LPMQUE);
}

int lpm_pque (int pri, char *prog, char *msg)
{
	LpmQue arg;

	setstring (arg.name, prog);
	setstring (arg.mesg, msg);
	arg.pid = getmypid ();
	arg.pri = pri;

	lpm_call_int (LPMQUE);
}

int lpm_done (int id, int xval)
{
	LpmDone arg;

	arg.id = id;
	arg.xval = xval;

	lpm_call_int (LPMDONE);
}

char *lpm_getmsg (void)
{
	static xmesg rc;
	int arg = getmypid ();

	*rc = 0;

	return lpm_call (LPMGETMSG, (char *)&arg, (char *)&rc) && *rc ? rc : NULL;
}

int lpm_sendmsg (char *host, int pid, char *mesg)
{
	LpmMsg arg;

	setstring (arg.mesg, mesg);
	setstring (arg.host, host);
	arg.pid = pid;

	lpm_call_int (LPMSENDMSG);
}

int lpm_checkpid (char *host, int pid)
{
	LpmCkpid arg;

	setstring (arg.host, host);
	arg.pid = pid;

	lpm_call_int (LPMCHECKPID);
}

int lpm_bye (void)
{
	int arg = getmypid ();

	lpm_call_int (LPMBYE);
}

unsigned int lpm_query (int nom, int rev)
{
	LpmQry arg;
	unsigned int rc = 0;

	arg.pid = getmypid ();
	arg.nom = nom;
	arg.rev = rev;

	if (rev == LPM_Option)
		if (!lpm_optinrange (nom))
			return 0;
	if (!lpm_call (LPMQUERY, (char *)&arg, (char *)&rc))
		return 0;
	else if (rev == LPM_Option)
		return rc;
	else if (rc)
	{
		lpm_setbit (lpm_nom, nom);
		return rc;
	}
	else
		return 0;
}

int lpm_verify (void)
{
	int arg = getmypid ();
	ulong rc = 0;

	if (!lpm_call (LPMVERIFY, (char *)&arg, (char *)&rc))
		return LpmRpcError == RPC_PROCUNAVAIL;
	if (lpm_nom != rc)
		return 0;

	return 1;
}

int lpm_preset (char *host, char *prog, int wait)
{
	LpmPset arg;

	setstring (arg.name, prog);
	setstring (arg.host, host);
	arg.wait = wait;

	lpm_call_int (LPMPRESET);
}

QUEUE *lpm_getpidq (char *host)
{
	xhost arg;

	setstring (arg, host);
	lpm_call_agg (LPMGETPIDQ, QUEUE);
}

QUEUE *lpm_getreqq (char *host)
{
	xhost arg;

	setstring (arg, host);
	lpm_call_agg (LPMGETREQQ, QUEUE);
}

QUEUE *lpm_getmsgq (char *host)
{
	xhost arg;

	setstring (arg, host);
	lpm_call_agg (LPMGETMSGQ, QUEUE);
}

QUEUE *lpm_gethostq (char *host)
{
	xhost arg;

	setstring (arg, host);
	lpm_call_agg (LPMGETHOSTQ, QUEUE);
}

LpmKeys *lpm_getkeys (char *host)
{
	xhost arg;

	setstring (arg, host);
	lpm_call_agg (LPMGETKEYS, LpmKeys);
}

/*
 * Extract return value from message returned by lpm_getmsg().
 * retval - gets the return value.
 * msg - the message returned by lpm_getmsg().
 * proc - the procedure invoked by lpm_sendmsg()/lpm_que() (i.e., "HandJ").
 * Return 1 if return value successfully extracted; else return 0.
 */
int lpm_retval (int *retval, char *msg, char *proc)
{
	char *cp;

	if (!msg || !*msg ||
		!streqn (msg, "LPMdone: RETVAL:", 16) ||
		!proc || !*proc ||
		!(cp = strchr (msg + 16, ' ')) ||
		!streqn (cp + 1, proc, strlen (proc)))
		return 0;

	*retval = atoi (msg + 16);

	return 1;
}
