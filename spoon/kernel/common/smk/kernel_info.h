#ifndef _SMK_KERNEL_INFO_H
#define _SMK_KERNEL_INFO_H

#include <smk/inttypes.h>
#include <smk/limits.h>


struct process_information
{
	char 			name[NAME_LENGTH];
	char			cmdline[CMDLINE_LENGTH];
	int 			pid;
	unsigned int	state;	
};



#endif

