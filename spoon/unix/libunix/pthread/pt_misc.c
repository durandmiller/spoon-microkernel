#include <pthread.h>
#include "../os/os.h"




pthread_t	pthread_self()
{
	pthread_t tid;
	int rc;

	if ( os_gettid( &tid, &rc ) == NOTIMPLEMENTED ) 
			os_freakout( "os_gettid not implemented" );

	return tid;
}

