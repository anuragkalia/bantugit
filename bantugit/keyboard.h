#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stddef.h>
namespace myos
{
	char kbd_getchar();
	void kbd_readstring(char *buf, size_t buf_sz);

	extern "C" char sys_getchar();

	bool pending_keystrokes();
}
#endif //__KEYBOARD_H