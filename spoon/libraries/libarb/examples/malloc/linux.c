#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>



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


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int os_lock()
{
	pthread_mutex_lock( &mutex );
	return 0;
}

int os_unlock()
{
	pthread_mutex_unlock( &mutex );
	return 0;
}

void* os_alloc( size_t size )
{
	char *p2 = (char*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0);
	if ( p2 == MAP_FAILED) return NULL;

/*
	char *p2 = (char*)mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0);
	if ( p2 == MAP_FAILED) return NULL;

	if(mprotect(p2, size, PROT_READ|PROT_WRITE) != 0) 
	{
		munmap(p2, size);
		return NULL;
	}
*/

	return p2;
}

int os_free( void* ptr, size_t size )
{
	return munmap( ptr, size );
}



