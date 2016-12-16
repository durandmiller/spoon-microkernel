#include <smk.h>
#include <gui.h>



int send_packet( int pid, int port, struct gui_packet *ds )
{
	int rc = smk_send_message( 0, pid, port, ds->length, ds );


	if ( rc != 0 ) return -1;
	return 0;
}

int send_resp( int pid, int port, struct gui_packet *ds, int stat )
{
	int rc;
	
	ds->length = GUI_PACKETSIZE;
	ds->status = stat;
		

	rc =  smk_send_message( 0, pid, port, GUI_PACKETSIZE, ds );

	if ( rc != 0 )
	{
		return rc;
	}

	return stat;
}


int send_event( int pid, int port, 
				unsigned int event,
				int wid, 
				int x, int y, unsigned int buttons )
{

	int rc = smk_send_pulse( 0, pid, port,
								event,
								(unsigned int)wid,
								(unsigned int)x,
								(unsigned int)y,
								(unsigned int)buttons,
								0 );

	return rc;
}



