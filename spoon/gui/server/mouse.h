#ifndef _MOUSE_H
#define _MOUSE_H


void	mouse_event( int x, int y, unsigned int buttons );

void 	init_mouse();

int		acquire_mouse( const char *filename );

#endif


