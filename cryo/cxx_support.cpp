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
#include "support_lib.h"

extern void *getCryoNullValue();

__EXTERN_C__ void __c_printInt(int value)
{
    printf("%d\n", value);
}

__EXTERN_C__ void __c_printIntPtr(int *value)
{
    printf("%d\n", *value);
}

__EXTERN_C__ void __c_printStr(char *value)
{
    printf("%s\n", (char *)value);
}

// __EXTERN_C__ void printStr(char *value)
// {
//     printf("%s\n", value);
// }

__EXTERN_C__ char *__c_intToString(int value)
{
    char *buffer = (char *)malloc(sizeof(char) * 1024);
    sprintf(buffer, "%d", value);
    return buffer;
}

__EXTERN_C__ int __c_strLen(const char *str)
{
    return strlen(str);
}

__EXTERN_C__ void __c_sys_exit(int code)
{
    exit(code);
}

// For the FS Class in Cryo
__EXTERN_C__ int __c_fs_mkdir(const char *path)
{
    int result = mkdir(path, 0777);
    return result;
}

__EXTERN_C__ int __c_fs_rmdir(const char *path)
{
    int result = rmdir(path);
    return result;
}

__EXTERN_C__ int __c_fs_rmfile(const char *path)
{
    int result = remove(path);
    return result;
}

__EXTERN_C__ void __c_fs_mvfile(const char *oldPath, const char *newPath)
{
    rename(oldPath, newPath);
}

__EXTERN_C__ int __c_fs_dirExists(const char *path)
{
    struct stat buffer;
    int exists = stat(path, &buffer);
    return exists == 0;
}

__EXTERN_C__ int __c_fs_fileExists(const char *path)
{
    struct stat buffer;
    int exists = stat(path, &buffer);
    return exists == 0;
}

__EXTERN_C__ char *__c_fs_readFile(const char *path, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (!file)
    {
        printf("Error: Failed to open file: %s\n", path);
        return nullptr;
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

__EXTERN_C__ int __c_fs_writeFile(const char *path, const char *data, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (!file)
    {
        printf("Error: Failed to open file: %s\n", path);
        return -1;
    }

    fwrite(data, 1, strlen(data), file);
    fclose(file);
}

__EXTERN_C__ char **__c_fs_listDir(const char *path)
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
        return nullptr;
    }
}

// =======================================================
// Low Level Operations

__EXTERN_C__ int __c_write(int fd, const void *buf, size_t count)
{
    return write(fd, buf, count);
}

/*
extern function __c_read(fd: int, buf: string, count: int) -> int;
extern function __c_open(path: string, flags: int, mode: int) -> int;
extern function __c_close(fd: int) -> int;
extern function __c_lseek(fd: int, offset: int, whence: int) -> int;
extern function __c_unlink(path: string) -> int;
extern function __c_fstat(fd: int, buf: string) -> int;
extern function __c_stat(path: string, buf: string) -> int;
extern function __c_mkdir(path: string, mode: int) -> int;
extern function __c_rmdir(path: string) -> int;
extern function __c_opendir(path: string) -> int;
*/

__EXTERN_C__ int __c_read(int fd, void *buf, size_t count)
{
    return read(fd, buf, count);
}

__EXTERN_C__ int __c_open(const char *path, int flags, int mode)
{
    return open(path, flags, mode);
}

__EXTERN_C__ int __c_close(int fd)
{
    return close(fd);
}

__EXTERN_C__ int __c_lseek(int fd, int offset, int whence)
{
    return lseek(fd, offset, whence);
}

__EXTERN_C__ int __c_unlink(const char *path)
{
    return unlink(path);
}

__EXTERN_C__ int __c_fstat(int fd, struct stat *buf)
{
    return fstat(fd, buf);
}

__EXTERN_C__ int __c_stat(const char *path, struct stat *buf)
{
    return stat(path, buf);
}

__EXTERN_C__ int __c_mkdir(const char *path, int mode)
{
    return mkdir(path, mode);
}

__EXTERN_C__ int __c_rmdir(const char *path)
{
    return rmdir(path);
}

