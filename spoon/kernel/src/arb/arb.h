#ifndef _ARB_H
#define _ARB_H


#include "arbtypes.h"


#define ARB_VERSION		0.90


/** Creates an arbitrary resource manager with
 * resources of units using the alloc and free functions
 * as resource thingies.  The cache_max is the largest
 * size resources which will be cached.
 *
 * \return NULL if it was unable to create the resource
 * manager.
 */
arb_t*		arb_create( arbnum_t units,  
						void* (*alloc)(size_t), 
						void (*free)(void*, size_t),
						arbnum_t cache_max );

/** Releases all memory associated with the resource allocator */
void		arb_destroy( arb_t* arb );


/** Adds a range to the resource allocator.  This range will be
 * divied up later by calls to the alloc and free functions.
 *
 * \param arb	The resource allocator to work with.
 * \param start The starting position of the range being added.
 * \param size  The size of the range being added.
 *
 * \return 0 for successful addition of the range.
 * 
 * \warning Added ranges should NOT overlap. Never.
 *
 * \note Please read the warning.
 */
int			arb_add_range( arb_t* arb, arbnum_t start, arbnum_t size );



/** Specifically allocates a certain range starting at res of 
 * specified size.
 *
 * \returns ARB_NOMEM if that range was not successfully reserved.
 * \returns res for success
 */
arbnum_t	arb_reserve( arb_t *arb, arbnum_t res, arbnum_t rsize );


/** This searches the resource for a range of the requested size.
 *
 * \return ARB_NOMEM if allocation failed.  (MAX_UINTPTR)
 * \return A valid position within the resource to use.
 */
arbnum_t	arb_alloc( arb_t* arb, arbnum_t rsize, int flags );


/** This frees the range and returns it back to the resource allocator.
 *
 * \param arb  The resource allocator to work with.
 * \param res  The starting position of the resource to return.
 * \param rsize The size of the resource being returned. This MUST be the
 *              same of the size passed to alloc.
 */
void 		arb_free( arb_t* arb, arbnum_t res, arbnum_t rsize );


/** This is an information function call which returns information regarding
 * the ranges in use and free.
 *
 * \param arb  The resource allocator to work with.
 * \param num  The number of the range to work with.
 * \param start The returned start of the range
 * \param size The returned size of the range
 *
 * \return ARB_FREE if the range is free
 * \return ARB_ALLOCATED if the range is in use
 * \return -1 if the requested range is invalid.
 */
int			arb_get( arb_t* arb, int num, arbnum_t *start, arbnum_t *size );


/** This returns the size of the range whose starting position in
 * the resource is at res.
 *
 * \return ARB_NOMEM if the resource is invalid
 * \return The size of the resource if it's valid.
 */
arbnum_t	arb_size( arb_t *arb, arbnum_t res );

/** This will be removed soon. \todo Remove soon */
int 		arb_display( arb_t *arb, int full );





#endif

