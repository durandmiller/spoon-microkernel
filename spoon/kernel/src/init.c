#include <smk/inttypes.h>
#include "include/dmesg.h"

#include "include/lfb.h"

#include "include/interrupts.h"
#include "include/multiboot.h"
#include "include/misc.h"
#include "include/gdt.h"
#include "include/physmem.h"
#include "include/paging.h"

#include "include/syscalls.h"

#include "include/modules.h"

#include "include/cpu.h"
#include "include/time.h"
#include "include/exceptions.h"

#include "include/irq.h"
#include "include/ioapic.h"

#include "include/apic.h"


#include "include/smp.h"

#include "include/process.h"

#include "include/smk.h"

#include "include/shmem.h"



void init( unsigned int magic, multiboot_info_t *mb_info )
{
	zero_bss();

	init_gdt();

	if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )      
	{
		while (1==1) 
			((char*)0xB8000)[0]++; 
	}
	
	dmesg_clear();

	dmesg("%!spoon microkernel (%s) loading ... ", KERNEL_VERSION );

	init_memory( mb_info );

	reserve_modules( mb_info );

	init_lfb( mb_info );

	init_paging();

	init_ioapic();

	init_irqs();

	init_time();

	init_cpus();

	shmem_init();

	init_processes();

	init_smk();

	init_modules();

	start_ap_processors();

	cpu_init( 0 );	// Take a short-cut for the BSP processor.
}









