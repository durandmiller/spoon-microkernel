#ifndef _FILES_H
#define _FILES_H


struct file
{
	int fd;

	struct device_entry *device;
	unsigned int flags;
};

extern int ft_key( void *fdt );
extern int ft_cmp( void* ldt, void* rdt );


struct file* get_file( struct devfs_htable *ftable, int fd );



int open( int pid, int port, struct devfs_packet* ds );
int close( int pid, int port, struct devfs_packet* ds );


int dispatch_request( int pid, int port, struct devfs_packet* ds );
int dispatch_push( int pid, int port, struct devfs_packet* ds );


int dispatch_pulse_push( int pid, int port, void *dsu );

int stream( int pid, int port, struct devfs_packet* ds );

int stat( int pid, int port, struct devfs_packet* ds );
int match( int pid, int port, struct devfs_packet* ds );

#endif

