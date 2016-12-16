#ifndef _FILESYSTEMS_H
#define _FILESYSTEMS_H

#include <vfs.h>


struct filesystem
{
	char *name;
	char *description;

	int (*probe)( int fd );
	int (*init)( int fd, void** data );
	int (*shutdown)( int fd, void* data );

	int (*create)( void *data, int fd, const char *filename,
						unsigned int mode );

	int (*open)( void *data, int fd, const char *filename, void **fdata );
	int (*close)( void *data, int fd, void *fdata );

	long long (*read)( void *data, int fd, void *fdata, 
						unsigned long long position,
						void *buffer, long long len );

	long long (*write)( void *data, int fd, void *fdata, 
						unsigned long long position,
						void *buffer, long long len );

	
	int (*rm)( void *data, int fd, const char *filename );
	int (*mkdir)( void *data, int fd, const char *filename );
	int (*stat)( void *data, int fd, const char *filename, struct vfsstat *st );

	int (*chmod)( void *data, int fd, const char *filename, unsigned int mode );

	int (*ls)( void *data, int fd, const char *pathname, 
					struct vfsstat *list, int *max );
	
};


int init_filesystems();

struct filesystem* auto_fsprobe( int did );
struct filesystem* fsprobe( int did, const char *fs );




#endif



