#ifndef _KERNEL_LFB_H
#define _KERNEL_LFB_H

#include "multiboot.h"
#include <smk/inttypes.h>
#include "lfb.h"

void 	init_lfb(  multiboot_info_t *multiboot_info );


void 	get_lfb( uint32_t **ptr, 
					uint32_t *width, 
					uint32_t *height );

#endif

