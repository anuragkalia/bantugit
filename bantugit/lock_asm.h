#include <stdint.h>

extern "C"
void asm_acquire_lock(uint32_t *lock);

extern "C"
void asm_release_lock(uint32_t *lock);