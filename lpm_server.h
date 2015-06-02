#ifndef _LPM_SERVER_H
#define _LPM_SERVER_H

#include <rpc/rpc.h>
#include "que.h"

#define LPMPROG				((u_long)1107714020)
#define LPMVERS				((u_long)1)
#define LPMINIT				((u_long)1)
#define LPMQUE				((u_long)2)
#define LPMDONE				((u_long)3)
#define LPMGETMSG			((u_long)4)
#define LPMSENDMSG			((u_long)5)
#define LPMCHECKPID			((u_long)6)
#define LPMQUERY			((u_long)7)
#define LPMBYE				((u_long)8)
#define LPMPRESET			((u_long)9)
#define LPMVERIFY			((u_long)10)
#define LPMGETPIDQ			((u_long)11)
#define LPMGETREQQ			((u_long)12)
#define LPMGETMSGQ			((u_long)13)
#define LPMGETHOSTQ			((u_long)14)
#define LPMGETKEYS			((u_long)15)
#define LPMPING				((u_long)16)
#define LPMPONG				((u_long)17)
#define LPMALIVE			((u_long)18)
#define LPMMAXPROC			LPMALIVE

#define LPM_DAY				(24L*60L*60L)	/* # of seconds in a day */
#define LPM_1994			757400400L		/* Unix time for Jan 1, 1994 */

#define	lpm_setbit(a,i)		((a) |= 1<<(i))
#define	lpm_clrbit(a,i)		((a) &= ~(1<<(i)))
#define	lpm_isset(a,i)		((a) & (1<<(i)))
#define	lpm_isclr(a,i)		(((a) & (1<<(i))) == 0)

#define lpm_optinrange(opt)	(((opt) >= 0 && (opt) <= 23) || ((opt) >= 52 && (opt) <= 62))
#define	lpm_optset(n,p)		((p)->bits[(n)/32] |=  ((unsigned)1 << ((n) % 32)))
#define lpm_optclr(n,p)		((p)->bits[(n)/32] &= ~((unsigned)1 << ((n) % 32)))
#define lpm_optisset(n,p)	(((p)->bits[(n)/32] &   ((unsigned)1 << ((n) % 32))) != 0)
#define lpm_optzero(p)		memset((char*)(p), 0, sizeof(*(p)))
#define get_today()			((time (0) - LPM_1994) / LPM_DAY)
#define deadpid(pid)		(kill (pid, 0) == -1 && errno != EPERM)
#define streq(a,b)			(!strcmp((a),(b)))
#define streqn(a,b,c) 		(!memcmp((a),(b),(c)))

typedef char xmesg[LPM_MAXMESG];
typedef char xname[LPM_MAXNAME];
typedef char xhost[LPM_MAXNAME];
typedef struct KEYtype { int tcnt; int ucnt; } KEYtype;
typedef struct OPTtype { unsigned long bits[(LPM_MaxOption + (32 - 1)) / 32]; } OPTtype;
typedef struct LpmKeys { KEYtype key[LPM_MaxFeature]; OPTtype opt; int date, days; } LpmKeys;
typedef struct LpmQry { int pid; int nom; int rev; } LpmQry;
typedef struct LpmMsg { xhost host; int pid; xmesg mesg; } LpmMsg;
typedef struct LpmQue { xname name; xmesg mesg; int pid; int pri; } LpmQue;
typedef struct LpmDone { int id; int xval; } LpmDone;
typedef struct LpmPset { xhost host; xname name; int wait; } LpmPset;
typedef struct LpmCkpid { xhost host; int pid; } LpmCkpid;
typedef struct LpmPidq { QE q; int pid; int nom; } LpmPidq;
typedef struct LpmReqq { QE q; int try; int pid; int pri; struct LpmProcl *pp; xmesg req; } LpmReqq;
typedef struct LpmMsgq { QE q; int pid; xmesg msg; } LpmMsgq;
#define ProcStatReset 2
typedef struct LpmProcl { int fd; int pid; int stat; int idx; LpmReqq *rp; xname name; } LpmProcl;
#if 0
typedef struct LpmHostq { QE q; int alive; ulong haddr; LpmKeys keys; CLIENT *cl; xhost host; } LpmHostq;
#else
typedef struct LpmHostq
{
	QE q;
	int flags;
#define LPMalive		1
#define LPMemergency	2
	ulong haddr;
	ulong cpuid;
	ulong eaddr;
	LpmKeys keys;
	CLIENT *cl;
	xhost host;
	struct sockaddr_in sin;
}	LpmHostq;
#endif
extern QUEUE *lpm_getpidq (char *host);
extern QUEUE *lpm_getreqq (char *host);
extern QUEUE *lpm_getmsgq (char *host);
extern QUEUE *lpm_gethostq (char *host);
extern LpmKeys *lpm_getkeys (char *host);

extern bool_t xdr_qry();
extern bool_t xdr_xmesg();
extern bool_t xdr_xname();
extern bool_t xdr_xhost();
extern bool_t xdr_host();
extern bool_t xdr_msg();
extern bool_t xdr_que();
extern bool_t xdr_done();
extern bool_t xdr_pset();
extern bool_t xdr_keys();
extern bool_t xdr_pidq();
extern bool_t xdr_ckpid();
extern bool_t xdr_reqq();
extern bool_t xdr_msgq();
extern bool_t xdr_hostq();

extern char* lpm_itoa (unsigned long value, char *bufend, int len);
extern unsigned long lpm_atoi (const char *s, int count);
extern void lpm_encode (char *line, int len);
extern int lpm_decode (FILE* fp, char *buf);
#endif
