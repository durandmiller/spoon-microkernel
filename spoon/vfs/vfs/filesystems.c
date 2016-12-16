#include <smk.h>
#include <devfs.h>
#include <vfs.h>

#include "filesystems.h"

#include "ext2/ext2.h"
#include "iso9660/iso9660.h"


/** This is the structure containing a list of all
 * the supported filesystems. The entries should be
 * defined as macros in the relevant header files.
 */
static struct filesystem *filesystems[] =
	{ &iso9660_fs, 
	  &ext2_fs,
	  NULL };


int init_filesystems()
{
	return 0;
}


/** 
 *
 *
 * \return NULL if the FS is unknown
 * \return a filesystem struct if known
 */
struct filesystem* auto_fsprobe( int did )
{
	int i = 0;


	while (filesystems[i] != NULL)
	{
		if ( filesystems[i]->probe != NULL )
		{
			if ( filesystems[i]->probe( did ) == 0 )
				return filesystems[i];
		}
			
		i++;
	}
	
	return NULL;
}


struct filesystem* fsprobe( int did, const char *fs )
{
	int i = 0;


	while (filesystems[i] != NULL)
	{
		if ( devfs_strcmp( filesystems[i]->name, fs ) == 0 )
		{
			if ( filesystems[i]->probe == NULL ) return NULL;

			if ( filesystems[i]->probe( did ) == 0 )
				return filesystems[i];

			return NULL;
		}
			
		i++;
	}
	
	return NULL;
}


