#include "process.h"
#include "filesystem.h"
#include "debug.h"
#include "pcb.h"
#include "stdfunc.h"
#include "pqueue.h"
#include "process_asm.h"

namespace myos
{
	proc_data running;
	bool FOREGROUND_PROCESS_RUNNING = false;
	void split_file_string(const char *prog_name, char (*path)[80], int *argc, char (*argv)[ARGV_NUMSTR][ARGV_STRSZ])
	{
		int i = 0;
		while (prog_name[i] != '\0' && isspace(prog_name[i]))
		{
			i++;
		}

		int j = 0;
		while (prog_name[i] != '\0' && !isspace(prog_name[i]))
		{
			(*path)[j++] = prog_name[i++];
		}
		(*path)[j] = '\0';

		int ac = 0;

		for (size_t k = 0; k < ARGV_NUMSTR; k++)
		{
			j = 0;

			while (prog_name[i] != '\0' && isspace(prog_name[i]))
			{
				i++;
			}

			if (prog_name[i] == '\0') break;

			while (prog_name[i] != '\0' && !isspace(prog_name[i]))
			{
				(*argv)[k][j++] = prog_name[i++];
			}
			(*argv)[k][j] = '\0';
			ac++;

			*argc = ac;
		}
	}
	myos::PCB_table & all_pcb()
	{
		static myos::PCB_table all_pcb;
		//all_pcb.print_addresses();

		return all_pcb;
	};

	bool arrival_time_comp(pid_t pid1, pid_t pid2)
	{
		//magic_breakpoint_msg("In less_arr_time()\n");
		bool res = (all_pcb()[pid1].arrival_time < all_pcb()[pid2].arrival_time);
		//magic_breakpoint_msg("Out less_arr_time()\n");
		return res;
	}

	bool rev_arrival_time_comp(pid_t pid1, pid_t pid2)
	{
		//magic_breakpoint_msg("In less_arr_time()\n");
		bool res = (all_pcb()[pid1].arrival_time > all_pcb()[pid2].arrival_time);
		//magic_breakpoint_msg("Out less_arr_time()\n");
		return res;
	}

	pqueue<pid_t, 256> & cpu_ready_queue()
	{
		static pqueue<pid_t, 256> cpu_ready_queue(rev_arrival_time_comp);
		return cpu_ready_queue;
	}

	void run_program()
	{

		//log(INFO, "In run program \n");
		pid_t next_proc_id = cpu_ready_queue().dequeue();

		//log(INFO, "next process = ", next_proc_id, '\n');

		if (next_proc_id == NULL_PID)
		{
			running.pid = NULL_PID;
			return;
		}
		else
		{
			running.pid = next_proc_id;
		}

		//log(OK, "PCB entry for next_proc_id = ", next_proc_id, ": \n");
		//all_pcb().print_PCB_entry(next_proc_id);

		all_pcb().change_state(next_proc_id, RUNNING);
		physical_addr start_proc = all_pcb()[next_proc_id].code_base;

		log(OK, "Now, we will run process with pid = ", next_proc_id, '\n');

		cpu_ready_queue().print();

		//timestamp_t beg = now();
		bool is_ok = call_process_at(start_proc);
		//timestamp_t end = now();

		//all_pcb().add_cpu_burst_time(next_proc_id, beg - end);

		all_pcb().change_state(next_proc_id, TERMINATED);

		all_pcb().delete_proc(next_proc_id);

		if (is_ok != 0)
		{
			log(INFO, "Process ", running.pid, " encountered an error\n");
		}
		else
		{
			log(INFO, "Process ", running.pid, " ran successfully\n");
		}

		running.pid = NULL_PID;

		return;

	}

	bool add_program(const char *prog_name)
	{
		char path[80];
		char argv[ARGV_NUMSTR][ARGV_STRSZ];
		int ac = 0;


		//log(OK, "prog_name initially is <", prog_name, ">\n");

		split_file_string(prog_name, &path, &ac, &argv);
		/*
		log(OK, "path : ", path, '\n');
		for (int i = 0; i < ac; i++)
		{
			log(OK, "argv[", i, "] : ", argv[i], '\n');
		}
		//*/
		if (is_file_exists(path) == false)
		{
			log(ERROR, "path ", path, " does not exist\n");
			return false;
		}
		else
		{

			pid_t newprocid = all_pcb().insert_proc(path, ac, argv, NORMAL);

			if (newprocid == NULL_PID)
			{
				log(ERROR, "Insertion in PCB table was unsuccessful\n");
				return false;
			}
			else
			{
				log(OK, "new process ", path, " has pid = ", newprocid, '\n');

				bool is_ok = cpu_ready_queue().enqueue(newprocid);

				cpu_ready_queue().print();

				return is_ok;
			}
		}
	}
}