#include <smk.h>
#include <devfs.h>


#include "devices.h"




int main( int argc, char *argv[] )
{
	init_devices();
   
	smk_go_dormant();

    return 0;
}

