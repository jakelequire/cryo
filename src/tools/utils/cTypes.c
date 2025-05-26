/********************************************************************************
 *  Copyright 2025 Jacob LeQuire                                                *
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
#include "diagnostics/diagnostics.h"

// Store every single `String *` in an array. This is used to either check for
// existing strings or to free all strings at the end of the program.
String **_stringArray = NULL;
size_t _stringArraySize = 0;
size_t _stringArrayCapacity = 0;

// =============================================================================
// General Utility Functions

bool cStringCompare(const char *str1, const char *str2)
{
    __STACK_FRAME__
    return strcmp(str1, str2) == 0;
}

// + ======================================================================================== + //
// +                             String Type & Functions                                      + //
// + ======================================================================================== + //

void String_init(struct String *self, char *str)
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    free(self->data);
    self->data = NULL;
    self->length = 0;
    self->capacity = 0;
}

void String_print(struct String *self)
{
    __STACK_FRAME__
    printf("%s\n", self->data);
}

const char *String_c_str(struct String *self)
{
    __STACK_FRAME__
    return self->data;
}

void String_clear(struct String *self)
{
    __STACK_FRAME__
    self->data[0] = '\0';
    self->length = 0;
}

void String_destroy(String *self)
{
    if (self->data)
    {
        free(self->data);
        self->data = NULL;
    }
    self->length = 0;
    self->capacity = 0;

    // Free the string itself
    free(self);
}

/// @brief This function will create the `String` structure and initialize its methods
/// @param
/// @return
String *_create_string_container(void)
{
    __STACK_FRAME__
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
    string->destroy = String_destroy;
    return string;
}

String *_create_alloca_string_container(void *(*allocator)(size_t))
{
    __STACK_FRAME__
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
    __STACK_FRAME__
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
    __STACK_FRAME__
    String *string = _create_string_container();
    String_init(string, "");
    return string;
}

String *createAllocaString(void *(*allocator)(size_t), char *str)
{
    __STACK_FRAME__
    String *string = _create_alloca_string_container(allocator);
    String_init(string, str);
    return string;
}

void freeString(String *string)
{
    __STACK_FRAME__
    String_free(string);
    free(string);
}

// + ======================================================================================== + //
// +                              File Types & Functions                                      + //
// + ======================================================================================== + //

void File_init(struct File *self, char *filename, char *mode)
{
    __STACK_FRAME__
    self->filename = filename;
    self->mode = mode;
    self->data = NULL;
    self->size = 0;
    self->capacity = 0;
}

void File_open(struct File *self)
{
    __STACK_FRAME__
    self->file = fopen(self->filename, self->mode);
    if (!self->file)
    {
        fprintf(stderr, "Error: Failed to open file: %s\n", self->filename);
        exit(EXIT_FAILURE);
    }
}

void File_close(struct File *self)
{
    __STACK_FRAME__
    fclose(self->file);
}

void File_read(struct File *self)
{
    __STACK_FRAME__
    fseek(self->file, 0, SEEK_END);
    self->size = ftell(self->file);
    rewind(self->file);
    self->capacity = self->size + 1;
    self->data = (char *)malloc(self->capacity);
    if (!self->data)
    {
        fprintf(stderr, "Error: Failed to allocate memory for file data\n");
        exit(EXIT_FAILURE);
    }
    fread(self->data, 1, self->size, self->file);
    self->data[self->size] = '\0';
}

void File_write(struct File *self)
{
    __STACK_FRAME__
    fwrite(self->data, 1, self->size, self->file);
}

void File_destroy(struct File *self)
{
    __STACK_FRAME__
    free(self->filename);
    free(self->mode);
    free(self->data);
    self->size = 0;
    self->capacity = 0;
    fclose(self->file);
}

// =============================================================================
// File Functions

File *createFile(char *filename, char *mode)
{
    __STACK_FRAME__
    File *file = (File *)malloc(sizeof(File));
    if (!file)
    {
        fprintf(stderr, "Error: Failed to allocate memory for file\n");
        exit(EXIT_FAILURE);
    }
    file->init = File_init;
    file->open = File_open;
    file->close = File_close;
    file->read = File_read;
    file->write = File_write;
    file->destroy = File_destroy;
    file->init(file, filename, mode);
    return file;
}

void freeFile(File *file)
{
    __STACK_FRAME__
    file->destroy(file);
    free(file);
}

// =============================================================================
