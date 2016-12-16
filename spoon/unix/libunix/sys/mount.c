#include <sys/mount.h>
#include "../os/os.h"

int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data)
{
	os_freakout( "sys/mount.h(mount) is not implemented" );
	return -1;
}

int umount(const char *target)
{
	os_freakout( "sys/mount.h(umount) is not implemented" );
	return -1;
}

int umount2(const char *target, int flags)
{
	os_freakout( "sys/mount.h(umount2) is not implemented" );
	return -1;
}


