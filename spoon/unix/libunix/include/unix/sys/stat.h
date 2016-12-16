#ifndef _SYS_STAT_H
#define _SYS_STAT_H		1


#include <bits/sys/stat.h>
																													

#define	S_IRWXU		( S_IRUSR | S_IWUSR | S_IXUSR )
#define	S_IRWXO		( S_IRGRP | S_IWGRP | S_IXGRP )
#define	S_IRWXG		( S_IROTH | S_IWOTH | S_IXOTH )

#define S_ISBLK(m)	( (m==S_IFBLK) )
#define S_ISCHR(m)	( (m==S_IFCHR) )
#define S_ISDIR(m)	( (m==S_IFDIR) )
#define S_ISFIFO(m)	( (m==S_IFIFO) )
#define S_ISREG(m)	( (m==S_IFREG) )
#define S_ISLNK(m)	( (m==S_IFLNK) )
// NOT A PART OF THE STANDARD
#define S_ISSOCK(m)	( (m==S_IFSOCK) )


// TODO: complete

#define	S_TYPEISMQ(buf)			0
#define	S_TYPEISSEM(buf)		0
#define	S_TYPEISSHM(buf)		0


#define	S_DEFAULT	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#ifdef __cplusplus
extern "C" {
#endif

int    chmod(const char *, mode_t);
int    fchmod(int, mode_t);
int    fstat(int, struct stat *);
int    lstat(const char *, struct stat *);
int    mkdir(const char *, mode_t);
int    mkfifo(const char *, mode_t);
int    mknod(const char *, mode_t, dev_t);
int    stat(const char *, struct stat *);
mode_t umask(mode_t);
																				      
#ifdef __cplusplus
}
#endif

#endif

