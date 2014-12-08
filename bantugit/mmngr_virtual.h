#ifndef _MMNGR_VIRT_H
#define _MMNGR_VIRT_H


#include <stdint.h>
#include "vmmngr_pte.h"
#include "vmmngr_pde.h"

namespace myos

{//! virtual address
	typedef uint32_t virtual_addr;

	//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)



	//! page table
	struct ptable {

		pt_entry m_entries[PAGES_PER_TABLE];
	};

	//! page directory
	struct pdirectory {

		pd_entry m_entries[PAGES_PER_DIR];
	};


	//! maps phys to virtual address
	extern "C" void MmMapPage(void* phys, void* virt);

	//! initialize the memory manager
	extern "C" void vmmngr_initialize();

	//! allocates a page in physical memory
	extern "C" bool vmmngr_alloc_page(pt_entry*);

	//! frees a page in physical memory
	extern "C" void vmmngr_free_page(pt_entry* e);

	//! switch to a new page directory
	extern "C" bool vmmngr_switch_pdirectory(pdirectory*);

	//! get current page directory
	extern "C" pdirectory* vmmngr_get_directory();

	//! flushes a cached translation lookaside buffer (TLB) entry
	extern "C" void vmmngr_flush_tlb_entry(virtual_addr addr);

	//! clears a page table
	extern "C" void vmmngr_ptable_clear(ptable* p);

	//! convert virtual address to page table index
	extern "C" uint32_t vmmngr_ptable_virt_to_index(virtual_addr addr);

	//! get page entry from page table
	extern "C" pt_entry* vmmngr_ptable_lookup_entry(ptable* p, virtual_addr addr);

	//! convert virtual address to page directory index
	extern "C" uint32_t vmmngr_pdirectory_virt_to_index(virtual_addr addr);

	//! clears a page directory table
	extern "C" void vmmngr_pdirectory_clear(pdirectory* dir);

	//! get directory entry from directory table
	extern "C" pd_entry* vmmngr_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr);

}

#endif
