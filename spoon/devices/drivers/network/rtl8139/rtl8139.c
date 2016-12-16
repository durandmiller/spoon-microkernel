#include <smk.h>
#include <devfs.h>


#include "rtl8139.h"

#define RTL_WRITE_8(rtl, reg, dat) \
	outb( (rtl)->io_port + (reg), dat )
#define RTL_WRITE_16(rtl, reg, dat) \
	outw( (rtl)->io_port + (reg), dat )
#define RTL_WRITE_32(rtl, reg, dat) \
	outl( (rtl)->io_port + (reg), dat )

#define RTL_READ_8(rtl, reg) \
	inb((rtl)->io_port + (reg))
#define RTL_READ_16(rtl, reg) \
	inw((rtl)->io_port + (reg))
#define RTL_READ_32(rtl, reg) \
	inl((rtl)->io_port + (reg))

#define TAILREG_TO_TAIL(in) \
		(uint16_t)(((uint32_t)(in) + 16) % 0x10000)
#define TAIL_TO_TAILREG(in) \
		(uint16_t)((uint16_t)(in) - 16)
				   


#define MYRT_INTS (RT_INT_PCIERR | RT_INT_RX_ERR | RT_INT_RX_OK | RT_INT_TX_ERR | RT_INT_TX_OK | RT_INT_RXBUF_OVERFLOW)
//#define MYRT_INTS ( RT_INT_RX_OK | RT_INT_TX_OK | RT_INT_TX_ERR )


// ***************************************

#define MAX_CARDS		16

static struct rtl8139 	cards[MAX_CARDS];
static int 				card_count = 0;




int rtl8139_init( uint32_t base, int irq )
{
	struct rtl8139 *rtl = &(cards[card_count]);
	int rc;
	int hasIRQ = 0;

	// Too many already
	if ( card_count == MAX_CARDS ) return -1;

	// Already exists?
	for ( rc = 0; rc < card_count; rc++ )
	{
		if ( (cards[rc].irq == irq) && (cards[rc].io_port == base) )
			return -1;

		if ( irq == cards[rc].irq ) hasIRQ = 1;
	}

	// Request the IRQ before we set it up to send IRQ's
	if ( hasIRQ == 0 )
	{
		rc = smk_request_irq( rtl8139_irq, irq, "rtl8139", NULL );
		if ( rc < 0 ) 
		{
			devfs_dmesg("Unable to request IRQ %i\n", irq );
			return -1;
		}
	}


	
	// Nope, new card in the system
	rtl->irq 		= irq;
	rtl->io_port 	= base;

	rtl->rxbuf = smk_mem_alloc( (64 * 1024 + 16) / 4096 + 1 );

	if ( rtl->rxbuf == NULL )
	{
		return -1;
	}
	
	rtl->txbuf = smk_mem_alloc( (8  * 1024     ) / 4096 + 1 );
	if ( rtl->txbuf == NULL )
	{
		smk_mem_free( rtl->rxbuf );
		return -1;
	}

	
	rtl->txbn  = 0;

	rtl->rxbuf_phys = (uintptr_t)smk_mem_location( rtl->rxbuf );
	rtl->txbuf_phys = (uintptr_t)smk_mem_location( rtl->txbuf );



	smk_disable_interrupts();
	
		RTL_WRITE_8(rtl, RT_CHIPCMD, RT_CMD_RESET);
		RTL_READ_8(rtl, RT_CHIPCMD);
		
		smk_ndelay( 1000 );
		while((RTL_READ_8(rtl, RT_CHIPCMD) & RT_CMD_RESET)) {
			/*
			if(system_time() - time > 1000000) {
				err = -1;
				goto err1;
			}
			*/
		}



		// read the mac address
		rtl->mac_addr[0] = RTL_READ_8(rtl, RT_IDR0);
		rtl->mac_addr[1] = RTL_READ_8(rtl, RT_IDR0 + 1);
		rtl->mac_addr[2] = RTL_READ_8(rtl, RT_IDR0 + 2);
		rtl->mac_addr[3] = RTL_READ_8(rtl, RT_IDR0 + 3);
	  	rtl->mac_addr[4] = RTL_READ_8(rtl, RT_IDR0 + 4);
	  	rtl->mac_addr[5] = RTL_READ_8(rtl, RT_IDR0 + 5);


		// enable writing to the config registers
		RTL_WRITE_8(rtl, RT_CFG9346, 0xc0);

		// reset config 1
		RTL_WRITE_8(rtl, RT_CONFIG1, 0);

		// Enable receive and transmit functions
		RTL_WRITE_8(rtl, RT_CHIPCMD, RT_CMD_RX_ENABLE | RT_CMD_TX_ENABLE);

		// Set Rx FIFO threashold to 1K, Rx size to 64k+16, 1024 byte DMA burst
		RTL_WRITE_32(rtl, RT_RXCONFIG, 0x0000de00);
	
		// Set Tx 1024 byte DMA burst
		RTL_WRITE_32(rtl, RT_TXCONFIG, 0x03000600);

		// Turn off lan-wake and set the driver-loaded bit
		RTL_WRITE_8(rtl, RT_CONFIG1, (RTL_READ_8(rtl, RT_CONFIG1) & ~0x30) | 0x20);
	
		// Enable FIFO auto-clear
		RTL_WRITE_8(rtl, RT_CONFIG4, RTL_READ_8(rtl, RT_CONFIG4) | 0x80);

		// go back to normal mode
		RTL_WRITE_8(rtl, RT_CFG9346, 0);



		// Setup RX buffers
		RTL_WRITE_32(rtl, RT_RXBUF, rtl->rxbuf_phys );

		// Setup TX buffers
		RTL_WRITE_32(rtl, RT_TXADDR0, rtl->txbuf_phys );
		RTL_WRITE_32(rtl, RT_TXADDR1, rtl->txbuf_phys + 2 * 1024);
		RTL_WRITE_32(rtl, RT_TXADDR2, rtl->txbuf_phys + 4 * 1024 );
		RTL_WRITE_32(rtl, RT_TXADDR3, rtl->txbuf_phys + 6 * 1024 );

		// Reset RXMISSED counter
		RTL_WRITE_32(rtl, RT_RXMISSED, 0);
	
		// Enable receiving broadcast and physical match packets
	//	RTL_WRITE_32(rtl, RT_RXCONFIG, RTL_READ_32(rtl, RT_RXCONFIG) | 0x0000000a);
		RTL_WRITE_32(rtl, RT_RXCONFIG, RTL_READ_32(rtl, RT_RXCONFIG) | 0x0000000f);

		// Filter out all multicast packets
		RTL_WRITE_32(rtl, RT_MAR0, 0);
		RTL_WRITE_32(rtl, RT_MAR0 + 4, 0);

		// Disable all multi-interrupts
		RTL_WRITE_16(rtl, RT_MULTIINTR, 0);

		// Send through the interrupt mask
		RTL_WRITE_16(rtl, RT_INTRMASK, MYRT_INTS);
	
		// Enable RX/TX once more
		RTL_WRITE_8(rtl, RT_CHIPCMD, RT_CMD_RX_ENABLE | RT_CMD_TX_ENABLE);

		RTL_WRITE_8(rtl, RT_CFG9346, 0);

	
	rc = card_count++;		// Keep it. It's good.

	smk_enable_interrupts();

	return rc;
}
		

void rtl8139_stop( rtl8139 *rtl )
{
	smk_disable_interrupts();

		// stop the rx and tx and mask all interrupts
		RTL_WRITE_8(rtl, RT_CHIPCMD, RT_CMD_RESET);
		RTL_WRITE_16(rtl, RT_INTRMASK, 0);

	smk_enable_interrupts();
}
							      

void rtl8139_resetrx( rtl8139 *rtl )
{
		// stop the rx and tx and mask all interrupts
		RTL_WRITE_8(rtl, RT_CHIPCMD, RT_CMD_RESET);
		RTL_WRITE_16(rtl, RT_INTRMASK, 0);
	
        // reset the rx pointers
        RTL_WRITE_16(rtl, RT_RXBUFTAIL, TAIL_TO_TAILREG(0));
        RTL_WRITE_16(rtl, RT_RXBUFHEAD, 0);

        // start it back up
        RTL_WRITE_16(rtl, RT_INTRMASK, MYRT_INTS);
		
        // Enable RX/TX once more
        RTL_WRITE_8(rtl, RT_CHIPCMD, RT_CMD_RX_ENABLE | RT_CMD_TX_ENABLE);
}


				  




// -------------------------------------

int rtl8139_transmit( struct rtl8139 *rtl, void *ptr, int len )
{
restart:
	smk_disable_interrupts();
	
		/* wait for clear-to-send */
		if(!(RTL_READ_32(rtl, RT_TXSTATUS0 + rtl->txbn*4) & RT_TX_HOST_OWNS)) {
			devfs_dmesg("rtl8139_xmit: no txbuf free.\n");
			smk_enable_interrupts();
			goto restart;
		}	
	
		devfs_memcpy((void*)((void*)rtl->txbuf + rtl->txbn * 0x800), ptr, len);
		if(len < 64) 
			len = 64;
	
		RTL_WRITE_32(rtl, RT_TXSTATUS0 + (rtl->txbn)*4, len | 0x80000);
		if(++rtl->txbn >= 4)
			rtl->txbn = 0;

	smk_enable_interrupts();
	return 0;
}


void rtl8139_intRX( struct rtl8139 *rtl )
{
	rx_entry *entry;
	uint32_t tail;
	uint16_t len;
	int rc;

	char buf[16384];
	int buf_len = 16384;
					

	if(!(RTL_READ_8(rtl, RT_CHIPCMD) & RT_CMD_RX_BUF_EMPTY)) {
		// Should release semaphore here or something..
	}

	tail = TAILREG_TO_TAIL(RTL_READ_16(rtl, RT_RXBUFTAIL));

	if ( tail == RTL_READ_16(rtl, RT_RXBUFHEAD)) 
	{
		devfs_dmesg("tail == RTL and going back");
		return;
	}

	if(RTL_READ_8(rtl, RT_CHIPCMD) & RT_CMD_RX_BUF_EMPTY) 
	{
		devfs_dmesg("CMD & EMPTY... returning");
		return;
	}
						      

	entry = (rx_entry *)((void*)rtl->rxbuf + tail);

	if(entry->len == 0xfff0) 
	{
		devfs_dmesg("unfinished buffer?");
		return;
	}
						      
	// figure the len that we need to copy
        len = entry->len - 4; // minus the crc

#define ETHERNET_HEADER_SIZE (6+6+2)
#define ETHERNET_MAX_SIZE (ETHERNET_HEADER_SIZE+1500)
#define ETHERNET_MIN_SIZE (ETHERNET_HEADER_SIZE+46)


	if((entry->status & RT_RX_STATUS_OK) == 0 || len > ETHERNET_MAX_SIZE) 
	{
		rtl8139_resetrx(rtl);
		smk_enable_interrupts();
		devfs_dmesg("ok and too big");
		return;
	}
								  

	// copy the buffer
	if(len > buf_len) {
		devfs_dmesg("rtl8139_rx: packet too large for buffer (len %i, buf_len %i)\n", len, buf_len);
		RTL_WRITE_16(rtl, RT_RXBUFTAIL, TAILREG_TO_TAIL(RTL_READ_16(rtl, RT_RXBUFHEAD)));
		goto out;
	}
	if(tail + len > 0xffff) {
		devfs_dmesg("packet wraps around.\n");
		devfs_memcpy(buf, (const void *)&entry->data[0], 0x10000 - (tail + 4));
		devfs_memcpy((uint8_t *)buf + 0x10000 - (tail + 4), (const void *)rtl->rxbuf, len - (0x10000 - (tail + 4)));
	} else {
		devfs_memcpy(buf, (const void *)&entry->data[0], len);
	}
	rc = len;

        // calculate the new tail
        tail = ((tail + entry->len + 4 + 3) & ~3) % 0x10000;
//      dprintf("new tail at 0x%x, tailreg will say 0x%x\n", tail, TAIL_TO_TAILREG(tail));
        RTL_WRITE_16(rtl, RT_RXBUFTAIL, TAIL_TO_TAILREG(tail));

        if(tail != RTL_READ_16(rtl, RT_RXBUFHEAD)) {
                // we're at last one more packet behind
//                release_sem = true;
        }

out:
	smk_enable_interrupts();

	devfs_dmesg("packet received on ethernet\n");

}

void rtl8139_intTX( struct rtl8139 *rtl, uint16_t status )
{
	uint32_t txstat;
	int i;

	for(i=0; i<4; i++) {
//		if(grtl->last_txbn == grtl->txbn)
//			break;
		txstat = RTL_READ_32(rtl, RT_TXSTATUS0 + rtl->txbn*4);
		if(txstat & RT_TX_HOST_OWNS) {
//			printf("%s\n","host now owns the buffer\n");
		} else {
//			printf("%s\n","host no own\n");
			break;
		}
//		if(++grtl->last_txbn >= 4)
//			grtl->last_txbn = 0;
	}
}


static int rtl8139_checkCard( int i )
{
	struct rtl8139 *rtl = &(cards[i]);


		// Disable interrupts
		RTL_WRITE_16(rtl, RT_INTRMASK, 0);
	
		for(;;) {
	
			uint16_t status = RTL_READ_16(rtl, RT_INTRSTATUS);

			if(status != 0)
				RTL_WRITE_16(rtl, RT_INTRSTATUS, status);
			else
				break;
	
			if(status & RT_INT_TX_OK)
			{
				rtl8139_intTX( rtl, status);
			}
			if ( status & RT_INT_TX_ERR) 
			{
				devfs_dmesg("interrupt tx ERR");
			}
			if(status & RT_INT_RX_ERR)
			{
				devfs_dmesg("interrupt rx ERR");
			}
			if ( status & RT_INT_RX_OK) 
			{
				rtl8139_intRX( rtl );
			}
			
			if(status & RT_INT_RXBUF_OVERFLOW) {
				devfs_dmesg("RX buffer overflow!\n");
				devfs_dmesg("buf %x, head %x, tail %x\n", 
							RTL_READ_32(rtl, RT_RXBUF), 
							RTL_READ_16(rtl, RT_RXBUFHEAD), 
							RTL_READ_16(rtl, RT_RXBUFTAIL));
				
				RTL_WRITE_32(rtl, RT_RXMISSED, 0);
				RTL_WRITE_16(rtl, RT_RXBUFTAIL, TAILREG_TO_TAIL(RTL_READ_16(rtl, RT_RXBUFHEAD)));
			}
			if(status & RT_INT_RXFIFO_OVERFLOW) {
				devfs_dmesg("RX fifo overflow!\n");
			}
			if(status & RT_INT_RXFIFO_UNDERRUN) {
				devfs_dmesg("RX fifo underrun\n");
			}
	
			
		}

		// reenable interrupts
		RTL_WRITE_16(rtl, RT_INTRMASK, MYRT_INTS);

	return 0;
}


int rtl8139_irq( int irq, void *data )
{
	int i;

	smk_disable_interrupts();
		
		for ( i = 0; i < card_count; i++ )
		{
			rtl8139_checkCard( i );
		}

	smk_enable_interrupts();

	smk_ack_irq( irq );
	return 0;
}



