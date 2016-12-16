#ifndef _DESKTOP_H
#define _DESKTOP_H

#include <inttypes.h>
#include "window.h"

#define DESKTOP_BGCOLOR		0x340034

/** Linked list stub for windows belonging to a desktop */
struct dk_list
{
	struct dk_list *next;
	struct dk_list *prev;
	struct window *win;
};


/** A desktop structure. */
struct desktop
{
	int 			width;		///< The width 
	int 			height;		///< The height 
	uint32_t*		bitmap;		///< The buffer 
	struct window**	winmap;		///< The winmap 

	struct dk_list	*windows;	///< The windows belonging to the desktop
};


int		desktop_init();
struct	desktop*	desktop_get();

int		desktop_create( struct desktop **desk, int width, int height );
int		desktop_destroy( struct desktop **desk );

int		desktop_add( struct desktop *desk, struct window *win );	
int		desktop_remove( struct desktop *desk, struct window *win );	

int		desktop_focus( struct desktop *desk, struct window *win );	
int		desktop_get_xy( struct desktop *desk, 
						int x, int y, 
						struct window **win );
int		desktop_get_active( struct desktop *desk, struct window **win );


int		desktop_update( struct desktop *desk, 
						int x, int y, 
						int width, int height );

int		desktop_refresh( struct desktop *desk );


int		desktop_sync( struct desktop *desk, 
						int x, int y, 
						int width, int height );


int		desktop_mouse_event( struct desktop *desk,
							 unsigned int event,
							 int x, int y,
							 unsigned int buttons );

#endif



