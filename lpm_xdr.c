#include <stdio.h>
#include <stdarg.h>
#include "p_lib.h"
#include "lpm.h"
#include "lpm_server.h"

#define XDR_XMESG(xdr,ap)		xdr_string (xdr, (ap), LPM_MAXMESG)
#define XDR_XNAME(xdr,ap)		xdr_string (xdr, (ap), LPM_MAXNAME)
#define XDR_XHOST(xdr,ap)		xdr_string (xdr, (ap), LPM_MAXNAME)


extern void dbug( char *msg, ...);

bool_t xdr_xmesg (XDR *xdr, xmesg *ap)
{
	char *mp = (char*) ap;
	return XDR_XMESG (xdr, &mp);
}

bool_t xdr_xname (XDR *xdr, xname *ap)
{
	char *np = (char*) ap;
	return XDR_XNAME (xdr, &np);
}

bool_t xdr_xhost (XDR *xdr, xhost *ap)
{
	char *hp = (char*) ap;
	return XDR_XHOST (xdr, &hp);
}


bool_t xdr_host (XDR *xdr, LpmHostq *pp)
{
	char *hp = pp->host;

	return
		xdr_int (xdr, &pp->flags) &&
		xdr_u_long (xdr, &pp->haddr) &&
		xdr_u_long (xdr, &pp->cpuid) &&
		xdr_u_long (xdr, &pp->eaddr) &&
		xdr_keys (xdr, &pp->keys) &&
		XDR_XHOST (xdr, &hp);
}

bool_t xdr_keys (XDR *xdr, LpmKeys *ap)
{
	int j;

	for (j = 0; j < LPM_MaxFeature; j++)
		if (!xdr_int (xdr, &ap->key[j].tcnt) ||
			!xdr_int (xdr, &ap->key[j].ucnt))
			return FALSE;

	for (j = 0; j < sizeof ap->opt.bits / sizeof ap->opt.bits[0]; j++)
		if (!xdr_u_long (xdr, &ap->opt.bits[j]))
			return FALSE;
		
	return
		xdr_int (xdr, &ap->date) &&
		xdr_int (xdr, &ap->days);
}

bool_t xdr_qry (XDR *xdr, LpmQry *ap)
{
	return
		xdr_int (xdr, &ap->pid) &&
		xdr_int (xdr, &ap->nom) &&
		xdr_int (xdr, &ap->rev);
}

bool_t xdr_msg (XDR *xdr, LpmMsg *ap)
{
	char *hp = ap->host;
	char *mp = ap->mesg;
	return
		XDR_XHOST (xdr, &hp) &&
		xdr_int (xdr, &ap->pid) &&
		XDR_XMESG (xdr, &mp);
}

bool_t xdr_ckpid (XDR *xdr, LpmCkpid *ap)
{
	char *hp = ap->host;
	return
		XDR_XHOST (xdr, &hp) &&
		xdr_int (xdr, &ap->pid);
}

bool_t xdr_que (XDR *xdr, LpmQue *ap)
{
	char *mp = ap->mesg;
	char *np = ap->name;
	return
		XDR_XNAME (xdr, &np) &&
		XDR_XMESG (xdr, &mp) &&
		xdr_int (xdr, &ap->pid) &&
		xdr_int (xdr, &ap->pri);
}

bool_t xdr_done (XDR *xdr, LpmDone *ap)
{
	return
		xdr_int (xdr, &ap->id) &&
		xdr_int (xdr, &ap->xval);
}

bool_t xdr_pset (XDR *xdr, LpmPset *ap)
{
	char *np = ap->name;
	return
		XDR_XNAME (xdr, &np) &&
		xdr_int (xdr, &ap->wait);
}

bool_t xdr_pidq (XDR *xdr, QUEUE *ap)
{
	LpmPidq *pp;

	switch (xdr->x_op) {
	default:
		return FALSE;

	case XDR_ENCODE:
		pp = (LpmPidq*)ap->head;
		for (;;)
		{
			bool_t more = pp != NULL;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			if (!xdr_int (xdr, &pp->pid) ||
				!xdr_int (xdr, &pp->nom))
					return FALSE;
			pp = (LpmPidq*)pp->q.next;
		}
		break;

	case XDR_DECODE:
		for (;;)
		{
			bool_t more;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			pp = (LpmPidq*) p_alloc (sizeof (LpmPidq));
			if (!pp)
				return FALSE;

			QinsertTail (ap, (QE*)pp);

			if (!xdr_int (xdr, &pp->pid) ||
				!xdr_int (xdr, &pp->nom))
					return FALSE;
		}
		break;
	}

	return TRUE;
}

bool_t xdr_reqq (XDR *xdr, QUEUE *ap)
{
	LpmReqq *rp;
	char *mp;

	switch (xdr->x_op) {
	default:
		return FALSE;

	case XDR_ENCODE:
		rp = (LpmReqq*)ap->head;
		for (;;)
		{
			bool_t more = rp != NULL;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			mp = rp->req;
			if (!xdr_int (xdr, &rp->try) ||
				!xdr_int (xdr, &rp->pid) ||
				!xdr_int (xdr, &rp->pri) ||
				!xdr_int (xdr, &rp->pp->idx) ||
				!XDR_XMESG (xdr, &mp))
					return FALSE;
			rp = (LpmReqq*)rp->q.next;
		}
		break;

	case XDR_DECODE:
		for (;;)
		{
			bool_t more;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			rp = (LpmReqq*) p_alloc (sizeof (LpmReqq));
			if (!rp)
				return FALSE;

			QinsertTail (ap, (QE*)rp);

			mp = rp->req;
			if (!xdr_int (xdr, &rp->try) ||
				!xdr_int (xdr, &rp->pid) ||
				!xdr_int (xdr, &rp->pri) ||
				!xdr_int (xdr, (int*)&rp->pp) ||
				!XDR_XMESG (xdr, &mp))
					return FALSE;
		}
		break;
	}

	return TRUE;
}

bool_t xdr_msgq (XDR *xdr, QUEUE *ap)
{
	LpmMsgq *pp;
	char *mp;

	switch (xdr->x_op) {
	default:
		return FALSE;

	case XDR_ENCODE:
		pp = (LpmMsgq*)ap->head;
		for (;;)
		{
			bool_t more = pp != NULL;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			mp = pp->msg;
			if (!xdr_int (xdr, &pp->pid) ||
				!XDR_XMESG (xdr, &mp))
					return FALSE;
			pp = (LpmMsgq*)pp->q.next;
		}
		break;

	case XDR_DECODE:
		for (;;)
		{
			bool_t more;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			pp = (LpmMsgq*) p_alloc (sizeof (LpmMsgq));
			if (!pp)
				return FALSE;

			QinsertTail (ap, (QE*)pp);

			mp = pp->msg;
			if (!xdr_int (xdr, &pp->pid) ||
				!XDR_XMESG (xdr, &mp))
					return FALSE;
		}
		break;
	}

	return TRUE;
}

bool_t xdr_hostq (XDR *xdr, QUEUE *ap)
{
	LpmHostq *pp;
	char *hp;

	switch (xdr->x_op) {
	default:
		return FALSE;

	case XDR_ENCODE:
		pp = (LpmHostq*)ap->head;
		for (;;)
		{
			bool_t more = pp != NULL;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;
			hp = pp->host;
			if (!xdr_int (xdr, &pp->flags) ||
				!xdr_u_long (xdr, &pp->haddr) ||
				!xdr_u_long (xdr, &pp->cpuid) ||
				!xdr_u_long (xdr, &pp->eaddr) ||
				!xdr_keys (xdr, &pp->keys) ||
				!XDR_XHOST (xdr, &hp))
					return FALSE;
			pp = (LpmHostq*)pp->q.next;
		}
		break;

	case XDR_DECODE:
		for (;;)
		{
			bool_t more;

			if (!xdr_bool (xdr, &more))
				return FALSE;
			if (!more)
				break;

			pp = (LpmHostq*) p_alloc (sizeof (LpmHostq));
			if (!pp)
				return FALSE;

			QinsertTail (ap, (QE*)pp);

			hp = pp->host;
			if (!xdr_int (xdr, &pp->flags) ||
				!xdr_u_long (xdr, &pp->haddr) ||
				!xdr_u_long (xdr, &pp->cpuid) ||
				!xdr_u_long (xdr, &pp->eaddr) ||
				!xdr_keys (xdr, &pp->keys) ||
				!XDR_XHOST (xdr, &hp))
					return FALSE;
		}
		break;
	}

	return TRUE;
}
