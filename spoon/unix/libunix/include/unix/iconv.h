#ifndef _ICONV_H
#define _ICONV_H		1

#include <inttypes.h>

#ifndef _HAVE_ICONV_T
#define _HAVE_ICONV_T
#ifndef iconv_t
typedef void* iconv_t;
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif
		
iconv_t iconv_open(const char *, const char *);
size_t  iconv(iconv_t, char **, size_t *, char **, size_t *);
int     iconv_close(iconv_t);


#ifdef __cplusplus
}
#endif
		


#endif

