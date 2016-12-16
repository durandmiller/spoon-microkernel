#ifndef _BITS_SYS_STAT_H
#define _BITS_SYS_STAT_H		1

#include <sys/types.h>

struct stat
{
	ino_t     st_ino;
	dev_t     st_dev;
	mode_t    st_mode;
	nlink_t   st_nlink;
	uid_t     st_uid;
	gid_t     st_gid;
	dev_t     st_rdev;
	off_t     st_size;
	time_t    st_atime;
	time_t    st_mtime;
	time_t    st_ctime; 
	blksize_t st_blksize;
	blkcnt_t  st_blocks; 
};



#define	S_IFMT				0170000
#define	S_IFBLK				0060000
#define	S_IFCHR				0020000
#define	S_IFIFO				0010000
#define	S_IFREG				0100000
#define	S_IFDIR				0040000
#define	S_IFLNK				0120000
#define S_IFSOCK			0140000


#define S_IREAD		(0x100)
#define S_IWRITE	(0x80)
#define S_IEXEC		(0x40)

#define	S_ISUID   (S_IREAD 	<< 3 )
#define	S_ISGID   (S_IWRITE << 3 )
#define	S_ISVTX   (S_IEXEC 	<< 3 )

#define	S_IRUSR   (S_IREAD)
#define S_IWUSR   (S_IWRITE)
#define	S_IXUSR   (S_IEXEC)
#define	S_IRGRP   (S_IRUSR >> 3 )
#define	S_IWGRP   (S_IWUSR >> 3 )
#define	S_IXGRP   (S_IXUSR >> 3 )
#define	S_IROTH   (S_IRGRP >> 3 )
#define	S_IWOTH   (S_IWGRP >> 3 )
#define	S_IXOTH   (S_IXGRP >> 3 )
	


#endif

