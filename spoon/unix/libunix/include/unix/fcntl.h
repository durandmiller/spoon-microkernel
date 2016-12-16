#ifndef _FCNTL_H
#define _FCNTL_H		1


#include <sys/types.h>
#include <bits/fcntl.h>


struct flock
{
	short l_type;
	short l_whence;
	off_t l_start;
	off_t l_len; 
	pid_t l_pid;
};

#ifdef __cplusplus
extern "C" {
#endif

int  creat(const char *, mode_t);
int  fcntl(int, int, ...);
int  open(const char *, int, ...);


#ifdef __cplusplus
}
#endif


#endif

