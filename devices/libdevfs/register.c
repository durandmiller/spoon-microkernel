#include <smk.h>
#include <devfs.h>





static struct d_entry *list = NULL;
static int list_size = 0;


/** Returns a device from the list */
struct d_entry* devfs_getDevice( int id )
{
	int i;

	for ( i = 0; i < list_size; i++ )
		if ( list[i].id == id ) return &(list[i]);

	return NULL;
}


/** Adds a device in the statically declared local list */
static int addDevice( int id, const char* name,
						struct devfs_hooks *hooks, void *data )
{
	list = (struct d_entry*)smk_realloc( list, 
								sizeof(struct d_entry) * (list_size+1) );
	if ( list == NULL ) return DEVFSERR_NOMEM;
	
	list[ list_size ].id = id;
	list[ list_size ].streaming = 0;
	list[ list_size ].device_data = data;
	list[ list_size ].hooks = hooks;
	list[ list_size ].name = devfs_strdup( name );

	list_size += 1;

	return 0;
}

static int removeDevice( int id )
{
	int i,j;

	for ( i = 0; i < list_size; i++ )
		if ( list[i].id == id ) break;

	if ( i == list_size ) return DEVFSERR_NODEV_LOCAL;

	smk_free( list[i].device_data );
	smk_free( list[i].name );
	
	for ( j = i; j < (list_size-1); j++ )
		list[j] = list[j+1];


	list_size -= 1;
	return 0;
}

// ---------------- FUNCTIONS ----------------------------------


/** This function will attempt to register the device with the
 * local data structures and the remote devfs daemon
 */
int devfs_register( const char *device, 
					unsigned long long size, 
					unsigned int flags,
					struct devfs_hooks *hooks,
					void *data )
{
	int rc;
	struct devfs_packet *ds; 
	struct devfs_packet *resp; 
	int len = devfs_strlen( device ) + 1;

	if ( (device == NULL) || (len < 1) ) 
			return DEVFSERR_BAD_PARAMETERS;
	if ( devfs_wait( 0 ) != 0 ) 
			return DEVFSERR_DEVFS_MISSING;
	if ( devfs_ev_comms == NULL )
			return DEVFSERR_DEVFS_MISSING;

			
	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE+len );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		
		ds->operation = DEVFS_REGISTER;
		ds->length = DEVFS_PACKETSIZE + len;
		ds->port = comms_get_port( devfs_ev_comms );
		ds->size = size;
		ds->flags = flags;

	
		devfs_strcpy( DEVFS_PACKETDATA(ds), device );

		// Send through the request
		if ( devfs_send_response( ds, &resp ) != DEVFSERR_OK ) 
		{
			smk_free( ds );
			return DEVFSERR_NOCOMMS;
		}

		smk_free( ds );

		// Determine if the devfs was happy
		switch (resp->status)
		{
			case DEVFSERR_OK:
				rc = resp->fd;
				smk_free( resp );
				break;

			default:
				rc = resp->status;
				smk_free( resp );
				return rc;
		}


	// Now add the device locally.
	addDevice( rc, device, hooks, data ); 

	return rc;
}

int devfs_deregister( int fd )
{
	struct devfs_packet ds; 
	struct d_entry *entry;

	if ( devfs_ev_comms == NULL )
					return DEVFSERR_DEVFS_MISSING;

	entry = devfs_getDevice( fd );
	if ( entry == NULL ) return DEVFSERR_NODEV_LOCAL;
			

		ds.operation = DEVFS_DEREGISTER;
		ds.length = DEVFS_PACKETSIZE;
		ds.fd = fd;

		int status = devfs_send( &ds );
		
		// Determine if the devfs was happy
		switch (status)
		{
			case DEVFSERR_OK:
				break;

			default: return status;
		}


	removeDevice( fd );
	return 0;
}



