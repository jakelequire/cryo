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
/**
 * runtime.c - Core runtime for the Cryo programming language
 *
 * This file provides the minimal C implementation needed to bootstrap
 * the Cryo language. It implements the external functions declared in
 * core.cryo and provides the essential runtime support.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

// ======================================================== //
//                 Type Definitions                         //
// ======================================================== //

// Cryo String structure - matches the one defined in core.cryo
typedef struct
{
    char *val;
    int32_t length;
    int32_t capacity;
    bool isEmpty;
    bool isNull;
} cryo_string_t;

// Cryo Int structure - matches the one defined in core.cryo
typedef struct
{
    int32_t val;
    bool isZero;
    bool isNegative;
} cryo_int_t;

// Cryo Pointer structure - matches the one defined in core.cryo
typedef struct
{
    int64_t val;
    cryo_string_t addr_string;
} cryo_pointer_t;

// ======================================================== //
//               Memory Management Functions                //
// ======================================================== //

/**
 * Cross-platform memory mapping function
 */
cryo_pointer_t cryo_mmap(int32_t size, int32_t flags, int32_t fd, int32_t offset)
{
    cryo_pointer_t ptr;
    void *mem_ptr;

#ifdef _WIN32
    HANDLE file_handle = INVALID_HANDLE_VALUE;
    if (fd != -1)
    {
        file_handle = (HANDLE)_get_osfhandle(fd);
    }

    HANDLE mapping = CreateFileMapping(
        file_handle,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        NULL);

    if (mapping == NULL)
    {
        ptr.val = 0;
        return ptr;
    }

    mem_ptr = MapViewOfFile(
        mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        offset,
        size);

    CloseHandle(mapping);
#else
    mem_ptr = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        flags,
        fd,
        offset);

    if (mem_ptr == MAP_FAILED)
    {
        ptr.val = 0;
        return ptr;
    }
#endif

    ptr.val = (int64_t)mem_ptr;

    // Create address string
    char addr_buf[32];
    snprintf(addr_buf, sizeof(addr_buf), "0x%llx", (unsigned long long)ptr.val);

    ptr.addr_string.val = strdup(addr_buf);
    ptr.addr_string.length = strlen(addr_buf);
    ptr.addr_string.capacity = ptr.addr_string.length + 1;
    ptr.addr_string.isEmpty = (ptr.addr_string.length == 0);
    ptr.addr_string.isNull = (ptr.addr_string.val == NULL);

    return ptr;
}

/**
 * Cross-platform memory unmapping function
 */
int32_t cryo_munmap(cryo_pointer_t ptr, int32_t size)
{
    int32_t result;

    if (ptr.val == 0)
    {
        return -1;
    }

#ifdef _WIN32
    result = UnmapViewOfFile((void *)ptr.val) ? 0 : -1;
#else
    result = munmap((void *)ptr.val, size);
#endif

    // Free the address string
    if (ptr.addr_string.val != NULL)
    {
        free(ptr.addr_string.val);
    }

    return result;
}

/**
 * Memory allocation wrapper
 */
cryo_pointer_t cryo_malloc(int32_t size)
{
    cryo_pointer_t ptr;
    void *mem_ptr = malloc(size);

    ptr.val = (int64_t)mem_ptr;

    // Create address string
    char addr_buf[32];
    snprintf(addr_buf, sizeof(addr_buf), "0x%llx", (unsigned long long)ptr.val);

    ptr.addr_string.val = strdup(addr_buf);
    ptr.addr_string.length = strlen(addr_buf);
    ptr.addr_string.capacity = ptr.addr_string.length + 1;
    ptr.addr_string.isEmpty = (ptr.addr_string.length == 0);
    ptr.addr_string.isNull = (ptr.addr_string.val == NULL);

    return ptr;
}

/**
 * Memory deallocation wrapper
 */
int32_t cryo_free(cryo_pointer_t ptr)
{
    if (ptr.val == 0)
    {
        return -1;
    }

    free((void *)ptr.val);

    // Free the address string
    if (ptr.addr_string.val != NULL)
    {
        free(ptr.addr_string.val);
    }

    return 0;
}

/**
 * Memory copy wrapper
 */
cryo_pointer_t cryo_memcpy(cryo_pointer_t dest, cryo_pointer_t src, int32_t size)
{
    if (dest.val == 0 || src.val == 0)
    {
        cryo_pointer_t null_ptr = {0};
        return null_ptr;
    }

    memcpy((void *)dest.val, (void *)src.val, size);
    return dest;
}

/**
 * Memory move wrapper
 */
cryo_pointer_t cryo_memmove(cryo_pointer_t dest, cryo_pointer_t src, int32_t size)
{
    if (dest.val == 0 || src.val == 0)
    {
        cryo_pointer_t null_ptr = {0};
        return null_ptr;
    }

    memmove((void *)dest.val, (void *)src.val, size);
    return dest;
}

/**
 * Memory set wrapper
 */
cryo_pointer_t cryo_memset(cryo_pointer_t dest, int32_t value, int32_t size)
{
    if (dest.val == 0)
    {
        cryo_pointer_t null_ptr = {0};
        return null_ptr;
    }

    memset((void *)dest.val, value, size);
    return dest;
}

/**
 * Memory compare wrapper
 */
int32_t cryo_memcmp(cryo_pointer_t ptr1, cryo_pointer_t ptr2, int32_t size)
{
    if (ptr1.val == 0 || ptr2.val == 0)
    {
        return -1;
    }

    return memcmp((void *)ptr1.val, (void *)ptr2.val, size);
}

// ======================================================== //
//              Cryo String Implementation                  //
// ======================================================== //

/**
 * Create a new Cryo string from a C string
 */
cryo_string_t cryo_string_create(const char *str)
{
    cryo_string_t result;

    if (str == NULL)
    {
        result.val = NULL;
        result.length = 0;
        result.capacity = 0;
        result.isEmpty = true;
        result.isNull = true;
        return result;
    }

    int32_t len = strlen(str);
    int32_t cap = len + 1;

    result.val = (char *)malloc(cap);

    if (result.val == NULL)
    {
        result.length = 0;
        result.capacity = 0;
        result.isEmpty = true;
        result.isNull = true;
        return result;
    }

    memcpy(result.val, str, len);
    result.val[len] = '\0';
    result.length = len;
    result.capacity = cap;
    result.isEmpty = (len == 0);
    result.isNull = false;

    return result;
}

// ======================================================== //
//                      Runtime Init                        //
// ======================================================== //

/**
 * Initialize the Cryo runtime
 */
void cryo_runtime_init()
{
    // Perform any necessary runtime initialization
}

/**
 * Cleanup the Cryo runtime
 */
void cryo_runtime_cleanup()
{
    // Perform any necessary runtime cleanup
}

// ======================================================== //
//                   External Function Exports              //
// ======================================================== //

// Memory Management Functions
cryo_pointer_t mmap_export(int32_t size, int32_t flags, int32_t fd, int32_t offset)
{
    return cryo_mmap(size, flags, fd, offset);
}

int32_t munmap_export(cryo_pointer_t ptr, int32_t size)
{
    return cryo_munmap(ptr, size);
}

cryo_pointer_t malloc_export(int32_t size)
{
    return cryo_malloc(size);
}

int32_t free_export(cryo_pointer_t ptr)
{
    return cryo_free(ptr);
}

cryo_pointer_t memcpy_export(cryo_pointer_t dest, cryo_pointer_t src, int32_t size)
{
    return cryo_memcpy(dest, src, size);
}

cryo_pointer_t memmove_export(cryo_pointer_t dest, cryo_pointer_t src, int32_t size)
{
    return cryo_memmove(dest, src, size);
}

cryo_pointer_t memset_export(cryo_pointer_t dest, int32_t value, int32_t size)
{
    return cryo_memset(dest, value, size);
}

int32_t memcmp_export(cryo_pointer_t ptr1, cryo_pointer_t ptr2, int32_t size)
{
    return cryo_memcmp(ptr1, ptr2, size);
}

int32_t strlen_export(cryo_pointer_t str_ptr)
{
    return cryo_strlen(str_ptr);
}

int32_t strnlen_export(cryo_pointer_t str_ptr, int32_t maxlen)
{
    return cryo_strnlen(str_ptr, maxlen);
}

int32_t strncmp_export(cryo_pointer_t str1_ptr, cryo_pointer_t str2_ptr, int32_t maxlen)
{
    return cryo_strncmp(str1_ptr, str2_ptr, maxlen);
}

int32_t printf_export(cryo_string_t format, ...)
{
    va_list args;
    int32_t result;

    va_start(args, format);
    result = cryo_printf(format, args);
    va_end(args);

    return result;
}

int32_t scanf_export(cryo_string_t format, ...)
{
    va_list args;
    int32_t result;

    va_start(args, format);
    result = cryo_scanf(format, args);
    va_end(args);

    return result;
}
