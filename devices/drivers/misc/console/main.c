#include <smk.h>
#include <devfs.h>



extern int init_puts();
extern void puts( const char *str );


static int	dev_shmem	=	-1;
static char *screen		= 	NULL;


// ---- CONSOLE OPERATIONS -------------------------

int open( void *data, int pid )
{
	return 0;
}

int close( void *data, int pid )
{
	return 0;
}


long long write( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	((char*)buffer)[ bytes - 1 ] = 0;

	puts( (const char*)buffer );
		
	return bytes;
}


int	map( void *data, int pid, unsigned long long position,
				unsigned long long bytes,
				unsigned int flags )
{
	int id;

	if ( position != 0 ) return DEVFSERR_NOT_PAGE_ALIGNED;

	id = smk_grant_shmem( dev_shmem, pid, SHMEM_RDWR );
	if ( id != 0 ) return DEVFSERR_SHMEM_FAILED;

	return dev_shmem;
}


int	unmap( void *data, int pid )
{
	return smk_revoke_shmem( dev_shmem, pid );
}





// -----------------------------



int main( int argc, char *argv[] )
{
	int dev_id = -1;


	// Now request the priviledges
	if ( smk_request_iopriv() != 0 ) 
	{
		devfs_dmesg( "Unable to get io priv\n" );
		return -1;
	}
		
	screen = (char*)smk_mem_map( (void*)0xb8000, 1 );
	if ( screen == NULL ) 
	{
		devfs_dmesg( "unable to memory map screen\n" );
		return -1;
	}

	dev_shmem = smk_create_shmem_direct( 
									(unsigned char*)"device:misc:console", 
									1,
									SHMEM_RDWR,
									0xB8000 );
	if ( dev_shmem < 0 )
	{
		devfs_dmesg("Unable to create shared memory direct: %i\n", dev_shmem );
		return -1;
	}




	struct devfs_hooks *hk = smk_malloc( sizeof(struct devfs_hooks) );
						hk->open 	= open;
						hk->close 	= close;
						hk->write 	= write;
						hk->stream 	= NULL;
						hk->script 	= NULL;
						hk->read 	= NULL;
						hk->map		= map;
						hk->unmap	= unmap;



	if ( init_puts() != 0 )
	{
		devfs_dmesg("Error initializing the screen\n" );
		return -1;
	}

	
	devfs_init();


	dev_id = devfs_register( "misc/console",  
							(80 * 25 * 2), 
							DEVFS_FLAG_OPEN 	| 
							DEVFS_FLAG_WRITE 	| 
							DEVFS_FLAG_READ 	| 
							DEVFS_FLAG_MAP 		| 
							DEVFS_FLAG_UNMAP, 
							hk,
							NULL );


	if ( dev_id < 0 )
		devfs_dmesg("devfs_register failed for device /device/misc/console" );

	smk_go_dormant();
    return 0;
}



