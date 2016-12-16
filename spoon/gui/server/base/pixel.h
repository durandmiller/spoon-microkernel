#ifndef _BASE_PIXEL_H
#define _BASE_PIXEL_H


#include <inttypes.h>

#include "types.h"
#include "reserves.h"
#include "globals.h"



static inline pixel_t	getpixel( int x, int y )
{
	if ( (x < 0) || (y < 0) || (x>=global_Width) || (y>=global_Height) ) return 0; 
	return global_Screen[ y * global_Width + x ];
}

static inline void		putpixel( int x, int y, pixel_t colour )
{
	if ( colour == TRANSPARENT ) return;

	if ( (x < 0) || (y < 0) || (x>=global_Width) || (y>=global_Height) ) return; 
	global_Screen[ y * global_Width + x ] = colour;
}


#endif

