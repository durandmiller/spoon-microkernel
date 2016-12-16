#ifndef _LIBDEVFS_H
#define _LIBDEVFS_H


/* -------- STANDARD DEFINES ---------- */

#define PACKETSIZE		sizeof( struct devfs_packet )

#ifndef	NULL
#define NULL		0
#endif


/* -------- DEVFS ERRORS -------------- */

#define DEVFSERR_OK					0
#define DEVFSERR_ERROR				-1
#define DEVFSERR_NOTFOUND			-2
#define DEVFSERR_BADMESSAGE			-3
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

#define DEVFSERR_MAXERR				15

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
	"operation not supported by device",
	"devfs server is missing",
	"one of the parameters provided is invalid",
	"out of memory",
	"unable to communicate with devfs",
	"no such device registered locally",
	"no such device registered remotely",
	"a spoofed response was returned. kernel error?",
	"timeout occured waiting for response from devfs",
	"IPC error occured"
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


/* -------- DEVICE FLAGS USED DURING REGISTRATION -------- */

#define DEVFS_FLAG_READ			1
#define DEVFS_FLAG_WRITE		2
#define DEVFS_FLAG_STREAM		4


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
	int (*stream)( void *data, int id );
	int (*script)( void *data, int pid, int req_id, 
					const char *commands );
	int (*read)( void *data, int pid, int req_id, 
					unsigned long long position, 
					unsigned long long bytes );
	int (*write)( void *data, int pid, int req_id, 
					unsigned long long position, 
					unsigned long long bytes,
					void *buffer );
};



/* Device registration and deregistration */

int devfs_register( const char *device, 
					unsigned long long size, 
					unsigned int flags,
					struct devfs_hooks *hooks,
					void *data );

int devfs_deregister( const char *device );


/* Device operations */

int devfs_open( const char *device, unsigned int flags );
int devfs_close( int fd );

/* Communication */

int devfs_init();
int devfs_send_response( struct devfs_packet *req, struct devfs_packet **resp );
int devfs_send( struct devfs_packet *ds );
int devfs_wait( unsigned int timeout );

/** Send back a single status code reflecting the outcome of a request */
int devfs_send_ack( int req_id, unsigned int status ); 
/** Send back actual data in response to a previous request */
int devfs_send_data( int req_id, void *buffer, unsigned long long bytes );
/** Send stream data to the devfs to be multiplexed amongst apps */
int devfs_stream_data( int id, void *buffer, unsigned long long bytes );

/* Miscellaneous */


int 	devfs_strlen( const char *str );
char* 	devfs_strcpy(char* s1, const char* s2);
int     devfs_strcmp(const char *, const char *);
char*	devfs_strdup(const char *s);
void* 	devfs_memcpy(void* s1, const void* s2, unsigned int n);
void* 	devfs_memset(void* s, int c, unsigned int n);


/* Memory operations */

void* devfs_malloc(int size);
void  devfs_free(void *ptr);
void* devfs_calloc(int nobj, int size);
void* devfs_realloc(void *p, int size);


/* Debugging assistance */

int devfs_dmesg( char *format, ... );

/* Hash table operations */

typedef int (*hash_key_func)(void *);
typedef int (*hash_compare_func)(void *,void*);


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



/* The event loop information */

extern int devfs_ev_tid;
extern int devfs_ev_port;


#endif



