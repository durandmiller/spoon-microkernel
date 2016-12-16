#ifndef _SMK_SYSCALLDEFS_H
#define _SMK_SYSCALLDEFS_H


#define     SYS_MEMORY      (0x1u<<8)
#define		SYS_SEMAPHORE	(0x2u<<8)
#define		SYS_SYSTEM		(0x3u<<8)
#define		SYS_PROCESS		(0x4u<<8)
#define		SYS_THREAD		(0x5u<<8)
#define		SYS_CAP			(0x6u<<8)
#define 	SYS_PULSE		(0x7u<<8)
#define 	SYS_MESSAGE		(0x8u<<8)
#define 	SYS_PORT		(0x9u<<8)
#define     SYS_EXEC		(0xAu<<8)
#define		SYS_PCI			(0xBu<<8)
#define		SYS_TIME		(0xCu<<8)
#define		SYS_IRQ			(0xDu<<8)
#define 	SYS_IO			(0xEu<<8)
#define		SYS_CONSOLE		(0xFu<<8)
#define		SYS_LFB			(0x10u<<8)
#define 	SYS_INFO		(0x11u<<8)
#define     SYS_MISC		(0x12u<<8)
#define     SYS_SHMEM		(0x13u<<8)
#define     SYS_EVENTS		(0x14u<<8)

// System call functions

#define		SYS_ONE			(1u<<0)
#define		SYS_TWO			(1u<<1)
#define		SYS_THREE		(1u<<2)
#define		SYS_FOUR		(1u<<3)
#define		SYS_FIVE		(1u<<4)
#define		SYS_SIX			(1u<<5)
#define		SYS_SEVEN		(1u<<6)
#define		SYS_EIGHT		(1u<<7)



#endif


