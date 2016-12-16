#include <smk.h>
#include <devfs.h>
#include "processes.h"
#include "files.h"


// ---------------------------------------------------

static int p_key( void *pdt )
{
	return ((struct process_info*)pdt)->pid;
}


static int p_cmp( void* ldt, void* rdt )
{
	struct process_info *ld = (struct process_info*)ldt;
	struct process_info *rd = (struct process_info*)rdt;

	if ( ld->pid < rd->pid ) return -1;
	if ( ld->pid > rd->pid ) return 1;
	return 0;
}


// ---------------------------------------------------


static struct devfs_htable *process_table = NULL;

int init_processes()
{
	process_table = devfs_init_htable( 1024, 50, p_key, p_cmp );
	if ( process_table == NULL ) return -1;
	return 0;
}


struct process_info* get_process( int pid )
{
	struct process_info pt;
						pt.pid = pid;

	return (struct process_info*)devfs_htable_retrieve( process_table, &pt );
}


struct process_info* new_process( int pid )
{
	struct process_info *pd = 
			(struct process_info*)smk_malloc( sizeof(struct process_info) );
	if ( pd == NULL ) return NULL;
	pd->pid = pid;
	pd->last_fd = 0;

	// Create a hash table with the file functions
	pd->files = devfs_init_htable( 24, 50, ft_key, ft_cmp );
	if ( pd->files == NULL )
	{
		smk_free( pd );
		return NULL;
	}

	if ( devfs_htable_insert( process_table, pd ) != 0 )
	{
		devfs_delete_htable( pd->files );
		smk_free( pd );
		return NULL;
	}
	
	return pd;
}







