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
#include "tools/utils/cTypes.h"

// =============================================================================
// General Utility Functions

bool cStringCompare(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

// =============================================================================
// String Type & Functions

void String_init(struct String *self, char *str)
{
    self->length = strlen(str);
    self->capacity = self->length + 1;
    self->data = (char *)malloc(self->capacity);
    if (!self->data)
    {
        fprintf(stderr, "Error: Failed to allocate memory for string\n");
        exit(EXIT_FAILURE);
    }
    strcpy(self->data, str);
}

void String_alloc_init(struct String *self, void *(*allocator)(size_t), char *str)
{
    self->length = strlen(str);
    self->capacity = self->length + 1;
    self->data = (char *)allocator(self->capacity);
    if (!self->data)
    {
        fprintf(stderr, "Error: Failed to allocate memory for string\n");
        exit(EXIT_FAILURE);
    }
    strcpy(self->data, str);
}

void String_append(struct String *self, char *str)
{
    size_t newLength = self->length + strlen(str);
    if (newLength >= self->capacity)
    {
        self->capacity = newLength + 1;
        self->data = (char *)realloc(self->data, self->capacity);
        if (!self->data)
        {
            fprintf(stderr, "Error: Failed to reallocate memory for string\n");
            exit(EXIT_FAILURE);
        }
    }
    strcat(self->data, str);
    self->length = newLength;
}

void String_free(struct String *self)
{
    free(self->data);
    self->data = NULL;
    self->length = 0;
    self->capacity = 0;
}

void String_print(struct String *self)
{
    printf("%s\n", self->data);
}

const char *String_c_str(struct String *self)
{
    return self->data;
}

void String_clear(struct String *self)
{
    self->data[0] = '\0';
    self->length = 0;
}

/// @brief This function will create the `String` structure and initialize its methods
/// @param
/// @return
String *_create_string_container(void)
{
    String *string = (String *)malloc(sizeof(String));
    if (!string)
    {
        fprintf(stderr, "Error: Failed to allocate memory for string\n");
        exit(EXIT_FAILURE);
    }
    string->init = String_init;
    string->append = String_append;
    string->print = String_print;
    string->c_str = String_c_str;
    string->clear = String_clear;
    string->free = String_free;
    return string;
}

String *_create_alloca_string_container(void *(*allocator)(size_t))
{
    String *string = (String *)allocator(sizeof(String));
    if (!string)
    {
        fprintf(stderr, "Error: Failed to allocate memory for string\n");
        exit(EXIT_FAILURE);
    }
    string->init = String_init;
    string->append = String_append;
    string->print = String_print;
    string->c_str = String_c_str;
    string->clear = String_clear;
    string->free = String_free;
    return string;
}

// =============================================================================
// String Functions

String *createString(const char *str, ...)
{
    String *string = _create_string_container();
    va_list args, args_copy;
    va_start(args, str);
    va_copy(args_copy, args);

    // Get required buffer size
    int size = vsnprintf(NULL, 0, str, args) + 1;
    char *buffer = (char *)malloc(size);
    if (!buffer)
    {
        fprintf(stderr, "Error: Failed to allocate memory for string buffer\n");
        exit(EXIT_FAILURE);
    }

    // Actually format the string
    vsnprintf(buffer, size, str, args_copy);
    String_init(string, buffer);

    free(buffer);
    va_end(args_copy);
    va_end(args);
    return string;
}

String *createEmptyString(void)
{
    String *string = _create_string_container();
    String_init(string, "");
    return string;
}

String *createAllocaString(void *(*allocator)(size_t), char *str)
{
    String *string = _create_alloca_string_container(allocator);
    String_init(string, str);
    return string;
}

void freeString(String *string)
{
    String_free(string);
    free(string);
}
