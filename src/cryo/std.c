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

void builtInError(char *type)
{
    if (strcmp(type, "invalid") == 0)
    {
        fprintf(stderr, "Error: Invalid operation exception\n");
        exit(1);
    }
    else if (strcmp(type, "null") == 0)
    {
        fprintf(stderr, "Error: Null pointer exception\n");
        exit(1);
    }
    else if (strcmp(type, "indexoutofbounds") == 0)
    {
        fprintf(stderr, "Error: Index out of bounds exception\n");
        exit(1);
    }
    else if (strcmp(type, "invalidcast") == 0)
    {
        fprintf(stderr, "Error: Invalid cast exception\n");
        exit(1);
    }
    else if (strcmp(type, "invalidoperation") == 0)
    {
        fprintf(stderr, "Error: Invalid operation exception\n");
        exit(1);
    }
    else
    {
        printf("Error: Unknown exception\n");
    }
}

void fib(int n)
{
    int a = 0, b = 1, c;
    for (int i = 0; i < n; i++)
    {
        c = a + b;
        a = b;
        b = c;
        printf("%d\n", a);
    }
}