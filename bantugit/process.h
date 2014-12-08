#ifndef __PROCESS_H
#define __PROCESS_H

#include "pcb.h"

namespace myos
{
	extern 	bool FOREGROUND_PROCESS_RUNNING;
	bool add_program(const char *prog_name);
	void run_program();

	struct proc_data
	{
		pid_t pid;
		timestamp_t quantum_end;
	};

	extern proc_data running;
}
#endif //__PROCESS_H