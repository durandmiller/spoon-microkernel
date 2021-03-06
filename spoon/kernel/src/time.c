#include <smk/inttypes.h>
#include "include/assert.h"
#include "include/atomic.h"
#include "include/io.h"
#include "include/dmesg.h"
#include "include/cpu.h"
#include "include/time.h"
#include "include/div.h"



uint64_t 	boot_time;
spinlock_t	time_lock		=	INIT_SPINLOCK;


// -----------------------------------------------------------------
// ******  TIMING CODE *********************************************  



// From Ralf Brown's list - PIT mode control word - 0x43

#define COUNTER_2				(1<<7)
#define RW0_15					(3<<4)
#define MODE_ZERO_DETECTION		0
#define BINARY_COUNTER			0

// Keyboard Controller Flags

#define KBD_SPEAKER_DATA_STATUS		(1<<1)
#define KBD_CLOCK_GATE_TO_SPEAKER	(1<<0)
#define KBD_TIMER2_CONDITION		(1<<5)

// PIT defines and variables

#define HZ			100
#define PIT_HZ		1193180			// PIT clock HZ

#define START_COUNT 	( 5 * PIT_HZ / HZ )
#define COUNT_TIME 		( 5 * 1000000 / HZ )

static unsigned long calculate_bogomips()
{
	uint64_t diff, datime;
	uint32_t rem, temp;
	unsigned long count = 0;

	// Turn the speaker off and clear status
	outb( 0x61, (inb(0x61) & ~(KBD_SPEAKER_DATA_STATUS) ) | KBD_CLOCK_GATE_TO_SPEAKER );
	outb( 0x43, (COUNTER_2 | RW0_15 | MODE_ZERO_DETECTION | BINARY_COUNTER) );
	outb( 0x42, (START_COUNT & 0xff) );
	outb( 0x42, (START_COUNT >> 8) & 0xff );

	// Wait until timer 2 fires
	diff = rdtsc();
	while ( ( inb( 0x61 ) & KBD_TIMER2_CONDITION ) == 0 ) count++;
	diff = rdtsc() - diff;
	  
	if ( count <= 1 ) return 0;
	if ( (diff > (1ull<<32)) || ( diff <= COUNT_TIME) ) return 0;
							// Ensure division won't cause exception.

		// Scale to preserve accuracy
	datime = COUNT_TIME;
	datime = datime << 32;

	temp = udiv( datime, (uint32_t)(diff & 0xFFFFFFFF), &rem );
	return udiv( (1000000ull <<32), temp, &rem ) / 1000;
}    


// --------------------------------------------------------------------


static time_t get_rtc_seconds()
{
  uint32_t seconds, minutes, hours;

	  do  // make sure UIP is not set. (bit 7, timer is being updated)
	  {
	    outb( 0x70, 0xA );
	  } while ( (inb(0x71) & (1<<7)) != 0);
	
	
	  // check CMOS
	
	  outb( 0x70, 0x0);
	  seconds = inb( 0x71 );
	
	  outb( 0x70, 0x2);
	  minutes = inb( 0x71 );
	
	  outb( 0x70, 0x4 );
	  hours = inb( 0x71 );
	
	    hours = (hours   >> 4) * 10 + (hours & 0xF);
	  seconds = (seconds >> 4) * 10 + (seconds & 0xF);
	  minutes = (minutes >> 4) * 10 + (minutes & 0xF);  
	  
  return (hours * 60 * 60 + minutes * 60 + seconds);
}


static void kinda_disable_pit( int hertz )
{
	uint32_t bob = PIT_HZ / hertz;

	// Set the timer to one-shot countdown
	outb_p( 0x43, 0x30 );
	outb_p( 0x40, (bob & 0xFF) );
	outb_p( 0x40, ((bob >> 8) & 0xFF) );
}


void init_time()
{
	assert( time_lock.lock == 0 );
	boot_time = get_rtc_seconds() * 1000;
	dmesg("boot time: %i\n", ((uint32_t)boot_time)/1000 );

	kinda_disable_pit( 100 );
}


void load_time( unsigned int cpu_id )
{
	acquire_spinlock( &(time_lock) );
		unsigned int cpu_speed = calculate_bogomips();
	release_spinlock( &(time_lock) );

	cpu[ cpu_id ].bogomips = cpu_speed;

	dmesg("CPU %i: %i MHz, %i operations per second\n", 
				   cpu_id, cpu_speed / 1000, cpu_speed );
}



