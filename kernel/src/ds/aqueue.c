#include "../include/ds/aqueue.h"


/**  \addtogroup DS_AQUEUE
 *
 */

/** @{ */


/** Initializes a queue for use.  
 *
 * \param aq A pointer to the ds_aqueue structure
 * \param obj_size  The size of an individual object in the queue
 * \param obj_count The number of objects to keep per page
 */
void aqueue_init( struct ds_aqueue *aq, 
					unsigned int obj_size,
					unsigned int obj_count )
{
	atomic_init( &(aq->lock) );

	aq->pages 		= 0;
	aq->obj_size	= obj_size;
	aq->obj_count	= obj_count;

	aq->position		= 0;
	aq->last_position 	= 0;
	aq->staging 		= 0;

	aq->page 		= NULL;
	aq->last_page	= NULL;
}

/** Removes all the associated pages with this queue and
 * resets it back to empty. Call this if you want to clean
 * up a queue once you're done with it. 
 *
 * \warning If the individual objects in the queue contain
 *		 	allocated memory themselves, this memory 
 * 			will be lost because the aqueue isn't that smart.
 */
void aqueue_reset( struct ds_aqueue *aq )
{
	struct ds_aqueue_page *tmp = NULL;

	while ( aq->page != NULL )
	{
		tmp = aq->page;
		aq->page = tmp->next;
		free( tmp );
	}

	aqueue_init( aq, aq->obj_size, aq->obj_count );
}

/** Returns the number of objects in the queue */
unsigned int aqueue_size( struct ds_aqueue *aq )
{
	return (aq->last_position - aq->position);
}

/** Must be used before all access to the queue. */ 
void aqueue_lock( struct ds_aqueue *aq )   
{ 
	acquire_spinlock( &(aq->lock) ); 
}

/** Must be used after all access to the queue. */
void aqueue_unlock( struct ds_aqueue *aq ) 
{ 
	aq->staging = 0;
	release_spinlock( &(aq->lock) ); 
}


/** Returns a pointer to a new position in the queue. Lock
 * the queue before calling this function.
 *
 * Once you are finished setting up the object/position in
 * the queue, don't forget to call aqueue_save() and/or
 * unlock the queue to discard the changes
 *
 */
void* aqueue_new( struct ds_aqueue *aq )
{
	struct ds_aqueue_page *page = NULL;
	uintptr_t position;
		
	int page_div = aq->last_position / aq->obj_count;	// # pages in
	int page_mod = aq->last_position % aq->obj_count;	// # objects in
	

	if ( (page_mod == 0) && (aq->pages <= page_div) )	// Requires a new page.
	{
		int size = sizeof(struct ds_aqueue_page) + aq->obj_count * aq->obj_size;
		page = (struct ds_aqueue_page*)malloc( size );
		if ( page == NULL ) return NULL;

		page->next 	= NULL;
		aq->pages 	+= 1;

		if ( aq->page == NULL ) aq->page = page;
						else	aq->last_page->next = page;

		aq->last_page = page;
	}
	else
	{
		page = aq->last_page;
	}


	aq->staging = 1;		// This queue is now in staging mode.

	// At this point, page always points to the correct page to use.

	position = (uintptr_t)( page );
	position += sizeof( struct ds_aqueue_page );
	position += aq->obj_size * page_mod;
		
	return (void*)position;
}


/** Records the existance of the latest object in the queue */
void aqueue_save( struct ds_aqueue *aq )
{
	if ( aq->staging != 1 ) return;		// Don't save if nothing was staged

	aq->last_position 	+= 1;
	aq->staging 		= 0;
}


/** Gets the first object in the queue that's available
 * for reading or returns NULL if there's nothing there.
 *
 * Does NOT remove the object from the queue. Call aqueue_next
 * if you want to do that.
 */
void* aqueue_get( struct ds_aqueue *aq )
{
	uintptr_t position;
	int page_mod;

	if ( aq->position == aq->last_position ) return NULL;
												// Nothing in the queue.

	page_mod = aq->position % aq->obj_count;	// # objects in


	position = (uintptr_t)( aq->page );
	position += sizeof( struct ds_aqueue_page );
	position += aq->obj_size * page_mod;

	return (void*)position;
}

/** This does the same thing as aqueue_get but it also increments
 * the position in the queue and frees up the space as it goes 
 * along.
 */
void* aqueue_next( struct ds_aqueue *aq )
{
	uintptr_t position;
	int page_mod;
	int page_div;

	if ( aq->position == aq->last_position )	// Ensure position
	{
		aq->position = 0;
		aq->last_position = 0;
		return NULL;
	}
												// Nothing in the queue.

	page_div = aq->position / aq->obj_count;	// # pages in
	page_mod = aq->position % aq->obj_count;	// # objects in

	// Remove any unnecessary pages
	while ( page_div > 0 )
	{
		struct ds_aqueue_page *tmp = aq->page;
		aq->page = aq->page->next;
		free( tmp );
		if ( aq->page == NULL )	aq->last_page = NULL;

		aq->last_position 	-= aq->obj_count;
		aq->position 		-= aq->obj_count;
		aq->pages -= 1;

		page_div -= 1;	// Down one.
	}


	position = (uintptr_t)( aq->page );
	position += sizeof( struct ds_aqueue_page );
	position += aq->obj_size * page_mod;

	aq->position += 1;			// Remove this object from the queue..


	return (void*)position;		// But return it... 
}


/** @} */


