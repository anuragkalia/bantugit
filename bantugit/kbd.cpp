/* Handles the keyboard interrupt */
#include "kbd.h"
#include "irq.h"
#include "lock.h"
#include "io_asm.h"
#include "debug.h"

namespace myos
{
	const uint16_t MAX_KBD_BUF_SIZE = 256;

	spinlock buf_size_lock;
	uint8_t kbd_buffer[MAX_KBD_BUF_SIZE];
	
	uint16_t buf_back, buf_front, buf_size;

	uint8_t kbdus[128] =
	{
		0, 27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
		'9', '0', '-', '=', '\b',	/* Backspace */
		'\t',			/* Tab */
		'q', 'w', 'e', 'r',	/* 19 */
		't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
		0,			/* 29   - Control */
		'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
		'\'', '`', 0,		/* Left shift */
		'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
		'm', ',', '.', '/', 0,				/* Right shift */
		'*',
		0,	/* Alt */
		' ',	/* Space bar */
		0,	/* Caps lock */
		0,	/* 59 - F1 key ... > */
		0, 0, 0, 0, 0, 0, 0, 0,
		0,	/* < ... F10 */
		0,	/* 69 - Num lock*/
		0,	/* Scroll Lock */
		0,	/* Home key */
		0,	/* Up Arrow */
		0,	/* Page Up */
		'-',
		0,	/* Left Arrow */
		0,
		0,	/* Right Arrow */
		'+',
		0,	/* 79 - End key*/
		0,	/* Down Arrow */
		0,	/* Page Down */
		0,	/* Insert Key */
		0,	/* Delete Key */
		0, 0, 0,
		0,	/* F11 Key */
		0,	/* F12 Key */
		0,	/* All other keys are undefined */
	};

	void inc_buf_size()
	{
		buf_size_lock.acquire();
		buf_size++;
		buf_size_lock.release();
	}
	void dec_buf_size()
	{

		buf_size_lock.acquire();
		buf_size--;
		buf_size_lock.release();
	}

	bool is_overflow()
	{
		return buf_size == MAX_KBD_BUF_SIZE;
	}

	bool is_underflow()
	{
		return buf_size == 0;
	}

	void put_in_kbd_buf(char c)
	{
		kbd_buffer[buf_back] = c;

		if (++buf_back == MAX_KBD_BUF_SIZE)
		{
			buf_back = 0;
		}
		inc_buf_size();
	}

	char get_from_kbd_buf()
	{
		//magic_breakpoint_msg("In get_from_kbd_buf: buf_back = ", buf_back, ", buf_front = ", buf_front, ", buf_size = ", buf_size, '\n');
		char c = kbd_buffer[buf_front];

		if (++buf_front == MAX_KBD_BUF_SIZE)
		{
			buf_front = 0;
		}
		dec_buf_size();
		//magic_breakpoint_msg("Out get_from_kbd_buf: buf_back = ", buf_back, ", buf_front = ", buf_front, ", buf_size = ", buf_size, '\n');
		return c;
	}

	void keyboard_handler(struct regs *)
	{
		//magic_breakpoint_msg("In keyboard_handler()\n");
		unsigned char scancode;
		static bool flag = 0;
		char inchar = -2;
		/* Read from the keyboard's data buffer */
		scancode = inportb(0x60);

		/* If the top bit of the byte we read from the keyboard is
		*  set, that means that a key has just been released */
		if (!(scancode ^ 0x3A) || !(scancode ^ 0xBA))
		{
			flag = flag ^ 0x01;
		}
		else if (scancode ^ 0x3A && !(scancode & 0x80))
		{
			if (flag == 1)
			{
				inchar = kbdus[scancode] - 32;
			}
			else
			{
				inchar = kbdus[scancode];
			}
		}

		if (inchar != -2)
		{
			if (!is_overflow())
			{
				put_in_kbd_buf(inchar);
			}
		}

		//magic_breakpoint_msg("Out keyboard_handler()\n");
	}

	void kbd_install()
	{
		irq_install_handler(1, keyboard_handler);
	}
}