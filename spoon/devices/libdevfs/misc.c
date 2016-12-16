#include <devfs.h>
#include <smk.h>


int devfs_strlen( const char *str )
{
	int i = 0;
	while ( str[i] != '\0' )  i++ ;
	return i;
}


char* 	devfs_strcat(char* s1, const char* s2)
{
	int len = devfs_strlen(s1);
	devfs_strcpy( s1 + len,  s2 );
	return s1;
}

char* 	devfs_strcpy(char* s1, const char* s2)
{
	int i = 0;
	do { s1[i] = s2[i]; } 
		while (s2[i++] != '\0');
	return s1;
}

int     devfs_strcmp(const char *s1, const char *s2)
{
	char c;

	while(1) 
	{
		if((c = *s1 - *s2++) != 0 || !*s1++)
			break;
	}

	return c;
}

int     devfs_strncmp(const char *s1, const char *s2, int n)
{
	char c = 0;

	while(n > 0) 
	{
		n -= 1;

		if((c = *s1 - *s2++) != 0 || !*s1++)
			break;
	}

	if ( c != 0 ) return c;
	if ( n != 0 ) return -1;
	return 0;
}

char*	devfs_strdup(const char *s)
{
	char *result;
	int len = devfs_strlen(s);
	result = (char*)smk_malloc( len + 1 );
	devfs_memcpy( result, s, len + 1 );
	return result;
}


char*	devfs_strndup(const char *s, int n)
{
	char *result;
	int len = devfs_strlen(s);
	if ( len > n ) len = n;
	
	result = (char*)smk_malloc( len + 1 );
	devfs_memcpy( result, s, len );
	result[len] = 0;
	return result;
}


int     devfs_memcmp(const char *s1, const char *s2, int len)
{
	char c = 0;

	while( len-- > 0) 
	{
		if((c = *s1 - *s2++) != 0 || !*s1++)
			break;
	}

	return c;
}

void* 	devfs_memcpy(void* s1, const void* s2, unsigned int n)
{
  char *cdest;
  char *csrc;
  unsigned int *ldest = (unsigned int*)s1;
  unsigned int *lsrc  = (unsigned int*)s2;

  while ( n >= sizeof(unsigned int) )
  {
      *ldest++ = *lsrc++;
	  n -= sizeof(unsigned int);
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


void* 	devfs_memset(void* s, int c, unsigned int n)
{
	int i;
	for ( i = 0; i < n ; i++)
		((char*)s)[i] = c;
	
	return s;
}




/** Helper function for devfs_split */
static int split_match( char c, const char *tokens )
{
	int i = 0;
	while ( tokens[i] != 0 )
	{
		if ( tokens[i] == c ) return 0;
		i++;
	}

	return -1;
}

/** Takes the string str and splits it based on the tokens given. The
 * array is filled according to the pointers of the new strings. The
 * array must be guaranteed to accomodate max amount of strings.
 *
 * \warning array must be allocated by you and it must be good for max
 *          strings.
 *
 * \warning Free the entries yourself once you're done with them.
 *
 * \return The number of non-empty strings.
 */
int		devfs_split( char *str, const char *tokens, char **array, int max )
{
	int count 		= 0;
	int pos 		= 0;
	int last_pos 	= 0;
	int len 		= devfs_strlen( str );

	if ( max <= 0 ) return 0;

	for ( pos = 0; pos < len; pos++ )
	{
		if ( split_match( str[pos], tokens ) != 0 ) continue;

		if ( (pos - last_pos) == 0 )
		{
			last_pos = pos + 1;
			continue;
		}


		array[ count++ ] = devfs_strndup( str + last_pos, (pos - last_pos ) ); 
		last_pos = pos + 1;

		if ( count == max ) break;
	}

	// The left-overs
	if ( count != max )
	{
		if ( (pos - last_pos) != 0 )
			array[ count++ ] = devfs_strndup( str + last_pos, (pos - last_pos ) ); 
	}

	
	return count;
}




