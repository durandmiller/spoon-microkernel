#include "include/atomic.h"
#include "include/physmem.h"
#include "include/dmesg.h"

/** Supporting functions for the liballoc library file */

static spinlock_t alloc_lock = INIT_SPINLOCK;


int liballoc_lock()
{
	acquire_spinlock( &alloc_lock );
	return 0;
}

int liballoc_unlock()
{
	release_spinlock( &alloc_lock );
	return 0;
}

int liballoc_abort(int code)
{
	dmesg("%!liballoc_abort!");
	return 0;
}


void* liballoc_alloc( int pages )
{
	return memory_alloc( pages );
}

int liballoc_free( void* ptr, int pages )
{
	memory_free( pages, ptr );
	return 0;
}



