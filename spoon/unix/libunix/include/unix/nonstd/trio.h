/*************************************************************************
 *
 * $Id: trio.h,v 1.17 2005/05/29 11:58:14 breese Exp $
 *
 * Copyright (C) 1998 Bjorn Reese and Daniel Stenberg.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 *************************************************************************
 *
 * http://ctrio.sourceforge.net/
 *
 ************************************************************************/

#ifndef TRIO_TRIO_H
#define TRIO_TRIO_H

#if !defined(WITHOUT_TRIO)

/*
 * Use autoconf defines if present. Packages using trio must define
 * HAVE_CONFIG_H as a compiler option themselves.
 */
#if defined(HAVE_CONFIG_H)
# include <config.h>
#endif

#include "triodef.h"

#include <stdio.h>
#include <stdlib.h>
#if defined(TRIO_COMPILER_ANCIENT)
# include <varargs.h>
#else
# include <stdarg.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Error codes.
 *
 * Remember to add a textual description to trio_strerror.
 */
enum {
  TRIO_EOF      = 1,
  TRIO_EINVAL   = 2,
  TRIO_ETOOMANY = 3,
  TRIO_EDBLREF  = 4,
  TRIO_EGAP     = 5,
  TRIO_ENOMEM   = 6,
  TRIO_ERANGE   = 7,
  TRIO_ERRNO    = 8,
  TRIO_ECUSTOM  = 9
};

/* Error macros */
#define TRIO_ERROR_CODE(x) ((-(x)) & 0x00FF)
#define TRIO_ERROR_POSITION(x) ((-(x)) >> 8)
#define TRIO_ERROR_NAME(x) trio_strerror(x)

typedef int (*trio_outstream_t) TRIO_PROTO((trio_pointer_t, int));
typedef int (*trio_instream_t) TRIO_PROTO((trio_pointer_t));

TRIO_CONST char *trio_strerror TRIO_PROTO((int));

/*************************************************************************
 * Print Functions
 */

int printf TRIO_PROTO((TRIO_CONST char *format, ...));
int vprintf TRIO_PROTO((TRIO_CONST char *format, va_list args));
int printfv TRIO_PROTO((TRIO_CONST char *format, void **args));

int fprintf TRIO_PROTO((FILE *file, TRIO_CONST char *format, ...));
int vfprintf TRIO_PROTO((FILE *file, TRIO_CONST char *format, va_list args));
int fprintfv TRIO_PROTO((FILE *file, TRIO_CONST char *format, void **args));

int dprintf TRIO_PROTO((int fd, TRIO_CONST char *format, ...));
int vdprintf TRIO_PROTO((int fd, TRIO_CONST char *format, va_list args));
int dprintfv TRIO_PROTO((int fd, TRIO_CONST char *format, void **args));

int cprintf TRIO_PROTO((trio_outstream_t stream, trio_pointer_t closure,
			     TRIO_CONST char *format, ...));
int vcprintf TRIO_PROTO((trio_outstream_t stream, trio_pointer_t closure,
			      TRIO_CONST char *format, va_list args));
int cprintfv TRIO_PROTO((trio_outstream_t stream, trio_pointer_t closure,
			      TRIO_CONST char *format, void **args));

int sprintf TRIO_PROTO((char *buffer, TRIO_CONST char *format, ...));
int vsprintf TRIO_PROTO((char *buffer, TRIO_CONST char *format, va_list args));
int sprintfv TRIO_PROTO((char *buffer, TRIO_CONST char *format, void **args));

int snprintf TRIO_PROTO((char *buffer, size_t max, TRIO_CONST char *format, ...));
int vsnprintf TRIO_PROTO((char *buffer, size_t bufferSize, TRIO_CONST char *format,
		   va_list args));
int snprintfv TRIO_PROTO((char *buffer, size_t bufferSize, TRIO_CONST char *format,
		   void **args));

int snprintfcat TRIO_PROTO((char *buffer, size_t max, TRIO_CONST char *format, ...));
int vsnprintfcat TRIO_PROTO((char *buffer, size_t bufferSize, TRIO_CONST char *format,
                      va_list args));

char *aprintf TRIO_PROTO((TRIO_CONST char *format, ...));
char *vaprintf TRIO_PROTO((TRIO_CONST char *format, va_list args));

int asprintf TRIO_PROTO((char **ret, TRIO_CONST char *format, ...));
int vasprintf TRIO_PROTO((char **ret, TRIO_CONST char *format, va_list args));
int asprintfv TRIO_PROTO((char **result, TRIO_CONST char *format, trio_pointer_t * args));

/*************************************************************************
 * Scan Functions
 */
int scanf TRIO_PROTO((TRIO_CONST char *format, ...));
int vscanf TRIO_PROTO((TRIO_CONST char *format, va_list args));
int scanfv TRIO_PROTO((TRIO_CONST char *format, void **args));

int fscanf TRIO_PROTO((FILE *file, TRIO_CONST char *format, ...));
int vfscanf TRIO_PROTO((FILE *file, TRIO_CONST char *format, va_list args));
int fscanfv TRIO_PROTO((FILE *file, TRIO_CONST char *format, void **args));

int dscanf TRIO_PROTO((int fd, TRIO_CONST char *format, ...));
int vdscanf TRIO_PROTO((int fd, TRIO_CONST char *format, va_list args));
int dscanfv TRIO_PROTO((int fd, TRIO_CONST char *format, void **args));

int cscanf TRIO_PROTO((trio_instream_t stream, trio_pointer_t closure,
			    TRIO_CONST char *format, ...));
int vcscanf TRIO_PROTO((trio_instream_t stream, trio_pointer_t closure,
			     TRIO_CONST char *format, va_list args));
int cscanfv TRIO_PROTO((trio_instream_t stream, trio_pointer_t closure,
			     TRIO_CONST char *format, void **args));

int sscanf TRIO_PROTO((TRIO_CONST char *buffer, TRIO_CONST char *format, ...));
int vsscanf TRIO_PROTO((TRIO_CONST char *buffer, TRIO_CONST char *format, va_list args));
int sscanfv TRIO_PROTO((TRIO_CONST char *buffer, TRIO_CONST char *format, void **args));

/*************************************************************************
 * Locale Functions
 */
void trio_locale_set_decimal_point TRIO_PROTO((char *decimalPoint));
void trio_locale_set_thousand_separator TRIO_PROTO((char *thousandSeparator));
void trio_locale_set_grouping TRIO_PROTO((char *grouping));


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WITHOUT_TRIO */

#endif /* TRIO_TRIO_H */
