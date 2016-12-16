#ifndef _SIGNAL_H
#define _SIGNAL_H			1

#include <ucontext.h>
#include <sys/types.h>
#include <time.h>

// Default signal handler looks like void(*)(int)


extern void __signal_default(int);
extern void __signal_error(int);
extern void __signal_hold(int);
extern void __signal_ignore(int);


#define	SIG_DFL		__signal_default	// Request for default signal handling. 
#define SIG_ERR		__signal_error		// Return value from signal() in error. 
#define SIG_HOLD	__signal_hold		// Request that signal be held. 
#define SIG_IGN		__signal_ignore		// Request that signal be ignored.


#ifndef _HAVE_SIG_ATOMIC_T
#define _HAVE_SIG_ATOMIC_T
typedef int sig_atomic_t;		
#endif

#ifndef _HAVE_PTHREAD_ATTR_T
#define _HAVE_PTHREAD_ATTR_T
typedef	int32_t		pthread_attr_t;
#endif

#ifndef _HAVE_PTHREAD_T
#define _HAVE_PTHREAD_T
typedef	int32_t		pthread_t;
#endif



#ifndef _HAVE_SIGSET_T
#define _HAVE_SIGSET_T
typedef struct
{
	unsigned long bits[ (1024 / sizeof( unsigned long )) + 1 ];
} sigset_t;
#endif

#ifndef _HAVE_SIGVAL
#define _HAVE_SIGVAL
typedef union
	{
		int    sival_int;
		void*  sival_ptr;
	} sigval;
#endif

#ifndef _HAVE_SIGEVENT
#define _HAVE_SIGEVENT
struct sigevent
{
	int                      sigev_notify;				// notification type
	int                      sigev_signo;				// signal number
	sigval          	   	 sigev_value;				// signal value
	void (*sigev_notify_function)(sigval);		// function
	pthread_attr_t*        sigev_notify_attributes;	// attributes
};
#endif


#include <bits/signal.h>


#ifndef _HAVE_SIGINFO_T
#define _HAVE_SIGINFO_T
typedef struct
{
	int           si_signo;
	int           si_errno;
	int           si_code;
	pid_t         si_pid;
	uid_t         si_uid;
	void         *si_addr;
	int           si_status;
	long          si_band;
	sigval  si_value;
} siginfo_t;
#endif


#ifndef _HAVE_SIGACTION
#define _HAVE_SIGACTION
struct sigaction
{
	void 		(*sa_handler)(int);
	sigset_t 	sa_mask;
	int 		sa_flags;
	void 		(*sa_sigaction)(int, siginfo_t*, void*);
};
#endif




#ifndef _HAVE_STACK_T
#define _HAVE_STACK_T
typedef struct 
{
	void     *ss_sp;
	size_t    ss_size;
	int       ss_flags;
} stack_t;
#endif



#ifndef _HAVE_SIGSTACK
#define _HAVE_SIGSTACK
struct sigstack
{
	int       ss_onstack;
	void     *ss_sp;
};
#endif





#ifdef __cplusplus
extern "C" {
#endif
		

void (*bsd_signal(int, void (*)(int)))(int);
int    kill(pid_t, int);
int    killpg(pid_t, int);
int    pthread_kill(pthread_t, int);
int    pthread_sigmask(int, const sigset_t *, sigset_t *);
int    raise(int);
int    sigaction(int, const struct sigaction *, struct sigaction *);
int    sigaddset(sigset_t *, int);
int    sigaltstack(const stack_t *, stack_t *);
int    sigdelset(sigset_t *, int);
int    sigemptyset(sigset_t *);
int    sigfillset(sigset_t *);
int    sighold(int);
int    sigignore(int);
int    siginterrupt(int, int);
int    sigismember(const sigset_t *, int);
void (*signal(int, void (*)(int)))(int);


int    sigpause(int);
int    sigpending(sigset_t *);
int    sigprocmask(int, const sigset_t *, sigset_t *);
int    sigqueue(pid_t, int, const sigval);
int    sigrelse(int);
void (*sigset(int, void (*)(int)))(int);
int    sigstack(struct sigstack *ss,
				           struct sigstack *oss);
int    sigsuspend(const sigset_t *);
int    sigtimedwait(const sigset_t *, siginfo_t *,
				           const struct timespec *);
int    sigwait(const sigset_t *set, int *sig);
int    sigwaitinfo(const sigset_t *, siginfo_t *);


#ifdef __cplusplus
}
#endif
		

#endif

