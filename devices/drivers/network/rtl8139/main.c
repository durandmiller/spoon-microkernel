#include <smk.h>
#include <devfs.h>

#include "rtl8139.h"


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
	return 0;
}

long long write( void *data, int pid, unsigned long long position, 
				unsigned long long bytes, void *buffer )
{
	return 0;
}


// -------------------------------------------------------------


static const char *get_name( int number )
{
	static char *name = "network/rtl8139_00";
	int len = devfs_strlen( name );

	if ( number > 99 ) return NULL;

	if ( number < 10 )
	{
		name[ len - 2 ] = '0' + number;
		name[ len - 1 ] = 0;
		return name;
	}
	
	name[ len - 2 ] = '0' + (number / 10);
	name[ len - 1 ] = '0' + (number % 10);
	return name;
}


int process_card( struct pci_cfg *cfg, struct devfs_hooks *hk )
{
	int i;
	const char *name = NULL;

	
	i = rtl8139_init( cfg->base[0], cfg->irq );
	if ( i < 0 ) return -1;

	name = get_name(i);
	if ( name == NULL ) return -1;
	
	if ( devfs_register( name,  
							0, 
							DEVFS_FLAG_OPEN | DEVFS_FLAG_WRITE |
							DEVFS_FLAG_STREAM | DEVFS_FLAG_SCRIPT,
							hk,
							(void*)i ) < 0 )
	{
		devfs_dmesg("devfs_register failed for device %s", name );
	}

	
	return 0;
}


/*
	0x1259	0x2503	network/rtl8139
	0x1743	0x8139	network/rtl8139
	0x14EA	0xAB06	network/rtl8139
	0x1039	0x8139	network/rtl8139
	0x1065	0x8139	network/rtl8139
	0x10EC	0x8138	network/rtl8139
	0x10EC	0x8139	network/rtl8139
*/



int main( int argc, char *argv[] )
{
	struct pci_cfg cfg;
	int i;

	if ( smk_request_iopriv() != 0 ) return -1;



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


	#define	CHECK_CARD( vendor_id, device_id )			\
		i = 0; 											\
		while ( smk_search_pci( vendor_id, device_id, i++, &cfg ) == 0 )	\
			process_card( &cfg, hk );

	CHECK_CARD( 0x1259, 0x2503 );
	CHECK_CARD( 0x1743, 0x8139 );
	CHECK_CARD( 0x14EA, 0xAB06 );
	CHECK_CARD( 0x1039, 0x8139 );
	CHECK_CARD( 0x1065, 0x8139 );
	CHECK_CARD( 0x10EC, 0x8139 );


	
	smk_go_dormant();

    return 0;
}



