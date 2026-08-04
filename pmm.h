#pragma once
#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

extern uint64_t hhdm_offset;   

void  pmm_init(void);
void* pmm_alloc_frame(void);   
void  pmm_free_frame(void* frame);

uint64_t pmm_total_frames(void);
uint64_t pmm_used_frames(void);
uint64_t pmm_free_frames(void);

void* memset(void* dest, int value, uint64_t count);

char* u64_to_dec(uint64_t v, char* buf);

#endif 