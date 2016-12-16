#ifndef _8042_H
#define _8042_H


void init_8042();

void 	write_data( unsigned char data );
void 	send_ctrl( unsigned char cmd );
int 	send_command( unsigned char cmd );
int	 	send_aux( unsigned char cmd );
void 	set_leds( unsigned char bits );
unsigned char read_data();


#define KEYBOARD_DATA	1
#define MOUSE_DATA		2

int data_waiting();


extern unsigned char extwheel;
extern unsigned char extbuttons;

#endif

