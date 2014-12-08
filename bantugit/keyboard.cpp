#include "keyboard.h"
#include "kbd.h"
#include "lock.h"
#include "stdfunc.h"
#include "debug.h"

namespace myos
{
	spinlock kbd_get_lock;

	char kbd_getchar()
	{
		//log(INFO, "In kbd_char()\n");
		kbd_get_lock.acquire();
		
		while (is_underflow())
			; //empty loop

		char c = get_from_kbd_buf();
		kbd_get_lock.release();

		//log(INFO, "Out kbd_char()\n");
		return c;
	}

	extern "C" char sys_getchar()
	{
		return kbd_getchar();
	}

	bool pending_keystrokes()
	{
		return !is_underflow();
	}

	void kbd_readstring(char *buf, size_t buf_sz)
	{
		kbd_get_lock.acquire();

		for (size_t i = 0; i < buf_sz; i++)
		{
			while (is_underflow())
				; //empty loop

			char c = get_from_kbd_buf();

			if (isspace(c))
			{
				buf[i] = '\0';
				break;
			}
			else
			{
				buf[i] = c;
			}
		}
		kbd_get_lock.release();
	}
}