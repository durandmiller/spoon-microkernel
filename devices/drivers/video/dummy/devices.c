#include <smk.h>
#include <devfs.h>


int 		dev_dummy = -1;
int			dev_shmem = -1;





// -------------------------------------------------
int open( void *data, int pid )
{
	devfs_dmesg( "dummy opened by %i\n", pid );
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
	return 0;
}

long long write( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	return 0;
}



int	map( void *data, int pid, unsigned long long position,
				unsigned long long bytes,
				unsigned int flags )
{
	int id;

	devfs_dmesg( "dummy map attempt by %i, %i, %i, %i\n", 
						pid, (int)position, (int)bytes, (int)flags );

	if ( position != 0 ) return DEVFSERR_NOT_PAGE_ALIGNED;

	id = smk_grant_shmem( dev_shmem, pid, SHMEM_RDWR );
	if ( id != 0 ) return DEVFSERR_SHMEM_FAILED;


	devfs_dmesg( "dummy map completed with %i\n", dev_shmem );

	return dev_shmem;
}


int	unmap( void *data, int pid )
{
	int rc = smk_revoke_shmem( dev_shmem, pid );
	return rc;
}





int init_devices()
{
	int size = 1024 * 768 * 4;
	int pages = (size / 4096) + 1;


	// Create the shared memory structure
	dev_shmem = smk_create_shmem( (unsigned char*)"device:video:dummy", 
											pages,
											SHMEM_RDWR );

	if ( dev_shmem < 0 )
	{
		devfs_dmesg("Failed to create shared memory region: %i\n", dev_shmem );
		return DEVFSERR_SHMEM_FAILED;
	}


	// Standard device initialization
	
	struct devfs_hooks *hk = smk_malloc( sizeof(struct devfs_hooks) );
						hk->open 	= open;
						hk->close 	= close;
						hk->stream 	= stream;
						hk->script 	= script;
						hk->read 	= read;
						hk->write 	= write;
						hk->map		= map;
						hk->unmap	= unmap;

	devfs_init();

	dev_dummy = devfs_register( "video/dummy",  
						(long long)(size), 
						DEVFS_FLAG_ALL,
						hk,
						(void*)0 );

	if ( dev_dummy < 0 )
	{
		devfs_dmesg("devfs_register failed for device /device/video/dummy" );
		smk_free( hk );
		return -1;
	}

    return 0;
}



