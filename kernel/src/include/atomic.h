#ifndef _KERNEL_ATOMIC_H
#define _KERNEL_ATOMIC_H


#include "dmesg.h"
#include <smk/inttypes.h>


// ----- DEADLOCK detection
#include "apic.h"
#define CPUID  (APIC_GET( APIC_apicID ) >> 24)


/** spinlock information */
typedef struct 
{
	volatile unsigned int lock;
	unsigned int cpu_id;
	void *return0;
	void *return1;
} spinlock_t;


extern spinlock_t kernel_lock;
extern volatile unsigned int catch_deadlocks;


#define INIT_SPINLOCK	{0,0,NULL,NULL}

#define SET_SPINLOCK( crow )	crow.lock = 0

static inline void atomic_init( spinlock_t *lock )
{
	lock->lock = 0;
	lock->cpu_id = 0;
	lock->return0 = 0;
	lock->return1 = 0;
}

static inline void atomic_add( volatile unsigned int *number, int diff )
{
  asm volatile ( "movl %1, %%eax;\n lock addl %%eax,(%0)\n" 
				  						: 
				  						: "p" (number), "g" (diff)
				  						: "eax");
}


static inline void atomic_sub( volatile unsigned int *number, int diff )
{
  asm volatile ( "movl %1, %%eax;\n lock subl %%eax, (%0)\n" 
				  						: 
				  						: "p" (number), "g" (diff)
										: "eax" );
}



static inline void acquire_spinlock( spinlock_t *spinlock )
{
  uint8_t gotit = 0;
  int failures = 0;


  while ( 1==1 )
  {
	asm volatile ( "lock btsl $0x0, (%1)\n"
              	   "setncb %0\n "
			  		: "=g" (gotit)
			  		: "p" (&(spinlock->lock))
				);

	if ( gotit != 0 ) break;
	asm ( "pause" );	// As per the Intel recommendations.


	if ( catch_deadlocks != 0 )
	{
		// Let people know..
		((char*)0xb8000)[160 * 24 + 158]++; 
		
		if ( failures++ > 1000000 )
		{
			dmesg_xy(0,CPUID*2, "CPU %i: deadlock on %x by %i from: %x,%x",
									CPUID,
									spinlock,
									spinlock->cpu_id,
									spinlock->return0,
									spinlock->return1 );

			dmesg_xy(0,CPUID*2+1, "       coming from: %x,%x",
									__builtin_return_address(0),
									__builtin_return_address(1) );

			while (1==1) asm("hlt");
		}
	}
	
	/// \todo sched_yield();
  }

	spinlock->cpu_id = CPUID;

	if ( __builtin_frame_address(0) != 0 )
  		spinlock->return0 = __builtin_return_address(0);
	else spinlock->return0 = NULL;
		
	if ( __builtin_frame_address(1) != 0 )
  		spinlock->return1 = __builtin_return_address(1);
	else spinlock->return1 = NULL;
}


static inline void release_spinlock( spinlock_t *spinlock )
{
	asm volatile ( "lock btrl $0x0, (%0)\n" : : "p" (&(spinlock->lock)) );
}




static inline void atomic_inc( unsigned int *number )
{
  asm volatile ( "lock incl (%0)\n" : : "p" (number) );
}


static inline void atomic_dec( unsigned int *number )
{
  asm volatile ( "lock decl (%0)\n" : : "p" (number) );
}


static inline void atomic_set( int *number, int num )
{
  asm volatile ( "lock xchgl %0, (%1)" : : "g" (num), "p" (number) );
}

static inline void atomic_exchange( void** left, void** memory )
{
  asm volatile ( "movl %1, %%eax\n"
		 "lock xchgl %%eax, (%2)\n" 
		  "movl %%eax, %0"
		  : "=p" (*left) 
		  : "p" (*left), "p" (memory)
		  : "eax" );

}

static inline int atomic_bitset( volatile unsigned int *val, int bit )
{
  char tmp = 0;
	
  asm volatile ( "movl %1, %%eax\n"
		 "movl %2, %%ebx\n"
		 "lock btsl %%ebx, (%%eax)\n"
		 "setc %0\n"
		 : "=g" (tmp)
		 : "p" (val), "g" (bit)
		 : "eax", "ebx"
		);

  if ( tmp == 0 ) return 0;
  return 1;
}

static inline int atomic_bitclear( volatile unsigned int *val, int bit )
{
  char tmp = 0;
	
  asm volatile ( "movl %1, %%eax\n"
		 "movl %2, %%ebx\n"
		 "lock btrl %%ebx, (%%eax)\n"
		 "setc %0\n"
		 : "=g" (tmp)
		 : "p" (val), "g" (bit)
		 : "eax", "ebx"
		);

  if ( tmp == 0 ) return 0;
  return 1;
}




static inline void begin_critical_section()
{
	acquire_spinlock( & kernel_lock );
}

static inline void end_critical_section()
{
	release_spinlock( & kernel_lock );
}



#endif

