
#include <smk.h>
#include <devfs.h>

#include "common.h"
#include "atapi.h"

void reset( struct atapi_node *node )
{
   		unsigned char devSel;

	    if ( node->_device == 0 ) devSel = CB_DH_DEV0;
		           		     else devSel = CB_DH_DEV1;
	
	smk_disable_interrupts();
	   DELAY400NS;
	   pio_outbyte( node, CB_DH, devSel );
	   DELAY400NS;
	   reg_reset( node, 0, node->_device );
	smk_enable_interrupts();
}




long long ata_read( struct atapi_node *node,
					  unsigned long long pos, 
					  unsigned long long len, void *buffer )
{
   long lba 	= pos / 512;
   int sectors 	= len / 512;
   int left    	= len % 512;

   int ans;

    void *dest = buffer;

    if ( sectors > 0 )
    {
		smk_disable_interrupts();
		ans = reg_pio_data_in_lba( node, node->_device, 
								   CMD_READ_SECTORS,
     							   0, sectors,
								   lba,
								   dest,
								   sectors, 0 );
		smk_enable_interrupts();


		if ( ans == 1 ) return -1;
    }
	if ( left == 0 ) return len;
	
	dest = (void*)((void*)buffer + sectors * 512);
	lba = lba + sectors;
	char temp[512];
	
	smk_disable_interrupts();
	ans = reg_pio_data_in_lba( node, node->_device, 
								CMD_READ_SECTORS,
			     			    0, 1,
							   lba,
							   temp,
							   1, 0 );
	smk_enable_interrupts();

	if ( ans == 1 ) return -1;

	devfs_memcpy( dest, temp, left );
	
   return len;
}


long long ata_write( struct atapi_node *node,
			   unsigned long long pos, 
		  	   unsigned long long len, void *buffer )
{
   long lba 	= pos / 512;
   int sectors 	= len / 512;
   int    left 	= len % 512;
   
   int ans;
  
   if ( sectors > 0 )
   {
	  	// Write all the complete sectors..
		smk_disable_interrupts();
		ans = reg_pio_data_out_lba( node, node->_device, 
								CMD_READ_SECTORS,
			     			    0, sectors,
							   lba,
							   buffer,
							   sectors, 0 );
		smk_enable_interrupts();
		if ( ans == 1 ) return -1;
   }
   
	if ( left == 0 ) return len;

	// Read in the last sector
	lba += sectors;
	char temp[512];
	
	smk_disable_interrupts();
	ans = reg_pio_data_in_lba( node, node->_device, 
								CMD_READ_SECTORS,
   				  			    0, 1,
							   lba,
							   temp,
							   1, 0 );
	smk_enable_interrupts();
	
	if ( ans == 1 ) return -1;

	// overwrite any new data in it... 
   	void *dest = (void*)((void*)buffer + sectors * 512);
	devfs_memcpy( temp, dest, left );
 
 	// write it to the disk
	smk_disable_interrupts();
	ans = reg_pio_data_out_lba( node, node->_device, 
								CMD_READ_SECTORS,
			     			    0, 1,
							   lba,
							   temp,
							   1, 0 );
	smk_enable_interrupts();

	if ( ans == 1 ) return -1;
   
   return len;
}

long long atapi_read( struct atapi_node *node,
				  unsigned long long pos, 
				  unsigned long long len, void *buffer )
{
   unsigned char cdb[16];
   int rc;
   int bytes = 0;
   
   long lba    = pos / 2048;
   int sectors = len / 2048;
   int left    = len % 2048;

    void *dest = buffer;

   char temp[2048];

   while ( sectors-- > 0 )
   {
       int failures = 0;
       for ( failures = 0; failures < 10; failures++ )
       {
		   // do an ATAPI read command
		   devfs_memset( cdb, 0, sizeof( cdb ) );
		   cdb[0] = 0x28;   // command code
		   cdb[2] = (lba >> 24) & 0xFF;
		   cdb[3] = (lba >> 16) & 0xFF;
		   cdb[4] = (lba >>  8) & 0xFF;
		   cdb[5] = lba & 0xFF;    // LBA
		   cdb[8] = 1;    // sectors
	
		   rc = packet( node, 10, cdb, 0, 2048, dest, lba );
		   
		   if ( rc == 0 )  break;
		   
		   smk_go_dormant_t( 1000 );
		   reset( node );
       }    

       if ( failures >= 10 ) return -1;


       dest = (void*)((void*)dest + 2048 );
       lba += 1;
       bytes += node->reg_cmd_info.totalBytesXfer;
   }
   
  // ........ left overs ....
   if ( left > 0 )
   {
       int failures = 0;
       for ( failures = 0; failures < 10; failures++ )
       {
		   // do an ATAPI Unload command
		   devfs_memset( cdb, 0, sizeof( cdb ) );
		   cdb[0] = 0x28;   // command code
		   cdb[2] = (lba >> 24) & 0xFF;
		   cdb[3] = (lba >> 16) & 0xFF;
		   cdb[4] = (lba >>  8) & 0xFF;
		   cdb[5] = lba & 0xFF;    // LBA
		   cdb[8] = 1;    // sectors
	
		   rc = packet( node, 10, cdb, 0, 2048, temp, lba );
		   if ( rc == 0 )  break;

		   smk_go_dormant_t( 1000 );
	   	   reset( node );
	   }    

        if ( failures >= 10 ) return -1;


		devfs_memcpy( dest, temp, left );
        bytes += left;
  }

  return bytes;
}

long long atapi_write( struct atapi_node *node,
				   unsigned long long pos, 
			  	   unsigned long long len, void *buffer )
{
   return -1;
}



