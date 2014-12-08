#include "mmngr_phys.h"
#include "paging_asm.h"
#include "stdfunc.h"

namespace myos
{
	//! size of physical memory
	static	uint32_t	_mmngr_memory_size = 0;

	//! number of blocks currently in use
	static	uint32_t	_mmngr_used_blocks = 0;

	//! maximum number of available memory blocks
	static	uint32_t	_mmngr_max_blocks = 0;

	//! memory map bit array. Each bit represents a memory block
	static	uint32_t*	_mmngr_memory_map = 0;

	//! set any bit (frame) within the memory map bit array
	inline void mmap_set(int bit);

	//! unset any bit (frame) within the memory map bit array
	inline void mmap_unset(int bit);

	//! test any bit (frame) within the memory map bit array
	inline bool mmap_test(int bit);

	//! finds first free frame in the bit array and returns its index
	int mmap_first_free();


	//============================================================================
	//    IMPLEMENTATION PRIVATE FUNCTIONS
	//============================================================================

	//! set any bit (frame) within the memory map bit array
	inline void mmap_set(int bit) {

		_mmngr_memory_map[bit / 32] |= (1 << (bit % 32));
	}

	//! unset any bit (frame) within the memory map bit array
	inline void mmap_unset(int bit) {

		_mmngr_memory_map[bit / 32] &= ~(1 << (bit % 32));
	}

	//! test if any bit (frame) is set within the memory map bit array
	inline bool mmap_test(int bit) {

		return _mmngr_memory_map[bit / 32] & (1 << (bit % 32));
	}

	//! finds first free frame in the bit array and returns its index
	int mmap_first_free() {

		//! find the first free bit
		for (uint32_t i = 0; i < pmmngr_get_block_count() / 32; i++)
			if (_mmngr_memory_map[i] != 0xffffffff)
				for (int j = 0; j < 32; j++) {				//! test each bit in the dword

			int bit = 1 << j;
			if (!(_mmngr_memory_map[i] & bit))
				return i * 4 * 8 + j;
				}

		return -1;
	}
	//! finds first free "size" number of frames and returns its index
	int mmap_first_free_s(size_t size) {

		if (size == 0)
			return -1;

		if (size == 1)
			return mmap_first_free();

		for (uint32_t i = 0; i < pmmngr_get_block_count() / 32; i++)
		{
			if (_mmngr_memory_map[i] != 0xffffffff)
			{
				for (int j = 0; j < 32; j++)	//! test each bit in the dword
				{
					int bit = 1 << j;
					if ((_mmngr_memory_map[i] & bit) == 0) 
					{
						int startingBit = i * 32 + bit;		//get the free bit in the dword at index i

						uint32_t free = 0; //loop through each bit to see if its enough space
						for (uint32_t count = 0; count <= size; count++)
						{
							if (mmap_test(startingBit + count) == 0)
							{
								free++;	// this bit is clear (free frame)
							}
							if (free == size)
							{
								return i * 4 * 8 + j; //free count==size needed; return index
							}
						}
					}
				}
			} //end of if( != 0xffffffff)
		}
		return -1;
	}



	void	pmmngr_init(size_t memSize, physical_addr bitmap) {

		_mmngr_memory_size = memSize;
		_mmngr_memory_map = (uint32_t*) bitmap;
		_mmngr_max_blocks = (pmmngr_get_memory_size() * 1024) / PMMNGR_BLOCK_SIZE;
		_mmngr_used_blocks = _mmngr_max_blocks;

		//! By default, all of memory is in use
		memset(_mmngr_memory_map, uint32_t(0xf), size_t(pmmngr_get_block_count() / PMMNGR_BLOCKS_PER_BYTE));

	}

	void	pmmngr_init_region(physical_addr base, size_t size) {

		int align = base / PMMNGR_BLOCK_SIZE;
		int blocks = size / PMMNGR_BLOCK_SIZE;

		for (; blocks >= 0; blocks--) {
			mmap_unset(align++);
			_mmngr_used_blocks--;


		}

		mmap_set(0);	//first block is always set. This insures allocs cant be 0
	}

	void	pmmngr_deinit_region(physical_addr base, size_t size) {

		int align = base / PMMNGR_BLOCK_SIZE;
		int blocks = size / PMMNGR_BLOCK_SIZE;

		for (; blocks >= 0; blocks--) {
			mmap_set(align++);
			_mmngr_used_blocks++;
		}

	}

	frame_t * pmmngr_alloc_block() {

		if (pmmngr_get_free_block_count() <= 0)
			return 0;	//out of memory

		int frame = mmap_first_free();

		if (frame == -1)
			return 0;	//out of memory

		mmap_set(frame);

		physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
		_mmngr_used_blocks++;

		return (frame_t *) addr;
	}

	void pmmngr_free_block(frame_t *p) {

		physical_addr addr = (physical_addr) p;
		int frame = addr / PMMNGR_BLOCK_SIZE;

		mmap_unset(frame);

		_mmngr_used_blocks--;
	}

	frame_t * pmmngr_alloc_blocks(size_t size) {

		if (pmmngr_get_free_block_count() <= size)
			return 0;	//not enough space

		int frame = mmap_first_free_s(size);

		if (frame == -1)
			return 0;	//not enough space

		for (uint32_t i = 0; i < size; i++)
			mmap_set(frame + i);

		physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
		_mmngr_used_blocks += size;

		return (frame_t*) addr;
	}

	void	pmmngr_free_blocks(frame_t* p, size_t size) {

		physical_addr addr = (physical_addr) p;
		int frame = addr / PMMNGR_BLOCK_SIZE;

		for (uint32_t i = 0; i < size; i++)
			mmap_unset(frame + i);

		_mmngr_used_blocks -= size;
	}


	size_t	pmmngr_get_memory_size() {

		return _mmngr_memory_size;
	}

	uint32_t pmmngr_get_block_count() {

		return _mmngr_max_blocks;
	}
	uint32_t pmmngr_get_use_block_count() {

		return _mmngr_used_blocks;
	}

	uint32_t pmmngr_get_free_block_count() {

		return _mmngr_max_blocks - _mmngr_used_blocks;
	}
	uint32_t pmmngr_get_block_size() {

		return PMMNGR_BLOCK_SIZE;
	}
	void	pmmngr_paging_enable() {

		paging_enable_asm();
		/*#ifdef _MSC_VER
			_asm {
			mov	eax, cr0
			cmp [b], 1
			je	enable
			jmp disable
			enable:
			or eax, 0x80000000		//set bit 31
			mov	cr0, eax
			jmp done
			disable:
			and eax, 0x7FFFFFFF		//clear bit 31
			mov	cr0, eax
			done:
			}
			#endif
			*/
	}

	bool pmmngr_is_paging() {

		uint32_t res = is_paging_asm();

		/*#ifdef _MSC_VER
			_asm {
			mov	eax, cr0
			mov	[res], eax
			}
			#endif
			*/

		return (res & 0x80000000) ? false : true;
	}

	void pmmngr_load_PDBR(physical_addr addr) {

		load_pdbr_asm(addr);
		/*
		#ifdef _MSC_VER
		_asm {
		mov	eax, [addr]
		mov	cr3, eax		// PDBR is cr3 register in i86
		}
		#endif
		*/
	}

	physical_addr pmmngr_get_PDBR() {

		return  (physical_addr) get_pdbr_asm();
		/*#ifdef _MSC_VER
			_asm {
			mov	eax, cr3
			ret
			}
			#endif
			*/
	}

}