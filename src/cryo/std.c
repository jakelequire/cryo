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
#include "stdint.h"

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

// Enum to represent different types
typedef enum
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_POINTER,
    TYPE_UNKNOWN
} ValueType;

// Union to hold different types
typedef union
{
    int64_t i;
    float f;
    double d;
    int b;
    const char *s;
    void *p;
} Value;

// Struct to represent Any type
typedef struct
{
    ValueType type;
    Value value;
} Any;

void type_of(Any *any)
{
    switch (any->type)
    {
    case TYPE_INT:
        printf("Type: int, Value: %lld\n", any->value.i);
        break;
    case TYPE_FLOAT:
        printf("Type: float, Value: %f\n", any->value.f);
        break;
    case TYPE_DOUBLE:
        printf("Type: double, Value: %f\n", any->value.d);
        break;
    case TYPE_BOOL:
        printf("Type: bool, Value: %s\n", any->value.b ? "true" : "false");
        break;
    case TYPE_STRING:
        printf("Type: string, Value: %s\n", any->value.s);
        break;
    case TYPE_POINTER:
        printf("Type: pointer, Address: %p\n", any->value.p);
        break;
    default:
        printf("Type: unknown\n");
    }
}

// Helper functions to create Any objects
Any create_int_any(int64_t value)
{
    Any any = {TYPE_INT, {.i = value}};
    return any;
}

Any create_float_any(float value)
{
    Any any = {TYPE_FLOAT, {.f = value}};
    return any;
}

Any create_double_any(double value)
{
    Any any = {TYPE_DOUBLE, {.d = value}};
    return any;
}

Any create_bool_any(int value)
{
    Any any = {TYPE_BOOL, {.b = value}};
    return any;
}

Any create_string_any(const char *value)
{
    Any any = {TYPE_STRING, {.s = value}};
    return any;
}

Any create_pointer_any(void *value)
{
    Any any = {TYPE_POINTER, {.p = value}};
    return any;
}
