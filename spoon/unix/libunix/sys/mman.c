#include <sys/mman.h>
#include <support/support.h>

#include "../os/os.h"


#warning proper error number setting for mmap
void  *mmap(void *start, size_t length, 
				int prot, int flags, int fd, off_t offset )
{
	int rc;
	void *ptr;
	void *data = NULL;

	FILE* file = support_retrieve_file( fd );
	if ( file != NULL ) data = (void*)-1;

	if ( os_mmap( data, start, 
				    (unsigned long long)length, 
					prot, flags,
					(unsigned long long)offset, 
					&rc, &ptr ) != OK )
			os_freakout( "os_mmap not implemented." );

	return ptr;
}


int    munmap(void *start, size_t length)
{
	int rc;

	if ( os_munmap( start, (unsigned long long)length, &rc ) != OK )
			os_freakout( "os_munmap not implemented." );

	return rc;
}



