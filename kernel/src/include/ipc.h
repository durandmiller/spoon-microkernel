#ifndef _KERNEL_IPC_H
#define _KERNEL_IPC_H

#include "ds/aqueue.h"

#define IPC_MASKED			(1<<0)

#define MAX_PORTS           1024

struct ipc_block
{
    int tid;
	unsigned int flags;

	struct ds_aqueue	*pulse_q;	///<  The pulse queue
	struct ds_aqueue	*message_q;	///<  The message queue
};


#endif

