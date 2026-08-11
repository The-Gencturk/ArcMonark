#pragma once
#include <stdint.h>

// Kesmeleri kapatip onceki IF durumunu geri yukleyen kucuk kritik-bolge
// yardimcilari. Preemption acikken paylasilan yapilar (heap, listeler,
// olay kuyrugu) bunlarla korunur. cli/sti yerine bunlari kullan ki cagiran
// zaten kesmeleri kapattiysa yanlislikla acmayalim.

static inline uint64_t irq_save(void) {
    uint64_t flags;
    __asm__ volatile ("pushfq; pop %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

static inline void irq_restore(uint64_t flags) {
    __asm__ volatile ("push %0; popfq" :: "r"(flags) : "memory", "cc");
}
