#ifndef __LOCK_H
#define __LOCK_H

#include <stdint.h>

namespace myos
{
	class spinlock
	{
		uint32_t lock;
	public:
		spinlock();
		void acquire();
		void release();
		bool is_locked();
	};
}
#endif //__LOCK_H