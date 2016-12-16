#ifndef _VFS_COMMS_C
#define _VFS_COMMS_C

#include <vfs.h>

int send_packet( int pid, int port, struct vfs_packet *vs );

int send_resp( int pid, int port, struct vfs_packet *vs, int status );


#endif

