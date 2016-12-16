#include <smk/inttypes.h>
#include <smk/err.h>
#include <smk/process_info.h>

#include "include/atomic.h"
#include "include/assert.h"
#include "include/misc.h"
#include "include/interrupts.h"
#include "include/process.h"
#include "include/syscalls.h"

#include "include/shmem.h"

#include "include/irq.h"
#include "include/lfb.h"

#include "include/pulse.h"
#include "include/ports.h"
#include "include/messages.h"

#include "include/scheduler.h"
#include "include/eflags.h"

#include "include/ioperms.h"

#include "include/ualloc.h"
#include "include/env.h"
#include "include/pci.h"
#include "include/events.h"

#include "include/exec.h"

#include "include/gc.h"

//DONE.DONE.
int syscall_memory( uint32_t r6, uint32_t r5, uint32_t r4, 
					uint32_t r3, uint32_t r2, uint32_t r1)
{
      int num;
      uint32_t *data;
      uint32_t loc;
      struct process *proc;


      switch( r1 & 0xFF )
      {
      	  // alloc/free
		  case SYS_ONE:
					if ( r2 == 0 )
					{
			    		proc = checkout_process( current_pid(), WRITER );
  		  				assert( proc != NULL );
						data = (void*)user_alloc( proc, r3 );
						commit_process( proc );
						return (int)data;
					}
					else if ( r2 == 1 )
					{
			    		proc = checkout_process( current_pid(), WRITER );
  		  				assert( proc != NULL );
				    	user_free( proc, r3 );
						commit_process( proc );
						return 0;
					}

	  	        return -1;

	case SYS_TWO:
			proc = checkout_process( current_pid(), WRITER );
  		  	assert( proc != NULL );
			
				loc = user_map( proc, r2, r3 );

			commit_process( proc );
			return loc;
			

	// returns size of area allocated
	case SYS_THREE:
			proc = checkout_process( current_pid(), READER );
  		  	assert( proc != NULL );
			
		    	num = user_size( proc, r2 );

			commit_process( proc );
		    return num;
		   
		
	// returns the physical location of the block
	case SYS_FOUR:
			proc = checkout_process( current_pid(), READER );
  		  	assert( proc != NULL );
		    
				loc = user_location( proc, r2 );
			
		    commit_process( proc );
		    return loc;
		 

      }


    return -1;
}


//DONE.DONE.
int syscall_system(unsigned int r6, unsigned int r5, unsigned int r4, 
				   unsigned int r3, unsigned int r2, unsigned int r1)
{
	struct process *proc;
	int answer;

	switch ( r1 & 0xFF )
	{
 	   case SYS_ONE:
			return find_process( (unsigned char*)r3 );

	   case SYS_TWO:
		   proc = checkout_process( r2, READER );
		   if ( proc == NULL ) return -1;
		   
		       memcpy( (char*)r3, proc->name, NAME_LENGTH );
		       answer = proc->pid;

		   commit_process( proc );
	       return answer;

       
	   case SYS_THREE:
		   proc = checkout_process( r2, READER );
		   if ( proc == NULL ) return -1;
		   commit_process( proc );
	       return r2;
	};

	
	return -1;
}





//DONE.DONE.
int syscall_process(uint32_t r6, uint32_t r5, uint32_t r4, 
					uint32_t r3, uint32_t r2, uint32_t r1) 
{
  struct process *proc;
  //void *pointer;
  int ans;

  
  switch( r1 & 0xFF )
  {
	  case SYS_ONE:
		  *((int32_t*)r2) = current_pid();
		  *((int32_t*)r3) = current_tid();
		  return 0;

	  case SYS_TWO:
		  proc = checkout_process( r2, WRITER );
		  if ( proc == NULL ) return -1;
		  
		  	if ( proc->state != PROCESS_INIT ) 
			{
				commit_process( proc );
				return -1;
			}
			
		  	proc->state = PROCESS_RUNNING;
			set_thread_state( proc->threads, THREAD_RUNNING );
		
		  commit_process( proc );
		  return 0;

	  case SYS_THREE:
		  proc = checkout_process( current_pid(), WRITER );
  		  assert( proc != NULL );

		  	proc->rc = (int)r2;

			while ( gc_queue( proc->pid, -1 ) != 0 )
						sched_yield();

			disable_interrupts();
			  atomic_dec( & (proc->kernel_threads) );
			  proc->state = PROCESS_FINISHING;
			  set_thread_state( current_thread(), THREAD_FINISHED );
		  	  commit_process( proc );
			enable_interrupts();
			sched_yield();
			assert( "we are running process " == "when we shouldn't be" );
			break;
		  
/*
      case SYS_FOUR:
		    if ( ((r2 != 0) && (r2 != 1)) ) return -1;
			
			proc = checkout_process( current_pid(), WRITER );
			assert( proc != NULL );

				pointer = proc->tls_location;
				if ( r2 == 0 ) *((void**)(r3)) = pointer; // safe to crash
				if ( r2 == 1 ) proc->tls_location = (void**)r3;

			commit_process( proc );
			return 0;
*/

	case SYS_FIVE:
			ans = -1;

			// Checkout appropriately.
			if ( (r2 == 0) || (r2 == 4) )	
				 proc = checkout_process( current_pid(), WRITER );
			else proc = checkout_process( current_pid(), READER );
			
			assert( proc != NULL );

			if ( r2 == 0 )
			{
				ans = set_environment( proc, (unsigned char*)r3, 
										(void*)r4, (unsigned int)r5 );
			}
			else if ( r2 == 1 )
			{
				ans = get_environment( proc, (unsigned char*)r3, 
											(void*)r4, (int)r5 );
			}
			else if ( r2 == 2 )
			{
				ans = get_environment_size( proc, (unsigned char*)r3, 
											(unsigned int*)r4 );
			}
			else if ( r2 == 3 )
			{
				ans = get_environment_information( proc, 
													(int)r3,
												  	(unsigned char*)r4,
												  	(unsigned int*)r5 );
			}
			else if ( r2 == 4 )
			{
				ans = remove_environment( proc, (unsigned char*)r3 );
			}

			commit_process( proc );
			return ans;
			
  }

  return -1;
}



//DONE.DONE.
int syscall_thread( uint32_t r6, uint32_t r5, uint32_t r4, 
					uint32_t r3, uint32_t r2, uint32_t r1)
{
  uint32_t *data;
  int answer;
  struct thread *tr;
  struct process *proc;
  //void *pointer;

  switch ( r1 & 0xFF )
  {
	  case SYS_ONE:
			proc = checkout_process( current_pid(), WRITER );
			assert( proc != NULL );

			    data = (uint32_t*)r4;
			    answer = new_thread( 0,
					  		   proc, 
							     current_thread(),
							     (const unsigned char*)r3,
							     r6,
							     r2,
							     data[0],
							     data[1],
							     data[2],
							     data[3]
							   );
	
			commit_process( proc );
		    return answer;
				   
	  case SYS_TWO:
			proc = checkout_process( current_pid(), WRITER );
			assert( proc != NULL );

			answer = -1;
			
			    tr = get_thread( proc, r2 );
			    if ( tr != NULL ) 
			    {
					disable_interrupts();
			    	set_thread_state( tr, r3 );
					if ( r2 == current_tid() ) sched_yield();
					enable_interrupts();
					answer = 0;
			    }

			commit_process( proc );
		    return answer;

/*
			
	  case SYS_THREE:
			proc = checkout_process( current_pid(), READER );
			assert( proc != NULL );
			
				answer = -1;
		    	tr = find_thread_with_name( proc, (char*)r3);
				if ( tr != NULL ) answer = tr->tid;
				
			commit_process( proc );
		    return answer;
*/


	  case SYS_FOUR:
			return go_dormant( r2 );
		
	  case SYS_FIVE:
			proc = checkout_process( current_pid(), WRITER );
			assert( proc != NULL );

			    tr = current_thread();
			    tr->rc = (int)r2;

				// Please kill me.
				while ( gc_queue( proc->pid, tr->tid ) != 0 )
						sched_yield();
				
				disable_interrupts();
			  		atomic_dec( & (proc->kernel_threads) );
				    set_thread_state( tr, THREAD_FINISHED );
					commit_process( proc );
					

				enable_interrupts();
			   	sched_yield();	// Never to return.
				
			assert( "we are running" == "when we shouldn't be" );
		    return 0;
			
/*
	case SYS_SIX:
		proc = checkout_process( current_pid(), WRITER );
		assert( proc != NULL );

			if ( ((int)r3) == -1 )  tr = current_thread(); // save time.
			else
			   tr = find_thread_with_id( current_process(), (int)r3 );
	
		        if ( ((r2 != 0) && (r2 != 1)) || (tr==NULL)  ) 
				{
					commit_process( proc );
				   	return -1;
		        }
	
			pointer = tr->tls;
			if ( r2 == 1 ) tr->tls = (void*)r4;
			if ( r2 == 0 ) *((void**)(r4)) = pointer; // safe to crash
	
		commit_process( proc );
		return 0;


		*/
  }

  return -1;
}




//DONE.DONE.
int syscall_pulse(	unsigned int r6, unsigned int r5, 
					unsigned int r4, unsigned int r3, 
					unsigned int r2, unsigned int r1 )
{
  int answer;
  unsigned int *data;
  struct process *proc;
  struct process *target;

  switch ( r1 & 0xFF )
  {
	  case SYS_ONE:
		  target = checkout_process( r3, WRITER );
		  if ( target == NULL ) return -1;
		  
			  data = (unsigned int*)r5;
			  answer = send_pulse( current_pid(),
			  			r2, 
			  			target, 
						r4,
						data[0],
						data[1], 
						data[2], 
						data[3], 
						data[4], 
						data[5] );

		  commit_process( target );
		  return answer;

	  case SYS_TWO:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );

			  data = (unsigned int*)r5;
			  answer = receive_pulse(  current_thread(),
			  			   (int*) r2, 
			   			   (int*) r3,
						   (int*) r4,
			  			   & ( data[0] ), 
						   & ( data[1] ), 
						   & ( data[2] ), 
						   & ( data[3] ), 
						   & ( data[4] ), 
						   & ( data[5] ) );
		  
		  commit_process( proc );
		  return answer;
				  
	  case SYS_THREE:
		  proc = checkout_process( current_pid(), READER );
		  assert( proc != NULL );
		  
			  data = (unsigned int*)r5;
			  answer = preview_pulse(  current_thread(),
			  			   (int*) r2, 
			  			   (int*) r3,
			  			   (int*) r4,
			  			   & ( data[0] ), 
						   & ( data[1] ), 
						   & ( data[2] ), 
						   & ( data[3] ), 
						   & ( data[4] ), 
						   & ( data[5] ) );

		  commit_process( proc );
		  return answer;
				  
	  case SYS_FOUR: 
		  proc = checkout_process( current_pid(), READER );
		  assert( proc != NULL );

		  	answer = pulses_waiting( current_thread() );
			
		  commit_process( proc );
		  return answer;

	  case SYS_FIVE:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );
		  
			  answer = drop_pulse( current_thread() );
		  
		  commit_process( proc );
		  return answer;
  }

  
	return -1;
}


//DONE.DONE.
int syscall_message( unsigned int r6, unsigned int r5, unsigned int r4, 
		    	     unsigned int r3, unsigned int r2, unsigned int r1 )
{
  struct process *proc;
  struct process *target;
  int answer;

   answer = -1;

  switch ( r1 & 0xFF )
  {
	  case SYS_ONE:
		  target = checkout_process( r3, WRITER );
		  if ( target == NULL ) return -1;
		  
			  answer = send_message( current_pid(),
						  			 r2,
						  			 target,
									 r4,
									 r5,
									 (void*)r6 );

		  commit_process( target );
		  return answer;

		// recv
	  case SYS_TWO:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );
		  
				  answer = recv_message( current_thread(),
							  			 (int*)r2,
										 (int*)r3,
										 (int*)r4,
							  			 r5,
										 (void*)r6 );
  	
		  commit_process( proc );
		  return answer;
		
		// peek
	  case SYS_THREE:
		  proc = checkout_process( current_pid(), READER );
		  assert( proc != NULL );
		  
				  answer = peek_message( current_thread(),
							  			 (int*)r2,
							  			 (int*)r3,
										 (int*)r4,
										 (int*)r5 );

		  commit_process( proc );
		  return answer;
		  
		// drop
	  case SYS_FOUR:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );
		  
			  answer = drop_message( current_thread() );
		  
		  commit_process( proc );
	  	  return answer;
				  
  }
  
  return answer;
}


//DONE.DONE.
int syscall_port( unsigned int r6, unsigned int r5, unsigned int r4, 
		 		  unsigned int r3, unsigned int r2, unsigned int r1 )
{
  struct process *proc;
  int answer = -1;


  switch ( r1 & 0xFF )
  {
	  case SYS_ONE:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );

			  answer = create_port( current_thread(), (int)r2, (int)r3 );

		  commit_process( proc );
		  return answer;

	  case SYS_TWO:
		  proc = checkout_process( current_pid(), READER );
		  assert( proc != NULL );

			  answer = port2tid( proc, (int)r2 ); 

		  commit_process( proc );
		  return answer;
		
	  case SYS_THREE:
		  proc = checkout_process( current_pid(), READER );
		  assert( proc != NULL );

			  answer = tid2port( proc, (int)r2 ); 

		  commit_process( proc );
		  return answer;
		  
	  case SYS_FOUR:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );
		  
		  	answer = release_port( current_thread(), (int)r2 ); 

		  commit_process( proc );
	  	  return answer;
	
	  case SYS_FIVE:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );

			  if ( r3 == 0 ) 
				   answer = set_port_flags( current_thread(), r2, 0 );
			     else 
				   answer = set_port_flags( current_thread(), r2, IPC_MASKED ); 

		  commit_process( proc );
		  return answer;
		  
	  case SYS_SIX:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );

			  if ( r2 == 0 )
					  answer = bulk_get_ports( current_thread(), (void*)r3 );
			  else if ( r2 == 1 )
					  answer = bulk_set_ports( current_thread(), (void*)r3 );
			
		  commit_process( proc );
		  return answer;
				  
  }
  
  return -1;
}


//DONE.DONE.
int syscall_exec(unsigned int r6, unsigned int r5, unsigned int r4, 
				 unsigned int r3, unsigned int r2, unsigned int r1)
{
  int ans;

  switch ( r1 & 0xFF )
  {
	  case SYS_ONE:
		  ans = exec_memory_region( current_pid(), 
						  			 (unsigned char*)r2,
					 			     (uintptr_t)r3, (uintptr_t)r4, 
							 	     (unsigned char*)r5 );

		  if ( ans == -1 ) return -1;
		  
		  /*
		  proc = checkout_process( ans, WRITER );
		  if ( proc == NULL ) return -1;  /// \todo freak;
		  

		  commit_process( proc );
		  */
		  return ans;
  }
		  
  return -1;	
}





//DONE.DONE.
int syscall_pci( uint32_t r6, uint32_t r5, uint32_t r4, 
				 uint32_t r3, uint32_t r2, uint32_t r1 )
{
  int ans;
  
  switch ( r1 & 0xFF )
  {
    case SYS_ONE:
			ans = pci_find_cfg( r2, r3, r4, ( struct pci_cfg* )r5 );
      		return ans; 

	case SYS_TWO:
			ans = pci_find_probe( r2, r3, r4, (struct pci_cfg*) r5 );
			return ans;

  };

  return -1;
}



//DONE.
int syscall_time( uint32_t r6, uint32_t r5, uint32_t r4, 
				  uint32_t r3, uint32_t r2, uint32_t r1 )
{
   //struct process *proc;
   //time_t one,two;
   //int answer;
   //int* rc;
	int ei;

	 switch ( r1 & 0xFF )
	 {
			 /*
		  case SYS_ONE: 
			  return epoch_seconds();
			 */
		  case SYS_TWO:
				ei = disable_interrupts();
			  	*((uint32_t*)r2) = ((uint32_t)cpu[ CPUID ].st_systemTime.usage) / 1000;
			 	*((uint32_t*)r3) = ((uint32_t)cpu[ CPUID ].st_systemTime.usage) % 1000;
				if ( ei != 0 ) enable_interrupts();
				return 0;

/*
		  case SYS_THREE:
			  delay( r2 );
			  return 0;
			  
		  case SYS_FOUR:
			  system_time( &one, &two );
			  proc = checkout_process( current_pid(), WRITER );
		  	  ASSERT( proc != NULL );

			    current_thread()->sleep_seconds = one + r2;
			    current_thread()->sleep_milliseconds = two;
				
				disable_interrupts();
				    set_thread_state( current_thread(), THREAD_SLEEPING );
					commit_process( proc );
					atomic_dec( &(proc->kernel_threads) );
				enable_interrupts();
			    sched_yield();
				atomic_inc( &(proc->kernel_threads) );
			  return 0;

	      case SYS_FIVE:

			   rc = (int*)r4;
			  
			   if ( ((int)r3) >= 0 ) 
					 answer = begin_wait_thread( r2, r3, rc );
			    else answer = begin_wait_process( r2, rc );

			  return answer;

		  case SYS_SIX:
			  return sched_yield();

*/
         } 


   return -1;
}



//DONE.DONE.
int syscall_irq( uint32_t r6, uint32_t r5, uint32_t r4, 
				 uint32_t r3, uint32_t r2, uint32_t r1)
{
   struct process *proc;
   struct thread *tr;
   int answer = -1;

    switch( r1 & 0xFF )
    {
       case SYS_ONE:
			  proc = checkout_process( current_pid(), WRITER );
		  	  assert( proc != NULL );
			  
				  tr = get_thread( proc, r3 );
				  if ( tr != NULL )
				  {
					  answer = request_irq( tr, r2 );
				  }

			  commit_process( proc );
			  return answer;
			  
	    case SYS_TWO:
		  proc = checkout_process( current_pid(), WRITER );
		  assert( proc != NULL );

		  	answer = release_irq( current_thread(),  r2 );

		  commit_process( proc );
		  return answer;
		  
		case SYS_THREE:
		  return irq_complete( (int)r2 );

		case SYS_FOUR:
			return irq_ack( (int)r2 );
    };

  return -1;
}


//DONE.DONE.
int syscall_io( uint32_t r6, uint32_t r5, uint32_t r4, 
				uint32_t r3, uint32_t r2, uint32_t r1 )
{
  int ans = -1;
  struct process *proc = NULL;


	proc = checkout_process( current_pid(), WRITER );
	assert( proc != NULL );

  switch( r1 & 0xFF )
  {
	  case SYS_ONE:
				 if ( r2 == 0 )  ans = io_grant_priv( proc );
				 if ( r2 == 1 )  ans = io_revoke_priv( proc );
			break;
	
		case SYS_TWO:
			 ans = 0;
		 	 if ( (proc->flags & IOPRIV) == 0 ) 
			 {
				ans = -1;
				break;
			 }
	
			 
			 if ( r2 == 0 ) 
			 {
				 current_thread()->interrupts_disabled = 0;
				 enable_interrupts();
			 }
			 else if ( r2 == 1 ) 
			 {
				 current_thread()->interrupts_disabled = 1;
				 disable_interrupts();
			 } 
			 else ans = -1;
			 
			break;
				 
  }
  			
  commit_process( proc );
  return ans;
}


//DONE.
int syscall_info(	uint32_t r6, uint32_t r5, uint32_t r4, 
					uint32_t r3, uint32_t r2, uint32_t r1)
{
	switch ( r1 & 0xFF )
	{
		case SYS_ONE:
			return process_get_list( (int)r2,  (struct process_information*)r3 );
	};

	return -1;
}



//DONE.DONE.
int syscall_misc( uint32_t r6, uint32_t r5, uint32_t r4, 
				  uint32_t r3, uint32_t r2, uint32_t r1) 
{
	struct process *proc;
	const char *message = NULL;

	switch ( r1 & 0xFF )
	{
		case SYS_ONE:
			sched_lock_all();
			reboot();
			sched_unlock_all();
			return 0;

		case SYS_TWO:
			  proc = checkout_process( current_pid(), WRITER );
	  		  assert( proc != NULL );

			  	proc->rc = (int)r2;
				message = (const char*)r3;

				if ( message == NULL ) message = "none given";

				dmesg("%!smk aborted %s(%i) with reason: %s\n",
							proc->name, proc->pid,
							message );
	
				while ( gc_queue( proc->pid, -1 ) != 0 )
							sched_yield();
	
				disable_interrupts();
				  atomic_dec( & (proc->kernel_threads) );
				  proc->state = PROCESS_FINISHING;
				  set_thread_state( current_thread(), THREAD_FINISHED );
			  	  commit_process( proc );
				enable_interrupts();
				sched_yield();
			assert( "we are running process " == "when we shouldn't be" );
			break;

		case SYS_THREE:
			get_lfb( 	(uint32_t**)r2, (uint32_t*)r3, (uint32_t*)r4 );
			break;	

   }

   return SMK_UNKNOWN_SYSCALL;
}


//DONE.DONE.
int syscall_shmem(uint32_t r6, uint32_t r5, uint32_t r4, 
				  uint32_t r3, uint32_t r2, uint32_t r1 ) 
{
	struct process *proc;
	int ans = -1;

   switch ( r1 & 0xFF )
   {
     case SYS_ONE:
	 	if ( r5 == 0 )
		{
			return shmem_create( (unsigned char*)r2, current_pid(), (int)r3, (int)r4 );
		}
		else
		{
			proc = checkout_process( current_pid(), READER );
			if ( proc == NULL ) return -1;

			 	if ( (proc->flags & IOPRIV) == 0 ) 
				{
					commit_process( proc );
					return -1; 
				}

			commit_process( proc );
			return shmem_create_direct( (unsigned char*)r2, 
										 current_pid(), 
										 (int)r3, 
										 (int)r4,
										 (uintptr_t)r6,
										 0 );
		}
		break;
		  
	 case SYS_TWO:
		  return shmem_delete( current_pid(), (int)r2 );

		  
		  
	 case SYS_THREE:
		  if ( r2 == 0 )
		  {
			return shmem_grant( current_pid(), (int)r3, (int)r4, (unsigned int)r5 );
		  }
		  if ( r2 == 1 )
		  {
			proc = checkout_process( (int)r4, WRITER );
			if ( proc == NULL ) return -1;

				ans = shmem_revoke( current_pid(), proc, (int)r3 );

			commit_process( proc );
			return ans;
		  }
		  return -1; 
		  
	 case SYS_FOUR:
		proc = checkout_process( current_pid(), WRITER );
		if ( proc == NULL ) return -1;


		  if ( r2 == 0 )
		  {
			ans = shmem_request( proc, (int)r3, 
								  (void**)r4,
								  (int*)r5,
								  (unsigned int*)r6 );
		  }
		  if ( r2 == 1 )
		  {
			ans = shmem_release( proc, (int)r3 );
		  }
			
		commit_process( proc );
		return ans;
		  
	 case SYS_FIVE:
			return shmem_find( (unsigned char*)r2, (int*)r3 );

	 case SYS_SIX:
			return shmem_get( (int)r2, (unsigned char*)r3, (int*)r4, (int*)r5, (unsigned int*)r6	);
	
   }

   
   return SMK_UNKNOWN_SYSCALL;
}




//DONE.DONE.
int syscall_events( uint32_t r6, uint32_t r5, uint32_t r4, 
				    uint32_t r3, uint32_t r2, uint32_t r1) 
{
	switch ( r1 & 0xFF )
	{
		case SYS_ONE:
			return set_event_handler( current_pid(), (int)r2, 
										(int)r3, (int)r4 );
	}

	return SMK_UNKNOWN_SYSCALL;
}








int syscall( uint32_t r6, uint32_t r5, uint32_t r4, 
			 uint32_t r3, uint32_t r2, uint32_t r1)
{
  uint32_t system_call;
  uint32_t function;
  //struct thread *tr;
  int rc;

  system_call = SYSCALL_OP(r1);
     function = SYSCALL_FUNC(r1);

/*
	tr = current_thread();
	if ( (tr->capabilities[system_call - 1] & function) != function )
	{
	  return -1;
	}
   // authorised
*/

	// Dying..... stop all threads from entering the kernel.
	if ( (current_process()->state == PROCESS_CRASHING) || 
		 (current_process()->state == PROCESS_FINISHING)    )
	{
		while (1==1) sched_yield();
	}

	/*
	if ( current_pid() == 4 )
	dmesg("%!(%i.%i) syscall(%i), function %i.\n", 
			current_pid(),
			current_tid(),
			system_call, 
			function );
	*/


	/*
  	dmesg("%!r1 = %i, r2 = %i, r3 = %i, r4 = %i, r5 = %i, r6 = %i\n", 
				  r1, r2, r3, r4, r5, r6 );
	*/
	

	atomic_inc( & (current_process()->kernel_threads) );


	rc = SMK_UNKNOWN_SYSCALL;
	  
	switch (system_call)
	{
     case SYS_MEMORY:	 rc = syscall_memory( r6, r5, r4, r3, r2, r1);	break;

/*
     case SYS_SEMAPHORE: rc = syscall_semaphore(r6,r5, r4, r3, r2, r1); break;
*/
     case SYS_SYSTEM:	 rc = syscall_system( r6, r5, r4, r3, r2, r1);	break;
     case SYS_PROCESS:	 rc = syscall_process( r6, r5, r4, r3, r2, r1);	break;
     case SYS_THREAD:	 rc = syscall_thread( r6, r5, r4, r3, r2, r1);	break;

/*
     case SYS_CAP:	 	 rc = syscall_cap( r6, r5, r4, r3, r2, r1);		break;
*/
     case SYS_PULSE: 	 rc = syscall_pulse( r6, r5, r4, r3, r2, r1);	break;
     case SYS_MESSAGE: 	 rc = syscall_message( r6, r5, r4, r3, r2, r1);	break;
     case SYS_PORT: 	 rc = syscall_port( r6, r5, r4, r3, r2, r1);	break;
     case SYS_EXEC: 	 rc = syscall_exec( r6, r5, r4, r3, r2, r1);	break;
						 
     case SYS_PCI: 	 	 rc = syscall_pci( r6, r5, r4, r3, r2, r1);		break;
     case SYS_TIME: 	 rc = syscall_time( r6, r5, r4, r3, r2, r1);	break;
     case SYS_IRQ: 	 	 rc = syscall_irq( r6, r5, r4, r3, r2, r1);		break;
     case SYS_IO: 	 	 rc = syscall_io( r6, r5, r4, r3, r2, r1);		break;
/*
     case SYS_CONSOLE: 	 rc = syscall_console( r6, r5, r4, r3, r2, r1);	break;
     case SYS_LFB:	 	 rc = syscall_lfb( r6, r5, r4, r3, r2, r1);		break;
*/
     case SYS_INFO:      rc = syscall_info( r6, r5, r4, r3, r2, r1);	break;

     case SYS_MISC:    	 rc = syscall_misc( r6, r5, r4, r3, r2, r1);	break;
     case SYS_SHMEM:     rc = syscall_shmem( r6, r5, r4, r3, r2, r1);	break;
     case SYS_EVENTS:    rc = syscall_events( r6, r5, r4, r3, r2, r1);	break;
	}
  
	atomic_dec( & (current_process()->kernel_threads) );

	/*
	dmesg("%!(%i.%i) syscall(%i), function %i: rc(%i) or (%x)\n", 
				current_pid(),
				current_tid(),
				system_call, function, rc, rc );
	*/
  
  return rc;
}


/** Returns 0 if the syscall is safe to conduct while interrupts
 * are disabled.
 */
int syscall_intsafe( uint32_t sysc )
{
	if ( sysc == (SYS_IO|SYS_TWO) ) return 0;
	if ( sysc == (SYS_TIME|SYS_TWO) ) return 0;
	return -1;
}

/** Little stub function which retrieves information from the stack
 * and passes it up to the real syscall() function.
 *
 * Enables interrupts, maybe
 *
 */
int syscall_prepare( uint32_t edi, uint32_t esi, uint32_t ebp,
					 uint32_t temp, uint32_t ebx, uint32_t edx,
					 uint32_t ecx, uint32_t eax )
{
	uint32_t *stack = (uint32_t*)ecx;
		
	// sysenter disabled interrupts so we restore them
	if ( current_thread()->interrupts_disabled == 0)
			 enable_interrupts();
	else
	{
		if ( syscall_intsafe( stack[6] ) != 0 )
		{
			dmesg("%!WARNING: syscall & interrupts disabled (%i.%i - %x, %x)\n",
						current_pid(), current_tid(), stack[6], stack[5] );
			
			//stack[0] = SMK_UNSAFE_SYSCALL_INT;
			//return 0;
		}
	}
	
	stack[0] = syscall( stack[1], stack[2], stack[3], 
						stack[4], stack[5], stack[6] );
	return 0;
}



