#ifndef _SUPPORT_SUPPORT_H
#define _SUPPORT_SUPPORT_H		1


#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


// ---------- Thread Safety support --------


/** This is the structure which each thread will have allocated 
 * to itself upon initialization. It includes things such as errno,
 * static buffers, etc.
 */
struct __UNIX_thread_data
{
	int error_number;			///<  Otherwise known as errno
};



int		initialize_thread_data();
struct __UNIX_thread_data*	get_thread_data();




// ---------- File support -----------------
int   support_init_files( int count );
void  support_shutdown_files(void);
FILE* support_retrieve_file( int fd );
FILE* support_remove_file( int fd );
int   support_insert_file( FILE* stream );

unsigned int support_fmodes2flags( const char *mode );


#ifdef __cplusplus
}
#endif

#endif
   
