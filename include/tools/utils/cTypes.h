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
///
/// cTypes.h | #include "/tools/utils/cTypes.h"
///
/// This file contains utility and wrapper functions / types / macros for C.
/// It's just to make things a little easier to work with.
///
#ifndef C_TYPES_H
#define C_TYPES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

#include "tools/utils/cWrappers.h"

// =============================================================================
// General Utility Functions

bool cStringCompare(const char *str1, const char *str2);

// + ======================================================================================== + //
// +                             String Type & Functions                                      + //
// + ======================================================================================== + //

/// @brief A structure to represent a string in a safer way
typedef struct String
{
    // The string data & properties --------------------------------------------
    char *data;
    size_t length;
    size_t capacity;

    // Allocator properties -----------------------------------------------------

    bool using_custom_allocator;
    void *(*allocator)(size_t);

    // String Constructors -----------------------------------------------------

    /// `void (*init)(struct String *self, char *str)`
    ///
    /// @brief Initialize the string with a C-style string
    /// @param self The string to initialize
    /// @param str The C-style string to initialize the string with
    /// @note This is the default constructor
    _NEW_METHOD(void, init, struct String *self, char *str);

    /// `void (*alloc_init)(struct String *self, void *(*allocator)(size_t), char *str)`
    ///
    /// @brief Initialize the string with a custom allocator and a C-style string
    /// @param self The string to initialize
    /// @param allocator The custom allocator to use
    /// @param str The C-style string to initialize the string with
    _NEW_METHOD(void, alloc_init, struct String *self, void *(*allocator), char *str);

    // String Methods ----------------------------------------------------------

    /// `void append(struct String *self, char *str)`
    ///
    /// @brief Append a string to the end of the current string
    /// @param self The string to append to
    /// @param str The string to append
    _NEW_METHOD(void, append, struct String *self, char *str);

    /// `void print(struct String *self)`
    ///
    /// @brief Print the string to stdout
    /// @param self The string to print
    _NEW_METHOD(void, print, struct String *self);

    /// `char *c_str(struct String *self)`
    ///
    /// @brief Get the C-style string from the string
    /// @param self The string to get the C-style string from
    /// @return The C-style string
    _NEW_METHOD(const char *, c_str, struct String *self);

    /// `void clear(struct String *self)`
    ///
    /// @brief Clear the string data. Note: This does not free the string,
    ///        it just sets the length to 0 and the first character to '\0'.
    /// @param self The string to clear
    _NEW_METHOD(void, clear, struct String *self);

    /// `void free(struct String *self)`
    ///
    /// @brief Free the string data and set the string to NULL
    /// @param self The string to free
    _NEW_METHOD(void, free, struct String *self);
} String;

// --------------------------
// String Functions

/// @brief Create a new string object.
/// @param str
/// @return
String *createString(const char *str, ...);

/// @brief Create a new string object that is empty.
/// @return
String *createEmptyString(void);

/// @brief Create a new string object using a custom allocator.
/// @param allocator
/// @param str
/// @return
String *createAllocaString(void *(*allocator)(size_t), char *str);

void *foo();

// --------------------------
// String Macros

#define Str(...) createString(__VA_ARGS__)
#define StrAlloc(allocator, ...) createAllocaString(allocator, __VA_ARGS__)

// + ======================================================================================== + //
// +                             Array Types & Functions                                      + //
// + ======================================================================================== + //

#endif // C_TYPES_H
