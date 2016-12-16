#include <smk.h>
#include <devfs.h>
#include <vfs.h>


#include "comms.h"
#include "misc.h"
#include "mounts.h"
#include "filesystems.h"


static struct mount *mounts = NULL;
static int mount_count = 0;


int init_mounts()
{
	return 0;
}



int mount( const char *device, const char *point, 
			const char *name, unsigned int flags )
{
	struct filesystem *fs = NULL;
	void *data = NULL;
	int rc;
	
	// Open device
	int did = devfs_open( device, flags_v2d(flags) );
	if ( did < 0 ) return did;


	// Auto probe
	fs = fsprobe( did, name );
	if ( fs == NULL )
	{
		devfs_close( did );
		return VFSERR_FILESYSTEM_INCORRECT;
	}
	
	// Initialize filesystem
	rc = fs->init( did, &data );
	if ( rc != 0 )
	{
		devfs_close( did );
		return VFSERR_FILESYSTEM_UNKNOWN;
	}
	
	// Create a mount.
	mounts = smk_realloc( mounts, sizeof(struct mount) * (mount_count+1) );
	if ( mounts == NULL )
	{
		fs->shutdown( did, data );
		devfs_close( did );
		return VFSERR_NOMEM;
	}
	
		// Give it a life.
		mounts[mount_count].device = devfs_strdup( device ); 
		mounts[mount_count].did = did; 
		mounts[mount_count].point = devfs_strdup( point ); 
		mounts[mount_count].fs = fs; 
		mounts[mount_count].fs_data = data; 
		mounts[mount_count].usage = 0; 
		mounts[mount_count].flags = flags;

	mount_count += 1;
		
	return 0;
}

int auto_mount( const char *device, const char *point,
				unsigned int flags )
{
	struct filesystem *fs = NULL;
	int rc;
	void *data;
	
	// Open device
	int did = devfs_open( device, flags_v2d(flags) );
	if ( did < 0 ) 
	{
		return did;
	}

	// Auto probe
	fs = auto_fsprobe( did );
	if ( fs == NULL )
	{
		devfs_close( did );
		return VFSERR_FILESYSTEM_UNKNOWN;
	}

	// Initialize filesystem
	rc = fs->init( did, &data );
	if ( rc != 0 )
	{
		devfs_close( did );
		return VFSERR_FILESYSTEM_UNKNOWN;
	}
	
	
	// Create a mount.
	mounts = smk_realloc( mounts, sizeof(struct mount) * (mount_count+1) );
	if ( mounts == NULL )
	{
		fs->shutdown( did, data );
		devfs_close( did );
		return VFSERR_NOMEM;
	}
	
		// Give it a life.
		mounts[mount_count].device = devfs_strdup( device ); 
		mounts[mount_count].did = did; 
		mounts[mount_count].point = devfs_strdup( point ); 
		mounts[mount_count].fs = fs; 
		mounts[mount_count].fs_data = data; 
		mounts[mount_count].usage = 0; 
		mounts[mount_count].flags = flags; 

	mount_count += 1;

	return 0;
}

int unmount( const char *point )
{
	int i, j;

	for ( i = 0; i < mount_count; i++ )
	{
		if ( devfs_strcmp( mounts[i].point, point ) == 0 )
		{
			if ( mounts[i].usage != 0 )
				return VFSERR_INUSE;

			// Shutdown this filesystem data
			mounts[i].fs->shutdown( mounts[i].did, mounts[i].fs_data );
				
			smk_free( mounts[i].point );		// Free the relatives
			smk_free( mounts[i].device );
			devfs_close( mounts[i].did );		// Close the device
				
												// Adjust ... 
			for ( j = i; j < (mount_count-1); j++ )
				mounts[j] = mounts[j+1];


			mount_count -= 1;
			mounts = smk_realloc( mounts, 
							sizeof(struct mount) * mount_count );
			return 0;
		}
	}
		
	return VFSERR_NOTFOUND;
}



struct mount *find_mount( const char *filename )
{
	int i;
	struct mount *match = NULL;
	int max_match = 0;


	for ( i = 0; i < mount_count; i++ )
	{
		int len = devfs_strlen( mounts[i].point );
			
		if ( devfs_strncmp( mounts[i].point, filename, len ) == 0 )
		{
			if ( max_match <= len )
			{
				match = &(mounts[i]);
				max_match = len;
			}
		}
	}
		
	return match;
}


int prep_mount( int pid, int port, struct vfs_packet *vs )
{
	char *str = VFS_PACKETDATA( vs );
	char *messages[3];
	int i;
	int rc;

	int count = devfs_split( str, ",", messages, 3 ); 

	// We need the right amount of parameters!
	if ( count != 3 )
	{
		send_resp( pid, port, vs, VFSERR_BAD_PARAMETERS );
		return -1;
	}


	// Now mount the file system... 

	if ( devfs_strcmp( messages[2], "auto" ) == 0 )
		rc = auto_mount( messages[0], messages[1], vs->flags );
	else
		rc = mount( messages[0], messages[1], messages[2], vs->flags );


	// Free the messages
	for ( i = 0; i < count; i++ )
		smk_free( messages[i] );


	send_resp( pid, port, vs, rc );
	return 0;
}

int prep_unmount( int pid, int port, struct vfs_packet *vs )
{
	char *str = VFS_PACKETDATA( vs );
	int rc = unmount( str );

	send_resp( pid, port, vs, rc );
	return 0;
}










