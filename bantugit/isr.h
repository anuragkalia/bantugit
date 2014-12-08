#ifndef _ISRS_H
#define _ISRS_H

#include "types_asm.h"

namespace myos
{
	void isrs_install();
	extern "C" void fault_handler(struct regs *r);
}

#endif //_ISRS_H