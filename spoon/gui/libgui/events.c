#include <gui.h>


void 	incrementWindow( wid_t *wid )
{
	int i, j;
	for ( i = 0; i < wid->width; i++ )
	 for ( j = 0; j < wid->height; j++ )
	 {
	 	wid->buffer[ j * wid->width + i ] += 0x101010;
	 }

	gui_sync( wid );
}

int window_pulses(void *data, int pid, int port, void *packet )
{
	wid_t *wid = (wid_t*)data;
//	int *info = (int*)packet;
	
	incrementWindow( wid );
	return -1;
}

int window_messages(void *data, int pid, int port, void *packet )
{
//	wid_t *wid = (wid_t*)data;
//	struct gui_packet *gs = (struct gui_packet*)packet;
	return -1;
}


