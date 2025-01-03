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
#include <assert.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tools/utils/c_logger.h"
#include "tools/utils/utility.h"
#include "tools/macros/debugMacros.h"
#include "tools/macros/printMacros.h"

#define ARENA_SIZE 1024 * 1024
#define ALIGNMENT 16

#define PRINT_WIDTH 80
#define MEMORY_BAR_WIDTH 60

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
 * @brief Memory block structure for tracking block sizes and chaining.
 */
typedef struct MemoryBlock
{
    size_t size;              // For tracking the size of the block
    struct MemoryBlock *next; // For chaining blocks
} MemoryBlock;

/**
 * @brief Arena structure for managing memory allocations.
 * each arena has a base pointer, size, offset, alignment, next arena, free list, and total allocations.
 *
 * @note The free list is used for pooling memory blocks to reduce fragmentation.
 */
typedef struct Arena
{
    void *base;
    size_t size;
    size_t offset;
    size_t alignment;
    struct Arena *next;
    struct MemoryBlock *free_list;
    size_t total_allocations;
    MemoryPool small_pool;
    MemoryPool medium_pool;
    MemoryPool large_pool;
} Arena;

#define ARENA_ALLOC(arena, size) arenaAllocAligned(arena, size, (arena)->alignment, __FILE__, __LINE__)
#define DEBUG_ARENA_ALLOC(arena, size) debugArenaAlloc(arena, size, __FILE__, __LINE__)
#define DEBUG_ARENA_PRINT(arena) arenaDebugPrint(arena)

/**
 * @brief Allocation header structure for tracking debugging information.
 */
typedef struct AllocationHeader
{
    size_t size;
    size_t padding;
    int line;
    char file[256]; // Adjust size as needed
} AllocationHeader;
// -----------------------------------------------------------------------------------
// Function prototypes

Arena *createArena(size_t size, size_t alignment);

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
void *arenaAllocAligned(Arena *arena, size_t size, size_t alignment, const char *file, int line);

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

void arenaDebugPrint(Arena *arena);

char *string_repeat(const char *str, int times);
char *truncate_string(const char *str, size_t max_len);
const char *get_allocation_type(size_t size);
void print_pool_info(const char *name, MemoryPool *pool);
#endif // ARENA_H
