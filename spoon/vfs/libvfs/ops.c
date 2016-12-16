#include <smk.h>
#include <devfs.h>
#include <vfs.h>




int			vfs_create( const char *filename, unsigned int mode )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_CREATE;
		vs->length = VFS_PACKETSIZE + len;
		vs->flags = mode;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		
		status = vfs_send( vs );
		switch (status)
		{
			case 0:
					status = vs->fd;
					break;

			default:
					break;
		}
		

	smk_free( vs );
	return status;
}


int vfs_open( const char *filename, unsigned int flags )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_OPEN;
		vs->length = VFS_PACKETSIZE + len;
		vs->flags = flags;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		
		status = vfs_send( vs );
		switch (status)
		{
			case 0:
					status = vs->fd;
					break;

			default:
					break;
		}
		

	smk_free( vs );
	return status;
}


int vfs_close( int fd )
{
	struct vfs_packet vs; 

		vs.operation = VFS_CLOSE;
		vs.length = VFS_PACKETSIZE;
		vs.fd = fd;

	return vfs_send( &vs );
}


long long 	vfs_read( int fd, long long position, 
							char *buffer, long long bytes )
{
	int status;
	long long num;
	struct vfs_packet vs; 
	struct vfs_packet *resp; 

	if ( bytes >= VFS_MAXSIZE ) return VFSERR_TOOLARGE; 
	if ( bytes <  0 ) return VFSERR_TOOSMALL; 
	if ( fd < 0 ) return VFSERR_BAD_ID;
	if ( buffer == NULL ) return VFSERR_BAD_PARAMETERS;
	

		vs.operation = VFS_READ;
		vs.length = VFS_PACKETSIZE;
		vs.fd = fd;
		vs.size = bytes;
		vs.position = position;
		
		
		status = vfs_send_response( &vs, &resp );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: 
					status = resp->status;
					smk_free( resp );
					return status;
		}



		num = resp->length - VFS_PACKETSIZE;

		if ( (num < 0) || (resp->size != num) )
		{
			smk_free( resp );
			return VFSERR_BAD_RESPONSE;
		}

		devfs_memcpy( buffer, VFS_PACKETDATA(resp), num );

		smk_free( resp );

	return num;
}


long long vfs_write( int fd, long long position,
						const char *buffer, long long bytes )
{
	int status;
	long long num;
	struct vfs_packet *vs; 
	struct vfs_packet *resp; 

	if ( bytes >= VFS_MAXSIZE ) return VFSERR_TOOLARGE; 
	if ( bytes <  0 ) return VFSERR_TOOSMALL; 
	if ( fd < 0 ) return VFSERR_BAD_ID;
	if ( buffer == NULL ) return VFSERR_BAD_PARAMETERS;
	

	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + bytes );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_WRITE;
		vs->length = VFS_PACKETSIZE + bytes;
		vs->fd = fd;
		vs->size = bytes;
		vs->position = position;

		devfs_memcpy( VFS_PACKETDATA(vs), buffer, bytes );
		
		
		status = vfs_send_response( vs, &resp );
		smk_free( vs );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default:
					status = resp->status;
					smk_free( resp );
					return status;
		}


		num = resp->size;
		smk_free( resp );

	return num;
}




int			vfs_rm( const char *filename, unsigned int mode )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_RM;
		vs->length = VFS_PACKETSIZE + len;
		vs->flags = mode;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		status = vfs_send( vs );

	smk_free( vs );
	return status;
}


int			vfs_stat( const char *filename, struct vfsstat *st )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	struct vfs_packet *resp; 
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_STAT;
		vs->length = VFS_PACKETSIZE + len;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		
		status = vfs_send_response( vs, &resp );
		smk_free( vs );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: 
					status = resp->status;
					smk_free( resp );
					return status;
		}


		if ( resp->size != sizeof( struct vfsstat ) )
		{
			smk_free( resp );
			return VFSERR_BAD_MESSAGE;

		}

		devfs_memcpy( st, VFS_PACKETDATA(resp), sizeof( struct vfsstat ) );

	smk_free( resp );
	return 0;
}



int			vfs_chmod( const char *filename, unsigned int mode )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_CHMOD;
		vs->length = VFS_PACKETSIZE + len;
		vs->flags = mode;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		status = vfs_send( vs );

	smk_free( vs );
	return status;

}


int			vfs_mkdir( const char *filename, unsigned int mode, 
							unsigned int flags )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_MKDIR;
		vs->length = VFS_PACKETSIZE + len;
		vs->flags = flags;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		status = vfs_send( vs );

	smk_free( vs );
	return status;
}

// ------- LS -------------------------------------------


int			vfs_ls( const char *filename, struct vfsstat **list, int *max )
{
	int status;
	int len = devfs_strlen( filename ) + 1;
	struct vfs_packet *vs; 
	struct vfs_packet *resp; 
	void *ptr;
	int i;
	
	if ( filename == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_LS;
		vs->length = VFS_PACKETSIZE + len;
		devfs_strcpy( VFS_PACKETDATA(vs), filename );
		
		
		status = vfs_send_response( vs, &resp );
		smk_free( vs );
		if ( status != 0 ) return status;

		status = resp->status;

		switch (status)
		{
			case 0: break;

			default: 
					smk_free( resp );
					return status;
		}


		// Now extract the data......
		ptr = VFS_PACKETDATA( resp );
		for ( i = 0; i < resp->size; i++ )
		{
			if ( i >= *max ) break;		// Don't exceed their max
			
			list[i] = (struct vfsstat*)smk_malloc( sizeof(struct vfsstat) );
			if ( list[i] == NULL ) 
			{
				i -= 1;
				break;
			}

			// Copy the information in.
			devfs_memcpy( list[i], ptr, sizeof(struct vfsstat) );

			list[i]->filename = devfs_strdup( ptr + sizeof(struct vfsstat) );
			if ( list[i]->filename == NULL )
			{
				smk_free( list[i] );
				i -= 1;
				break;
			}

			ptr = ptr + sizeof(struct vfsstat);
			ptr = ptr + devfs_strlen( list[i]->filename ) + 1;
				// Now we're on the next one.
		}


		*max = i;

	smk_free( resp );
	return 0;
}


int			vfs_ls_free( struct vfsstat **list, int max )
{
	int i;

	for ( i = 0; i < max; i++ )
	{
		smk_free( list[i]->filename );
		smk_free( list[i] );
	}

	return 0;
}


// ------- MOUNT OPERATIONS -----------------------------

int			vfs_mount( const char *source, const char *mount, 
						const char *fs, unsigned int flags )
{
	int status;
	int len;
	struct vfs_packet *vs; 
	
	if ( source == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( mount == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( fs == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;

 	len = devfs_strlen( source ) + devfs_strlen( mount ) + devfs_strlen( fs ) + 1 + 1 + 1;
		// Plus two commas and a NULL
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation 	= VFS_MOUNT;
		vs->length 		= VFS_PACKETSIZE + len;
		vs->flags 		= flags;
		devfs_strcpy( VFS_PACKETDATA(vs), source );
		devfs_strcat( VFS_PACKETDATA(vs), "," );
		devfs_strcat( VFS_PACKETDATA(vs), mount );
		devfs_strcat( VFS_PACKETDATA(vs), "," );
		devfs_strcat( VFS_PACKETDATA(vs), fs );
		
		status = vfs_send( vs );
		switch (status)
		{
			case 0:
					status = vs->fd;
					break;

			default:
					break;
		}
		

	smk_free( vs );
	return status;
}



int			vfs_umount( const char *mount, unsigned int flags )
{
	int status;
	int len;
	struct vfs_packet *vs; 
	
	if ( mount == NULL ) return VFSERR_BAD_PARAMETERS;
	if ( vfs_wait(0) != 0 ) return VFSERR_VFS_MISSING;

 	len = devfs_strlen( mount ) + 1;
			
	vs = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + len );
	if ( vs == NULL ) return VFSERR_NOMEM;

		vs->operation = VFS_UNMOUNT;
		vs->length = VFS_PACKETSIZE + len;
		vs->flags = flags;
		devfs_strcpy( VFS_PACKETDATA(vs), mount );
		
		
		status = vfs_send( vs );
		switch (status)
		{
			case 0:
					status = vs->fd;
					break;

			default:
					break;
		}
		

	smk_free( vs );
	return status;
}




