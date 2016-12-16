#include <smk.h>
#include <devfs.h>

#ifndef _HAVE_VA_LIST
#define _HAVE_VA_LIST
typedef struct 
{
	void **start;
	void **ptr;
} va_list;
#endif


#define		va_start( ap, lastarg )					\
			ap.ptr = (void**)( (unsigned int)&lastarg + sizeof(const char*) );	\
			ap.start = ap.ptr;	


#define		va_arg( ap, type) \
			((type)*(ap.ptr)); \
			ap.ptr = (void**)( ((unsigned int)ap.ptr) + sizeof( type ) );
		
#define		va_end( ap )	\
			ap.ptr = NULL;



// Forward declarations 
static int dmesg_int( char *buffer, int len,  int num );
static int dmesg_uint( char *buffer, int len, unsigned int num );
static int dmesg_hex( char *buffer, int len, unsigned int hex );
static int dmesg_bin( char *buffer, int len,  unsigned int num );
static int dmesg_string( char *buffer, int len, const char *str );



/** Prints an integer into the dmesg_buffer */
static int dmesg_int( char *buffer, int len,  int num )
{
	int val;
		
	if ( num >= 0 )
	{
		return dmesg_uint( buffer, len, num );
	}


	if ( dmesg_string( buffer, len, "-" ) == len ) return len;

	val = dmesg_uint( buffer + 1, len - 1, -num );
	if ( val >= len - 1) return val;
	return val + 1;
}

/** Prints an unsigned integer into the dmesg_buffer */

static int dmesg_uint( char *buffer, int len, unsigned int num )
{
	char number[32];
	int i 	= 0;
	int tmp = num;
	int count = 0;
		
	
	while ( 1==1 )
	{
		number[i++] = (tmp % 10) + '0';
		if ( tmp == 0 ) break;
		tmp = tmp / 10;
		if ( tmp == 0 ) break;
	}
	
	while ( i-- > 0 )
	{
		if ( count >= len ) return len;
		buffer[ count++ ] = number[i];
	}
	
	return count;
}

/** Prints an unsigned hexadecimal number into the dmesg_buffer */
static int dmesg_hex( char *buffer, int len, unsigned int hex )
{
	char number[64];
	int i 	= 0;
	unsigned int tmp = hex;
	char letter;
	int count = 0;
		

	while ( 1==1 )
	{
		letter = tmp % 16;

		if ( letter < 10 ) letter = letter + '0';
					else   letter = letter - 10 + 'A';
		
		number[i++] = letter;

		if ( tmp == 0 ) break;
		tmp = tmp / 16;
		if ( tmp == 0 ) break;
	}
	
	number[i++] = 'x';
	number[i++] = '0';
	
	while ( i-- > 0 )
	{
		if ( count >= len ) return len;
		buffer[ count++ ] = number[i];
	}
	
	return count;
}


/** Prints a binary number into the dmesg_buffer */
static int dmesg_bin( char *buffer, int len, unsigned int num )
{
	char number[128];
	unsigned int tmp = num;
	int i = 0;
	int count = 0;


	number[i++] = 'b';

	while ( tmp > 0 )
	{
		if ( (tmp & 1) == 1 ) number[i++] = '1';
			 			 else number[i++] = '0';

		if ( tmp == 0 ) break;

		if ( (i % 9) == 0 ) 
			number[i++] = ' ';

		tmp = tmp >> 1;
	}
	
	
	while ( i-- > 0 )
	{
		if ( count >= len ) return len;
		buffer[ count++ ] = number[i];
	}
	
	return count;
}


/** Prints a string into the dmesg_buffer */
static int dmesg_string( char *buffer, int len, const char *str )
{
	char *s = (char*)str;
	int count = 0;

	if ( str == NULL ) return dmesg_string( buffer, len, "NULL" );	// Cool.
		
	while ( *s != 0 )
	{
		if ( count >= len ) return len;
		buffer[ count++ ] = *s++;
	}
	
	return count;
}


//--------------


/** sdmesg is a printf-styled function that accepts multiple modifiers
 * and instructions.
 *
 * It currently supports 5 modifiers:
 *
 *   %! - print to screen.
 *   %s - string argument.
 *   %i - signed integer.
 *   %u - unsigned integer.
 *   %x - hexadecimal unsigned number.
 * 
 * So you see that it's a very basic printf function.
 *
 * \return 0
 */
static int sdmesg( char *buffer, int max_len, char *format, va_list ap  )
{
   char *tmpBuf;
   int position = 0;
   int len;

   int modifier = 0;
   char msg[2];
  		msg[1] = 0; 
   
	// pointers & modified data.
	char *string;
	int  number;
	unsigned int unsigned_number;
	
	// ----------------------
	
   while (*format)
   {
	 char letter = *format;
	      format++;

	 len = max_len - position;
	 tmpBuf = buffer + position;
		   
	 if ( modifier != 0 )
	 {
      	switch( letter ) 
		{
        	case 's': 
					string = va_arg( ap, char* );
					position += dmesg_string( tmpBuf, len, string );  
					break;

       	    case 'i': 
					number = va_arg( ap, int );
					position += dmesg_int( tmpBuf, len, number ); 
					break;

       	    case 'u': 
					unsigned_number = va_arg( ap, unsigned int );
					position += dmesg_uint( tmpBuf, len, unsigned_number ); 
					break;

       	    case 'b': 
					unsigned_number = va_arg( ap, unsigned int );
					position += dmesg_bin( tmpBuf, len, unsigned_number ); 
					break;

        	case 'x': 
					unsigned_number = va_arg( ap, unsigned int );
					position += dmesg_hex( tmpBuf, len, unsigned_number ); 
					break;

	  	}

	 	modifier = 0;
  	 }
	 else
	 {
		switch( letter )
		{
			case '%':
					modifier = 1;
					break;
				
			default:
				msg[0] = letter;
				position += dmesg_string( tmpBuf, len, msg );
				break;
		}
	 }
   }

   if ( position < max_len ) buffer[ position ] = 0;
   
   return position;
}


/** dmesg thunks downn to sdmesg and then transfers the 
 * temporary buffer into the kernel's main buffer.
 */

int devfs_dmesg( char *format, ... )
{
	va_list ap;
	char buffer[ 1024 ];
	int rc;
	int i;

	static int x = -666;
	static int y = -999;
	static char *dmesg_screen = NULL;

	if ( (x == -666) && (y == -999) )
	{
		// Do your evil thing.
		x = 0;
		y = (smk_getpid() * 5) % 25;
	}
	
	va_start( ap, format );

		rc = sdmesg( buffer, 1024, format, ap );
	
	va_end( ap );

	// Copy into the buffer.

	if ( dmesg_screen == NULL )
	{
		dmesg_screen = (char*)smk_mem_map( (void*)0xb8000, 1 );
		if ( dmesg_screen == NULL ) return -1;
	}

	i = 0;
	while ( buffer[i] != 0 )
	{
		switch (buffer[i])
		{
			case '\r':
			case '\n':
				x = 0;
				if ( ++y >= 25 ) y = 0;
				break;
				
			default:
				dmesg_screen[ x * 2 + y * 160 ] = buffer[i];
				if ( ++x >= 80 ) 
				{
					x = 0;
					if ( ++y >= 25 ) y = 0;
				}

		}

		i += 1;
	}
	
	
	return rc;
}

 





