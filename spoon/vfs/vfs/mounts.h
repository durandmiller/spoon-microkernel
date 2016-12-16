#ifndef _MOUNTS_H
#define _MOUNTS_H

#include "filesystems.h"

struct mount
{
	char *device;
	int did;
	char *point;
	struct filesystem *fs;
	void *fs_data;

	unsigned int usage;
	unsigned int flags;
};


int init_mounts();


int mount( const char *device, const char *point, 
			const char *name, unsigned int flags );

int unmount( const char *point );

int auto_mount( const char *device, const char *point, 
				unsigned int flags );

struct mount *find_mount( const char *filename );


int prep_mount( int pid, int port, struct vfs_packet *vs );
int prep_unmount( int pid, int port, struct vfs_packet *vs );


#endif

