#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>

namespace myos
{
	/*In idt.cpp*/
	
	void idt_install();
	void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
}

#endif //_IDT_H