#include <smk.h>
#include <comms.h>


extern	int		comms_loop( void * data );



int		comms_create( comms_t **comms, void *data )
{
	*comms = (comms_t*)smk_malloc( sizeof( comms_t ) );
	if ( *comms == NULL ) return SMK_NOMEM;


		(*comms)->tid = smk_spawn_thread( comms_loop, 
										"comms_loop",
										NORMAL_PRIORITY,
										(void*)(*comms) );

		if ( (*comms)->tid < 0 )
		{
			smk_free( *comms );
			*comms = NULL;
			return SMK_SPAWN_FAILED;
		}

		
		(*comms)->port = smk_new_port( (*comms)->tid );
		if ( (*comms)->port < 0 )
		{
			smk_kill_thread( (*comms)->tid );
			smk_free( *comms );
			*comms = NULL;
			return SMK_ERROR;
		}

		(*comms)->count = 0;
		(*comms)->lock = 0;
		(*comms)->data = data;
		(*comms)->handlers = NULL;

	return 0;
}



int		comms_destroy( comms_t **comms )
{
#warning Think of a better way to do this.
	comms_kill( *comms );
	if ( (*comms)->handlers != NULL ) smk_free( (*comms)->handlers );
	smk_free( *comms );
	return 0;
}


int		comms_register( comms_t *comms, 
						int type,
						unsigned int operation, 
						comms_handler_t handler )
{
	struct comms_pair* previous = NULL;
	int rc = -1;

	if ( operation == COMMS_TERMINATE ) return -1;

	acquire_spinlock( &(comms->lock) );

		previous = comms->handlers;

		comms->handlers = smk_realloc( comms->handlers, 
						(comms->count + 1) * sizeof( struct comms_pair ));

		if ( comms->handlers == NULL )
		{
			comms->handlers = previous;
			rc = -1;
		}
		else
		{
			comms->handlers[ comms->count ].type = type;
			comms->handlers[ comms->count ].operation = operation;
			comms->handlers[ comms->count ].handler = handler;

			comms->count += 1;
			rc = 0;
		}
		

	release_spinlock( &(comms->lock) );
	return rc;
}

int		comms_deregister( comms_t *comms, int type, unsigned int operation )
{
	int i;
	int offset = 0;

	acquire_spinlock( &(comms->lock) );

		for ( i = 0; i < (comms->count - offset); i++ )
		{
			if ( (comms->handlers[i].operation == operation) &&
					(comms->handlers[i].type == type ) )
						offset += 1;

			if ( offset != 0 )
				comms->handlers[ i ] = comms->handlers[ i + offset ];
		}


		comms->count -= offset;

	release_spinlock( &(comms->lock) );
	if ( offset == 0 ) return -1;
	return 0;
}

int		comms_get_port( comms_t *comms )
{
	return comms->port;
}


int		comms_get_tid( comms_t *comms )
{
	return comms->tid;
}

				
int		comms_start( comms_t *comms )
{
	return smk_resume_thread( comms->tid );
}

int		comms_terminate( comms_t *comms )
{
	return smk_send_pulse( 0, comms->tid, comms->port, COMMS_TERMINATE, 0, 0, 0, 0, 0 );
}

int		comms_wait( comms_t *comms, unsigned int timeout )
{
#warning implement me
	return -1;
}

int		comms_kill( comms_t *comms )
{
	return smk_kill_thread( comms->tid );
}




int	comms_test_pid( int pid, int port, unsigned int timeout )
{
	int duration = timeout;
	if( pid < 0 ) return -1;


	while ( smk_send_pulse( 0, pid, port, 
							COMMS_PING,
							0,0,0,0,0 ) != 0 )
	{
		if ( timeout == 0 )								 // Eternal wait.
		{
			smk_go_dormant_t( COMMS_WAIT_TIME );
			continue;
		}

		if ( duration < COMMS_WAIT_TIME )				 // Timed wait.
			duration -= smk_go_dormant_t( duration );	
		else
			duration -= smk_go_dormant_t( COMMS_WAIT_TIME );
		
		if ( duration <= 0 ) return -1;
	}

	return 0;
}

int comms_test_name( const char *name, int port, unsigned int timeout )
{
	int pid = smk_find_process_id( name );
	return comms_test_pid( pid, port, timeout );
}



