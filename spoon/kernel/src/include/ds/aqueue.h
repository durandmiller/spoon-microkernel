#ifndef _KERNEL_DS_AQUEUE_H
#define _KERNEL_DS_AQUEUE_H

#include "../atomic.h"
#include "../alloc.h"

/**  \defgroup DS_AQUEUE  Atomic FIFO queue 
 *
 */

/** @{ */


/** This is an individual page in the queue. It contains the objects. 
 * The last page in a queue will always have next set to NULL. It's
 * a linked-list.
 */
struct ds_aqueue_page
{
	struct ds_aqueue_page *next;	///< The next page.
};

/** This is the primary aqueue structure which contains
 * information about the queue and the pages within it.
 * It's initialized by the aqueue_init() call. It maintains
 * a linked-list of pages. This structure can be cleaned
 * by calling aqueue_reset();
 */
struct ds_aqueue
{
	spinlock_t 	 lock;				///< The lock for this queue
	volatile unsigned int pages;	///< The number of active pages in queue
	unsigned int obj_size;			///< The size of an object in the queue
	unsigned int obj_count;			///< The number of objects per page

	volatile unsigned int position;		///< Position in the page for reading.
	volatile unsigned int last_position; ///< Position of the first free slot.
	volatile unsigned int staging;		///< Indicates the staging of a new

	struct ds_aqueue_page *page;		///< The first page in a queue.
	struct ds_aqueue_page *last_page;	///< The last page in a queue.
};

/** @} */

void 		aqueue_init( struct ds_aqueue *aq, 
					unsigned int obj_size,
					unsigned int obj_count );

void 		aqueue_reset( struct ds_aqueue *aq );
unsigned int aqueue_size( struct ds_aqueue *aq );

void 		aqueue_lock( struct ds_aqueue *aq );
void 		aqueue_unlock( struct ds_aqueue *aq );

void* 		aqueue_new( struct ds_aqueue *aq );
void 		aqueue_save( struct ds_aqueue *aq );
void* 		aqueue_get( struct ds_aqueue *aq );
void* 		aqueue_next( struct ds_aqueue *aq );


#endif

