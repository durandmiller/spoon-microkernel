#ifndef _SMK_INTTYPES_H
#define _SMK_INTTYPES_H


#define PAGE_SIZE		4096

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef _HAVE_SIZE_T
#define _HAVE_SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _HAVE_TIME_T
#define _HAVE_TIME_T
typedef unsigned int time_t;
#endif



typedef unsigned long long 		uint64_t;
typedef unsigned int 			uint32_t;
typedef unsigned short 			uint16_t;
typedef unsigned char 			uint8_t;


typedef long long 				int64_t;
typedef int 					int32_t;
typedef short 					int16_t;
typedef char 					int8_t;

typedef int32_t  				intptr_t;
typedef uint32_t 				uintptr_t;

#define TO_INTPTR(ptr) 		  	((intptr_t)(ptr))
#define TO_UINTPTR(ptr) 		((uintptr_t)(ptr))
#define TO_PTR(intptr)   		((void*)(intptr))


#endif


