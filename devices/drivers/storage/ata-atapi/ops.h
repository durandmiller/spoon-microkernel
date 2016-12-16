#ifndef _OPS_H
#define _OPS_H


long long ata_read( struct atapi_node *node,
			  unsigned long long pos, 
			  unsigned long long len, void *buffer );

long long ata_write( struct atapi_node *node,
			   unsigned long long pos, 
		  	   unsigned long long len, void *buffer );

long long atapi_read( struct atapi_node *node,
			  unsigned long long pos, 
			  unsigned long long len, void *buffer );

long long atapi_write( struct atapi_node *node,
			   unsigned long long pos, 
		  	   unsigned long long len, void *buffer );


#endif

