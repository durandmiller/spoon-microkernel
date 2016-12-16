#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H		1



#include <bits/sys/mount.h>


#ifdef __cplusplus
extern "C" {
#endif


int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data);

int umount(const char *target);

int umount2(const char *target, int flags);



#ifdef __cplusplus
}
#endif



#endif
