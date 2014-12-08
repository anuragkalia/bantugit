#ifndef __PAGING_ASM_H
#define __PAGING_ASM_H

#include <stdint.h>

extern "C" void flush_tlb_asm(uint32_t addr);
extern "C" void paging_enable_asm();
extern "C" uint32_t is_paging_asm();
extern "C" void load_pdbr_asm(uint32_t addr);
extern "C" uint32_t get_pdbr_asm();


#endif //__PAGING_ASM_H