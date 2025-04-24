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

#define COLOR_LIGHT_GREEN "\033[1;32m"
#define COLOR_LIGHT_YELLOW "\033[1;33m"

#define BOLD "\033[1m"
#define COLOR_RESET "\033[0m"

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

// ======================================================== //
//               Memory Management Functions                //
// ======================================================== //

/**
 * Cross-platform memory mapping function
 */
void *cryo_mmap(int32_t size, int32_t flags, int32_t fd, int32_t offset)
{
    void *ptr;

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
        ptr = 0;
        return ptr;
    }

    ptr = MapViewOfFile(
        mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        offset,
        size);

    CloseHandle(mapping);
#else
    ptr = mmap(
        NULL,
        size,
        PROT_READ | PROT_WRITE,
        flags,
        fd,
        offset);

    if (ptr == MAP_FAILED)
    {
        ptr = 0;
        return ptr;
    }
#endif

    return ptr;
}

/**
 * Cross-platform memory unmapping function
 */
int32_t cryo_munmap(void *ptr, int32_t size)
{
    int32_t result;

#ifdef _WIN32
    result = UnmapViewOfFile((void *)ptr.val) ? 0 : -1;
#else
    result = munmap((void *)ptr, size);
#endif
    return result;
}

/**
 * Memory allocation wrapper
 */
void *cryo_malloc(int32_t size)
{
    void *mem_ptr = malloc(size);
    if (mem_ptr == NULL)
    {
        void *null_ptr = {0};
        return null_ptr;
    }

    return mem_ptr;
}

/**
 * Memory deallocation wrapper
 */
int32_t cryo_free(void *ptr)
{
    free((void *)ptr);

    return 0;
}

/**
 * Memory copy wrapper
 */
void *cryo_memcpy(void *dest, void *src, int32_t size)
{
    memcpy((void *)dest, (void *)src, size);
    return dest;
}

/**
 * Memory move wrapper
 */
void *cryo_memmove(void *dest, void *src, int32_t size)
{
    memmove((void *)dest, (void *)src, size);
    return dest;
}

/**
 * Memory set wrapper
 */
void *cryo_memset(void *dest, int32_t value, int32_t size)
{
    memset((void *)dest, value, size);
    return dest;
}

/**
 * Memory compare wrapper
 */
int32_t cryo_memcmp(void *ptr1, void *ptr2, int32_t size)
{
    return memcmp((void *)ptr1, (void *)ptr2, size);
}

void sys_exit(int32_t status)
{
    exit(status);
}

int32_t sys_read(int32_t fd, void *buf, int32_t count)
{
    if (fd < 0 || buf == 0 || count <= 0)
    {
        return -1;
    }
#ifdef _WIN32
    DWORD bytesRead;
    BOOL result = ReadFile((HANDLE)fd, (void *)buf, count, &bytesRead, NULL);
    return result ? bytesRead : -1;
#else
    return read(fd, (void *)buf, count);
#endif
}

int32_t sys_write(int32_t fd, void *buf, int32_t count)
{
    if (fd < 0 || buf == 0 || count <= 0)
    {
        return -1;
    }
#ifdef _WIN32
    DWORD bytesWritten;
    BOOL result = WriteFile((HANDLE)fd, (void *)buf, count, &bytesWritten, NULL);
    return result ? bytesWritten : -1;
#else
    return write(fd, (void *)buf, count);
#endif
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
void *mmap_export(int32_t size, int32_t flags, int32_t fd, int32_t offset)
{
    return cryo_mmap(size, flags, fd, offset);
}

int32_t munmap_export(void *ptr, int32_t size)
{
    return cryo_munmap(ptr, size);
}

void *malloc_export(int32_t size)
{
    return cryo_malloc(size);
}

int32_t free_export(void *ptr)
{
    return cryo_free(ptr);
}

void *memcpy_export(void *dest, void *src, int32_t size)
{
    return cryo_memcpy(dest, src, size);
}

void *memmove_export(void *dest, void *src, int32_t size)
{
    return cryo_memmove(dest, src, size);
}

void *memset_export(void *dest, int32_t value, int32_t size)
{
    return cryo_memset(dest, value, size);
}

int32_t memcmp_export(void *ptr1, void *ptr2, int32_t size)
{
    return cryo_memcmp(ptr1, ptr2, size);
}

int32_t strlen_export(cryo_string_t str)
{
    if (str.val == NULL)
    {
        return -1;
    }
    return strlen(str.val);
}

int32_t strcmp_export(cryo_string_t str1, cryo_string_t str2)
{
    if (str1.val == NULL || str2.val == NULL)
    {
        return -1;
    }
    return strcmp(str1.val, str2.val);
}

int32_t strncmp_export(cryo_string_t str1, cryo_string_t str2, int32_t n)
{
    if (str1.val == NULL || str2.val == NULL)
    {
        return -1;
    }
    return strncmp(str1.val, str2.val, n);
}

cryo_string_t strcpy_export(cryo_string_t dest, cryo_string_t src)
{
    if (dest.val == NULL || src.val == NULL)
    {
        return dest;
    }
    strcpy(dest.val, src.val);
    return dest;
}

int32_t printf_export(const char *format, ...)
{
    va_list args;
    int32_t result;

    va_start(args, format);
    result = vprintf(format, args);
    va_end(args);

    printf(BOLD COLOR_LIGHT_YELLOW "addr: %p%s\n", format, COLOR_RESET);

    return result;
}

int32_t scanf_export(cryo_string_t format, ...)
{
    va_list args;
    int32_t result;

    va_start(args, format);
    result = scanf(format.val, args);
    va_end(args);

    return result;
}

int32_t printI32_export(int32_t value)
{
    int32_t result = (int32_t)printf("%d\n", value);

    return result;
}
void printString_export(cryo_string_t str)
{
    if (str.val != NULL)
    {
        printf("%s\n", str.val);
    }
    else
    {
        printf("NULL\n");
    }

    printf(BOLD COLOR_LIGHT_YELLOW "addr: %p%s\n", str.val, COLOR_RESET);
}

void printStr_export(char *str)
{
    if (str != NULL)
    {
        printf("%s\n", str);
    }
    else
    {
        printf("NULL\n");
    }

    printf(BOLD COLOR_LIGHT_YELLOW "addr: %p%s\n", str, COLOR_RESET);
}

char *c_readFile_export(char *filename)
{
    printf("[C::DEBUG] Reading File Executed");
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);

    printf("[C::DEBUG] File read successfully: %s\n", filename);
    printf("[C::DEBUG] File Contents: %s\n", buffer);
    return buffer;
}