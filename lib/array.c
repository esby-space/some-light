#ifndef ARRAY
#define ARRAY

#include "arena.c"

// https://nullprogram.com/blog/2023/10/05/

#define Array_Push(slice, arena) \
    ((slice)->length >= (slice)->capacity \
        ? Array_Grow(slice, sizeof(*(slice)->data), arena), \
          (slice)->data + (slice)->length++ \
        : (slice)->data + (slice)->length++)



static void Array_Grow(void *slice, ptrdiff_t size, Arena *arena) {
    struct {
        void     *data;
        ptrdiff_t length;
        ptrdiff_t capacity;
    } replica;
    memcpy(&replica, slice, sizeof(replica));

    replica.capacity = replica.capacity ? replica.capacity : 1;
    ptrdiff_t align = 16;
    void *data = Arena_AllocAlign(arena, 2*size, align, replica.capacity);
    replica.capacity *= 2;

    if (replica.length) { memcpy(data, replica.data, size*replica.length); }
    replica.data = data;
    memcpy(slice, &replica, sizeof(replica));
}

#endif

