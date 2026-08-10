#include "string.h"

void* memset(void* dest, int value, size_t count) {
    uint8_t* p = (uint8_t*)dest;
    for (size_t i = 0; i < count; i++) p[i] = (uint8_t)value;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < count; i++) d[i] = s[i];
    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    if (d == s || count == 0) return dest;
    if (d < s) {
        for (size_t i = 0; i < count; i++) d[i] = s[i];
    } else {
        for (size_t i = count; i > 0; i--) d[i - 1] = s[i - 1];
    }
    return dest;
}

int memcmp(const void* a, const void* b, size_t count) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    for (size_t i = 0; i < count; i++) {
        if (pa[i] != pb[i]) return (int)pa[i] - (int)pb[i];
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (int)(uint8_t)*a - (int)(uint8_t)*b;
}

int streq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

// prefix ile basliyorsa geri kalanina isaret eder, yoksa NULL.
const char* str_starts_with(const char* s, const char* prefix) {
    while (*prefix) { if (*s++ != *prefix++) return (void*)0; }
    return s;
}

char* u64_to_dec(uint64_t v, char* buf) {
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return buf; }

    char tmp[20];
    int i = 0;
    while (v > 0) { tmp[i++] = (char)('0' + (v % 10)); v /= 10; }

    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
    return buf;
}
