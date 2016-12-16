#ifdef _TEST_
#define _GNU_SOURCE
#define __USE_GNU
#include <assert.h>
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <arb.h>
#include "os.h"


//#define		SMALLEST_BLOCK		(1<<25)
//#define		SMALLEST_BLOCK		(1<<20)
#define		SMALLEST_BLOCK		(1<<16)


static arb_t *malloc_arb = NULL;


static void* compatible_alloc(size_t size)
{
#ifdef _TEST_
	dprintf(1,"system allocation(%u)\n", size );
#endif
	return os_alloc(size);
}

static void compatible_free(void* ptr,size_t size)
{
	os_free(ptr,size);
}


static void increase_ranges( size_t size )
{
	void *ptr;
	size_t real_size = size;
	
	if ( real_size < SMALLEST_BLOCK)
			real_size = SMALLEST_BLOCK;

	real_size = ((real_size >> 12) + 1) << 12;

	ptr = os_alloc( real_size );

	if ( ptr != NULL )
		arb_add_range( malloc_arb, (arbnum_t)ptr, real_size );
}

static int init_structure( size_t size )
{
	malloc_arb = arb_create( 1,  
								compatible_alloc, 
								compatible_free,
								16384 );
	if ( malloc_arb == NULL ) return -1;

	increase_ranges( size );

	return 0;
}



#ifdef _TEST_
static void dump_info()
{
	arb_display( malloc_arb, 0 );
}
#endif


void *malloc(size_t size)
{
	void *ptr = NULL;
	arbnum_t tmp = 0;

	if ( size == 0 ) return malloc(5);
	if ( size == 0 ) return NULL;
	
	os_lock();

		if ( malloc_arb == NULL )
		{
			if ( init_structure(size) != 0 )
			{
				os_unlock();
				return NULL;
			}

#ifdef _TEST_
			atexit( dump_info );
#endif
		}
	
		// Request the thing.
	
		tmp = arb_alloc( malloc_arb, size, ARB_BESTFIT ); 
		if ( tmp == ARB_NOMEM )
		{
			// One more time.
			increase_ranges( size );
			tmp = arb_alloc( malloc_arb, size, ARB_BESTFIT ); 
			if ( tmp != ARB_NOMEM )
					ptr = (void*)tmp;
		}
		else
		{
			ptr = (void*)tmp;
		}

	
	
	os_unlock();
	return ptr;
}

void free(void *ptr)
{
	os_lock();
	
		if ( malloc_arb != NULL )
		{
			arbnum_t size = arb_size( malloc_arb, (arbnum_t)ptr );
			if ( size != ARB_NOMEM )
			{
				arb_free( malloc_arb, (arbnum_t)ptr, size );
			}
		}

	os_unlock();
}

void *realloc(void *ptr, size_t size)
{
	void *new_ptr = NULL;
	arbnum_t real_size;

	if ( ptr == NULL ) return malloc( size );
	if ( size == 0 ) 
	{
		free( ptr );
		return NULL;
	}
		
	os_lock();

		real_size = arb_size( malloc_arb,  (arbnum_t)ptr );
		if ( real_size != ARB_NOMEM )
		{
			if ( real_size < size )		// Request is bigger..
			{
				os_unlock();

					new_ptr = malloc( size ); 
					if ( new_ptr != NULL )
					{
						memcpy( new_ptr, ptr, real_size );
					}
						
				os_lock();

				if ( new_ptr != NULL )
					arb_free( malloc_arb, (arbnum_t)ptr, real_size );
			}
			else	// Request is smaller. Safe to use
			{
				new_ptr = ptr;
			}
		}
	
	os_unlock();
	return new_ptr;
}


void *calloc(size_t nmemb, size_t size)
{
	void *ptr = malloc(nmemb*size);
	if ( ptr == NULL ) return NULL;
	memset( ptr, 0, nmemb*size);
	return ptr;
}
							


