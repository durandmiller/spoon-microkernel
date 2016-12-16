#include <smk.h>
#include <vfs.h>



int send_packet( int pid, int port, struct vfs_packet *vs )
{
	int rc = smk_send_message( 0, pid, port, vs->length, vs );


	if ( rc != 0 ) return -1;
	return 0;
}

int send_resp( int pid, int port, struct vfs_packet *vs, int stat )
{
	int rc;
	
	vs->length = VFS_PACKETSIZE;
	vs->status = stat;
		
	rc =  smk_send_message( 0, pid, port, VFS_PACKETSIZE, vs );

	if ( rc != 0 )
	{
		return rc;
	}

	return stat;
}





