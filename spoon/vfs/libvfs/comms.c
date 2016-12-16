#include <smk.h>
#include <devfs.h>
#include <vfs.h>


int vfs_pid = -1;


#define VFS_TIMEOUT		30000
						// Is 30 seconds too much? Possibly..



/** This is the send function which expects a lot of response from
 * the vfs daemon. Returns 0 if there's a response available.  
 *
 * \note If this returns anything other than 0 (success), you don't need
 * to free *resp
 * 
 * */
int vfs_send_response( struct vfs_packet *req, struct vfs_packet **resp )
{
	int port;
	int duration = VFS_TIMEOUT;

	*resp = NULL;

	if ( vfs_pid < 0 )
	{
		vfs_pid = smk_find_process_id( "vfs" );
		if ( vfs_pid < 0 ) return VFSERR_VFS_MISSING;
	}


	port = smk_new_port( smk_gettid() );
	if ( port < 0 ) return VFSERR_IPC;
	

		if ( smk_send_message( port, vfs_pid, 0, req->length, req ) != 0 )
		{
			smk_release_port( port );
			return VFSERR_NOCOMMS;
		}

		
		while ( 1==1 )
		{
			int spid;
			int sport;
			int dport = port;
			int bytes;


			duration -= smk_go_dormant_t( duration );


			if ( smk_peek_message( &spid, &sport, &dport, &bytes ) == 0 )
			{
			
				*resp = (struct vfs_packet*)smk_malloc( bytes );
				if ( *resp == NULL )
				{
					smk_drop_message();
					smk_release_port( port );
					return VFSERR_NOMEM;
				}
					
					
				if ( smk_recv_message( &spid, &sport, &dport, 
										bytes, *resp )  > 0 )
				{
					// No spoofing.
					if ( spid != vfs_pid ) 
					{
						smk_release_port( port );
						smk_free( *resp );
						return VFSERR_SPOOFED;
					}
					break;
				}
				else
				{
					smk_release_port( port );
					smk_free( *resp );
					return VFSERR_IPC;
				}
			}
			else
			{
				smk_drop_pulse();
			}
			

			// Timeout has occured. Our message took forever.
			if ( duration < 0 )
			{
				smk_release_port( port );
				return VFSERR_TIMEOUT;
			}
		}

	
	smk_release_port( port );
	return 0;
}




/** This function will send the packet to the vfs server and
 * wait for a response. However, this particular function will
 * only read the response for VFS_PACKETSIZE amount back into
 * the passed parameter and it will return the status field of 
 * the response as the return code.
 */
int vfs_send( struct vfs_packet *ds )
{
	int duration = VFS_TIMEOUT;
	int port;


	if ( vfs_pid < 0 )
	{
		vfs_pid = smk_find_process_id( "vfs" );
		if ( vfs_pid < 0 ) return VFSERR_VFS_MISSING;
	}

	port = smk_new_port( smk_gettid() );
	if ( port < 0 ) return VFSERR_NOCOMMS;
	

		if ( smk_send_message( port, vfs_pid, 0, ds->length, ds ) != 0 )
		{
			smk_release_port( port );
			return VFSERR_NOCOMMS;
		}


		while ( 1==1 )
		{
			int spid;
			int sport;
			int dport = port;

			duration -= smk_go_dormant_t( duration );

			if ( smk_recv_message( &spid, &sport, &dport, 
									VFS_PACKETSIZE, ds )  > 0 )
			{
				// No spoofing.
				if ( spid != vfs_pid ) 
				{
					smk_release_port( port );
					return VFSERR_SPOOFED;
				}

				break;
			}

			// Timeout has occured. Our message took forever.
			if ( duration < 0 )
			{
				smk_release_port( port );
				return VFSERR_TIMEOUT;
			}
		}

	
	smk_release_port( port );
	return ds->status;
}

/** This waits on the vfs server. It is written
 * to wait for STANDARD_WAITTIME amount of time
 * before giving up. It's a good way to 
 * pause while the system is starting up to ensure that
 * the vfs is up and running successfully.
 */
int vfs_wait( unsigned int timeout )
{
	int duration = timeout;

	if ( duration == 0 )
				duration = STANDARD_WAITTIME;
		
	if ( vfs_pid < 0 )
	{
		vfs_pid = smk_find_process_id( "vfs" );
		if ( vfs_pid < 0 ) return VFSERR_VFS_MISSING;
	}


	while ( smk_send_pulse( 0, vfs_pid, 0, 
							0,0,0, 0,0,0 ) != 0 )
	{
		if ( duration < 100 )
			duration -= smk_go_dormant_t( duration );	// Nap
		else
			duration -= smk_go_dormant_t( 100 );
		
		if ( duration <= 0 ) return VFSERR_TIMEOUT;
	}

	return 0;
}













