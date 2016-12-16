#include <smk.h>
#include <devfs.h>


#define INITIAL_STATES			1024


/** This is the state information structure which maintains information
 * about which request went where and by whom it was sent.
 */
struct state_information
{
	volatile int state;			///< The state of the state
	int req_pid;				///< The requesting PID 
	int req_port;				///< The requesting port
	int dest_pid;				///< The PID of the sendee (is that a word?)
	unsigned long long timeout;	///< The time when a request should timeout
};


static volatile unsigned int state_lock = 0;
static struct state_information *state_table = NULL;
static int state_count = 0;
static int request_id = 0;

static int stateThread_id = -1;

// ------------ Timeout expiry thread --------------------------------


int stateThread( void *data )
{
	while (1==1) 
	{
#warning smart sleeping
		smk_go_dormant();



		acquire_spinlock( & state_lock );

#warning implement
		
		release_spinlock( & state_lock );
	}

	return 0;
}


// ------------ Logic ------------------------------------------------


int init_states()
{
	int i;

	state_table = (struct state_information*)
			smk_malloc( INITIAL_STATES * sizeof( struct state_information ) );
	if ( state_table == NULL ) return DEVFSERR_NOMEM;

	state_count = INITIAL_STATES;


	// Clear them all.
	for ( i = 0; i < state_count; i++ )
		state_table[i].state = -1;


	// Now start the expiry thread

	stateThread_id = smk_spawn_thread( stateThread, "stateThread",
										NORMAL_PRIORITY,
										NULL );

	if ( stateThread_id < 0 ) return -1;

	smk_resume_thread( stateThread_id );

	return 0;
}



static int get_next_request_id()
{
	int id;

		while ( state_table[request_id].state != -1 )
		{
			request_id = ( request_id + 1 ) % state_count;
		}

		id = request_id;
		state_table[ id ].state = 0;


		request_id = ( request_id + 1 ) % state_count;


	return id;
}




int insert_state( int req_pid, int req_port,
				  int dest_pid, 
				  unsigned long long duration )
{
	unsigned long long	theTimeNow;
	int id = get_next_request_id();


		state_table[ id ].req_pid = req_pid;
		state_table[ id ].req_port = req_port;
		state_table[ id ].dest_pid = dest_pid;


#warning Get the time here
		theTimeNow = 0;

		state_table[ id ].timeout = theTimeNow + duration;

		state_table[ id ].state = 1;	// And ... go!

	return id;
}



int remove_state( int req_id, int dest_pid, int *req_pid, int *req_port )
{
	int rc = -1;

	acquire_spinlock( & state_lock );

		
		if ( state_table[ req_id ].state == 1 )
		{
			// Make sure the right process is sending the response
			if ( state_table[ req_id ].dest_pid == dest_pid )
			{
				// All good. Return information.

			   *req_pid = state_table[ req_id ].req_pid;
			   *req_port = state_table[ req_id ].req_port;
			   state_table[ req_id ].state = -1;		// And it's free..
			   rc = 0;		// Kaplah!
			}
		}


	release_spinlock( & state_lock );
	return rc;
}




