#include <smk.h>
#include <comms.h>
#include <devfs.h>
#include <vfs.h>

#include "globals.h"


#define		DRIVER_PATH		"/spoon/system/drivers/"

#define		MAX_APPS		32
static 	struct process_information	apps[MAX_APPS];
static	int							pids[MAX_APPS];
static	int							max = 0;




int		start_server( const char *serverName )
{
	int rc;

	int pid = smk_find_process_id( serverName );
	if ( pid < 0 ) return -1;

	rc = smk_start_process( pid );
	if ( rc != 0 ) return -1;

	rc = comms_test_pid( pid, 0, 2000 );	// 2 seconds is okay.
	if ( rc != 0 ) return -1;

	for ( rc = 0; rc < max; rc++ )
		if ( pids[rc] == pid ) 
				pids[rc] = -1;

	return 0;
}


int		start_drivers()
{
	int count = 0;
	int i;

	for ( i = 0; i < max; i++ )
	{
		if ( devfs_memcmp( apps[i].cmdline, 
							 DRIVER_PATH, 
							 devfs_strlen(DRIVER_PATH) ) == 0 )
		{
			pids[i] = -1;

			smk_start_process( apps[i].pid );
			count += 1;
		}
	}

	return count;
}


int	do_mount( char *device )
{
	int rc = -1;
	int count = 0;
	
	while ( (rc != 0) && (count++ < 100) )
	{
		rc = vfs_mount( device, "/", "auto", VFS_FLAG_ALL ); 
		if ( rc == 0 ) break;

		smk_go_dormant_t( 100 );

		if ( (count % 10) == 0 ) devfs_dmesg(".");
	}


	devfs_dmesg("\n");
	return rc;
}


int	display_list( char *pattern )
{
	struct devfs_matchinfo **list = NULL;
	int rc = devfs_match( pattern, &list );

	devfs_dmesg( "Registered device drivers:\n" );

	if ( rc >= 0 )
	{
		int i = 0;
		while ( list[i] != NULL )
		{
			devfs_dmesg( "  %s\n", list[i]->name );
			smk_free( list[i] );
			i += 1;
		}

		smk_free( list[i] );
		return 0;
	}
	else
	{
			devfs_dmesg( "There was a problem getting the device driver list!!\n" );
	}

	return -1;
}





int main( int argc, char *argv[])
{
	int i;

	max = smk_process_get_list( MAX_APPS, apps );
	for ( i = 0; i < max; i++ ) 
	{
		pids[i] = apps[i].pid;
		if ( devfs_strcmp( apps[i].name, "smk" ) == 0 )
				pids[i] = -1;

		if ( devfs_strcmp( apps[i].name, "bootstrap" ) == 0 )
				pids[i] = -1;
	}


	start_server( "devfs" );
	start_drivers();
	start_server( "vfs" );


	if ( parse_opts( argc, argv ) != 0 )
	{
		devfs_dmesg( "bootstrap unable to parse command line. Check it.\n" );
		print_usage();
		smk_go_dormant();
	}

	smk_go_dormant_t( startup_wait );

	if ( list_pattern != NULL )
	{
		display_list( list_pattern );
		smk_go_dormant();
	}

	if ( root_device != NULL )
	{
		if ( do_mount( root_device ) != 0 )
		{
			devfs_dmesg( "bootstrap unable to mount root device [%s]\n", 
							root_device );
			smk_go_dormant();
		}
	}
	

	// start the rest.
	for ( i = 0; i < max; i++ ) 
		if ( pids[i] >= 0 )
			smk_start_process( pids[i] );

	
	smk_go_dormant();
	return 0;
}



