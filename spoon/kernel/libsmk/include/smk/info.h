#ifndef _LIBSMK_INFO_H
#define _LIBSMK_INFO_H

#include <smk/kernel_info.h>


#ifdef __cplusplus
extern "C" {
#endif


int 	smk_process_get_list( int max, struct process_information *list );

#ifdef __cplusplus
}
#endif

#endif

