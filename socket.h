#ifndef _P_SOCKET_H
#define _P_SOCKET_H 1

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#if OS_dgux
#if OS_dgux < 3
extern int getsockname(int, struct sockaddr*, int*);
extern int accept(int, struct sockaddr*, int*);
extern int bind(int, struct sockaddr*, int);
extern int connect(int, struct sockaddr*, int);
extern int getpeername(int, struct sockaddr*, int*);
extern int getsockname(int, struct sockaddr*, int*);
extern int getsockopt(int, int, int, char*, int*);
extern int listen(int, int);
extern int recv(int, char*, int, int);
extern int recvfrom(int, char*, int, int, struct sockaddr*, int*);
extern int send(int, const char*, int, int);
extern int sendto(int, const char*, int, int, struct sockaddr*, int);
extern int setsockopt(int, int, int, const char*, int);
extern int socket(int, int, int);
extern int recvmsg(int, struct msghdr*, int);
extern int sendmsg(int, struct msghdr*, int);
extern int shutdown(int, int);
extern int socketpair(int, int, int, int*);
extern int select(int, fd_set*, fd_set*, fd_set*, struct timeval*);
extern char* inet_ntoa(struct in_addr);
#endif
extern int inet_netof(struct in_addr);
#endif

#endif
