
extern char __bss_start;
extern char __bss_end;


void zero_bss()
{
  char *bss;
	  
  for ( bss = &__bss_start; bss < &__bss_end; bss++ )
					*bss = 0;
}

void *memset(void *dest, int c, int n)
{
  char *new_dest = dest;
  int i = 0;
  
  while (i++ != n )
  {
    *new_dest = c;
     new_dest++;
  }

  return dest;
}


int strlen(const unsigned char *src)
{
	int i = 0;

	while ( src[i++] != 0 ) continue;

	return i;
}

int strnlen(const unsigned char *src, unsigned int n)
{
	int i = 0;

	while ( i < n )
	{
		if ( src[i] == 0 ) return i;
		i++;
	}

	return n;
}

unsigned char *strncpy(unsigned char *s1, const unsigned char *s2, unsigned int n)
{
  while ( n-- > 0 )
  {
	*s1++ = *s2;
	if ( *s2 == 0 ) break;
	s2++;
  }

  s1[n-1] = 0;
  return s1;
}

int strncmp( const unsigned char* a, const unsigned char* b, unsigned int n)
{
  unsigned char *src;
  unsigned char *dst;
  int count = 0;
  int boom = 0;
  
  src = (unsigned char*)a;
  dst = (unsigned char*)b;

  while ((*src != 0) && (*dst != 0))
  {
    if ( *src != *dst ) count++;
    if ((*src == 0) ||  (*dst == 0))  break;
    src++;
    dst++; 
    boom ++;
    if (boom == n) return count;
  }

  if ( *src != *dst ) count++;

 
  return count;
}


int 	memcmp(const void* s1, const void* s2, unsigned int n)
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


#include "include/dmesg.h"
#include "include/atomic.h"

void* 	memcpy(void* s1, const void* s2, unsigned int n)
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
  
  while ( n-- > 0 )
      *cdest++ = *csrc++;
  
  return s1;
}



