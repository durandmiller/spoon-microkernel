#include <smk.h>
#include <devfs.h>
#include <vfs.h>

#include "comms.h"
#include "files.h"
#include "processes.h"
#include "mounts.h"

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



// -----------------------------------------------------


int create( int pid, int port, struct vfs_packet *vs )
{
	char *name = (char*)VFS_PACKETDATA( vs );
	struct mount *mnt;

	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );


	// mkdir it.
	if ( mnt->fs->create( mnt->fs_data, 
							mnt->did,
							name + devfs_strlen(mnt->point),
							vs->flags ) != 0 )
	{
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


	return send_resp( pid, port, vs, 0 );
}



int open( int pid, int port, struct vfs_packet *vs )
{
	struct file* fp = NULL;
	char *name = (char*)VFS_PACKETDATA( vs );
	int fd;
	struct mount *mnt;
	void *fdata;

	// Load the process information 
	struct process_info *proc = get_process( vs->pid );
	if ( proc == NULL )
	{
		proc = new_process( vs->pid );
		if ( proc == NULL ) 
			return send_resp( pid, port, vs, VFSERR_NOMEM );
	}

	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
	{
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );
	}


	if ( mnt->fs->open( mnt->fs_data, 
						mnt->did,
						name + devfs_strlen(mnt->point),
						&fdata ) != 0 )
	{
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


	// Get the first available FD
	fd = proc->last_fd;
	
	// Now create the file structure.
	fp = new_file( proc->files, proc->last_fd );
	if ( fp == NULL )
	{
		mnt->fs->close( mnt->fs_data, 
						mnt->did,
						fdata );

		return send_resp( pid, port, vs, VFSERR_NOMEM );
	}

	fp->data = fdata;
	fp->mnt 	= mnt;


	// Find the next available FD
	while ( get_file( proc->files, ++fd ) != NULL  )
			continue;
	
	// Save time next time..
	proc->last_fd = fd;

	vs->fd = fp->fd;

	return send_resp( pid, port, vs, 0 );
}



int close( int pid, int port, struct vfs_packet *vs )
{
	struct file* fp = NULL;
	int fd = vs->fd;
	int rc;

	// Oopsie!
	if ( fd < 0 )
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	// Load the process information 
	struct process_info *proc = get_process( vs->pid );
	if ( proc == NULL ) 
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	fp = get_file( proc->files, fd );
	if ( fp == NULL )
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	// Request the file system to close the file..
	rc = fp->mnt->fs->close( fp->mnt->fs_data,
								fp->mnt->did,
								fp->data );
	if ( rc != 0 )
		return send_resp( pid, port, vs, VFSERR_ERROR );


	proc->last_fd = fd;

	
	devfs_htable_remove( proc->files, fp );
	smk_free( fp );

	vs->fd = 0;
	return send_resp( pid, port, vs, 0 );
}

// ------------------------------------------------------------


int read( int pid, int port, struct vfs_packet *vs )
{
	struct vfs_packet *resp;
	struct file* fp = NULL;
	long long rc;
	int fd = vs->fd;

	// Oopsie!
	if ( fd < 0 )
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	// Load the process information 
	struct process_info *proc = get_process( vs->pid );
	if ( proc == NULL ) 
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	fp = get_file( proc->files, fd );
	if ( fp == NULL )
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );

	// The silly 0 byte reads
	if ( vs->size == 0 )
		return send_resp( pid, port, vs, 0 );

	// Get the response ready.
	resp = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE + vs->size ); 
	if ( resp == NULL )
		return send_resp( pid, port, vs, VFSERR_NOMEM );
	
	// file system attempt
	rc = fp->mnt->fs->read(  fp->mnt->fs_data,
							 fp->mnt->did,
							 fp->data,
							 vs->position,
							 VFS_PACKETDATA( resp ),
							 vs->size );

	// Check for any problems.
	if ( rc <= 0 )
		return send_resp( pid, port, vs, VFSERR_ERROR );
	
	
		resp->req_id = vs->req_id;
		resp->operation = VFS_RESPONSE;
		resp->size = rc;
		resp->status = 0;
		resp->length = VFS_PACKETSIZE + rc;	// only send what is necessary
	

	rc = send_packet( pid, port, resp );
	if ( rc != 0 )
	{
		// Ag well. what can you do? Sorry bud. 
	}

	smk_free( resp );
	return 0;
}



int write( int pid, int port, struct vfs_packet *vs )
{
	struct file* fp = NULL;
	long long rc;
	int fd = vs->fd;

	// Oopsie!
	if ( fd < 0 )
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	// Load the process information 
	struct process_info *proc = get_process( vs->pid );
	if ( proc == NULL ) 
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );


	fp = get_file( proc->files, fd );
	if ( fp == NULL )
		return send_resp( pid, port, vs, VFSERR_NOFILE_REMOTE );

	// The silly 0 byte writes
	if ( vs->size == 0 )
		return send_resp( pid, port, vs, 0 );

	// file system attempt
	rc = fp->mnt->fs->write(  fp->mnt->fs_data,
								 fp->mnt->did,
								 fp->data,
								 vs->position,
								 VFS_PACKETDATA( vs ),
								 vs->size );

	// Check for any problems.
	if ( rc <= 0 )
		return send_resp( pid, port, vs, VFSERR_ERROR );
	
	// VS
	vs->size = rc;
	
	return send_resp( pid, port, vs, 0 );
}


int rm( int pid, int port, struct vfs_packet *vs )
{
	char *name = (char*)VFS_PACKETDATA( vs );
	struct mount *mnt;

	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );


	// Stat it.
	if ( mnt->fs->rm( mnt->fs_data, 
							mnt->did,
							name + devfs_strlen(mnt->point) ) != 0 )
	{
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


	return send_resp( pid, port, vs, 0 );
}


int chmod( int pid, int port, struct vfs_packet *vs )
{
	char *name = (char*)VFS_PACKETDATA( vs );
	struct mount *mnt;

	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );


	// Stat it.
	if ( mnt->fs->chmod( mnt->fs_data, 
							mnt->did,
							name + devfs_strlen(mnt->point),
							vs->flags ) != 0 )
	{
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


	return send_resp( pid, port, vs, 0 );
}


int mkdir( int pid, int port, struct vfs_packet *vs )
{
	char *name = (char*)VFS_PACKETDATA( vs );
	struct mount *mnt;

	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );


	// mkdir it.
	if ( mnt->fs->mkdir( mnt->fs_data, 
							mnt->did,
							name + devfs_strlen(mnt->point) ) != 0 )
	{
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


	return send_resp( pid, port, vs, 0 );
}



int stat( int pid, int port, struct vfs_packet *vs )
{
	char *name = (char*)VFS_PACKETDATA( vs );
	struct mount *mnt;
	int rc;
	struct vfs_packet *resp = NULL;


	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );


	// See if we can allocate a response
	resp = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE 
												+ sizeof(struct vfsstat) );
	if ( resp == NULL )
		return send_resp( pid, port, vs, VFSERR_NOMEM );


	// Stat it.
	if ( mnt->fs->stat( mnt->fs_data, 
							mnt->did,
							name + devfs_strlen(mnt->point),
							VFS_PACKETDATA(resp) ) != 0 )
	{
		smk_free( resp );
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


		resp->req_id = vs->req_id;
		resp->operation = VFS_RESPONSE;
		resp->size = sizeof(struct vfsstat);
		resp->status = 0;
		resp->length = VFS_PACKETSIZE + sizeof(struct vfsstat);	

	rc = send_packet( pid, port, resp );
	if ( rc != 0 )
	{
		// Ag well. what can you do? Sorry bud. 
	}

	smk_free( resp );
	return 0;
}



int ls( int pid, int port, struct vfs_packet *vs )
{
	char *name = (char*)VFS_PACKETDATA( vs );
	struct mount *mnt;
	int rc;
	struct vfs_packet *resp = NULL;
	struct vfsstat stats[1024];
	int max;
	void *ptr;
	unsigned long long offset = 0;

	// Find the file, if possible
	mnt = find_mount( name );
	if ( mnt == NULL )
		return send_resp( pid, port, vs, VFSERR_NO_MOUNT );



	// Stat it.
	max = 1024;

	if ( mnt->fs->ls( mnt->fs_data, 
							mnt->did,
							name + devfs_strlen(mnt->point),
							stats, &max ) != 0 )
	{
		return send_resp( pid, port, vs, VFSERR_NOTFOUND );
	}


	// See if we can allocate a response
	resp = (struct vfs_packet*)smk_malloc( VFS_PACKETSIZE );
	if ( resp == NULL )
	{
		for ( rc = 0; rc < max; rc++ )
			smk_free( stats[rc].filename );

		return send_resp( pid, port, vs, VFSERR_NOMEM );
	}


		resp->req_id = vs->req_id;
		resp->operation = VFS_RESPONSE;
		resp->size = 0;
		resp->status = 0;
		resp->length = VFS_PACKETSIZE;

		ptr = VFS_PACKETDATA( resp );
		offset = VFS_PACKETSIZE;

	// ---------------------------

		for ( rc = 0; rc < max; rc++ )
		{
			void *tmp;
			int strLen = devfs_strlen( stats[rc].filename ) + 1;
			int extraLen  = strLen + sizeof( struct vfsstat );

			tmp = resp;
			resp = (struct vfs_packet*)smk_realloc( resp,
											(int)(resp->length + extraLen) );
			if ( resp == NULL )
			{
				smk_free( tmp );
				for ( rc = 0; rc < max; rc++ )
					smk_free( stats[rc].filename );
				
				return send_resp( pid, port, vs, VFSERR_NOMEM );
			}

			// Update the PTR ..........
			ptr = (void*)((unsigned char*)resp + offset);



			devfs_memcpy( ptr, &(stats[rc]), sizeof(struct vfsstat) );
			devfs_memcpy( ptr + sizeof( struct vfsstat ), 
							stats[rc].filename, strLen  );
			
			resp->size 		+= 1;
			resp->length 	+= extraLen;

			offset += extraLen;
		}


	// -------------------
	rc = send_packet( pid, port, resp );
	if ( rc != 0 )
	{
		// Ag well. what can you do? Sorry bud. 
	}


	for ( rc = 0; rc < max; rc++ )
		smk_free( stats[rc].filename );

	smk_free( resp );
	return 0;
}







