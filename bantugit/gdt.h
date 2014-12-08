#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>

namespace myos
{
	/*In boot.s*/
	extern "C" uint32_t gdtptr();
	extern "C" void gdt_flush();

	/*In gdt.cpp*/
	void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
	void gdt_install();
}

#endif //_GDT_H