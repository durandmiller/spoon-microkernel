#include <stdlib.h>
#include <pthread.h>


#include <support/support.h>



static	pthread_key_t	key;	///< The key for all thread data



/** Initializes the thread data for all library operations 
 *
 * \return EAGAIN	if you should try again later
 * \return ENOMEM	if memory was not available
 * \return 0		on success
 */
int		initialize_thread_data()
{
	return pthread_key_create( &key, NULL );
}


/** Returns a pointer to the thread's specific data. It should
 * be guaranteed to return something. If it returns NULL, then
 * there was no memory available.
 *
 * \return NULL on failure.
 * \return __UNIX_thread_data struct on success.
 */
struct __UNIX_thread_data*	get_thread_data()
{
	void *ptr = pthread_getspecific( key );
	if ( ptr == NULL )
	{
		ptr = malloc( sizeof(struct __UNIX_thread_data) );
		if ( ptr == NULL ) return NULL;

		if ( pthread_setspecific( key, ptr ) != 0 )
		{
			free( ptr );
			return NULL;
		}
	}

	return ptr;
}


