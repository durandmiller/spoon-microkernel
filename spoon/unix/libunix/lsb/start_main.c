#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <support/support.h>

int __libc_start_main(int (*main) (int, char **, char **), 
					  int argc, char **ubp_av, 
					  void (*init) (void), 
					  void (*fini) (void), 
					  void (*rtld_fini) (void), 
					  void (*stack_end))
{
	int rc;



#warning implement me properly
	environ = &ubp_av[argc + 1];

	rc = initialize_thread_data();
	if ( rc != 0 ) return rc;

	errno = 0;

	if ( rtld_fini != NULL) atexit(rtld_fini);

	if ( fini != NULL) atexit(fini);

	if ( init != NULL ) init();

	exit( main(argc, ubp_av, environ) );
	return 0;
}



