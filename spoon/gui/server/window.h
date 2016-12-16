#ifndef _WINDOW_H
#define _WINDOW_H

#include <smk.h>
#include <gui.h>


/** The internal structure of a window. */
struct window 
{
	int pid;				///< The PID of the owner
	int wid;				///< The WID of the window
	int port;				///< The port of the window

	int x;					///< X position
	int y;					///< Y position
	int width;				///< The width of the window
	int height;				///< The height of the window

	unsigned int	flags;	///< The flags;
	unsigned int	state;	///< The state;
	unsigned int	events;	///< The events of the window

	uint32_t	*buffer;	///< The buffer of the window

	int shmem_id;			///< The shared memory ID.
};



int 	windows_init();

int 	window_create( struct window **result,
						int x, int y,
						int width, int height,
						unsigned int flags,
						unsigned int state,
						unsigned int events,
						unsigned int port,
						int pid );

int 	window_destroy( struct window *win );


struct window 	*window_get( int wid );


#endif
