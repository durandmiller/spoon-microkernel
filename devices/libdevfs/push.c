#include <smk.h>
#include <devfs.h>



// -------------- INTERNAL LIBRARY CONTROL ---------



static struct d_stream *list = NULL;
static int list_size = 0;


/** Returns a device from the list */
struct d_stream* devfs_getStream( int fd )
{
	int i;

	for ( i = 0; i < list_size; i++ )
		if ( list[i].fd == fd ) return &(list[i]);

	return NULL;
}


/** Adds a device in the statically declared local list */
int devfs_addStream( int fd, void (*func)(int,void*,long long) )
{
	list = (struct d_stream*)smk_realloc( list, 
								sizeof(struct d_stream) * (list_size+1) );
	if ( list == NULL ) return DEVFSERR_NOMEM;
	
	list[ list_size ].fd = fd;
	list[ list_size ].func = func;
	list_size += 1;
	return 0;
}

int devfs_removeStream( int fd )
{
	int i,j;

	for ( i = 0; i < list_size; i++ )
		if ( list[i].fd == fd ) break;

	if ( i == list_size ) return DEVFSERR_NODEV_LOCAL;

	for ( j = i; j < (list_size-1); j++ )
		list[j] = list[j+1];


	list_size -= 1;
	return 0;
}


// -------------- CLIENT SIDE ---------------------- 



comms_t*	devfs_st_comms = NULL;


static int	devfs_streamMessageLoop( void *data, 
									 int pid, int port, void *packet )
{
	struct devfs_packet *ds = (struct devfs_packet*)packet;
	struct d_stream *dev;

	if ( pid != devfs_pid ) return -1;

	if ( (dev = devfs_getStream( ds->fd )) == NULL ) // Load the device
			return -1;

	if ( dev->func == NULL ) return -1;

	// Now switch the operation to the listening function.
	switch (ds->operation)
	{
		case DEVFS_PUSH:
				dev->func( ds->fd, DEVFS_PACKETDATA(ds), ds->size ); 
				break;
	}


	return 0;
}

static int	devfs_streamPulseLoop( void *data, 
								   int pid, int port, void *packet )
{
	int *pulse_data = (int*)packet;
	struct d_stream *dev;
	int fixed_data[6];

	if ( pid != devfs_pid ) return -1;

	if ( pulse_data[0] != DEVFS_PUSH ) 
			return -1;

	if ( (dev = devfs_getStream( pulse_data[1] ) ) == NULL )
			return -1;

	if ( dev->func == NULL ) 
			return -1;

	// Quickly reorder
	fixed_data[5] = pulse_data[0];
	fixed_data[4] = pulse_data[1];
	fixed_data[0] = pulse_data[2];
	fixed_data[1] = pulse_data[3];
	fixed_data[2] = pulse_data[4];
	fixed_data[3] = pulse_data[5];

	dev->func( fixed_data[4], fixed_data, sizeof(int)*4 );

	return 0;
}


/** This function initialized the streaming loop. The loop listens
 * for all incoming push messages that arrive from the devfs.
 */
int devfs_streamInit()
{
	if ( comms_create( &devfs_st_comms, NULL ) != 0 )
			return DEVFSERR_COMMS_SUBSYSTEM_FAILURE;


	if ( comms_register( devfs_st_comms,
							COMMS_MESSAGE, COMMS_DEFAULT,
								devfs_streamMessageLoop ) != 0 )
			{
				comms_destroy( &devfs_st_comms );
				return DEVFSERR_COMMS_SUBSYSTEM_FAILURE;
			}

	if ( comms_register( devfs_st_comms,
							COMMS_PULSE, COMMS_DEFAULT,
								devfs_streamPulseLoop ) != 0 )
			{
				comms_destroy( &devfs_st_comms );
				return DEVFSERR_COMMS_SUBSYSTEM_FAILURE;
			}


	comms_start( devfs_st_comms );
	return 0;
}







