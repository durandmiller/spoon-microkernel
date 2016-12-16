#ifndef _GLOBAL_H
#define _GLOBAL_H



extern		char*			root_device;
extern		char*			list_pattern;
extern		unsigned int	startup_wait;




int	parse_opts( int argc, char *argv[] );
int print_usage();


#endif

