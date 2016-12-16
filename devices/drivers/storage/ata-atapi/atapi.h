#ifndef _ATAPI_H
#define _ATAPI_H

#include "ataio.h"
#include "common.h"

int eject( struct atapi_node *node );

int packet( struct atapi_node *node,
			int cmdSize, 
			void *cmd, 
			int dir,
       		long bufSize, 
			void* buffer,
			unsigned long lba );

int probe( struct atapi_node *node );


int reg_packet( struct atapi_node *node,
				int cmdSize, void *cmd, int dir,
       			long bufSize, void* buffer,
	 			unsigned long lba );
   

#endif



