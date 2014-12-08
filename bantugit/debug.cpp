#include "debug.h"
#include "terminal.h"


namespace myos
{
	namespace
	{
		int log_int_base = 10;
	}

	void set_log_int_base(int new_base)
	{
		log_int_base = new_base;
	}

	void log(alertlevel, const char *str)
	{
		terminal_writestring(str);
	}
	void log(alertlevel, bool boolval)
	{
		terminal_writebool(boolval);
	}
	void log(alertlevel, uint32_t N)
	{
		terminal_writenumber(N, log_int_base);
	}
	
	void log(alertlevel, int N)
	{
		terminal_writenumber(N, log_int_base);
	}
	//*/
	void log(alertlevel, char c)
	{
		terminal_putchar(c);
	}
}
