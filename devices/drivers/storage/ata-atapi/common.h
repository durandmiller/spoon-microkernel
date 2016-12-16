#ifndef _COMMON_H
#define _COMMON_H

#include "ataio.h"


#define ATAPI_TIMEOUT		30

#define DELAY400NS  { pio_inbyte( node, CB_ASTAT ); \
					  pio_inbyte( node, CB_ASTAT ); \
                      pio_inbyte( node, CB_ASTAT ); \
					  pio_inbyte( node, CB_ASTAT ); }


struct atapi_node
{
	unsigned int _base;
	int _device;

	struct REG_CMD_INFO reg_cmd_info;
	unsigned int pio_memory_seg;
	unsigned int pio_base_addr1;
	unsigned int pio_base_addr2;
	unsigned int pio_reg_addrs[10];
	int reg_config_info[2];
	      
	unsigned char pio_last_write[10];
	unsigned char pio_last_read[10];
	
	int pio_xfer_width;
	int reg_atapi_delay_flag;
	int int_use_intr_flag;
	
	unsigned int tmr_time_out;
	unsigned int timeOut;
};


void init_atapi( struct atapi_node *node, unsigned int base, int device );
      
void pio_set_iobase_addr( struct atapi_node *node,
						  unsigned int base1, unsigned int base2 );


void reg_wait_poll( struct atapi_node *node, int we, int pe );

int reg_reset( struct atapi_node *node, int skipFlag, int devRtrn );

unsigned char 	pio_inbyte( struct atapi_node *node, 
							unsigned int addr );
void 			pio_outbyte( struct atapi_node *node, 
							unsigned int addr, unsigned char data );
unsigned int 	pio_inword( struct atapi_node *node, 
							unsigned int addr );
void 			pio_outword( struct atapi_node *node, 
							unsigned int addr, unsigned int data );


void pio_rep_inbyte( struct atapi_node *node, 
							unsigned int addrDataReg, void* buffer, long byteCnt );
void pio_rep_outbyte( struct atapi_node *node, 
							unsigned int addrDataReg, void* buffer, long byteCnt );
void pio_rep_indword( struct atapi_node *node, 
							unsigned int addrDataReg, void* buffer, unsigned int dwordCnt );
void pio_rep_outdword( struct atapi_node *node, 
							unsigned int addrDataReg, void* buffer, unsigned int dwordCnt );
void pio_rep_inword( struct atapi_node *node, 
							unsigned int addrDataReg, void* buffer, unsigned int wordCnt );
void pio_rep_outword( struct atapi_node *node, 
							unsigned int addrDataReg, void* buffer, unsigned int wordCnt );
int reg_pio_data_in_lba( struct atapi_node *node, 
						int dev, int cmd, int fr, int sc,
                         long lba, void* buffer, int numSect, int multiCnt );

int reg_pio_data_in( struct atapi_node *node, 
					int dev, int cmd, int fr, int sc,
                     unsigned int cyl, int head, int sect,
		             void* buffer, int numSect, int multiCnt );

int reg_pio_data_out_lba( struct atapi_node *node, 
						int dev, int cmd, int fr, int sc,
                         long lba, void* buffer,
                         int numSect, int multiCnt );

int reg_pio_data_out( struct atapi_node *node, 
					int dev, int cmd, int fr, int sc,
                     unsigned int cyl, int head, int sect,
		             void* buffer, int numSect, int multiCnt );




void sub_zero_return_data( struct atapi_node *node );
void sub_atapi_delay( int dev );
void sub_xfer_delay( void );
void sub_trace_command( struct atapi_node *node );
int  sub_select( struct atapi_node *node, int dev );

int tmr_chk_timeout( struct atapi_node *node );
void tmr_set_timeout( struct atapi_node *node );
unsigned int tmr_read_bios_timer( void );

#endif

