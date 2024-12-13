#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

// String utilities
static inline uint32_t str_len(const char* str) {
    uint32_t len = 0;
    while (str[len]) len++;
    return len;
}

static inline void str_copy(char* dest, const char* src, uint32_t max_len) {
    uint32_t i;
    for (i = 0; i < max_len - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

static inline int str_compare(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

static inline const char* str_find(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return haystack;
    }
    return 0;
}

// Memory utilities
static inline void mem_copy(void* dest, const void* src, uint32_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while (size--) *d++ = *s++;
}

static inline void mem_set(void* dest, uint8_t value, uint32_t size) {
    uint8_t* d = (uint8_t*)dest;
    while (size--) *d++ = value;
}

static inline int mem_compare(const void* s1, const void* s2, uint32_t size) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    while (size--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

#endif // UTIL_H
