#include "kmem_cache.h"




// ---- internal functions --------------

static kmem_slab_t*	kmem_new_slab( kmem_cache_t* cp, int kmflag )
{
	int i;
		
	// Allocate the slab structure
	kmem_slab_t *slab = cp->alloc( sizeof(kmem_slab_t) );
	if ( slab == NULL ) return NULL;

		slab->total = KMEM_OBJECTS;
		slab->usage = 0;
		slab->next	= NULL;
		slab->prev	= NULL;

		// Allocate the object space
		slab->ptr	= (void*)cp->alloc( cp->size * KMEM_OBJECTS );
		if ( slab->ptr == NULL ) 
		{
			cp->free( slab, sizeof(kmem_slab_t) );
			return NULL;
		}


	// Construct all the objects
	if ( cp->constructor != NULL )
	{
		for ( i = 0; i < slab->total; i++ )
		{
			void *obj = slab->ptr + i * cp->size;
	
			if ( cp->constructor(obj, cp->private, kmflag ) != 0 )
			{
				slab->total = i;
				break;
			}
		}
	}

	// Check to see at least 1 was constructed
	if ( slab->total == 0 )
	{
		cp->free( slab->ptr, cp->size * KMEM_OBJECTS ); 
		cp->free( slab, sizeof(kmem_slab_t) ); 
		return NULL;
	}

	cp->total_objects += slab->total;
	return slab;
}


static void	kmem_destroy_slab( kmem_cache_t* cp, kmem_slab_t *slab )
{
	int i;
		
	// Destruct all the objects
	if ( cp->destructor != NULL )
	{
		for ( i = 0; i < slab->total; i++ )
		{
			void *obj = slab->ptr + i * cp->size;
			cp->destructor(obj, cp->private );
		}
	}

	cp->total_objects -= slab->total;

	// Free the memory
	cp->free( slab->ptr, cp->size * KMEM_OBJECTS ); 
	cp->free( slab, sizeof(kmem_slab_t) ); 
}


static kmem_slab_t *kmem_get_slab( kmem_cache_t* cp, void* obj )
{
	kmem_slab_t *tmp = cp->slabs;
	while ( tmp != NULL )
	{
		if ( (tmp->ptr <= obj) && (obj < (tmp->ptr + tmp->total*cp->size)) )
				break;

		tmp = tmp->next;
	}
	
	return tmp;
}


static kmem_stack_t*	kmem_new_stack( kmem_cache_t* cp, int kmflag )
{
	int i;
		
	kmem_stack_t* stack = cp->alloc( sizeof(kmem_stack_t) );
	if ( stack == NULL ) return NULL;

		stack->next = NULL;
		stack->prev = NULL;
		stack->position = 0;


		// Now allocate the slab for the stack
		kmem_slab_t *slab = kmem_new_slab( cp, kmflag );
		if ( slab == NULL ) 
		{
			cp->free( stack, sizeof(kmem_stack_t) );
			return NULL;
		}

		// insert into list
		if ( cp->slabs != NULL ) cp->slabs->prev = slab;
		slab->next = cp->slabs;
		cp->slabs = slab;

		
		// Load the objects into the stack
		for ( i = 0; i < slab->total; i++ )
		{
			stack->cache[ (stack->position)++ ] = slab->ptr + i * cp->size;
		}

	return stack;
}

static void kmem_destroy_stack( kmem_cache_t* cp, kmem_stack_t *stack )
{
	cp->free( stack, sizeof(kmem_stack_t) );
}




// --------------------------------------


kmem_cache_t*	kmem_cache_create(
					size_t size,
					size_t align,
					int low_mark,
					int (*constructor)(void *obj, void *private, int kmflag),
					void (*destructor)(void *obj, void *private),
					void *private,
					void* (*alloc)(size_t), 
					void (*free)(void*,size_t),
					int cflags)
{
	kmem_cache_t *kmem = alloc( sizeof(kmem_cache_t) );
	if ( kmem == NULL ) return NULL;

		kmem->size 	= size;
		kmem->align = align;
		kmem->constructor = constructor;
		kmem->destructor = destructor;
		kmem->private = private;
		kmem->alloc = alloc;
		kmem->free = free;
		kmem->cflags = cflags;

		kmem->slabs = NULL;
		kmem->stack = NULL;

		kmem->total_objects = 0;
		kmem->used_objects = 0;

		kmem->low_mark = low_mark;

		kmem->stack = kmem_new_stack( kmem, cflags );
		if ( kmem->stack == NULL )
		{
			free( kmem, sizeof(kmem_cache_t) );
			return NULL;
		}
		
	return kmem;
}


void 	kmem_cache_destroy(kmem_cache_t *cp)
{
	kmem_stack_t*	stack = cp->stack;
	kmem_slab_t*	slab = cp->slabs;

	// Free the slabs and the stacks
	while ( (slab != NULL) && (stack != NULL) )
	{
		kmem_slab_t *tmp_slab = slab->next;
		kmem_stack_t *tmp_stack = stack->next;

		kmem_destroy_slab( cp, slab );
		kmem_destroy_stack( cp, stack );

		slab = tmp_slab;
		stack = tmp_stack;
		
	}

	cp->free( cp, sizeof(kmem_cache_t) );
}


void*	kmem_cache_alloc(kmem_cache_t *cp, int kmflag)
{
	void *obj;

	// Ensure low-water mark
	if ( (cp->total_objects - cp->used_objects) == cp->low_mark )
	{
		static int recursive_note = 0;
			
		if ( recursive_note == 0 )
		{
			recursive_note = 1;
		
			kmem_stack_t *stack = kmem_new_stack( cp, kmflag );
			if ( stack != NULL ) 
			{
				// Insert into the next position, not the first.
				stack->next = cp->stack->next;
				stack->prev = cp->stack;
				
				if ( stack->next != NULL ) stack->next->prev = stack;
	
				cp->stack->next = stack; 
			}

			recursive_note = 0;
		}
	}

	// Short circuit if nothing available.
	if ( cp->total_objects == cp->used_objects ) 
	{
		return NULL;
	}

	// Search for some pages which have objects
	if ( cp->stack->position == 0 )	
	{
		kmem_stack_t *stack = cp->stack->next;

		while (stack != NULL)	// Search for pages
		{
			if ( stack->position != 0 ) break;
			stack = stack->next;
		}
		// stack is gauranteed to be !NULL because there are objects available.
			
		// Unlink this one temporarily 
		if ( stack->next != NULL ) stack->next->prev = stack->prev;
		stack->prev->next = stack->next;
		stack->prev = NULL;

		// Move this page to the front
		cp->stack->prev = stack;
		stack->next = cp->stack;
		cp->stack = stack;
	}

		
	// Take the first available object

	obj = cp->stack->cache[ --(cp->stack->position) ];

	kmem_slab_t *slab = kmem_get_slab( cp, obj );
	slab->usage += 1;

	cp->used_objects += 1;

	return obj;
}


void 	kmem_cache_free(kmem_cache_t *cp, void *obj)
{

	kmem_slab_t *slab = kmem_get_slab( cp, obj );
	slab->usage -= 1;


	if ( cp->stack->position == KMEM_STACKSIZE )
	{
		kmem_stack_t *stack = cp->stack->next;
		while ( stack != NULL )
		{
			if ( stack->position != KMEM_STACKSIZE )
			{
				if ( stack->next != NULL ) stack->next->prev = stack->prev;
				stack->prev->next = stack->next;

				cp->stack->prev = stack;
				stack->next = cp->stack;
				stack->prev = NULL;
				cp->stack = stack;
				break;
			}

			stack = stack->next;
		}

	}
	

	cp->used_objects -= 1;
	cp->stack->cache[ (cp->stack->position)++ ] = obj;

}



// --------------


#ifdef _TEST_
#include <stdio.h>
#include <stdlib.h>

void	kmem_display(kmem_cache_t *cp)
{
	int i = 0;
	kmem_stack_t *stack = cp->stack;
	kmem_slab_t  *slabs = cp->slabs;
		
	printf("kmem_cache_t: %u size with %u align\n", cp->size, cp->align );
	printf("  %i of %i objects in use, %i low mark\n", cp->used_objects, cp->total_objects, cp->low_mark );
	printf("  contructor %p\n", cp->constructor );
	printf("  destructor %p\n", cp->destructor );
	printf("  private %p\n", cp->private );
	printf("  alloc %p\n", cp->alloc );
	printf("  free %p\n", cp->free );
	printf("  cflags %i\n", cp->cflags );

	printf("\n");
	
	i = 0;
	printf("  slabs:\n");
	while (slabs != NULL)
	{
		printf("   ( %5i out of %5i at %p )\n",
						slabs->usage,
						slabs->total,
						slabs->ptr );

		slabs = slabs->next;
		i++;
	}


	i = 0;
	printf("  stack:\n");
	while (stack != NULL)
	{
		printf("   ( %5i out of %5i )\n",
						stack->position,
						KMEM_STACKSIZE );

		stack = stack->next;
		i++;
	}


}


#endif


