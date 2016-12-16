#include <errno.h>



/** Put in here for libm. Seems to want it. */
int *__errno_location(void)
{
	return &(get_thread_data()->error_number);
}

