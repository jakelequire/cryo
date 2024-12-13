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
__EXTERN_C__ void __c_fs_mkdir(const char *path)
{
    mkdir(path, 0777);
}

__EXTERN_C__ void __c_fs_rmdir(const char *path)
{
    rmdir(path);
}

__EXTERN_C__ void __c_fs_rmfile(const char *path)
{
    remove(path);
}

__EXTERN_C__ void __c_fs_mvfile(const char *oldPath, const char *newPath)
{
    rename(oldPath, newPath);
}

__EXTERN_C__ bool __c_fs_dirExists(const char *path)
{
    struct stat statbuf;
    return stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode);
}

__EXTERN_C__ char *__c_fs_readFile(const char *path, const char *mode)
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
