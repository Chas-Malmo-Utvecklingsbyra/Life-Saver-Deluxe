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

/**
 * 
 * @brief Allocate a space in the arena allocator
 * 
 * This function works pretty much exactly how malloc works, this does NOT use the heap at all.
 * 
 * @param size The size of the buffer you wanna allocate.
 * 
 * @return Returns the address of your allocated memory. NULL if there is no available memory!
 *
 * @note This does NOT use the heap, instead it exists in the .data section of the program. 
 * 
 */
void* Arena_Malloc(size_t size);

/**
 * 
 * @brief Clears the memory of the Arena
 * 
 * This should be used whenever you are done using the data in the Arena to not clutter the memory available.
 * Otherwise you will run out of memory fast!
 * 
 * Examples are: 
 * 
 * Using Packet_Build and then after having sent it to your Client you can use Arena_Reset() to clear the Memory!
 * 
 * @warning This clears ALL the memory reserved with Arena_Malloc.
 * 
 */
void Arena_Reset();

#endif