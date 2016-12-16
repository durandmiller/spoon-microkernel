#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "os/os.h"

#include <support/support.h>


int  creat(const char *filename, mode_t mode)
{
	return open( filename, O_CREAT|O_WRONLY|O_TRUNC, S_DEFAULT );
}



#warning set errno correctly for this function
int  open(const char *filename, int flags, ...)
{
	void *data;
	FILE* file;
	int fd;
	va_list	vargs;
	mode_t mode;

	va_start( vargs, flags );
	mode = va_arg( vargs, mode_t );
	va_end( vargs );


	if ( os_open( filename, flags, 
			mode,
			&data, &fd ) == NOTIMPLEMENTED ) 
			os_freakout( "os_open not implemented" );


	if ( fd < 0 ) return -1;

	file = (FILE*)malloc( sizeof(FILE) );
	file->data 	= data;
	file->fd 	= fd;
	file->eof 	= 0;
	file->error 	= 0;
	file->pos	= 0;
	file->mode 	= flags;
	file->pushedBack = EOF;

	if ( support_insert_file( file ) != 0 )
	{
		// Only allow files that we can keep track of.
		close( fd );	// close the stream
		return -1;
	}
	
	return fd;
}



