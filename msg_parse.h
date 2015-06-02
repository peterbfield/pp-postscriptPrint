#ifndef _MSG_PARSE_H
#define _MSG_PARSE_H

typedef struct msg_parse MSG_PARSE;
struct msg_parse
{
#define MAXSIZE_KEYWORD 20
	char keyword[MAXSIZE_KEYWORD];
	char *answer;
};
#define msg_parse_size(msg) (sizeof ((msg)) / sizeof (MSG_PARSE))

int msg_parse (char *message, int msize, int mcount, MSG_PARSE *msg);
int msg_parse_args (int argc, char *argv[], int nmsgs, MSG_PARSE *msg);

#endif
