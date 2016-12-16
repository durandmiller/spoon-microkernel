#include <smk.h>
#include <devfs.h>
#include <vfs.h>

#include "comms.h"
#include "filesystems.h"
#include "mounts.h"
#include "files.h"
#include "processes.h"


int main( int argc, char *argv[] )
{
	struct vfs_packet *vs = NULL;

	if ( init_processes() != 0 )
		return -1;

	if ( init_filesystems() != 0 )
		return -1;

	if ( init_mounts() != 0 ) 
		return -1;

	if ( smk_create_port( 0, smk_gettid() ) < 0 )
		return -1;


	while (1==1) 
	{
		int spid, sport, dport = -1;
		int bytes;
		int data1, data2, data3, data4, data5, data6;
		
		smk_go_dormant();
				

		if ( smk_peek_message( &spid, &sport, &dport, &bytes ) == 0 )
		{

			// Ensure the correct size
			if ( bytes < VFS_PACKETSIZE )
			{
				smk_drop_message();
				continue;
			}

		
			// Ensure memory enough to process it.
			vs = (struct vfs_packet*)smk_realloc( vs, bytes );
			if ( vs == NULL ) 
			{
				smk_drop_message();
				continue;
			}

			smk_recv_message( &spid, &sport, &dport, bytes, vs );



			vs->pid = spid;		// No spoofing when talking to vfs


			switch (vs->operation)
			{
				case VFS_READ:
					read( spid, sport, vs );
					break;

				case VFS_WRITE:
					write( spid, sport, vs );
					break;

				case VFS_LS:
					ls( spid, sport, vs );
					break;

				case VFS_STAT:
					stat( spid, sport, vs );
					break;

				case VFS_RM:
					rm( spid, sport, vs );
					break;

				case VFS_CREATE:
					create( spid, sport, vs );
					break;

				case VFS_MKDIR:
					mkdir( spid, sport, vs );
					break;

				case VFS_CHMOD:
					chmod( spid, sport, vs );
					break;
					
				case VFS_OPEN:
					open( spid, sport, vs );
					break;

				case VFS_CLOSE:
					close( spid, sport, vs );
					break;
					
				case VFS_MOUNT:
					prep_mount( spid, sport, vs );
					break;

				case VFS_UNMOUNT:
					prep_unmount( spid, sport, vs );
					break;
						
				default:
					send_resp( spid, sport, vs, VFSERR_NOTSUPPORTEDOP );
					break;
			}

		}
		else
		{
			if ( smk_recv_pulse( &spid, &sport, &dport,
							(unsigned int*)&data1, 
							(unsigned int*)&data2, 
							(unsigned int*)&data3,
							(unsigned int*)&data4, 
							(unsigned int*)&data5, 
							(unsigned int*)&data6 ) != 0 ) continue;

			switch (data1)
			{
				/*
				case DEVFS_RESPONSE:
					process_response_pulse( spid, (int)data2, (int)data3 );
					break;

				case DEVFS_PUSH:
					dispath_pulse_push( spid, data2,
										data3, data4, data5, data6 );
					break;
				*/
			}
		}


	}

		
	smk_release_port( 0 );

	if ( vs != NULL ) smk_free( vs );
	return 0;
}



