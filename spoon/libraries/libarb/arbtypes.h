#ifndef _ARBTYPES_H
#define _ARBTYPES_H


#include <inttypes.h>
#include "kmem_cache.h"


#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
typedef unsigned int size_t; 
#endif

#ifndef _HAVE_ARBNUM_T
#define _HAVE_ARBNUM_T
typedef uintptr_t	arbnum_t;
#define ARB_NOMEM			((uintptr_t)(-1))
#endif

#ifndef NULL
#define NULL	(void*)0
#endif


#define	ARB_BESTFIT			0
#define ARB_INSTANT			1
#define	ARB_SEARCH			2


#define ARB_FREE			0
#define ARB_ALLOCATED		1



/** The boundary tag used to identify ranges */
typedef struct arb_bt
{
	arbnum_t start;		///<  The first valid unit
	arbnum_t size;			///<  The size of this range
	int allocated;			///<  Indication of allocation state

	struct arb_bt *next;	///<  Linked list information
	struct arb_bt *prev;	///<  Linked list information

	int			fl_indicator;	///< Indicates the belonging to a free list
	struct arb_bt *fl_next;		///< Free-list next;
	struct arb_bt *fl_prev;		///< Free-list prev;
} arb_bt_t;


typedef struct arb_span
{
	arbnum_t start;
	arbnum_t size;

	arb_bt_t	**hash;		///<  The allocated hash table.
	
	struct arb_span *next;
} arb_span_t;



/** The actual structure used to maintain the resource
 * allocator */
typedef struct arb
{
	arbnum_t units;			///< The units of the resources
	arbnum_t cache_max;		///< The maximum range size to cache
	//int		 base_shift;	///< The amount to shift ranges by during bit lookup

	arbnum_t	admin_usage;	///< The total amount of memory allocated
	arbnum_t	admin_overhead;	///< The memory used for admin (bt,spans,etc)
	arbnum_t	admin_hash;		///< The memory used by hashes
	arbnum_t	admin_bt;		///< The number of bt's
	arbnum_t	admin_span;		///< The number of spans's

	arb_bt_t 	*tags;			///< The ordered, linked list of boundary tags
	arb_span_t 	*spans;			///< The ordered, linked list of spans

	arb_bt_t	**free_cache;		///< The free page caches

#ifdef _TEST_
	unsigned int	*free_hits;	///< The free page caches hits/misses
	unsigned int	*free_misses;	///< The free page caches hits/misses
#endif

	kmem_cache_t	*kmem_bt;	///< The BT object cache

	void* (*alloc)(size_t);			///< The allocation function
	void  (*free)(void*,size_t);	///< The free function.
} arb_t;



#endif

