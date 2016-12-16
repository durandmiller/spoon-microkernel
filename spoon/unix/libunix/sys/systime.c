#include <sys/time.h>
#include <errno.h>

#include "../os/os.h"





int   gettimeofday(struct timeval *tv, struct timezone *tz)
{
	time_t seconds;
	time_t milliseconds;
	int rc;

	if ( tz != NULL )
	{
		errno = EINVAL;
		return -1;
	}

	if ( os_system_time( &seconds, &milliseconds, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_system_time not implemented" );

	tv->tv_sec = seconds;
	tv->tv_usec = milliseconds;
	tv->tv_usec = tv->tv_usec * 1000;

	return 0;
}


int   settimeofday(const struct timeval *tv, const struct timezone *tz)
{
	if ( tz != NULL )
	{
		errno = EINVAL;
		return -1;
	}

	#warning Implement me
	errno = EPERM;
	return -1;
}



