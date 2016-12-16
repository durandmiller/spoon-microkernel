#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>


#define MAX_SIZE 		( 1024 * 1024 )
#define MAX_TIME		20




static int g_verbose = 0;




static int realloc_random( int verbose )
{
	g_verbose = verbose;

	printf("realloc_random: this will take %i minute...\n", MAX_TIME/ 60 );
	
	int transactions = 0;
	int old_size 	 = 0;
	time_t start_time = time(NULL);
	char *block = NULL;

	int count = 0;

	//	Random madness.
	while (1==1)
	{
			// get a new size.
		int new_size = rand() % MAX_SIZE;

		int diff = time(NULL) - start_time;
		if ( diff > ( MAX_TIME ) ) break;
		
		int tps = (++transactions) / (diff + 1);

		block = (char*)realloc( block, new_size );

		if ( (block == NULL) && (new_size != 0) )
		{
			printf("realloc_random: realloc of size %i returned NULL\n", new_size );
			abort();
		}

		if ( (new_size == 0) && (block != NULL) )
		{
			printf("realloc_random: realloc of size 0 did not return NULL\n");
			abort();
		}

		int max = new_size;
		if ( new_size > old_size ) max = old_size;

		for ( int i = 0; i < max; i++ )
			if ( block[i] != (char)count )
			{
				printf("realloc_random: old memory was no longer valid\n");
				abort();
			}

		count += 1;

		for ( int i = 0; i < new_size; i++ )
			block[i] = (char)count;

		old_size = new_size;
	}	

	// Dump the memory map here.
		

	// Final results.
	printf("%i TPS\n", transactions / MAX_TIME );
	return 0;
}




int realloc_test( int verbose )
{
	realloc_random( verbose );
	realloc_random( verbose );
	realloc_random( verbose );
	return 0;
}



