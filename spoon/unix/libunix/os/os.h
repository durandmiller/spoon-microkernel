#ifndef _OS_H
#define _OS_H

#include <time.h>
#include <setjmp.h>
#include <sys/stat.h>

/** \defgroup OSFUNCS OS Dependent Functions 
 *
 * These are the OS specific functions which need to 
 * be implemented on any platform that the library
 * is expected to work on.
 */

/** @{
 */


#define 		NOTIMPLEMENTED			-1
#define			OK						0


#ifdef __cplusplus
extern "C" {
#endif

/** You will want to implement these stub functions in your
 * own OS subdirectory.
 *
 *
 * They should all take the form of:
 *
 *       int os_function( ... params ... );
 *
 * The macro NOTIMPLEMENTED should be returned by an
 * OS hook which is not supported or not implemented
 * on your platform. Otherwise, OK should be returned.
 *
 * If a parameter is required to be returned (example:
 * file descriptor), then it is usually returned as a 
 * parameter which was passed during the call.
 *
 */



/** Given the number of pages required, will return some memory
 * available for reading and writing. A page size is expected
 * to be 4096 bytes and to be aligned on a 4K bounday. The memory 
 * does not need to be cleaned but it is expected to be
 * writable.
 * 
 * \param pages The number of 4K-aligned 4096 byte pages required.
 * \param ptr The pointer in which the newly allocated memory will be returned.
 * \return OK on success. *ptr wil contain the allocated memory.
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int 	os_mem_alloc( int pages, void** ptr ); 

/** This function is expected to free the memory previously
 * allocated using os_mem_alloc. As you can see the size
 * and pages used by the memory is not returned so it's up 
 * to you or your OS to remember that kind of information
 * if it is required.
 *
 * \param mem A pointer to memory previously allocated by
 *            os_mem_alloc.
 * \param pages The number of pages to allocate
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_mem_free( void* mem, int pages );


/** os_acquire_spinlock should be a platform specific way 
 * of implementing spinlocks. According to the library, a 
 * spinlock is an unsigned int which can either be 
 * unlocked (0) or locked ( not 0 ). 
 *
 * This function should be atomic and the library should
 * be guaranteed that there is 1 and only 1 thread holding
 * a spinlock at any one time.
 *
 * The default implementation defined in the empty platform
 * should work on any x86 machine. 
 *
 * \note The default implementation is not quite a spinlock
 * in that it calls sched_yield() on each spin, to ensure
 * faster turnover time.
 *  
 * \param lock A pointer to an unsigned int.
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int 	os_acquire_spinlock( unsigned int *lock  );

/** os_release_spinlock should be a platform specific way 
 * of implementing spinlocks. According to the library, a 
 * spinlock is an unsigned int which can either be 
 * unlocked (0) or locked ( not 0 ). 
 *
 * This function should be atomic and the library should
 * be guaranteed that there is 1 and only 1 thread holding
 * a spinlock at any one time.
 *  
 * The default implementation defined in the empty platform
 * should work on any x86 machine.
 *  
 * \param lock A pointer to an unsigned int.
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */

int 	os_release_spinlock( unsigned int *lock  );


/** This function is supposed to set the value tid to the
 * thread ID (or tid) of the calling thread.
 *
 * \param tid A pointer to the tid which should be set
 * \param rc The return code of the operation
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_gettid( pthread_t *tid, int *rc );

/** This function is supposed to set the pid to the
 * process ID (or pid) of the calling application.
 *
 * \param pid A pointer to the pid which should be set
 * \param rc The return code of the operation
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_getpid( int *pid, int *rc );


/** Returns the current user id and group id of the current
 * application.
 *
 * \param uid The user id
 * \param gid The group id
 * \param rc The return code of the operation
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_getid( int *uid, int *gid, int *rc );



/** Returns the current working directory of the 
 * application. Honours everything that the standard
 * getcwd should.
 *
 *
 * \param buf The buffer in which to return the working directory
 * \param size The size of the buffer
 * \param rc The return code of the operation
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_getcwd( char *buf, unsigned int size, int *rc );


/** This function should return a random number, any number
 * between 0 and MAX_INT. 
 *
 * \param rand A pointer to an integer which will hold the random number. Range
 *        is from 0 to MAX_INT
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_rand( int *rand );


/** This function should put a NULL terminated string onto
 * the currently active console/screen. It is up to the
 * OS to handle the screen scrolling, cursor manipulation, etc
 * ... but really the library just cares that the string has
 * been displayed.
 *
 * \param str A NULL-terminated string.
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_puts( const char *str );


/** This is the file system open command, very much like the
 * standard UNIX open. Your function can open the file, create
 * a data structure in which to maintain it's information about
 * it and then it must return this structure in *data. This
 * value will be passed back to the OS functions in all
 * later file operations to maintain state, etc.
 *
 * Basically, *data is your's to build and do with as you please. You'll
 * be passed it back in future file operations.
 * 
 *
 * \param filename  A fully qualified filename.
 * \param flags The flags in which the file should be opened. These are
 *             	the same flags specified in the fcntl.h file - 
 *            	 O_RDWR, O_CREATE, etc.  
 *
 * \param mode The ownership bits with which to open the file.
 *             
 * \param data The os_open function must return a data structure
 *             in *data.  It doesn't matter what the structure looks
 *             like or what's in it, as long as there is something.
 *             It's only for your use in later file operations.
 *
 * \param fd The file descriptor returned by the open call or the error.
 * 
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_open( const char *filename, unsigned int flags, 
						unsigned int mode,
					void **data, int *fd );

/** Typical read function which copies at least len amount of
 * bytes into the given buffer. It is not supposed to return 
 * more than "len" specified amount of bytes.
 *
 * \param data The original data returned in the os_open function.
 * \param pos The position in the file to start reading from. 0 is the first byte
 * \param buffer A buffer into which the data should be copied.
 * \param len The maximum amount of bytes requested.
 * \param rc The pointer containing the return code.
 * 
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_read( void *data, unsigned long long pos, void *buffer, int len, int *rc );


/** Typical write function.
 *
 * \param data The original data returned in the os_open function.
 * \param pos The position in the file to start writing to. 0 is the first byte
 * \param buffer A buffer which contains the data to be written.
 * \param len The number of bytes to write.
 * \param rc The pointer containing the return code. Should be the number of bytes written.
 * 
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_write( void *data, unsigned long long pos, const void *buffer, int len, int *rc );

/** Closes the open file. The data value which your os_open
 * function returned should be released and free here. All
 * resources should be returned to the system. After
 * calling this function, the file is completely forgotten
 * by the library.
 *
 * \param data The original data returned in the os_open function.
 * \param rc The return code of the operation
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_close( void *data, int *rc );

/** This should return as much information as possible
 * with regards to the struct stat parameter.
 *
 * \param data The original data returned by os_open.
 * \param st A pointer to a struct stat which must
 *           hold the gathered data.
 *
 * \param rc The pointer containing the return code.
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_stat( void *data, struct stat *st, int *rc );


/** This function sets the umask of the environment.
 *
 * \param mask The unsigned integer
 * \param rc The pointer containing the return code.
 *
 * \return OK on success
 * \return NOTIMPLEMENTED on non-implementation
 */
int	os_umask( int mask, int *rc );

/** This function should modify the file mode for the
 * already open file.
 *
 * \param data The original data returned by os_open.
 * \param mode The requested mode which contains the
 *             flags as specified in fcntl
 *
 * \param rc The pointer containing the return code.
 *
 * \return OK on success
 * \return NOTIMPLEMENTED on non-implementation
 */
int 	os_chmod( void *data, mode_t mode, int *rc );


/** This function should modify the user id and the
 * group id for the file.
 *
 * \param data The original data returned by os_open.
 * \param uid The user id
 * \param gid The group id
 *
 * \param rc The pointer containing the return code.
 *
 * \return OK on success
 * \return NOTIMPLEMENTED on non-implementation
 */
int 	os_chown( void *data, uid_t uid, gid_t gid, int *rc );


/** Changes the working directory of the currently
 * executing application.
 *
 * \param data The original data returned by os_open
 * \param rc The pointer containing the return code.
 *
 * \return OK on success.
 * \return NOTIMPLEMENTED on non-implementation
 */
int		os_chdir( void *data, int *rc );


/** This is the standard mmap implementation which should map
 * the contents of the file starting at position offset
 * in the file to, preferably, the pointer specified as start.
 *
 * \param data The original data returned by os_open
 * \param start The preferred position in memory at which to map the file.
 * \param length The length of the file which should be mapped.
 * \param prot Protection flags. (See sys/mman.h)
 * \param flags The type of the mapped object. (see sys/mman.h)
 * \param offset The position in the file at which to start mapping.
 * \param rc The pointer containing the return code or error.
 * \param ptr The pointer at which the file was mapped, if successful.
 * 
 * \return OK on success
 * \return NOTIMPLEMENTED on non-implementation
 */
int		os_mmap( void* data, void *start, 
				    unsigned long long length, int prot, int flags,
					unsigned long long offset, 
					int *rc, void **ptr );

/** Unmaps the memory already mapped by a previous call to os_mmap.
 *
 * \param start The position in memory at which to start unmapping.
 * \param length The amount of memory to unmap.
 * \param rc The return code of the operation if implemented.
 *
 * \return OK on success
 * \return NOTIMPLEMENTED on non-implementation
 */
int		os_munmap(void *start, unsigned long long length, int *rc);


/** File operation which will delete the given filename.
 *
 * \param filename The fully qualified name of the file to delete.
 * \param rc The return code of the operation
 * 
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_delete( const char *filename, int *rc );

/** File operation which will create the directory with
 * the filename given. 
 *
 * \param filename The fully qualified name of the directory to create.
 * \param mode The mode that the directory will be created with.
 * \param rc The return code of the operation.
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_mkdir( const char *filename, unsigned int mode, int *rc );

/** File operation which will remove a directory with the
 * given filename.
 *
 * \param filename The fully qualified name of the directory to create.
 * \param rc The return code of the operation.
 * 
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_rmdir( const char *filename, int *rc );

/** Just like the standard unix command, this function must
 * return the amount of clock ticks used by the current 
 * application.  To get the number of seconds used by the 
 * application, divide the return value by CLOCKS_PER_SEC
 *
 * \param clock The number of clocks used by the processor.
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */ 
int		os_clock( clock_t *clock );

/** This returns the system time. It returns seconds and
 * milliseconds. This time is presumed to be the amount of
 * seconds and milliseconds since the standard UNIX epoch.
 *
 * (00:00:00 GMT, January 1, 1970)
 *
 * \param seconds The amount of seconds elapsed since the UNIX epoch.
 * \param milliseconds The remaining milliseconds since the last second.
 * \param rc The return code of the operation.
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int		os_system_time( time_t *seconds, time_t *milliseconds, int *rc );	

/** This function should put the calling thread to sleep for the
 * given amount of seconds.
 *
 * \param seconds The number of seconds to sleep.
 * \param rc The number of seconds actually slept
 *
 * \return OK on success. 
 * \return NOTIMPLEMENTED on non-implementation. 
 */
int 	os_sleep( unsigned int seconds, unsigned int *rc );

/** This will attempt to execute the command line given. The
 * command line is a full command with command line parameters. It's
 * up to your system to parse it, split parameters if necessary,
 * pre-launch, execute the application, etc.
 *
 * \return PID of spawned process on successful execution of the
 * command.
 * \return -1 if command failed.
 */
int		os_system( const char *s, int *rc );

/** OS specific exit routine. This should, theoretically, return
 * the status back to the parent application which launched this
 * one. When this function is called, all threads stop and the
 * application terminated.
 *
 * \return -1 if failure to exit ocurred.
 */
void 	os_exit( int status ) __attribute__ ((__noreturn__));


/** If your OS supports signalling, this is the signalling
 * hook into your OS. It's supposed to trigger a signal to
 * the provided pid.
 *
 * \param pid The pid of the process to which to send the signal.
 * \param sig The signal number to send.
 * \param rc The return code of the operation.
 *
 * \return 0 on successful signal.
 * \return NOTIMPLEMENTED on not being implemented.
 */
int		os_send_signal( int pid, int sig, int *rc);



/** Returns the environment variable setting identified
 * by the name parameter or NULL if the requested environment
 * variable does not exist.
 *
 * \return NULL if the environment variable does not exist.
 * \return The data of the environment variable.
 */
int 	os_getenv(const char* name, char **env );

/** This function should release control of the CPU and
 * timeslice so that any other process or thread may run.
 *
 * \param rc This should be set to 0 if the yield was successful. 
 * Otherwise, this should be the error code (err) that needs to
 * be set.
 */
int 	os_sched_yield( int *rc );


/** This is the only operation that needs to be implemented. 
 * It should know how to freak out on your system when an
 * operation that is required to me implemented is not
 * implemented.
 *
 * \param reason This is the reason for the freakout.
 *
 */
void	os_freakout( const char *reason ) __attribute__ ((__noreturn__));

#ifdef __cplusplus
}
#endif


/** @} 
 */

#endif

