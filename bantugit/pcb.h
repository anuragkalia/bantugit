#ifndef __PCB_H
#define __PCB_H

#include "clock.h"
#include "mmngr_phys.h"

namespace myos
{
	const size_t ARGV_STRSZ = 32;
	const size_t ARGV_NUMSTR = 8;

	typedef int16_t pid_t;
	typedef uint32_t register_t;

	const pid_t NULL_PID = 0;

	enum PROC_STATE { TERMINATED = 0, NEW, READY, RUNNING, WAITING };
	enum PRIO_TYPE { NORMAL };

	class PCB_table;
	
	struct  all_registers
	{
		register_t EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP, EIP, EFLAGS;
	};

	struct PCB_struct
	{
		//process memory data
		physical_addr header_base;
		physical_addr code_base;
		size_t code_size;
		physical_addr stack_base;

		//process registers
		all_registers p_regs;

		//timestamps
		uint32_t total_time;
		uint32_t last_cpu_burst_time;
		timestamp_t arrival_time;

		//other info
		pid_t pid;
		PROC_STATE p_currently;
		PRIO_TYPE priority;

		PCB_struct(frame_t *frame_base, int num_frames, all_registers regs, pid_t free_pid, PRIO_TYPE priority);
		PCB_struct();

		friend class PCB_table;
	};

	class PCB_table
	{
		PCB_struct table[256];
		size_t table_size;
		uint32_t bitmap;

		int find_free_pcbslot() const;

	public:
		void print_addresses() const;
		void print_PCB_entry(int i) const;
		PCB_table();
		const PCB_struct & operator[] (int i);
		void set_bitmap(pid_t pid);
		void clr_bitmap(pid_t pid);

		pid_t insert_proc(const char *filename, const size_t ac, const char av[ARGV_NUMSTR][ARGV_STRSZ], PRIO_TYPE prio);
		void delete_proc(pid_t pid);
		void change_state(pid_t pid, PROC_STATE s);
		void add_cpu_burst_time(pid_t pid, uint32_t period);
	};

}
#endif //__PCB_H