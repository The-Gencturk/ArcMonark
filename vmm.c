#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "vmm.h"
#include "pmm.h"
#include "string.h"

extern uint64_t hhdm_offset;   

#define PML4_INDEX(v) (((v) >> 39) & 0x1FF)
#define PDPT_INDEX(v) (((v) >> 30) & 0x1FF)
#define PD_INDEX(v)   (((v) >> 21) & 0x1FF)
#define PT_INDEX(v)   (((v) >> 12) & 0x1FF)

static uint64_t* kernel_pml4;

static inline uint64_t* phys_to_virt(uint64_t phys) {
    return (uint64_t*)(phys + hhdm_offset);
}

static inline void invlpg(uint64_t virt) {
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

static inline uint64_t read_cr3(void) {
    uint64_t v; asm volatile("mov %%cr3, %0" : "=r"(v)); return v;
}

static inline void write_cr3(uint64_t v) {
    asm volatile("mov %0, %%cr3" : : "r"(v) : "memory");
}

static uint64_t* get_next_level(uint64_t* table, uint64_t index, bool allocate) {
    if (table[index] & PAGE_PRESENT) {
        return phys_to_virt(table[index] & PAGE_ADDR_MASK);
    }
    if (!allocate) return NULL;

    void* frame = pmm_alloc_frame();     // fiziksel
    if (!frame) return NULL;

    uint64_t phys = (uint64_t)frame;
    uint64_t* next = phys_to_virt(phys);
    memset(next, 0, PAGE_SIZE);


    table[index] = phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    return next;
}

void vmm_map(uint64_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t* pdpt = get_next_level(pml4, PML4_INDEX(virt), true);
    uint64_t* pd = get_next_level(pdpt, PDPT_INDEX(virt), true);
    uint64_t* pt = get_next_level(pd, PD_INDEX(virt), true);

    pt[PT_INDEX(virt)] = (phys & PAGE_ADDR_MASK) | flags | PAGE_PRESENT;
    invlpg(virt);
}

void vmm_unmap(uint64_t* pml4, uint64_t virt) {
    uint64_t* pdpt = get_next_level(pml4, PML4_INDEX(virt), false);
    if (!pdpt) return;
    uint64_t* pd = get_next_level(pdpt, PDPT_INDEX(virt), false);
    if (!pd) return;
    uint64_t* pt = get_next_level(pd, PD_INDEX(virt), false);
    if (!pt) return;

    pt[PT_INDEX(virt)] = 0;
    invlpg(virt);
}

uint64_t* vmm_kernel_pml4(void) { return kernel_pml4; }

void vmm_init(void) {
    uint64_t* current = phys_to_virt(read_cr3() & PAGE_ADDR_MASK);

    void* frame = pmm_alloc_frame();
    uint64_t new_phys = (uint64_t)frame;
    kernel_pml4 = phys_to_virt(new_phys);
    memset(kernel_pml4, 0, PAGE_SIZE);


    for (int i = 256; i < 512; i++) {
        kernel_pml4[i] = current[i];
    }

    write_cr3(new_phys);   
}