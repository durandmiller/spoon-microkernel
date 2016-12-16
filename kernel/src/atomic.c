#include "include/atomic.h"



spinlock_t kernel_lock = INIT_SPINLOCK;
volatile unsigned int catch_deadlocks = 0;	// For catching deadlocks




