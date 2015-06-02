#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "p_lib.h"
#include "msg_parse.h"

/*
 * char *message - pointer to input message block to be parsed.
 * int msize - number of bytes in the input message block to be parsed.
 * int mcount - count (at least one) of the number of messages to be parsed.
 * struct msg_parse *msg - pointer to structure which contains parse results.
 */
int msg_parse (char *message, int msize, int mcount, struct msg_parse *msg)
{
	char *mp, *kp, *ap, *ep;
	int i, escape, rc = 0;

	/* change all tabs, newlines, formfeeds, returns to nulls */
	mp = message;
	ep = message + msize;
	escape = 0;
	ap = 0;
	while (mp < ep)
		switch (*mp) {
		default:					/* Leave most chars as is.  */
			if (!ap) ap = mp;		/* Hold loc of 1st char of string.  */
			mp++;
			break;
		case 1:						/* Filter ctrl-a to space.  */
			if (!ap) ap = mp;
			*mp++ = ' ';
			break;
		case ':':					/* Colon: Alone it terms a string.  */
			if (*(mp-1) == '\\')
			{
				escape = 1;			/* But after \, it doesn't.  */
				mp++;
				break;
			}
		case '\t':
		case '\n':
		case '\f':
		case '\r':
		case ' ':					/* Any white-space or colon terms an arg.  */
			*mp++ = '\0';
			if (ap && escape)
			{
				kp = ap-1;		/* Copy inptr ap to outptr kp.  */
								/* Loop to copy string ap into string kp,
									including closing null: */
				while ((*++kp = *ap++))
								/* But upon "\:", bump ap 1 extra and
									adjust, to drop the backslash:  */
					if ((*kp=='\\') && (*ap==':'))
						*kp = *ap++;
								/* Account for dropped \s by appending nulls: */
				while (++kp < ap) *kp = '\0';
			}
			escape = 0;
			ap = 0;
			break;
		}

	/* set all answers to null */
	for (i = 0; i < mcount; i++)
		msg[i].answer = "";

	mp = message;
	for (;;)
	{
		if (mp >= ep)					/* done with process */
			break;

		while (mp < ep && *mp == '\0')	/* look for start of keyword string */
			mp++;
		if (mp >= ep)
			break;
		kp = mp;						/* found start of keyword */
		while (mp < ep && *mp != '\0')	/* look for end of keyword */
			mp++;
		while (mp < ep && *mp == '\0')	/* look for start of answer string */
			mp++;
		ap = mp;						/* found start of answer */
		while (mp < ep && *mp != '\0')	/* look for end of answer */
			mp++;
		if (kp == ap)
			break;
		/* look for matching keyword */
		for (i = 0; i < mcount; i++)
			if (!strcmp (kp, msg[i].keyword))	/* got a match */
			{
				msg[i].answer = ap;
				if (*ap)
					rc++;
				break;
			}

		if (i == mcount)
			p_info (PI_WLOG, "MSG_PARSE: WARNING: KEYWORD '%s', ANSWER '%s' is not used.\n", kp, ap);
	}
	return rc;
}

int msg_parse_args (int argc, char *argv[], int nmsgs, struct msg_parse *msg)
{
	static char buf[2048];
	char *bp, *ep;
	int i, len;

	bp = buf;
	ep = buf + sizeof (buf);

	memset (bp, 0, sizeof (buf));
	for (i = 1; i < argc; i++)
	{
		len = strlen (argv[i]) + 1;
		if (bp + len >= ep)
		{
			p_info (PI_WLOG, "MSG_PARSE_ARGS: ERROR: total length of args exceeds max of %d\n", sizeof (buf));
			break;
		}

		strcpy (bp, argv[i]);
		bp += len;
	}

	return msg_parse (buf, bp - buf, nmsgs, msg);
}
