#ifndef __VKLIB__SET_H_
#define __VKLIB__SET_H_ 1

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <memory.h>
#include <stdlib.h>

#define SET_BUFFER_SIZE 0x10

#define set_init(TYPE) ((TYPE ## _set){0,0,0})

#define DECLARE_SET(TYPE) \
typedef struct \
{ \
    size_t count; \
    size_t buffer_size; \
    TYPE* data; \
} TYPE ## _set ; \
void TYPE ## _set_append(TYPE ## _set* set, TYPE value); \
void TYPE ## _set_remove(TYPE ## _set* set, TYPE value); \
void TYPE ## _set_dispose(TYPE ## _set* set)

#define IMPLEMENT_SET(TYPE) \
size_t TYPE ## _set_contains(TYPE ## _set* set, TYPE value) \
{ \
    if (!set) \
        return -1; \
\
    for (size_t i = 0; i < set->count; i++) \
        if (set->data[i] == value) \
            return i; \
\
    return -1; \
} \
\
void TYPE ## _set_append(TYPE ## _set* set, TYPE value) \
{ \
    if (!set) \
        return; \
\
    size_t idx = TYPE ## _set_contains(set, value); \
\
    if (idx != (size_t)-1) \
    { \
        set->data[idx] = value; \
        return; \
    } \
\
    if (set->count + 1 >= set->buffer_size) \
    { \
        set->buffer_size += SET_BUFFER_SIZE; \
        set->data = realloc(set->data, sizeof(TYPE) * set->buffer_size); \
    } \
\
    set->data[set->count++] = value; \
} \
\
void TYPE ## _set_remove(TYPE ## _set* set, TYPE value) \
{ \
    if (!set) \
        return; \
\
    size_t idx = TYPE ## _set_contains(set, value); \
\
    if (idx == (size_t)-1) \
        return; \
\
    if (idx < set->count) \
        memmove(&set->data[idx], &set->data[idx+1], sizeof(TYPE) * (set->count-idx-1)); \
\
    set->count--; \
} \
\
void TYPE ## _set_dispose(TYPE ## _set* set) \
{ \
    if (!set || !set->data) \
        return; \
\
    if (!set->data) \
        free(set->data); \
\
    memset(set, 0, sizeof(TYPE ## _set)); \
}

typedef uint32_t u32;
typedef uint64_t u64;

DECLARE_SET(u32);
DECLARE_SET(u64);

#endif
