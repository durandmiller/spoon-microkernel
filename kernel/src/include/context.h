#ifndef _KERNEL_CONTEXT_H
#define _KERNEL_CONTEXT_H

#include <smk/inttypes.h>



struct context
{
	uint32_t	eip;
	uint32_t	esp;
	uint32_t	eax, ebx, ecx, edx;
	uint32_t	eflags;
	uint32_t	cr2;
};


#endif
