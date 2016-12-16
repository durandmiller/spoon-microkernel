#ifndef _MONETARY_H
#define _MONETARY_H			1


#include <inttypes.h>


#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
#ifndef size_t
typedef	uint32_t	size_t;
#endif
#endif

#ifndef _HAVE_SSIZE_T
#define _HAVE_SSIZE_T
#ifndef ssize_t
typedef	int32_t		ssize_t;
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif
		

ssize_t    strfmon(char *, size_t, const char *, ...);

#ifdef __cplusplus
}
#endif
		
	

#endif

