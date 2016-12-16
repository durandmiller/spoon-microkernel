#include <smk.h>
#include <devfs.h>


#include "support.h"
#include "globals.h"

char*			root_device  = NULL;
char*			list_pattern = NULL;
unsigned int	startup_wait = 100;



int	parse_opts( int argc, char *argv[] )
{
	int position = 0;


	while ( ++position < argc )
	{
		if ( devfs_strncmp( argv[position], "list=", devfs_strlen("list=") ) == 0 )
		{
			list_pattern = argv[position] + devfs_strlen("list=");
			continue;
		}


		if ( devfs_strcmp( argv[position], "help" ) == 0 )
		{
			print_usage();
			smk_go_dormant();
			continue;
		}

		if ( devfs_strncmp( argv[position], "wait=", devfs_strlen("wait=") ) == 0 )
		{
			startup_wait = atoi( argv[position] + devfs_strlen("wait=") );
			if ( startup_wait < 0 ) return -1;
		}
	

		if ( devfs_strncmp( argv[position], "root=", devfs_strlen("root=") ) == 0 )
		{
			root_device = argv[position] + devfs_strlen("root=");
			continue;
		}

	}

	return 0;
}


int print_usage()
{
	devfs_dmesg( "usage: bootstrap root=<device name> wait=<ms> list help \n" );
	devfs_dmesg( "    root=<device name>    - mount the device as the root entry using fs auto\n" );
	devfs_dmesg( "    wait=<ms>             - wait ms milliseconds after starting servers and drivers\n" );
	devfs_dmesg( "    list=<pattern>        - list all registered device drivers. (Follows wait). Try */*.\n" );
	devfs_dmesg( "    help                  - display this message\n" );
	return 0;
}


