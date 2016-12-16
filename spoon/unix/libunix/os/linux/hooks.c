#include "../os.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <pthread.h>

#include <support/support.h>


#include "syscalls.h"



// --------------------------------------------------------


#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
# define MAP_ANONYMOUS MAP_ANON
#endif
#if !defined(MAP_FAILED)
# define MAP_FAILED ((char*)-1)
#endif

#ifndef MAP_NORESERVE
# ifdef MAP_AUTORESRV
#  define MAP_NORESERVE MAP_AUTORESRV
# else
#  define MAP_NORESERVE 0
# endif
#endif

struct mmap_arg_struct {
	unsigned long addr;
	unsigned long len;
	unsigned long prot;
	unsigned long flags;
	unsigned long fd;
	unsigned long offset;
};




int os_mem_alloc( int pages, void** ptr )
{
	unsigned int size = pages * 4096;
	struct mmap_arg_struct marg;

	// Set it up.
		marg.addr = 0;
		marg.len = size;
		marg.prot =  PROT_READ|PROT_WRITE;
		marg.flags = MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS;
		marg.fd = -1;
		marg.offset = 0;

	*ptr = sys_mmap( &marg );
	if ( *ptr == MAP_FAILED ) *ptr = NULL;
	return 0;
}


int		os_mem_free( void* mem, int pages )
{
	sys_munmap( mem, pages * 4096 );
	return 0;
}


int		os_acquire_spinlock( unsigned int *lock )
{
	int rc;

#ifdef __i386__
  while ( 1==1 )
  {
  	unsigned short gotit = 0;
	asm volatile ( "lock btsl $0x0, (%1);\n"
              	   "setnc %0;\n "
			  		: "=g" (gotit)
			  		: "p" (lock)
				);

	if ( gotit != 0 ) break;
	asm ( "pause" );

	os_sched_yield( &rc );
  }
  return 0;
#else
  return NOTIMPLEMENTED;
#endif
}

int		os_release_spinlock( unsigned int *lock )
{
#ifdef __i386__
	asm volatile ( "lock btrl $0x0, (%0)\n" : : "p" (lock) );
	return 0;
#else
  return NOTIMPLEMENTED;
#endif
}

int		os_gettid( pthread_t *tid, int *rc )
{
	*tid = sys_gettid();
	*rc = 0;
	return 0;
}



int		os_getpid( int *pid, int *rc )
{
	*pid = sys_getpid();
	*rc = 0;
	return 0;
}



int		os_getid( int *uid, int *gid, int *rc )
{
	*uid = sys_getuid();
	*gid = sys_getgid();
	*rc = 0;
	return 0;
}


int		os_getcwd( char *buf, unsigned int size, int *rc )
{
	sys_getcwd( buf, size );
	*rc = 0;
	return 0;
}


int		os_rand( int *rand )
{
	return NOTIMPLEMENTED;
}


int		os_puts( const char *str )
{
	return NOTIMPLEMENTED;
}


int		os_open( const char *filename, unsigned int flags, 
					unsigned int mode,
					void **data, int *fd )
{
	*fd = sys_open( filename, flags, mode );
	*data = (void*)*fd;
	return 0;
}


int		os_read( void *data, unsigned long long pos, void *buffer, int len, int *rc )
{
	unsigned int our_pos = sys_lseek( (int)data, (unsigned int)pos, 0 );

	if ( our_pos != (unsigned int)pos ) 
	{
#warning Appropriate errno set
		*rc = -1;
		return 0;
	}

	our_pos = sys_read( (int)data, buffer, len );
	*rc = our_pos;
	return 0;
}


int		os_write( void *data, unsigned long long pos, const void *buffer, int len, int *rc )
{
	sys_lseek( (int)data, (unsigned int)pos, 0 );
	*rc = sys_write( (int)data, buffer, len );
	return 0;
}



int		os_close( void *data, int *rc )
{
	*rc = sys_close( (int)data );
	return 0;
}





int		os_stat( void *data, struct stat *st, int *rc )
{
	struct __old_kernel_stat kst;

#warning This is wrong wrong wrong

	*rc = sys_fstat( (int)data, &kst );

#define	equal_it( name )   st->st_##name = kst.st_##name

		equal_it( ino );
		equal_it( dev );
		equal_it( mode );
		equal_it( nlink );
		equal_it( uid );
		equal_it( gid );
		equal_it( rdev );
		equal_it( size );
		equal_it( atime );
		equal_it( mtime );
		equal_it( ctime );

	return 0;
}


int	os_umask( int mask, int *rc )
{
	*rc = sys_umask( mask );
	return 0;
}


int 	os_chmod( void *data, mode_t mode, int *rc )
{
	*rc = sys_fchmod( (int)data, mode );
	return 0;
}


int 	os_chown( void *data, uid_t uid, gid_t gid, int *rc )
{
	*rc = sys_fchown( (int)data, uid, gid );
	return 0;
}

int		os_chdir( void *data, int *rc )
{
	*rc = sys_fchdir( (int)data );
	return 0;
}


int		os_mmap( void* data, void *start, 
				    unsigned long long length, int prot, int flags,
					unsigned long long offset, 
					int *rc, void **ptr )
{
	struct mmap_arg_struct marg;

	// Set it up.
		marg.addr = (unsigned long)start;
		marg.len = length;
		marg.prot =  prot;
		marg.flags = flags;
		marg.fd = (int)data;
		marg.offset = offset;

	*ptr = sys_mmap( &marg );
	if ( *ptr == MAP_FAILED ) 
	{
#warning proper rc codes
		*rc = -1;
		*ptr = NULL;
	}
	return 0;
}

int		os_munmap(void *start, unsigned long long length, int *rc)
{
	*rc = sys_munmap( start, length );
	return 0;
}


int		os_delete( const char *filename, int *rc )
{
	*rc = sys_unlink( filename );
	return 0;
}


int		os_mkdir( const char *filename, unsigned int mode, int *rc )
{
	*rc = sys_mkdir( filename, mode );
	return 0;
}


int		os_rmdir( const char *filename, int *rc )
{
	*rc = sys_rmdir( filename );
	return 0;
}


int		os_clock( clock_t *clock )
{
	*clock = (clock_t)-1;
	return 0;
}


int		os_system_time( time_t *seconds, time_t *milliseconds, int *rc )
{
	struct timeval tv;

	*rc = sys_gettimeofday( &tv, NULL );

	*seconds = tv.tv_sec;
	*milliseconds = tv.tv_usec / 1000;

	return 0;
}

	
int 	os_sleep( unsigned int seconds, unsigned int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_system( const char *s, int *rc )
{
	return NOTIMPLEMENTED;
}


void 	os_exit( int status )
{
	sys_exit( status );
}


int		os_send_signal( int pid, int sig, int *rc)
{
	*rc = sys_kill( pid, sig );
	return 0;
}


int 	os_getenv(const char* name, char **rc )
{
	*rc = NULL;
	return 0;
}

int 	os_sched_yield( int *rc )
{
	sys_sched_yield();
	*rc = 0;
	return 0;
}

void    os_freakout( const char *reason )
{
	printf("os_freakout called with: %s\n", reason );
	exit(-1);
}


