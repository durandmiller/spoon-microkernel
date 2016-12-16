#ifndef _DEVFS_STATE_H
#define _DEVFS_STATE_H


/** This function will allocate and create the appropriate stuff
 *  used by devfs to maintain the states. Returns 0 on success.
 */
int init_states();


/** This function inserts a state into the state structure. It
 * returns the request ID on success or -1 on failure.  */
int insert_state( int req_pid, int req_port,
				  int dest_pid, 
				  unsigned long long duration );

/** This will determine if the state corresponding to the request ID
 * and the destination PID exists in the state machine. If it doesn't,
 * the function will return -1. If it does, then it will remove 
 * the state and return the correct PID and port of the requester 
 * as per the paramters. An rc of 0 is returned on success.
 */
int remove_state( int req_id, int dest_pid, 
							  int *req_pid, int *req_port ); 



#endif

