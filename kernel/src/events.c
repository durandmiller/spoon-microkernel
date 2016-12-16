#include <smk/err.h>
#include "include/assert.h"
#include "include/events.h"
#include "include/process.h"



/** This will lock the target process and set the event handler to the
 * provided information.
 *
 * \note syscall function
 */
int		set_event_handler( int handler_pid,
						   int handler_tid, 
						   int target_pid, 
						   int target_tid )
{
	assert( handler_pid >= 0 );						// System is broken
	
	if (target_pid < 0) return SMK_PID_NOT_FOUND;
		
	struct process *proc = checkout_process( target_pid, WRITER );
	if ( proc == NULL ) return SMK_PID_NOT_FOUND;

		// Already allocated to someone else?
		if ( (proc->event_pid >= 0) && (proc->event_pid != handler_pid) )
		{
			commit_process( proc );
			return SMK_RESOURCE_ALLOCATED;
		}

		/*
		handler_tid:target_tid

		First case:	(specific thread)
			xx:xx   set a valid thread on a valid thread
			-1:xx   unset a valid thread

		Second case: (process)
			xx:-1   set process-wide thread
			-1:-1   unset process-wide thread
		*/
		
		  
		if ( target_tid >= 0 )	// Targetting a specific thread
		{
			// Thread events
			struct thread *tr = get_thread( proc, target_tid );
			if (tr == NULL)
			{
				commit_process( proc );
				return SMK_TID_NOT_FOUND;
			}

			if ( handler_tid >= 0 )	// Setting a handler
			{
				// If there was no handler, increment hooks
				if ( tr->event_tid < 0 ) 
						proc->event_hooks += 1;
			}
			else	// Unsetting a handler
			{
				if ( tr->event_tid < 0 )	// Wasn't ours anyway	
				{
					commit_process( proc );
					return SMK_NOT_OWNER;
				}
				proc->event_hooks -= 1; 	// Decrement hooks
			}


			tr->event_tid = handler_tid;
		}
		else		// Targetting a process
		{
			// Process events
			if ( handler_tid < 0 )
			{
				if ( proc->event_tid < 0 )	// Wasn't ours anyway
				{
					commit_process( proc );
					return SMK_NOT_OWNER;
				}

				proc->event_hooks 	-= 1;	// hooks
			}
			else
			{
				if ( proc->event_tid < 0 )
					proc->event_hooks	+= 1;	// hooks
			}


			proc->event_tid = handler_tid;
		}
		

		// Now clear the pid if there are no more hooks
		if ( proc->event_hooks == 0 ) proc->event_pid = -1;
		else
			proc->event_pid = handler_pid;
	
		assert( proc->event_hooks >= 0 );	// Sanity
		
	commit_process( proc );
	return SMK_OK;
}


