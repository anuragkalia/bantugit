#ifndef __IRQ_H
#define __IRQ_H

#include "types_asm.h"
namespace myos
{
	typedef void(*irq_routine_ptr)(struct regs *r);

	void irq_install_handler(int irq, irq_routine_ptr handler);
	void irq_install();
}
#endif //__IRQ_H