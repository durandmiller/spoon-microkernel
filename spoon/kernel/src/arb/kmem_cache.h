#ifndef _KMEM_CACHE_H
#define _KMEM_CACHE_H


#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
typedef unsigned int size_t; 
#endif

#ifndef NULL
#define NULL	(void*)0
#endif


#define KMEM_OBJECTS		1024
#define KMEM_STACKSIZE		1024


/** This is the kmem_slab which is a bulk allocation of the
 * objects which are supposed to be cached by the kmem_cache
 * object.  Objects are allocated in bulk using one allocation
 * call and this slab is then split up into objects and popped
 * onto the free-object stack.
 */
typedef struct kmem_slab
{
	int usage;			///< The number of used objects in this slab
	int total;			///< The total number of objects in this slab
	void *ptr;			///< The pointer to a bulk allocation of objects.

	struct kmem_slab *next;	///< Linked list information.
	struct kmem_slab *prev;	///< Linked list information.
	
} kmem_slab_t;



/** This structure represents a stack object within the kmem_cache
 * structure.  Objects allocated from the slabs are popped
 * on and off these objects as they are required.  They're maintained
 * in a linked-list within the kmem_cache structure.
 */
typedef struct kmem_stack
{
	struct kmem_stack *next; ///< Linked list information.
	struct kmem_stack *prev; ///< Linked list information.

	int position;	///< The position in the stack.
	
	void *cache[KMEM_STACKSIZE];	///< The stack of constructed objects.

} kmem_stack_t;



/** This is the object cache structure proper which contains all
 * the information regarding the object cache 
 */
typedef struct kmem_cache
{
		
	size_t size;		///<  The size of an individual object
	size_t align;		///<  The alignment of the objects.
	int	low_mark;		///<  The low mark at which to bulk allocate new objects
	
	int (*constructor)(void *obj, void *private, int kmflag);	///< The constructor to call for each object
	void (*destructor)(void *obj, void *private);	///< The desctructor to call for each object.
	void *private;		///< Private data passed to the construction and destruction hooks.

	void* (*alloc)(size_t);	///< The allocation hook
	void (*free)(void*,size_t);	///< The free hook
	
	int cflags;				///<  Flags affecting the behaviour of the cache

	int total_objects;		///< The total num of objects owned by this cache
	int used_objects;		///< The total num of objects in use.

	kmem_slab_t  *slabs;	///< The linked list of slabs
	kmem_stack_t *stack;	///< The linked list of stacks
	
} kmem_cache_t;



/** The creation of a kmem object.
 *
 * \param size  The size of an individual object in the cache
 * \param align The alignment of an individual object in the cache
 * \param low_mark The level at which to bulk allocate some new objects
 * \param constructor  The constructor to call for each object called at allocation
 * \param destructor  The destructor to call for each object at destruction
 * \param private	Private data to pass to the constructor and destructor functions. Useful for having 2 functions servicing multiple object caches
 * \param alloc	The allocation hook. (can use stdlib/malloc)
 * \param free The free hook (can use stdlib/free)
 * \param cflags The flags which determine behaviour of the object cache.
 *
 * \return NULL if failure to allocate.
 * \return A new object cache
 */
kmem_cache_t*	kmem_cache_create(
					size_t size,
					size_t align,
					int low_mark,
					int (*constructor)(void *obj, void *private, int kmflag),
					void (*destructor)(void *obj, void *private),
					void *private,
					void* (*alloc)(size_t), 
					void (*free)(void*,size_t),
					int cflags);


/** Destroys and frees all memory associated with the object cache. This
 * will ensure that the destructor is called for all objects. Even the
 * objects currently being used by the system. So..
 *
 * \warning Make sure all objects are returned to the cache before 
 * 			destroying it.
 */
void 	kmem_cache_destroy(kmem_cache_t *cp);

/** Returns an object from the object cache.
 *
 * \return NULL if failure.
 * \return An constructed object if success.
 */
void*	kmem_cache_alloc(kmem_cache_t *cp, int kmflag);

/** Returns an object to the cache. */
void 	kmem_cache_free(kmem_cache_t *cp, void *obj);

/** Runs through the object cache and releases all slabs and
 * stacks which are currently available to be used. In effect, returns all
 * unused resources back to the system.
 */
void	kmem_cache_release(kmem_cache_t *cp);

/** This won't be here long. \todo Remove this function for integration into the kernel */
void	kmem_display(kmem_cache_t *cp);


#endif

