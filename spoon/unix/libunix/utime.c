#include <utime.h>
#include <errno.h>

#include "os/os.h"


int utime(const char *filename, const struct utimbuf *buf)
{
/*
	int rc;
	if ( os_system_time( &seconds, &milliseconds, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_system_time not implemented" );

	os_freakout( "utime not implemented" );
*/
	errno = EPERM;
	return -1;
}


