#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

namespace myos
{
	void timer_install();
	uint32_t get_global_timer();
	uint32_t get_global_clock();
}
#endif //__TIMER_H