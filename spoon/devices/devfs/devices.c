#include <smk.h>
#include <devfs.h>

#include "devices.h"
#include "devcomms.h"



// ---------------------------------------------------

static int dv_key( void *d )
{
	int count = 1;
	int i = 0;
		
	struct device_entry *dev = (struct device_entry*)d;

	if ( dev->id >= 0 ) return (dev->id * 20);

	// This will be pot luck!
	while ( dev->name[i] != 0 )
	{
			count += count * dev->name[i++];
	}
	
	return count;
}


static int dv_cmp( void* dl, void* dr )
{
	struct device_entry *right = (struct device_entry *)dr;
	struct device_entry *left = (struct device_entry *)dl;

	if ( right->id >= 0 )
	{
		if ( left->id < right->id ) return -1;
		if ( left->id > right->id ) return 1;
		return 0;
	}
		
		
	return devfs_strcmp( ((struct device_entry*)dl)->name, 
					((struct device_entry*)dr)->name );
}


// ---------------------------------------------------

static struct devfs_htable *device_table = NULL;
static int devfs_deviceCount = 1;

int init_devices()
{
	device_table = devfs_init_htable( 1024, 50, dv_key, dv_cmp );
	if ( device_table == NULL ) return -1;
	return 0;
}




int register_device( int pid, int port, struct devfs_packet* ds )
{
	struct device_entry *dev = NULL;
	char *name = (char*)(DEVFS_PACKETDATA(ds));

	ds->status = DEVFSERR_ERROR;	// A failure from the beginning.

	// Does the device already exist?
	if ( find_device_by_name( name ) != NULL )
		return send_resp( pid, port, ds, DEVFSERR_EXISTS );
	
	// Add this device into the list.
	
	dev = (struct device_entry*)smk_malloc( sizeof(struct device_entry) );
	if ( dev == NULL )
		return send_resp( pid, port, ds, DEVFSERR_NOMEM );

	dev->name = devfs_strdup( name );
	if ( dev->name == NULL )
	{
		smk_free( dev );
		return send_resp( pid, port, ds, DEVFSERR_NOMEM );
	}

	dev->size = ds->size;
	dev->flags = ds->flags;
	dev->pid = ds->pid;
	dev->port = ds->port;
	dev->stream_count = 0;
	dev->streams = NULL;
	dev->hook_count = 0;
	dev->hooks = NULL;
	dev->id	= devfs_deviceCount++;

	if ( devfs_htable_insert( device_table, dev ) != 0 )
	{
		smk_free( dev->name );
		smk_free( dev );

		return send_resp( pid, port, ds, DEVFSERR_NOMEM );
	}

	// Set up a nice reply.
	
	ds->fd = dev->id;	// We're good.

	
	return send_resp( pid, port, ds, 0 );
}


int deregister_device( int pid, int port, struct devfs_packet* ds )
{
	int fd = ds->fd;
	struct device_entry *dev = find_device_by_id( fd );

	if ( (dev == NULL) || (dev->pid != ds->pid) ) 
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );

	// Unable to remove devices in use.
	if ( dev->hook_count != 0 ) 
		return send_resp( pid, port, ds, DEVFSERR_INUSE );
	
	devfs_htable_remove( device_table, dev );
	smk_free( dev->name );
	smk_free( dev );

	return send_resp( pid, port, ds, 0 );
}


/** \todo Make this faster!  
 *
 * Accepted patterns:
 *
 *    '*' matches until the next pattern character is found
 * 
 */
struct device_entry* find_device_by_match( const char *pattern, int pos )
{
	struct device_entry *dev = NULL;
	int count = 0;
	int matches = 0;
	int max_pat = devfs_strlen( pattern );


	while ((dev = devfs_htable_get( device_table, count )) != NULL )
	{
		int pos_pat = 0;
		int pos_name = 0;
		int max_name = devfs_strlen( dev->name );

		int match = 1;

		while ( (pos_pat < max_pat) && (pos_name < max_name) )
		{
			unsigned char cn = dev->name[pos_name];
			unsigned char cp = pattern[pos_pat];
			unsigned char np = pattern[pos_pat + 1];

			if ( cp != '*' ) 	// Non-patterning
			{
				if ( cn == cp )
				{
					pos_name += 1;
					pos_pat += 1;
					continue;
				}

				match = 0;
				break;
			}

			// Patternerific

			if ( cn == np )		// If the next character is found
			{
				pos_pat += 2;
				pos_name += 1;
				continue;
			}

			if ( cn == '/' )	// * must be bound by forward slashes
			{
				if ( pos_pat == (max_pat - 1) ) break;	// Final star, match
				match = 0;
				break;
			}
			
			pos_name += 1;
		}

		// The pattern must be parsed entirely
		if ( pos_pat < ( max_pat - 1) ) match = 0;

		// Make sure we got the right one.
		if ( match == 1 )
		{
			if ( matches++ == pos ) return dev;
		}
		
		count += 1;
	}

	return NULL;
}


struct device_entry* find_device_by_name( const char *name )
{
	struct device_entry dev;
						dev.name = (char*)name;
						dev.id = -1;
		
	return devfs_htable_retrieve( device_table, &dev );
}

struct device_entry* find_device_by_id( int id )
{
	struct device_entry dev;
						dev.id   = id;

	if ( id < 0 ) return NULL;
		
	return devfs_htable_retrieve( device_table, &dev );
}





struct device_entry* hook_device( const char *name, int pid, int fd )
{
	struct device_entry *dev = find_device_by_name( name );
	if ( dev == NULL ) return NULL;


	dev->hooks = (struct device_hook*)
					smk_realloc( dev->hooks, (dev->hook_count+1) * sizeof(struct device_hook) );
	if ( dev->hooks == NULL )
			return NULL;
		
	// Now add this device into the list.

	dev->hooks[ dev->hook_count ].pid = pid;
	dev->hooks[ dev->hook_count ].fd = fd;
	dev->hook_count += 1;
	return dev;
}



int release_device( struct device_entry* dev, int pid, int fd )
{
	int i, j;

	for ( i = 0; i < dev->hook_count; i++ )
	{
		if ( (dev->hooks[i].pid == pid) && (dev->hooks[i].fd == fd ) )
		{
			// Move everything up.
			for ( j = i; j < (dev->hook_count-1); j++ )
				dev->hooks[j] = dev->hooks[j+1];

			dev->hook_count -= 1;

			if ( dev->hook_count == 0 )
			{
				smk_free( dev->hooks );
				dev->hooks = NULL;
			}
			return 0;
		}
	}
		
	// Not found..
	return -1;
}




int get_stream( struct device_entry* dev, int pid, int port, int fd )
{
	dev->streams = (struct stream_hook*)
					smk_realloc( dev->streams, (dev->stream_count+1) * sizeof(struct stream_hook) );
	if ( dev->streams == NULL ) return -1;
		
	// Now add this device into the list.

	dev->streams[ dev->stream_count ].pid = pid;
	dev->streams[ dev->stream_count ].port = port;
	dev->streams[ dev->stream_count ].fd = fd;
	dev->stream_count += 1;
	return 0;
}

int release_stream( struct device_entry* dev, int pid, int port )
{
	int i, j;

	for ( i = 0; i < dev->stream_count; i++ )
	{
		if ( (dev->streams[i].pid == pid) && (dev->streams[i].port == port ) )
		{
			// Move everything up.
			for ( j = i; j < (dev->stream_count-1); j++ )
				dev->streams[j] = dev->streams[j+1];

			dev->stream_count -= 1;

			if ( dev->stream_count == 0 )
			{
				smk_free( dev->streams );
				dev->streams = NULL;
			}
			return 0;
		}
	}
		
	// Not found..
	return -1;
}


