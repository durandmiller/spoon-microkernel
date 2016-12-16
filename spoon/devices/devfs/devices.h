#ifndef _DEVICES_H
#define _DEVICES_H

#include <devfs.h>

struct device_hook
{
	int pid;
	int fd;
};

struct stream_hook
{
	int pid;
	int port;
	int fd;
};

struct device_entry
{
	int pid;
	int port;
	int id;

	unsigned int hook_count;
	struct device_hook *hooks;

	unsigned int stream_count;
	struct stream_hook *streams;
	
	unsigned long long size;
	unsigned int flags;
	char *name;
};



int init_devices();

int register_device( int pid, int port, struct devfs_packet* ds );
int deregister_device( int pid, int port, struct devfs_packet* ds );




struct device_entry* find_device_by_match( const char *pattern, int pos );
struct device_entry* find_device_by_name( const char *name );
struct device_entry* find_device_by_id( int id );


struct device_entry* hook_device( const char *name, int pid, int fd );
int release_device( struct device_entry* dev, int pid, int fd );


int get_stream( struct device_entry* dev, int pid, int port, int fd );
int release_stream( struct device_entry* dev, int pid, int port );


#endif

