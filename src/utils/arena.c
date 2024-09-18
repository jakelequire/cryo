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
#include "arena.h"

#define PRINT_WIDTH 80
#define MEMORY_BAR_WIDTH 60

Arena *createArena(size_t size, size_t alignment)
{
    Arena *arena = (Arena *)malloc(sizeof(Arena));
    if (!arena)
    {
        logMessage("ERROR", __LINE__, "Arena", "Failed to allocate memory for arena");
        return NULL;
    }
    initArena(arena, size, alignment);
    return arena;
}

void initMemoryPool(MemoryPool *pool, size_t block_size, size_t num_blocks)
{
    pool->block_size = block_size;
    pool->num_blocks = num_blocks;
    pool->free_list = NULL;

    logMessage("INFO", __LINE__, "MemoryPool", "Memory pool initialized");

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
    if (pool->free_list == NULL)
    {
        logMessage("ERROR", __LINE__, "MemoryPool", "Pool is empty");
        return NULL; // Pool is empty
    }
    PoolBlock *block = pool->free_list;
    pool->free_list = block->next;

    logMessage("INFO", __LINE__, "MemoryPool", "Block allocated");

    return block;
}

void poolFree(MemoryPool *pool, void *ptr)
{
    PoolBlock *block = (PoolBlock *)ptr;
    block->next = pool->free_list;
    pool->free_list = block;

    logMessage("INFO", __LINE__, "MemoryPool", "Block freed");
}

void initArena(Arena *arena, size_t size, size_t alignment)
{
    logMessage("INFO", __LINE__, "Arena", "Initializing arena");

    arena->base = aligned_alloc(alignment, size);
    if (!arena->base)
    {
        logMessage("ERROR", __LINE__, "Arena", "Failed to allocate memory for arena");
        return;
    }

    logMessage("INFO", __LINE__, "Arena", "Arena initialized");
    arena->size = size;
    arena->offset = 0;
    arena->alignment = alignment;
    arena->next = NULL;
    arena->free_list = NULL;
    arena->total_allocations = 0;
    logMessage("INFO", __LINE__, "Arena", "Arena initialization complete");

    initMemoryPool(&arena->small_pool, 16, 1024);
    initMemoryPool(&arena->medium_pool, 32, 512);
    initMemoryPool(&arena->large_pool, 64, 256);

    logMessage("INFO", __LINE__, "Arena", "Memory pools initialized");
}

void *debugArenaAlloc(Arena *arena, size_t size, const char *file, int line)
{
    logMessage("INFO", __LINE__, "Arena", "Allocating memory with debug information");
    size_t total_size = size + sizeof(AllocationHeader);
    void *raw_memory = arenaAllocAligned(arena, total_size, arena->alignment, __FILE__, __LINE__);
    logMessage("INFO", __LINE__, "Arena", "Memory allocated");
    if (!raw_memory)
    {
        logMessage("ERROR", __LINE__, "Arena", "Failed to allocate memory");
        return NULL;
    }

    AllocationHeader *header = (AllocationHeader *)raw_memory;
    header->size = size;
    header->padding = total_size - size - sizeof(AllocationHeader);
    header->line = line;
    strncpy(header->file, file, sizeof(header->file) - 1);
    header->file[sizeof(header->file) - 1] = '\0'; // Ensure null-termination

    logMessage("INFO", __LINE__, "Arena", "Debug information added to memory allocation");

    return (char *)raw_memory + sizeof(AllocationHeader);
}

void *arenaAllocAligned(Arena *arena, size_t size, size_t alignment, const char *file, int line)
{
    // logMessage("INFO", __LINE__, "Arena", "Allocating memory with alignment");
    size_t offset = arena->offset;
    // logMessage("INFO", __LINE__, "Arena", "Getting current offset: %zu", offset);
    size_t aligned_offset = (offset + (alignment - 1)) & ~(alignment - 1);
    // logMessage("INFO", __LINE__, "Arena", "Calculating aligned offset: %zu", aligned_offset);
    size_t new_offset = aligned_offset + size;
    // logMessage("INFO", __LINE__, "Arena", "Calculating new offset: %zu", new_offset);

    if (new_offset > arena->size)
    {
        logMessage("ERROR", __LINE__, "Arena", "Arena out of memory");
        logMessage("INFO", __LINE__, "Arena", "Attempting to grow arena");
        // Attempt to grow the arena
        void *result = growArena(arena, size);
        assert(result != NULL);

        logMessage("INFO", __LINE__, "Arena", "Growing arena successful");
        return result;
    }

    void *result = (char *)arena->base + aligned_offset;
    arena->offset = new_offset;
    arena->total_allocations++;

    // logMessage("INFO", __LINE__, "Arena", "Memory allocated with alignment");
    return result;
}

void *growArena(Arena *arena, size_t size)
{
    logMessage("INFO", __LINE__, "Arena", "Growing arena");
    Arena *current = arena;
    while (current->next)
    {
        current = current->next;
    }

    size_t new_size = current->size * 2;
    logMessage("INFO", __LINE__, "Arena", "Creating new arena with size %zu", new_size);
    Arena *new_arena = createArena(new_size, current->alignment);
    if (!new_arena)
    {
        logMessage("ERROR", __LINE__, "Arena", "Failed to create new arena");
        return NULL;
    }

    current->next = new_arena;
    void *alloc = arenaAllocAligned(new_arena, size, current->alignment, __FILE__, __LINE__);
    assert(alloc != NULL);

    logMessage("INFO", __LINE__, "Arena", "Arena grown successfully");
    return alloc;
}

void *pushSize(Arena *arena, size_t size)
{
    logMessage("INFO", __LINE__, "Arena", "Pushing size");
    size_t alignment = arena->alignment;
    size_t offset = arena->offset;
    size_t new_offset = (offset + size + (alignment - 1)) & ~(alignment - 1);
    if (new_offset > arena->size)
    {
        logMessage("ERROR", __LINE__, "Arena", "Arena out of memory");
        return NULL;
    }
    void *result = (char *)arena->base + offset;
    arena->offset = new_offset;
    arena->total_allocations++;

    logMessage("INFO", __LINE__, "Arena", "Size pushed");
    return result;
}

void resetArena(Arena *arena)
{
    printf("[INFO] { Arena } Resetting arena\n");
    arena->offset = 0;
    arena->free_list = NULL;
    arena->total_allocations = 0;
}

void clearArena(Arena *arena)
{
    logMessage("INFO", __LINE__, "Arena", "Clearing arena");
    resetArena(arena);
    logMessage("INFO", __LINE__, "Arena", "Arena cleared");
}

void freeArena(Arena *arena)
{
    logMessage("INFO", __LINE__, "Arena", "Freeing arena");
    Arena *current = arena;
    while (current)
    {
        Arena *next = current->next;
        free(current->base);
        if (current != arena)
        {
            logMessage("INFO", __LINE__, "Arena", "Freeing arena");
            free(current);
        }

        logMessage("INFO", __LINE__, "Arena", "Arena freed");
        current = next;
    }

    logMessage("INFO", __LINE__, "Arena", "Arena chain freed");
}

void arenaDebugPrint(Arena *arena)
{
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
            logMessage("ERROR", __LINE__, "Arena", "Invalid header detected");
            break;
        }

        // Check if the header fields are valid
        if (header->size > arena->size || header->padding > arena->size)
        {
            logMessage("ERROR", __LINE__, "Arena", "Invalid header fields detected");
            break;
        }

        logMessage("DEBUG", __LINE__, "Arena", "Printing allocation information...");
        logMessage("DEBUG", __LINE__, "Arena", "Size: %zu, Padding: %zu, Line: %d, File: %s",
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
    size_t len = strlen(str);
    char *result = malloc(len * times + 1);
    for (int i = 0; i < times; i++)
    {
        memcpy(result + i * len, str, len);
    }
    result[len * times] = '\0';
    return result;
}

char *truncate_string(const char *str, size_t max_len)
{
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
