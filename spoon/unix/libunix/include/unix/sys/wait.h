#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H		1


#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>



#define WEXITED		4
#define WSTOPPED	2
#define	WCONTINUED	8
#define WNOHANG		1
#define WUNTRACED	2
#define	WNOWAIT		0x1000000





#ifndef _HAVE_IDTYPE_T
#define _HAVE_IDTYPE_T
typedef enum
{
	P_ALL,
	P_PID,
	P_PGID
}	idtype_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif


pid_t  wait(int *);
pid_t  wait3(int *, int, struct rusage *);
int    waitid(idtype_t, id_t, siginfo_t *, int);
pid_t  waitpid(pid_t, int *, int);


#ifdef __cplusplus
}
#endif




#endif

