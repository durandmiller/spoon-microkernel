#include <stdlib.h>
#include <stdio.h>
#include <gui.h>
#include <devfs.h>

#include "window.h"
#include "devcomms.h"
#include "desktop.h"



static unsigned int 	g_windowId 		=	0;


// ---------------------------------------------------

static int w_key( void *pdt )
{
	struct window *win = ((struct window*)pdt);
	return (win->wid);
}


static int w_cmp( void* ldt, void* rdt )
{
	struct window *ld = (struct window*)ldt;
	struct window *rd = (struct window*)rdt;

	if ( ld->wid < rd->wid ) return -1;
	if ( ld->wid > rd->wid ) return 1;
	return 0;
}


// ---------------------------------------------------


static struct devfs_htable *window_table = NULL;


struct window* window_get( int wid )
{
	struct window pt;
				pt.wid = wid;

	return (struct window*)devfs_htable_retrieve( window_table, &pt );
}




int windows_init()
{
	window_table = devfs_init_htable( 1024, 50, w_key, w_cmp );
	if ( window_table == NULL ) return -1;
	return 0;
}


int window_create( struct window **result,
					int x, int y,
					int width, int height,
					unsigned int flags,
					unsigned int state,
					unsigned int events,
					unsigned int port,
					int pid )
{
	unsigned int tmpFlags;
	int pages;

	struct window *win = (struct window*)malloc( sizeof(struct window) );
	if ( win == NULL )
			return GUIERR_NOMEM;

	*result = win;

	// Ensure uniqueness
	do
	{
		if ( g_windowId < 0 ) g_windowId = 0;
		win->wid = g_windowId++;
	} while ( window_get( win->wid ) != NULL );

	// Fill the rest
	win->x	 	= x;
	win->y 		= y;
	win->width 	= width;
	win->height = height;
	win->flags 	= flags;
	win->state 	= state;
	win->events	= events;
	win->port 	= port;
	win->pid 	= pid;

	if ( (win->width > 2000) || (win->height > 2000) )
	{
		free( win );
		return GUIERR_BAD_PARAMETERS;
	}

	pages = ((win->width * win->height * 4) / 4096) + 1;

	win->shmem_id = smk_create_shmem( (unsigned char*)"gui_window", 
										pages, 
										SHMEM_RDWR );
	if ( win->shmem_id < 0 )
	{
		free( win );
		return GUIERR_SHMEM_FAILED;
	}


	if ( smk_grant_shmem( win->shmem_id, smk_getpid(), SHMEM_RDWR ) != 0 ) 
	{
		smk_delete_shmem( win->shmem_id );
		free( win );
		return GUIERR_SHMEM_FAILED;
	}

	if ( smk_grant_shmem( win->shmem_id, pid, SHMEM_RDWR ) != 0 ) 
	{
		smk_delete_shmem( win->shmem_id );
		free( win );
		return GUIERR_SHMEM_FAILED;
	}


	if ( smk_request_shmem( win->shmem_id,
							(void**)&(win->buffer),
							&pages,
							&tmpFlags ) != 0 )
	{
		smk_delete_shmem( win->shmem_id );
		free( win );
		return GUIERR_SHMEM_FAILED;
	}



	if ( devfs_htable_insert( window_table, win ) != 0 )
	{
		smk_release_shmem( win->shmem_id );
		smk_delete_shmem( win->shmem_id );
		free( win );
		return GUIERR_ERROR;
	}


	return GUIERR_OK;
}



int 	window_destroy( struct window *win )
{
	if ( devfs_htable_remove( window_table, win ) != 0 )
		return -1;


	smk_release_shmem( win->shmem_id );
	smk_delete_shmem( win->shmem_id );
	free( win );
	return 0;
}














