#ifndef __INIT_H
#define __INIT_H

#include "multiboot.h"

 typedef int __guard;

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release(__guard *);
//extern "C" void __cxa_guard_abort(__guard *);

namespace myos
{
	void system_init(multiboot_info_t *mbd);
}

#endif //__INIT_H