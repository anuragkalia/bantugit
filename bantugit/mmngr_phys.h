#ifndef _MMNGR_PHYS_H
#define _MMNGR_PHYS_H
#include <stdint.h>
#include<stddef.h>

namespace myos
{
	//! 8 blocks per byte
#define PMMNGR_BLOCKS_PER_BYTE 8

	//! block size (4k)
#define PMMNGR_BLOCK_SIZE	4096

	//! block alignment
#define PMMNGR_BLOCK_ALIGN	PMMNGR_BLOCK_SIZE

	struct frame_t
	{
		char data[PMMNGR_BLOCK_SIZE];
	};

	typedef	uint32_t physical_addr;
	//! initialize the physical memory manager
	extern	"C" void	pmmngr_init(size_t, physical_addr);

	//! enables a physical memory region for use
	extern	"C" void	pmmngr_init_region(physical_addr, size_t);

	//! disables a physical memory region as in use (unuseable)
	extern	"C" void	pmmngr_deinit_region(physical_addr base, size_t);
	//! allocates a single memory block
	extern	"C" frame_t* pmmngr_alloc_block();

	//! releases a memory block
	extern	"C" void	pmmngr_free_block(frame_t*);
	//! allocates blocks of memory
	extern	"C" frame_t	* pmmngr_alloc_blocks(size_t);

	//! frees blocks of memory
	extern	"C" void pmmngr_free_blocks(frame_t*, size_t);

	extern	"C" size_t pmmngr_get_memory_size();

	//! returns default memory block size in bytes
	extern "C" uint32_t pmmngr_get_block_size();

	//! returns number of blocks currently in use
	extern	"C" uint32_t pmmngr_get_use_block_count();

	//! returns number of blocks not in use
	extern	"C" uint32_t pmmngr_get_free_block_count();

	//! returns number of memory blocks
	extern	"C" uint32_t pmmngr_get_block_count();

	//! enable or disable paging
	extern	"C" void	pmmngr_paging_enable();

	//! test if paging is enabled
	extern	"C" bool	pmmngr_is_paging();

	//! loads the page directory base register (PDBR)
	extern	"C" void	pmmngr_load_PDBR(physical_addr);

	//! get PDBR physical address
	extern	"C" physical_addr pmmngr_get_PDBR();
}
#endif
