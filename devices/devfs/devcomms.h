#ifndef _DEVFS_COMMS_C
#define _DEVFS_COMMS_C

#include <devfs.h>

int send_packet( int pid, int port, struct devfs_packet *ds );
int send_resp( int pid, int port, struct devfs_packet *ds, int status );


#endif

