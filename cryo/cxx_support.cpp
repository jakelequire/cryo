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

// __EXTERN_C__ char *intToString(int value)
// {
//     char *buffer = (char *)malloc(sizeof(char) * 1024);
//     sprintf(buffer, "%d", value);
//     return buffer;
// }

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

__EXTERN_C__ char *__c_fs_readFile(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0';

    return buffer;
}
