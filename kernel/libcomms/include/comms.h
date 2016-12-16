#ifndef _COMMS_H
#define _COMMS_H


#define	COMMS_MAX_PACKETSIZE	(1024*1024*10)	
#define COMMS_WAIT_TIME			10

#define	COMMS_PULSE		1
#define	COMMS_MESSAGE	2


#define	COMMS_TERMINATE		0x9999u
#define	COMMS_DEFAULT		0x9998u
#define COMMS_PING			0x9997u


#ifndef NULL
#define NULL (void*)0
#endif

typedef int (*comms_handler_t)(void *data, int pid, int port, void *packet);

struct	comms_pair
{
	unsigned int 	operation;	/**< The operation code. */
	int				type;		/**< Indicates pulse versus message */
	comms_handler_t	handler;	/**< The handler function to call. */
};

/** This is the comms_t type which contains information about
 * the communication setup.  */
typedef struct
{
	unsigned int lock;	/**< The lock to safety */
	int tid;			/**< Thread ID of the comms handler. */
	int port;			/**< The port that the comms handler listens on. */
	void *data;			/**< The user data passed to the comms */
	int	count;			/**< The number of handlers. */
	struct comms_pair 	*handlers;	/** The registered handlers. */
} comms_t;

#ifdef __cplusplus
extern "C" {
#endif


int		comms_create( comms_t **comms, void *data );
int		comms_destroy( comms_t **comms );


int		comms_register( comms_t *comms, 
						int type,
						unsigned int operation, 
						comms_handler_t handler );

int		comms_deregister( comms_t *comms, int type, unsigned int operation );

int		comms_get_port( comms_t *comms );
int		comms_get_tid( comms_t *comms );
				
int		comms_start( comms_t *comms );
int		comms_terminate( comms_t *comms );
int		comms_wait( comms_t *comms, unsigned int timeout );
int		comms_kill( comms_t *comms );



int	comms_test_pid( int pid, int port, unsigned int timeout );
int	comms_test_name( const char *name, int port, unsigned int timeout );


#ifdef __cplusplus
}       
#endif


#endif

