#include "timer.h"
#include "irq.h"
#include "lock.h"
#include "debug.h"
#include "process.h"
#include "new_shell.h"

namespace myos
{
	uint32_t GLOBAL_TIMER = 0;
	spinlock timer_lock;

	uint32_t GLOBAL_CLOCK = 0;
	spinlock clock_lock;

	/* This will keep track of how many ticks that the system
	*  has been running for */

	/* Handles the timer. In this case, it's very simple: We
	*  increment the 'timer_ticks' variable every time the
	*  timer fires. By default, the timer fires 18.222 times
	*  per second. Why 18.222Hz? Some engineer at IBM must've
	*  been smoking something funky */

	void inc_global_timer()
	{
		timer_lock.acquire();
		GLOBAL_TIMER++;
		timer_lock.release();
	}

	uint32_t get_global_timer()
	{
		timer_lock.acquire();
		uint32_t val = GLOBAL_TIMER;
		timer_lock.release();
		return val;
	}

	void inc_global_clock()
	{
		//magic_breakpoint_msg("In inc_global_clock()\n");
		clock_lock.acquire();
		GLOBAL_CLOCK++;
		clock_lock.release();
		//magic_breakpoint_msg("Out inc_global_clock()\n");
	}

	uint32_t get_global_clock()
	{
		clock_lock.acquire();
		uint32_t val = GLOBAL_CLOCK;
		clock_lock.release();
		return val;
	}

	void timer_handler(regs *)
	{
		//log(OK, "In timer handler\n");
		/* Increment our 'tick count' */
		inc_global_timer();

		/* Every 18 clocks (approximately 1 second), we will
		*  display a message on the screen */

		if (GLOBAL_TIMER % 18 == 0) //are division operations allowed?? :O
		{
			inc_global_clock();
		}

		if (SHELL_RUNNING == true && FOREGROUND_PROCESS_RUNNING == false)
		{
			//log(INFO, "shell runniong = true && fgnd running = false\n");
			if (get_global_timer() % 6 == 0)
			{
				//log(INFO, "Time for checking kbd input \n");
				//check for shell input
				xfer_to_shell_buffer();
				extract_programs_from_shell_buffer();
				//xferred_time = get_global_timer();
			}
			
			if (running.pid == NULL_PID)
			{
				//log(INFO, "run something \n");
				run_program();
			}
			//*/
		}
		//log(OK, "Out timer handler\n");
	}

	/* Sets up the system clock by installing the timer handler
	*  into IRQ0 */
	void timer_install()
	{
		GLOBAL_TIMER = 0;
		/* Installs 'timer_handler' to IRQ0 */
		irq_install_handler(0, timer_handler);
	}

}