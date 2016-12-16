#include <smk.h>
#include <devfs.h>

#include "atapi.h"
#include "common.h"
#include "ops.h"


#define MAX_NODES		4

// My devices ......
struct atapi_node nodes[MAX_NODES];


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
	struct atapi_node *node = (struct atapi_node*)data;

	if ( (node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATA) )
	{
		devfs_dmesg("libdevfs error! ATA device accepted scripting\n" );
	}
	
	return 0;
}

long long read( void *data, int pid, 
		  unsigned long long position, 
		  unsigned long long bytes, void *buffer )
{
	struct atapi_node *node = (struct atapi_node*)data;

	if ( (node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATA) )
	{
		return ata_read( node, position, bytes, buffer );
	}
	else
	{
		return atapi_read( node, position, bytes, buffer );
	}

	return bytes;
}

long long write( void *data, int pid, 
		   unsigned long long position, 
		   unsigned long long bytes, void *buffer )
{
	struct atapi_node *node = (struct atapi_node*)data;

	if ( (node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATA) )
	{
		return ata_write( node, position, bytes, buffer );
	}
	else
	{
		return atapi_write( node, position, bytes, buffer );
	}


	return bytes;
}




// -----------------------------


int check( struct atapi_node *node, unsigned int base, int dev )
{
	int rc = 0;

	init_atapi( node, base, dev );

	smk_disable_interrupts();

		rc = probe( node );

	smk_enable_interrupts();

	return rc;
}



int detect_drives()
{
	int count = 0;
		count += check( &(nodes[0]), BASE1, 0 );
		count += check( &(nodes[1]), BASE1, 1 );
		count += check( &(nodes[2]), BASE2, 0 );
		count += check( &(nodes[3]), BASE2, 1 );
	return count;
}

const char *get_name( struct atapi_node *node, unsigned int *flags )
{
	if ( (node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATA) )
	{
		*flags = DEVFS_FLAG_OPEN | DEVFS_FLAG_WRITE | DEVFS_FLAG_READ;
			
		switch ((node->_base+node->_device))
		{
			case (BASE1+0): return "storage/ata0";
			case (BASE1+1): return "storage/ata1";
			case (BASE2+0): return "storage/ata2";
			case (BASE2+1): return "storage/ata3";
		}
	}

	if ( (node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATAPI) )
	{
		*flags = DEVFS_FLAG_OPEN | DEVFS_FLAG_WRITE | DEVFS_FLAG_READ |
					DEVFS_FLAG_SCRIPT;
			
		switch ((node->_base+node->_device))
		{
			case (BASE1+0): return "storage/atapi0";
			case (BASE1+1): return "storage/atapi1";
			case (BASE2+0): return "storage/atapi2";
			case (BASE2+1): return "storage/atapi3";
		}
	}

	return NULL;
}

int register_devices( struct devfs_hooks *hooks )
{
	int i;
	int dev_id = 0;
		
	for ( i = 0; i < MAX_NODES; i++ )
	{
		unsigned int flags;
		const char *name = get_name( &(nodes[i]), &flags );
		if ( name == NULL ) continue;

#warning size of the device
		dev_id = devfs_register( name,  
									0, 
									flags, 
									hooks,
									&(nodes[i]) );

		if ( dev_id < 0 )
			devfs_dmesg("Failed on device %s\n", name );

	}
	
	return 0;
}


int	my_irq( int irq, void *data )
{
	devfs_dmesg("ata-atapi: spurious IRQ received %i\n", irq );
	smk_ack_irq( irq );
	return 0;
}


int main( int argc, char *argv[] )
{
	int rc = 0;

	// Request super-human privileges from the kernel

	if ( smk_request_iopriv() != 0 ) return -1;

	// Request the ability to handle IRQ's 14 and 15

	rc = smk_request_irq( my_irq, 0xE, "ata_irq", (void*)0 );
	if ( rc < 0 ) return -1;

	rc = smk_request_irq( my_irq, 0xF, "ata_irq", (void*)0 );
	if ( rc < 0 ) 
	{
		smk_release_irq( 0xE );
		return -1;
	}

	// Now run through and attempt to detect the drives
	
	rc = detect_drives();

	if ( rc == 0 )		// No devices!
	{
		smk_release_irq( 0xE );
		smk_release_irq( 0xF );
		return -1;
	}
	


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


	register_devices( hk );

	smk_go_dormant();
    return 0;
}



