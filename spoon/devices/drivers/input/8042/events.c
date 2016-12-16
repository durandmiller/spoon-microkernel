#include <smk.h>
#include <devfs.h>

#include "8042.h"
#include "devices.h"
#include "events.h"

#define TOTAL_INPUTS	1024

struct input_event
{
	int type;
	int a;
	int b;
	int c;
	int d;
};


static struct input_event events[ TOTAL_INPUTS ];
static int events_start = 0; 
static int events_stop = 0; 


int add_event( int type, int a, int b, int c, int d )
{
	events[events_stop].type = type;
	events[events_stop].a = a;
	events[events_stop].b = b;
	events[events_stop].c = c;
	events[events_stop].d = d;

	events_stop = (events_stop + 1) % TOTAL_INPUTS;

	if ( events_start == ((events_stop + 2) % TOTAL_INPUTS) )
		return 1;

	if ( events_start == ((events_stop + 1) % TOTAL_INPUTS) )
		return 1;

	return 0;
}


void process_events()
{
	while ( events_start != events_stop )
	{
		int did = -1;
	
		switch ( events[events_start].type )
		{
			case KEYBOARD_DATA: did = dev_kbd;	break;
			case MOUSE_DATA: did = dev_mouse; break;
		}
	

		devfs_push_fast( did, 
						events[events_start].a,
						events[events_start].b,
						events[events_start].c,
						events[events_start].d );
	
		events_start = (events_start + 1) % TOTAL_INPUTS;
	}
}




