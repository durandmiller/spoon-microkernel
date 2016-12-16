#include <stdlib.h>
#include <smk.h>
#include <devfs.h>

#include <gui.h>


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



int main( int argc, char *argv[])
{
	wid_t *myWindow;
	int rc = 0;


	smk_go_dormant_t( 2000 );

	rc = gui_create_window( &myWindow,
							10, 50,
							200, 20,
							GUI_FLAGS_DEFAULT,
							GUI_STATE_DEFAULT,
							GUI_EVENT_DEFAULT );

	if ( rc != 0 )
	{
		devfs_dmesg( "gui_test: unable to create window!!\n" );
		return -1;
	}

	gui_set_state( myWindow, gui_get_state( myWindow ) & ~GUI_STATE_HIDDEN );

	while (1==1)
	{
		smk_go_dormant();
	}


	gui_destroy_window( myWindow );

	smk_go_dormant();
	return EXIT_SUCCESS;
}



