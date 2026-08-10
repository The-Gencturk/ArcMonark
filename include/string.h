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
int    streq(const char* a, const char* b);                       // strcmp==0 kisayolu
const char* str_starts_with(const char* s, const char* prefix);   // eslesirse kalan, yoksa NULL

char*  u64_to_dec(uint64_t v, char* buf);

#endif
