#include "init.h"

#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "io_asm.h"
#include "timer.h"
#include "kbd.h"

#include "mmngr_phys.h"

#include "filesystem.h"
#include "debug.h"


//chori ka code START
int __cxa_guard_acquire(__guard *g) { return !*(char *) (g); };
void __cxa_guard_release(__guard *g) { *(char *) g = 1; };
//chori ka code END

extern "C" uint32_t end;

namespace myos
{
	void system_init(multiboot_info_t *mbd)
	{
		//magic_breakpoint_msg("Out: system_init\n");

		terminal_init();
		gdt_install();
		idt_install();
		isrs_install();
		irq_install();

		kbd_install();
		timer_install();

		setinterrupt(); // <- move to the end to signal end of initializing and start of computer functioning

		
		//magic_breakpoint_msg("Memory mgmt starts\n");

		memory_map_t * mmap = (memory_map_t*) mbd->mmap_addr;
		uint32_t memsize = mbd->mem_upper + mbd->mem_lower;
		void *endofkernel = (uint32_t*) &end;

		//terminal_writenumber(mbd->mem_lower,10);
		//terminal_writestring("yolo");
		//terminal_writenumber(mbd->mem_upper,10);
		//*/
		/*
		terminal_writestring("\nKernel ends at");
		terminal_writenumber((uint16_t(reinterpret_cast<uint32_t>(endofkernel) >> 16)), 16);
		terminal_writenumber(uint16_t(reinterpret_cast<uint32_t>(endofkernel)), 16);
		terminal_writestring("\nSize of memory");
		terminal_writenumber((uint16_t(memsize >> 16)), 16);
		terminal_writenumber(uint16_t(memsize), 16);
		//*/
		
		pmmngr_init(memsize, (uint32_t) endofkernel);


		while ((unsigned int) mmap < mbd->mmap_addr + mbd->mmap_length)
		{
			//log(OK,"/n Memorey map type:",mmap->type,"base addr lo :",mmap->base_addr_low,"length low :",mmap->length_low); 
			if (mmap->type == 1)//can be used
			{
				pmmngr_init_region(mmap->base_addr_low, mmap->length_low);
			}
			mmap = (memory_map_t*) ((unsigned int) mmap + mmap->size + sizeof(unsigned int));
		}
		//deinintialize region where kernel it as it cant be used
		pmmngr_deinit_region(0x100000, (uint32_t) endofkernel);
		//*/
		/*
		terminal_writestring("\n Allocation blocks:");
		terminal_writenumber(pmmngr_get_block_count(), 10);


		terminal_writestring("\n Used or reserved blocks:");
		terminal_writenumber(pmmngr_get_use_block_count(), 10);

		terminal_writestring("\n free blocks: ");
		terminal_writenumber(pmmngr_get_free_block_count(), 10);
		//*/

		//uint32_t *p = (uint32_t*) pmmngr_alloc_block();
		//uint32_t *q = (uint32_t*) pmmngr_alloc_block();

		//log(INFO, "p allocated at ", uint32_t(p), '\n');
		/*
		set_log_int_base(16);
		log(INFO, "p = ", uint32_t(p), '\n');
		log(INFO, "q = ", uint32_t(q), '\n');
		set_log_int_base(10);
		//*/
		

		//magic_breakpoint_msg("Memory Mgmt is done\n");
		//*/
		
		//magic_breakpoint_msg("Filesystem init starts \n");
		format();
		fs_init();
		//*/

		magic_breakpoint_msg("Out: system_init\n");
	}
}