#ifndef UNIX
#include <smk.h>
#include <comms.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



#include "os.h"
#include "mouse.h"
#include "base/globals.h"
#include "ops.h"
#include "window.h"
#include "desktop.h"


int parse_args( int argc, char *argv[] )
{
	int c;


	while ((c = getopt( argc, argv, "w:h:f:" )) != -1 )
	{
		switch (c)
		{
			case 'w':
					global_Width = atoi( optarg );
					break;
			case 'h':
					global_Height = atoi( optarg );
					break;
			case 'f':
					global_Device = strdup( optarg );
					break;

		}
	}

	return 0;
}


#ifndef UNIX
void showme( uint32_t color, int error )
{
	static uint32_t* 	ptr = NULL;
	static uint32_t 	width = 0;
	static uint32_t 	height = 0;
	static uint32_t 	size = 0;

	int i,j;

	if ( ptr == NULL )
	{
		smk_get_lfb( &ptr, &width, &height );
		size = width * height * 4;
		smk_request_iopriv();
		ptr = (uint32_t*)smk_mem_map( 
							(void*)ptr, 
							(size / 4096) + 1 );
	}

	for ( i = 0; i < (width * height); i++ )
		ptr[ i ] = color;

	if ( error < 0 ) error = -error;

	for ( i = 0; i < error; i++ )
	 for ( j = 0; j < 300; j++ )
		ptr[ (i*5) * width + j ] = ~color;
}
#endif

int message_handler( void *data, int pid, int port, void *packet )
{
	struct gui_packet *gs = (struct gui_packet*)packet;

	switch (gs->operation)
	{
		case GUI_CREATE:	return ops_create( pid, port, packet );
		case GUI_DESTROY:	return ops_destroy( pid, port, packet );
	}

	return -1;
}

int pulse_handler( void *data, int pid, int port, void *packet )
{
	unsigned int* info = (unsigned int*)packet;

	switch (info[0])
	{
		case GUI_SET:  		return ops_set( pid, port, packet );
		case GUI_UPDATE: 	return ops_set( pid, port, packet );
		case GUI_SYNC:		return ops_sync( pid, port, packet );
	}

	return -1;
}


int main( int argc, char *argv[] )
{
	comms_t		*comms = NULL;

	atexit( generic_shutdown );

#ifndef UNIX
	smk_go_dormant_t( 1000 );
#endif


	if ( parse_args( argc, argv ) != 0 )
	{
		fprintf( stderr, "unable to parse arguments\n" );
		return -1;
	}

	if ( map_screen() != 0 )
	{
		fprintf( stderr, "unable to open device %s\n", global_Device );
		return -1;
	}


	comms_create( &comms, NULL );

	windows_init();

	desktop_init();

	init_mouse();

	acquire_mouse( "input/ps2mouse" );


	comms_register( comms, COMMS_MESSAGE, COMMS_DEFAULT, message_handler );
	comms_register( comms, COMMS_PULSE, COMMS_DEFAULT, pulse_handler );


	comms_start( comms );
	comms_wait( comms, 0 );

#warning Should be comms wait.
	smk_go_dormant();

	comms_destroy( &comms );
	return 0;
}



