#ifdef _TEST_
#define _GNU_SOURCE
#define __USE_GNU
#include <assert.h>
#include <stdio.h>
#include <string.h>
#endif


#include "arb.h"


// -------------------------------------------------
static inline arb_span_t	*arb_getspan( arb_t *arb, arbnum_t res )
{
	arb_span_t *span = arb->spans;

	while ( span != NULL )
	{
		if ( (res >= span->start) && (res < (span->start + span->size)) )
			break;

		span = span->next;
	}

	return span;
}

/** Returns:  base * 2^power */
static inline arbnum_t		toPow( arbnum_t base, int power )
{
	if ( power < 0 ) return 0;
		
	return (base << power);
}

/** Returns an integer(i) such that:  2^(i+1) > value >= 2^i  */
static inline int		highbit( arbnum_t value )
{
	int i;
	
	asm ( "movl $-1, %%eax;"
		  "bsrl %1, %%eax;"
		  "movl %%eax, %0;"
		  : "=g" (i)
		  : "g" (value)
		  : "eax" );

	if ( i < 0 ) return -1;
	return i;
}




/** Adds the bt to the freelists, if it should */
static inline void 	add_freelist( arb_t *arb, arb_bt_t *bt )
{
#ifdef _TEST_
	assert( bt->fl_indicator == -1 );
#endif

	int pos = highbit( bt->size/arb->units );

#ifdef _TEST_
	assert( pos >= 0 );
#endif

	if ( pos < 0 ) return;				// Too small
	if ( pos >= arb->cache_max ) return; 
	/*
	{
		pos = arb->cache_max - 1;
	}
	*/

	// ... just right.

	if ( arb->free_cache[pos] != NULL )
			arb->free_cache[pos]->fl_prev = bt;

	bt->fl_prev = NULL;
	bt->fl_next = arb->free_cache[pos];
	arb->free_cache[pos] = bt;

	bt->fl_indicator = pos;
}

/** remove a bt from a freelist */
static inline void 	rem_freelist( arb_t *arb, arb_bt_t *bt )
{
	if ( bt->fl_indicator == -1 ) return;
		
	
	//int pos = highbit( bt->size ) - arb->base_shift;
	int pos = bt->fl_indicator;

#ifdef _TEST_
	assert( pos >= 0 );
#endif

	//if ( pos < 0 ) return;				// Too small
	//if ( pos >= arb->cache_max ) return; 
	/*
	{
		pos = arb->cache_max - 1;
	}
	*/

	// ... just right.

	if ( arb->free_cache[pos] == bt )
			arb->free_cache[pos] = bt->fl_next;

	if ( bt->fl_prev != NULL ) bt->fl_prev->fl_next = bt->fl_next;
	if ( bt->fl_next != NULL ) bt->fl_next->fl_prev = bt->fl_prev;

	bt->fl_next = NULL;
	bt->fl_prev = NULL;
	bt->fl_indicator = -1;
}

// -------------------------------------------------


// -------------------------------------------------



arb_t*		arb_create( arbnum_t units,  
						void* (*alloc)(size_t), 
						void (*free)(void*,size_t),
						arbnum_t cache_max )
{
	arb_t *arb = alloc( sizeof( arb_t ) );
	if ( arb == NULL ) return NULL;

		arb->kmem_bt = 	
			kmem_cache_create( sizeof(arb_bt_t),
								0,
								0,
								NULL,
								NULL,
								NULL,
								alloc,
								free,
								0 );

		if ( arb->kmem_bt == NULL )
		{
			free( arb, sizeof(arb_t) );
			return NULL;
		}
								

		/*
#ifdef _TEST_
		{
			int i = 1;
			int j = 0;
			for ( i = 1; i < 16; i++ )
			{
				int base_shift = highbit(i);
					
				for ( j = 0; j < 20; j++ )
				{
					dprintf(1, "num = %i, base = %i, (%i>>%i) = %i (%i) | (%i) = %i (%i)\n", 
								j, 
								i,
								highbit(j), 
								base_shift,
								highbit(j>>base_shift),
								toPow( i, highbit(j>>base_shift) ),
								j / i,
								highbit(j/i),
								toPow( i, highbit(j/i) )
								);

					if (  j < toPow( i, highbit(j>>base_shift) ) ) 
					{
						dprintf(1,"failure on cause 1\n" );
						exit(0);
					}


					if (  j < toPow( i, highbit(j/i)) ) 
					{
						dprintf(1,"failure on cause 2\n" );
						exit(0);
					}
				}
			}
			exit(0);
		}
#endif
*/

		arb->alloc 	= alloc;
		arb->free 	= free;
		arb->units	= units;
		arb->cache_max = highbit(cache_max/units) + 1;

		arb->tags 	= NULL;
		arb->spans 	= NULL;


		arb->admin_usage		=	0;
		arb->admin_overhead		=   sizeof( arb_t );
		arb->admin_hash			=	0;
		arb->admin_bt			=	0;
		arb->admin_span			=	0;


		// Allocate free page list.
		if ( arb->cache_max > 0 )
		{
			int i = 0;

			arb->free_cache = alloc( sizeof( arb_bt_t* ) * arb->cache_max );
			if ( arb->free_cache == NULL )
			{
				free( arb, sizeof( arb_t ) );
				return NULL;
			}

			for ( i = 0; i < arb->cache_max; i++ )
					arb->free_cache[i] = NULL;


#ifdef _TEST_
			arb->free_hits = alloc( sizeof( unsigned int ) * arb->cache_max );
			arb->free_misses = alloc( sizeof( unsigned int ) * arb->cache_max );
			for ( i = 0; i < arb->cache_max; i++ )
			{
				arb->free_hits[i] = 0;
				arb->free_misses[i] = 0;
			}
#endif

			
		}
		else
		{
			arb->free_cache = NULL;
#ifdef _TEST_
			arb->free_hits = NULL;
			arb->free_misses = NULL;
#endif
		}



	return arb;
}


void		arb_destroy( arb_t* arb )
{
	arb_span_t *span = arb->spans;
	arb_bt_t *bt = arb->tags;

	while ( bt != NULL )
	{
		arb_bt_t *tmp = bt->next;
		kmem_cache_free( arb->kmem_bt, bt );
		bt = tmp;
	}

	while ( span != NULL )
	{
		arb_span_t *tmp = span->next;

		arbnum_t req = span->size / arb->units;
				 req = req * sizeof(arb_bt_t*);
		
		arb->free( span->hash, req );
		kmem_cache_free( arb->kmem_bt, span );
		span = tmp;
	}


	if ( arb->free_cache != NULL )
		arb->free( arb->free_cache, 
						sizeof( arb_bt_t* ) * arb->cache_max );

#ifdef _TEST_
		arb->free( arb->free_hits, sizeof(unsigned int) * arb->cache_max );
		arb->free( arb->free_misses, sizeof(unsigned int) * arb->cache_max );
#endif

	kmem_cache_destroy( arb->kmem_bt );
		
	arb->free( arb, sizeof(arb_t) );
}



int			arb_add_range( arb_t* arb, arbnum_t start, arbnum_t size )
{
	int i = 0;
	if ( size == 0 ) return -1;
	if ( (size % arb->units) != 0 ) return -1;
		
	/* ------- SPAN -------- */
	// Determine position in list.
	arb_span_t *insert_span = NULL;
	arb_span_t *span = arb->spans;
	while ( span != NULL )
	{
		if ( span->start >= start ) break;

		insert_span = span;
		span = span->next;
	}


	// Create the structure
	span = kmem_cache_alloc( arb->kmem_bt, 0 );
	if ( span == NULL ) return -1;

	arb->admin_overhead	+=	sizeof(arb_span_t);
	arb->admin_span		+=	1;

		span->start = start;
		span->size = size;

		arbnum_t req = size / (arb->units);
				 req = sizeof(arb_bt_t*) * req;
				 
		span->hash = arb->alloc( req );
		if ( span->hash == NULL )
		{
			kmem_cache_free( arb->kmem_bt, span );
			arb->admin_overhead	-=	sizeof(arb_span_t);
			arb->admin_span		-=	1;
			return -1;
		}

		arb->admin_overhead	+= sizeof(arb_bt_t*) * (size/arb->units);
		arb->admin_hash		+= sizeof(arb_bt_t*) * (size/arb->units);

		for ( i = 0; i < (size/arb->units); i++ )	// clean it
				span->hash[i] = NULL;


	
		
	/* ------------------------------------------------------- */
	arb_bt_t *insert = NULL;
	arb_bt_t *bt = arb->tags;

	
	// Determine position in list.
	bt = arb->tags;
	while ( bt != NULL )
	{
		if ( bt->start >= start ) break;

		insert = bt;
		bt = bt->next;
	}


	// Create the BT structure
	bt = kmem_cache_alloc( arb->kmem_bt, 0 );
	if ( bt == NULL ) 
	{
		arb->free( span->hash, req );
		kmem_cache_free( arb->kmem_bt, span );
		arb->admin_overhead	-=	sizeof(arb_span_t);
		arb->admin_span		-=	1;
		arb->admin_overhead	-= sizeof(arb_bt_t*) * (size/arb->units);
		arb->admin_hash		-= sizeof(arb_bt_t*) * (size/arb->units);
		return -1;
	}

		// ------ SPAN INSERT --------------
		// Insert with disregard to overlapping.
		if ( insert_span == NULL )
		{
			span->next = arb->spans;
			arb->spans = span;
		}
		else
		{
			span->next = insert_span->next;
			insert_span->next = span;
		}
		// ------ END OF SPAN INSERT
	

	arb->admin_bt		+= 1;
	arb->admin_overhead	+=	sizeof(arb_bt_t);

		bt->start = start;
		bt->size = size;
		bt->allocated = ARB_FREE;
		bt->fl_indicator = -1;
		bt->fl_next = NULL;
		bt->fl_prev = NULL;

		// Insert with disregard to overlapping.
		if ( insert == NULL )
		{
			bt->next = arb->tags;
			bt->prev = NULL;
			arb->tags = bt;
		}
		else
		{
			bt->next = insert->next;
			bt->prev = insert;
		}
	
	
		if ( bt->prev != NULL ) bt->prev->next = bt;
		if ( bt->next != NULL ) bt->next->prev = bt;

		
	add_freelist( arb, bt );
	return 0;
}



/** This is a static function to assist the arb_alloc function. It returns
 * the first available BT of requested size by searching the 2^n free
 * lists.  
 *
 * \note It does not remove it the BT from the list or mark it as used.
 */
static	arb_bt_t*	arb_al_freesearch( arb_t* arb, arbnum_t rsize, int flags )
{
#ifdef _TEST_
	int orig;
#endif
	int pos = highbit( rsize/arb->units );

	if ( pos < 0 ) return NULL;					// Too small
	if ( pos >= arb->cache_max ) return NULL;
	/*
	{
		pos = arb->cache_max - 1;
	}
	*/

	
	if ( flags == ARB_INSTANT )					// If an instant fit is required.
	{
		pos += 1;
		if ( pos >= arb->cache_max ) return NULL; 
			//	pos -= 1;
	}

	// -----
#ifdef _TEST_
	orig = pos;
#endif
	
	// Search this free list and upwards.
	while ( pos < arb->cache_max )
	{
		arb_bt_t *bt = arb->free_cache[ pos ];
		while ( bt != NULL )
		{
			if ( bt->size >= rsize ) 
			{
#ifdef _TEST_
				if ( orig == pos )
					arb->free_hits[orig] += 1;
#endif
				return bt;
			}
			bt = bt->fl_next;
		}

#ifdef _TEST_
		if ( orig == pos )
		{
			arb->free_misses[orig] += 1;
		}
#endif

		pos += 1;	// Next list
		break;	// Don't really search, sorry.
	}

		
	return NULL;
}

/** This splits a boundary tag into two boundary tags (required size
 * is on the right) and returns a pointer to the boundary tag
 * of requested size. 
 *
 * The new tag of requested size is marked as ARB_FREE but not 
 * in the freelist.
 *
 */ 
static arb_bt_t*	arb_al_splittag( arb_t *arb, arb_bt_t *bt, arbnum_t size )
{
	arb_bt_t *tmp = kmem_cache_alloc( arb->kmem_bt, 0 );
	if ( tmp == NULL ) return NULL;

#ifdef _TEST_
	assert( bt->allocated == ARB_FREE );
#endif
			
	// Accounting
	arb->admin_bt		+= 1;
	arb->admin_overhead	+=	sizeof(arb_bt_t);


	// Remove the BT from the freelist
	rem_freelist( arb, bt );

	// truncate bt
		bt->size 		= bt->size - size;

	// temporary
		tmp->start 		= bt->start + bt->size;
		tmp->size		= size;
		tmp->prev		= bt;
		tmp->next		= bt->next;
		tmp->fl_indicator = -1;
		tmp->fl_next	= NULL;
		tmp->fl_prev	= NULL;
		tmp->allocated 	= ARB_FREE;

		if ( tmp->next != NULL )
				tmp->next->prev = tmp;

		bt->next		= tmp;

	// Re-insert BT into the free-list with the new size.
	add_freelist( arb, bt );
	return tmp;
}


arbnum_t	arb_reserve( arb_t *arb, arbnum_t res, arbnum_t rsize )
{
	arb_bt_t *bt = arb->tags;
	arbnum_t size = rsize % arb->units;

	if ( rsize == 0 ) return ARB_NOMEM;

	if ( size == 0 ) size = rsize;
				else size = rsize + (arb->units - size);

#ifdef _TEST_
	dprintf(1, "arb_reserve( %p, %p, %p )\n", 
					(void*)res, 
					(void*)rsize, 
					(void*)size );
#endif


	// ----------------------------
	while ( bt != NULL )
	{
		if ( bt->start > res ) 
		{
			bt = NULL;
			break;
		}

		if ( (bt->start <= res) && ((bt->start + bt->size) >= (res+size)))
		{
			arbnum_t diff; 	// Got it!
			arb_bt_t *right = NULL;

			if ( (bt->start == res) && (bt->size == size) )
			{
				// Hooray! Perfect
				rem_freelist( arb, bt );
				break;
			}


			if ( bt->start == res )	// Needs to be split
			{
				diff = bt->size - size;
				right = arb_al_splittag( arb, bt, diff );
				add_freelist( arb, right );
				rem_freelist( arb, bt );
				break;
			}

			diff = bt->start + bt->size;
			diff = diff - (res + size);

			if ( diff != 0 )
			{
				right = arb_al_splittag( arb, bt, diff );
				add_freelist( arb, right );
			}

			bt = arb_al_splittag( arb, bt, size );
			break;
		}

		bt = bt->next;
	}



	if ( bt == NULL ) return ARB_NOMEM;


	// Now mark the BT as allocated
	bt->allocated = ARB_ALLOCATED;

	arb_span_t *span = arb_getspan( arb, bt->start );

		span->hash[ (bt->start - span->start) / arb->units ] = bt; 

	arb->admin_usage	+= size;
	return bt->start;
}


arbnum_t	arb_alloc( arb_t* arb, arbnum_t rsize, int flags )
{
	arb_bt_t *bt;
	arbnum_t size = rsize % arb->units;

	if ( rsize == 0 ) return ARB_NOMEM;

	if ( size == 0 ) size = rsize;
				else size = rsize + (arb->units - size);


	// ----------------  First, free list search
	if ( flags != ARB_SEARCH )
	{
		bt = arb_al_freesearch( arb, size, flags );
		if ( bt == NULL ) bt = arb->tags;
	}
	else
	{
		bt = arb->tags;
	}

	// ----------------  Now search the actual linked list
		
	// Free list determination and break up
	while ( bt != NULL )
	{
		if ( (bt->allocated != ARB_FREE) || (bt->size < size) ) 
		{
			bt = bt->next;
			continue;
		}
			
		if ( bt->size != size )	// We can split this tag
		{
			bt = arb_al_splittag( arb, bt, size );
		}
		else
		{
			// Otherwise, it's no longer free.
			rem_freelist( arb, bt );
		}
				
		bt->allocated = ARB_ALLOCATED;
		break;
	}

	// At this point, bt is either valid or not.
	if ( bt == NULL ) return ARB_NOMEM;


	arb_span_t *span = arb_getspan( arb, bt->start );

		span->hash[ (bt->start - span->start) / arb->units ] = bt; 

	arb->admin_usage	+= size;
	return bt->start;
}


void 		arb_free( arb_t* arb, arbnum_t res, arbnum_t size )
{
	arb_bt_t *tmp;

	arb_span_t *span = arb_getspan( arb, res );
	if ( span == NULL ) return;

	arb_bt_t *bt = span->hash[ (res - span->start) / arb->units ]; 
	if ( bt == NULL ) return;


	span->hash[ (res - span->start) / arb->units ] = NULL; 
				// Clear the BT


	arb->admin_usage	-= bt->size;

	bt->allocated = ARB_FREE; 

	// Now we melt it back in.

	if ( bt->prev != NULL )	// Left
	{
		if ( (bt->prev->allocated == ARB_FREE) && (bt->prev->start >= span->start)  )
		{
			rem_freelist( arb, bt->prev );	// Remove from free list
				
			bt->prev->size  = bt->prev->size + bt->size;
			bt->prev->next 	= bt->next;

			if ( bt->next != NULL )
					bt->next->prev = bt->prev;

			tmp = bt->prev;

			kmem_cache_free( arb->kmem_bt, bt );
			bt = tmp;


			arb->admin_bt		-= 1;
			arb->admin_overhead	-=	sizeof(arb_bt_t);
		}
	}

	// bt is still valid

	if ( bt->next != NULL )
	{
		if ( (bt->next->allocated == ARB_FREE) && 
					(bt->next->start < (span->start + span->size))  )	// Right
		{
			rem_freelist( arb, bt->next );
				
			bt->size = bt->size + bt->next->size;

			tmp = bt->next;

			bt->next = tmp->next;

			if ( tmp->next != NULL )
					tmp->next->prev = bt;

			kmem_cache_free( arb->kmem_bt, tmp );

			arb->admin_bt		-= 1;
			arb->admin_overhead	-=	sizeof(arb_bt_t);
		}
	}

	// bt is still valid

	add_freelist( arb, bt );		// Insert new size into free list
}



int			arb_get( arb_t* arb, int num, arbnum_t *start, arbnum_t *size )
{
	int i = 0;
	arb_bt_t *bt = arb->tags;
	
	while ( bt != NULL )
	{
		if ( i++ == num )
		{
			*start = bt->start;
			*size = bt->size;
			return bt->allocated;
		}

		bt = bt->next;
	}
	

	return -1;
}




// --------------------------------



#ifdef _TEST_
int arb_display( arb_t *arb, int full )
{
	int i = 0;
	arb_bt_t *bt = arb->tags;
	arb_span_t *span = arb->spans;
	arbnum_t 	managed = 0;

	float perc = arb->admin_overhead;
	if ( arb->admin_usage == 0 ) perc = 0;
	else
			perc = perc / arb->admin_usage;

	// Count total managed range
	while ( span != NULL )
	{
		managed += span->size;
		span = span->next;
	}
	

	dprintf(1, "---------------------------\n");
	dprintf(1, "total = %p, usage = %p  (%2.2f%%)\n",
					(void*)managed,
					(void*)arb->admin_usage,
					(((float)arb->admin_usage)/((float)managed))
					);
	dprintf(1, "overhead = %p (%2.2f%%)\n",
					(void*)arb->admin_overhead,
					(perc * 100));
	dprintf(1, "hash = %p\n", (void*)arb->admin_hash );
	dprintf(1, "bt = %p,  span = %p\n", 
					(void*)arb->admin_bt, 
					(void*)arb->admin_span );


	// --- Print free list information

	for ( i = 0; i < arb->cache_max; i++ )
	{
		int count = 0;
		int invalid = 0;

		dprintf(1, "%i: %5u - %5u : ",  
					toPow( arb->units, i ), 
					arb->free_hits[i],
					arb->free_misses[i] );

		// Actual display
		bt = arb->free_cache[ i ];
		while ( bt != NULL )
		{
			dprintf(1, " %i,", (int)bt->size ); 

			if ( bt->allocated != ARB_FREE ) invalid = 1;
			count += 1;

			bt = bt->fl_next;
		}

		// Now verify the free caches against the list
		bt = arb->tags;
		while ( bt != NULL )
		{
			if ( bt->allocated == ARB_FREE )
			{
				/*
				if ( i == ( arb->cache_max - 1 ) )
				{
					if ( bt->size >= toPow( arb->units, i )) 
						count -= 1;
				}
				else
				{
				*/
					if ( (bt->size >= toPow( arb->units, i )) && 
						 (bt->size < toPow( arb->units, i+1 )) )
						count -= 1;
				//}
			}
			bt = bt->next;
		}

		// -- check count
		if ( count != 0 ) invalid = 1;

		// -------
		if ( invalid == 1 )
				dprintf(1, "(INVALID!!)\n");
			else
				dprintf(1, "(GOOD)\n" );
	}


	// -------------------------------

	if ( full == 0 ) return 0;

	// -------------------------------
	
	bt = arb->tags;
	while ( bt != NULL )
	{
		char *msg;

		if ( bt->allocated == ARB_ALLOCATED )
				msg = "USED";
			else
				msg = "FREE";
			
						
		dprintf(1,  "(%s) %p  ->  %p  (%p)  [prev = %p, this = %p, next = %p]\n", 
						msg, 
						(void*)(bt->start), 
						(void*)(bt->start + bt->size), 
						(void*)(bt->size),
						bt->prev,
						bt,
						bt->next
						);

		bt = bt->next;
	}

	return 0;
}
#endif



arbnum_t	arb_size( arb_t *arb, arbnum_t res )
{
	arb_span_t *span = arb_getspan( arb, res );
	if ( span == NULL ) return ARB_NOMEM;

	arb_bt_t *bt = span->hash[ (res - span->start) / arb->units ]; 
	if ( bt == NULL ) return ARB_NOMEM;

	return bt->size;
}

