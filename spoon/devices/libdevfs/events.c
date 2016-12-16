#include <smk.h>
#include <devfs.h>





static int devfs_loopScript( struct d_entry *dev, struct devfs_packet *ds )
{
	struct devfs_packet *resp = NULL;
	char *response = NULL;
	int rc;

	if ( dev->hooks->script == NULL )
			return devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );
			
		
	rc =  dev->hooks->script( dev->device_data, ds->pid,
									(const char*)DEVFS_PACKETDATA(ds),
									&response );

	if ( rc != 0 ) return devfs_send_ack( ds->req_id, rc );
	if ( response == NULL )
		return devfs_send_ack( ds->req_id, 0 );


	rc = devfs_strlen( response ) + 1; 

	resp = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + rc );
	if ( resp == NULL )
	{
		smk_free( response );
		return devfs_send_ack( ds->req_id, DEVFSERR_NOMEM );
	}


	resp->req_id = ds->req_id;
	resp->operation = DEVFS_RESPONSE;
	resp->status = 0;
	resp->length = DEVFS_PACKETSIZE + rc;
	devfs_memcpy( DEVFS_PACKETDATA( resp ), response, rc );
	
	smk_free( response );


	rc = devfs_respond( resp ); 
	if ( rc != 0 )
	{
		// Ag well. what can you do?
	}


	smk_free( resp );
	return 0;
}
	

static int devfs_loopRead( struct d_entry *dev, struct devfs_packet *ds )
{
	struct devfs_packet *resp = NULL;
	long long rc;

	if ( dev->hooks->read == NULL )
			return devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );


	resp = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + ds->size );
	if ( resp == NULL )
			return devfs_send_ack( ds->req_id, DEVFSERR_NOMEM );
			
		
#warning get the correct position
	rc =  dev->hooks->read( dev->device_data, ds->pid,
							ds->position,
							ds->size,
							DEVFS_PACKETDATA(resp) );

	if ( rc < 0 ) 
	{
		smk_free( resp );
		return devfs_send_ack( ds->req_id, rc );
	}


	resp->req_id = ds->req_id;
	resp->operation = DEVFS_RESPONSE;
	resp->status = 0;
	resp->size = rc;
	resp->length = DEVFS_PACKETSIZE + rc;			// only send what is necessary
	

	rc = devfs_respond( resp ); 
	if ( rc != 0 )
	{
		// Ag well. what can you do?
	}


	smk_free( resp );
	return 0;
}
	

static int devfs_loopWrite( struct d_entry *dev, struct devfs_packet *ds )
{
	struct devfs_packet resp;
	long long rc;

	if ( dev->hooks->write == NULL )
			return devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );


		
#warning get the correct position
	rc =  dev->hooks->write( dev->device_data, ds->pid,
							ds->position,
							ds->size,
							DEVFS_PACKETDATA(ds) );


	resp.req_id = ds->req_id;
	resp.operation = DEVFS_RESPONSE;
	resp.status = 0;
	resp.size = rc;
	resp.length = DEVFS_PACKETSIZE;			// only send what is necessary
	

	rc = devfs_respond( &resp ); 
	if ( rc != 0 )
	{
		// Ag well. what can you do?
	}

	return 0;
}


static int devfs_loopMap( struct d_entry *dev, struct devfs_packet *ds )
{
	struct devfs_packet resp;
	int rc;

	if ( dev->hooks->map == NULL )
			return devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );



#warning get the correct position
	rc =  dev->hooks->map( dev->device_data, ds->pid,
							ds->position,
							ds->size,
							ds->flags);

	resp.req_id = ds->req_id;
	resp.operation = DEVFS_RESPONSE;
	resp.status = rc;
	resp.size = 0;
	resp.length = DEVFS_PACKETSIZE;			// only send what is necessary
	

	rc = devfs_respond( &resp ); 
	if ( rc != 0 )
	{
		// Ag well. what can you do?
	}
	return 0;
}


static int devfs_loopUnmap( struct d_entry *dev, struct devfs_packet *ds )
{
	struct devfs_packet resp;
	int rc;

	if ( dev->hooks->unmap == NULL )
			return devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );



#warning get the correct position
	rc =  dev->hooks->unmap( dev->device_data, ds->pid );

	resp.req_id = ds->req_id;
	resp.operation = DEVFS_RESPONSE;
	resp.status = 0;
	resp.size = 0;
	resp.length = DEVFS_PACKETSIZE;			// only send what is necessary
	

	rc = devfs_respond( &resp ); 
	if ( rc != 0 )
	{
		// Ag well. what can you do?
	}
	return 0;
}







static int devfs_loopStream( struct d_entry *dev, struct devfs_packet *ds )
{
	dev->hooks->stream( dev->device_data, ds->flags );

	if ( ds->flags != 0 )
		dev->streaming = 1;
	else
		dev->streaming = 0;
	
	return 0;
}
	
/** This is the primary event loop. If you're going to register
 * a device, make sure you have initiated the primary event loop
 * so that this can receive all messages from devfs and call
 * the hooks you have implemented. It's static because you
 * have no access to it. 
 */
static int devfs_commsHandler( void *data, int pid, int port, void *packet )
{
	struct devfs_packet *ds = (struct devfs_packet*)packet;
	struct d_entry *dev;

	if ( pid != devfs_pid ) return -1;	// No one except the devfs

	if ( (dev = devfs_getDevice( ds->fd )) == NULL ) // Load the device
	{
		devfs_send_ack( ds->req_id, DEVFSERR_BAD_DEVICE );
		return -1;
	}


	// Now switch the operation to the device.
	switch (ds->operation)
	{
		case DEVFS_STREAM:
				devfs_loopStream( dev, ds );
				break;

		case DEVFS_OPEN:
				if ( dev->hooks->open != NULL )
					dev->hooks->open( dev->device_data, ds->pid ); 
				else
					devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );
				break;

		case DEVFS_CLOSE:
				if ( dev->hooks->close != NULL )
					dev->hooks->close( dev->device_data, ds->pid ); 
				else
					devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );
				break;

		case DEVFS_READ:
				devfs_loopRead( dev, ds );
				break;

		case DEVFS_WRITE:
				devfs_loopWrite( dev, ds );
				break;

		case DEVFS_SCRIPT:
				devfs_loopScript( dev, ds );
				break;

		case DEVFS_MAP:
				devfs_loopMap( dev, ds );
				break;

		case DEVFS_UNMAP:
				devfs_loopUnmap( dev, ds );
				break;

				
		default:
			devfs_send_ack( ds->req_id, DEVFSERR_NOTSUPPORTEDOP );
			break;
	}

	return 0;
}


/** This starts the event loop and returns the tid of the
 * handling thread.  It will spawn a separate thread
 * which is responsible for all communication between
 * the devfs and the device driver. It will determine
 * the operation requested by the devfs and will call
 * the registered hooks (see devfs_register) when 
 * required.
 *
 * Just note that this spawns a separate thread that will
 * do all the handling. So this will return immediately
 * after being called. The event loop runs in the 
 * background.
 *
 * \return The tid of the event thread or a value less
 * than 0 if failure occurred.
 */
int devfs_init()
{
	if ( comms_create( &devfs_ev_comms, NULL ) != 0 )
			return DEVFSERR_NOMEM;

	if ( comms_register( devfs_ev_comms,
		 				COMMS_MESSAGE, COMMS_DEFAULT, 
						devfs_commsHandler ) != 0 )
	{
		comms_destroy( &devfs_ev_comms );
		return DEVFSERR_COMMS_SUBSYSTEM_FAILURE;
	}

	if ( comms_start( devfs_ev_comms ) != 0 )
	{
		comms_destroy( &devfs_ev_comms );
		return DEVFSERR_COMMS_SUBSYSTEM_FAILURE;
	}
	

	return 0;
}


/** This waits on the devfs server. It is written
 * to wait for STANDARD_WAITTIME amount of time
 * before giving up. It's a good way to 
 * pause while the system is starting up to ensure that
 * the devfs is up and running successfully.
 */
int devfs_wait( unsigned int timeout )
{
	int duration = timeout;

	if ( duration == 0 )
				duration = STANDARD_WAITTIME;
		
	if ( devfs_pid < 0 )
	{
		devfs_pid = smk_find_process_id( DEVFS_PROCESSNAME );
		if ( devfs_pid < 0 ) return DEVFSERR_DEVFS_MISSING;
	}


	while ( smk_send_pulse( 0, devfs_pid, 0, 
							0,0,0, 0,0,0 ) != 0 )
	{
		if ( duration < 100 )
			duration -= smk_go_dormant_t( duration );	// Nap
		else
			duration -= smk_go_dormant_t( 100 );
		
		if ( duration <= 0 ) return DEVFSERR_TIMEOUT;
	}

	return 0;
}



