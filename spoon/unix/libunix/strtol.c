//
// strtol.c
//
// String to number conversion
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

#define FL_UNSIGNED   1
#define FL_NEG        2
#define FL_OVERFLOW   4
#define FL_READDIGIT  8

static unsigned long strtoxl(const char *nptr, char **endptr, int ibase, int flags)
{
  const char *p;
  char c;
  unsigned long number;
  unsigned digval;
  unsigned long maxval;

  p = nptr;
  number = 0;

  c = *p++;
  while (isspace((int)(unsigned char) c)) c = *p++;

  if (c == '-') 
  {
    flags |= FL_NEG;
    c = *p++;
  }
  else if (c == '+')
    c = *p++;

  if (ibase < 0 || ibase == 1 || ibase > 36) 
  {
    if (endptr) *endptr = (char *) nptr;
    return 0L;
  }
  else if (ibase == 0)
  {
    if (c != '0')
      ibase = 10;
    else if (*p == 'x' || *p == 'X')
      ibase = 16;
    else
      ibase = 8;
  }

  if (ibase == 16)
  {
    if (c == '0' && (*p == 'x' || *p == 'X')) 
    {
      ++p;
      c = *p++;
    }
  }

  maxval = ULONG_MAX / ibase;

  for (;;) 
  {
    if (isdigit((int) (unsigned char) c))
      digval = c - '0';
    else if (isalpha((int) (unsigned char) c))
      digval = toupper(c) - 'A' + 10;
    else
      break;

    if (digval >= (unsigned) ibase) break;

    flags |= FL_READDIGIT;

    if (number < maxval || (number == maxval && (unsigned long) digval <= ULONG_MAX % ibase)) 
      number = number * ibase + digval;
    else 
      flags |= FL_OVERFLOW;

    c = *p++;
  }

  --p;

  if (!(flags & FL_READDIGIT)) 
  {
    if (endptr) p = nptr;
    number = 0L;
  }
  else if ((flags & FL_OVERFLOW) || (!(flags & FL_UNSIGNED) && (((flags & FL_NEG) && (number < LONG_MIN)) || (!(flags & FL_NEG) && (number > LONG_MAX)))))
  {
    errno = ERANGE;

    if (flags & FL_UNSIGNED)
      number = ULONG_MAX;
    else if (flags & FL_NEG)
      number = LONG_MIN;
    else
      number = LONG_MAX;
  }

  if (endptr != NULL) *endptr = (char *) p;

  if (flags & FL_NEG) number = (unsigned long) (-(long) number);

  return number;
}

long strtol(const char *nptr, char **endptr, int ibase)
{
  return (long) strtoxl(nptr, endptr, ibase, 0);
}

unsigned long strtoul(const char *nptr, char **endptr, int ibase)
{
  return strtoxl(nptr, endptr, ibase, FL_UNSIGNED);
}



long atol(const char *nptr)
{
  int c;
  long total;
  int sign;

  while (isspace((int)(unsigned char) *nptr)) ++nptr;

  c = (int)(unsigned char) *nptr++;
  sign = c;
  if (c == '-' || c == '+') c = (int)(unsigned char) *nptr++;

  total = 0;
  while (isdigit(c)) 
  {
    total = 10 * total + (c - '0');
    c = (int)(unsigned char) *nptr++;
  }

  if (sign == '-')
    return -total;
  else
    return total;
}

int atoi(const char *nptr)
{
  return (int) atol(nptr);
}
