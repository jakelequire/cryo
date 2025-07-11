/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "tools/arena/arena.h"
#include "diagnostics/diagnostics.h"

Arena *createArena(size_t size, size_t alignment)
{
    __STACK_FRAME__
    Arena *arena = (Arena *)malloc(sizeof(Arena));
    if (!arena)
    {
        logMessage(LMI, "ERROR", "Arena", "Failed to allocate memory for arena");
        return NULL;
    }
    initArena(arena, size, alignment);
    return arena;
}

void initMemoryPool(MemoryPool *pool, size_t block_size, size_t num_blocks)
{
    __STACK_FRAME__
    pool->block_size = block_size;
    pool->num_blocks = num_blocks;
    pool->free_list = NULL;

    logMessage(LMI, "INFO", "MemoryPool", "Memory pool initialized");

    // Allocate memory for the pool
    char *memory = (char *)malloc(block_size * num_blocks);
    for (size_t i = 0; i < num_blocks; i++)
    {
        // printf("[INFO] { MemoryPool } Allocating block %zu\n", i);
        PoolBlock *block = (PoolBlock *)(memory + i * block_size);
        block->next = pool->free_list;
        pool->free_list = block;
        // printf("[INFO] { MemoryPool } Block %zu allocated\n", i);
    }
}

void *poolAlloc(MemoryPool *pool)
{
    __STACK_FRAME__
    if (pool->free_list == NULL)
    {
        logMessage(LMI, "ERROR", "MemoryPool", "Pool is empty");
        return NULL; // Pool is empty
    }
    PoolBlock *block = pool->free_list;
    pool->free_list = block->next;

    logMessage(LMI, "INFO", "MemoryPool", "Block allocated");

    return block;
}

void poolFree(MemoryPool *pool, void *ptr)
{
    __STACK_FRAME__
    PoolBlock *block = (PoolBlock *)ptr;
    block->next = pool->free_list;
    pool->free_list = block;

    logMessage(LMI, "INFO", "MemoryPool", "Block freed");
}

void initArena(Arena *arena, size_t size, size_t alignment)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Arena", "Initializing arena");

    arena->base = aligned_alloc(alignment, size);
    if (!arena->base)
    {
        logMessage(LMI, "ERROR", "Arena", "Failed to allocate memory for arena");
        return;
    }

    logMessage(LMI, "INFO", "Arena", "Arena initialized");
    arena->size = size;
    arena->offset = 0;
    arena->alignment = alignment;
    arena->next = NULL;
    arena->free_list = NULL;
    arena->total_allocations = 0;
    logMessage(LMI, "INFO", "Arena", "Arena initialization complete");

    initMemoryPool(&arena->small_pool, 16, 1024);
    initMemoryPool(&arena->medium_pool, 32, 512);
    initMemoryPool(&arena->large_pool, 64, 256);

    logMessage(LMI, "INFO", "Arena", "Memory pools initialized");
}

void *debugArenaAlloc(Arena *arena, size_t size, const char *file, int line)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Arena", "Allocating memory with debug information");
    size_t total_size = size + sizeof(AllocationHeader);
    void *raw_memory = arenaAllocAligned(arena, total_size, arena->alignment, __FILE__, __LINE__);
    logMessage(LMI, "INFO", "Arena", "Memory allocated");
    if (!raw_memory)
    {
        logMessage(LMI, "ERROR", "Arena", "Failed to allocate memory");
        return NULL;
    }

    AllocationHeader *header = (AllocationHeader *)raw_memory;
    header->size = size;
    header->padding = total_size - size - sizeof(AllocationHeader);
    header->line = line;
    strncpy(header->file, file, sizeof(header->file) - 1);
    header->file[sizeof(header->file) - 1] = '\0'; // Ensure null-termination

    logMessage(LMI, "INFO", "Arena", "Debug information added to memory allocation");

    return (char *)raw_memory + sizeof(AllocationHeader);
}

void *arenaAllocAligned(Arena *arena, size_t size, size_t alignment, const char *file, int line)
{
    __STACK_FRAME__
    // logMessage(LMI, "INFO", "Arena", "Allocating memory with alignment");
    size_t offset = arena->offset;
    // logMessage(LMI, "INFO", "Arena", "Getting current offset: %zu", offset);
    size_t aligned_offset = (offset + (alignment - 1)) & ~(alignment - 1);
    // logMessage(LMI, "INFO", "Arena", "Calculating aligned offset: %zu", aligned_offset);
    size_t new_offset = aligned_offset + size;
    // logMessage(LMI, "INFO", "Arena", "Calculating new offset: %zu", new_offset);

    if (new_offset > arena->size)
    {
        // Attempt to grow the arena
        void *result = growArena(arena, size);
        assert(result != NULL);
        return result;
    }

    void *result = (char *)arena->base + aligned_offset;
    arena->offset = new_offset;
    arena->total_allocations++;

    logMessage(LMI, "INFO", "Arena", "Memory allocated with alignment");
    return result;
}

void *growArena(Arena *arena, size_t size)
{
    __STACK_FRAME__
    Arena *current = arena;
    while (current->next)
    {
        current = current->next;
    }

    size_t new_size = current->size * 2;
    Arena *new_arena = createArena(new_size, current->alignment);
    if (!new_arena)
    {
        return NULL;
    }

    current->next = new_arena;
    void *alloc = arenaAllocAligned(new_arena, size, current->alignment, __FILE__, __LINE__);
    assert(alloc != NULL);

    return alloc;
}

void *pushSize(Arena *arena, size_t size)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Arena", "Pushing size");
    size_t alignment = arena->alignment;
    size_t offset = arena->offset;
    size_t new_offset = (offset + size + (alignment - 1)) & ~(alignment - 1);
    if (new_offset > arena->size)
    {
        logMessage(LMI, "ERROR", "Arena", "Arena out of memory");
        return NULL;
    }
    void *result = (char *)arena->base + offset;
    arena->offset = new_offset;
    arena->total_allocations++;

    logMessage(LMI, "INFO", "Arena", "Size pushed");
    return result;
}

void resetArena(Arena *arena)
{
    __STACK_FRAME__
    printf("[INFO] { Arena } Resetting arena\n");
    arena->offset = 0;
    arena->free_list = NULL;
    arena->total_allocations = 0;
}

void clearArena(Arena *arena)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Arena", "Clearing arena");
    resetArena(arena);
    logMessage(LMI, "INFO", "Arena", "Arena cleared");
}

void freeArena(Arena *arena)
{
    __STACK_FRAME__
    logMessage(LMI, "INFO", "Arena", "Freeing arena");
    Arena *current = arena;
    while (current)
    {
        Arena *next = current->next;
        free(current->base);
        if (current != arena)
        {
            logMessage(LMI, "INFO", "Arena", "Freeing arena");
            free(current);
        }

        logMessage(LMI, "INFO", "Arena", "Arena freed");
        current = next;
    }

    logMessage(LMI, "INFO", "Arena", "Arena chain freed");
}

void arenaDebugPrint(Arena *arena)
{
    __STACK_FRAME__
    printf("\n================================================================================\n");
    printf("                                ARENA DEBUG INFO                                \n");
    printf("================================================================================\n\n");

    printf("Base Address: %p | Size: %zu bytes | Offset: %zu bytes | Alignment: %zu bytes\n",
           arena->base, arena->size, arena->offset, arena->alignment);
    printf("Total Allocations: %zu\n\n", arena->total_allocations);

    size_t bytes_used = arena->offset;
    size_t bytes_free = arena->size - arena->offset;
    printf("Bytes Used: %zu | Bytes Free: %zu\n", bytes_used, bytes_free);
    printf("Memory Usage: [.................................................] %.1f%%\n\n",
           (double)bytes_used / arena->size * 100);

    printf("Detailed Memory Map:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Address      | Size     | Padding  | Type            | Line   | File                \n");
    printf("--------------------------------------------------------------------------------\n");

    char *current = (char *)arena->base;
    size_t allocation_count = 0;
    while (current < (char *)arena->base + arena->offset)
    {
        AllocationHeader *header = (AllocationHeader *)current;
        if ((char *)header + sizeof(AllocationHeader) > (char *)arena->base + arena->size)
        {
            printf("[ERROR] { Arena } Invalid header detected at %p\n", header);
            break;
        }

        // Additional checks and logging
        if ((char *)header < (char *)arena->base || (char *)header >= (char *)arena->base + arena->size)
        {
            logMessage(LMI, "ERROR", "Arena", "Invalid header detected");
            break;
        }

        // Check if the header fields are valid
        if (header->size > arena->size || header->padding > arena->size)
        {
            logMessage(LMI, "ERROR", "Arena", "Invalid header fields detected");
            break;
        }

        logMessage(LMI, "DEBUG", "Arena", "Printing allocation information...");
        logMessage(LMI, "DEBUG", "Arena", "Size: %zu, Padding: %zu, Line: %d, File: %s",
                   header->size, header->padding, header->line, header->file);

        current += sizeof(AllocationHeader) + header->size + header->padding;
        allocation_count++;
        if (allocation_count % 5 == 0)
        {
            printf("%.*s\n", 80, "--------------------------------------------------------------------------------");
        }
    }

    // Memory pools information
    printf("\nMemory Pools:\n");
    printf("%.*s\n", 80, "--------------------------------------------------------------------------------");
    printf("%-15s | %-12s | %-12s | %-12s\n", "Pool", "Block Size", "Total Blocks", "Free Blocks");
    printf("%.*s\n", 80, "--------------------------------------------------------------------------------");
    printf("%-15s | %-12zu | %-12zu | %-12s\n", "Small Pool", arena->small_pool.block_size, arena->small_pool.num_blocks, "N/A");
    printf("%-15s | %-12zu | %-12zu | %-12s\n", "Medium Pool", arena->medium_pool.block_size, arena->medium_pool.num_blocks, "N/A");
    printf("%-15s | %-12zu | %-12zu | %-12s\n", "Large Pool", arena->large_pool.block_size, arena->large_pool.num_blocks, "N/A");
    printf("\n%.*s\n", 80, "================================================================================\n\n");
}

// ------------------------------
// Helper functions
char *string_repeat(const char *str, int times)
{
    __STACK_FRAME__
    size_t len = strlen(str);
    char *result = (char *)malloc(len * times + 1);
    for (int i = 0; i < times; i++)
    {
        memcpy(result + i * len, str, len);
    }
    result[len * times] = '\0';
    return result;
}

char *truncate_string(const char *str, size_t max_len)
{
    __STACK_FRAME__
    static char buffer[21]; // Max length + 1 for null terminator
    if (strlen(strdup(str)) <= max_len)
    {
        return strdup(str);
    }
    snprintf(buffer, sizeof(buffer), "%.*s...", (int)(max_len - 3), str);
    return buffer;
}

const char *get_allocation_type(size_t size)
{
    __STACK_FRAME__
    if (size <= 16)
        return "Small";
    if (size <= 32)
        return "Medium";
    if (size <= 64)
        return "Large";
    return "Custom";
}

void print_pool_info(const char *name, MemoryPool *pool)
{
    __STACK_FRAME__
    size_t free_blocks = 0;
    PoolBlock *current = pool->free_list;
    while (current)
    {
        free_blocks++;
        current = current->next;
    }
    printf("%-15s | %-12zu | %-12zu | %-12zu\n",
           name, pool->block_size, pool->num_blocks, free_blocks);
}
