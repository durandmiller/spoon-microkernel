#ifndef _OPS_H
#define _OPS_H

#include <gui.h>

int 	ops_create( int pid, int port, void *ds );
int 	ops_destroy( int pid, int port, void *ds );


int 	ops_set( int pid, int port, void *ds );
int 	ops_update( int pid, int port, void *ds );
int 	ops_sync( int pid, int port, void *ds );


#endif

