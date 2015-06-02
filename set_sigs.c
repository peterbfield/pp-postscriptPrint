#include <stdio.h>
#include <signal.h>
#include "p_lib.h"

void set_sigs();
void cleanup(int sig);
extern void exit(int);

void set_sigs()
{
	void cleanup();
	signal(SIGHUP,cleanup);
	signal(SIGINT,cleanup);
	signal(SIGQUIT,cleanup);
	signal(SIGTERM,cleanup);
}

void cleanup(int sig)
{
	p_info(PI_ELOG, "exit code %d\n",sig);
	exit(1);
}
