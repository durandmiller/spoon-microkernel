#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H		1


#include <signal.h>
#include <sys/time.h>
#include <time.h>


int  pselect(int, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
			  const struct timespec *timeout, const sigset_t *sigmask);

int  select(int, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
				         struct timeval *timeout);


#endif

