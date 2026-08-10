#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "heap.h"
#include "vmm.h"
#include "pmm.h"
#include "string.h"


#define HEAP_BASE       0xFFFFA00000000000ull
#define HEAP_INIT_PAGES 16                       
#define ALIGN           16

typedef struct block {
    size_t         size;   
    bool           free;
    struct block* next;
    struct block* prev;
} block_t;

static uint64_t heap_end;      
static block_t* head;          
static size_t   used_bytes;

static inline size_t align_up(size_t n, size_t a) {
    return (n + a - 1) & ~(a - 1);
}


static bool grow_pages(size_t pages) {
    for (size_t i = 0; i < pages; i++) {
        void* frame = pmm_alloc_frame();       
        if (!frame) return false;
        vmm_map(vmm_kernel_pml4(), heap_end, (uint64_t)frame, PAGE_WRITE);
        heap_end += PAGE_SIZE;
    }
    return true;
}

void heap_init(void) {
    heap_end = HEAP_BASE;
    if (!grow_pages(HEAP_INIT_PAGES)) {
        for (;;) asm volatile("cli; hlt");
    }

    head = (block_t*)HEAP_BASE;
    head->size = (HEAP_INIT_PAGES * PAGE_SIZE) - sizeof(block_t);
    head->free = true;
    head->next = NULL;
    head->prev = NULL;
    used_bytes = 0;
}


static void split_block(block_t* b, size_t size) {
    size_t remaining = b->size - size;

    if (remaining < sizeof(block_t) + ALIGN) return;

    block_t* nb = (block_t*)((uint8_t*)b + sizeof(block_t) + size);
    nb->size = remaining - sizeof(block_t);
    nb->free = true;
    nb->next = b->next;
    nb->prev = b;
    if (nb->next) nb->next->prev = nb;

    b->size = size;
    b->next = nb;
}


static block_t* extend_heap(size_t min_size) {
    size_t need = align_up(min_size + sizeof(block_t), PAGE_SIZE);
    size_t pages = need / PAGE_SIZE;

    block_t* nb = (block_t*)heap_end;
    if (!grow_pages(pages)) return NULL;

    nb->size = (pages * PAGE_SIZE) - sizeof(block_t);
    nb->free = true;
    nb->next = NULL;

    block_t* last = head;
    while (last->next) last = last->next;
    last->next = nb;
    nb->prev = last;

    if (last->free) {
        last->size += sizeof(block_t) + nb->size;
        last->next = NULL;
        return last;
    }
    return nb;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = align_up(size, ALIGN);

    block_t* b = head;
    while (b) {
        if (b->free && b->size >= size) break;
        b = b->next;
    }

    if (!b) {
        b = extend_heap(size);
        if (!b) return NULL;
    }

    split_block(b, size);
    b->free = false;
    used_bytes += b->size;
    return (void*)((uint8_t*)b + sizeof(block_t));
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_t* b = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    if (b->free) return;               
    b->free = true;
    used_bytes -= b->size;

    if (b->next && b->next->free) {
        b->size += sizeof(block_t) + b->next->size;
        b->next = b->next->next;
        if (b->next) b->next->prev = b;
    }
   
    if (b->prev && b->prev->free) {
        b->prev->size += sizeof(block_t) + b->size;
        b->prev->next = b->next;
        if (b->next) b->next->prev = b->prev;
    }
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) { kfree(ptr); return NULL; }

    block_t* b = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    size_t new_size = align_up(size, ALIGN);
    if (b->size >= new_size) return ptr;   

    void* np = kmalloc(size);
    if (!np) return NULL;
    memcpy(np, ptr, b->size);
    kfree(ptr);
    return np;
}

size_t heap_used_bytes(void) { return used_bytes; }