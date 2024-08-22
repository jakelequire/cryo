/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#ifndef ARENA_H
#define ARENA_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/stat.h>

// Forward declarations
void *aligned_alloc(size_t alignment, size_t size);

// ------------------------------
// Macros
#define ARENA_ALLOC(arena, size) arena_alloc_aligned(arena, size, 8)
#define DEBUG_ARENA_ALLOC(arena, size) debugArenaAlloc(arena, size, __FILE__, __LINE__)
#define SMALL_POOL_SIZE 32
#define MEDIUM_POOL_SIZE 128
#define LARGE_POOL_SIZE 512

typedef struct PoolBlock
{
    struct PoolBlock *next;
} PoolBlock;

typedef struct MemoryPool
{
    size_t block_size;
    PoolBlock *free_list;
    size_t num_blocks;
} MemoryPool;

// ------------------------------
// Structs
/**
 * @brief Arena structure for managing memory allocations.
 * each arena has a base pointer, size, offset, alignment, next arena, free list, and total allocations.
 *
 * @note The free list is used for pooling memory blocks to reduce fragmentation.
 */
typedef struct Arena
{
    void *base;                    // For tracking the base of the arena
    size_t size;                   // For tracking the size of the arena
    size_t offset;                 // For tracking the current offset
    size_t alignment;              // For alignment
    struct Arena *next;            // For chaining arenas
    struct MemoryBlock *free_list; // Pooling free blocks
    size_t total_allocations;      // For debugging or tracking purposes
    MemoryPool small_pool;
    MemoryPool medium_pool;
    MemoryPool large_pool;

} Arena;

/**
 * @brief Memory block structure for tracking block sizes and chaining.
 */
typedef struct MemoryBlock
{
    size_t size;              // For tracking the size of the block
    struct MemoryBlock *next; // For chaining blocks
} MemoryBlock;

/**
 * @brief Allocation header structure for tracking debugging information.
 */
typedef struct AllocationHeader
{
    size_t size;
    const char *file;
    int line;
} AllocationHeader;
// ------------------------------
// Function prototypes

/**
 * @brief Initializes a memory pool with the specified block size and number of blocks.
 *
 * @param pool Pointer to the MemoryPool structure to initialize.
 * @param block_size The size of each memory block in bytes.
 * @param num_blocks The number of blocks to allocate in the pool.
 *
 * @note This function initializes the free list with all blocks in the pool.
 */
void initMemoryPool(MemoryPool *pool, size_t block_size, size_t num_blocks);

/**
 * @brief Allocates a block of memory from the pool.
 *
 * @param pool Pointer to the MemoryPool from which to allocate memory.
 */
void *poolAlloc(MemoryPool *pool);

/**
 * @brief Frees a block of memory from the pool.
 *
 * @param pool Pointer to the MemoryPool from which to free memory.
 * @param ptr Pointer to the memory block to free.
 */
void poolFree(MemoryPool *pool, void *ptr);

/**
 * @brief Initializes a memory arena with the specified size and alignment.
 *
 * @param arena Pointer to the Arena structure to be initialized.
 * @param size The size of the memory arena in bytes.
 * @param alignment The alignment requirement for memory allocations.
 *
 * @note This function allocates memory using aligned_alloc and sets up the arena structure.
 */
void initArena(Arena *arena, size_t size, size_t alignment);

/**
 * @brief Allocates memory from the arena with debugging information.
 *
 * @param arena Pointer to the Arena from which to allocate memory.
 * @param size The size of the memory block to allocate in bytes.
 * @param file The name of the source file making the allocation.
 * @param line The line number in the source file making the allocation.
 *
 * @return Pointer to the allocated memory block, or NULL if allocation fails.
 *
 * @note The allocated block includes a header with debugging information, such as the source file and line number.
 */
void *debugArenaAlloc(Arena *arena, size_t size, const char *file, int line);

/**
 * @brief Allocates memory from the arena with specific alignment.
 *
 * @param arena Pointer to the Arena from which to allocate memory.
 * @param size The size of the memory block to allocate in bytes.
 * @param alignment The alignment requirement for the allocation.
 *
 * @return Pointer to the allocated memory block, or NULL if out of memory.
 *
 * @note If the arena doesn't have enough space, it attempts to grow the arena by allocating more memory.
 */
void *arenaAllocAligned(Arena *arena, size_t size, size_t alignment);

/**
 * @brief Grows the memory arena by allocating a new, larger block of memory.
 *
 * @param arena Pointer to the current Arena structure.
 * @param size The minimum size required for the new memory block in bytes.
 *
 * @return Pointer to the allocated memory block in the new arena, or NULL if allocation fails.
 *
 * @note This function creates a new arena with double the previous size and links it to the current one.
 */
void *growArena(Arena *arena, size_t size);

/**
 * @brief Allocates a block of memory from the arena, ensuring proper alignment.
 *
 * @param arena Pointer to the Arena from which to allocate memory.
 * @param size The size of the memory block to allocate in bytes.
 *
 * @return Pointer to the allocated memory block, or NULL if out of memory.
 *
 * @note This function also increments the total allocation counter in the arena structure.
 */
void *pushSize(Arena *arena, size_t size);

/**
 * @brief Resets the arena, allowing its memory to be reused.
 *
 * @param arena Pointer to the Arena to reset.
 *
 * @note This function does not free the memory but resets the offset and clears the free list.
 */
void resetArena(Arena *arena);

/**
 * @brief Clears the arena, effectively resetting it for reuse.
 *
 * @param arena Pointer to the Arena to clear.
 *
 * @note This function calls resetArena to reset the offset and free list, preparing the arena for new allocations.
 */
void clearArena(Arena *arena);

/**
 * @brief Frees all memory associated with the arena.
 *
 * @param arena Pointer to the Arena to free.
 *
 * @note This function frees the base memory of each arena in the chain and the arena structures themselves.
 */
void freeArena(Arena *arena);

// ------------------------------

/**
 * @brief Allocates a block of memory from the arena.
 *
 * @param arena Pointer to the Arena from which to allocate memory.
 * @param size The size of the memory block to allocate in bytes.
 *
 * @return Pointer to the allocated memory block, or NULL if out of memory.
 *
 * @note This function hides the complexity of alignment and debugging, making it simple to use.
 */
void *arenaAlloc(Arena *arena, size_t size);

#endif // ARENA_H
