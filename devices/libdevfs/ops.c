#include <smk.h>
#include <devfs.h>




int devfs_open( const char *device, unsigned int flags )
{
	int status;
	int len = devfs_strlen( device ) + 1;
	struct devfs_packet *ds; 
	
	if ( device == NULL ) return DEVFSERR_BAD_PARAMETERS;
	if ( devfs_wait(0) != 0 ) return DEVFSERR_DEVFS_MISSING;
			
	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + len );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_OPEN;
		ds->length = DEVFS_PACKETSIZE + len;
		ds->flags = flags;
		devfs_strcpy( DEVFS_PACKETDATA(ds), device );
		
		
		status = devfs_send( ds );
		switch (status)
		{
			case 0:
					status = ds->fd;
					break;

			default:
					break;
		}
		

	smk_free( ds );
	return status;
}


int devfs_close( int fd )
{
	struct devfs_packet ds; 

		ds.operation = DEVFS_CLOSE;
		ds.length = DEVFS_PACKETSIZE;
		ds.fd = fd;

	return devfs_send( &ds );
}



/** This function will send through the script commands to the open
 * device specified using the file descriptor returned from an open
 * request.
 *
 * The script commands are a NULL terminated string. (If you need to 
 * send a NULL character, make sure it's escaped.)
 * 
 * If there is a response, the response pointer will point to it. It's
 * up to you to free it. It should also be a NULL terminated string
 * with escaped characters..
 * 
 * The commands and responses are device-dependent. Generally, I want
 * them to look like this: (Or something very close to this)
 *
 *
 * request:
 *   set buffersize=16384; set volume=35; get ID;
 *
 * response:
 *   OK;OK;ID=12312;
 *
 *
 * If there is no response, the value will be NULL.
 *
 */
int devfs_script( int fd, char *commands, char **response )
{
	int status;
	int len = devfs_strlen( commands ) + 1;
	struct devfs_packet *ds; 
	struct devfs_packet *resp; 

	if ( fd < 0 ) return DEVFSERR_BAD_DEVICE;
	if ( commands == NULL ) return DEVFSERR_BAD_PARAMETERS;
	
	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + len );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_SCRIPT;
		ds->length = DEVFS_PACKETSIZE + len;
		ds->fd = fd;
		devfs_memcpy( DEVFS_PACKETDATA(ds), commands, len );
		
		
		status = devfs_send_response( ds, &resp );
		smk_free( ds );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: return resp->status;
		}


		*response = NULL;

		if ( resp->length > DEVFS_PACKETSIZE )
		{
			// Move the data up to the top of the pointer so the
			// user's smk_free will work.
			devfs_memcpy( resp, DEVFS_PACKETDATA(resp), resp->length - DEVFS_PACKETSIZE );	

			*response = (char*)resp;		// Give it to the user
		}
		

	return status;
}



long long devfs_read( int fd, long long position, 
							char *buffer, long long bytes )
{
	int status;
	long long num;
	struct devfs_packet ds; 
	struct devfs_packet *resp; 

	if ( bytes >= DEVFS_MAXSIZE ) return DEVFSERR_TOOLARGE; 
	if ( bytes <  0 ) return DEVFSERR_TOOSMALL; 
	if ( fd < 0 ) return DEVFSERR_BAD_DEVICE;
	if ( buffer == NULL ) return DEVFSERR_BAD_PARAMETERS;
	

		ds.operation = DEVFS_READ;
		ds.length = DEVFS_PACKETSIZE;
		ds.fd = fd;
		ds.size = bytes;
		ds.position = position;
		
		
		status = devfs_send_response( &ds, &resp );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: return resp->status;
		}



		num = resp->length - DEVFS_PACKETSIZE;

		if ( (num < 0) || (resp->size != num) )
		{
			smk_free( resp );
			return DEVFSERR_BAD_RESPONSE;
		}

		devfs_memcpy( buffer, DEVFS_PACKETDATA(resp), num );

		smk_free( resp );

	return num;
}

long long devfs_write( int fd, long long position,
						char *buffer, long long bytes )
{
	int status;
	long long num;
	struct devfs_packet *ds; 
	struct devfs_packet *resp; 

	if ( bytes >= DEVFS_MAXSIZE ) return DEVFSERR_TOOLARGE; 
	if ( bytes <  0 ) return DEVFSERR_TOOSMALL; 
	if ( fd < 0 ) return DEVFSERR_BAD_DEVICE;
	if ( buffer == NULL ) return DEVFSERR_BAD_PARAMETERS;
	

	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + bytes );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_WRITE;
		ds->length = DEVFS_PACKETSIZE + bytes;
		ds->fd = fd;
		ds->size = bytes;
		ds->position = position;

		devfs_memcpy( DEVFS_PACKETDATA(ds), buffer, bytes );
		
		
		status = devfs_send_response( ds, &resp );
		smk_free( ds );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: return resp->status;
		}


		num = resp->size;
		smk_free( resp );

	return num;
}



/** This function returns a ptr to a piece of shared memory 
 * which belongs to the device driver.  It's great for
 * mapping in video memory or allocating memory, etc.
 *
 * The type of memory returned will correspond to the flags
 * used to open the device. For example: if the device
 * is opened as read-only, a write to this map will cause
 * a system fault as the memory will not be writable.
 */
int	devfs_map( int fd, 
				long long position, 
				long long bytes,
				unsigned int flags,
				void **ptr )
{
	int status;
	int shmem_id;
	int pages;
	unsigned int fl;
	struct devfs_packet *ds; 
	struct devfs_packet *resp; 

	if ( bytes <=  0 ) return DEVFSERR_TOOSMALL; 
	if ( fd < 0 ) return DEVFSERR_BAD_DEVICE;
	if ( position < 0 ) return DEVFSERR_BAD_PARAMETERS;
	

	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE );
	if ( ds == NULL ) return DEVFSERR_NOMEM;
	

		ds->operation = DEVFS_MAP;
		ds->length = DEVFS_PACKETSIZE;
		ds->fd = fd;
		ds->position = position;
		ds->size = bytes;
		ds->flags = flags;

		status = devfs_send_response( ds, &resp );
		smk_free( ds );
		if ( status != 0 ) return status;

		shmem_id = resp->status;
		smk_free( resp );
		if ( shmem_id < 0 ) 	// error.
			return shmem_id;


	// We have the shared memory ID. Now request it from the kernel.

	status = smk_request_shmem( shmem_id, ptr, &pages, &fl );
	if ( status != 0 ) return DEVFSERR_SHMEM_FAILED; 

	return 0;
}



/** This function will unmap the memory allocated by a previous
 * devfs_map function.
 */
int			devfs_unmap( int fd, void **ptr )
{
	int status;
	struct devfs_packet *ds; 
	struct devfs_packet *resp; 

	if ( fd < 0 ) return DEVFSERR_BAD_DEVICE;
	

	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_UNMAP;
		ds->length = DEVFS_PACKETSIZE;
		ds->fd = fd;
		ds->size = 0;
		ds->position = 0;

		status = devfs_send_response( ds, &resp );
		smk_free( ds );
		if ( status != 0 ) return status;

		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: return resp->status;
		}

		smk_free( resp );

	*ptr = NULL;
	return 0;
}


/** This is a global static */
static int streaming_initialized = 0;

/** This will cause the device driver to start
 * pushing data to your streaming hook whenever there is 
 * data to be retrieved.  You need to set status to 0 if you 
 * want the streaming to stop and set it to 1 if you want it to
 * start.
 *
 * \param fd The descriptor returned by devfs_open
 * \param status 0 to turn off streaming or 1 to turn it on.
 * \param func The streaming function.
 *
 * \return 0 on successful operation.
 */
int devfs_set_streaming( int fd, int status, 
								void (*func)(int,void*,long long) )
{
	struct devfs_packet ds; 
	struct d_stream *stream = NULL;
	int rc;

	if ( (status != 0) && (status != 1) ) return DEVFSERR_BAD_PARAMETERS;

	if ( streaming_initialized == 0 )
	{
		if ( devfs_streamInit() < 0 )
			return DEVFSERR_NOMEM;

		streaming_initialized = 1;
	}


	// If it exists and the status is still active
	stream = devfs_getStream(fd);
	if ( (stream != NULL) && (status == 1) )
	{
		stream->func = func;	// Just update the function
		return 0;
	}


	// If it doesn't exist and the status is off.
	if ( (stream == NULL) && (status == 0) )
		return DEVFSERR_NOTFOUND;


	// If going on.. add before request.
	if ( status == 1 )
	{
		rc = devfs_addStream( fd, func );
		if ( rc != 0 ) return rc;
	}
	

		ds.operation = DEVFS_STREAM;
		ds.length = DEVFS_PACKETSIZE;
		ds.fd = fd;
		ds.flags = status;
		ds.port = comms_get_port( devfs_st_comms );


	rc = devfs_send( &ds );
	if ( rc != 0 ) 
	{
		if ( status == 1 ) devfs_removeStream( fd );
		return rc;
	}


	// If going off... remove after request
	if ( status == 0 ) devfs_removeStream( fd );


	return 0;
}


int			devfs_stat( const char *device, struct dstat *st )
{
	int status;
	int len = devfs_strlen( device ) + 1;
	struct devfs_packet *ds; 
	
	if ( device == NULL ) return DEVFSERR_BAD_PARAMETERS;
	if ( devfs_wait(0) != 0 ) return DEVFSERR_DEVFS_MISSING;
			
	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + len );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_STAT;
		ds->length = DEVFS_PACKETSIZE + len;
		devfs_strcpy( DEVFS_PACKETDATA(ds), device );
		
		
		status = devfs_send( ds );
		

		if ( status == 0 )
		{
			st->size 	= ds->size;
			st->flags 	= ds->flags;
			st->pid	 	= ds->pid;
		}

	smk_free( ds );
	return status;
}



/** This does a pattern match in the device file system for anything that
 * matches the given pattern. The patten uses a "*" as a wildcard and that
 * matches everything. So if you search for just "*", you'll get all the
 * device driver information back. You can also search for "input\/\*" to 
 * receive all device drivers in that subtree including children.
 *
 * If you just want the root nodes, try "*\/".
 * Or maybe "input\/\*\/" to receive only the files in the input subtree
 *
 * list will be NULL terminated. It's up to you to free all individual entries
 * and the list itself.  You must do this using smk_free!
 *
 */
int			devfs_match( const char *pattern, struct devfs_matchinfo ***list )
{
	int status;
	int len = devfs_strlen( pattern ) + 1;
	struct devfs_packet *ds; 
	struct devfs_packet *resp; 
	struct devfs_matchinfo *record = NULL;
	int num;
	int i;

	*list = NULL;
	
	if ( pattern == NULL ) return DEVFSERR_BAD_PARAMETERS;
	if ( devfs_wait(0) != 0 ) return DEVFSERR_DEVFS_MISSING;
			
	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + len );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_MATCH;
		ds->length = DEVFS_PACKETSIZE + len;
		devfs_strcpy( DEVFS_PACKETDATA(ds), pattern );
		
		
		status = devfs_send_response( ds, &resp );
		smk_free( ds );
		if ( status != 0 ) return status;
		
		switch (resp->status)
		{
			case 0: status = resp->status;
					break;

			default: return resp->status;
		}


	num = resp->size;		// devfs tells us how many

	*list = (struct devfs_matchinfo**)smk_malloc( (num + 1) * sizeof(struct devfs_matchinfo*) );
	if ( *list == NULL )
	{
		smk_free( resp );
		return DEVFSERR_NOMEM;
	}

	// Now parse them all and build a reply
	record = (struct devfs_matchinfo*)DEVFS_PACKETDATA(resp);
	for ( i = 0; i < num; i++ )
	{
		(*list)[i] = (struct devfs_matchinfo*)smk_malloc( record->bytes );
		devfs_memcpy( ((*list)[i]), record, record->bytes );

		record = (struct devfs_matchinfo*)((void*)record + record->bytes);
	}

	(*list)[num] = NULL;	// Last one is NULL

	smk_free( resp );
	return 0;
}



int	devfs_push( int device_id, void *data, long long bytes )
{
	struct devfs_packet *ds; 
	struct d_entry *dev;
	int status;


	if ( device_id < 0 ) return DEVFSERR_BAD_DEVICE;
	if ( data == NULL ) return DEVFSERR_BAD_PARAMETERS;

	dev = devfs_getDevice(device_id);
	if ( dev == NULL ) return DEVFSERR_BAD_DEVICE;
	if ( dev->streaming == 0 ) return DEVFSERR_NOCOMMS;
	

	ds = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE + bytes );
	if ( ds == NULL ) return DEVFSERR_NOMEM;

		ds->operation = DEVFS_PUSH;
		ds->length = DEVFS_PACKETSIZE + bytes;
		ds->fd = device_id;
		ds->size = bytes;

		devfs_memcpy( DEVFS_PACKETDATA(ds), data, bytes );
		
		
		status = devfs_send( ds );

	smk_free( ds );
	return status;
}




/** This pushes information using a pulse to the different
 * listening streams. It's much faster for small pieces
 * of information coming in, for example, a keyboard or
 * mouse.
 * 
 * Use of this function does not return any error generated
 * by the devfs.
 */
int			devfs_push_fast( int device_id, 
								int d1, int d2, int d3, int d4 )
{
	struct d_entry *dev;

	if ( device_id < 0 ) return DEVFSERR_BAD_DEVICE;

	dev = devfs_getDevice(device_id);
	if ( dev == NULL ) return DEVFSERR_BAD_DEVICE;
	if ( dev->streaming == 0 ) return DEVFSERR_NOCOMMS;
	
	if ( devfs_pid < 0 ) return DEVFSERR_DEVFS_MISSING;
		
	if ( smk_send_pulse( 0, devfs_pid, 0, 
							DEVFS_PUSH, device_id, d1, d2, d3, d4 ) != 0 )
			return DEVFSERR_NOCOMMS;


	return 0;
}





