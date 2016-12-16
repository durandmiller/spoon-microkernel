#include <smk.h>
#include <devfs.h>


#include <comms.h>

#include "devices.h"
#include "processes.h"
#include "files.h"
#include "devcomms.h"
#include "state.h"
#include "response.h"

#warning shutdown everything correctly.



int	message_handler( void *data, int pid, int port, void *packet )
{
	struct devfs_packet *ds = (struct devfs_packet*)packet;

	ds->pid = pid;

	switch (ds->operation)
	{
		case DEVFS_REGISTER:	return register_device( pid, port, ds );
		case DEVFS_DEREGISTER: 	return deregister_device( pid, port, ds );
		case DEVFS_OPEN: 		return open( pid, port, ds );
		case DEVFS_CLOSE:		return close( pid, port, ds );
		case DEVFS_RESPONSE: 	return process_response( pid, port, ds );
		case DEVFS_SCRIPT:
		case DEVFS_READ:
		case DEVFS_WRITE:
		case DEVFS_MAP:
		case DEVFS_UNMAP: 
				return dispatch_request( pid, port, ds );
		case DEVFS_STREAM: 	return stream( pid, port, ds );
		case DEVFS_STAT: 	return stat( pid, port, ds );
		case DEVFS_MATCH: 	return match( pid, port, ds );
		case DEVFS_PUSH: 	return dispatch_push( pid, port, ds );
	}

	return -1;
}


int	pulse_handler( void *data, int pid, int port, void *packet )
{
	unsigned int *info = (unsigned int*)packet;

	switch (info[0])
	{
		case DEVFS_RESPONSE: return process_response_pulse( pid, port, packet );
		case DEVFS_PUSH: return dispatch_pulse_push( pid, port, packet );
	}

	return -1;
}



int main( int argc, char *argv[] )
{
	comms_t		*comms = NULL;

	if ( init_states() != 0 )
		return -1;

	if ( init_devices() != 0 ) 
		return -1;

	if ( init_processes() != 0 ) 
		return -1;

	if ( comms_create( &comms, NULL ) != 0 )
		return -1;


	comms_register( comms, COMMS_MESSAGE, COMMS_DEFAULT, message_handler );
	comms_register( comms, COMMS_PULSE, COMMS_DEFAULT, pulse_handler );


	comms_start( comms );
	comms_wait( comms, 0 );

#warning Should be comms wait.
	smk_go_dormant();


	comms_destroy( &comms );
	return 0;
}



