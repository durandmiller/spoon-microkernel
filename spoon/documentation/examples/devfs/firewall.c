/**  \file
 *
 * This is an userland application which demonstrates the 
 * use of devfs. It's a simple network packet repeater (is there a real
 * name for that?).
 *
 * Basically, all packets coming in on network card 1 will be sent out
 * on network card 2. And vice-versa. If you want to put a firewall in 
 * place, just add some logic to analyze the packet before it gets
 * sent out on the other network card.
 *
 * I think it's a nice example that demonstrates the power of the
 * devfs and libdevfs library.
 * 
 */


#include <smk.h>
#include <devfs.h>


// Here are the network card device descriptors
int 	networdCard_1 	=	-1;
int 	networdCard_2 	=	-1;


// Here's the streaming hook.  
void stream_network( int fd, void *data, long long len )
{
	int target;
	long long bytes;
		
	if ( fd == networkCard_1 )
			target = networkCard_2;
	else
			target = networkCard_1;

	// Add some firewall logic here, maybe?

	bytes = devfs_write( target, data, len );

	if ( bytes != len )
		devfs_dmesg("WARNING! Unable to repeat network packet\n" );
}


int main( int argc, char *argv[])
{
	int i;

	// Try and open network card 1
	networkCard_1 = devfs_open( "network/rtl8139_0", DEVFS_FLAG_STREAM );
	if ( networkCard_1 < 0 )
	{
		devfs_dmesg( "Unable to open network/rtl8139_0 with error: %s\n",
						devfs_err(networkCard_1) );
		return -1;
	}
	
	// Try and open network card 2
	networkCard_2 = devfs_open( "network/rtl8139_1", DEVFS_FLAG_STREAM );
	if ( networkCard_2 < 0 )
	{
		devfs_dmesg( "Unable to open network/rtl8139_1 with error: %s\n",
						devfs_err(networkCard_2) );
		return -1;
	}
	
	
	// Set streaming on them all.
	devfs_set_streaming( networkCard_1, 1, stream_network );
	devfs_set_streaming( networkCard_2, 1, stream_network );

	smk_go_dormant();
	return 0;
}



