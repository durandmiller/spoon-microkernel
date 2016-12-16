#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

 

void    *memchr(const void *s, int c, size_t n)
{
	unsigned char *tmp = (unsigned char*)s;
	int i = 0;

	while ( i < n )
	{
		if ( tmp[i] == c ) return (tmp+i);
		i++;
	}

	return NULL;
}


void    *memrchr(const void *s, int c, size_t n)
{
	unsigned char *tmp = (unsigned char*)s;

	while ( n > 0 )
	{
		if ( tmp[n-1] == c ) return (tmp+n-1);
		n--;
	}

	return NULL;
}



void* 	memcpy(void* /* restrict */s1, const void* /* restrict */s2, size_t n)
{
  char *cdest;
  char *csrc;
  uint32_t *ldest = (uint32_t*)s1;
  uint32_t *lsrc  = (uint32_t*)s2;

  while ( n >= 4 )
  {
      *ldest++ = *lsrc++;
	  n -= 4;
  }

  cdest = (char*)ldest;
  csrc  = (char*)lsrc;
  
  while ( n > 0 )
  {
      *cdest++ = *csrc++;
	  n -= 1;
  }
  
  return s1;
}

 

void *mempcpy(void *dest, const void *src, size_t n)
{
	memcpy( dest, src, n );
	return (dest + n);
}
 

void* 	memmove(void* s1, const void* s2, size_t n)
{
	char *dest,*src;
	int i;

	dest = (char*)s1;
	src = (char*)s2;
	for ( i = 0; i < n; i++)
		dest[i] = src[i];
	
	return s1;
}






char* 	strcpy(char* /* restrict */s1, const char* /* restrict */s2)
{
	int i = 0;
	do { s1[i] = s2[i]; } 
		while (s2[i++] != '\0');
	return s1;
}

char*	strncpy(char* /* restrict */s1, const char* /* restrict */s2, size_t n)
{
	int i;

	for ( i = 0; i < n; i++ )
	{
		s1[i] = s2[i]; 
		if ( s2[i] == '\0' ) break;
	}
	
	while ( i < n ) s1[i++] = '\0'; 
	   
	return s1;
}

char* 	strcat(char* /* restrict */s1, const char* /* restrict */s2)
{
	char *tmp = s1;
	while(*s1 != '\0') s1++;
	while((*s1++ = *s2++) != '\0');
	return tmp;
}


char* 	strncat(char* /* restrict */s1, const char* /* restrict */s2, size_t n)
{
	char *tmp = s1;

	if(n > 0) 
	{
		while(*s1) s1++;
		while((*s1++ = *s2++)) 
		{
			if (--n == 0) 
			{
				*s1 = '\0';
				break;
			}
		}
	}

	return tmp;
}



int 	memcmp(const void* s1, const void* s2, size_t n)
{
	char *s,*t;
	int i;

	s = (char*)s1;
	t = (char*)s2;
	for ( i = 0; i < n; i++)
	{
		if ( s[i] < t[i] ) return -1;
		if ( s[i] > t[i] ) return  1;
	}
	return 0;
}





int 	strcmp(const char* s1, const char* s2)
{
	char c;

	while(1) 
	{
		if((c = *s1 - *s2++) != 0 || !*s1++)
			break;
	}

	return c;
}



int 	strncmp(const char* s1, const char* s2, size_t n)
{
	char c = 0;

	while(n > 0) 
	{
		if ((c = *s1 - *s2++) != 0 || !*s1++)
			break;
		n--;
	}

	return c;
}


char* 	strchr(const char* s, int c)
{
	int i;
	for ( i = 0; i < strlen(s); i++ )
		if ( s[i] == c ) return (char*)(s + i);
	return NULL;
}



size_t   strcspn(const char *s1, const char *s2)
{
	int len1;
	int len2;
	int i;
	int j;
	char c;

	len1 = strlen( s1 );
	len2 = strlen( s2 );

	for ( i = 0; i < len1; i++ )
	{
		c = s1[i];

		for ( j = 0; j < len2; j++ )
			if ( s2[j] == c ) return i;
	}
		   	
	return len1;
}



char* strpbrk(const char *s1, const char *s2)
{
	int len1;
	int len2;
	int i;
	int j;
	char c;

	len1 = strlen( s1 );
	len2 = strlen( s2 );

	for ( i = 0; i < len1; i++ )
	{
		c = s1[i];

		for ( j = 0; j < len2; j++ )
			if ( s2[j] == c ) return (char*)(s1 + i);
	}
		   	
	return NULL;
}


char* 	strrchr(const char* s, int c)
{
	int i;
	for ( i = strlen(s) - 1; i >= 0; i-- )
		if ( s[i] == c ) return (char*)(s + i);
	return NULL;
}


size_t   strspn(const char *s1, const char *s2)
{
	int len1;
	int len2;
	int i;
	int j;
	char c;

	len1 = strlen( s1 );
	len2 = strlen( s2 );

	for ( i = 0; i < len1; i++ )
	{
		c = s1[i];

		for ( j = 0; j < len2; j++ )
			if ( s2[j] != c ) return i;
	}
		   	
	return len1;
}




char    *strstr(const char *s1, const char *s2)
{
	int len1;
	int len2;
	int i;

	len2 = strlen( s2 );
	if ( len2 == 0 ) return (char*)s1;
	len1 = strlen( s1 );

	for ( i = 0; i < len1; i++ )
	{
#warning should this not be strncmp ?
		if ( strcmp( s1 + i, s2 ) == 0 ) return (char*)(s1 + i);
	}
		   	
	return NULL;
}








void* 	memset(void* s, int c, size_t n)
{
	int i;
	for ( i = 0; i < n ; i++)
		((char*)s)[i] = c;
	
	return s;
}


char *stpcpy(char *dest, const char *src)
{
	strcpy( dest, src );
	return (dest + strlen(dest));
}



char    *strerror(int errnum)
{
	static char temp[512];


	switch(errnum)
	{
		case EDOM:			return "domain error";
		case ERANGE:		return "range error";
	}


	snprintf( temp, 512, "unknown error number (%i)", errnum );
	return temp;
}



size_t 	strlen(const char* s)
{
	size_t ans = 0;
	while ( s[ans++] != '\0' ) {};
	return (ans-1);
}


size_t   strnlen(const char *s, size_t maxlen)
{
	size_t ans = 0;
	while ( (ans++ < maxlen) && (s[ans-1] != '\0') ) 
				continue;
	return ans;
}


/** Taken from Sanos source. See ADDITIONAL_LICENCES */
char *strtok_r(char *string, const char *control, char **lasts)
{
	unsigned char *str;
	const unsigned char *ctrl = (unsigned char*)control;

	unsigned char map[32];
	int count;
	
	//Clear control map
	for (count = 0; count < 32; count++) map[count] = 0;

	// Set bits in delimiter table
	do { map[*ctrl >> 3] |= (1 << (*ctrl & 7)); } while (*ctrl++);

	// Initialize str. If string is NULL, set str to the saved
	// pointer (i.e., continue breaking tokens out of the string
	// from the last strtok call)
	if (string)
		str = (unsigned char*)string;
	else
		str = (unsigned char*)*lasts;
	
	// Find beginning of token (skip over leading delimiters). Note that
	// there is no token iff this loop sets str to point to the terminal
	// null (*str == '\0')
	
	while ((map[*str >> 3] & (1 << (*str & 7))) && *str) str++;
	
	string = (char*)str;
	
	// Find the end of the token. If it is not the end of the string,
	// put a null there
	for ( ; *str ; str++)
	{
		if (map[*str >> 3] & (1 << (*str & 7)))
		{
			*str++ = '\0';
			break;
		}
	}

	// Update nexttoken
	*lasts = (char*)str;


	// Determine if a token has been found.
	if ((unsigned char*)string == str) return NULL;

	return string;
}



char *strtok(char *string, const char *control)
{
	char *cookie;
	return strtok_r(string, control, &cookie);
}






/// NOT PART OF THE STANDARD
char*	strndup(const char *s, size_t n)
{
	char *result;
	result = (char*)malloc( n + 1 );
	memcpy( result, s, n + 1 );
	result[n] = 0;
	return result;
}

/// NOT PART OF THE STANDARD
char*	strdup(const char *s)
{
	char *result;
	int len = strlen(s);
	result = (char*)malloc( len + 1 );
	memcpy( result, s, len + 1 );
	return result;
}




