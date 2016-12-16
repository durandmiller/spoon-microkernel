#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>


#include "base/globals.h"


#ifndef UNIX
#include <devfs.h>
#endif


int	map_screen()
{
#ifndef UNIX
	int rc = 0;

	global_Device_fd = devfs_open( global_Device, 
									DEVFS_FLAG_READ | 
									DEVFS_FLAG_WRITE |
									DEVFS_FLAG_MAP |
									DEVFS_FLAG_UNMAP );
	if ( global_Device_fd < 0 ) 
	{
		devfs_dmesg( "devfs_open returned %i\n", global_Device_fd );
		return -1;
	}

	rc = devfs_map( global_Device_fd,
				0,
				global_Width * global_Height * sizeof(uint32_t),
				DEVFS_FLAG_READ | DEVFS_FLAG_WRITE,
				(void**)&global_Screen );
	if ( rc != 0 )	
	{
		devfs_dmesg( "devfs_map returned %i\n", rc );
		return -1;
	}
#else
	global_Device_fd = open( global_Device, O_RDWR );
	if ( global_Device_fd < 0 ) return -1;

	global_Screen = mmap( NULL, 
							global_Width * global_Height * sizeof(uint32_t),
							PROT_READ | PROT_WRITE,
							MAP_SHARED,
							global_Device_fd,
							0 );
#endif

	if ( global_Screen == NULL )
	{
		perror( "unable to mmap device" );
		return -1;
	}


	return 0;
} 


void generic_shutdown()
{
	if ( global_Screen != NULL )
	{
		munmap( global_Screen, 
				global_Width * global_Height * sizeof(uint32_t) );

		global_Screen = NULL;
	}

	if ( global_Device_fd >= 0 )
	{
		close( global_Device_fd );
		global_Device_fd = -1;
	}


	if ( global_Device != NULL )
	{
		free( global_Device );
		global_Device = NULL;
	}
}


