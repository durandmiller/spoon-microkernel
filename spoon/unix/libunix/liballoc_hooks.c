#include "os/os.h"





static unsigned int lock = 0;


int liballoc_lock()
{
	if ( os_acquire_spinlock( &lock ) != 0 ) return -1;
	return 0;
}

int liballoc_unlock()
{
	if ( os_release_spinlock( &lock ) != 0 ) return -1;
	return 0;
}

void* liballoc_alloc( int pages )
{
	void *ptr;
		
	if ( os_mem_alloc( pages, &ptr ) == NOTIMPLEMENTED )
			os_freakout( "os_mem_alloc not implemented" );

	return ptr;
}

int liballoc_free( void* ptr, int pages )
{
	if ( os_mem_free( ptr, pages ) == NOTIMPLEMENTED )
			os_freakout( "os_mem_free not implemented" );

	return 0;
}





