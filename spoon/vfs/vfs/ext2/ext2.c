#include <smk.h>
#include <devfs.h>
#include <vfs.h>

#include "ext2.h"
#include "../filesystems.h"

static int ext2_create_volume( int fd, ext2_vol **vol_desc ) {
  ext2_vol *vol = ( ext2_vol* )smk_malloc( sizeof( ext2_vol ) );
  if ( !vol ) {
    devfs_dmesg( "ext2: Out of memory allocatin vol\n" );
    return -1;
  }

  vol->fd = fd;
  vol->super = ( ext2_superblock* )smk_malloc( sizeof( ext2_superblock ) );
  if ( vol->super == NULL ) {
    smk_free( vol );
    devfs_dmesg( "ext2: Out of memory allocatin vol->super\n" );
    return -1;
  }

  if ( devfs_read( vol->fd, 1024, ( void* )vol->super, sizeof( ext2_superblock ) ) != sizeof( ext2_superblock ) ) {
    smk_free( vol );
    smk_free( vol->super );
    devfs_dmesg( "ext2: Failed to load the superblock\n" );
    return -1;
  }

  if ( vol->super->s_magic == EXT2_MAGIC ) {
    char* inodes = NULL;

    vol->inodes = vol->super->s_inodes_count;
    vol->blocks = vol->super->s_blocks_count;
    vol->r_blocks = vol->super->s_r_blocks_count;
    vol->free_blocks = vol->super->s_free_blocks_count;
    vol->free_inodes = vol->super->s_free_inodes_count;
    vol->b_size = 1024 << vol->super->s_log_block_size;
    if ( vol->super->s_log_frag_size < 0 ) {
      vol->f_size = 1024 >> -vol->super->s_log_block_size;
    } else {
      vol->f_size = 1024 << vol->super->s_log_block_size;
    }
    vol->bpg = vol->super->s_blocks_per_group;
    vol->fpg = vol->super->s_frags_per_group;
    vol->ipg = vol->super->s_inodes_per_group;
    vol->mtime = vol->super->s_mtime;
    vol->root_inode = 2;

    vol->fpb = vol->b_size / vol->f_size;
    vol->ipb = vol->b_size / sizeof( _ext2_inode );
    vol->itb_pg = vol->ipg / vol->ipb;
    vol->groups = vol->blocks / vol->bpg;

    if ( vol->ipb ) {
      vol->ipb_bits = 1;
      while ( vol->ipb >> vol->ipb_bits ) {
        if ( ( ( vol->ipb >> vol->ipb_bits ) & 1 ) && ( (vol->ipb >> vol->ipb_bits) & ~1 ) ) {
          devfs_dmesg( "ext2: inodes_per_block is not properly aligned! (%i)\n", vol->ipb );
          smk_free( vol->super );
          smk_free( vol );
          return -1;
        }

        vol->ipb_bits++;
      }
      vol->ipb_bits -= 1;
    } else {
      smk_free( vol->super );
      smk_free( vol );
      devfs_dmesg( "ext2: inodes_per_block = 0! What's this?\n" );
      return -1;
    }
    vol->ipb_mask = vol->ipb - 1;

    vol->grouptable = ( ext2_groupblock* )smk_malloc( vol->b_size );
    if ( !vol->grouptable ) {
      smk_free( vol->super );
      smk_free( vol );
      devfs_dmesg( "ext2: Out of memory allocatin vol->grouptable\n" );
      return -1;
    }

    if ( devfs_read( vol->fd, ( vol->super->s_first_data_block + 1 ) * vol->b_size, ( void* )vol->grouptable, vol->b_size ) != vol->b_size ) {
      smk_free( vol->grouptable );
      smk_free( vol->super );
      smk_free( vol );
      devfs_dmesg( "ext2: Failed to load grouptable\n" );
      return -1;
    }

    inodes = ( char* )smk_malloc( vol->b_size );
    if ( inodes == NULL ) {
      smk_free( vol->grouptable );
      smk_free( vol->super );
      smk_free( vol );
      devfs_dmesg( "ext2: Out of memory allocatin inodes\n" );
      return -1;
    }

    vol->root = ( ext2_inode* )smk_malloc( sizeof( ext2_inode ) );
    if ( vol->root == NULL ) {
      smk_free( vol->grouptable );
      smk_free( vol->super );
      smk_free( vol );
      devfs_dmesg( "ext2: Out of memory allocatin vol->root\n" );
      smk_free( inodes );
      return -1;
    }

    if ( devfs_read( vol->fd, vol->grouptable[ 0 ].inode_table * vol->b_size, inodes, vol->b_size ) != vol->b_size ) {
      smk_free( vol->root );
      smk_free( vol->grouptable );
      smk_free( vol->super );
      smk_free( vol );
      devfs_dmesg( "ext2: Out of memory allocatin vol->root\n" );
      smk_free( inodes );
      return -1;
    }

    if ( vol->super->s_rev_level == 1 ) {
      vol->i_size = vol->super->s_inode_size;
    } else {
      vol->i_size = 128;
    }

    devfs_memcpy( vol->root, inodes + sizeof( _ext2_inode ), sizeof( _ext2_inode ) );
    vol->root->id = vol->root_inode;
    smk_free( inodes );

    *vol_desc = vol;
    return 0;
  } else {
    smk_free( vol->super );
    smk_free( vol );
    return -1;
  }
}

static int ext2_probe( int fd ) {
  ext2_vol *vol = NULL;
  if ( ext2_create_volume( fd, &vol ) == 0 ) {
    smk_free( vol->root );
    smk_free( vol->super );
    smk_free( vol->grouptable );
    smk_free( vol );
    return 0;
  } else {
    return -1;
  }
}

static int ext2_init( int fd, void** data ) {
  ext2_vol *vol = NULL;
  if ( ext2_create_volume( fd, &vol ) == 0 ) {
    *data = vol;
    return 0;
  } else {
    return -1;
  }
}

static int ext2_shutdown( int fd, void* data ) {
  ext2_vol *vol = ( ext2_vol* )data;
  if ( vol == NULL ) {
    devfs_dmesg( "ext2: Volume descriptor is corrupt!\n" );
    return -1;
  }

  smk_free( vol->root );
  smk_free( vol->super );
  smk_free( vol->grouptable );
  smk_free( vol );

  return 0;
}

static unsigned long ext2_gbfbtab( int fd, unsigned long tab, unsigned long b_size, unsigned long index ) {
  unsigned long bpb = b_size >> 2;
  unsigned long blocktab[ b_size ];

  if ( index >= bpb ) {
    return -1;
  }

  if ( devfs_read( fd, tab * b_size, ( void* )blocktab, b_size ) != b_size ) {
    return -1;
  }

  return blocktab[ index ];
}

static unsigned long ext2_get_block( int fd, ext2_inode *node, unsigned long b_size, unsigned long index ) {
  unsigned long block;
  unsigned long bpb = b_size >> 2;

  if ( index < 12 ) {
    block = node->n.block[ index ];
  } else if ( ( index >= 12 ) && ( index < ( 12 + bpb ) ) ) {
    block = ext2_gbfbtab( fd, node->n.block[ 12 ], b_size, index - 12 );
  } else if ( ( index >= ( 12 + bpb ) ) && ( index < ( 12 + bpb + bpb * bpb ) ) ) {
    block = ext2_gbfbtab( fd, node->n.block[ 13 ], b_size, ( index - 12 - bpb ) / bpb );
    if ( block < 0 ) { return block; }
    block = ext2_gbfbtab( fd, block, b_size, ( index - 12 - bpb ) % bpb );
  } else {
    block = ext2_gbfbtab( fd, node->n.block[ 14 ], b_size, ( index - 12 - bpb - bpb * bpb ) / ( bpb * bpb ) );
    if ( block < 0 ) { return block; }
    block = ext2_gbfbtab( fd, block, b_size, ( ( index - 12 - bpb - bpb * bpb ) / bpb ) % bpb );
    if ( block < 0 ) { return block; }
    block = ext2_gbfbtab( fd, block, b_size, ( index - 12 - bpb - bpb * bpb ) % bpb );
  }

  return block;
}

static int ext2_walk_directory( void *data, void *parent_inode, const char *entry_name, int entry_name_len, unsigned long *inode ) {
  ext2_vol *vol = ( ext2_vol* )data;
  ext2_inode *node = ( ext2_inode* )parent_inode;

  if ( ( vol == NULL ) || ( node == NULL ) ) {
    devfs_dmesg( "ext2: Volume descriptor or Inode is corrupt!\n" );
    return -1;
  }

  int rc = -1;

  if ( node->n.mode & EXT2_S_IFDIR ) {
    char* dirtab = ( char* )smk_malloc( vol->b_size );
    if ( dirtab == NULL ) {
      devfs_dmesg( "ext2: Out of memory\n" );
      return -1;
    }

    int ba = node->n.blocks * ( vol->b_size >> 9 );
    int i;

    for ( i = 0; i < ba; i++ ) {
      int block;
      unsigned long pos = 0;
      unsigned long totalpos = i * vol->b_size;
      ext2_dirent *dirent = NULL;

      if ( totalpos >= node->n.size ) {
        goto ext2_walk_directory_done;
      }

      block = ext2_get_block( vol->fd, node, vol->b_size, i );
      if ( block < 0 ) {
        goto ext2_walk_directory_done;
      }

      if ( devfs_read( vol->fd, block * vol->b_size, dirtab, vol->b_size ) != vol->b_size ) {
        goto ext2_walk_directory_done;
      }

      while ( 1 ) {
        dirent = ( ext2_dirent* )dirtab;
        if ( !dirent->inode ) {
          i = ba;
          break;
        }

        if ( totalpos >= node->n.size ) {
          break;
        }

        if ( ( dirent->name_len == entry_name_len ) && ( devfs_strncmp( dirent->name, entry_name, entry_name_len ) == 0 ) ) {
          rc = 0;
          *inode = dirent->inode;
          goto ext2_walk_directory_done;
        }

        dirtab += dirent->rec_len;
        pos += dirent->rec_len;
        totalpos += dirent->rec_len;
        if ( pos >= vol->b_size ) {
          break;
        }
      }
    }

ext2_walk_directory_done:
    smk_free( dirtab );
  }

  return rc;
}

static int ext2_read_inode( void *data, unsigned long inode_id, void **new_node ) {
  ext2_vol *vol = ( ext2_vol* )data;
  if ( vol == NULL ) {
    devfs_dmesg( "ext2: Volume descriptor is corrupt!\n" );
    return -1;
  }

  ext2_inode *node = ( ext2_inode* )smk_malloc( sizeof( ext2_inode ) );
  if ( node == NULL ) {
    devfs_dmesg( "ext2: Out of memory\n" );
    return -1;
  }

  if ( inode_id >= vol->inodes ) {
    smk_free( node );
    devfs_dmesg( "ext2: Tried to read out of bounds inode. Request %i, limit %i\n", ( unsigned long )inode_id, vol->inodes );
    return -1;
  }

  inode_id--;
  if ( inode_id + 1 == vol->root_inode ) {
    devfs_memcpy( node, vol->root, sizeof( ext2_inode ) );
  } else {
    unsigned long imipg = inode_id % vol->ipg;
    unsigned long group = inode_id  / vol->ipg;
    unsigned long block = ( imipg ) >> vol->ipb_bits;
    unsigned long iib = ( imipg ) & vol->ipb_mask;
    _ext2_inode *inodes = ( _ext2_inode* )smk_malloc( vol->b_size );
    if ( inodes == NULL ) {
      devfs_dmesg( "ext2: Out of memory\n" );
      smk_free( node );
      return -1;
    }

    if ( devfs_read( vol->fd, ( vol->grouptable[ group ].inode_table + block ) * vol->b_size, ( void* )inodes, vol->b_size ) != vol->b_size ) {
      devfs_dmesg( "ext2: Failed to load Inode %i\n", inode_id );
      smk_free( inodes );
      smk_free( node );
      return -1;
    }

    devfs_memcpy( node, inodes + iib, vol->i_size );
    smk_free( inodes );
  }

  node->id = inode_id + 1;
  *new_node = node;

  return 0;
}

static int ext2_create( void *data, int fd, const char *filename, unsigned int mode ) {
  return -1;
}

#define EXT2_FREE_DIR_NODE \
  if ( dir_node != vol->root ) { \
    smk_free( dir_node ); \
    dir_node = NULL; \
  }

static ext2_inode* ext2_parse_dirs( ext2_vol* vol, const char* filename, int* fname_start ) {
  ext2_inode *dir_node = vol->root;
  unsigned long dir_node_id = 0;

  int start = 0, i;
  for ( i = 0; i < devfs_strlen( filename ); i++ ) {
    if ( filename[ i ] == '/' ) {
      char* entry_name = devfs_strndup( filename + start, i - start );
      if ( entry_name == NULL ) {
        EXT2_FREE_DIR_NODE
        return NULL;
      }

      if ( ext2_walk_directory( vol, dir_node, entry_name, i - start, &dir_node_id ) != 0 ) {
        EXT2_FREE_DIR_NODE
        smk_free( entry_name );
        return NULL;
      }

      EXT2_FREE_DIR_NODE
      if ( ext2_read_inode( vol, dir_node_id, ( void** )&dir_node ) != 0 ) {
        smk_free( entry_name );
        return NULL;
      }

      smk_free( entry_name );
      start = i + 1;
    }
  }

  *fname_start = start;
  return dir_node;
}

static int ext2_open( void *data, int fd, const char *filename, void **fdata ) {
  // trying to open a directory
  if ( ( devfs_strlen( filename ) > 0 ) && ( filename[ devfs_strlen( filename ) - 1 ] == '/' ) ) {
    return -1;
  }

  ext2_vol *vol = ( ext2_vol* )data;
  if ( vol == NULL ) {
    devfs_dmesg( "ext2: Volume descriptor is corrupt!\n" );
    return -1;
  }

  int start = -1;
  ext2_inode *dir_node = ext2_parse_dirs( vol, filename, &start );
  if ( ( dir_node == NULL ) || ( start == -1 ) ) {
    return -1;
  }

  unsigned long file_node_id = 0;
  if ( ext2_walk_directory( data, dir_node, filename + start, devfs_strlen( filename + start ), &file_node_id ) != 0 ) {
    EXT2_FREE_DIR_NODE
    return -1;
  }

  EXT2_FREE_DIR_NODE

  ext2_inode *file_node = NULL;
  if ( ext2_read_inode( data, file_node_id, ( void** )&file_node ) != 0 ) {
    return -1;
  }

  // opening directories not allowed ;)
  if ( file_node->n.mode & EXT2_S_IFDIR ) {
    smk_free( file_node );
    return -1;
  }

  *fdata = file_node;
  return 0;
}

static int ext2_close( void *data, int fd, void *fdata ) {
  if ( fdata == NULL ) {
    devfs_dmesg( "ext2: File data is corrupt!\n" );
    return -1;
  }

  smk_free( fdata );
  return 0;
}

static long long ext2_read( void *data, int fd, void *fdata, unsigned long long position, void *buffer, long long len ) {
  ext2_vol *vol = ( ext2_vol* )data;
  ext2_inode *node = ( ext2_inode* )fdata;

  if ( ( vol == NULL ) || ( node == NULL ) ) {
    devfs_dmesg( "ext2: Volume descriptor or Inode is corrupt\n" );
    return -1;
  }

  unsigned long start_block;
  unsigned long end_block;
  unsigned long block;
  unsigned long pos_in_buf = 0;
  char *buf = ( void* )buffer;
  unsigned long startpos = 0;
  unsigned long endpos = 0;
  char b[ vol->b_size ];
  int i;

  if ( node->n.mode & EXT2_S_IFDIR ) { return -1; }
  if ( position < 0 ) { position = 0; }
  if ( len + position > node->n.size ) { len = node->n.size - position; }
  if ( len <= 0 ) { return 0; }

  start_block = position / vol->b_size;
  end_block = ( position + len ) / vol->b_size;
  block = ext2_get_block( vol->fd, node, vol->b_size, start_block );
  if ( block < 0 ) { return -1; }

  startpos = position % vol->b_size;
  endpos = ( position + len ) % vol->b_size;

  if ( start_block == end_block ) {
    if ( devfs_read( vol->fd, block * vol->b_size, ( void* )b, vol->b_size ) != vol->b_size ) { return -1; }
    devfs_memcpy( buf, ( char* )b + startpos, len );
    return len;
  }

  if ( devfs_read( vol->fd, block * vol->b_size, ( void* )b, vol->b_size ) != vol->b_size ) { return -1; }
  devfs_memcpy( buf, ( char* )b + startpos, vol->b_size - startpos );
  pos_in_buf += vol->b_size - startpos;

  for ( i = start_block + 1; i < end_block; i++ ) {
    block = ext2_get_block( vol->fd, node, vol->b_size, i );
    if ( block < 0 ) {
      return -1;
    }

    if ( devfs_read( vol->fd, block * vol->b_size, ( void* )b, vol->b_size ) != vol->b_size ) { return -1; }
    devfs_memcpy( buf + pos_in_buf, b, vol->b_size );
    pos_in_buf += vol->b_size;
  }

  if ( endpos > 0 ) {
    block = ext2_get_block( vol->fd, node, vol->b_size, end_block );
    if ( block < 0 ) { return -1; }

    if ( devfs_read( vol->fd, block * vol->b_size, ( void* )b, vol->b_size ) != vol->b_size ) { return -1; }
    devfs_memcpy( buf + pos_in_buf, b, endpos );
  }

  return len;
}

static long long ext2_write( void *data, int fd, void *fdata, unsigned long long position, void *buffer, long long len ) {
  return -1;
}

static int ext2_rm( void *data, int fd, const char *filename ) {
  return -1;
}

static int ext2_mkdir( void *data, int fd, const char *filename ) {
  return -1;
}

static int ext2_stat( void *data, int fd, const char *filename, struct vfsstat *st ) {
  // we can't use ext2_open here :(
  ext2_vol *vol = ( ext2_vol* )data;
  if ( vol == NULL ) {
    devfs_dmesg( "ext2: Volume descriptor is corrupt!\n" );
    return -1;
  }

  int start = -1;
  ext2_inode *dir_node = ext2_parse_dirs( vol, filename, &start );
  if ( ( dir_node == NULL ) || ( start == -1 ) ) {
    return -1;
  }

  char* file_name = devfs_strdup( filename + start );
  if ( file_name == NULL ) {
    EXT2_FREE_DIR_NODE
    return -1;
  }

  unsigned long file_node_id = 0;
  if ( ext2_walk_directory( data, dir_node, file_name, devfs_strlen( file_name ), &file_node_id ) != 0 ) {
    EXT2_FREE_DIR_NODE
    return -1;
  }

  EXT2_FREE_DIR_NODE
  smk_free( file_name );

  ext2_inode *file_node = NULL;
  if ( ext2_read_inode( data, file_node_id, ( void** )&file_node ) != 0 ) {
    return -1;
  }

  st->filename = NULL;
  st->size = file_node->n.size;
  st->mode = file_node->n.mode;
  st->uid = file_node->n.uid;
  st->gid = file_node->n.gid;
  st->did = fd;
  st->inode = file_node->id;
  st->nlinks = file_node->n.links;
  st->blocksize = vol->b_size;
  st->blocks = file_node->n.blocks;
  st->atime = file_node->n.atime;
  st->mtime = file_node->n.mtime;
  st->ctime = file_node->n.ctime;

  smk_free( file_node );

  return 0;
}

static int ext2_chmod( void *data, int fd, const char *filename, unsigned int mode ) {
  return -1;
}

static int ext2_ls( void *data, int fd, const char *_pathname, struct vfsstat *list, int *max ) {
  char* pathname = ( char* )_pathname;

  if ( ( devfs_strlen( pathname ) > 0 ) && ( pathname[ devfs_strlen( pathname ) - 1 ] == '/' ) ) {
    pathname[ devfs_strlen( pathname ) - 1 ] = 0;
  }

  ext2_vol *vol = ( ext2_vol* )data;
  if ( vol == NULL ) {
    devfs_dmesg( "ext2: Volume descriptor is corrupt!\n" );
    return -1;
  }

  int start = -1;
  ext2_inode *dir_node = NULL;

  dir_node = ext2_parse_dirs( vol, pathname, &start );
  if ( ( dir_node == NULL ) || ( start == -1 ) ) {
    return -1;
  }

  if ( devfs_strlen( pathname + start ) > 0 ) {
    unsigned long dir_node_id = 0;
    if ( ext2_walk_directory( data, dir_node, pathname + start, devfs_strlen( pathname + start ), &dir_node_id ) != 0 ) {
      EXT2_FREE_DIR_NODE
      return -1;
    }

    EXT2_FREE_DIR_NODE
    if ( ext2_read_inode( data, dir_node_id, ( void** )&dir_node ) != 0 ) {
      return -1;
    }

    if ( dir_node == NULL ) {
      return -1;
    }
  }

  if ( ( dir_node->n.mode & EXT2_S_IFDIR ) == 0 ) {
    EXT2_FREE_DIR_NODE
    return -1;
  }

  char* block = ( char* )smk_malloc( vol->b_size );
  if ( block == NULL ) {
    EXT2_FREE_DIR_NODE
    return -1;
  }

  ext2_inode *tmp_node = NULL;
  ext2_dirent *dirent = NULL;

  unsigned long b;
  unsigned long c_pos = 0;
  unsigned long c_block = 0;
  unsigned long c_totalpos = 0;
  int ba = dir_node->n.blocks * ( vol->b_size >> 9 );

  unsigned long act_entry = 0;

  while ( c_block < ba ) {
    if ( c_totalpos >= dir_node->n.size ) {
      goto ext2_ls_done;
    }

    b =  ext2_get_block( vol->fd, dir_node, vol->b_size, c_block );
    if ( b < 0 ) {
      EXT2_FREE_DIR_NODE
      smk_free( block );
      return -1;
    }

    if ( devfs_read( vol->fd, b * vol->b_size, ( void* )block, vol->b_size ) != vol->b_size ) {
      EXT2_FREE_DIR_NODE
      smk_free( block );
      return -1;
    }

    dirent = ( ext2_dirent* )( block + c_pos );
    if ( ( dirent->inode ) && ( ( c_pos + dirent->rec_len ) <= vol->b_size ) ) {
      if ( dirent->name_len > 0 ) {
        if ( ext2_read_inode( data, dirent->inode, ( void** )&tmp_node ) != 0 ) {
          EXT2_FREE_DIR_NODE
          smk_free( block );
          return -1;
        }

        list[ act_entry ].filename = devfs_strndup( dirent->name, dirent->name_len );
        if ( list[ act_entry ].filename == NULL ) {
          EXT2_FREE_DIR_NODE
          smk_free( block );
          return -1;
        }

        list[ act_entry ].size = tmp_node->n.size;
        list[ act_entry ].mode = tmp_node->n.mode;
        list[ act_entry ].uid = tmp_node->n.uid;
        list[ act_entry ].gid = tmp_node->n.gid;
        list[ act_entry ].did = vol->fd;
        list[ act_entry ].inode = tmp_node->id;
        list[ act_entry ].nlinks = tmp_node->n.links;
        list[ act_entry ].blocksize = vol->b_size;
        list[ act_entry ].blocks = tmp_node->n.blocks;
        list[ act_entry ].atime = tmp_node->n.atime;
        list[ act_entry ].mtime = tmp_node->n.mtime;
        list[ act_entry ].ctime = tmp_node->n.ctime;

        smk_free( tmp_node );

        if ( ++act_entry == *max ) {
          goto ext2_ls_done;
        }
      }

      c_pos += dirent->rec_len;
      c_totalpos += dirent->rec_len;
    } else {
      c_pos = 0;
      c_block++;

      c_totalpos = c_block  * vol->b_size;
      if ( !dirent->inode ) {
        c_block = ba;
      }
    }
  }

ext2_ls_done:
  *max = act_entry;
  EXT2_FREE_DIR_NODE
  return 0;
}

struct filesystem ext2_fs = {
  "ext2",
  "ext2 file system",
  ext2_probe,
  ext2_init,
  ext2_shutdown,
  ext2_create,
  ext2_open,
  ext2_close,
  ext2_read,
  ext2_write, 
  ext2_rm,
  ext2_mkdir,
  ext2_stat,
  ext2_chmod,
  ext2_ls
};
