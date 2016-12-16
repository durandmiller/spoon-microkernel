#include <smk.h>
#include <gui.h>
#include <comms.h>


extern int gui_pid;


extern int window_pulses(void *data, int pid, int port, void *packet );
extern int window_messages(void *data, int pid, int port, void *packet );


/** There are a few steps to creating a window.
 *
 * 	 1. Create a wid_t
 * 	 2. Create the comms structure
 *   3. Request registration with gui_server
 *   4. Map in shared memory
 *   5. Complete the wid
 *   6. Register all events
 *   7. Resume window thread
 *
 */
int 		gui_create_window( wid_t **wid,
								int x, int y,
								int width, int height,
								unsigned int flags,
								unsigned int state,
								unsigned int events )
{
	struct gui_packet	gs;
	struct gui_packet	*resp;
	int rc;
	int pages;
	unsigned int daFlags;


	// 1. Create a wid
	(*wid) = (wid_t*)smk_malloc( sizeof(wid_t) );
	if ( (*wid) == NULL ) return GUIERR_NOMEM;

	// 2. Create the comms structure
	if ( comms_create( &((*wid)->comms), (void*)(*wid) ) != 0 )
	{
		smk_free( (*wid) );
		return GUIERR_COMMS_SUBSYSTEM_ERR;
	}

	// 3. Request registration.

		gs.operation 	= GUI_CREATE;
		gs.length		= GUI_PACKETSIZE;
		gs.wid			= -1;
		gs.status		= 0;
		gs.data[GUI_DATA_X] 		= x;
		gs.data[GUI_DATA_Y] 		= y;
		gs.data[GUI_DATA_WIDTH]		= width;
		gs.data[GUI_DATA_HEIGHT]	= height;
		gs.data[GUI_DATA_FLAGS]		= flags;
		gs.data[GUI_DATA_STATE]		= state;
		gs.data[GUI_DATA_EVENTS]	= events;
		gs.data[GUI_DATA_PORT]		= comms_get_port( (*wid)->comms );


	rc = gui_send_response( &gs, &resp );
	if ( rc != 0 ) 
	{
		comms_destroy( &((*wid)->comms) );
		smk_free( *wid );
		return rc;
	}

	if ( resp->status != GUIERR_OK )
	{
		comms_destroy( &((*wid)->comms) );
		rc = resp->status;
		smk_free( resp );
		return rc;
	}

	// 4. Map in the shared memory.

		(*wid)->shmem_id	=	resp->data[GUI_DATA_SHMEM];
		if ( smk_request_shmem( (*wid)->shmem_id,
								(void**)&((*wid)->buffer),
								&pages,
								&daFlags ) != 0 )
		{
#warning	Destroy window in server
			comms_destroy( &((*wid)->comms) );
			smk_free( resp );
			smk_free( *wid );
			return GUIERR_SHMEM_FAILED;
		}


	// 5. Complete the wid

		(*wid)->wid		= 	resp->wid;
		(*wid)->x 		= 	x;
		(*wid)->y 		= 	y;
		(*wid)->width 	= 	width;
		(*wid)->height	= 	height;
		(*wid)->flags 	= 	flags;
		(*wid)->state 	= 	state;
		(*wid)->events	=	events;
		(*wid)->lock 	=	0;

	smk_free( resp );	// Free the response.

	// 6. Register all events
	comms_register( (*wid)->comms, 
						COMMS_MESSAGE, COMMS_DEFAULT, window_messages );
	comms_register( (*wid)->comms, 
						COMMS_PULSE, COMMS_DEFAULT, window_pulses );

	// 7. Start the subcommunication
	if ( comms_start( (*wid)->comms ) != 0 )
	{
#warning	Destroy window in server
		comms_destroy( &((*wid)->comms) );
		smk_free( *wid );
		return GUIERR_ERROR;
	}

	return 0;
}



void 		gui_destroy_window( wid_t* wid )
{
	comms_terminate( wid->comms );
	comms_wait( wid->comms, 0 );
	comms_destroy( &(wid->comms) );
	smk_free( wid );
}







static int		gui_set( wid_t* wid, 
							unsigned int flags, 
							unsigned int state,
							unsigned int events )
{
	if ( gui_pid < 0 ) return GUIERR_GUI_MISSING;
		
	if ( smk_send_pulse( 0, gui_pid, 0, 
							GUI_SET, wid->wid, 
						 	flags, state, events, 0 ) != 0 )
			return GUIERR_NOCOMMS;

	return 0;
}


unsigned int	gui_get_flags( wid_t* wid )
{
	return wid->flags;
}

unsigned int	gui_get_state( wid_t* wid )
{
	return wid->state;
}

unsigned int	gui_get_events( wid_t* wid )
{
	return wid->events;
}



int			gui_set_flags( wid_t* wid, unsigned int flags )
{
	wid->flags = flags;
	return gui_set( wid, wid->flags, wid->state, wid->events );
}

int			gui_set_state( wid_t* wid, unsigned int state )
{
	wid->state = state;
	return gui_set( wid, wid->flags, wid->state, wid->events );
}

int			gui_set_events( wid_t* wid, unsigned int events )
{
	wid->events = events;
	return gui_set( wid, wid->flags, wid->state, wid->events );
}


int			gui_update(wid_t* wid, int x, int y, int width, int height )
{
	if ( gui_pid < 0 ) return GUIERR_GUI_MISSING;
		
	if ( smk_send_pulse( 0, gui_pid, 0, 
							GUI_UPDATE, wid->wid, 
							x, y, width, height ) != 0 )
			return GUIERR_NOCOMMS;

	return 0;
}



int			gui_sync( wid_t* wid )
{
	if ( gui_pid < 0 ) return GUIERR_GUI_MISSING;
		
	if ( smk_send_pulse( 0, gui_pid, 0, 
							GUI_SYNC, wid->wid, 0, 0,0,0 ) != 0 )
			return GUIERR_NOCOMMS;

	return 0;
}






