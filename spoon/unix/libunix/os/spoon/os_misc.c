#include <smk.h>
#include "../os.h"

int os_acquire_spinlock( unsigned int *lock  )
{
	acquire_spinlock( lock );
	return 0;
}

int os_release_spinlock( unsigned int *lock  )
{
	release_spinlock( lock );
	return 0;
}


int		os_gettid( pthread_t *tid, int *rc )
{
	return NOTIMPLEMENTED;
}

int		os_getpid( int *pid, int *rc )
{
	return NOTIMPLEMENTED;
}


int	os_rand( int *rc )
{
	return NOTIMPLEMENTED;
}



void    os_freakout( const char *reason )
{
	smk_abort( -1, reason );
}



