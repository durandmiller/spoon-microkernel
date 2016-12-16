#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H		1


#include <sys/types.h>

#include <bits/sys/mman.h>


#ifdef __cplusplus
extern "C" {
#endif
		

int    mlock(const void *, size_t);
int    mlockall(int);
void  *mmap(void *, size_t, int, int, int, off_t);
int    mprotect(void *, size_t, int);
int    msync(void *, size_t, int);
int    munlock(const void *, size_t);
int    munlockall(void);
int    munmap(void *, size_t);
int    shm_open(const char *, int, mode_t);
int    shm_unlink(const char *);


#ifdef __cplusplus
}
#endif
		


#endif

