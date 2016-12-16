#include <smk.h>
#include <devfs.h>


int	dev_shmem		=	-1;

// -------------------------------------------------
int open( void *data, int pid )
{
	return 0;
}

int close( void *data, int pid )
{
	return 0;
}

int stream( void *data, int id )
{
	return 0;
}

int script( void *data, int pid, const char *commands, char **response )
{
	return 0;
}

long long read( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	devfs_memset( buffer, 0, bytes );
	return bytes;
}

long long write( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	return bytes;
}

int	map( void *data, int pid, unsigned long long position,
				unsigned long long bytes,
				unsigned int flags )
{
	int id;

	// short circuit
#warning Create a table lookup for all responses. So that unix is simulated nicely.
	return -1;

	if ( position != 0 ) return DEVFSERR_NOT_PAGE_ALIGNED;

	dev_shmem = smk_create_shmem( (unsigned char*)"device:misc:null", 
									16,
									SHMEM_RDWR );


	id = smk_grant_shmem( dev_shmem, pid, SHMEM_RDWR );
	if ( id != 0 ) return DEVFSERR_SHMEM_FAILED;

	return dev_shmem;
}


int	unmap( void *data, int pid )
{
	return -1;
}






// -----------------------------



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
						hk->map		= NULL /*map*/;
						hk->unmap	= NULL /*unmap*/;

	
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



