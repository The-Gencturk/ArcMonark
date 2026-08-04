#include <stdint.h>
#include <stddef.h>     
#include <stdbool.h>
#include "limine.h"     
#include "pmm.h"
#include "string.h"     

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID, .revision = 0
};

uint64_t hhdm_offset;              
static uint8_t* bitmap;
static uint64_t total_frames, used_frames;
static uint64_t bitmap_size;        

static inline void bmp_set(uint64_t f) { bitmap[f / 8] |= (1 << (f % 8)); }
static inline void bmp_clear(uint64_t f) { bitmap[f / 8] &= ~(1 << (f % 8)); }
static inline bool bmp_test(uint64_t f) { return bitmap[f / 8] & (1 << (f % 8)); }

void pmm_init(void) {
    if (!memmap_request.response || !hhdm_request.response) {
        for (;;) asm volatile("cli; hlt");
    }

    struct limine_memmap_response* mm = memmap_request.response;
    hhdm_offset = hhdm_request.response->offset;

    
    uint64_t highest = 0;
    for (uint64_t i = 0; i < mm->entry_count; i++) {
        struct limine_memmap_entry* e = mm->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE) continue;
        uint64_t top = e->base + e->length;
        if (top > highest) highest = top;
    }
    total_frames = highest / PAGE_SIZE;
    bitmap_size = (total_frames + 7) / 8;

    bitmap = NULL;
    for (uint64_t i = 0; i < mm->entry_count; i++) {
        struct limine_memmap_entry* e = mm->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE) continue;
        if (e->length >= bitmap_size) {
            bitmap = (uint8_t*)(e->base + hhdm_offset);
            break;
        }
    }

    memset(bitmap, 0xFF, bitmap_size);
    used_frames = total_frames;

    for (uint64_t i = 0; i < mm->entry_count; i++) {
        struct limine_memmap_entry* e = mm->entries[i];
        if (e->type != LIMINE_MEMMAP_USABLE) continue;
        for (uint64_t j = 0; j < e->length; j += PAGE_SIZE) {
            bmp_clear((e->base + j) / PAGE_SIZE);
            used_frames--;
        }
    }

    uint64_t bmp_phys = (uint64_t)bitmap - hhdm_offset;
    for (uint64_t i = 0; i < bitmap_size; i += PAGE_SIZE) {
        bmp_set((bmp_phys + i) / PAGE_SIZE);
        used_frames++;
    }
}

void* pmm_alloc_frame(void) {
    for (uint64_t i = 1; i < total_frames; i++) { 
        if (!bmp_test(i)) {
            bmp_set(i);
            used_frames++;
            return (void*)(i * PAGE_SIZE);          
        }
    }
    return NULL;
}

void pmm_free_frame(void* frame) {
    uint64_t idx = (uint64_t)frame / PAGE_SIZE;
    if (!bmp_test(idx)) return;   
    bmp_clear(idx);
    used_frames--;
}

char* u64_to_dec(uint64_t v, char* buf) {
    char tmp[21];
    int i = 0;
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return buf; }
    while (v > 0) { tmp[i++] = '0' + (v % 10); v /= 10; }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
    return buf;
}


void* memset(void* dest, int value, uint64_t count) {
    uint8_t* p = (uint8_t*)dest;
    for (uint64_t i = 0; i < count; i++) p[i] = (uint8_t)value;
    return dest;
}

uint64_t pmm_total_frames(void) { return total_frames; }
uint64_t pmm_used_frames(void) { return used_frames; }
uint64_t pmm_free_frames(void) { return total_frames - used_frames; }