#ifdef _TEST_
#define _GNU_SOURCE
#define __USE_GNU
#include <assert.h>
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <arb.h>


static void* compatible_alloc(size_t size)
{
	return malloc( size );
}

static void compatible_free(void* ptr,size_t size)
{
	free(ptr);
}




int main( int argc, char *argv[] )
{
	arb_t *arb = arb_create( 0x1000,  
								compatible_alloc, 
								compatible_free,
							0x10000 );
	if ( arb == NULL ) return EXIT_FAILURE;


		arb_add_range( arb, 0x0,  0x1000000 );


		// Different test conditions
		
		//arb_reserve( arb, 0, 0x1000 );
		//arb_reserve( arb, 0, 0x1000000 );
		//arb_reserve( arb, 0x1000000 - 0x1000, 0x1000 );
		arb_reserve( arb, 0x1000000 - 0x2000, 0x1000 );
		//arb_reserve( arb, 0x1000000 - 0x2000, 0x5000 );



		arb_display( arb, 1 );


	arb_destroy( arb );
		
	return EXIT_SUCCESS;
}



							


