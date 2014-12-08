#ifndef __ASM_H
#define __ASM_H

#include <stdint.h>

namespace myos
{
	typedef uint16_t port_t;
	extern "C" uint8_t inportb(port_t port);
	extern "C" uint16_t inportw(port_t port);
	extern "C" uint32_t inportl(port_t port);
	extern "C" void outportb(port_t port, uint8_t data);
	extern "C" void outportw(port_t port, uint16_t data);
	extern "C" void outportl(port_t port, uint32_t data);

	extern "C" void rep_inportw(port_t port, uint16_t *buf, int count);
	extern "C" void rep_outportw(port_t port, uint16_t *buf, int count);

	extern "C" void setinterrupt();
}

#endif //__ASM_H