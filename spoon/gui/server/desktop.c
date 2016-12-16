#include <devfs.h>
#include <gui.h>

#include "base/globals.h"
#include "base/reserves.h"
#include "desktop.h"
#include "window.h"
#include "devcomms.h"


static	struct desktop*	g_desktop = NULL;


int		desktop_init()
{
	if ( desktop_create( &g_desktop, global_Width, global_Height ) != 0 )
		return -1;
	return 0;
}

struct	desktop*	desktop_get()
{
	return g_desktop;
}

int		desktop_create( struct desktop **desk, int width, int height )
{
	*desk = smk_malloc( sizeof(struct desktop) );
	if ( *desk == NULL ) return GUIERR_NOMEM;

	(*desk)->width = width;
	(*desk)->height = height;
	(*desk)->windows = NULL;
	(*desk)->bitmap = smk_malloc( width * height * sizeof(uint32_t) );
	if ( (*desk)->bitmap == NULL )
	{
		smk_free( *desk );
		return GUIERR_NOMEM;
	}

	(*desk)->winmap = smk_malloc( width * height * sizeof(struct window*) );
	if ( (*desk)->winmap == NULL )
	{
		smk_free( *desk );
		return GUIERR_NOMEM;
	}


	return 0;
}


int		desktop_destroy( struct desktop **desk )
{
	// Destroy the window list
	while ( (*desk)->windows != NULL )
	{
		struct dk_list *tmp = (*desk)->windows;
		(*desk)->windows = tmp->next;
		smk_free( tmp );
	}

	// Destroy the maps!
	smk_free( (*desk)->winmap );
	smk_free( (*desk)->bitmap );
	smk_free( (*desk) );
	*desk = NULL;
	return 0;
}



int		desktop_add( struct desktop *desk, struct window *win )
{
	struct dk_list *tmp = smk_malloc( sizeof(struct dk_list) );
	if ( tmp == NULL ) return GUIERR_NOMEM;

	tmp->prev = NULL;
	tmp->next = desk->windows;
	tmp->win = win;

	if ( desk->windows != NULL )
		desk->windows->prev = tmp;
	desk->windows = tmp;
	return 0;
}

	
int		desktop_remove( struct desktop *desk, struct window *win )
{
	struct dk_list *tmp = desk->windows;
	while ( tmp != NULL )
	{
		if ( tmp->win == win )
		{
			if ( tmp->prev != NULL ) tmp->prev->next = tmp->next;
			if ( tmp->next != NULL ) tmp->next->prev = tmp->prev;
			if ( desk->windows == tmp )
					desk->windows = tmp->next;
			smk_free( tmp );
			return 0;
		}

		tmp = tmp->next;
	}

	return -1;
}

	

int		desktop_focus( struct desktop *desk, struct window *win )
{
	if ( desktop_remove( desk, win ) != 0 ) return -1;
	if ( desktop_add( desk, win ) != 0 ) return -1;
	return 0;
}

	
int		desktop_get_xy( struct desktop *desk, 
						int x, int y, 
						struct window **win )
{
	if ( (x < 0)  || (y < 0) ) return -1;
	if ( (x >= desk->width)  || (y >= desk->height) ) return -1;

	*win = desk->winmap[ y * desk->width + x ];
	return 0;
}


int		desktop_get_active( struct desktop *desk, struct window **win )
{
	if ( desk->windows == NULL ) return -1;
	*win = desk->windows->win;
	return 0;
}

static int	desktop_draw_background( struct desktop *desk,
									 int x, int y,
									 int width, int height )
{
	int i, j;

	// Draw the background.
	for ( i = 0; i < width; i++ )
	{
		int new_x = x + i;

		if ( new_x < 0 ) continue;			// Still too left
		if ( new_x >= desk->width ) break;	// Gone too right.

		for ( j = 0; j < height; j++ )
		{
			int new_y = y + j;

			if ( new_y < 0 ) continue;			// Still too high
			if ( new_y >= desk->height ) break;	// Gone too low.

			desk->bitmap[ new_y * desk->width + new_x ] = DESKTOP_BGCOLOR;
			desk->winmap[ new_y * desk->width + new_x ] = NULL;
		}
	}

	return 0;
}

static int	desktop_draw_window( struct desktop *desk,
								struct window *win,
								int x, int y,
								int width, int height )
{
	int i, j;

	// Workout where to draw.
	int real_left 	= x;
	int real_top 	= y;
	int real_right 	= x + width;
	int real_bottom = y + height;

	// Now shrink it down if necessary
	if ( x < win->x ) real_left = win->x;
	if ( y < win->y ) real_top = win->y;

	if ( (x + width) > (win->x + win->width) ) 
				real_right = win->x + win->width;

	if ( (y + height) > (win->y + win->height) ) 
				real_bottom = win->y + win->height;

	// Ensure sanity
	if ( real_top < 0 ) real_top = 0;
	if ( real_left < 0 ) real_left = 0;
	if ( real_right > desk->width ) real_right = desk->width;
	if ( real_bottom > desk->height ) real_bottom = desk->height;


	// Draw the window contents onto the desktop buffer and winmap
	for ( i = real_left; i < real_right; i++ )
	{
		int buf_x = i - win->x;		// Index into window

		for ( j = real_top; j < real_bottom; j++ )
		{
			int buf_y = j - win->y;		// Index into window

			uint32_t colour = win->buffer[ buf_y * win->width + buf_x ];

			if ( colour == TRANSPARENT ) continue;

			desk->bitmap[ j * desk->width + i ] = colour;
			desk->winmap[ j * desk->width + i ] = win;
		}
	}

	return 0;
}




int		desktop_update( struct desktop *desk, 
						int x, int y, 
						int width, int height )
{
#warning speed up drastically by only drawing visible regions, etc.

	desktop_draw_background( desk, x, y, width, height );

	if ( desk->windows != NULL )
	{
		struct dk_list *tmp = desk->windows;
		while ( tmp->next != NULL )
			tmp = tmp->next;

		// Now draw backwards. sigh.
		while ( tmp != NULL )
		{
			struct window *win = tmp->win;

			if ( (win->state & GUI_STATE_HIDDEN) == 0 ) 
			{
				desktop_draw_window( desk, win, x, y, width, height );
			}

			tmp = tmp->prev;
		}

	}

	return 0;
}



int		desktop_refresh( struct desktop *desk )
{
	return desktop_update( desk, 0, 0, desk->width, desk->height );
}


int		desktop_sync( struct desktop *desk, 
						int x, int y, 
						int width, int height )
{
	int i, j;

	for ( i = x; i < (x + width); i++ )
	{
		if ( i < 0 ) continue;
		if ( i >= desk->width ) break;

		for ( j = y; j < (y + height); j++ )
		{
			if ( j < 0 ) continue;
			if ( j >= desk->height ) break;

			int new_pos = j * desk->width + i;

			global_Screen[ new_pos ] = desk->bitmap[ new_pos ];
		}
	}
	
	return 0;
}



int		desktop_mouse_event( struct desktop *desk,
							 unsigned int event,
							 int x, int y,
							 unsigned int buttons )
{
	struct window *win = NULL;
	int rc = -1;
	
	if ( desktop_get_xy( desk, x, y, &win ) != 0 )
		return -1;

	if ( win == NULL ) return -1;

	if ( (win->events & event) != 0 )
	{
		rc = send_event( win->pid, win->port,
						event,
						win->wid,
						x, y, buttons );
	}

	return rc;
}




