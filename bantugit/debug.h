#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

extern "C" void magic_breakpoint();

namespace myos
{
	enum alertlevel {OK, INFO, WARNING, ERROR, FAULT, BREAKPOINT};
	
	

	void set_log_int_base(int base);
	void log(alertlevel, const char *str);
	void log(alertlevel, bool boolval);
	void log(alertlevel, uint32_t N);
	void log(alertlevel, int N);
	void log(alertlevel, char c);

	template < class T, class V, class ... U>
	void log(alertlevel LVL, T arg1, V arg2, U ...rest)
	{
		log(LVL, arg1);
		log(LVL, arg2, rest...);
	}

	template< class ...T>
	void magic_breakpoint_msg(T... args)
	{
		log(BREAKPOINT, args...);
		magic_breakpoint();
	}
}

#endif //__DEBUG_H