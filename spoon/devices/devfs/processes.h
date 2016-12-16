#ifndef _PROCESS_H
#define _PROCESS_H

#include <devfs.h>

struct process_info
{
	int pid;
	int last_fd;
	struct devfs_htable *files;
};


int init_processes();



struct process_info* get_process( int pid );
struct process_info* new_process( int pid );



#endif


