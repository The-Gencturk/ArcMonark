#pragma once
#include <stdint.h>
#include <stddef.h>

void  heap_init(void);
void* kmalloc(size_t size);
void  kfree(void* ptr);
void* krealloc(void* ptr, size_t size);

size_t heap_used_bytes(void);