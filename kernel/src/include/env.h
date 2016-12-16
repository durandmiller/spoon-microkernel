#ifndef _KERNEL_ENV_H
#define _KERNEL_ENV_H

/// Declaration
struct process;

/// The maximum length of the name that can be used for env variables.
#define ENVIRONMENT_NAME_LENGTH		1024


int	set_environment( struct process *proc, 
					unsigned char *name, 
					void *data, unsigned int size );

int get_environment( struct process *proc, 
					unsigned char *name, 
					void *data, unsigned int size );

int get_environment_size( struct process* proc, 
					unsigned char *name, 
					unsigned int *size );

int get_environment_information( struct process* proc, 
						int i, 
						unsigned char *name, 
						unsigned int *size );

int remove_environment( struct process *proc, unsigned char *name );
int release_environment( struct process *proc );


int clone_environment( struct process *dest, struct process *src );


#endif

