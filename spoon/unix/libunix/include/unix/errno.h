#ifndef _ERRNO_H
#define _ERRNO_H		1


#include <bits/errno.h>



#ifdef __cplusplus
extern "C" {
#endif


#include <support/support.h>
#define errno	get_thread_data()->error_number
int *__errno_location(void);

#ifdef __cplusplus
}
#endif


#endif

