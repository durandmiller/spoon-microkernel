/** \file
 *
 * This is the example NULL device driver which is included as an
 * example on how to write a device driver using libdevfs and devfs.
 *
 */

#include <smk.h>
#include <devfs.h>


/** Standard hook */
int open( void *data, int pid )
{
	return 0;
}

/** Standard hook */
int close( void *data, int pid )
{
	return 0;
}

/** Standard hook */
int stream( void *data, int id )
{
	return 0;
}


/** This is the standard hook function which we register later on. It
 * does nothing.  
 *
 * Typically, the commands must be a NULL terminated string with parameters
 * that your device driver will support. For example:
 *
 * set baud=19200;
 * set name=elvis;
 *
 * ... etc ...
 *
 * You have to either set a response or set it as NULL so that the
 * scripted response is returned.
 */
int script( void *data, int pid, const char *commands, char **response )
{
	return 0;
}

/** Standard hook */
long long read( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	devfs_memset( buffer, 0, bytes );
	return bytes;
}

/** Standard hook */
long long write( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	return bytes;
}



/** You'll notice that the main function just initializes
 * the event loop and registers the device. It then goes to
 * sleep forever because this loop is never expected to return.
 *
 * \todo Change this and make it die one day.
 */
int main( int argc, char *argv[] )
{

	int dev_id = -1;

	struct devfs_hooks *hk = smk_malloc( sizeof(struct devfs_hooks) );
						hk->open 	= open;
						hk->close 	= close;
						hk->stream 	= stream;
						hk->script 	= script;
						hk->read 	= read;
						hk->write 	= write;

	
	devfs_init();


	dev_id = devfs_register( "misc/null",  
							0, 
							DEVFS_FLAG_ALL, 
							hk,
							NULL );


	if ( dev_id < 0 )
		devfs_dmesg("devfs_register failed for device /device/misc/null" );


	smk_go_dormant();
    return 0;
}



