#ifndef _LIBVFS_H
#define _LIBVFS_H


/* -------- STANDARD DEFINES ---------- */

#define VFS_PACKETSIZE			sizeof( struct vfs_packet )
#define VFS_PACKETDATA(pkt)		(void*)((char*)pkt + VFS_PACKETSIZE) 		

#define VFS_MAXSIZE			(((2ull<<63)-1) - VFS_PACKETSIZE)

#define STANDARD_WAITTIME		10000

#ifndef	NULL
#define NULL		0
#endif


/* -------- VFS ERRORS -------------- */

#define VFSERR_OK					0	
#define VFSERR_ERROR				-1
#define VFSERR_NOTFOUND				-2
#define VFSERR_BAD_MESSAGE			-3
#define VFSERR_EXISTS				-4
#define VFSERR_BAD_ID				-5
#define VFSERR_NOTSUPPORTEDOP		-6
#define VFSERR_VFS_MISSING			-7
#define VFSERR_BAD_PARAMETERS		-8
#define VFSERR_NOMEM				-9
#define VFSERR_NOCOMMS				-10
#define VFSERR_NOFILE_LOCAL			-11
#define VFSERR_NOFILE_REMOTE		-12
#define VFSERR_SPOOFED				-13
#define VFSERR_TIMEOUT				-14
#define VFSERR_IPC					-15
#define VFSERR_INUSE				-16
#define VFSERR_BAD_RESPONSE			-17
#define VFSERR_BAD_FLAGS			-18
#define VFSERR_TOOLARGE				-19
#define VFSERR_TOOSMALL				-20
#define VFSERR_FILESYSTEM_UNKNOWN	-21
#define VFSERR_FILESYSTEM_INCORRECT	-22
#define VFSERR_NO_MOUNT				-23

#define VFSERR_MAXERR				23

/** These are error strings which match the error returned
 * by the vfs daemon. Have a look at vfs_err to convert them 
 * inline.
 */
static const char* vfs_errorstrings[] = 
{
	"no error",
	"general error",
	"no such file or directory",
	"bad message",
	"already exists",
	"bad file id given",
	"operation not supported",
	"vfs server is missing",
	"one of the parameters provided is invalid",
	"out of memory",
	"unable to communicate with devfs",
	"no such file registered locally",
	"no such file registered remotely",
	"a spoofed response was returned. kernel error?",
	"timeout occured waiting for response from vfs",
	"IPC error occured",
	"file in use",
	"bad response from vfs",
	"some of the flags or modes sent are not available",
	"the size of the packet is too large",
	"the size of the packet is too small",
	"the filesystem is unknown",
	"the filesystem is incorrect",
	"no valid mount associated with that filepath"
};


/** Converts an error code from vfs or drivers into a
 * convenient little string.  */
static inline const char* vfs_err( int pos )
{
	if ( pos < 0 ) pos = -pos;
	if ( pos > VFSERR_MAXERR ) return "unknown error";
	return vfs_errorstrings[ pos ];
};


/* ------- GENERAL MESSAGES ----------- */

#define VFS_OPEN				1
#define VFS_CLOSE				2
#define VFS_READ				4
#define VFS_WRITE				8
#define VFS_STAT				16
#define VFS_RM					32
#define VFS_CHMOD				64
#define VFS_MKDIR				128
#define VFS_CREATE				256
#define VFS_LS					512


#define VFS_UNMOUNT				60000
#define VFS_MOUNT				60001

#define VFS_MATCH				60002

#define VFS_RESPONSE			65535


/* ------ FILE AND MOUNT FLAGS ---------- */

#define VFS_FLAG_OPEN			1
#define VFS_FLAG_READ			2
#define VFS_FLAG_WRITE			4
#define VFS_FLAG_EXECUTE		8


#define VFS_FLAG_ALL			(1|2|4|8)

/* ------ FILE MASKS RETURNED IN STAT --- */

#define	VFS_S_IFMT     017000	//	bitmask for the file type bitfields
#define	VFS_S_IFSOCK   014000	//	socket
#define	VFS_S_IFLNK    012000	//	symbolic link
#define	VFS_S_IFREG    010000	//	regular file
#define	VFS_S_IFBLK    006000	//	block device
#define	VFS_S_IFDIR    004000	//	directory
#define	VFS_S_IFCHR    002000	//	character device
#define	VFS_S_IFIFO    001000	//	FIFO
#define	VFS_S_ISUID    000400	//	set UID bit
#define	VFS_S_ISGID    000200	//	set-group-ID bit (see below)
#define	VFS_S_ISVTX    000100	//	sticky bit (see below)
#define	VFS_S_IRWXU    0070	//	mask for file owner permissions
#define	VFS_S_IRUSR    0040	//	owner has read permission
#define	VFS_S_IWUSR    0020	//	owner has write permission
#define	VFS_S_IXUSR    0010	//	owner has execute permission
#define	VFS_S_IRWXG    0007	//	mask for group permissions
#define	VFS_S_IRGRP    0004	//	group has read permission
#define	VFS_S_IWGRP    0002	//	group has write permission
#define	VFS_S_IXGRP    0001	//	group has execute permission
#define	VFS_S_IRWXO    0007	//	mask for permissions for others (not in group)
#define	VFS_S_IROTH    0004	//	others have read permission
#define	VFS_S_IWOTH    0002	//	others have write permission
#define	VFS_S_IXOTH    0001	//	others have execute permission


/** This is the internal packet structure used between the
 * vfs and all client applications.  Data will usually be appended
 * onto the packet.  */
struct vfs_packet
{
	unsigned int operation;			///< operation to perform.
	unsigned long long length;		///< Length of the packet
	int fd;							///< file descriptor
	int req_id;						///< request id
	int status;						///< status of the request

	unsigned long long size;		///< size of the file or relevant thing 
	unsigned long long position;	///< position in the file
	unsigned int flags;				///< flags for the operation 
	
	int pid;						///< pid information
	int port;						///< port information
};


/** This is the file stat structure which returns information
 * about the file you're trying to stat */
struct vfsstat
{
	char *filename;					///< The name of the file (only used in ls)
	unsigned long long size;		///< size of the file or relevant thing 
	unsigned int mode;				///< The mode of the file. (directory, etc)
	unsigned int uid;				///< The user ID associated with the file
	unsigned int gid;				///< The group ID associated with the file
	unsigned int did;				///< The device ID associated with the file
	unsigned int inode;				///< The inode number
	unsigned int nlinks;			///< The number of hard links
	unsigned int blocksize;			///< The blocksize of the file
	unsigned int blocks;			///< The number of blocks used.
	unsigned int atime;				///< The last access time
	unsigned int mtime;				///< The last modification time
	unsigned int ctime;				///< The last change time
};



/* File operations */

int			vfs_create( const char *filename, unsigned int mode );

int 		vfs_open( const char *filename, unsigned int flags );
int 		vfs_close( int fd );

long long 	vfs_read( int fd, long long position, 
							char *buffer, long long bytes );
long long 	vfs_write( int fd, long long position, 
							const char *buffer, long long bytes );


int			vfs_mkdir( const char *filename, unsigned int mode, 
							unsigned int flags );
int			vfs_rm( const char *filename, unsigned int mode );
int			vfs_stat( const char *filename, struct vfsstat *st );
int			vfs_chmod( const char *filename, unsigned int mode );
int			vfs_ls( const char *filename, struct vfsstat **list, int *max );


int			vfs_ls_free( struct vfsstat **list, int max );

/* Mount operations */

int			vfs_mount( const char *source, const char *mount, 
						const char *fs, unsigned int flags );
int			vfs_umount( const char *mount, unsigned int flags );


/* Communication */

int vfs_send_response( struct vfs_packet *req, struct vfs_packet **resp );
int vfs_send( struct vfs_packet *ds );
int vfs_wait( unsigned int timeout );




#endif



