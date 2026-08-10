#pragma once
#include <stdint.h>

#define PAGE_PRESENT   (1ull << 0)
#define PAGE_WRITE     (1ull << 1)
#define PAGE_USER      (1ull << 2)
#define PAGE_NX        (1ull << 63)

#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ull

void      vmm_init(void);
void      vmm_map(uint64_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void      vmm_unmap(uint64_t* pml4, uint64_t virt);
uint64_t* vmm_kernel_pml4(void);