#include <smk.h>
#include <devfs.h>



comms_t* devfs_ev_comms	= NULL;
int devfs_pid 			= -1;


#define DEVFS_TIMEOUT		30000
						// Is 30 seconds too much? Possibly..



/** This is the send function which expects a lot of response from
 * the devfs daemon. Returns 0 if there's a response available.  
 *
 * \note If this returns anything other than 0 (success), you don't need
 * to free *resp
 * 
 * */
int devfs_send_response( struct devfs_packet *req, struct devfs_packet **resp )
{
	int port;
	int duration = DEVFS_TIMEOUT;

	*resp = NULL;

	if ( devfs_pid < 0 )
	{
		devfs_pid = smk_find_process_id( DEVFS_PROCESSNAME );
		if ( devfs_pid < 0 ) return DEVFSERR_DEVFS_MISSING;
	}


	port = smk_new_port( smk_gettid() );
	if ( port < 0 ) return DEVFSERR_IPC;
	

		if ( smk_send_message( port, devfs_pid, 0, req->length, req ) != 0 )
		{
			smk_release_port( port );
			return DEVFSERR_NOCOMMS;
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
			
				*resp = (struct devfs_packet*)smk_malloc( bytes );
				if ( *resp == NULL )
				{
					smk_drop_message();
					smk_release_port( port );
					return DEVFSERR_NOMEM;
				}
					
					
				if ( smk_recv_message( &spid, &sport, &dport, 
										bytes, *resp )  > 0 )
				{
					// No spoofing.
					if ( spid != devfs_pid ) 
					{
						smk_release_port( port );
						smk_free( *resp );
						return DEVFSERR_SPOOFED;
					}
					break;
				}
				else
				{
					smk_release_port( port );
					smk_free( *resp );
					return DEVFSERR_IPC;
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
				return DEVFSERR_TIMEOUT;
			}
		}

	
	smk_release_port( port );
	return 0;
}




/** This function will send the packet to the devfs server and
 * wait for a response. However, this particular function will
 * only read the response for DEVFS_PACKETSIZE amount back into
 * the passed parameter and it will return the status field of 
 * the response as the return code.
 */
int devfs_send( struct devfs_packet *ds )
{
	int duration = DEVFS_TIMEOUT;
	int port;


	if ( devfs_pid < 0 )
	{
		devfs_pid = smk_find_process_id( DEVFS_PROCESSNAME );
		if ( devfs_pid < 0 ) return DEVFSERR_DEVFS_MISSING;
	}

	port = smk_new_port( smk_gettid() );
	if ( port < 0 ) return DEVFSERR_NOCOMMS;
	

		if ( smk_send_message( port, devfs_pid, 0, ds->length, ds ) != 0 )
		{
			smk_release_port( port );
			return DEVFSERR_NOCOMMS;
		}


		while ( 1==1 )
		{
			int spid;
			int sport;
			int dport = port;

			duration -= smk_go_dormant_t( duration );

			if ( smk_recv_message( &spid, &sport, &dport, 
									DEVFS_PACKETSIZE, ds )  > 0 )
			{
				// No spoofing.
				if ( spid != devfs_pid ) 
				{
					smk_release_port( port );
					return DEVFSERR_SPOOFED;
				}

				break;
			}

			// Timeout has occured. Our message took forever.
			if ( duration < 0 )
			{
				smk_release_port( port );
				return DEVFSERR_TIMEOUT;
			}
		}

	
	smk_release_port( port );
	return ds->status;
}

/** This function will send the response to the devfs server. That's all */
int devfs_respond( struct devfs_packet *ds )
{

	if ( devfs_pid < 0 )
	{
		devfs_pid = smk_find_process_id( DEVFS_PROCESSNAME );
		if ( devfs_pid < 0 ) return DEVFSERR_DEVFS_MISSING;
	}


		if ( smk_send_message( comms_get_port( devfs_ev_comms ), 
								devfs_pid, 0, ds->length, ds ) != 0 )
		{
			return DEVFSERR_NOCOMMS;
		}


	return 0;
}




/** Send a simple operation status back to the devfs */
int devfs_send_ack( int req_id, unsigned int status )
{
	if ( devfs_pid < 0 ) return DEVFSERR_DEVFS_MISSING;
		
	if ( smk_send_pulse( 0, devfs_pid, 0, 
							DEVFS_RESPONSE, req_id, status, 0,0,0 ) != 0 )
			return DEVFSERR_NOCOMMS;

	return 0;
}









