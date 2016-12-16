#include <smk.h>
#include <devfs.h>

#include "8042.h"


// Command port (0x64) specific commands

#define i8042_CMD_READ_BYTE_ZERO			0x20
#define i8042_CMD_DISABLE_MOUSE				0xA7
#define i8042_CMD_ENABLE_MOUSE				0xA8
#define i8042_CMD_INITIATE_SELF_TEST		0xAA
#define i8042_CMD_INITIATE_INTERFACE_TEST	0xAB
#define i8042_CMD_DIAGNOSTIC_DUMP			0xAC
#define i8042_CMD_DISABLE_KEYBOARD			0xAD
#define i8042_CMD_ENABLE_KEYBOARD			0xAE


// Mouse specific commands

#define i8042_SET_MOUSE_SCALING_1to1	0xE6
#define i8042_SET_MOUSE_SCALING_1to2	0xE7
#define i8042_SET_MOUSE_RESOLUTION		0xE8
#define i8042_GET_MOUSE_INFORMATION		0xE9
#define i8042_SET_MOUSE_STREAMING		0xEA
#define i8042_GET_MOUSE_DATA			0xEB
#define i8042_RESET_MOUSE_WRAP_MODE		0xEC
#define i8042_SET_MOUSE_WRAP_MODE		0xEE
#define i8042_SET_REMOTE_MODE			0xF0
#define i8042_READ_MOUSE_ID				0xF2
#define i8042_SET_MOUSE_SAMPLE_RATE		0xF3
#define i8042_ENABLE_MOUSE				0xF4
#define i8042_DISABLE_MOUSE				0xF5
#define i8042_RESET_TO_DEFAULTS			0xF6
#define i8042_RESEND_LAST_MOUSE_DATA	0xFE
#define i8042_RESET_MOUSE				0xFF

// Keyboard specific commands

#define i8042_SET_MODE_INDICATORS		0xED
#define i8042_ECHO						0xEE
#define i8042_NOP						0xEF
#define i8042_SCANCODE_SET				0xF0
#define i8042_READ_KEYBOARD_ID			0xF2
#define i8042_SET_TYPEMATIC_RATE		0xF3
#define i8042_ENABLE_KEYBOARD			0xF4
#define i8042_DISABLE_DEFAULT_KEYBOARD	0xF5
#define i8042_SET_DEFAULT_PARAMETERS	0xF6
#define i8042_RESEND_LAST_SCANCODE		0xFE
#define i8042_INTERNAL_POWER_RESET		0xFF


#define i8042_ACK			0xFA
#define i8042_RESEND		0xFE

#define i8042_AUX_DATA		0x20

// find wheel
unsigned char ps2_init_string[] = { 0xf3, 0xc8, 0xf3, 0x64, 0xf3, 0x50, 0x00 };
// find buttons
unsigned char ps2_init_string2[] = { 0xf3, 0xc8, 0xf3, 0xc8, 0xf3, 0x50, 0x00 };
// real init
unsigned char ps2_init_string3[] = { 0xf6, 0xe6, 0xf4, 0xf3, 0x64, 0xe8, 0x03, 0x00 };

// mouse type
unsigned char extwheel = 0xff, extbuttons = 0xff;




/** Flush data from keyboard controller */
static int flush_buffer()
{
	int count = 0;
	while( (inb(0x64) & 0x1) != 0 ) 
	{
		inb(0x60);
		count += 1;
	}
	return count;
}


/** Will return -1, MOUSE_DATA or KEYBOARD_DATA depending
 * on what kind of data is there, if any */
int data_waiting()
{
	unsigned int timeout = 5000;

	while ( timeout-- > 0 )
	{
		unsigned char ch = inb(0x64);
		if ( (ch & 0x1) == 0 ) continue;

		if ( (ch & i8042_AUX_DATA) != 0 ) return MOUSE_DATA;
		return KEYBOARD_DATA;
	}

	return -1;
}

/** Read input from the keyboard data port */
unsigned char read_data()
{
	while( (inb(0x64) & 0x1) == 0 ) continue;
	return inb(0x60);
}

void write_data( unsigned char data )
{
	while( (inb(0x64) & 0x2) != 0 ) continue;
	outb(0x60, data );
}

/** Send a command to the keyboard controller port */
void send_ctrl( unsigned char cmd )
{
	while( (inb(0x64) & 0x2) != 0 ) continue;
	outb(0x64, cmd );
}

/** Send a command to the data port, returns ACK */
int send_command( unsigned char cmd )
{
	write_data( cmd );
	return read_data();
}

/** Sends a command to the aux data port, returns ACK */
int send_aux( unsigned char cmd )
{
	while( (inb(0x64) & 0x2) != 0 ) continue;
	outb(0x64, 0xD4 );
	return send_command( cmd );
}

/** Set the LEDS to whatever */
void set_leds( unsigned char bits )
{
	write_data( 0xED );
	write_data( bits );
}






void init_8042()
{
	unsigned char *ch;
	int badFlushes = 0;

	smk_disable_interrupts();

		send_ctrl( 0xAD );	// disable keyboard

		flush_buffer();			// Empty all data.

		/*
		send_aux( 0xFF );		// Reset aux
		send_command( 0xFF );	// Reset keyboard
		send_command( 0xF6 );	// Set defaults
		*/

		/*
		set_leds( 0 );			// Reset LEDS
		write_data( 0xF3 );		// Set typematic rate to fast
		write_data( 0x0 );		// Everything is max.
		read_data();			// Receive the ACK back
		send_command( 0xF4 );	// Enable keyboard
		*/

		send_ctrl( 0x60 );
		write_data( 0x47 );	// Enable IRQ1, IRQ2 and PC compatibility mode

		send_ctrl( 0xA8 );		// Enable aux
		send_ctrl( 0xA9 );
		read_data();			// Read response

		send_aux( 0xF2 );
		read_data();

		for (ch = ps2_init_string; *ch != '\0'; ch++) 
		{
			send_aux( *ch );
		}
		
		send_aux( 0xF2 );
		extwheel = read_data();

		for (ch = ps2_init_string2; *ch != '\0'; ch++) 
		{
			send_aux( *ch );
		}
		
		send_aux( 0xF2 );
		extbuttons = read_data();

		for (ch = ps2_init_string3; *ch != '\0'; ch++) {
			send_aux( *ch );
		 }

		send_aux( 0xF4 );

		send_ctrl( 0xAE );	// Enable keyboard

		badFlushes += flush_buffer();
		

	smk_enable_interrupts();


	if ( badFlushes != 0 )
		devfs_dmesg("WARNING: 8042 had extraneous data on the chip. "
		            "Initialization problem. Good luck.\n" );

}


