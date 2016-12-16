#include <smk.h>
#include <devfs.h>



int send_packet( int pid, int port, struct devfs_packet *ds )
{
	int rc = smk_send_message( 0, pid, port, ds->length, ds );


	if ( rc != 0 ) return -1;
	return 0;
}

int send_resp( int pid, int port, struct devfs_packet *ds, int stat )
{
	int rc;
	
	ds->length = DEVFS_PACKETSIZE;
	ds->status = stat;
		

	rc =  smk_send_message( 0, pid, port, DEVFS_PACKETSIZE, ds );

	if ( rc != 0 )
	{
		return rc;
	}

	return stat;
}





