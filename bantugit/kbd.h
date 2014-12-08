#ifndef __KBD_H
#define __KBD_H

#include <stdint.h>

namespace myos
{
	void kbd_install();
	
	void inc_buf_size();
	void dec_buf_size();
	bool is_overflow();
	bool is_underflow();
	void put_in_kbd_buf();
	char get_from_kbd_buf();
}
#endif //__KBD_H