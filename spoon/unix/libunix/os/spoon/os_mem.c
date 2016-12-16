#include <smk.h>
#include <stdio.h>
#include "../os.h"

int 	os_mem_alloc( int pages, void** ptr )
{
	*ptr = (void*)smk_mem_alloc( pages );
	return 0;
}	

int		os_mem_free( void* mem, int pages )
{
	smk_mem_free( mem );
	return 0;
}



