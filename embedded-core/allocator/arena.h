#ifndef ALLOCATOR_ARENA_H
#define ALLOCATOR_ARENA_H

#ifndef ALLOCATOR_ARENA_SIZE
    #define ALLOCATOR_ARENA_SIZE 1024
#endif

#include <stdint.h>
#include <stdio.h>

typedef struct
{
    size_t bytes_in_use;
    uint8_t buffer[ALLOCATOR_ARENA_SIZE];
} Allocator_Arena;

void* Arena_Malloc(size_t size);
void Arena_Reset();

#endif