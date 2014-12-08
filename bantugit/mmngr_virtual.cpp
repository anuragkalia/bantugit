#include "mmngr_virtual.h"
#include "mmngr_phys.h"
#include "paging_asm.h"
#include "stdfunc.h"
#include "debug.h"

namespace myos
{
	//! page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000

	//! directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

	//! page sizes are 4k
#define PAGE_SIZE 4096

	//! current directory table
	pdirectory *	_cur_directory = 0;

	//! current page directory base register
	physical_addr	_cur_pdbr = 0;



	inline pt_entry* vmmngr_ptable_lookup_entry(ptable* p, virtual_addr addr)
	{
		if (p != 0)
		{
			return &p->m_entries[PAGE_TABLE_INDEX(addr)];
		}
		else
		{
			return 0;
		}
	}

	inline pd_entry* vmmngr_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr)
	{
		if (p != 0) //what does this signify?
		{
			return &p->m_entries[PAGE_DIRECTORY_INDEX(addr)];
		}
		else
		{
			return 0;
		}
	}

	inline bool vmmngr_switch_pdirectory(pdirectory* dir)
	{
		if (!dir)
		{
			return false;
		}
		_cur_directory = dir;
		pmmngr_load_PDBR(_cur_pdbr);
		return true;
	}

	void vmmngr_flush_tlb_entry(virtual_addr addr)
	{
		flush_tlb_asm(addr);
	}

	pdirectory* vmmngr_get_directory()
	{
		return _cur_directory;
	}

	bool vmmngr_alloc_page(pt_entry* e)
	{
		//! allocate a free physical frame
		void* p = pmmngr_alloc_block();
		if (!p)
		{
			return false;
		}
		//! map it to the page
		pt_entry_set_frame(e, (physical_addr) p);
		pt_entry_add_attrib(e, I86_PTE_PRESENT);
		//doesent set WRITE flag...

		return true;
	}

	void vmmngr_free_page(pt_entry* e)
	{
		void* p = (void*) pt_entry_pfn(*e);
		if (p != 0)
		{
			pmmngr_free_block(p);
		}
		pt_entry_del_attrib(e, I86_PTE_PRESENT);
	}

	void vmmngr_map_page(void* phys, void* virt)
	{
		//! get page directory
		pdirectory* pageDirectory = vmmngr_get_directory();

		//! get page table
		pd_entry* e = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t) virt)];
		if ((*e & I86_PDE_PRESENT) != I86_PDE_PRESENT) {

			//! page table not present, allocate it
			ptable* table = (ptable*) pmmngr_alloc_block();
			if (!table)
			{
				return;
			}
			//! clear page table
			memset((uint8_t *) table, uint8_t(0), sizeof(*table));

			//! map in the table (Can also just do *entry |= 3) to enable these bits
			pd_entry_add_attrib(e, I86_PDE_PRESENT);
			pd_entry_add_attrib(e, I86_PDE_WRITABLE);
			pd_entry_set_frame(e, (physical_addr) table);
		}

		//! get table address from pde entry e
		ptable* table = (ptable*) pd_entry_pfn(*e);

		//! get page
		pt_entry* page = &table->m_entries[PAGE_TABLE_INDEX((uint32_t) virt)];

		//! map it in (Can also do (*page |= 3 to enable..)
		pt_entry_set_frame(page, (physical_addr) phys);
		pt_entry_add_attrib(page, I86_PTE_PRESENT);
	}

	void vmmngr_initialize()
	{
		magic_breakpoint_msg("In vmmngr_initialize() \n");

		//! allocate default page table
		ptable* table1 = (ptable*) pmmngr_alloc_block();

		set_log_int_base(16);
		log(OK, "table1 = ", int(table1), '\n');
		set_log_int_base(10);

		if (table1 == 0)
		{
			log(ERROR, "Could not allocate frame to table1\n");
			return;
		}
		//! allocates 3gb page table
		ptable* table2 = (ptable*) pmmngr_alloc_block();
		
		set_log_int_base(16);
		log(OK, "table2 = ", int(table2), '\n');
		set_log_int_base(10);

		if (table2 == 0)
		{
			log(ERROR, "Could not allocate frame to table\n");
			return;
		}
		
		//! clear page table
		memset((uint8_t *) table1, uint8_t(0), sizeof(*table1));
		magic_breakpoint_msg("Page table 1 cleared \n");

		memset((uint8_t *) table2, uint8_t(0), sizeof(*table2));
		magic_breakpoint_msg("Page table 2 cleared \n");


		//! 1st 4mb are idenitity mapped
		for (int i = 0, frame = 0x0, virt = 0x00000000; i < 1024; i++, frame += 4096, virt += 4096)
		{
			//! create a new page
			pt_entry page = 0;
			pt_entry_add_attrib(&page, I86_PTE_PRESENT);
			pt_entry_set_frame(&page, frame);

			//! ...and add it to the page table
			table2->m_entries[PAGE_TABLE_INDEX(virt)] = page;
		}

		magic_breakpoint_msg("Mapped 1st 4 MB : identity wise \n");

		//! map 1mb to 3gb (where we are at)
		for (int i = 0, frame = 0x100000, virt = 0xc0000000; i < 1024; i++, frame += 4096, virt += 4096)
		{
			//! create a new page
			pt_entry page = 0;
			pt_entry_add_attrib(&page, I86_PTE_PRESENT);
			pt_entry_set_frame(&page, frame);

			//! ...and add it to the page table
			table1->m_entries[PAGE_TABLE_INDEX(virt)] = page;
		}

		magic_breakpoint_msg("Mapped 1MB to 3GB \n");

		//! create default directory table
		pdirectory*   dir = (pdirectory*) pmmngr_alloc_blocks(3);
		if (!dir)
		{
			return;
		}
		//! clear directory table and set it as current
		memset((uint8_t *) dir, uint8_t(0), sizeof(*dir) * 3);// sizeof (pdirectory));

		//! get first entry in dir table and set it up to point to our table
		pd_entry* entry = &dir->m_entries[PAGE_DIRECTORY_INDEX(0xc0000000)];
		pd_entry_add_attrib(entry, I86_PDE_PRESENT);
		pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
		pd_entry_set_frame(entry, (physical_addr) table1);

		pd_entry* entry2 = &dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)];
		pd_entry_add_attrib(entry2, I86_PDE_PRESENT);
		pd_entry_add_attrib(entry2, I86_PDE_WRITABLE);
		pd_entry_set_frame(entry2, (physical_addr) table2);

		//! store current PDBR
		_cur_pdbr = (physical_addr) &dir->m_entries;

		//! switch to our page directory
		magic_breakpoint_msg("Before switching : \n");
		vmmngr_switch_pdirectory(dir);
		magic_breakpoint_msg("After switching : \n");
		//! enable paging
		pmmngr_paging_enable();
		//*/
		magic_breakpoint_msg("Out vmmngr_initialize() \n");
	}
}