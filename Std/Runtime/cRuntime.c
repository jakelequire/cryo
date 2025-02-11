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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

#include "./c_support/print_support.c"

extern void *getCryoNullValue();

void __c_printInt(int value)
{
    printf("%d\n", value);
}

void __c_printIntPtr(int *value)
{
    printf("%d\n", *value);
}

void __c_printStr(char *value)
{
    printf("%s\n", (char *)value);
}

void __c_println(char *value)
{
    printf("%s", value);
}

char *__c_intToString(int value)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    sprintf(buffer, "%d", value);
    return buffer;
}

int __c_strLen(const char *str)
{
    return strlen(str);
}

void __c_sys_exit(int code)
{
    exit(code);
}

void __c_memset(void *ptr, int value, size_t num)
{
    memset(ptr, value, num);
}

void *__c_malloc(size_t size)
{
    printf("Allocating %ld bytes\n", size);
    void *ptr = (void *)malloc(size);
    return ptr;
}

void __c_free(void *ptr)
{
    free(ptr);
}

void __c_printAddr(void *ptr)
{
    printf("%p\n", ptr);
}

intptr_t __c_getAddr(void *ptr)
{
    return (intptr_t)ptr;
}

void __c_printIntAddr(int *ptr)
{
    printf("[DEBUG] IntAddr: \t%p\n", ptr);
}

void __c_printI64Hex(int64_t value)
{
    // Turn the `value` into a hex string `0x...`
    printf("0x%lx\n", value);
}

void __c_printPointer(void *ptr)
{
    printf("[DEBUG] printPtr: \t%p\n", ptr);
}

// ======================================================= //
//                      FS Operations                      //
// ======================================================= //

// For the FS Class in Cryo
int __c_fs_mkdir(const char *path)
{
    int result = mkdir(path, 0777);
    return result;
}

int __c_fs_rmdir(const char *path)
{
    int result = rmdir(path);
    return result;
}

int __c_fs_rmfile(const char *path)
{
    int result = remove(path);
    return result;
}

void __c_fs_mvfile(const char *oldPath, const char *newPath)
{
    rename(oldPath, newPath);
}

int __c_fs_dirExists(const char *path)
{
    struct stat buffer;
    int exists = stat(path, &buffer);
    return exists == 0;
}

int __c_fs_fileExists(const char *path)
{
    struct stat buffer;
    int exists = stat(path, &buffer);
    return exists == 0;
}

char *__c_fs_readFile(const char *path, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (!file)
    {
        printf("Error: Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);

    return buffer;
}

int __c_fs_writeFile(const char *path, const char *data, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (!file)
    {
        printf("Error: Failed to open file: %s\n", path);
        return -1;
    }

    fwrite(data, 1, strlen(data), file);
    fclose(file);

    return 0;
}

char **__c_fs_listDir(const char *path)
{
    DIR *dir;
    struct dirent *ent;
    int capacity = 1024;
    char **array = (char **)malloc(sizeof(char *) * capacity);
    int count = 0;

    if ((dir = opendir(path)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (count >= capacity)
            {
                capacity *= 2;
                array = (char **)realloc(array, sizeof(char *) * capacity);
            }
            array[count] = (char *)malloc(strlen(ent->d_name) + 1);
            strcpy(array[count], ent->d_name);
            count++;
        }
        closedir(dir);
    }
    else
    {
        perror("");
        return NULL;
    }

    return array;
}

// =======================================================
// Low Level Operations

int __c_write(int fd, const void *buf, size_t count)
{
    return write(fd, buf, count);
}

int __c_read(int fd, void *buf, size_t count)
{
    return read(fd, buf, count);
}

int __c_open(const char *path, int flags, int mode)
{
    return open(path, flags, mode);
}

int __c_close(int fd)
{
    return close(fd);
}

int __c_lseek(int fd, int offset, int whence)
{
    return lseek(fd, offset, whence);
}

int __c_unlink(const char *path)
{
    return unlink(path);
}

int __c_fstat(int fd, struct stat *buf)
{
    return fstat(fd, buf);
}

int __c_stat(const char *path, struct stat *buf)
{
    return stat(path, buf);
}

int __c_mkdir(const char *path, int mode)
{
    return mkdir(path, mode);
}

int __c_rmdir(const char *path)
{
    return rmdir(path);
}
