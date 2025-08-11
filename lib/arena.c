#pragma once

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>
#include "types.c"

// https://nullprogram.com/blog/2023/09/27/
#define sizeof(x)    (ptrdiff_t)sizeof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)

typedef struct {
    void *base;
    void *start;
    void *end;
} Arena;

Arena Arena_New(usize capacity) {
    Arena a = {0};
    a.base = malloc(capacity);
    a.start = a.base;
    a.end = a.start ? a.start+capacity : 0;
    return a;
}

#define Arena_Alloc(...)            Arena_Allocx(__VA_ARGS__,Arena_Alloc4,Arena_Alloc3,Arena_Alloc2)(__VA_ARGS__)
#define Arena_Allocx(a,b,c,d,e,...) e
#define Arena_Alloc2(a, t)          (t *)Arena_AllocAlign(a, sizeof(t), alignof(t), 1, 0)
#define Arena_Alloc3(a, t, n)       (t *)Arena_AllocAlign(a, sizeof(t), alignof(t), n, 0)
#define Arena_Alloc4(a, t, n, f)    (t *)Arena_AllocAlign(a, sizeof(t), alignof(t), n, f)

void *Arena_AllocAlign(Arena *arena, usize size, usize align, u32 count, u8 flags) {
    ptrdiff_t padding = -(uptr) arena->start & (align - 1);
    ptrdiff_t available = arena->end - arena->start - padding;
    if (available < 0 || count > available / size) { raise(SIGTRAP); }

    void *p = arena->start + padding;
    arena->start += padding + count * size;
    return memset(p, 0, count * size);
}

void Arena_Free(Arena *arena) {
    free(arena->base);
    arena->base = arena->start = arena->end = NULL;
}

void Arena_Reset(Arena *arena) {
    arena->start = arena->base;
}

