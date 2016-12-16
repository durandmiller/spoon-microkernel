#ifndef _RESPONSE_H
#define _RESPONSE_H

#include <devfs.h>

int process_response( int pid, int port, void* dsu );
int process_response_pulse( int pid, int port, void *dsu );

#endif

