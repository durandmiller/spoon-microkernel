#ifndef _DEVCOMMS_H
#define _DEVCOMMS_H

int send_packet( int pid, int port, struct gui_packet *ds );
int send_resp( int pid, int port, struct gui_packet *ds, int stat );


int send_event( int pid, int port, 
				unsigned int event,
				int wid, 
				int x, int y, unsigned int buttons );

#endif

