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
#include "std.h"

void printInt(int value)
{
    printf("%d\n", value);
}

void printIntPtr(int *value)
{
    printf("%d\n", *value);
}

void printStr(char *value)
{
    printf("%s\n", value);
}

int strLength(char *str)
{
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

void sys_exit(int code)
{
    exit(code);
}

int sizeOf(void *ptr)
{
    return sizeof(ptr);
}

#define typename(x) _Generic((x), /* Get the name of a type */ \
    _Bool: "_Bool",                                            \
    unsigned char: "unsigned char",                            \
    char: "char",                                              \
    signed char: "signed char",                                \
    short int: "short int",                                    \
    unsigned short int: "unsigned short int",                  \
    int: "int",                                                \
    unsigned int: "unsigned int",                              \
    long int: "long int",                                      \
    unsigned long int: "unsigned long int",                    \
    long long int: "long long int",                            \
    unsigned long long int: "unsigned long long int",          \
    float: "float",                                            \
    double: "double",                                          \
    long double: "long double",                                \
    char *: "pointer to char",                                 \
    void *: "pointer to void",                                 \
    int *: "pointer to int",                                   \
    const char *: puts,                                        \
    const char[sizeof(x)]: puts,                               \
    default: "other")

void cryoTypeToCType(void *cryoPtr)
{
}

void type_of(void *ptr)
{
    // Get the memory block of the pointer
    printf("%s\n", typename(*ptr));
}
