#ifndef _SGTTY_H
#define _SGTTY_H	1


#ifndef USE_OLD_TTY
#define	USE_OLD_TTY
#endif

#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

int gtty(int fd, struct sgttyb *params);
int stty(int fd, const struct sgttyb *params);


#ifdef __cplusplus
}
#endif


#endif

