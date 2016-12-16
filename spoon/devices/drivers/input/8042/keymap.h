#ifndef _KEYMAP_H
#define _KEYMAP_H

#define CTRL	29
#define ALT	56
#define LSHIFT  42
#define RSHIFT  54
#define SCRLOCK 70
#define NUMLOCK 69
#define CAPS    58
#define SYSREQ  55
#define F11     87
#define F12     88

#define EXTENDED	224

#define LED_SCROLL 1
#define LED_NUM    2
#define LED_CAPS   4


#define CONTROL_KEY 		(1<<3) 


#define OPTION_KEY  		(1<<6) 
#define SHIFT_KEY		(1<<0)
#define SHIFTED	((modifiers & SHIFT_KEY) != 0) || ((leds & LED_CAPS) != 0) 



char unshifted_keymap[128] = {
	   0,  27,  '1', '2',  '3', 
	 '4', '5',  '6', '7',  '8', 
	 '9', '0',  '-', '=', '\b', 
    '\t', 'q',  'w', 'e',  'r',  
	 't', 'y',  'u', 'i',  'o',  
	 'p', '[',  ']','\n',    0,  
	 'a', 's',  'd', 'f',  'g',
	 'h', 'j',  'k', 'l',  ';', 
	 '\'', '`',   0,   0,  'z',  
	 'x', 'c',  'v', 'b',  'n', 
	 'm', ',',  '.', '/',    0, 
	  '*',    0,  ' ',    0,   59,
       60,   61,   62,   63,   64,
	   65,   66,   67,   68,    0,
	    0,    0,   72,    0,    0, 
	   75,    0,   77,    0,    0,
       80,    0,    0,    0,    0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0
};

char shifted_keymap[128] = {
        0,   27,  '!',  '@',  '#', 
	  '$',  '%',  '^',  '&',  '*', 
	  '(',  ')',  '_',  '+', '\b', 
	 '\t',  'Q',  'W',  'E',  'R',  
	  'T',  'Y',  'U',  'I',  'O', 
	  'P',  '{',  '}', '\n',    0, 
	  'A',  'S',  'D',  'F',  'G', 
	  'H',  'J',  'K',  'L',  ':', 
	  '"',  '~',    0,    0,  'Z', 
	  'X',  'C',  'V',  'B',  'N', 
	  'M',  '<',  '>',  '?',    0, 
	  '*',    0,  ' ',    0,   59,
       60,   61,   62,   63,   64,
	   65,   66,   67,   68,    0,
	    0,    0,   72,    0,    0, 
	   75,    0,   77,    0,    0,
       80,    0,    0,    0,    0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0
};

#endif

