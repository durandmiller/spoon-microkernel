#include <smk.h>
#include <devfs.h>


#include "ataio.h"
#include "atapi.h"
#include "common.h"



int eject( struct atapi_node *node )
{
   unsigned char cdb[16];
   int rc;

   int failures = 0;
   char buffer[2048];

   for ( failures = 0; failures < 10; failures++ )
   {
	   // do an ATAPI Unload command
	   devfs_memset( cdb, 0, sizeof( cdb ) );
	   cdb[0] = 0x1B;   // command code
	   cdb[4] = 2; // eject the CD, no power changes.
	
	   rc = packet( node, 10, cdb, 0, 4096, buffer, 0 );
	   if ( rc == 0 ) return 0;


#warning IMPLEMENT SLEEPING again
	   smk_go_dormant_t( 1000 );
	   reg_reset( node, 0, node->_device ); 
    }
 
  return -1;
}

/*
int ATAPINode::read( int64 pos, void *buffer, int len )
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
		   memset( cdb, 0, sizeof( cdb ) );
		   cdb[0] = 0x28;   // command code
		   cdb[2] = (lba >> 24) & 0xFF;
		   cdb[3] = (lba >> 16) & 0xFF;
		   cdb[4] = (lba >>  8) & 0xFF;
		   cdb[5] = lba & 0xFF;    // LBA
		   cdb[8] = 1;    // sectors
	
  		   PRINTIT(0);
		   rc = packet( 10, cdb, 0, 2048, dest, lba );
  		   PRINTIT(0);
		   
		   if ( rc == 0 )  break;
		   
		   PRINTIT(0);
		   smk_sleep( 1 );
	   	   Reset();
       }    

       if ( failures >= 10 ) return -1;


       dest = (void*)((uint32)dest + 2048 );
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
		   memset( cdb, 0, sizeof( cdb ) );
		   cdb[0] = 0x28;   // command code
		   cdb[2] = (lba >> 24) & 0xFF;
		   cdb[3] = (lba >> 16) & 0xFF;
		   cdb[4] = (lba >>  8) & 0xFF;
		   cdb[5] = lba & 0xFF;    // LBA
		   cdb[8] = 1;    // sectors
	
		   rc = packet( 10, cdb, 0, 2048, temp, lba );
		   if ( rc == 0 )  break;

		   PRINTIT(0);
		   smk_sleep( 1 );
	   	   Reset();
	   }    

        if ( failures >= 10 ) return -1;


		memcpy( dest, temp, left );
        bytes += left;
  }

  return bytes;
}

int ATAPINode::write( int64 pos, void *buffer, int len )
{
   return -1;
}
*/


int packet( struct atapi_node *node,
					   int cmdSize, void *cmd, int dir,
       			   	   long bufSize, void* buffer,
			   		   unsigned long lba )
{
  int ans = 0;
  int failures = 0;
  smk_disable_interrupts();
   for ( failures = 0; failures < 10; failures++ )
   {
	   ans = reg_packet( node, cmdSize, cmd, dir, bufSize, buffer, lba );
	   if ( ans == 0 ) break;

	   reg_reset( node, 0, node->_device );
   }
  smk_enable_interrupts();
  return ans;
}


int reg_packet( struct atapi_node *node, 
				int cmdSize, void *cmd, int dir,
       			long bufSize, void* buffer,
			   	unsigned long lba )
{
   long dpbc 		 = bufSize;
   unsigned int cpbc = bufSize;
   void* cpbuf 		 = cmd;
   void* dpbuf 		 = buffer;

   unsigned char devCtrl;
   unsigned char devHead;
   unsigned char cylLow;
   unsigned char cylHigh;
   unsigned char frReg;
   unsigned char scReg;
   unsigned char snReg;
   unsigned char status;
   unsigned char reason;
   unsigned char lowCyl;
   unsigned char highCyl;
   unsigned int byteCnt;
   unsigned int wordCnt;
   int ndx;
   unsigned long dpaddr;
   unsigned char * cp;
   unsigned char * cfp;
   int slowXferCntr = 0;
   int prevFailBit7 = 0;
   int dev = node->_device;

	// ----------------------
	unsigned char reg_atapi_reg_fr;  // see reg_packet()
	unsigned char reg_atapi_reg_sc;  // see reg_packet()
	unsigned char reg_atapi_reg_sn;  // see reg_packet()
	unsigned char reg_atapi_reg_dh;  // see reg_packet()
	long reg_atapi_max_bytes = 32768L;
	
	// ATAPI command packet size and data
	
	int reg_atapi_cp_size;
	unsigned char reg_atapi_cp_data[16];
	// ....................................


   // mark start of PI cmd in low level trace


   // setup register values

   devCtrl = CB_DC_HD15 | ( node->int_use_intr_flag ? 0 : CB_DC_NIEN );
   reg_atapi_reg_dh = reg_atapi_reg_dh & 0x0f;
   devHead = ( dev ? CB_DH_DEV1 : CB_DH_DEV0 ) | reg_atapi_reg_dh;
   cylLow = dpbc & 0x00ff;
   cylHigh = ( dpbc & 0xff00 ) >> 8;
   frReg = reg_atapi_reg_fr;
   scReg = reg_atapi_reg_sc;
   snReg = reg_atapi_reg_sn;
   reg_atapi_reg_fr = 0;
   reg_atapi_reg_sc = 0;
   reg_atapi_reg_sn = 0;
   reg_atapi_reg_dh = 0;

   // Reset error return data.

   sub_zero_return_data( node );
   node->reg_cmd_info.flg = TRC_FLAG_CMD;
   node->reg_cmd_info.ct  = dir ? TRC_TYPE_PPDO : TRC_TYPE_PPDI;
   node->reg_cmd_info.cmd = CMD_PACKET;
   node->reg_cmd_info.fr1 = frReg;
   node->reg_cmd_info.sc1 = scReg;
   node->reg_cmd_info.sn1 = snReg;
   node->reg_cmd_info.cl1 = cylLow;
   node->reg_cmd_info.ch1 = cylHigh;
   node->reg_cmd_info.dh1 = devHead;
   node->reg_cmd_info.dc1 = devCtrl;

   // Make sure the command packet size is either 12 or 16
   // and save the command packet size and data.

   cpbc = cpbc < 12 ? 12 : cpbc;
   cpbc = cpbc > 12 ? 16 : cpbc;
   reg_atapi_cp_size = cpbc;
   cp = reg_atapi_cp_data;
   cfp = (unsigned char*)cpbuf;
   for ( ndx = 0; ndx < (int)cpbc; ndx ++ )
   {
      * cp = * cfp;
      cp ++ ;
      cfp ++ ;
   }

   // Set command time out.
   tmr_set_timeout( node );

   // Select the drive - call the sub_select function.
   // Quit now if this fails.

   if ( sub_select( node, dev ) )
   {
      sub_trace_command( node );
      reg_atapi_max_bytes = 32768L;    // reset max bytes
      return 1;
   }

   // Set up all the registers except the command register.

   pio_outbyte( node, CB_DC, devCtrl );
   pio_outbyte( node, CB_FR, frReg );
   pio_outbyte( node, CB_SC, scReg );
   pio_outbyte( node, CB_SN, snReg );
   pio_outbyte( node, CB_CL, cylLow );
   pio_outbyte( node, CB_CH, cylHigh );
   pio_outbyte( node, CB_DH, devHead );

   // Start the command by setting the Command register.  The drive
   // should immediately set BUSY status.

   pio_outbyte( node, CB_CMD, CMD_PACKET );

   // Waste some time by reading the alternate status a few times.
   // This gives the drive time to set BUSY in the status register on
   // really fast systems.  If we don't do this, a slow drive on a fast
   // system may not set BUSY fast enough and we would think it had
   // completed the command when it really had not even started the
   // command yet.

   DELAY400NS;

   // Command packet transfer...
   // Check for protocol failures,
   // the device should have BSY=1 or
   // if BSY=0 then either DRQ=1 or CHK=1.

   sub_atapi_delay( dev );
   status = pio_inbyte( node, CB_ASTAT );
   if ( status & CB_STAT_BSY )
   {
      // BSY=1 is OK
   }
   else
   {
      if ( status & ( CB_STAT_DRQ | CB_STAT_ERR ) )
      {
         // BSY=0 and DRQ=1 is OK
         // BSY=0 and ERR=1 is OK
      }
      else
      {
         node->reg_cmd_info.failbits |= FAILBIT0;  // not OK
      }
   }

   // Command packet transfer...
   // Poll Alternate Status for BSY=0.
   while ( 1 )
   {
      status = pio_inbyte( node, CB_ASTAT );       // poll for not busy
      if ( ( status & CB_STAT_BSY ) == 0 )
	  {
             break;
	  }
      if ( tmr_chk_timeout( node ) )               // time out yet ?
      {
         node->reg_cmd_info.to = 1;
         node->reg_cmd_info.ec = 51;
         dir = -1;   // command done
         break;
      }
   }

   // Command packet transfer...
   // Check for protocol failures... no interrupt here please!
   // Clear any interrupt the command packet transfer may have caused.

   // Command packet transfer...
   // If no error, transfer the command packet.
   if ( node->reg_cmd_info.ec == 0 )
   {
      // Command packet transfer...
      // Read the primary status register and the other ATAPI registers.

      status  = pio_inbyte( node, CB_STAT );
      reason  = pio_inbyte( node, CB_SC );
      lowCyl  = pio_inbyte( node, CB_CL );
      highCyl = pio_inbyte( node, CB_CH );

      // Command packet transfer...
      // check status: must have BSY=0, DRQ=1 now

      if ( ( status & ( CB_STAT_BSY | CB_STAT_DRQ | CB_STAT_ERR ) )
           != CB_STAT_DRQ
         )
      {
         node->reg_cmd_info.ec = 52;
         dir = -1;   // command done
      }
      else
      {
         // Command packet transfer...
         // Check for protocol failures...
         // check: C/nD=1, IO=0.

         if ( ( reason &  ( CB_SC_P_TAG | CB_SC_P_REL | CB_SC_P_IO ) )
              || ( ! ( reason &  CB_SC_P_CD ) )
            )
            node->reg_cmd_info.failbits |= FAILBIT2;
         if ( ( lowCyl != cylLow ) || ( highCyl != cylHigh ) )
            node->reg_cmd_info.failbits |= FAILBIT3;

         // Command packet transfer...
         // trace cdb byte 0,
         // xfer the command packet (the cdb)

         pio_rep_outword( node, CB_DATA, cpbuf, cpbc >> 1 );

         DELAY400NS;    // delay so device can get the status updated
      }
   }

   // Data transfer loop...
   // If there is no error, enter the data transfer loop.
   // First adjust the I/O buffer address so we are able to
   // transfer large amounts of data (more than 64K).

   dpaddr = (unsigned long)dpbuf;

   while ( node->reg_cmd_info.ec == 0 )
   {
      // Data transfer loop...
      // Wait for INT 7x -or- wait for not BUSY -or- wait for time out.

      sub_atapi_delay( dev );
      reg_wait_poll( node, 53, 54 );

      // Data transfer loop...
      // If there was a time out error, exit the data transfer loop.

      if ( (node->reg_cmd_info.ec)  )
      {
         dir = -1;   // command done
         break;
      }

      // Data transfer loop...
      // Read the primary status register.

      status = pio_inbyte( node, CB_STAT );
      reason = pio_inbyte( node, CB_SC );
      lowCyl = pio_inbyte( node, CB_CL );
      highCyl = pio_inbyte( node, CB_CH );

      // Data transfer loop...
      // Exit the read data loop if the device indicates this
      // is the end of the command.

      if ( ( status & ( CB_STAT_BSY | CB_STAT_DRQ ) ) == 0 )
      {
         dir = -1;   // command done
         break;
      }

      // Data transfer loop...
      // The device must want to transfer data...
      // check status: must have BSY=0, DRQ=1 now.

      if ( ( status & ( CB_STAT_BSY | CB_STAT_DRQ ) ) != CB_STAT_DRQ )
      {
         node->reg_cmd_info.ec = 55;
         dir = -1;   // command done
         break;
      }
      else
      {
         // Data transfer loop...
         // Check for protocol failures...
         // check: C/nD=0, IO=1 (read) or IO=0 (write).

         if ( ( reason &  ( CB_SC_P_TAG | CB_SC_P_REL ) )
              || ( reason &  CB_SC_P_CD )
            )
            node->reg_cmd_info.failbits |= FAILBIT4;
         if ( ( reason & CB_SC_P_IO ) && dir )
            node->reg_cmd_info.failbits |= FAILBIT5;
      }

      // do the slow data transfer thing
      int reg_slow_xfer_flag = 0;

      if ( reg_slow_xfer_flag )
      {
         slowXferCntr ++ ;
         if ( slowXferCntr <= reg_slow_xfer_flag )
         {
            sub_xfer_delay();
            reg_slow_xfer_flag = 0;
         }
      }

      // Data transfer loop...
      // get the byte count, check for zero...

      byteCnt = ( highCyl << 8 ) | lowCyl;
      if ( byteCnt < 1 )
      {
         node->reg_cmd_info.ec = 60;
         dir = -1;   // command done
         break;
      }

      // Data transfer loop...
      // and check protocol failures...

      if ( (int)byteCnt > dpbc )
         node->reg_cmd_info.failbits |= FAILBIT6;
      node->reg_cmd_info.failbits |= prevFailBit7;
      prevFailBit7 = 0;
      if ( byteCnt & 0x0001 )
         prevFailBit7 = FAILBIT7;

      // Data transfer loop...
      // make sure we don't overrun the caller's buffer

      if ( (int)( node->reg_cmd_info.totalBytesXfer + byteCnt ) > reg_atapi_max_bytes )
      {
         node->reg_cmd_info.ec = 59;
         dir = -1;   // command done
         break;
      }

      // increment number of DRQ packets

      node->reg_cmd_info.drqPackets ++ ;

      // Data transfer loop...
      // transfer the data and update the i/o buffer address
      // and the number of bytes transfered.

      wordCnt = ( byteCnt >> 1 ) + ( byteCnt & 0x0001 );
      node->reg_cmd_info.totalBytesXfer += ( wordCnt << 1 );
      dpbuf = (void*)dpaddr;
      if ( dir )
	  {
         pio_rep_outword( node, CB_DATA, dpbuf, wordCnt );
	  }
      else
	  {
         pio_rep_inword( node, CB_DATA, dpbuf, wordCnt );
	  }
      dpaddr = dpaddr + byteCnt;

      DELAY400NS;    // delay so device can get the status updated
   }

   // End of command...
   // Wait for interrupt or poll for BSY=0,
   // but don't do this if there was any error or if this
   // was a commmand that did not transfer data.

   if ( ( node->reg_cmd_info.ec == 0 ) && ( dir >= 0 ) )
   {
      sub_atapi_delay( dev );
      reg_wait_poll( node, 56, 57 );
   }

   // Final status check, only if no previous error.

   if ( node->reg_cmd_info.ec == 0 )
   {
      // Final status check...
      // Read the primary status register and other regs.

      status = pio_inbyte( node, CB_STAT );
      reason = pio_inbyte( node, CB_SC );
      lowCyl = pio_inbyte( node, CB_CL );
      highCyl = pio_inbyte( node, CB_CH );

      // Final status check...
      // check for any error.

      if ( status & ( CB_STAT_BSY | CB_STAT_DRQ | CB_STAT_ERR ) )
      {
         node->reg_cmd_info.ec = 58;
      }
   }

   // Final status check...
   // Check for protocol failures...
   // check: C/nD=1, IO=1.

   if ( ( reason & ( CB_SC_P_TAG | CB_SC_P_REL ) )
        || ( ! ( reason & CB_SC_P_IO ) )
        || ( ! ( reason & CB_SC_P_CD ) )
      )
      node->reg_cmd_info.failbits |= FAILBIT8;

   // Done...
   // Read the output registers and trace the command.
   // Also put the command packet in the trace.

   if ( ! node->reg_cmd_info.totalBytesXfer )
      node->reg_cmd_info.ct = TRC_TYPE_PND;
   sub_trace_command( node );

   // For interrupt mode, restore the INT 7x vector.

//   int_restore_int_vect();

   // mark end of PI cmd in low level trace


   // All done.  The return values of this function are described in
   // ATAIO.H.

   reg_atapi_max_bytes = 32768L;    // reset max bytes
   if ( node->reg_cmd_info.ec )
   {
      return 1;
   }
   
   return 0;
}



// ******************************************************

int probe( struct atapi_node *node )
{
   unsigned char sc;
   unsigned char sn;
   unsigned char cl;
   unsigned char ch;
   unsigned char st;
   unsigned char devCtrl;
   int attempts;

   // setup register values

   devCtrl = CB_DC_HD15 | ( node->int_use_intr_flag ? 0 : CB_DC_NIEN );

   unsigned char devSel;

   if ( node->_device == 0 ) devSel = CB_DH_DEV0;
		   		  else devSel = CB_DH_DEV1;
   
   node->reg_config_info[node->_device] = REG_CONFIG_TYPE_NONE;

   // set up Device Control register
   pio_outbyte( node, CB_DC, devCtrl );

   // lets see if there is a device 0

   pio_outbyte( node, CB_DH, devSel );
   DELAY400NS;
   pio_outbyte( node, CB_SC, 0x55 );
   pio_outbyte( node, CB_SN, 0xaa );
   pio_outbyte( node, CB_SC, 0xaa );
   pio_outbyte( node, CB_SN, 0x55 );
   pio_outbyte( node, CB_SC, 0x55 );
   pio_outbyte( node, CB_SN, 0xaa );
   sc = pio_inbyte( node, CB_SC );
   sn = pio_inbyte( node, CB_SN );
   if ( ( sc == 0x55 ) && ( sn == 0xaa ) )
      node->reg_config_info[node->_device] = REG_CONFIG_TYPE_UNKN;

   pio_outbyte( node, CB_DH, devSel );
   DELAY400NS;
   reg_reset( node, 0, node->_device );

   // lets check device 0 again, is device 0 really there?
   // is it ATA or ATAPI?

   for ( attempts = 0; attempts < 10; attempts++ )
   {
       node->reg_config_info[node->_device] = REG_CONFIG_TYPE_NONE;
	   
	   pio_outbyte( node, CB_DH, devSel );
	   DELAY400NS;
	   sc = pio_inbyte( node, CB_SC );
	   sn = pio_inbyte( node, CB_SN );

//	   printf("%s (%x,%x)\n","ATAPI device returned FIRST:", sc,sn );
	   
	   if ( ( sc == 0x01 ) && ( sn == 0x01 ) )
	   {
	      node->reg_config_info[node->_device] = REG_CONFIG_TYPE_UNKN;
	      cl = pio_inbyte( node, CB_CL );
	      ch = pio_inbyte( node, CB_CH );
	      st = pio_inbyte( node, CB_STAT );

//	   	  printf("%s (%x,%x)\n","ATAPI device returned SECOND:", sc,sn );

	      if ( (( cl == 0x14 ) && ( ch == 0xeb )) ||		// PATAPI
		  		(( cl == 0x69 ) && ( ch == 0x96)) )			// SATAPI
	         node->reg_config_info[node->_device] = REG_CONFIG_TYPE_ATAPI;
	      else
	         if ( (st != 0) && 
			 		( (( cl == 0x00 ) && ( ch == 0x00 )) || // PATA
					  ((cl == 0x3c) && (ch == 0xc3)) ) )	// SATA
	            node->reg_config_info[node->_device] = REG_CONFIG_TYPE_ATA;
	   }
	
	   // Is there a device present?
	      if ( node->reg_config_info[node->_device] != REG_CONFIG_TYPE_UNKN ) 
		  {
				  //printf("%s\n","ATAPINode::Present unknown device.");
				  break;
		  }
		  
		  //printf("%s%i%s%i\n", "ATAPINode::Present failed for ",
				 //_device, " on attempt ", attempts );
   	   
	   pio_outbyte( node, CB_DH, devSel );
	   DELAY400NS;			// delete
       reg_reset( node, 0, node->_device );	// delete
       //printf("%s\n","ATAPINode::Present() reset.");
	}
				  
	      
   
   if ( node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATAPI )
		   	return 1;
   if ( node->reg_config_info[node->_device] == REG_CONFIG_TYPE_ATA )
			return 1;

   return 0;
}











