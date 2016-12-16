#include <smk.h>
#include <devfs.h>


int 		dev_grub = -1;
int			dev_shmem = -1;

uint32_t	*ptr;
uint32_t	*screen;
uint32_t	width;
uint32_t	height;
long long	size;




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
	long long available = bytes;

	if ( (position + bytes) >= size )
		available = (size - position);

	devfs_memcpy( buffer, ptr + position, available );

	return available;
}

long long write( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	long long available = bytes;

	if ( (position + bytes) >= size )
		available = (size - position);

	devfs_memcpy( ptr + position, buffer, available );

	return available;
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
	int rc = smk_revoke_shmem( dev_shmem, pid );
	return rc;
}




// -----------------------------
void showme( uint32_t color, int error )
{
	static uint32_t* 	ptr = NULL;
	static uint32_t 	width = 0;
	static uint32_t 	height = 0;
	static uint32_t 	size = 0;

	int i,j;

	if ( ptr == NULL )
	{
		smk_get_lfb( &ptr, &width, &height );
		size = width * height * 4;
		smk_request_iopriv();
		ptr = (uint32_t*)smk_mem_map( 
							(void*)ptr, 
							(size / 4096) + 1 );
	}

	for ( i = 0; i < (width * height); i++ )
		ptr[ i ] = color;

	if ( error < 0 ) error = -error;

	for ( i = 0; i < error; i++ )
	 for ( j = 0; j < 300; j++ )
		ptr[ (i*5) * width + j ] = ~color;
}





int init_devices()
{
	int pages;

	smk_request_iopriv();

	smk_get_lfb( &ptr, &width, &height );
	if ( ptr == NULL ) 
	{
		devfs_dmesg("vbe mode not present in the kernel" );
		return -1;
	}

	size = width * height * 4;

	// Try and map the screen into our space.
	pages = (size / 4096) + 1;	//pages

	screen = (uint32_t*)smk_mem_map( (void*)ptr, pages );
	if ( screen == NULL )
	{
		devfs_dmesg("unable to map memory" );
		return -1;
	}


	// Create the shared memory structure
	dev_shmem = smk_create_shmem_direct( (unsigned char*)"device:video:grub", 
											pages,
											SHMEM_RDWR,
											(uintptr_t)ptr );
	/*
	ptr = NULL;
	width = 1024;
	height = 768;
	pages = (width * height * 4) / 4096 + 1;
	dev_shmem = smk_create_shmem( (unsigned char*)"device:video:grub", 
											pages,
											SHMEM_RDWR );
	*/
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

	dev_grub = devfs_register( "video/grub",  
						(long long)(size), 
						DEVFS_FLAG_ALL,
						hk,
						(void*)0 );

	if ( dev_grub < 0 )
	{
		devfs_dmesg("devfs_register failed for device /device/video/grub" );
		smk_free( hk );
		return -1;
	}

    return 0;
}



