#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define SIZE		0x100


void doit_local()
{
	int i;
	void *ptr[1000];

	srand(time(NULL));

	for ( i = 0; i < 1000; i++ )
	{
		size_t size = random() % SIZE;
		ptr[i] = malloc( size );
	}

	for ( i = 0; i < 1000; i++ )
	{
		size_t size = random() % SIZE;
		ptr[i] = realloc( ptr[i], size );
	}

	for ( i = 0; i < 1000; i++ )
	{
		free( ptr[i] );
	}

	
}

int main( int argc, char *argv[] )
{
	doit_local();
		
	return EXIT_SUCCESS;
}

