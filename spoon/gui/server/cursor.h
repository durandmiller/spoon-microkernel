#ifndef _CURSOR_H
#define _CURSOR_H

#include "base/pixel.h"

#define 	CURSOR_WIDTH		8
#define 	CURSOR_HEIGHT		8

extern pixel_t cursor_up_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH];
extern pixel_t cursor_down_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH];
extern pixel_t buffer_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH]; 



#endif

