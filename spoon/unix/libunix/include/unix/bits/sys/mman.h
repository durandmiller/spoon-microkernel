#ifndef _BITS_SYS_MMAN_H
#define _BITS_SYS_MMAN_H		1


#define	PROT_READ			0x1
#define	PROT_WRITE			0x2
#define	PROT_EXEC			0x4
#define	PROT_NONE			0x0


#define	MAP_SHARED			0x1
#define	MAP_PRIVATE			0x2
#define	MAP_FIXED			0x10

#define MAP_ANONYMOUS		0x20
#define MAP_ANON			MAP_ANONYMOUS


#define	MS_ASYNC			1
#define	MS_SYNC				4
#define	MS_INVALIDATE		2


#define	MCL_CURRENT			1
#define	MCL_FUTURE			2

#define	MAP_FAILED		((void*)-1)


#endif

