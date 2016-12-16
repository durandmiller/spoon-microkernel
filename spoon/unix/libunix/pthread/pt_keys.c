#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "../os/os.h"




/** The internal structure used to manage pthread_key
 * stuff. It's used as a list which should
 * be linearly sorted by the id.
 */
struct __UNIX_pthread_key
{
	int	id;								///<  The ID of the key
	void (*destruct)(void*);			///<  The destructor of the key
	int	count;							///<  The size of the data array.	
	void **data;						///<  The data array.
};


static struct __UNIX_pthread_key *key_data	= NULL;	///< App keys
static unsigned int	key_count 				= 0;	///< Total number
static unsigned int __UNIX_keylock 			= 0;	///< The lock



int   pthread_key_create(pthread_key_t *key, void (*destruct)(void*))
{
	int i, j;
	int last_check;
	void *tmp;

	if ( destruct != NULL )
		os_freakout( "Need to implement the destruction calls" );

	os_acquire_spinlock( &__UNIX_keylock );

		// See if we can allocate memory.
		tmp = key_data;
		key_data = realloc( key_data,
		                (key_count+1) * sizeof(struct __UNIX_pthread_key) );

		if ( key_data == NULL )
		{
			key_data = tmp;
			os_release_spinlock( &__UNIX_keylock );
			return ENOMEM;
		}

		// Find the sequential position in the list
		last_check = 0;
		for ( i = 0; i < key_count; i++ )
		{
			// Find the first gap in the IDs
			if ( key_data[i].id == last_check )
			{
				last_check += 1;
				continue;
			}

			break;
		}


		// Now move them all up.
		for ( j = key_count; j > i; j-- )
			key_data[j] = key_data[j-1];


		key_data[last_check].id 		= last_check;
		key_data[last_check].destruct 	= destruct;
		key_data[last_check].count		= 0;
		key_data[last_check].data		= NULL;

		// Remember
		key_count += 1;

		*key = last_check;

	os_release_spinlock( &__UNIX_keylock );
	return 0;
}


int   pthread_key_delete(pthread_key_t key)
{
	int i;
	int j;
	void *tmp;

	os_acquire_spinlock( &__UNIX_keylock );

		// Try and find the key.
		for ( i = 0; i < key_count; i++ )
			if ( key_data[i].id == key ) break;

		// Was it there?
		if ( i == key_count )
		{
			os_release_spinlock( &__UNIX_keylock );
			return EINVAL;
		}

		// Free any data, if any.
		if ( key_data[i].count != 0 )
			free( key_data[i].data );

		// Move them all up.
		for ( j = i; j < (key_count-1); j++ )
			key_data[j] = key_data[j+1];

		key_count -= 1;

		if ( key_count == 0 )
		{
			free( key_data );
			key_data = NULL;
		}
		else
		{
			tmp = key_data;
			key_data = realloc( key_data, 
								(key_count * sizeof(struct __UNIX_pthread_key)) );

			if ( key_data == NULL )		// weird situation but harmless kinda
					key_data = tmp;
		}

	os_release_spinlock( &__UNIX_keylock );
	return 0;
}




void* pthread_getspecific(pthread_key_t key)
{
	int i, pos;
	void *tmp;
	pthread_t tid	= pthread_self();


	os_acquire_spinlock( &__UNIX_keylock );


		// Try and find the key.
		for ( i = 0; i < key_count; i++ )
			if ( key_data[i].id == key ) break;

		// Was it there?
		if ( i == key_count )
		{
			os_release_spinlock( &__UNIX_keylock );
			return NULL;
		}

		// We have the right one.
		pos = i;


		// Ensure there
		if ( key_data[pos].count <= tid )
		{
			os_release_spinlock( &__UNIX_keylock );
			return NULL;
		}

		tmp = key_data[pos].data[tid];


	os_release_spinlock( &__UNIX_keylock );
	return tmp;
}


int pthread_setspecific(pthread_key_t key, const void *value)
{
	int i, pos;
	void *tmp;
	pthread_t tid	= pthread_self();


	os_acquire_spinlock( &__UNIX_keylock );


		// Try and find the key.
		for ( i = 0; i < key_count; i++ )
			if ( key_data[i].id == key ) break;

		// Was it there?
		if ( i == key_count )
		{
			os_release_spinlock( &__UNIX_keylock );
			return EINVAL;
		}

		// We have the right one.
		pos = i;


		// Ensure there
		if ( key_data[pos].count <= tid )
		{
			tmp = key_data[pos].data;
			key_data[pos].data = realloc( key_data[pos].data, 
											(tid+1) * sizeof(void*) );

			// Out of memory
			if ( key_data[pos].data == NULL )
			{
				key_data[pos].data = tmp;
				os_release_spinlock( &__UNIX_keylock );
				return ENOMEM;
			}	


			// Set the values to NULL
			for ( i = key_data[pos].count; i < (tid+1); i++ )
				key_data[pos].data[i] = NULL;

			key_data[pos].count = (tid+1);
		}

		// We're good.
		key_data[pos].data[tid] = (void*)value;

	os_release_spinlock( &__UNIX_keylock );
	return 0;
}




#include <stdio.h>

void 	pthread_dump()
{
	int i;


	os_acquire_spinlock( &__UNIX_keylock );

		printf("There are %i keys\n", key_count );

		for ( i = 0; i < key_count; i++ )
		{
			printf("  %5i.  %x  %i values\n", 
						key_data[i].id,
						key_data[i].destruct,
						key_data[i].count
					);
		}


	os_release_spinlock( &__UNIX_keylock );
}

