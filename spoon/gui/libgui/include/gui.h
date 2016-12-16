#ifndef _LIBGUI_H
#define _LIBGUI_H


#include <smk.h>
#include <comms.h>


#ifdef __cplusplus
extern "C" {
#endif

/* -------- STANDARD DEFINES ---------- */

#define	GUI_PROCESSNAME			"gui_server"

#define GUI_PACKETSIZE			sizeof( struct gui_packet )
#define GUI_PACKETDATA(pkt)		(void*)((char*)pkt + GUI_PACKETSIZE) 		

#define GUI_MAXSIZE				(((2ull<<63)-1) - GUI_PACKETSIZE)

#define STANDARD_WAITTIME		10000

#ifndef	NULL
#define NULL		0
#endif


/* -------- GUI ERRORS -------------- */

#define GUIERR_OK					0
#define GUIERR_ERROR				-1
#define GUIERR_GUI_MISSING			-2
#define GUIERR_NOCOMMS				-3
#define GUIERR_IPC					-4
#define GUIERR_NOMEM				-5
#define GUIERR_SPOOFED				-6
#define GUIERR_TIMEOUT				-7
#define GUIERR_NOTSUPPORTEDOP		-8
#define GUIERR_SHMEM_FAILED			-9
#define	GUIERR_BAD_PARAMETERS		-10
#define	GUIERR_NOT_OWNER			-11
#define	GUIERR_COMMS_SUBSYSTEM_ERR	-12

#define GUIERR_MAXERR				12

/** These are error strings which match the error returned
 * by devices or the gui daemon. Have a look at gui_err 
 * to convert them inline.
 */
static const char* gui_errorstrings[] = 
{
	"no error",
	"general error",
	"the gui server is not running",
	"communication rejected from gui server",
	"unusual IPC error",
	"out of memory",
	"gui server spoofed",
	"timeout waiting for gui server",
	"operation not supported",
	"shared memory failed",
	"the parameters were bad",
	"not the owner",
	"communications subsystems error"
};


/** Converts an error code from gui or drivers into a
 * convenient little string.  */
static inline const char* gui_err( int pos )
{
	if ( pos < 0 ) pos = -pos;
	if ( pos > GUIERR_MAXERR ) return "unknown error";
	return gui_errorstrings[ pos ];
};


/* ------- GENERAL MESSAGES ----------- */

#define GUI_CREATE			1
#define GUI_DESTROY			2

#define GUI_SET				4

#define	GUI_UPDATE			16
#define	GUI_SYNC			32


#define GUI_RESPONSE		65535	


/** This is the internal packet structure used between the
 * gui and all client applications.  Data will usually be appended
 * onto the packet.  */
struct gui_packet
{
	unsigned int 		operation;	///< operation to perform.
	unsigned long long 	length;		///< Length of the packet
	int 				wid;		///< window ID
	int 				req_id;		///< request id
	int 				status;		///< status of the request
	int 				port;		///< port information
	unsigned int		data[10];	///< Any data required.
};


#define		GUI_DATA_X			0
#define		GUI_DATA_Y			1
#define		GUI_DATA_WIDTH		2
#define		GUI_DATA_HEIGHT		3
#define		GUI_DATA_BUTTONS	4
#define		GUI_DATA_FLAGS		5
#define		GUI_DATA_STATE		6
#define		GUI_DATA_SHMEM		7
#define		GUI_DATA_EVENTS		8
#define		GUI_DATA_PORT		9
// If you add another one here, increase 10 limit in struct.



/* ------- WINDOW DEFINES ----------- */


#define		GUI_FLAGS_NORMAL			(1u<<0)
#define		GUI_FLAGS_MODAL				(1u<<1)
#define		GUI_FLAGS_TRANSPARENT		(1u<<2)

#define		GUI_FLAGS_DEFAULT			(GUI_FLAGS_NORMAL)


#define		GUI_STATE_NORMAL			(1u<<0)
#define		GUI_STATE_MINIMIZED			(1u<<1)
#define		GUI_STATE_MAXIMIZED			(1u<<2)
#define		GUI_STATE_HIDDEN			(1u<<3)

#define		GUI_STATE_DEFAULT			(GUI_STATE_HIDDEN)		


#define		GUI_EVENT_MOUSE_MOVED			(1u<<1)
#define		GUI_EVENT_MOUSE_UP				(1u<<2)
#define		GUI_EVENT_MOUSE_DOWN			(1u<<3)
#define		GUI_EVENT_WINDOW_MAXIMIZED		(1u<<4)
#define		GUI_EVENT_WINDOW_MINIMIZED		(1u<<5)
#define		GUI_EVENT_WINDOW_RESTORED		(1u<<6)
#define		GUI_EVENT_WINDOW_RESIZED		(1u<<7)
#define		GUI_EVENT_WINDOW_MOVED			(1u<<8)


#define		GUI_EVENT_DEFAULT										\
				(GUI_EVENT_MOUSE_UP|GUI_EVENT_MOUSE_DOWN|			\
				 GUI_EVENT_WINDOW_RESIZED|GUI_EVENT_WINDOW_MOVED)	

/**  \defgroup GUIOPS  GUI Operations
 *
 * These operations are the typical userland calls which
 * will be done by an application wishing to use the GUI in
 * any way.
 *
 * These are the functions for which you are looking.
 * 
 */

/** @{ */

typedef	struct
{
	int x;					///< X Position of top-left corner
	int y;					///< Y Position of top-left corner
	int width;				///< The width of the window
	int height;				///< The height of the window
	unsigned int flags;		///< The flags of the window
	unsigned int state;		///< The state of the window
	int	wid;				///< Window ID from the server.
	comms_t	*comms;			///< The communication task.
	int	shmem_id;			///< The shared memory ID.
	uint32_t *buffer;		///< The window buffer.
	unsigned int events;	///< The events that the window receives
	unsigned int lock;		///< The lock for the structure
} wid_t;



int 			gui_create_window( wid_t **wid,
									int x, int y,
								   int width, int height,
								   unsigned int flags,
								   unsigned int state,
								   unsigned int events );
void 			gui_destroy_window( wid_t* wid );


int				gui_request_events( wid_t* wid, unsigned int events );
int				gui_release_events( wid_t* wid, unsigned int events );

int				gui_set_flags( wid_t* wid, unsigned int flags );
int				gui_set_state( wid_t* wid, unsigned int state );
int				gui_set_events( wid_t* wid, unsigned int state );

unsigned int	gui_get_flags( wid_t* wid );
unsigned int	gui_get_state( wid_t* wid );
unsigned int	gui_get_events( wid_t* wid );


int				gui_update( wid_t* wid, int x, int y, int width, int height );
int				gui_sync( wid_t* wid );



/** @} */


/**  \defgroup GUICOMMS  GUI Communications
 *
 * These operations are the typical communication channels
 * between an application and the GUI server.
 * 
 */

/** @{ */

int gui_send_response( struct gui_packet *req, struct gui_packet **resp );
int gui_send( struct gui_packet *ds );
int gui_wait( unsigned int timeout );


/** @} */


#ifdef __cplusplus
}
#endif

#endif


