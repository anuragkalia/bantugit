#include "clock.h"
#include "timer.h"

namespace myos
{
	timestamp_t now()
	{
		return static_cast<timestamp_t>(get_global_clock());
	}

	extern "C" timestamp_t sys_getclock()
	{
		return now();
	}
}