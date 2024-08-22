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
#include "utils/arena.h"

void *aligned_alloc(size_t alignment, size_t size);


void initMemoryPool(MemoryPool *pool, size_t block_size, size_t num_blocks)
{
    pool->block_size = block_size;
    pool->num_blocks = num_blocks;
    pool->free_list = NULL;

    // Allocate memory for the pool
    char *memory = (char *)malloc(block_size * num_blocks);
    for (size_t i = 0; i < num_blocks; i++)
    {
        PoolBlock *block = (PoolBlock *)(memory + i * block_size);
        block->next = pool->free_list;
        pool->free_list = block;
    }
}

void *poolAlloc(MemoryPool *pool)
{
    if (pool->free_list == NULL)
    {
        return NULL; // Pool is empty
    }
    PoolBlock *block = pool->free_list;
    pool->free_list = block->next;
    return block;
}

void poolFree(MemoryPool *pool, void *ptr)
{
    PoolBlock *block = (PoolBlock *)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
}

void initArena(Arena *arena, size_t size, size_t alignment)
{
    printf("[INFO] { Arena } Initializing arena with size %zu\n", size);
    // the `arena` parameter is an uninitialized pointer
    arena = (Arena *)aligned_alloc(alignment, sizeof(Arena));

    arena->base = aligned_alloc(alignment, size);
    printf("[INFO] { Arena } Arena base address: %p\n", arena->base);
    if (!arena->base)
    {
        printf("[ERROR] { Arena } Failed to allocate memory for arena\n");
        return;
    }
    arena->size = size;
    arena->offset = 0;
    arena->alignment = alignment;
    arena->next = NULL;
    arena->free_list = NULL;
    arena->total_allocations = 0;
    printf("[INFO] { Arena } Arena initialized...\n");

    initMemoryPool(&arena->small_pool, 16, 1024);
    initMemoryPool(&arena->medium_pool, 32, 512);
    initMemoryPool(&arena->large_pool, 64, 256);

    printf("[INFO] { Arena } Memory pools initialized...\n");
}

void *debugArenaAlloc(Arena *arena, size_t size, const char *file, int line)
{
    size_t total_size = size + sizeof(AllocationHeader);
    void *raw_memory = arenaAllocAligned(arena, total_size, arena->alignment);
    if (!raw_memory)
    {
        printf("[ERROR] { Arena } Allocation failed in file %s at line %d\n", file, line);
        return NULL;
    }

    AllocationHeader *header = (AllocationHeader *)raw_memory;
    header->size = size;
    header->file = file;
    header->line = line;

    return (char *)raw_memory + sizeof(AllocationHeader);
}

void *arenaAllocAligned(Arena *arena, size_t size, size_t alignment)
{
    size_t current_offset = arena->offset;
    size_t alignment_adjustment = (alignment - (current_offset % alignment)) % alignment;
    size_t aligned_offset = current_offset + alignment_adjustment;

    if (aligned_offset + size > arena->size)
    {
        return growArena(arena, size); // Attempt to grow the arena
    }

    void *ptr = (char *)arena->base + aligned_offset;
    arena->offset = aligned_offset + size;
    arena->total_allocations += 1;
    return ptr;
}

void *growArena(Arena *arena, size_t size)
{
    printf("[INFO] { Arena } Growing arena with size %zu\n", size);
    Arena *new_arena = (Arena *)malloc(sizeof(Arena));
    if (!new_arena)
    {
        printf("[ERROR] { Arena } Failed to allocate new arena structure\n");
        return NULL;
    }
    size_t new_size = arena->size * 2 + size;
    initArena(new_arena, new_size, arena->alignment);
    new_arena->next = arena;
    return arenaAllocAligned(new_arena, size, arena->alignment);
}

void *pushSize(Arena *arena, size_t size)
{
    printf("[INFO] { Arena } Pushing size %zu\n", size);
    size_t alignment = arena->alignment;
    size_t offset = arena->offset;
    size_t new_offset = (offset + size + (alignment - 1)) & ~(alignment - 1);
    if (new_offset > arena->size)
    {
        printf("[ERROR] { Arena } Arena out of memory\n");
        return NULL;
    }
    void *result = (char *)arena->base + offset;
    arena->offset = new_offset;
    arena->total_allocations++;
    printf("[INFO] { Arena } Size pushed...\n");
    return result;
}

void resetArena(Arena *arena)
{
    arena->offset = 0;
    arena->free_list = NULL;
    arena->total_allocations = 0;
}

void clearArena(Arena *arena)
{
    printf("[INFO] { Arena } Clearing arena\n");
    resetArena(arena);
    printf("[INFO] { Arena } Arena cleared...\n");
}

void freeArena(Arena *arena)
{
    printf("[INFO] { Arena } Freeing arena\n");
    Arena *current = arena;
    while (current)
    {
        Arena *next = current->next;
        free(current->base);
        free(current);
        current = next;
    }
    printf("[INFO] { Arena } Arena freed...\n");
}

// -----------------------------------------------------------------------------

void *arenaAlloc(Arena *arena, size_t size)
{
    // Use the existing alignment in the arena for the allocation
    void *memory = arenaAllocAligned(arena, size, arena->alignment);

    // If the allocation fails, handle it (e.g., by growing the arena)
    if (!memory)
    {
        memory = growArena(arena, size);
    }

    return memory;
}
