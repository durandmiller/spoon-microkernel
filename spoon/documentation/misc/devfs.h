


/** \mainpage Device File System

  The Device File System is a system made out of three components: devfs,
  device drivers and libdevfs.

  \image html devfslay.jpg 

  The devfs is the userland server application with which all devices/drivers  
  will register and receive events from. In addition, all userland 
  applications which wish to make use of any device registered in the 
  system will interact directly with devfs. Devfs will, in turn, relay 
  all requests to the relevant device drivers if necessary and device drivers 
  respond directly to the devfs when a response is required.

  All device drivers are standalone userland applications, sometimes with 
  special
  IO privileges granted by the kernel. All device drivers should register
  themselves with the devfs using a nodename, for example: input/kbd or 
  storage/ata0. This is the node name that will be used whenever another
  user application wants to perform operations on the device.

  libdevfs is the glue that holds it all together. It's a support library
  which includes standard operations, memory operations and devfs/driver
  interfacing routines - all of which is designed to make device driver
  programming effortless, fast and painless.
  
  
  What follows is a list of the relevant sections you'll need in order 
  to make use of the devfs.

		-	\ref DEVHASH
		-	\ref DEVMISC
		-	\ref DEVOPS
		-	\ref DEVREG

		
 \note  Please have a look at the top index for the "Examples" page. You'll find
 		 some simple code there.

*/

/** \example null.c
 *
 * This is the example NULL device driver which is included as an
 * example on how to write a device driver using libdevfs and devfs.
 *
 */


/** \example firewall.c
 *
 * This is an userland application which demonstrates the 
 * use of devfs. It's a simple network packet repeater (is there a real
 * name for that?).
 *
 * Basically, all packets coming in on network card 1 will be sent out
 * on network card 2. And vice-versa. If you want to put a firewall in 
 * place, just add some logic to analyze the packet before it gets
 * sent out on the other network card.
 *
 * I think it's a nice example that demonstrates the power of the
 * devfs and libdevfs library.
 * 
 */


