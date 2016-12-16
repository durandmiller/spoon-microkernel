#ifndef _OS_H
#define _OS_H


#ifdef __cplusplus
extern "C" {
#endif


/** This function is supposed to lock the memory data structures. It
 * could be as simple as disabling interrupts or acquiring a spinlock.
 * It's up to you to decide. 
 *
 * \return 0 if the lock was acquired successfully. Anything else is
 * failure.
 */
extern int os_lock();

/** This function unlocks what was previously locked by the os_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
extern int os_unlock();

/** This is the hook into the local system which arbates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the os_init function.
 *
 * \return NULL if the pages were not arbated.
 * \return A pointer to the arbated memory.
 */
extern void* os_alloc(size_t);

/** This frees previously arbated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * os_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
extern int os_free(void*,size_t);


       

#endif


