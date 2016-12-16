#include <smk.h>
#include <gui.h>


int gui_pid = -1;


#define GUI_TIMEOUT		30000
						// Is 30 seconds too much? Possibly..



/** This is the send function which expects a lot of response from
 * the gui daemon. Returns 0 if there's a response available.  
 *
 * \note If this returns anything other than 0 (success), you don't need
 * to smk_free *resp
 * 
 * */
int gui_send_response( struct gui_packet *req, struct gui_packet **resp )
{
	int port;
	int duration = GUI_TIMEOUT;

	*resp = NULL;

	if ( gui_pid < 0 )
	{
		gui_pid = smk_find_process_id( GUI_PROCESSNAME );
		if ( gui_pid < 0 ) return GUIERR_GUI_MISSING;
	}


	port = smk_new_port( smk_gettid() );
	if ( port < 0 ) return GUIERR_IPC;
	

		if ( smk_send_message( port, gui_pid, 0, req->length, req ) != 0 )
		{
			smk_release_port( port );
			return GUIERR_NOCOMMS;
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
			
				*resp = (struct gui_packet*)smk_malloc( bytes );
				if ( *resp == NULL )
				{
					smk_drop_message();
					smk_release_port( port );
					return GUIERR_NOMEM;
				}
					
					
				if ( smk_recv_message( &spid, &sport, &dport, 
										bytes, *resp )  > 0 )
				{
					// No spoofing.
					if ( spid != gui_pid ) 
					{
						smk_release_port( port );
						smk_free( *resp );
						return GUIERR_SPOOFED;
					}
					break;
				}
				else
				{
					smk_release_port( port );
					smk_free( *resp );
					return GUIERR_IPC;
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
				return GUIERR_TIMEOUT;
			}
		}

	
	smk_release_port( port );
	return 0;
}




/** This function will send the packet to the gui server and
 * wait for a response. However, this particular function will
 * only read the response for GUI_PACKETSIZE amount back into
 * the passed parameter and it will return the status field of 
 * the response as the return code.
 */
int gui_send( struct gui_packet *ds )
{
	int duration = GUI_TIMEOUT;
	int port;


	if ( gui_pid < 0 )
	{
		gui_pid = smk_find_process_id( GUI_PROCESSNAME );
		if ( gui_pid < 0 ) return GUIERR_GUI_MISSING;
	}

	port = smk_new_port( smk_gettid() );
	if ( port < 0 ) return GUIERR_NOCOMMS;
	

		if ( smk_send_message( port, gui_pid, 0, ds->length, ds ) != 0 )
		{
			smk_release_port( port );
			return GUIERR_NOCOMMS;
		}


		while ( 1==1 )
		{
			int spid;
			int sport;
			int dport = port;

			duration -= smk_go_dormant_t( duration );

			if ( smk_recv_message( &spid, &sport, &dport, 
									GUI_PACKETSIZE, ds )  > 0 )
			{
				// No spoofing.
				if ( spid != gui_pid ) 
				{
					smk_release_port( port );
					return GUIERR_SPOOFED;
				}

				break;
			}

			// Timeout has occured. Our message took forever.
			if ( duration < 0 )
			{
				smk_release_port( port );
				return GUIERR_TIMEOUT;
			}
		}

	
	smk_release_port( port );
	return ds->status;
}



/** Send a simple operation status back to the gui */
int gui_send_ack( int req_id, unsigned int status )
{
	if ( gui_pid < 0 ) return GUIERR_GUI_MISSING;
		
	if ( smk_send_pulse( 0, gui_pid, 0, 
							GUI_RESPONSE, req_id, status, 0,0,0 ) != 0 )
			return GUIERR_NOCOMMS;

	return 0;
}









