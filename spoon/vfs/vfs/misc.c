#include <devfs.h>
#include <vfs.h>


unsigned int flags_d2v( unsigned int flags )
{
	unsigned int tmp = 0;

		if ( (flags & DEVFS_FLAG_OPEN) != 0 ) tmp |= VFS_FLAG_OPEN;
		if ( (flags & DEVFS_FLAG_READ) != 0 ) tmp |= VFS_FLAG_READ;
		if ( (flags & DEVFS_FLAG_WRITE) != 0 ) tmp |= VFS_FLAG_WRITE;
	
	return tmp;
}


unsigned int flags_v2d( unsigned int flags )
{
	unsigned int tmp = 0;

		if ( (flags & VFS_FLAG_OPEN) != 0 ) tmp |= DEVFS_FLAG_OPEN;
		if ( (flags & VFS_FLAG_READ) != 0 ) tmp |= DEVFS_FLAG_READ;
		if ( (flags & VFS_FLAG_WRITE) != 0 ) tmp |= DEVFS_FLAG_WRITE;
		if ( (flags & VFS_FLAG_EXECUTE) != 0 ) tmp |= DEVFS_FLAG_READ;
	
	return tmp;
}

