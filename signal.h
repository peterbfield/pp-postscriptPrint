#ifndef _P_SIGNAL_H
#define _P_SIGNAL_H 1

#include <sys/resource.h>
#if OS_sunos
#include <wait.h>
#elif OS_dgux
#include <wait.h>
#elif OS_linux
#include <sys/wait.h>
#else
#error OS not sunos|dgux|linux
#endif
#include <signal.h>

#if OS_sunos
typedef void (*SignalHandler)(int);
#define berk_signal sigset
#elif OS_dgux
typedef int (*SignalHandler)(int);
extern int getrlimit(int, struct rlimit *);
#elif OS_linux
#define berk_signal signal
#else
#error OS not sunos|dgux|linux
#endif

typedef	void SIG_FUNC_TYP(int);
typedef	SIG_FUNC_TYP *SIG_TYP;

#endif
