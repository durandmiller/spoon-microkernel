#include <devfs.h>
#include <smk.h>

#define SCREEN_ATTR				7
#define WIDTH					80
#define HEIGHT					25

static char *screen = NULL; 
static int s_x = 0;	
static int s_y = 0;

// --------- CURSOR OPERATIONS --------------------------

static void get_cursor()
{
	unsigned int position;
	
	smk_disable_interrupts();
		outb( 0x3D4, 0x0E );
		position = inb( 0x3D5 );
		outb( 0x3D4, 0x0F );
		position = (position << 8) + inb( 0x3D5 );
	smk_enable_interrupts();

	s_x = position % 80;
	s_y = position / 80;

	if ( (s_x < 0) || (s_x >= WIDTH) ) s_x = 0;
	if ( (s_y < 0) || (s_y >= HEIGHT) ) s_y = 0;
}


static void set_cursor( int x, int y)
{
	unsigned int position = y * 80 + x;

	smk_disable_interrupts();
		outb(0x3D4, 0x0F);
		outb(0x3D5, (unsigned char)(position & 0xFF));
		outb(0x3D4, 0x0E);
		outb(0x3D5, (unsigned char )((position >> 8) & 0xFF));
	smk_enable_interrupts();
}


// ------- INITIALIZATION ---------------------

int init_puts()
{
	// Now request the priviledges
	if ( smk_request_iopriv() != 0 ) return -1;
		
	screen = (char*)smk_mem_map( (void*)0xb8000, 1 );
	if ( screen == NULL ) return -1;
	get_cursor();

	return 0;
}


// -------------------------------------------

static void scroll()
{
   int j;

   devfs_memcpy( screen, screen + WIDTH*2, (HEIGHT-1) * WIDTH * 2 );
	
	for ( j = 0; j < (WIDTH * 2); j += 2 )
	{
		screen[ (HEIGHT - 1) * WIDTH * 2 + j ] = ' ';
		screen[ (HEIGHT - 1) * WIDTH * 2 + j + 1 ] = SCREEN_ATTR;
	}
}


/*
static void clear() 
{
  int i,j;

     for( i = 0; i < HEIGHT; i++)
      for( j = 0; j < WIDTH; j++ )
      {
        screen[i * WIDTH * 2 + j * 2] = ' ';
        screen[i * WIDTH * 2 + j * 2 +1] = SCREEN_ATTR;	
      }

	s_x = 0;
	s_y = 0;
}
*/


static void putc(int x, int y, char c)
{
	screen[ y * WIDTH * 2 + x * 2] = c;
	screen[ y * WIDTH * 2 + x * 2 + 1 ] = SCREEN_ATTR;
}


static void puts_xy( int *x, int *y, const char *s )
{
    int i = 0;

    while(s[i] != '\0') 
	{

		if ( *x >= WIDTH )
		{
			*x  = 0;
			*y += 1;
		}

		if ( *y >= HEIGHT )
		{
			scroll();
			*y -= 1;
		}
			
			
		switch (s[i])
		{
			case '\n':
					*x  = 0;
					*y += 1;
					break;
				
			default:
					putc( *x, *y, s[i] );
					*x += 1;
					break;
		}
		
		i += 1;
	}
}


void puts( const char *str )
{
	puts_xy( &s_x, &s_y, str );

	set_cursor( s_x, s_y );
}




