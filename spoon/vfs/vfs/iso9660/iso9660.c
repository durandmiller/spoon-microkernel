#include <smk.h>
#include <devfs.h>
#include <vfs.h>

#include "iso9660.h"
#include "iso9660_defs.h"



// -------- Forward declarations ---------------------
static struct primary_volume_descriptor* iso9660_getPrimary( int fd );
static struct directory_record* iso9660_run_dirs(  int fd,
													char *match,
        						 					int block, 
													int offset, 
													int size );
static int iso9660_insensitive_match( char *src, char *dest );


// -------- Implementation ---------------------------

int iso9660_probe( int fd )
{
	char block[2048];
	long long rc;
	struct volume_descriptor *vd = (struct volume_descriptor*)block;
  
	if ( (rc = devfs_read( fd, 0x8000, block, 2048 )) != 2048 ) 
	{
		devfs_dmesg("iso9660: unable to read block (%i)\n", rc );
		return -1;
	}

	if ( devfs_memcmp( (vd->identifier), "CD001", 5 ) != 0 ) 
	{
		devfs_dmesg("iso9660: identifier not found\n" );
		return -1;
	}

	return 0;
}


/** Initializes the primary volume descriptor */
int iso9660_init( int fd, void** data )
{
  struct primary_volume_descriptor *p = iso9660_getPrimary( fd );
  if ( p == NULL ) return -1;
  
  *data = p;
  return 0;
}

/** Frees the primary volume descriptor */
int iso9660_shutdown( int fd, void* data )
{
	smk_free( data );
	return 0;
}


long long iso9660_read( void *data, int fd, void *fdata, 
						unsigned long long position,
						void *buffer, long long len )
{
	long long req, new_pos, ans;
	struct directory_record *dent = (struct directory_record*)fdata;

	req = dent->data_length_msb - (int)position;
	if ( len < req ) req = len;

	if ( req < 0 ) return -1;
	if ( req == 0 ) return 0;
	
	new_pos = dent->location_extent_msb * 2048 + position;
	
	ans = devfs_read( fd, new_pos, buffer, req );

	return ans;
}

long long iso9660_write( void *data, int fd, void *fdata, 
							unsigned long long position,
							void *buffer, long long len )
{
	return -1;
}


/*
int fs_iso9660::List( char *node, Message **list )
{
   struct directory_record *dir;
   struct directory_record *tmp;
   bool root = true;
       *list = NULL;
   
   
   if ( strcmp(node,"") == 0 ) 
   	dir = (struct directory_record*) (&(pvd.root_directory_record) );
	   else 
	     {
		dir = findFile( node );
		root = false;
	     }

   if ( dir == NULL ) return -1;

   int size = dir->data_length_msb;
   int position = 0;
   
   Message *msg = new Message(OK);
   
   char *data = (char*)malloc( size );

    if ( dev_read( dir->location_extent_msb * 2048, data, size ) == size ) 
    {
      int count = 0;
      while ( position <= size )
      {
        tmp = (struct directory_record*)(data + position);
	if ( tmp->length == 0 ) break;

        char *name = (char*)strndup( (char*)tmp->file_id, tmp->file_id_len );
	  

	  for ( unsigned int i = 0; i < strlen(name); i++ )
	  	if ( name[i] == ';' ) 
		{
			name[i] = 0;
			break;
		}

	  // 12-02-2005 added quick support for files which don't have extensions
	  // and yet, whatever application created the CD, now magically do. Probably
	  // mkisofs

	  int len = strlen(name);
	  for ( int i = len - 1; i >= 0; i-- )
	    {
		if ( name[i] == '.' ) { name[i] = 0; break; }
		if ( (name[i] != '.') && (name[i] != ' ') ) break;
	    }

	  // finished support...  this might cause problems later on.
	  

	if ( count == 0 ) {
			     free( name );
			     name = strdup(".");
	  	  	   }
	if ( count == 1 ) {
	  		     free( name );
			     name = strdup("..");
			  }
	count++;

	msg->AddString("entry", name );

	free( name );

	position += tmp->length;
      }
    }
    else msg->what = ERROR;

   free( data );

   if ( ! root ) free(dir);

   *list = msg;

   return 0;
}


// --------------------------------------------------------
*/

int iso9660_create( void *data, int fd, const char *filename,
						unsigned int mode )
{
	return -1;
}


int iso9660_open( void *data, int fd, const char *filename, void **fdata )
{
    int prev = 0;
    int last = 0;
	unsigned int i;
	struct primary_volume_descriptor *pvd = 
			(struct primary_volume_descriptor*)data; 

	for ( i = devfs_strlen(filename)-1; i > 0; i-- )
		if ( filename[i] == '/' ) 
		{
			last = i+1;
			break;
		}

    struct directory_record *dir = 
    	(struct directory_record *)&(pvd->root_directory_record); 

    for ( i = 0; i <= devfs_strlen(filename); i++ )
    {
      if ( (filename[i] == '/') || (i == devfs_strlen(filename)) ) 
      {
        char *dn = (char*)devfs_strndup( filename + prev, i - prev );

		 struct directory_record *old = dir;
		 dir = iso9660_run_dirs( fd,
						 		 dn, dir->location_extent_msb, i,
		 					     dir->data_length_msb );

		 if ( prev != 0 ) smk_free( old );
		 smk_free( dn );
		 
		 if ( dir == NULL )  return -1;	// didn't find
		 if ( prev == last )  			// We found it!
		 {
			*fdata = dir;
			return 0;
		 }
 		 if (  (dir->flags & DIR_FLAG) != DIR_FLAG ) return -1;
		 		// we can't go further

		prev = i + 1;	// go to next dir
      }
    }

    return -1;
}


int iso9660_close( void *data, int fd, void *fdata )
{
	if ( fdata != NULL ) smk_free( fdata );
	return 0;
}



int iso9660_stat( void *data, int fd, const char *filename, struct vfsstat *st )
{
	void *fdata;
	struct directory_record *dent;
		
	int rc = iso9660_open( data, fd, filename, &fdata );
	if ( rc != 0 ) return rc;

	 	dent = (struct directory_record*)fdata;

		st->filename = NULL;
		st->size = dent->data_length_msb;
		st->did = fd;
		st->uid = 0;
		st->gid = 0;
		st->nlinks = 0;
		st->blocksize = 2048;
		st->inode = 0;
		st->blocks = 0;
		st->atime = 0;
		st->mtime = 0;
		st->ctime = 0;
		st->mode = 0;

		if ( (dent->flags & DIR_FLAG) == DIR_FLAG ) st->mode |= VFS_S_IFDIR;


	iso9660_close( data, fd, fdata );
	return 0;
}


int iso9660_rm( void *data, int fd, const char *filename )
{
	return -1;
}

int iso9660_mkdir( void *data, int fd, const char *filename )
{
	return -1;
}


int iso9660_chmod( void *data, int fd, const char *filename, unsigned int mode )
{
	return -1;
}


int iso9660_ls( void *data, int fd, const char *pathname, 
					struct vfsstat *list, int *max )
{
   struct directory_record *dir;
   struct directory_record *tmp;
   int root = 1;
   int position =0;
   int count = 0;
   int size;
   char *tmpdata;
   struct primary_volume_descriptor *pvd = 
			(struct primary_volume_descriptor*)data; 
   
   
   // First, get out directory record of the directory they want.
   if ( (pathname == NULL) || (devfs_strcmp(pathname,"") == 0) ||
		(devfs_strcmp(pathname,"/") == 0) ) 
   	dir = (struct directory_record*) (&(pvd->root_directory_record) );
	   else 
	     {
			if ( iso9660_open( data, fd, pathname, (void**)&dir ) != 0 )
			{
				dir = NULL;
			}
			root = 0;
	     }

   if ( dir == NULL ) return -1;

   // Sounds good. Read the directory list.
   size = dir->data_length_msb;
   tmpdata = (char*)smk_malloc( size );	// Buffer for data.

    if ( devfs_read( fd, dir->location_extent_msb*2048, tmpdata, size ) != size ) 
	{
		smk_free( tmpdata );
		if ( root == 0 ) smk_free( dir );
		return -1;
	}
	

	// Run through the list we have and add in information
      while ( position <= size )
      {
		char *name = NULL;
		int len;
		int i;
			  
        tmp = (struct directory_record*)(tmpdata + position);
		if ( tmp->length == 0 ) break;

        name = (char*)devfs_strndup( (char*)tmp->file_id, 
											tmp->file_id_len );
	  

		for ( i = 0; i < devfs_strlen(name); i++ )
			if ( name[i] == ';' ) 
			{
				name[i] = 0;
				break;
			}

	  // 12-02-2005 added quick support for files which don't have extensions
	  // and yet, whatever application created the CD, now magically do. 
	  // Probably mkisofs
	  // See previous OOPS message.

		  len = devfs_strlen(name);
		  for ( i = len - 1; i >= 0; i-- )
		    {
				if ( name[i] == '.' ) { name[i] = 0; break; }
				if ( (name[i] != '.') && (name[i] != ' ') ) break;
		    }

	  // finished support...  this might cause problems later on.
	  

			if ( count == 0 ) {
					     smk_free( name );
					     name = devfs_strdup(".");
	  	  	   }
			if ( count == 1 ) {
	  			     smk_free( name );
				     name = devfs_strdup("..");
			  }

		// Okay! We have a name!
			
			list[count].filename = name;
			list[count].size = tmp->data_length_msb;
			list[count].did  = fd; 
			list[count].mode = 0;
			list[count].gid  = 0;
			list[count].uid  = 0; 
			
			
		// -----------
		if ( ++count == *max ) break;
		position += tmp->length;
      }

	*max = count;
	
	smk_free( tmpdata );

	if ( root == 0 ) smk_free(dir);
	return 0;
}


static struct primary_volume_descriptor* iso9660_getPrimary( int fd )
{
    struct volume_descriptor *p = 
	    (struct volume_descriptor *)
 		    smk_malloc( sizeof(struct volume_descriptor)) ;

    int offset = 0;
    while ( devfs_read( fd, 0x8000 + offset, (void*)p, 2048 ) == 2048 ) 
    {
       if ( p->type == VD_TYPE_PRIMARY ) 
         return (struct primary_volume_descriptor*)p;
       if ( p->type == VD_TYPE_TERMINATOR ) break;

       offset += 2048;	// ok, next sector;
       
       if ( offset > (2048 * 300) ) break;	// i dunno.
    }

    smk_free( p );
    return NULL;
}

static struct directory_record* iso9660_run_dirs(  int fd, 
													char *match,
        						 					int block, 
													int offset, 
													int size )
{
	int position = 0;
	char *data = (char*)smk_malloc(size);

	if ( devfs_read( fd, block * 2048, data, size ) != size )
	{
		smk_free( data );
		return NULL;
	}
        
	struct directory_record *dir = (struct directory_record*)data;

	int count = 0;
	while ( (position < size) )
	{
		char *name = NULL;
		unsigned int i;
			
	       dir = (struct directory_record*)(data + position);
	       if ( dir->length == 0 ) break;
	       position += dir->length;

	       if (count++ < 2) continue;

	       name = (char*)devfs_strndup( (char*)dir->file_id, dir->file_id_len );

			for ( i = 0; i < devfs_strlen(name); i++ )
				if ( name[i] == ';' ) 
				{
					name[i] = 0;
					break;
				}

		  	  // 12-02-2005 added quick support for files which don't 
			  // have extensions and yet, whatever application created 
			  // the CD, now magically do. Probably mkisofs.
			  // -- 2006-07-07 OOPS! This is actually the versioning on 
			  // the CD.

			  int len = devfs_strlen(name);
			  for ( i = len - 1; i >= 0; i-- )
			    {
				if ( name[i] == '.' ) { name[i] = 0; break; }
				if ( (name[i] != '.') && (name[i] != ' ') ) break;
			    }

			  // finished support...  this might cause problems later on.
	
		  
		 if ( iso9660_insensitive_match( name, match ) == 0 )
		  {
		    smk_free( name );
		    struct directory_record *de = (struct directory_record*)
		    	smk_malloc( sizeof(struct directory_record) );
		    devfs_memcpy( de, dir, dir->length );
		    smk_free( data );
		    return de;
		  }

	       smk_free( name );
	}
	

	smk_free( data );
	return NULL;
}


static int tolower( int c )
{
	if ( (c >= 'A') && (c <= 'Z') ) return (c-'A'+'a');
	return c;
}

static int iso9660_insensitive_match( char *src, char *dest )
{
	unsigned int i;
	int len = devfs_strlen( src );

	if ( devfs_strlen( dest ) != len ) return -1;
   
	for ( i = 0; i < len; i++ ) 
		if ( tolower( src[i] ) != tolower( dest[i] ) ) return -1;

	return 0;
}


/** The ISO9660 filesystem structure which points to the
 * correct function hooks
 */

struct filesystem iso9660_fs =
		{ "iso9660",
		  "iso9660 file systems for CDs",
		  	iso9660_probe, 
			iso9660_init, 
			iso9660_shutdown,
			iso9660_create,
			iso9660_open,
			iso9660_close,
			iso9660_read,
			iso9660_write,
			iso9660_rm, 
			iso9660_mkdir, 
			iso9660_stat,
			iso9660_chmod,
			iso9660_ls };									




