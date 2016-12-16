#include <smk.h>
#include <devfs.h>


int dev_kbd = -1;
int dev_mouse = -1;


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




// -----------------------------



int init_devices()
{
	struct devfs_hooks *hk = smk_malloc( sizeof(struct devfs_hooks) );
						hk->open 	= open;
						hk->close 	= close;
						hk->stream 	= stream;
						hk->script 	= script;
						hk->read 	= read;
						hk->write 	= write;
						hk->map		= NULL;
						hk->unmap	= NULL;

	
	devfs_init();


	dev_kbd = devfs_register( "input/kbd",  
								0, 
								DEVFS_FLAG_OPEN | DEVFS_FLAG_STREAM, 
								hk,
								(void*)0 );

	if ( dev_kbd < 0 )
		devfs_dmesg("devfs_register failed for device /device/input/kbd" );


	dev_mouse = devfs_register( "input/ps2mouse",  
								0, 
								DEVFS_FLAG_OPEN | DEVFS_FLAG_STREAM, 
								hk,
								(void*)1 );

	if ( dev_mouse < 0 )
		devfs_dmesg("devfs_register failed for device /device/input/mouse" );



	smk_go_dormant();
    return 0;
}



