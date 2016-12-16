#include <smk.h>
#include <devfs.h>


#include "8042.h"
#include "devices.h"
#include "events.h"

#include "keymap.h"


#define EXTENDED	224


int kbd_irq()
{
	static int extended = 0;
	static unsigned int modifiers = 0;
	static int leds = 0;

	unsigned char scancode = read_data();
	unsigned char ascii = 0;
	int rc = 0;

	if ( scancode == EXTENDED )
	{
		extended = 1;
		return 0;
	}

	extended = 0;
	
	if ( (scancode & 0x80) != 0 ) 
	{
		scancode = scancode & 0x7f;
		 
	   	 switch (scancode)
		 {
		 	case CTRL: modifiers &= ~ CONTROL_KEY; break;
		 	case ALT:  modifiers &= ~ OPTION_KEY; break;
			case LSHIFT:
			case RSHIFT:  
			     modifiers &= ~ SHIFT_KEY;
				 break;

			default: 
				 if ( SHIFTED ) ascii = shifted_keymap[scancode];
				 		   else ascii = unshifted_keymap[scancode];

				 rc = add_event( KEYBOARD_DATA, scancode, ascii, modifiers, 0 );
				 break;
		 }
	
	}
	else
	{
		scancode = scancode & 0x7f;

	    switch(scancode)
	    {
    	    case CTRL:    modifiers |= CONTROL_KEY; break;
    	    case ALT:     modifiers |= OPTION_KEY; break;
		    case LSHIFT:
		    case RSHIFT:
		    	 modifiers |= SHIFT_KEY;
				 break;

		    case CAPS:
				 if ( leds & LED_CAPS ) leds &= ~LED_CAPS;
				 				   else leds |= LED_CAPS;
				 set_leds( leds );
				 break;

		    case SCRLOCK:
				 if ( leds & LED_SCROLL ) leds &= ~LED_SCROLL;
				 					 else leds |= LED_SCROLL;
				 set_leds( leds );
				 break;
		    case NUMLOCK:
				 if ( leds & LED_NUM ) leds &= ~LED_NUM;
				 				  else leds |= LED_NUM;
				 set_leds( leds );
				 break;
		    default:
				 if ( SHIFTED ) ascii = shifted_keymap[scancode];
				 		   else ascii = unshifted_keymap[scancode];


				 rc = add_event( KEYBOARD_DATA, scancode, ascii, modifiers, 1 );
				 break;
	    
	  	}
	
	}



	return rc;
}





int mouse_irq()
{
	static unsigned char buf[4] = {0, 0, 0, 0};
	static int bytes = 0;
	static int mx = 0;
	static int my = 0;
	static int mz = 0;
	static unsigned int buttons[5] = {0, 0, 0, 0, 0};
	static unsigned int prev_buttons = 0;
	int dx, dy, dz;
	int i;
	int rc;
	
	
	int needed = 0;
	if ((extwheel == 0x03) || (extbuttons == 0x04)) needed = 1;

	buf[ bytes++ ] = read_data();

	if ( bytes < (3 + needed) ) return 0;

	// We have a full set of data

 	 bytes = 0;

	
	 buttons[0] = buf[0] & 1;
	 buttons[1] = (buf[0] & 2) >> 1;
	 buttons[2] = (buf[0] & 4) >> 2;
	 buttons[3] = (buf[3] & 0x10) >> 4;
	 buttons[4] = (buf[3] & 0x20) >> 5;
	 dx = (buf[0] & 0x10) ? buf[1] - 256 : buf[1];
	 dy = (buf[0] & 0x20) ? -(buf[2] - 256) : -buf[2];
	 dz = (buf[3] & 0x08) ? (buf[3]&7) - 8 : buf[3]&7;
	
	 if (dx > 5 || dx < -5)
		dx *= 3;
	 if (dy > 5 || dy < -5)
		dy *= 3;
	
	 mx += dx;
	 my += dy;
	 mz += dz;
	
	 if (mx > 1023) mx = 1023;
	 if (mx < 0) mx = 0;
	 if (my > 767) my = 767;
	 if (my < 0) my = 0;
	

 	// buttons! Switch them around so that left = bit 0, right = bit 1.
	unsigned int butts = 0;				// see more?
	for (i = 0; i < 5; i++ )
		butts = butts + (buttons[i] << i);

 	if ( ( dx != 0 ) || ( dy != 0 ) || ( dz != 0 ) )
	{
		rc = add_event( MOUSE_DATA, 0, mx, my, butts ); 
	}
 
// ---------  MOUSE BUTTON DIFFERENCES -----------------------
       unsigned int tb = (butts ^ prev_buttons);

       if ( tb != 0 )
       {
	  	  if ( butts != 0 ) 
			rc = add_event( MOUSE_DATA, 1, mx, my, butts ); 
		  else 
			rc = add_event( MOUSE_DATA, 2, mx, my, butts ); 
		}
// -----------------------------------------------------------

	prev_buttons = butts;

	return rc;
}


int myirq( int irq, void *data )
{
	int msg;
	int count = 0;
	int loopOver = 1;


	smk_disable_interrupts();

		send_ctrl( 0xAD );	// disable

		while ( loopOver == 1 )	// Event buffer full?
		{
			loopOver = 0;
		
			msg = data_waiting();
			while ( (msg != -1) && (loopOver == 0) )
			{
				count += 1;
			
				if ( msg == MOUSE_DATA )
					loopOver = mouse_irq();
				else
					loopOver = kbd_irq();
	
				msg = data_waiting();

				// Quick hack
				if ( (count % 2) == 0 ) loopOver = 1;
			}

			if ( loopOver == 1 )
			{
				smk_enable_interrupts();
				process_events();
				smk_disable_interrupts();
			}
		}

	smk_enable_interrupts();
	process_events();
	smk_ack_irq( irq );	
		
	smk_disable_interrupts();

		send_ctrl( 0xAE );	// enable

	smk_enable_interrupts();

	return IRQ_HANDLED;
}



// -----------------------------




int main( int argc, char *argv[] )
{
	int rc;
	
	if ( smk_request_iopriv() != 0 ) return -1;

	rc = smk_request_irq( myirq, 1, "kbd_handler", (void*)0 );
	if ( rc < 0 ) return -1;

	rc = smk_request_irq( myirq, 12, "ps2mouse_handler", (void*)0 );
	if ( rc < 0 ) return -1;

	init_8042();

	init_devices();
   
	smk_go_dormant();

    return 0;
}

