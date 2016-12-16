#include "base/globals.h"
#include "cursor.h"

#include "desktop.h"


static int mouse_x = 0;					///< Where the mouse used to be.
static int mouse_y = 0;					///< Where the mouse used to be.
static unsigned int mouse_buttons = 0;	///< Which buttons used to be pressed.


static void put_mouse_buffer( int x, int y, 
								pixel_t buffer[CURSOR_HEIGHT][CURSOR_WIDTH] )
{
	int i,j;

	for ( i = 0; i < CURSOR_HEIGHT; i++ )
		for ( j = 0; j < CURSOR_WIDTH; j++ )
		{
			putpixel( x + j, y + i, buffer[i][j] );
		}
}

static void get_mouse_buffer( int x, int y, 
								pixel_t buffer[CURSOR_HEIGHT][CURSOR_WIDTH] )
{
	int i,j;

	for ( i = 0; i < CURSOR_HEIGHT; i++ )
		for ( j = 0; j < CURSOR_WIDTH; j++ )
		{
			buffer[i][j] = getpixel( x + j, y + i );
		}
}


void 	init_mouse()
{
	mouse_x = mouse_y = mouse_buttons = 0;
	get_mouse_buffer( mouse_x, mouse_y, buffer_bitmap );
}




void	mouse_event( int x, int y, unsigned int buttons )
{
	put_mouse_buffer( mouse_x, mouse_y, buffer_bitmap );

		if ( (x != mouse_x) || (y != mouse_y) )
		{
			desktop_mouse_event( desktop_get(),
								 GUI_EVENT_MOUSE_MOVED,
								 x,y, buttons );
		}
	
		if ( mouse_buttons != buttons )
		{
			if ( (mouse_buttons | buttons) > mouse_buttons )
					desktop_mouse_event( desktop_get(),
										 GUI_EVENT_MOUSE_DOWN,
										 x,y, buttons );
				else
					desktop_mouse_event( desktop_get(),
										 GUI_EVENT_MOUSE_UP,
										 x,y, buttons );
		}

		mouse_x = x;
		mouse_y = y;
		mouse_buttons = buttons;


	get_mouse_buffer( mouse_x, mouse_y, buffer_bitmap );
	if ( mouse_buttons != 0 )
	{
		put_mouse_buffer( mouse_x, mouse_y, cursor_down_bitmap );
	}
	else
	{
		put_mouse_buffer( mouse_x, mouse_y, cursor_up_bitmap );
	}
}


#ifndef UNIX

#include <devfs.h>

static int	mouse_fd = -1;

void mouse_stream( int fd, void *data, long long len )
{
	int *info = (int*)data;
	int x = info[1] * global_Width / 1024;
	int y = info[2] * global_Height / 768;
	unsigned int buttons = info[3];

	if ( x >= global_Width ) x = global_Width - 1;
	if ( y >= global_Height ) y = global_Height - 1;
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	
	mouse_event( x, y, buttons );
}


int		acquire_mouse( const char *filename )
{
	mouse_fd = devfs_open( filename, DEVFS_FLAG_STREAM );
	if ( mouse_fd < 0 )
		return -1;
	
	devfs_set_streaming( mouse_fd, 1, mouse_stream );

	return 0;
}
#else


int		acquire_mouse( const char *filename )
{
	return -1;
}

#endif
	


