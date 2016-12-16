#include "../os.h"

int 	os_mem_alloc( int pages, void** ptr )
{
	return NOTIMPLEMENTED;
}

 
int		os_mem_free( void* mem, int pages )
{
	return NOTIMPLEMENTED;
}


int		os_acquire_spinlock( unsigned int *lock )
{
#ifdef __i386__
  while ( 1==1 )
  {
  	unsigned char gotit = 0;
	int yc;
	asm volatile ( "lock btsl $0x0, (%1)\n"
              	   "setncb %0\n "
			  		: "=g" (gotit)
			  		: "p" (lock)
				);

	if ( gotit != 0 ) break;
	asm ( "pause" );

	os_sched_yield( &yc );
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
	return NOTIMPLEMENTED;
}


int		os_getpid( int *pid, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_getid( int *uid, int *gid, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_getcwd( char *buf, unsigned int size, int *rc )
{
	return NOTIMPLEMENTED;
}




int		os_rand( int *rand )
{
	return NOTIMPLEMENTED;
}


int		os_puts( const char *str )
{
	return NOTIMPLEMENTED;
}


int		os_open( const char *filename, unsigned int mode, 
					unsigned int bits,
					void **data, int *fd )
{
	return NOTIMPLEMENTED;
}


int		os_read( void *data, unsigned long long pos, void *buffer, int len, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_write( void *data, unsigned long long pos, const void *buffer, int len, int *rc )
{
	return NOTIMPLEMENTED;
}



int		os_close( void *data, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_stat( void *data, struct stat *st, int *rc )
{
	return NOTIMPLEMENTED;
}

int	os_umask( int mask, int *rc )
{
	return NOTIMPLEMENTED;
}


int 	os_chown( void *data, uid_t uid, gid_t gid, int *rc )
{
	return NOTIMPLEMENTED;
}

int 	os_chmod( void *data, mode_t mode, int *rc )
{
	return NOTIMPLEMENTED;
}


int		os_chdir( void *data, int *rc )
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


int		os_clock( clock_t *clock )
{
	return NOTIMPLEMENTED;
}


int		os_system_time( time_t *seconds, time_t *milliseconds, int *rc )
{
	return NOTIMPLEMENTED;
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
	os_freakout( "normal os_exit" );
}


int		os_send_signal( int pid, int sig, int *rc)
{
	return NOTIMPLEMENTED;
}


int 	os_getenv(const char* name, char **rc )
{
	return NOTIMPLEMENTED;
}

int 	os_sched_yield( int *rc )
{
	return NOTIMPLEMENTED;
}

void    os_freakout( const char *reason )
{
	while (1==1) continue;
}


