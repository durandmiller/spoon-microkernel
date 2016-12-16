#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <support/support.h>

#include "os/os.h"



char **environ = NULL;	///< Global environment information



int          access(const char *filename, int mode)
{
	if ( mode == 0 )
	{
		struct stat st;
		if ( lstat( filename, &st ) == 0 ) return 0;
		return -1;
	}


	os_freakout( "non-existance test for unist.h(access). implement" );
	return 0;
}


int			chdir(const char *path)
{
	int fd;
	int result = -1;

	fd = open( path, O_RDONLY );
	if ( fd < 0 ) return -1;

		result = fchdir( fd ); 

	close( fd );
	return result;
}


int          chown(const char *filename, uid_t uid, gid_t gid)
{
	int fd;
	int result = -1;

	fd = open( filename, O_RDWR );
	if ( fd < 0 ) return -1;

		result = fchown( fd, uid, gid ); 

	close( fd );
	return result;
}



int          close(int fd)
{
	// Exist?
	int rc;
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;

	// Can close?
	if ( os_close( file->data, &rc ) == NOTIMPLEMENTED )
		   os_freakout( "os_close not implemented" );

	if ( rc != 0 ) return -1;
	support_remove_file( fd );
	free( file );
	return 0;
}


void        _exit(int rc)
{
	exit( rc );
}



int          fchown(int fd, uid_t uid, gid_t gid)
{
	int rc;
	FILE *file = NULL;
	
	file = support_retrieve_file( fd );
	if ( file == NULL )
	{
		errno = EBADF;
		return -1;
	}

	if ( os_chown( file->data, uid, gid, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_chown was not implemented" );

#warning Proper error code handling

	return rc;
}



int          fchdir(int fd)
{
	int rc;
	FILE *file = NULL;
	
	file = support_retrieve_file( fd );
	if ( file == NULL )
	{
		errno = EBADF;
		return -1;
	}

	if ( os_chdir( file->data, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_chdir was not implemented" );

#warning Proper error code handling

	return rc;
}



int          ftruncate(int fd, off_t offset)
{
	int rc;
	FILE *file = NULL;
	
	file = support_retrieve_file( fd );
	if ( file == NULL )
	{
		errno = EBADF;
		return -1;
	}

	//if ( os_chown( file->data, uid, gid, &rc ) == NOTIMPLEMENTED )
	os_freakout( "ftruncate was not implemented" );

#warning Proper error code handling

	return rc;
}


char        *getcwd(char *buf, size_t size)
{
	int rc;

	if ( os_getcwd( buf, size, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_getcwd is not implemented" );

	if ( rc == 0 ) return buf;

	errno = rc;
	return NULL;
}


pid_t        getpid(void)
{
	int pid, rc;

	if ( os_getpid( &pid, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_getpid has not been implemented" );

	return (pid_t)pid;
}

gid_t        getgid(void)
{
	uid_t uid;
	gid_t gid;
	int rc;

	if ( os_getid( &uid, &gid, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_getid has not been implemented" );

	return gid;
}




uid_t        getuid(void)
{
	uid_t uid;
	gid_t gid;
	int rc;

	if ( os_getid( &uid, &gid, &rc ) == NOTIMPLEMENTED )
		os_freakout( "os_getid has not been implemented" );

	return uid;
}


char        *getwd(char *buf)
{
#warning This is probably not correct
	return getcwd( buf, PATH_MAX );
}

int          isatty(int desc)
{	
	FILE* file = support_retrieve_file( desc );
	if ( file == NULL ) return 0;
#warning A temporary hack around for bc. All available terminals
	return 1;
}





off_t        lseek(int fd, off_t position, int whence)
{
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) 
	{
		return (position-1);
	}

	if ( fseek( file, position, whence ) == 0 )
			return file->pos;

	return (position-1);
}


ssize_t      read(int fd, void *buffer, size_t num)
{
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) 
	{
		errno = EBADF;
		return -1;
	}

	return fread( buffer, 1, num, file );
}


int          rmdir(const char *directory)
{
	int rc;
	if ( os_rmdir( directory, &rc ) == NOTIMPLEMENTED )
				os_freakout( "os_rmdir not implemented" );
	return rc;
}


unsigned int sleep(unsigned int seconds)
{
	unsigned int rc;

	if ( os_sleep( seconds, &rc ) == NOTIMPLEMENTED )
			os_freakout( "os_sleep not implemented" );
		
	return rc;
}


int          truncate(const char *filename, off_t offset)
{
	int fd;
	int result = -1;

	fd = open( filename, O_RDWR );
	if ( fd < 0 ) return -1;

		result = ftruncate( fd, offset ); 

	close( fd );
	return result;
}



ssize_t      write(int fd, const void * buffer, size_t num)
{
	FILE* file = support_retrieve_file( fd );
	if ( file == NULL ) return -1;

	return fwrite( buffer, 1, num, file);
}



int     unlink(const char *pathname)
{
	int rc;

	if ( os_delete( pathname, &rc ) == NOTIMPLEMENTED )
			os_freakout( "os_delete not implemented" );

#warning set errno to the cause

	return rc;
}



