#ifndef _KERNEL_EVENTS_H
#define _KERNEL_EVENTS_H


#include "context.h"


struct thread_events
{
	int pid;
	int tid;
	int category;
	int subcategory;
	int rc;
	struct context con; 
};



int		set_event_handler( int handler_pid,
						   int handler_tid, 
						   int target_pid, 
						   int target_tid );



#endif
