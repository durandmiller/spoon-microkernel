#include <smk.h> 
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include "../os.h"


int		os_system( const char *s, int *rc )
{
	return NOTIMPLEMENTED;
}


void 	os_exit( int status )
{
	smk_exit( status );
}


int		os_send_signal( int pid, int sig, int *rc )
{
	return NOTIMPLEMENTED;
}


int 	os_getenv(const char* name, char **env )
{
	int size;

	*env = NULL;
		
	size = smk_getenv_size( name );
	if ( size <= 0 ) return 0;

	*env = malloc( size );
	if ( env == NULL ) return 0;
	
	if ( smk_getenv( name, *env, size ) <= 0 )
	{
		free( *env );
		*env = NULL;
	}

	return 0;
}






