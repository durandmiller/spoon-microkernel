#ifndef _LIBDEVFS_H
#define _LIBDEVFS_H

#include <comms.h>


/* -------- STANDARD DEFINES ---------- */

#define	DEVFS_PROCESSNAME			"devfs"

#define DEVFS_PACKETSIZE			sizeof( struct devfs_packet )
#define DEVFS_PACKETDATA(pkt)		(void*)((char*)pkt + DEVFS_PACKETSIZE) 		

#define DEVFS_MAXSIZE			(((2ull<<63)-1) - DEVFS_PACKETSIZE)

#define STANDARD_WAITTIME		10000

#ifndef	NULL
#define NULL		0
#endif


/* -------- DEVFS ERRORS -------------- */

#define DEVFSERR_OK					0
#define DEVFSERR_ERROR				-1
#define DEVFSERR_NOTFOUND			-2
#define DEVFSERR_BAD_MESSAGE		-3
#define DEVFSERR_EXISTS				-4
#define DEVFSERR_BAD_DEVICE			-5
#define DEVFSERR_NOTSUPPORTEDOP		-6
#define DEVFSERR_DEVFS_MISSING		-7
#define DEVFSERR_BAD_PARAMETERS		-8
#define DEVFSERR_NOMEM				-9
#define DEVFSERR_NOCOMMS			-10
#define DEVFSERR_NODEV_LOCAL		-11
#define DEVFSERR_NODEV_REMOTE		-12
#define DEVFSERR_SPOOFED			-13
#define DEVFSERR_TIMEOUT			-14
#define DEVFSERR_IPC				-15
#define DEVFSERR_INUSE				-16
#define DEVFSERR_BAD_RESPONSE		-17
#define DEVFSERR_BAD_FLAGS			-18
#define DEVFSERR_TOOLARGE			-19
#define DEVFSERR_TOOSMALL			-20
#define DEVFSERR_SHMEM_FAILED		-21
#define DEVFSERR_NOT_PAGE_ALIGNED	-22
#define DEVFSERR_FLAGS_REJECTED		-23
#define	DEVFSERR_COMMS_SUBSYSTEM_FAILURE	-24

#define DEVFSERR_MAXERR				24

#ifdef __cplusplus
extern "C" {
#endif

/** These are error strings which match the error returned
 * by devices or the devfs daemon. Have a look at devfs_err 
 * to convert them inline.
 */
static const char* devfs_errorstrings[] = 
{
	"no error",
	"general error",
	"no such file or directory",
	"bad message",
	"already exists",
	"bad device id given",
	"operation not supported",
	"devfs server is missing",
	"one of the parameters provided is invalid",
	"out of memory",
	"unable to communicate with devfs",
	"no such device registered locally",
	"no such device registered remotely",
	"a spoofed response was returned. kernel error?",
	"timeout occured waiting for response from devfs",
	"IPC error occured",
	"device in use",
	"bad response from devfs",
	"some of the flags or modes sent are not available",
	"the size of the packet is too large",
	"the size of the packet is too small",
	"shared memory failed",
	"not page aligned",
	"flags rejected (driver side)",
	"communication subsystem failure"
};


/** Converts an error code from devfs or drivers into a
 * convenient little string.  */
static inline const char* devfs_err( int pos )
{
	if ( pos < 0 ) pos = -pos;
	if ( pos > DEVFSERR_MAXERR ) return "unknown error";
	return devfs_errorstrings[ pos ];
};


/* ------- GENERAL MESSAGES ----------- */

#define DEVFS_REGISTER			1
#define DEVFS_DEREGISTER		2
#define DEVFS_STREAM			4
#define DEVFS_OPEN				8
#define DEVFS_CLOSE				16
#define DEVFS_READ				32
#define DEVFS_WRITE				64
#define DEVFS_SCRIPT			128
#define DEVFS_STAT				256
#define DEVFS_MATCH				512
#define DEVFS_MAP				1024
#define DEVFS_UNMAP				2048

#define DEVFS_PUSH				65534
#define DEVFS_RESPONSE			65535	


/* -------- DEVICE FLAGS USED DURING REGISTRATION -------- */

#define DEVFS_FLAG_OPEN			1
#define DEVFS_FLAG_READ			2
#define DEVFS_FLAG_WRITE		4
#define DEVFS_FLAG_STREAM		8
#define DEVFS_FLAG_SCRIPT		16
#define DEVFS_FLAG_MAP			32
#define DEVFS_FLAG_UNMAP		64


#define DEVFS_FLAG_ALL			(1|2|4|8|16|32|64)

/** This is the internal packet structure used between the
 * devfs and all client applications.  Data will usually be appended
 * onto the packet.  */
struct devfs_packet
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


/** This is the device stat structure which returns information
 * about the device you're trying to stat */
struct dstat
{
	unsigned long long size;		///< size of the file or relevant thing 
	unsigned int flags;				///< flags for the operation 
	int pid;						///< pid information
};

/** This is the struct returned by the devfs when a pattern
 * match was requested of devfs.  
 * 
 * "bytes" is the size of the entire data structure (bytes,type,etc + name)
 * "type" refers to 0 or 1 where 0 is a directory or 1 is an actual device
 * "pid" The device driver pid
 * "flags" is the flags of the device
 * "size" is the size of the device if type == 1
 * "name is the name of the device with a terminating NULL character
 *
 */
struct devfs_matchinfo
{
	unsigned int bytes;
	int type;
	int pid;
	unsigned int flags;
	unsigned long long size;
	char	name[];
} __attribute__ ((packed));


/* ------- DEVICE HOOKS -------------------- */

/** This is the structure that should be passed to the
 * devfs_register function. It provides a list of hooks
 * which the main device loop should call whenever
 * the device is required.
 *
 * You only need to provide hooks that you impement
 * and the device_data.
 *
 * \warning device_data will be freed when the
 * hook structure is freed. Make sure you don't own
 * or plan to use either past that point.
 */
struct devfs_hooks
{
	int (*open)( void* data, int pid );
	int (*close)( void *data, int pid );
	int (*stream)( void *data, int flag );
	int (*script)( void *data, int pid, 
					const char *commands, char **response );
	long long (*read)( void *data, int pid,  
					unsigned long long position, 
					unsigned long long bytes,
					void *buffer );
	long long (*write)( void *data, int pid,  
					unsigned long long position, 
					unsigned long long bytes,
					void *buffer );
	int	(*map)( void *data, int pid,
				unsigned long long position,
				unsigned long long bytes,
				unsigned int flags );
	int	(*unmap)( void *data, int pid );
};



/**  \defgroup DEVREG Device Registration and Deregistration
 *
 * These functions allow an application to register as a 
 * device driver with the devfs.
 * 
 * Only actual drivers will make use of these function. You
 * shouldn't be calling these if you just want to use a
 * device driver that is already registered with devfs.
 * 
 * \note You will need to have already called devfs_init() 
 * before the (de)registration functions will work. Please do 
 * that!
 * 
 */

/** @{ */

int devfs_init();

int devfs_register( const char *device, 
					unsigned long long size, 
					unsigned int flags,
					struct devfs_hooks *hooks,
					void *data );

int devfs_deregister( int fd );

/** @} */

/**  \defgroup DEVOPS  Device Operations
 *
 * These operations are the typical userland calls which
 * will be done by an application wishing to make use of
 * the devfs and all registered drivers.
 *
 * These are probably the functions you are looking for.
 * 
 */

/** @{ */



int 		devfs_open( const char *device, unsigned int flags );
int 		devfs_close( int fd );
int 		devfs_script( int fd, char *commands, char **response );
long long 	devfs_read( int fd, long long position, 
							char *buffer, long long bytes );
long long 	devfs_write( int fd, long long position, 
							char *buffer, long long bytes );
int 		devfs_set_streaming( int fd, int status, 
								void (*func)(int,void*,long long) );
int			devfs_stat( const char *device, struct dstat *st );
int			devfs_match( const char *pattern, struct devfs_matchinfo ***list );
int			devfs_map( int fd, long long position, 
						long long bytes, unsigned int flags,
						void **ptr );
int			devfs_unmap( int fd, void **ptr );


/** @} */



/* Internal device push */

int			devfs_push( int device_id, void *data, long long len );
int			devfs_push_fast( int device_id, 
								int d1, int d2, int d3, int d4 );


/* Communication */

int devfs_send_response( struct devfs_packet *req, struct devfs_packet **resp );
int devfs_send( struct devfs_packet *ds );
int devfs_respond( struct devfs_packet *ds );
int devfs_wait( unsigned int timeout );

/* Event Loop */

int devfs_send_ack( int req_id, unsigned int status ); 
int devfs_send_data( int req_id, void *buffer, unsigned long long bytes );
int devfs_stream_data( int id, void *buffer, unsigned long long bytes );

/**  \defgroup DEVMISC Miscellaneous
 *
 * These are miscellaneous functions which libdevfs
 * includes by default. They're here in order to provide
 * libdevfs, devfs and drivers independance from any
 * other userland library. Please use these instead of
 * any standard functions which do the same!
 * 
 */

/** @{ */


int 	devfs_strlen( const char *str );
char* 	devfs_strcat(char* s1, const char* s2);
char* 	devfs_strcpy(char* s1, const char* s2);
int     devfs_strcmp(const char *, const char *);
int     devfs_strncmp(const char *, const char *, int n);
char*	devfs_strdup(const char *s);
char*	devfs_strndup(const char *s, int n);
int     devfs_memcmp(const char *, const char *, int i);
void* 	devfs_memcpy(void* s1, const void* s2, unsigned int n);
void* 	devfs_memset(void* s, int c, unsigned int n);
int		devfs_split( char *str, const char *tokens, char **array, int max ); 

/** @} */


/* Debugging assistance */

int devfs_dmesg( char *format, ... );

/**  \defgroup DEVHASH Hash Table operations
 *
 * libdevfs has hash table operations included in it in
 * order to speed up lookups, etc. These are the related
 * functions, defines and data-structures.
 */

/** @{ */

/** This is the key function. It's supposed to return a
 * unique (or close enough) number based on the data it 
 * have been given. Internally, the hash table ensures
 * that the returned number is sane.
 */
typedef int (*hash_key_func)(void *);

/** This is the comparision function. This function is called
 * during a hash_retrieve() in order to determine
 * if the data is the data being searched for. The hash_retrieve()
 * function accepts sample data and this sample data is always
 * returned as the second parameter to this function. The
 * changing data is the first parameter.
 *
 * \return <0 on the sample data being less than the other
 * \return 0 on two data equal
 * \return >0 on the sample data being greater than the other
 */
typedef int (*hash_compare_func)(void *,void*);


/** This is the hash table structure which contains
 * all information relevant for hash tables. */
struct devfs_htable 
{
	hash_compare_func compare;
	hash_key_func key;
	int total;
	int size;
	int percent;
	void **table;
};


struct devfs_htable *devfs_init_htable( int size, 
				int percent, 
				hash_key_func key,
				hash_compare_func compare
			  );

int devfs_delete_htable( struct devfs_htable* );

int devfs_htable_rehash( struct devfs_htable*, int new_size );

int devfs_htable_insert( struct devfs_htable* table, void *data );

int devfs_htable_remove( struct devfs_htable* table, void *data );

void *devfs_htable_retrieve( struct devfs_htable* table, void *sample );

void *devfs_htable_get( struct devfs_htable* table, int num );

/** @} */



/* The event loop information */

extern int 		devfs_pid;
extern comms_t* devfs_ev_comms;


/** This is an internal structure used by libdevfs to
 * manage multiple devices in an application.
 *
 * This is just a nice place to put this structure definition
 * because duplicate copies have already caused me days of
 * debugging
 */
struct d_entry
{
	int id;
	int streaming;
	void *device_data;
	char *name;
	struct devfs_hooks *hooks;
};



extern struct d_entry* devfs_getDevice( int id );


/** This manages the internal streaming on the client side */
struct d_stream
{
	int fd;
	void (*func)(int,void*,long long);
};


extern	comms_t*	devfs_st_comms;

struct d_stream* 	devfs_getStream( int fd );
int 				devfs_streamInit();
int 				devfs_removeStream( int fd );
int				 	devfs_addStream( int fd, 
									 void (*func)(int,void*,long long) );


#ifdef __cplusplus
}
#endif


#endif



