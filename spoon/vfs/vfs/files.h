#ifndef _FILES_H
#define _FILES_H

#include <vfs.h>

#include "mounts.h"


struct file
{
	int fd;
	int pid;
	void *data;
	struct mount *mnt;
};


extern int ft_key( void *fdt );
extern int ft_cmp( void* ldt, void* rdt );

// ----------------------------------------------------------
struct file* get_file( struct devfs_htable *ftable, int fd );
struct file* new_file( struct devfs_htable *ftable, int fd );
// ----------------------------------------------------------

int create( int pid, int port, struct vfs_packet *vs );

int open( int pid, int port, struct vfs_packet *vs );
int close( int pid, int port, struct vfs_packet *vs );

int read( int pid, int port, struct vfs_packet *vs );
int write( int pid, int port, struct vfs_packet *vs );


int rm( int pid, int port, struct vfs_packet *vs );
int stat( int pid, int port, struct vfs_packet *vs );
int chmod( int pid, int port, struct vfs_packet *vs );
int mkdir( int pid, int port, struct vfs_packet *vs );


int ls( int pid, int port, struct vfs_packet *vs );

#endif

