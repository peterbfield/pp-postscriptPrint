#ifndef _NOPROTO_H
#define _NOPROTO_H 1

/*
 * Include prototypes here for system funcs that
 * don't have prototypes in any system header files.
 * Socket and signal related funcs should go in
 * "socket.h" and "signal.h", respectively.
 * If structure info is needed, its probably best to
 * include appropriate system header files in
 * the file that includes this file, rather than
 * including them here.
 */
#if 0
#include <sys/time.h>
#endif

#if OS_sunos
extern int gethostname(char*, int);	/* BSD flavor */
#if OS_sunos < 5
extern int gettimeofday(struct timeval* tp);
#define p_gettimeofday(a)		gettimeofday((a));
#else
#define p_gettimeofday(a)		gettimeofday((a), NULL);
#endif
#elif OS_dgux
#if OS_dgux < 3
extern void openlog(const char*, int, int);
extern void syslog(int, const char*, ...);
extern int setitimer(int, struct itimerval*, struct itimerval*);
extern long gethostid(void);
extern int gettimeofday(struct timeval*, struct timezone*);
#endif
extern int gethostname(char*, int);	/* BSD flavor */
#define p_gettimeofday(a)		gettimeofday((a), NULL);
#elif OS_linux
#define p_gettimeofday(a)		gettimeofday((a), NULL);
#else
#error OS not sunos|dgux|linux
#endif

/* comes from libgen.a on dgux and sunos, libc on linux. linux has proto in string.h */
#if OS_sunos
extern char* basename (char*);
#elif OS_dgux
extern char* basename (char*);
#elif OS_linux
extern char* basename (char*);
#endif

#endif
