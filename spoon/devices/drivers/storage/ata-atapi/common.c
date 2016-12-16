#include <smk.h>
#include <devfs.h>

#include "common.h"
#include "ataio.h"


void init_atapi( struct atapi_node *node, unsigned int base, int device )
{
	node->_base 	= base;
	node->_device 	= device;

	node->pio_memory_seg		= 0;
	node->pio_xfer_width		= 16;
	node->reg_atapi_delay_flag	= 0;
	node->int_use_intr_flag		= 0;
	node->tmr_time_out			= ATAPI_TIMEOUT;

	pio_set_iobase_addr( node, base, base + 0x200 );

	node->reg_config_info[0] = REG_CONFIG_TYPE_NONE;
	node->reg_config_info[1] = REG_CONFIG_TYPE_NONE;
}
 
void pio_set_iobase_addr( struct atapi_node *node,
						  unsigned int base1, unsigned int base2 )
{
   node->pio_base_addr1 = base1;
   node->pio_base_addr2 = base2;
   node->pio_memory_seg = 0;
   node->pio_reg_addrs[ CB_DATA ] = node->pio_base_addr1 + 0;  // 0
   node->pio_reg_addrs[ CB_FR   ] = node->pio_base_addr1 + 1;  // 1
   node->pio_reg_addrs[ CB_SC   ] = node->pio_base_addr1 + 2;  // 2
   node->pio_reg_addrs[ CB_SN   ] = node->pio_base_addr1 + 3;  // 3
   node->pio_reg_addrs[ CB_CL   ] = node->pio_base_addr1 + 4;  // 4
   node->pio_reg_addrs[ CB_CH   ] = node->pio_base_addr1 + 5;  // 5
   node->pio_reg_addrs[ CB_DH   ] = node->pio_base_addr1 + 6;  // 6
   node->pio_reg_addrs[ CB_CMD  ] = node->pio_base_addr1 + 7;  // 7
   node->pio_reg_addrs[ CB_DC   ] = node->pio_base_addr2 + 6;  // 8
   node->pio_reg_addrs[ CB_DA   ] = node->pio_base_addr2 + 7;  // 9
}







void reg_wait_poll( struct atapi_node *node, int we, int pe )
{
   unsigned char status;


   // Wait for INT 7x -or- wait for not BUSY -or- wait for time out.

      while ( 1 )
      {
         status = pio_inbyte( node, CB_ASTAT );       // poll for not busy
         if ( ( status & CB_STAT_BSY ) == 0 )
            break;
         if ( tmr_chk_timeout( node ) )               // time out yet ?
         {
            node->reg_cmd_info.to = 1;
            node->reg_cmd_info.ec = pe;
            break;
         }
      }

}



//*************************************************************
//
// These functions do basic IN/OUT of byte and word values.
//
//*************************************************************


unsigned char pio_inbyte( struct atapi_node *node, unsigned int addr )
{
   unsigned int regAddr;
   unsigned char uc;
   unsigned char * ucp;

   regAddr = node->pio_reg_addrs[ addr ];
   if ( node->pio_memory_seg )
   {
      ucp = (unsigned char  *) MK_FP( node->pio_memory_seg, regAddr );
      uc = * ucp;
   }
   else
   {
      uc = (unsigned char) inb( regAddr );
   }
   node->pio_last_read[ addr ] = uc;
   return uc;
}

//*********************************************************

void pio_outbyte( struct atapi_node *node, unsigned int addr, unsigned char data )
{
   unsigned int regAddr;
   unsigned char * ucp;

   regAddr = node->pio_reg_addrs[ addr ];
   if ( node->pio_memory_seg )
   {
      ucp = (unsigned char  *) MK_FP( node->pio_memory_seg, regAddr );
      * ucp = data;
   }
   else
   {
      outb( regAddr , data);
   }
   node->pio_last_write[ addr ] = data;
}

//*********************************************************

unsigned int pio_inword( struct atapi_node *node, unsigned int addr )
{
   unsigned int regAddr;
   unsigned int ui;
   unsigned int * uip;

   regAddr = node->pio_reg_addrs[ addr ];
   if ( node->pio_memory_seg )
   {
      uip = (unsigned int *) MK_FP( node->pio_memory_seg, regAddr );
      ui = * uip;
   }
   else
   {
      ui = inw( regAddr );
   }
   return ui;
}

//*********************************************************

void pio_outword( struct atapi_node *node, unsigned int addr, unsigned int data )
{
   unsigned int regAddr;
   unsigned int * uip;

   regAddr = node->pio_reg_addrs[ addr ];
   if ( node->pio_memory_seg )
   {
      uip = (unsigned int  *) MK_FP( node->pio_memory_seg, regAddr );
      * uip = data;
   }
   else
   {
      outw( regAddr , data);
   }
}

//*************************************************************
//
// These functions do REP INS/OUTS (PIO data transfers).
//
//*************************************************************

// Note: pio_rep_inbyte() can be called directly but usually it
// is called by pio_rep_inword() based on the value of the
// pio_xfer_width variables.

void pio_rep_inbyte( struct atapi_node *node,
					 unsigned int addrDataReg,
                     void* buffer,
                     long byteCnt )
{
   unsigned int dataRegAddr;
   unsigned int bCnt;

   dataRegAddr = node->pio_reg_addrs[ addrDataReg ];

   while ( byteCnt > 0L )
   {

      if ( byteCnt > 16384L )
         bCnt = 16384;
      else
         bCnt = (unsigned int) byteCnt;

      asm volatile
      (
			"  mov %0, %%edi\n"
			"  mov %1, %%ecx\n"
			"  mov %2, %%edx\n"
			"  cld\n"
			"  rep insb\n"
         :
	 : "p" (buffer),
	   "g" (bCnt), "g" (dataRegAddr)
		: "eax", "ecx", "edx", "edi"
      );



      byteCnt = byteCnt - (long) bCnt;

      pio_inbyte( node, CB_ASTAT );    // just for debugging
   }
}

//*********************************************************

// Note: pio_rep_outbyte() can be called directly but usually it
// is called by pio_rep_outword() based on the value of the
// pio_xfer_width variables.

void pio_rep_outbyte( struct atapi_node *node, 
					  unsigned int addrDataReg,
		      		  void* buffer,
                      long byteCnt )
{
   unsigned int dataRegAddr;
   unsigned int bCnt;

   dataRegAddr = node->pio_reg_addrs[ addrDataReg ];

   while ( byteCnt > 0L )
   {

      if ( byteCnt > 16384L )
         bCnt = 16384;
      else
         bCnt = (unsigned int) byteCnt;

      asm volatile
      (
			"  mov %0, %%esi\n"
			"  mov %1, %%ecx\n"
			"  mov %2, %%edx\n"
		  
			"  cld\n"
	  
			"  rep outsb\n"
         :
	 : "p" (buffer),
	   "g" (bCnt), "g" (dataRegAddr)
		: "eax", "ecx", "edx", "esi"
      );


      byteCnt = byteCnt - (long) bCnt;

      pio_inbyte( node, CB_ASTAT );    // just for debugging
   }
}

//*********************************************************

// Note: pio_rep_indword() can be called directly but usually it
// is called by pio_rep_inword() based on the value of the
// pio_xfer_width variable.

void pio_rep_indword( struct atapi_node *node,
					  unsigned int addrDataReg,
                      void* buffer,
                      unsigned int dwordCnt )
{
   unsigned int dataRegAddr;

   dataRegAddr = node->pio_reg_addrs[ addrDataReg ];

      asm volatile
      (
			"  mov %0, %%edi\n"
			"  mov %1, %%ecx\n"
			"  mov %2, %%edx\n"
			  
			"  cld\n"
		  
			"  rep insl\n"

     :
	 : "p" (buffer),
	   "g" (dwordCnt), "g" (dataRegAddr)
		: "eax", "ecx", "edx", "edi"
      );


}

//*********************************************************

// Note: pio_rep_outdword() can be called directly but usually it
// is called by pio_rep_outword() based on the value of the
// pio_xfer_width variable.

void pio_rep_outdword( struct atapi_node *node,
					   unsigned int addrDataReg,
                       void* buffer,
                       unsigned int dwordCnt )
{
   unsigned int dataRegAddr;

   dataRegAddr = node->pio_reg_addrs[ addrDataReg ];

      asm volatile
      (
			"  mov %0, %%esi\n"
			"  mov %1, %%ecx\n"
			"  mov %2, %%edx\n"
			"  cld\n"
			"  rep outsl\n"
         :
		 : "p" (buffer),
		   "g" (dwordCnt), "g" (dataRegAddr)
		 : "eax", "ecx", "edx", "esi"
      );

}

//*********************************************************

// Note: pio_rep_inword() is the primary way perform PIO
// Data In transfers. It will handle 8-bit, 16-bit and 32-bit
// I/O based data transfers and 8-bit and 16-bit PCMCIA Memory
// mode transfers.

void pio_rep_inword( struct atapi_node *node,
					 unsigned int addrDataReg,
                     void* buffer,
                     unsigned int wordCnt )

{
   unsigned int dataRegAddr;
   volatile unsigned int * uip1;
   unsigned int  * uip2;
   volatile unsigned char * ucp1;
   unsigned char * ucp2;
   long bCnt;

   dataRegAddr = node->pio_reg_addrs[ addrDataReg ];

   if ( node->pio_memory_seg )
   {

      // PCMCIA Memory mode data transfer.

      if ( node->pio_xfer_width == 8 )
      {
         // PCMCIA Memory mode 8-bit
         bCnt = ( (long) wordCnt ) * 2L;
         ucp1 = (unsigned char *) MK_FP( node->pio_memory_seg, dataRegAddr );
         ucp2 = (unsigned char *) buffer;
         for ( ; bCnt > 0; bCnt -- )
         {
            * ucp2 = * ucp1;
            ucp2 ++ ;
         }
      }
      else
      {
         // PCMCIA Memory mode 16-bit
         uip1 = (unsigned int  *) MK_FP( node->pio_memory_seg, dataRegAddr );
         uip2 = (unsigned int  *) buffer;
         for ( ; wordCnt > 0; wordCnt -- )
         {
            * uip2 = * uip1;
            uip2 ++ ;
         }
      }
   }
   else
   {

      // Data transfer using INS instruction.

      if ( node->pio_xfer_width == 8 )
      {
         // do REP INS
         pio_rep_inbyte( node, addrDataReg, buffer, ( (long) wordCnt ) * 2L );
         return;
      }

      if ( ( node->pio_xfer_width == 32 ) && ( ! ( wordCnt & 0x0001 ) ) )
      {
         // do REP INSD
         pio_rep_indword( node, addrDataReg, buffer, wordCnt / 2 );
         return;
      }

      // do REP INSW

      asm volatile
      (
			"  mov %0, %%edi\n"
			"  mov %1, %%ecx\n"
			"  mov %2, %%edx\n"
			"  cld\n"
			"  rep insw\n"
         :
		 : "p" (buffer),
		   "g" (wordCnt), "g" (dataRegAddr)
	 	 : "eax", "ecx", "edx", "edi"
      );

   }
}

//*********************************************************

// Note: pio_rep_outword() is the primary way perform PIO
// Data Out transfers. It will handle 8-bit, 16-bit and 32-bit
// I/O based data transfers and 8-bit and 16-bit PCMCIA Memory
// mode transfers.

void pio_rep_outword( struct atapi_node *node,
					  unsigned int addrDataReg,
                      void* buffer,
                      unsigned int wordCnt )

{
   unsigned int dataRegAddr;
   unsigned int * uip1;
   unsigned int * uip2;
   unsigned char * ucp1;
   unsigned char * ucp2;
   long bCnt;

   dataRegAddr = node->pio_reg_addrs[ addrDataReg ];

   if ( node->pio_memory_seg )
   {

      // PCMCIA Memory mode data transfer.

      if ( node->pio_xfer_width == 8 )
      {
         // PCMCIA Memory mode 8-bit
         bCnt = ( (long) wordCnt ) * 2L;
         ucp1 = (unsigned char *) buffer;
         ucp2 = (unsigned char *) MK_FP( node->pio_memory_seg, dataRegAddr );
         for ( ; bCnt > 0; bCnt -- )
         {
            * ucp2 = * ucp1;
            ucp1 ++ ;
         }
      }
      else
      {
         // PCMCIA Memory mode 16-bit
         uip1 = (unsigned int *) buffer;
         uip2 = (unsigned int *) MK_FP( node->pio_memory_seg, dataRegAddr );
         for ( ; wordCnt > 0; wordCnt -- )
         {
            * uip2 = * uip1;
            uip1 ++ ;
         }
      }
   }
   else
   {

      // Data transfer using OUTS instruction.

      if ( node->pio_xfer_width == 8 )
      {
         // do REP OUTS
         pio_rep_outbyte( node, addrDataReg, buffer, ( (long) wordCnt ) * 2L );
         return;
      }

      if ( ( node->pio_xfer_width == 32 ) && ( ! ( wordCnt & 0x0001 ) ) )
      {
         // do REP OUTSD
         pio_rep_outdword( node, addrDataReg, buffer, wordCnt / 2 );
         return;
      }

      // do REP OUTSW

      asm volatile
      (
		 " mov %0, %%esi\n"
		 " mov %1, %%ecx\n"
		 " mov %2, %%edx\n"
		  
		 " cld\n"
		  
		 " rep outsw\n"
 	 :
	 : "p" (buffer),
	   "g" (wordCnt), "g" (dataRegAddr)
	 : "eax", "ecx", "edx", "esi"
      );

   }
}



//*************************************************************
//
// reg_reset() - Execute a Software Reset.
//
// See ATA-2 Section 9.2, ATA-3 Section 9.2, ATA-4 Section 8.3.
//
//*************************************************************

int reg_reset( struct atapi_node *node, int skipFlag, int devRtrn )

{
   unsigned char sc;
   unsigned char sn;
   unsigned char status;
   unsigned char devCtrl;

   // setup register values

   devCtrl = CB_DC_HD15 | ( node->int_use_intr_flag ? 0 : CB_DC_NIEN );

   // mark start of reset in low level trace


   // Reset error return data.

   sub_zero_return_data( node );
   node->reg_cmd_info.flg = TRC_FLAG_SRST;
   node->reg_cmd_info.ct  = TRC_TYPE_ASR;

   // initialize the command timeout counter

   tmr_set_timeout( node );


   // Set and then reset the soft reset bit in the Device Control
   // register.  This causes device 0 be selected.

   if ( ! skipFlag )
   {
      pio_outbyte( node, CB_DC, devCtrl | CB_DC_SRST );
      DELAY400NS;
      pio_outbyte( node, CB_DC, devCtrl );
      DELAY400NS;
   }


   // If there is a device 0, wait for device 0 to set BSY=0.

   if ( node->reg_config_info[0] != REG_CONFIG_TYPE_NONE )
   {
      sub_atapi_delay( 0 );
      while ( 1 )
      {
         status = pio_inbyte( node, CB_STAT );
         if ( ( status & CB_STAT_BSY ) == 0 )
            break;
         if ( tmr_chk_timeout( node ) )
         {
            node->reg_cmd_info.to = 1;
            node->reg_cmd_info.ec = 1;
            break;
         }
      }
   }

   // If there is a device 1, wait until device 1 allows
   // register access.

   if ( node->reg_config_info[1] != REG_CONFIG_TYPE_NONE )
   {
      sub_atapi_delay( 1 );
      while ( 1 )
      {
         pio_outbyte( node, CB_DH, CB_DH_DEV1 );
         DELAY400NS;
         sc = pio_inbyte( node, CB_SC );
         sn = pio_inbyte( node, CB_SN );
         if ( ( sc == 0x01 ) && ( sn == 0x01 ) )
            break;
         if ( tmr_chk_timeout( node ) )
         {
            node->reg_cmd_info.to = 1;
            node->reg_cmd_info.ec = 2;
            break;
         }
      }

      // Now check if drive 1 set BSY=0.

      if ( node->reg_cmd_info.ec == 0 )
      {
         if ( pio_inbyte( node, CB_STAT ) & CB_STAT_BSY )
         {
            node->reg_cmd_info.ec = 3;
         }
      }
   }

   // RESET_DONE:

   // We are done but now we must select the device the caller
   // requested before we trace the command.  This will cause
   // the correct data to be returned in reg_cmd_info.

   pio_outbyte( node, CB_DH, devRtrn ? CB_DH_DEV1 : CB_DH_DEV0 );
   DELAY400NS;
   sub_trace_command( node );


   // All done.  The return values of this function are described in
   // ATAIO.H.

   if ( node->reg_cmd_info.ec ) return 1;
   return 0;
}

//*************************************************************
//
// sub_zero_return_data() -- zero the return data areas.
//
//*************************************************************

void sub_zero_return_data( struct atapi_node *node )
{
   node->reg_cmd_info.flg = TRC_FLAG_EMPTY;
   node->reg_cmd_info.ct  = TRC_TYPE_NONE;
   node->reg_cmd_info.cmd = 0;
   node->reg_cmd_info.fr1 = 0;
   node->reg_cmd_info.sc1 = 0;
   node->reg_cmd_info.sn1 = 0;
   node->reg_cmd_info.cl1 = 0;
   node->reg_cmd_info.ch1 = 0;
   node->reg_cmd_info.dh1 = 0;
   node->reg_cmd_info.dc1 = 0;
   node->reg_cmd_info.ec  = 0;
   node->reg_cmd_info.to  = 0;
   node->reg_cmd_info.st2 = 0;
   node->reg_cmd_info.as2 = 0;
   node->reg_cmd_info.er2 = 0;
   node->reg_cmd_info.sc2 = 0;
   node->reg_cmd_info.sn2 = 0;
   node->reg_cmd_info.cl2 = 0;
   node->reg_cmd_info.ch2 = 0;
   node->reg_cmd_info.dh2 = 0;
   node->reg_cmd_info.totalBytesXfer = 0L;
   node->reg_cmd_info.failbits = 0;
   node->reg_cmd_info.drqPackets = 0L;
}

//*************************************************************
//
// sub_atapi_delay() - delay for at least two ticks of the bios
//                     timer (or at least 110ms).
//
//*************************************************************

void sub_atapi_delay( int dev )
{
    smk_ndelay(110);
    return;
}

void sub_xfer_delay( void )

{

    smk_ndelay(60);
   
//   lw = tmr_read_bios_timer();
//   while ( lw == tmr_read_bios_timer() )
      /* do nothing */ ;
}



//*************************************************************
//
// sub_trace_command() -- trace the end of a command.
//
//*************************************************************

void sub_trace_command( struct atapi_node *node )
{
   node->reg_cmd_info.st2 = pio_inbyte( node, CB_STAT );
   node->reg_cmd_info.as2 = pio_inbyte( node, CB_ASTAT );
   node->reg_cmd_info.er2 = pio_inbyte( node, CB_ERR );
   node->reg_cmd_info.sc2 = pio_inbyte( node, CB_SC );
   node->reg_cmd_info.sn2 = pio_inbyte( node, CB_SN );
   node->reg_cmd_info.cl2 = pio_inbyte( node, CB_CL );
   node->reg_cmd_info.ch2 = pio_inbyte( node, CB_CH );
   node->reg_cmd_info.dh2 = pio_inbyte( node, CB_DH );
}

int sub_select( struct atapi_node *node, int dev )
{
   unsigned char status;

   // PAY ATTENTION HERE
   // The caller may want to issue a command to a device that doesn't
   // exist (for example, Exec Dev Diag), so if we see this,
   // just select that device, skip all status checking and return.
   // We assume the caller knows what they are doing!
   pio_outbyte( node, CB_DH, dev ? CB_DH_DEV1 : CB_DH_DEV0 );
   DELAY400NS;

   if ( node->reg_config_info[ dev ] < REG_CONFIG_TYPE_ATA )
   {
      // select the device and return
      return 0;
   }

   // The rest of this is the normal ATA stuff for device selection
   // and we don't expect the caller to be selecting a device that
   // does not exist.
   // We don't know which drive is currently selected but we should
   // wait for it to be not BUSY.  Normally it will be not BUSY
   // unless something is very wrong!

   while ( 1 )
   {
      status = pio_inbyte( node, CB_STAT );
      if ( ( status & CB_STAT_BSY ) == 0 )
         break;
      if ( tmr_chk_timeout( node ) )
      {
         node->reg_cmd_info.to = 1;
         node->reg_cmd_info.ec = 11;
         node->reg_cmd_info.st2 = status;
         node->reg_cmd_info.as2 = pio_inbyte( node, CB_ASTAT );
         node->reg_cmd_info.er2 = pio_inbyte( node, CB_ERR );
         node->reg_cmd_info.sc2 = pio_inbyte( node, CB_SC );
         node->reg_cmd_info.sn2 = pio_inbyte( node, CB_SN );
         node->reg_cmd_info.cl2 = pio_inbyte( node, CB_CL );
         node->reg_cmd_info.ch2 = pio_inbyte( node, CB_CH );
         node->reg_cmd_info.dh2 = pio_inbyte( node, CB_DH );
         return 1;
      }
   }

   // Here we select the drive we really want to work with by
   // putting 0xA0 or 0xB0 in the Drive/Head register (1f6).

   pio_outbyte( node, CB_DH, dev ? CB_DH_DEV1 : CB_DH_DEV0 );
   DELAY400NS;

   // If the selected device is an ATA device,
   // wait for it to have READY and SEEK COMPLETE
   // status.  Normally the drive should be in this state unless
   // something is very wrong (or initial power up is still in
   // progress).  For any other type of device, just wait for
   // BSY=0 and assume the caller knows what they are doing.

   while ( 1 )
   {
      status = pio_inbyte( node, CB_STAT );
      if ( node->reg_config_info[ dev ] == REG_CONFIG_TYPE_ATA )
      {
           if ( ( status & ( CB_STAT_BSY | CB_STAT_RDY | CB_STAT_SKC ) )
                     == ( CB_STAT_RDY | CB_STAT_SKC ) )
         break;
      }
      else
      {
         if ( ( status & CB_STAT_BSY ) == 0 )
            break;
      }
      if ( tmr_chk_timeout( node ) )
      {
         node->reg_cmd_info.to = 1;
         node->reg_cmd_info.ec = 12;
         node->reg_cmd_info.st2 = status;
         node->reg_cmd_info.as2 = pio_inbyte( node, CB_ASTAT );
         node->reg_cmd_info.er2 = pio_inbyte( node, CB_ERR );
         node->reg_cmd_info.sc2 = pio_inbyte( node, CB_SC );
         node->reg_cmd_info.sn2 = pio_inbyte( node, CB_SN );
         node->reg_cmd_info.cl2 = pio_inbyte( node, CB_CL );
         node->reg_cmd_info.ch2 = pio_inbyte( node, CB_CH );
         node->reg_cmd_info.dh2 = pio_inbyte( node, CB_DH );
         return 1;
      }
   }

   // All done.  The return values of this function are described in
   // ATAIO.H.

   if ( node->reg_cmd_info.ec )
      return 1;
   return 0;
}



//*************************************************************
//
// tmr_set_timeout() - function used to set command timeout time
//
// The command time out time is computed as follows:
//
//    timer + ( tmr_time_out * 18 )
//
//**************************************************************

void tmr_set_timeout( struct atapi_node *node )

{

   // get value of BIOS timer

   node->timeOut = tmr_read_bios_timer();

   // add command timeout value

   node->timeOut = node->timeOut + ( node->tmr_time_out * 18L );

   // ignore the lower 4 bits

   node->timeOut = node->timeOut & 0xfffffff0;
}

//*************************************************************
//
// tmr_chk_timeout() - function used to check for command timeout.
//
// Gives non-zero return if command has timed out.
//
//**************************************************************

int tmr_chk_timeout( struct atapi_node *node )
{
   unsigned int curTime;

   // get current time

   curTime = tmr_read_bios_timer();

   // ignore lower 4 bits

   curTime = curTime & 0xfffffff0;

   // timed out yet ?

   if ( curTime >= node->timeOut ) return 1;

   return 0;
}

//*************************************************************
//
// tmr_read_bios_timer() - function to read the BIOS timer
//
//**************************************************************

unsigned int tmr_read_bios_timer( void )
{
   unsigned int seconds;
   unsigned int milli;
   
   smk_system_time( &seconds, &milli );
   return seconds;
}



// **********************************************

int reg_pio_data_in_lba( struct atapi_node *node,
						 int dev, int cmd,
                         int fr, int sc,
                         long lba,
			 			 void* buffer,
                         int numSect, int multiCnt )

{
   unsigned int cyl;
   int head, sect;

   sect = (int) ( lba & 0x000000ffL );
   lba = lba >> 8;
   cyl = (int) ( lba & 0x0000ffff );
   lba = lba >> 16;
   head = ( (int) ( lba & 0x0fL ) ) | 0x40;
   return reg_pio_data_in( node, dev, cmd,
                           fr, sc,
                           cyl, head, sect,
                           buffer,
                           numSect, multiCnt );
}


// -----------------------------------------

int reg_pio_data_in( struct atapi_node *node,
					 int dev, int cmd,
                     int fr, int sc,
                     unsigned int cyl, int head, int sect,
	             	 void* buffer,
                     int numSect, int multiCnt )

{
   unsigned char devHead;
   unsigned char devCtrl;
   unsigned char cylLow;
   unsigned char cylHigh;
   unsigned char status;
   unsigned int wordCnt;

   // mark start of PDI cmd in low level trace


   // setup register values and adjust parameters

   devCtrl = CB_DC_HD15 | ( node->int_use_intr_flag ? 0 : CB_DC_NIEN );
   devHead = dev ? CB_DH_DEV1 : CB_DH_DEV0;
   devHead = devHead | ( head & 0x4f );
   cylLow = cyl & 0x00ff;
   cylHigh = ( cyl & 0xff00 ) >> 8;
   // these commands transfer only 1 sector
   if (    ( cmd == CMD_IDENTIFY_DEVICE )
        || ( cmd == CMD_IDENTIFY_DEVICE_PACKET )
        || ( cmd == CMD_READ_BUFFER )
      )
      numSect = 1;
   // only Read Multiple uses multiCnt
   if ( cmd != CMD_READ_MULTIPLE )
      multiCnt = 1;

   // Reset error return data.

   sub_zero_return_data( node );
   node->reg_cmd_info.flg = TRC_FLAG_CMD;
   node->reg_cmd_info.ct  = TRC_TYPE_APDI;
   node->reg_cmd_info.cmd = cmd;
   node->reg_cmd_info.fr1 = fr;
   node->reg_cmd_info.sc1 = sc;
   node->reg_cmd_info.sn1 = sect;
   node->reg_cmd_info.cl1 = cylLow;
   node->reg_cmd_info.ch1 = cylHigh;
   node->reg_cmd_info.dh1 = devHead;
   node->reg_cmd_info.dc1 = devCtrl;

   // Set command time out.

   tmr_set_timeout( node );

   // Select the drive - call the sub_select function.
   // Quit now if this fails.

   if ( sub_select( node, dev ) )
   {
      sub_trace_command( node );
      return 1;
   }

   // Set up all the registers except the command register.

   pio_outbyte( node, CB_DC, devCtrl );
   pio_outbyte( node, CB_FR, fr );
   pio_outbyte( node, CB_SC, sc );
   pio_outbyte( node, CB_SN, sect );
   pio_outbyte( node, CB_CL, cylLow );
   pio_outbyte( node, CB_CH, cylHigh );
   pio_outbyte( node, CB_DH, devHead );

   // For interrupt mode,
   // Take over INT 7x and initialize interrupt controller
   // and reset interrupt flag.

   //int_save_int_vect();

   // Start the command by setting the Command register.  The drive
   // should immediately set BUSY status.

   pio_outbyte( node, CB_CMD, cmd );

   // Waste some time by reading the alternate status a few times.
   // This gives the drive time to set BUSY in the status register on
   // really fast systems.  If we don't do this, a slow drive on a fast
   // system may not set BUSY fast enough and we would think it had
   // completed the command when it really had not even started the
   // command yet.

   DELAY400NS;

   // Loop to read each sector.

   while ( 1 )
   {
      // READ_LOOP:
      //
      // NOTE NOTE NOTE ...  The primary status register (1f7) MUST NOT be
      // read more than ONCE for each sector transferred!  When the
      // primary status register is read, the drive resets IRQ 14.  The
      // alternate status register (3f6) can be read any number of times.
      // After INT 7x read the the primary status register ONCE
      // and transfer the 256 words (REP INSW).  AS SOON as BOTH the
      // primary status register has been read AND the last of the 256
      // words has been read, the drive is allowed to generate the next
      // IRQ 14 (newer and faster drives could generate the next IRQ 14 in
      // 50 microseconds or less).  If the primary status register is read
      // more than once, there is the possibility of a race between the
      // drive and the software and the next IRQ 14 could be reset before
      // the system interrupt controller sees it.

      // Wait for INT 7x -or- wait for not BUSY -or- wait for time out.

      sub_atapi_delay( dev );
      reg_wait_poll( node, 34, 35 );

      // Read the primary status register.  In keeping with the rules
      // stated above the primary status register is read only
      // ONCE.

      status = pio_inbyte( node, CB_STAT );

      // If there was a time out error, go to READ_DONE.

      if ( node->reg_cmd_info.ec )
      {
//         printf("%s\n","ata: time out error");
         break;   // go to READ_DONE
      }

      // If BSY=0 and DRQ=1, transfer the data,
      // even if we find out there is an error later.

      int reg_slow_xfer_flag = 0;

      if ( ( status & ( CB_STAT_BSY | CB_STAT_DRQ ) ) == CB_STAT_DRQ )
      {
         // do the slow data transfer thing
//           printf("%s\n","ata: BSY = 0, DRQ = 1");

         if ( reg_slow_xfer_flag )
         {
            if ( numSect <= reg_slow_xfer_flag )
            {
               sub_xfer_delay();
               reg_slow_xfer_flag = 0;
            }
         }

         // increment number of DRQ packets

         node->reg_cmd_info.drqPackets ++ ;

         // determine the number of sectors to transfer

         wordCnt = multiCnt ? multiCnt : 1;
         if ( (int)wordCnt > numSect )
            wordCnt = numSect;
         wordCnt = wordCnt * 256;

         // Do the REP INSW to read the data for one block.

         node->reg_cmd_info.totalBytesXfer += ( wordCnt << 1 );
         pio_rep_inword( node, CB_DATA, buffer, wordCnt );

         DELAY400NS;    // delay so device can get the status updated

         // Note: The drive should have dropped DATA REQUEST by now.  If there
         // are more sectors to transfer, BUSY should be active now (unless
         // there is an error).

         // Decrement the count of sectors to be transferred
         // and increment buffer address.

         numSect = numSect - ( multiCnt ? multiCnt : 1 );
         buffer = (void*)
	          ((unsigned long)buffer + 
		      ( 512 * ( multiCnt ? multiCnt : 1 ) ) );
      }

      // So was there any error condition?

      if ( status & ( CB_STAT_BSY | CB_STAT_DF  | CB_STAT_ERR ) )
      {
//         printf("%s%x\n","ata: there were error conditions -> ", status);
         node->reg_cmd_info.ec = 31;
         break;   // go to READ_DONE
      }

      // DRQ should have been set -- was it?

      if ( ( status & CB_STAT_DRQ ) == 0 )
      {
//         printf("%s\n","ata: drq was not set. ");
         node->reg_cmd_info.ec = 32;
         break;   // go to READ_DONE
      }

      // If all of the requested sectors have been transferred, make a
      // few more checks before we exit.

      if ( numSect < 1 )
      {
         // Since the drive has transferred all of the requested sectors
         // without error, the drive should not have BUSY, DEVICE FAULT,
         // DATA REQUEST or ERROR active now.

         sub_atapi_delay( dev );
         status = pio_inbyte( node, CB_STAT );
         if ( status & ( CB_STAT_BSY | CB_STAT_DF | CB_STAT_DRQ | CB_STAT_ERR ) )
         {
//            printf("%s%x\n","ata: secondary error -> ", status );
            node->reg_cmd_info.ec = 33;
            break;   // go to READ_DONE
         }

         // All sectors have been read without error, go to READ_DONE.

         break;   // go to READ_DONE

      }

      // This is the end of the read loop.  If we get here, the loop is
      // repeated to read the next sector.  Go back to READ_LOOP.

   }

   // read the output registers and trace the command.

   sub_trace_command( node );

   // READ_DONE:

   // For interrupt mode, restore the INT 7x vector.

   //int_restore_int_vect();

   // mark end of PDI cmd in low level trace


   // All done.  The return values of this function are described in
   // ATAIO.H.

   if ( node->reg_cmd_info.ec )
      return 1;
   return 0;
}


//*************************************************************
//
// reg_pio_data_out_lba() - Easy way to execute a PIO Data In command
//                          using an LBA sector address.
//
//*************************************************************

int reg_pio_data_out_lba( struct atapi_node *node,
						  int dev, int cmd,
                          int fr, int sc,
                          long lba,
						  void* buffer,
                          int numSect, int multiCnt )

{
   unsigned int cyl;
   int head, sect;

   sect = (int) ( lba & 0x000000ffL );
   lba = lba >> 8;
   cyl = (int) ( lba & 0x0000ffff );
   lba = lba >> 16;
   head = ( (int) ( lba & 0x0fL ) ) | 0x40;
   return reg_pio_data_out( node, dev, cmd,
                            fr, sc,
                            cyl, head, sect,
                            buffer,
                            numSect, multiCnt );
}

//*************************************************************
//
// reg_pio_data_out() - Execute a PIO Data Out command.
//
// See ATA-2 Section 9.4, ATA-3 Section 9.4,
// ATA-4 Section 8.7 Figure 11.
//
//*************************************************************

int reg_pio_data_out( struct atapi_node *node,
					  int dev, int cmd,
                      int fr, int sc,
                      unsigned int cyl, int head, int sect,
                      void* buffer,
                      int numSect, int multiCnt )

{
   unsigned char devHead;
   unsigned char devCtrl;
   unsigned char cylLow;
   unsigned char cylHigh;
   unsigned char status;
   int loopFlag = 1;
   unsigned int wordCnt;

   // mark start of PDO cmd in low level trace



   // setup register values and adjust parameters

   devCtrl = CB_DC_HD15 | ( node->int_use_intr_flag ? 0 : CB_DC_NIEN );
   devHead = dev ? CB_DH_DEV1 : CB_DH_DEV0;
   devHead = devHead | ( head & 0x4f );
   cylLow = cyl & 0x00ff;
   cylHigh = ( cyl & 0xff00 ) >> 8;
   // these commands transfer only 1 sector
   if ( cmd == CMD_WRITE_BUFFER )
      numSect = 1;
   // only Write Multiple and CFA Write Multiple W/O Erase uses multCnt
   if (    ( cmd != CMD_WRITE_MULTIPLE )
        && ( cmd != CMD_CFA_WRITE_MULTIPLE_WO_ERASE )
      )
      multiCnt = 1;

   // Reset error return data.

   sub_zero_return_data( node );
   node->reg_cmd_info.flg = TRC_FLAG_CMD;
   node->reg_cmd_info.ct  = TRC_TYPE_APDO;
   node->reg_cmd_info.cmd = cmd;
   node->reg_cmd_info.fr1 = fr;
   node->reg_cmd_info.sc1 = sc;
   node->reg_cmd_info.sn1 = sect;
   node->reg_cmd_info.cl1 = cylLow;
   node->reg_cmd_info.ch1 = cylHigh;
   node->reg_cmd_info.dh1 = devHead;
   node->reg_cmd_info.dc1 = devCtrl;

   // Set command time out.

   tmr_set_timeout( node );

   // Select the drive - call the sub_select function.
   // Quit now if this fails.

   if ( sub_select( node, dev ) )
   {
      sub_trace_command( node );
      return 1;
   }

   // Set up all the registers except the command register.

   pio_outbyte( node, CB_DC, devCtrl );
   pio_outbyte( node, CB_FR, fr );
   pio_outbyte( node, CB_SC, sc );
   pio_outbyte( node, CB_SN, sect );
   pio_outbyte( node, CB_CL, cylLow );
   pio_outbyte( node, CB_CH, cylHigh );
   pio_outbyte( node, CB_DH, devHead );

   // For interrupt mode,
   // Take over INT 7x and initialize interrupt controller
   // and reset interrupt flag.

//   int_save_int_vect();

   // Start the command by setting the Command register.  The drive
   // should immediately set BUSY status.

   pio_outbyte( node, CB_CMD, cmd );

   // Waste some time by reading the alternate status a few times.
   // This gives the drive time to set BUSY in the status register on
   // really fast systems.  If we don't do this, a slow drive on a fast
   // system may not set BUSY fast enough and we would think it had
   // completed the command when it really had not even started the
   // command yet.

   DELAY400NS;

   // Wait for not BUSY or time out.
   // Note: No interrupt is generated for the
   // first sector of a write command.  Well...
   // that's not really true we are working with
   // a PCMCIA PC Card ATA device.

   sub_atapi_delay( dev );
   while ( 1 )
   {
      status = pio_inbyte( node, CB_ASTAT );
      if ( ( status & CB_STAT_BSY ) == 0 )
         break;
      if ( tmr_chk_timeout( node ) )
      {
         node->reg_cmd_info.to = 1;
         node->reg_cmd_info.ec = 47;
         loopFlag = 0;
         break;
      }
   }

   // If we are using interrupts and we just got an interrupt, this is
   // wrong.  The drive must not generate an interrupt at this time.

   if ( loopFlag && node->int_use_intr_flag /*&& int_intr_flag*/ )
   {
      node->reg_cmd_info.ec = 46;
      loopFlag = 0;
   }

   // This loop writes each sector.

   while ( loopFlag )
   {
      // WRITE_LOOP:
      //
      // NOTE NOTE NOTE ...  The primary status register (1f7) MUST NOT be
      // read more than ONCE for each sector transferred!  When the
      // primary status register is read, the drive resets IRQ 14.  The
      // alternate status register (3f6) can be read any number of times.
      // For correct results, transfer the 256 words (REP OUTSW), wait for
      // INT 7x and then read the primary status register.  AS
      // SOON as BOTH the primary status register has been read AND the
      // last of the 256 words has been written, the drive is allowed to
      // generate the next IRQ 14 (newer and faster drives could generate
      // the next IRQ 14 in 50 microseconds or less).  If the primary
      // status register is read more than once, there is the possibility
      // of a race between the drive and the software and the next IRQ 14
      // could be reset before the system interrupt controller sees it.

      // If BSY=0 and DRQ=1, transfer the data,
      // even if we find out there is an error later.

      int reg_slow_xfer_flag = 0;

      if ( ( status & ( CB_STAT_BSY | CB_STAT_DRQ ) ) == CB_STAT_DRQ )
      {
         // do the slow data transfer thing

         if ( reg_slow_xfer_flag )
         {
            if ( numSect <= reg_slow_xfer_flag )
            {
               sub_xfer_delay();
               reg_slow_xfer_flag = 0;
            }
         }

         // increment number of DRQ packets

         node->reg_cmd_info.drqPackets ++ ;

         // determine the number of sectors to transfer

         wordCnt = multiCnt ? multiCnt : 1;
         if ( (int)wordCnt > numSect )
            wordCnt = numSect;
         wordCnt = wordCnt * 256;

         // Do the REP OUTSW to write the data for one block.

         node->reg_cmd_info.totalBytesXfer += ( wordCnt << 1 );
         pio_rep_outword( node, CB_DATA, buffer, wordCnt );

         DELAY400NS;    // delay so device can get the status updated

         // Note: The drive should have dropped DATA REQUEST and
         // raised BUSY by now.

         // Decrement the count of sectors to be transferred
         // and increment buffer address.

         numSect = numSect - ( multiCnt ? multiCnt : 1 );
         buffer = (void*)
	          ((unsigned long)buffer + 
		      ( 512 * ( multiCnt ? multiCnt : 1 ) ));
      }

      // So was there any error condition?

      if ( status & ( CB_STAT_BSY | CB_STAT_DF | CB_STAT_ERR ) )
      {
         node->reg_cmd_info.ec = 41;
         break;   // go to WRITE_DONE
      }

      // DRQ should have been set -- was it?

      if ( ( status & CB_STAT_DRQ ) == 0 )
      {
         node->reg_cmd_info.ec = 42;
         break;   // go to WRITE_DONE
      }

      // Wait for INT 7x -or- wait for not BUSY -or- wait for time out.

      sub_atapi_delay( dev );
      reg_wait_poll( node, 44, 45 );

      // Read the primary status register.  In keeping with the rules
      // stated above the primary status register is read only ONCE.

      status = pio_inbyte( node, CB_STAT );

      // If there was a time out error, go to WRITE_DONE.

      if ( node->reg_cmd_info.ec )
         break;   // go to WRITE_DONE

      // If all of the requested sectors have been transferred, make a
      // few more checks before we exit.

      if ( numSect < 1 )
      {
         // Since the drive has transferred all of the sectors without
         // error, the drive MUST not have BUSY, DEVICE FAULT, DATA REQUEST
         // or ERROR status at this time.

         if ( status & ( CB_STAT_BSY | CB_STAT_DF | CB_STAT_DRQ | CB_STAT_ERR ) )
         {
            node->reg_cmd_info.ec = 43;
            break;   // go to WRITE_DONE
         }

         // All sectors have been written without error, go to WRITE_DONE.

         break;   // go to WRITE_DONE

      }

      //
      // This is the end of the write loop.  If we get here, the loop
      // is repeated to write the next sector.  Go back to WRITE_LOOP.

   }

   // read the output registers and trace the command.

   sub_trace_command( node );

   // WRITE_DONE:

   // For interrupt mode, restore the INT 7x vector.

//   int_restore_int_vect();

   // mark end of PDO cmd in low level trace


   // All done.  The return values of this function are described in
   // ATAIO.H.

   if ( node->reg_cmd_info.ec )
      return 1;
   return 0;
}


