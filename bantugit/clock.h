#ifndef __CLOCK_H
#define __CLOCK_H

#include <stdint.h>

namespace myos
{
	typedef uint32_t timestamp_t;

	timestamp_t now();

	extern "C" timestamp_t sys_getclock();
}

#endif //__CLOCK_H