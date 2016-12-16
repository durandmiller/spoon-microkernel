#include <smk.h>
#include <devfs.h>



#include "devcomms.h"
#include "state.h"



/** This function returns the response to the original requester
 */
int process_response( int pid, int port, void* dsu )
{
	struct devfs_packet* ds = (struct devfs_packet*)dsu;
	int sourcePid;
	int sourcePort;


	int rc = remove_state( ds->req_id, pid, &sourcePid, &sourcePort );

	if ( rc != 0 )
	{
#warning inform the sender of timeout or error?
		return -1;
	}


	ds->port = 0;

	// Send the packet home...

	rc = send_packet( sourcePid, sourcePort, ds );
	if ( rc != 0 ) return DEVFSERR_NOCOMMS;


	return 0;
}



int process_response_pulse( int pid, int port, void *dsu )
{
	int *data = (int*)dsu;
	int req_id = data[1];
	int status = data[2];

	struct devfs_packet ds;
	int sourcePid;
	int sourcePort;


	int rc = remove_state( req_id, pid, &sourcePid, &sourcePort );

	if ( rc != 0 )
	{
#warning inform the sender of timeout or error?
		return -1;
	}


	ds.length = DEVFS_PACKETSIZE;
	ds.status = status;
	ds.port = 0;

	// Send the packet home...

	rc = send_packet( sourcePid, sourcePort, &ds );
	if ( rc != 0 ) return DEVFSERR_NOCOMMS;

	return 0;
}


