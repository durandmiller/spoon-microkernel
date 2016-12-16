#ifndef _LIBKERNEL_ATOMIC_H
#define _LIBKERNEL_ATOMIC_H

extern void smk_release_timeslice();

/**  \defgroup ATOMIC  Atomic Operations  
 *
 */


/** @{  */



static inline void acquire_spinlock( volatile unsigned int *lock )
{
  unsigned char gotit = 0;

  while ( 1==1 )
  {
	asm volatile ( "lock btsl $0x0, (%1)\n"
              	   "setncb %0\n "
			  		: "=g" (gotit)
			  		: "p" (lock)
				);

	if ( gotit != 0 ) break;
	smk_release_timeslice();
  }
}


static inline void release_spinlock( volatile unsigned int *lock )
{
	asm volatile ( "lock btrl $0x0, (%0)\n" : : "p" (lock) );
}


static inline void atomic_inc( int *number )
{
  asm volatile ( "lock incl (%0)\n" : : "p" (number) );
}


static inline void atomic_dec( int *number )
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



/** @} */


#endif

