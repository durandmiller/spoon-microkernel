#include <smk.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


//#include <readline/readline.h>



int main( int argc, char *argv[] )
{
	int c;


	while ((c = getopt( argc, argv, "a:b:c:vh" )) != -1 )
	{
		switch (c)
		{
			case 'a':
			case 'b':
			case 'c':
					printf( "shell: received parameter %c = \"%s\"\n", c, optarg );
					break;

			case 'v':
			case 'h':
					printf( "shell: received parameter %c\n", c );
					break;
		}
	}


/*
	while (1==1)
	{
		char *line = readline( "Enter a line: " );
		if ( line == NULL ) continue;

		printf( "You wrote: %s\n", line );
	}
*/

	smk_go_dormant();

	return EXIT_SUCCESS;
}

