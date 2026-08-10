#pragma once
#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

void*  memset(void* dest, int value, size_t count);
void*  memcpy(void* dest, const void* src, size_t count);
void*  memmove(void* dest, const void* src, size_t count);
int    memcmp(const void* a, const void* b, size_t count);

size_t strlen(const char* s);
int    strcmp(const char* a, const char* b);

char*  u64_to_dec(uint64_t v, char* buf);

#endif
