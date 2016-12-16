#include <smk.h>
#include <vfs.h>

#include <stdio.h>
#include <fcntl.h>

#include "../os.h"

/* C file to add support for OS file operations.  */

// File operations



int     os_open( const char *filename, unsigned int mode, 
					unsigned int bits,
					void **data, int *rc )
{
	int fd = vfs_open( filename, mode );
	if ( fd < 0 )
	{
		*rc = -1;
		return 0;
	}

	*data = (void*)fd;
	*rc = fd;
	return 0;
}



int     os_read( void *data, unsigned long long pos, void *buffer, int len, int *rc )
{
	int fd = (int)data;
	*rc = (int)vfs_read( fd, pos, buffer, len );
	return 0; 
}

int     os_write( void *data, unsigned long long pos, const void *buffer, int len, int *rc )
{
	int fd = (int)data;
	*rc = (int)vfs_write( fd, pos, buffer, len );
	return 0; 
}



int    os_tell( void *data, long *rc )
{
	return NOTIMPLEMENTED;
}

int     os_close( void *data, int *rc )
{
	int fd = (int)data;
	vfs_close( fd );
	return 0;
}




int		os_stat( void *data, struct stat *st, int *rc )
{
	return NOTIMPLEMENTED; 
}

int		os_mmap( void* data, void *start, 
				    unsigned long long length, int prot, int flags,
					unsigned long long offset, 
					int *rc, void **ptr )
{
	return NOTIMPLEMENTED;
}

int		os_munmap(void *start, unsigned long long length, int *rc)
{
	return NOTIMPLEMENTED;
}




int		os_delete( const char *filename, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_mkdir( const char *filename, unsigned int mode, int *rc )
{
	return NOTIMPLEMENTED;
}

int		os_rmdir( const char *filename, int *rc )
{
	return NOTIMPLEMENTED;
}




