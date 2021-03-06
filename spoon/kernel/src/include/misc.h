#ifndef _KERNEL_MISC_H
#define _KERNEL_MISC_H


void 			zero_bss();
int 			strlen(const unsigned char *src);
int 			strnlen(const unsigned char *src, unsigned int n);
unsigned char*	strncpy(unsigned char *dest, const unsigned char *src, unsigned int n);
int 			strncmp( const unsigned char* a, const unsigned char* b, unsigned int n);

int 	memcmp(const void* s1, const void* s2, unsigned int n);
void* 	memcpy(void* s1, const void* s2, unsigned int n);
void* 	memset(void* dest, int c, unsigned int n);


void call_task( uint32_t gdt );
void jump_task( uint32_t gdt );


static inline unsigned int cr0()
{
	unsigned int cr;
	asm (" movl %%cr0, %%eax\n "
		 " movl %%eax, %0\n"
					 : "=g" (cr)
					 :
					 : "eax" );
	return cr;
}

static inline unsigned int cr2()
{
	unsigned int cr;
	asm (" movl %%cr2, %%eax\n "
		 " movl %%eax, %0\n"
					 : "=g" (cr)
					 :
					 : "eax" );
	return cr;
}

static inline unsigned int cr3()
{
	unsigned int cr;
	asm (" movl %%cr3, %%eax\n "
		 " movl %%eax, %0\n"
					 : "=g" (cr)
					 : 
					 : "eax" );
	return cr;
}

static inline void invlpg( uint32_t eax )
{
	__asm__
		(
		 "\n"
		   "mov %0, %%eax;\n"
		   "invlpg (%%eax);\n"
		 
		 : 
		 : "g" (eax)
		 : "eax"
		);
}

static inline uint32_t cpu_flags()
{
  uint32_t old_flags;
    asm ( 
		  "pushfl\n"
		  "pop %%eax\n"
		  "mov %%eax, %0\n"
		: "=g" (old_flags)
		:
		: "eax"
      );
      
  return old_flags;
}

static inline void set_cpu_flags( uint32_t flags )
{
    asm ( 
		  "mov %0, %%eax\n"
		  "pushl %%eax\n"
		  "popfl\n"
		: 
		: "g" (flags)
		: "eax"
      );
}


#include "io.h"
static inline void reboot()
{
    int i, j;

    *((uint16_t*)0x472) = 0x1234;
	
    while (1==1)
		for (i = 0 ; i < 100 ; ++i )
		    for(j = 0; j < 100000 ; ++j )  
					outb(0x64, 0xfe);	 // pulse reset low
}



#endif

