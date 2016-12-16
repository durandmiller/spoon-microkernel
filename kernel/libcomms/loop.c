#include <smk.h>
#include <comms.h>



static	int	send_request( comms_t *comms, 
							int pid, int port, 
							unsigned int operation, 
							int type, void *packet )
{
	int i;
	int rc = -1;

	acquire_spinlock( &(comms->lock) );

		for ( i = 0; i < comms->count; i++ )
		{
			if ( ((comms->handlers[i].operation == operation) ||
				  (comms->handlers[i].operation == COMMS_DEFAULT)) 
					&&
					(comms->handlers[i].type == type)  )
			{
				comms->handlers[i].handler( comms->data, pid, port, packet );
				
				rc = 0;
			}
		}

	release_spinlock( &(comms->lock) );
	return rc;
}




int		comms_loop( void * data )	
{
	comms_t	*comms		= (comms_t*)data;
	void *incoming_data = NULL;
	unsigned int running = 1;
	unsigned int pulse_data[6];

	while ( running == 1 )  
	{
		int spid, sport, dport = -1;
		int bytes;
		unsigned int *operation;
		
		smk_go_dormant();


		if ( smk_peek_message( &spid, &sport, &dport, &bytes ) == 0 )
		{
			if ( bytes > COMMS_MAX_PACKETSIZE )		// Too big?
			{
				smk_drop_message();
				continue;
			}

			incoming_data = smk_realloc( incoming_data, bytes );
			if ( incoming_data == NULL ) 
			{
				smk_drop_message();
				continue;
			}

			smk_recv_message( &spid, &sport, &dport, bytes, incoming_data );

			operation = incoming_data;

			send_request( comms, 
							spid, sport, 
							*operation, COMMS_MESSAGE, incoming_data );

			continue;
		}



		if ( smk_recv_pulse( &spid, &sport, &dport,
							(pulse_data + 0), 
							(pulse_data + 1), 
							(pulse_data + 2),
							(pulse_data + 3), 
							(pulse_data + 4), 
							(pulse_data + 5) ) != 0 ) continue;

		switch (pulse_data[0])
		{
			case COMMS_PING: break;

			case COMMS_TERMINATE:
				if ( spid == smk_getpid() ) running = 0;
				break;

			default:
				send_request( comms, 
								spid, sport, 
								pulse_data[0], COMMS_PULSE, pulse_data );	
				break;
		};
	}

		
	smk_release_port( comms->port );

	if ( incoming_data != NULL ) smk_free( incoming_data );
	return 0;
}





