#include "pcb.h"
#include "stdfunc.h"
#include "clock.h"
#include "filesystem.h"
#include "debug.h"

namespace myos
{
	void print_reg_values(all_registers regs)
	{
		set_log_int_base(16);
		log(OK,
			"EAX", regs.EAX, ", ",
			"EBX", regs.EBX, ", ",
			"ECX", regs.ECX, ", ",
			"EDX", regs.EDX, ", ",
			"ESI", regs.ESI, ", ",
			"EDI", regs.EDI, ", ",
			"EBP", regs.EBP, ", ",
			"ESP", regs.ESP, ", ",
			"EIP", regs.EIP, ", ",
			"EFLAGS", regs.EFLAGS, '\n'
		);
		set_log_int_base(10);
	}

	PCB_struct::PCB_struct(frame_t *frame_base, int num_frames, all_registers regs, pid_t free_pid, PRIO_TYPE prio)
	{
		header_base = (physical_addr) frame_base;
		code_base = header_base + PMMNGR_BLOCK_SIZE;
		code_size = num_frames - 2;
		stack_base = code_base + PMMNGR_BLOCK_SIZE * code_size;

		p_regs = regs;
		total_time = 0;
		last_cpu_burst_time = 0;
		arrival_time = now();

		pid = free_pid;
		priority = prio;
		p_currently = NEW;
	}


	PCB_struct::PCB_struct()
		: p_currently(TERMINATED)
	{ }

	void PCB_table::set_bitmap(pid_t pid)
	{
		bitmap |= 0x1 << pid;
	}

	void PCB_table::clr_bitmap(pid_t pid)
	{

		bitmap &= ~(0x1 << pid);
	}

	PCB_table::PCB_table()
		: table_size(0), bitmap(0)
	{ 
		set_bitmap(NULL_PID);
	}

	pid_t PCB_table::insert_proc(const char *filename, const size_t ac, const char av[ARGV_NUMSTR][ARGV_STRSZ], PRIO_TYPE prio)
	{
		//magic_breakpoint_msg("In PCB_table::insert_proc\n");

		if (!is_file_exists(filename))
		{
			log(ERROR, "bad filepath\n");
			return NULL_PID;
		}

		if (ac > ARGV_NUMSTR)
		{
			log(ERROR, "number of cmdline vars exceed specified limit\n");
			return NULL_PID;
		}
		
		for (size_t i = 0; i < ac; i++)
		{
			if (strlen(av[i]) > ARGV_STRSZ)
			{
				log(ERROR, i, "the variable exceeds specified length of cmdline var\n");
				return NULL_PID;
			}
		}

		pid_t in_slot = find_free_pcbslot();
		//magic_breakpoint_msg("Back in PCB_table::insert_proc after getting find_free_slot()\n");

		if (in_slot == NULL_PID)
		{
			magic_breakpoint_msg("no free slot in PCB_table\n");
			//magic_breakpoint_msg("Out PCB_table::insert_proc\n");
			return NULL_PID;
		}
		
		int file_size = get_file_size(filename);

		int code_size = file_size / PMMNGR_BLOCK_SIZE + (file_size % PMMNGR_BLOCK_SIZE > 0) ? 1 : 0;

		frame_t * allocated_area = pmmngr_alloc_blocks(code_size + 2);

		if (allocated_area == 0)
		{
			log(ERROR, "contiguous chunk of ", code_size + 2, " frames could not be provided\n");
			return NULL_PID;
		}

		//we have area in memory as well as free slot in PCB_table: run with it!

		//set cmdline args
		for (size_t i = 0; i < ac; i++)
		{
			strncpy(allocated_area[0].data + ARGV_NUMSTR * i, ARGV_NUMSTR, av[i], '\0');
		}
		
		
		//magic_breakpoint_msg("assigned frame at ", uint32_t(allocated_area), '\n');

		*(uint32_t *) (allocated_area[1].data - 8) = ac;
		*(uint32_t *) (allocated_area[1].data - 4) = uint32_t(allocated_area);

		//magic_breakpoint_msg("process header frame is set\n");

		read_from_file(filename, allocated_area[1].data, code_size * PMMNGR_BLOCK_SIZE);
		//magic_breakpoint_msg("process code frames are set\n");

		memset(allocated_area[code_size + 1].data, '\0', sizeof(frame_t));
		//magic_breakpoint_msg("process stack frame is set \n");


		all_registers regs;
		regs.EAX = 0;
		regs.EBX = 0;
		regs.ECX = 0;
		regs.EDX = 0;
		regs.ESI = 0;
		regs.EDI = 0;
		regs.EBP = 0;
		regs.ESP = uint32_t(allocated_area + code_size + 2);
		regs.EIP = uint32_t(allocated_area + 1);
		regs.EFLAGS = 0;



		//magic_breakpoint_msg("Setting PCB entry in pcb table - \n");
		table[in_slot] = PCB_struct(allocated_area, code_size + 2, regs, in_slot, prio);

		
		set_bitmap(in_slot);

		//magic_breakpoint_msg("Out PCB_table::insert_proc\n");
		return in_slot;
		
	}


	void PCB_table::change_state(pid_t pid, PROC_STATE s)
	{

		table[pid].p_currently = s;
	}

	void PCB_table::delete_proc(pid_t pid)
	{
		pmmngr_free_blocks((frame_t *)table[pid].header_base, 1);
		pmmngr_free_blocks((frame_t *)table[pid].code_base, table[pid].code_size);
		pmmngr_free_blocks((frame_t *)table[pid].stack_base, 1);

		clr_bitmap(pid);
		change_state(pid, TERMINATED);		
	}


	int PCB_table::find_free_pcbslot() const
	{
		//magic_breakpoint_msg("In find_free_pcbslot()\n");
		if (bitmap == uint32_t(0xFFFFFFFF))
		{
			magic_breakpoint_msg("Out find_free_pcbslot() - error\n");
			return 256;
		}
		else
		{
			int freeslot = 0;

			//magic_breakpoint_msg("Before loop of finding slot\n");

			for (freeslot = 0; freeslot < 32; freeslot++)
			{
				if ((bitmap & (0x1 << freeslot)) == 0)
				{
					break;
				}
			}

			return freeslot;
		}
	}

	const PCB_struct & PCB_table::operator[] (int i)
	{
		return table[i];
	}

	const char * print_priority(PRIO_TYPE P)
	{
		switch (P)
		{
		case NORMAL:
			return "NORMAL";
		default:
			return "INVALID PRIORITY";
		}
	}

	const char * print_state(PROC_STATE S)
	{
		switch (S)
		{
		case PROC_STATE::NEW: 
			return "NEW";
		case PROC_STATE::READY:
			return "READY";
		case PROC_STATE::RUNNING:
			return "RUNNING";
		case PROC_STATE::TERMINATED:
			return "TERMINATED";
		case PROC_STATE::WAITING:
			return "WAITING";
		default:
			return "INVALID STATE";
		}
	}


	void PCB_table::print_PCB_entry(int i) const
	{
		set_log_int_base(16);
		log(INFO, "& PCB_table[", i, "] = ", (uint32_t)( table + i), '\n');
		log(INFO, "header_base = ", table[i].header_base, '\n');
		log(INFO, "code base = ", table[i].code_base, '\n');
		log(INFO, "stack base = ", table[i].stack_base, '\n');
		set_log_int_base(10);
		log(INFO, "code size = ", table[i].code_size, '\n');
		//process registers
		print_reg_values(table[i].p_regs);

		//timestamps
		log(INFO, "total time = ", table[i].total_time, '\n');
		log(INFO, "cpu burst time = ", table[i].last_cpu_burst_time, '\n');
		log(INFO, "arrival time = ", table[i].arrival_time, '\n');

		//other info
		log(INFO, "pid = ", table[i].pid, '\n');
		log(INFO, "current state = ", print_state(table[i].p_currently), '\n');
		log(INFO, "priority = ", print_priority(table[i].priority), '\n');

	}

	void PCB_table::print_addresses() const
	{
		set_log_int_base(16);
		log(INFO, "Address of PCB object is : ", (uint32_t) this, '\n');
		log(INFO, "Address of PCB::bitmap is : ", (uint32_t) &this->bitmap, '\n');
		log(INFO, "Address of PCB::table is : ", (uint32_t) &this->table, '\n');
		log(INFO, "Address of PCB::table_size is : ", (uint32_t) &this->table_size, '\n');
		set_log_int_base(10);
	}

	void PCB_table::add_cpu_burst_time(pid_t pid, uint32_t period)
	{
		table[pid].last_cpu_burst_time = period;
		table[pid].total_time += period;
	}
}