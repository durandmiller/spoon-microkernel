#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <support/support.h>

#include "../os/os.h"




int    chmod(const char *filename, mode_t mode)
{
	int fd;
	int result = -1;

	fd = open( filename, O_RDWR );
	if ( fd < 0 ) return -1;

		result = fchmod( fd, mode ); 

	close( fd );
	return result;
}

int    fchmod(int fd, mode_t mode)
{
	int rc;
	FILE *file = NULL;
	
	file = support_retrieve_file( fd );
	if ( file == NULL )
	{
		errno = EBADF;
		return -1;
	}

	if ( os_chmod( file->data, mode, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_chmod was not implemented" );

#warning Proper error code handling

	return rc;
}


int    mkdir(const char *dirname, mode_t mode)
{
	int rc;
		
	if ( os_mkdir( dirname, mode, &rc ) == NOTIMPLEMENTED )
			os_freakout( "os_mkdir not implemented" );

	return rc;
}


int    fstat(int fd, struct stat *st)
{
	int rc;

	errno = 0;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL )
	{
		errno = EBADF;
		return -1;
	}

	if ( os_stat( file->data, st, &rc ) == NOTIMPLEMENTED )
		   os_freakout( "os_stat not implemented" );

#warning set errno correctly

	return rc;	
}




int    stat(const char *filename, struct stat *st)
{
	int rc;
#warning First resolve any symbolic links. Get the real file. 
	int fd = open( filename, O_RDONLY );
	if ( fd < 0 ) return -1;

	rc = fstat( fd, st );
	close( fd );
	return rc;
}


int    lstat(const char *filename, struct stat *st)
{
	int rc;
	int fd = open( filename, O_RDONLY );
	if ( fd < 0 ) return -1;

	rc = fstat( fd, st );
	close( fd );
	return rc;
}



mode_t	umask(mode_t mask)
{
	int rc;

	if ( os_umask( mask, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_umask not implemented" );

	return rc;
}


/** This is the global umask within the application space.  It's global so
 * there's no need for TLS. */
/*
static mode_t		__UNIX_umask 		= 0x022;	// from bash
static unsigned int 	__UNIX_umask_lock 	= 0;

mode_t umask(mode_t mask)
{
	mode_t	previous;
	os_acquire_spinlock( &__UNIX_umask_lock );
		previous = __UNIX_umask;
		__UNIX_umask = mask & 0x777;
	os_release_spinlock( &__UNIX_umask_lock );
	return previous;
}
*/

/** Not declared in headers because it's not
 * really supposed to be used anywhere. However, used in:
 *
 * fcntl.c/open
 */
/*
mode_t get_umask()
{
	mode_t	previous;
	os_acquire_spinlock( &__UNIX_umask_lock );
		previous = __UNIX_umask;
	os_release_spinlock( &__UNIX_umask_lock );
	return previous;
}
*/





