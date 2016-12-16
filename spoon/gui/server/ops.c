#include <devfs.h>
#include <gui.h>

#include "base/globals.h"
#include "window.h"
#include "desktop.h"
#include "devcomms.h"


int 	ops_create( int pid, int port, void *dsu )
{
	struct gui_packet *ds = (struct gui_packet*)dsu;
	struct window *win;
	int rc;

	rc = window_create( &win,
						ds->data[GUI_DATA_X],
						ds->data[GUI_DATA_Y],
						ds->data[GUI_DATA_WIDTH],
						ds->data[GUI_DATA_HEIGHT],
						ds->data[GUI_DATA_FLAGS],
						ds->data[GUI_DATA_STATE],
						ds->data[GUI_DATA_EVENTS],
						ds->data[GUI_DATA_PORT], 
						pid );

	if ( rc != GUIERR_OK )
			return send_resp( pid, port, ds, rc );

	ds->wid = win->wid;
	ds->data[GUI_DATA_SHMEM] = win->shmem_id;

	// Add to desktop
	desktop_add( desktop_get(), win );
	desktop_refresh( desktop_get() );
	desktop_sync( desktop_get(), 0, 0, global_Width, global_Height );

	return send_resp( pid, port, ds, GUIERR_OK );
}

int 	ops_destroy( int pid, int port, void *dsu )
{
	struct gui_packet *ds = (struct gui_packet*)dsu;

	int wid = ds->wid;
	struct window *win = window_get( wid );
	if ( win == NULL )
		return send_resp( pid, port, ds, GUIERR_ERROR );

	if ( win->pid != pid ) 
		return send_resp( pid, port, ds, GUIERR_NOT_OWNER );

	desktop_remove( desktop_get(), win );
	desktop_refresh( desktop_get() );
	desktop_sync( desktop_get(), 0, 0, global_Width, global_Height );

	window_destroy( win );
	return send_resp( pid, port, ds, GUIERR_OK );
}



int 	ops_set( int pid, int port, void *dsu ) 
{
	unsigned int *data = (unsigned int*)dsu;
	int wid = data[1];
	unsigned int flags = data[2];
	unsigned int state = data[3];
	unsigned int events = data[4];
	int diff = 0;


	struct window *win = window_get( wid );
	if ( win == NULL ) return -1; 

	if ( win->pid != pid ) return -1;

	if ( (win->flags != flags) || (win->state != state) )
		diff = 1;

		win->flags = flags;
		win->state = state;
		win->events = events;

	if ( diff == 1 )
	{
		desktop_refresh( desktop_get() );
		desktop_sync( desktop_get(), 0, 0, global_Width, global_Height );
	}

	return 0;
}


int 	ops_update( int pid, int port, void *dsu ) 
{
	int *data = (int*)dsu;
	int wid 	= data[1];
	int x 		= data[2];
	int y 		= data[3];
	int width 	= data[4];
	int height 	= data[5];

	struct window *win = window_get( wid );
	if ( win == NULL ) return -1; 

	if ( win->pid != pid ) return -1;

	desktop_update( desktop_get(),
					win->x + x, win->y + y, 
					width, height );
		

	desktop_sync( desktop_get(),
					win->x + x, win->y + y, 
					width, height );

	return 0;
}

int 	ops_sync( int pid, int port, void *dsu ) 
{
	int *data = (int*)dsu;
	int wid 	= data[1];

	struct window *win = window_get( wid );
	if ( win == NULL ) return -1; 

	if ( win->pid != pid ) return -1;

	desktop_update( desktop_get(),
					win->x, win->y, 
					win->width, win->height );
		

	desktop_sync( desktop_get(),
					win->x, win->y, 
					win->width, win->height );

	return 0;
}








