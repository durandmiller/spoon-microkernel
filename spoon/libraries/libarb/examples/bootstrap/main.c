#define _GNU_SOURCE
#define __USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>


#include "../../arb.h"
#include "../../kmem_cache.h"


#define		NUM_CACHES		5

static 	kmem_cache_t	b_cache;
static	kmem_stack_t	b_cache_stack;
static	kmem_slab_t		b_cache_slab;
static	char 			b_cache_data[ sizeof(arb_bt_t) * KMEM_OBJECTS ]; 


arb_t			b_arb;
static arb_bt_t*		b_arb_free_caches[NUM_CACHES];
static char				b_arb_data[1024];


static	unsigned int	b_arb_free_hits[ sizeof(unsigned int) * NUM_CACHES ]; 
static	unsigned int	b_arb_free_misses[ sizeof(unsigned int) * NUM_CACHES ]; 


// -----------------------------------------------------

static void *official_malloc( size_t size )
{
	void *ptr = (void*)arb_alloc( &b_arb, size, 0 );
	dprintf( 1, "official_malloc( %p, %u )\n", ptr, size );
	return ptr;
}

static void official_free( void *ptr, size_t size )
{
	dprintf( 1, "official_free( %p, %u )\n", ptr, size );
	arb_free( &b_arb, (arbnum_t)ptr, (arbnum_t)size );
}



// -----------------------------------------------------

static void *fake_malloc( size_t size )
{
	dprintf( 1, "fake_malloc( %u )\n", size );
	assert( size <= 1024 );
	return b_arb_data;
}

static void fake_free( void *ptr, size_t size )
{
	dprintf( 1, "fake_free( %p, %u )\n", ptr, size );
	assert( "fake_free" == "called" );
}

// -----------------------------------------------------

void setup_cache_slab( kmem_slab_t* slab )
{
	slab->total = KMEM_OBJECTS;
	slab->usage = 0;
	slab->next	= NULL;
	slab->prev	= NULL;

	slab->ptr	= &b_cache_data;
}

void setup_cache_stack( kmem_stack_t* stack )
{
	stack->next = NULL;
	stack->prev = NULL;
	stack->position = 0;
}


void setup_cache( kmem_cache_t *kmem )
{
	int i;

	kmem->size 	= sizeof( arb_bt_t );
	kmem->align = 0;
	kmem->constructor = NULL;
	kmem->destructor = NULL;
	kmem->private = NULL;
	kmem->alloc = fake_malloc;
	kmem->free = fake_free;
	kmem->cflags = 0;

	kmem->used_objects = 0;
	kmem->low_mark = 5;


	// set up the stack.
	setup_cache_stack( &b_cache_stack );
	kmem->stack = &b_cache_stack;

	// set up the slabs
	setup_cache_slab( &b_cache_slab );
	kmem->slabs = &b_cache_slab;

	// Load the objects into the stack
	for ( i = 0; i < b_cache_slab.total; i++ )
	{
		kmem->stack->cache[ (kmem->stack->position)++ ] = 
				kmem->slabs->ptr + i * kmem->size;
	}

	kmem->total_objects = kmem->slabs->total;
}


// ----------------------------------------------


void setup_arb_span( arb_t *arb, 
					 arb_span_t *span, 
					 arbnum_t start, arbnum_t size )
{
	span->start = start;
	span->size = size;
	span->next = NULL;
}


void setup_arb( arb_t *arb )
{
	int i = 0;

	arb->cache_max = NUM_CACHES;
	arb->alloc 	= fake_malloc;
	arb->free 	= fake_free;
	arb->units 	= 0x1000;

	arb->tags 	= NULL;
	arb->spans 	= NULL;

	arb->admin_usage		=	0;
	arb->admin_overhead		=   sizeof( arb_t );
	arb->admin_hash			=	0;
	arb->admin_bt			=	0;
	arb->admin_span			=	0;

	arb->kmem_bt			=	&b_cache;

	arb->free_cache = b_arb_free_caches;

	for ( i = 0; i < arb->cache_max; i++ )
			arb->free_cache[i] = NULL;


#ifdef _TEST_
		arb->free_hits = b_arb_free_hits;
		arb->free_misses = b_arb_free_misses;
#endif

}


#ifdef _TEST_
static void dump_info()
{
	arb_display( &b_arb, 0 );
}
#endif




// ----------------------------------------------

int main( int argc, char *argv[] )
{
	int i;

	atexit( dump_info );

	setup_cache( &b_cache );
	setup_arb( &b_arb );

		void *data = malloc( 0x100000 );
		assert( data != NULL );

		arb_add_range( &b_arb, data, 0x100000 );	// Fake free will assist

		b_arb.alloc = official_malloc;
		b_arb.free = official_free;


		data = malloc( 0x100000 );
		assert( data != NULL );

		arb_add_range( &b_arb, data, 0x100000 );

		b_cache.alloc = official_malloc;
		b_cache.free = official_free;
		
		/*
		for ( i = 0; i < 4000; i++ )
		{
			void *obj = kmem_cache_alloc( &b_cache, 0 );
		}
		*/

		while ( arb_alloc( &b_arb, 0x1000, 0 ) != ARB_NOMEM )
				continue;
		

	//arb_destroy( &b_arb );
		
	return EXIT_SUCCESS;
}



