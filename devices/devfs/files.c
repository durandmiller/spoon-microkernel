#include <smk.h>
#include <devfs.h>

#include "devices.h"
#include "files.h"
#include "processes.h"
#include "devcomms.h"

#include "state.h"


#define			STATE_TIMEOUT			30000


// ---------------------------------------------------

int ft_key( void *fdt )
{
	return ((struct file*)fdt)->fd;
}


int ft_cmp( void* ldt, void* rdt )
{
	struct file *ld = (struct file*)ldt;
	struct file *rd = (struct file*)rdt;

	if ( ld->fd < rd->fd ) return -1;
	if ( ld->fd > rd->fd ) return 1;
	return 0;
}

// ---------------------------------------------------


struct file* get_file( struct devfs_htable *ftable, int fd )
{
	struct file ft;
				ft.fd = fd;

	return (struct file*)devfs_htable_retrieve( ftable, &ft );
}


struct file* new_file( struct devfs_htable *ftable, int fd )
{
	struct file *f = (struct file*)smk_malloc( sizeof(struct file) );
	if ( f == NULL ) return NULL;

	f->fd = fd;

	devfs_htable_insert( ftable, f );
	return f;
}

// ---------------------------------------------------


int open( int pid, int port, struct devfs_packet* ds )
{
	struct device_entry *dev = NULL;
	struct devfs_packet notify;
	struct file* fp = NULL;
	char *name = (char*)DEVFS_PACKETDATA( ds );
	int fd;


	// Load the process information 
	struct process_info *proc = get_process( pid );
	if ( proc == NULL )
	{
		proc = new_process( pid );
		if ( proc == NULL ) 
			return send_resp( pid, port, ds, DEVFSERR_NOMEM );
	}

	// Find the device, if possible
	fd = proc->last_fd;

	if ( (dev = hook_device( name, pid, fd )) == NULL )
			return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	// Determine if the device allows itself to be opened. Dunno why not..
	if ( (dev->flags & DEVFS_FLAG_OPEN) == 0 )
			return send_resp( pid, port, ds, DEVFSERR_NOTSUPPORTEDOP );

	// Ensure the flags are okay.
	if ( (dev->flags & ds->flags) != ds->flags )
	{
		release_device( dev, pid, fd ); 
		return send_resp( pid, port, ds, DEVFSERR_BAD_FLAGS );
	}

	
	fp = new_file( proc->files, fd );
	if ( fp == NULL )
	{
		release_device( dev, pid, fd ); 
		return send_resp( pid, port, ds, DEVFSERR_NOMEM );
	}

	fp->device = dev;
	fp->flags  = ds->flags;

	// Find the next available FD
	while ( get_file( proc->files, ++fd ) != NULL  )
			continue;
	
	// Save time next time..
	proc->last_fd = fd;
	ds->fd = fp->fd;

		// Let's just notify the device
		notify.operation 	= DEVFS_OPEN;
		notify.length		= DEVFS_PACKETSIZE;
		notify.pid 			= pid;
		notify.fd			= fp->device->id;

		if ( smk_send_message( 0,  
							   fp->device->pid, fp->device->port,
							   DEVFS_PACKETSIZE, &notify ) != 0 )
		{
#warning DEVICE NEEDS A PING
		}

		
	return send_resp( pid, port, ds, 0 );
}



int close( int pid, int port, struct devfs_packet* ds )
{
	struct devfs_packet notify;
	struct file* fp = NULL;
	int fd = ds->fd;


	if ( fd < 0 )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	// Load the process information 
	struct process_info *proc = get_process( pid );
	if ( proc == NULL ) 
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	fp = get_file( proc->files, fd );
	if ( fp == NULL )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );



		// Let's just notify the device
		notify.operation 	= DEVFS_CLOSE;
		notify.length		= DEVFS_PACKETSIZE;
		notify.pid 			= pid;
		notify.fd			= fp->device->id;

		if ( smk_send_message( 0,  
							   fp->device->pid, fp->device->port,
							   DEVFS_PACKETSIZE, &notify ) != 0 )
		{
#warning DEVICE NEEDS A PING
		}
	
		// ---------------------------------
	

	
	proc->last_fd = fd;


	release_device( fp->device, pid, fd ); 
	
	devfs_htable_remove( proc->files, fp );
	smk_free( fp );


	ds->fd = 0;
	return send_resp( pid, port, ds, 0 );
}



int dispatch_request( int pid, int port, struct devfs_packet* ds )
{
	struct file* fp = NULL;
	int fd = ds->fd;
	int req_id;
	int rc;
	unsigned int flags = 0;



	if ( fd < 0 )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	// Load the process information 
	struct process_info *proc = get_process( pid );
	if ( proc == NULL ) 
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	fp = get_file( proc->files, fd );
	if ( fp == NULL )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	// Ensure Flag allowance
	switch (ds->operation)
	{
		case DEVFS_SCRIPT: flags = DEVFS_FLAG_SCRIPT; break;
		case DEVFS_READ: flags = DEVFS_FLAG_READ; break;
		case DEVFS_WRITE: flags = DEVFS_FLAG_WRITE; break;
		case DEVFS_STREAM: flags = DEVFS_FLAG_STREAM; break;
		case DEVFS_MAP: flags = DEVFS_FLAG_MAP; break;
		case DEVFS_UNMAP: flags = DEVFS_FLAG_UNMAP; break;
		default:
			return send_resp( pid, port, ds, DEVFSERR_NOTSUPPORTEDOP );
	}

	if ( (fp->flags & flags) != flags )
		return send_resp( pid, port, ds, DEVFSERR_BAD_FLAGS );


	// ----------------


	// Now insert this state into the state machine
	req_id = insert_state( pid, port, fp->device->pid, STATE_TIMEOUT );


	if (req_id < 0 ) 
		return send_resp( pid, port, ds, DEVFSERR_ERROR );



		ds->port = 0;
		ds->req_id = req_id;
		ds->fd = fp->device->id;

		rc = send_packet( fp->device->pid, fp->device->port, ds );
		if ( rc != 0 )
		{
			remove_state( req_id, fp->device->pid, &rc, &rc );
			return send_resp( pid, port, ds, rc );
		}


		
	// Now we wait for the response..
	return 0;
}



int dispatch_push( int pid, int port, struct devfs_packet* ds )
{
	struct device_entry* dev = NULL;
	int fd = ds->fd;
	int rc;
	int i;



	if ( fd < 0 )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );

	dev = find_device_by_id( fd );
	if ( dev == NULL )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );
	

	// Ensure that it's the correct owning process
	if ( dev->pid != pid ) 
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );
	

	// ----------------

	for ( i = 0; i < dev->stream_count; i++ )
	{
		ds->port = 0;
		ds->fd = dev->streams[i].fd;


		rc = send_packet( dev->streams[i].pid, dev->streams[i].port, ds );
		if ( rc != 0 )
		{
#warning remove stream and file hook
		}
	}
		
	return send_resp( pid, port, ds, 0 );
}



int dispatch_pulse_push( int pid, int port, void *dsu )
{
	int *data = (int*)dsu;

	int	fd = data[1];
	int	d1 = data[2];
	int	d2 = data[3];
	int	d3 = data[4];
	int	d4 = data[5];

	struct device_entry* dev = NULL;
	int i;

	if ( fd < 0 ) return DEVFSERR_NOTFOUND;

	dev = find_device_by_id( fd );
	if ( dev == NULL ) return DEVFSERR_NOTFOUND;

	// Ensure that it's the correct owning process
	if ( dev->pid != pid ) return DEVFSERR_NOTFOUND; 

	// ----------------

	for ( i = 0; i < dev->stream_count; i++ )
	{
		smk_send_pulse( 0, 
						dev->streams[i].pid,
						dev->streams[i].port,
						DEVFS_PUSH,
						dev->streams[i].fd,
						d1,d2,d3,d4 );
	}

		
	return 0;
}



int stream( int pid, int port, struct devfs_packet* ds )
{
	struct devfs_packet notify;
	struct file* fp = NULL;
	int note = -1;
	int fd = ds->fd;



	if ( fd < 0 )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	// Load the process information 
	struct process_info *proc = get_process( pid );
	if ( proc == NULL ) 
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );


	fp = get_file( proc->files, fd );
	if ( fp == NULL )
		return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );

	// Ensure flag honouring
	if ( (fp->flags & DEVFS_FLAG_STREAM) == 0 )
		return send_resp( pid, port, ds, DEVFSERR_BAD_FLAGS );

	// Depending on flags ... 

	if ( ds->flags != 0 )
	{
		if ( get_stream( fp->device, pid, ds->port, ds->fd ) != 0 )
			return send_resp( pid, port, ds, DEVFSERR_ERROR );

		if ( fp->device->stream_count == 1 ) note = 1;
	}
	else
	{
		if ( release_stream( fp->device, pid, ds->port ) != 0 )
			return send_resp( pid, port, ds, DEVFSERR_ERROR );

		if ( fp->device->stream_count == 0 ) note = 0;
	}



	if ( note != -1 )
	{
		// Let's just notify the device
		notify.operation 	= DEVFS_STREAM;
		notify.length		= DEVFS_PACKETSIZE;
		notify.pid 			= pid;
		notify.flags		= note;
		notify.fd			= fp->device->id;

		if ( smk_send_message( 0,  
							   fp->device->pid, fp->device->port,
							   DEVFS_PACKETSIZE, &notify ) != 0 )
		{
#warning DEVICE NEEDS A PING
		}
	}
	
		// ---------------------------------
	


	return send_resp( pid, port, ds, 0 );
}


int stat( int pid, int port, struct devfs_packet* ds )
{
	struct device_entry *dev = NULL;
	char *name = (char*)DEVFS_PACKETDATA( ds );


	// Try and find the device by name
	if ( (dev = find_device_by_name( name )) == NULL )
			return send_resp( pid, port, ds, DEVFSERR_NOTFOUND );

		ds->flags	= dev->flags;
		ds->size	= dev->size;
		ds->pid		= dev->pid;
		

	return send_resp( pid, port, ds, 0 );
}


/*
struct devfs_matchinfo
{
	unsigned int bytes;
	int type;
	int pid;
	unsigned int flags;
	unsigned long long size;
	const char	name[];
};
*/

struct devfs_matchinfo_stub
{
	unsigned int bytes;
	int type;
	int pid;
	unsigned int flags;
	unsigned long long size;
} __attribute__ ((packed));


static int already_has(struct devfs_packet *data, const char *name, int len)
{
	struct devfs_matchinfo *match = NULL;
	int i;

	match = (struct devfs_matchinfo*)DEVFS_PACKETDATA(data);

	for ( i = 0; i < data->size; i++ )
	{
		if ( match->bytes == (sizeof(struct devfs_matchinfo_stub) + len + 1))
		{
			if ( devfs_memcmp( match->name, name, len ) == 0 ) return 0;
		}


		match = (struct devfs_matchinfo*)((void*)match + match->bytes);
	}

	
	return -1;
}


int match( int pid, int port, struct devfs_packet* ds )
{	
	struct devfs_packet *response = NULL;
	struct device_entry *dev 	  = NULL;
	struct devfs_matchinfo *match = NULL;


	char *pattern = (char*)DEVFS_PACKETDATA( ds );
	int count = 0;
	int words_needed = 0;
	int i;
	int current_size = 0;

	// Allocate the initial response
	response = (struct devfs_packet*)smk_malloc( DEVFS_PACKETSIZE );
	if ( response == NULL )
		return send_resp( pid, port, ds, DEVFSERR_NOMEM );

	response->size = 0;

	current_size = DEVFS_PACKETSIZE;

	// Find out how many words they want back.
	for ( i = 0; i < devfs_strlen( pattern ); i++ )
		if ( pattern[i] == '/' ) words_needed += 1;

	// Try and find the device by name
	while ( (dev = find_device_by_match( pattern, count++ )) != NULL )
	{
		int words_found = 0;
		int new_size = 0;

		for ( i = 0; i < devfs_strlen( dev->name ); i++ )
		{
			if ( dev->name[i] == '/' ) 
				if ( words_found++ == words_needed )
					break;
		}

		if ( already_has( response, dev->name, i ) == 0 )
			continue;

		
		// Allocate a new size
		new_size = current_size + sizeof(struct devfs_matchinfo_stub) + i + 1;

		response = (struct devfs_packet*)smk_realloc( response, new_size );
		if ( response == NULL )
			return send_resp( pid, port, ds, DEVFSERR_NOMEM );

		// Point match to it..
		match = (struct devfs_matchinfo*)((void*)response + current_size);
		current_size = new_size;


		match->bytes = sizeof(struct devfs_matchinfo_stub) + i + 1;

		if ( devfs_strlen( dev->name ) == i )
		{
			match->type  = 1;
			match->pid   = dev->pid;
			match->flags = dev->flags;
			match->size  = dev->size;
		}
		else
		{
			match->type = 0;
		}
		
		devfs_memcpy( match->name, dev->name, i );
		match->name[i] = 0;

		response->size += 1;
	}


	response->status 	= 0;
	response->length 	= current_size;

	send_packet( pid, port, response );

	smk_free( response );

	return 0;
}





