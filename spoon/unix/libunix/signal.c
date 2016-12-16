#include <errno.h>
#include <signal.h>
#include "os/os.h"

void (*bsd_signal(int sig, void (*func)(int)))(int)
{
	struct sigaction act, oact;

		act.sa_handler = func;
		act.sa_flags = SA_RESTART;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, sig);
		if (sigaction(sig, &act, &oact) == -1)
					return SIG_ERR;

	return oact.sa_handler;
}


int    kill(pid_t pid, int sig)
{
#warning proper error codes

	if ( pid > 0 )
	{
		int rc;

		if ( os_send_signal( pid, sig, &rc ) == NOTIMPLEMENTED )
			os_freakout( "os_send_signal is not implemented" );

		return rc;
	}

	if ( pid == 0 )
	{
		os_freakout( "kill not implemented for pid == 0" );
	}

	if ( pid == -1 )
	{
		os_freakout( "kill not implemented for pid == -1" );
	}


	if ( pid < -1 )
	{
		os_freakout( "kill not implemented for pid < -1" );
	}



	errno = EPERM;
	return -1;
}

int raise(int sig)
{
	return kill( getpid(), sig );
}



int    sigaction(int signum, 
					const struct sigaction *act, 
					struct sigaction *oldact )
{

	os_freakout( "sigaction called but not implemented" );

//	if ( os_signal( num, handle, &rc ) == NOTIMPLEMENTED )
//		os_freakout( "os_signal is not implemented." );


	#warning implement me
	return -1;
}


int    sigaddset(sigset_t *set, int signum)
{
	#warning implement me
	return -1;
}


int    sigemptyset(sigset_t *set)
{
	#warning implement me
	return -1;
}



void (*signal(int num, void (*handle)(int)))(int)
{
	struct sigaction act, oact;

		act.sa_handler = handle;
		act.sa_flags = SA_RESTART;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, num);
		if (sigaction(num, &act, &oact) == -1)
					return SIG_ERR;

	return oact.sa_handler;
}



int siginterrupt(int sig, int flag) 
{
	struct sigaction act;
				    
	sigaction(sig, NULL, &act);
	if (flag)
		act.sa_flags &= ~SA_RESTART;
	else
		act.sa_flags |= SA_RESTART;
	
	return sigaction(sig, &act, NULL);
}








// ---- DEFAULT SIGNAL HANDLERS ----------------------

void __signal_default(int sig)
{
}


void __signal_error(int sig)
{
}


void __signal_hold(int sig)
{
}


void __signal_ignore(int sig)
{
}




// ---- FINISHED


