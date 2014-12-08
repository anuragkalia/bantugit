#include "lock.h"
#include "lock_asm.h"
#include "debug.h"

namespace myos
{
	spinlock::spinlock() : lock(0)
	{ }

	void spinlock::acquire()
	{
		//magic_breakpoint_msg("Acquiring lock...\n");
		asm_acquire_lock(&lock);
		//magic_breakpoint_msg("Acquired lock\n");
	}

	void spinlock::release()
	{
		//magic_breakpoint_msg("Releasing lock...\n");
		asm_release_lock(&lock);
		//magic_breakpoint_msg("Released lock\n");
	}

	bool spinlock::is_locked()
	{
		if (&lock == 0)
			return false;
		else
			return true;
	}
}