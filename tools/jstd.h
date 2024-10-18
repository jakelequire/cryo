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
#ifndef JSTD_H
#define JSTD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum FSPathOptions
{
    PATH_DEFAULT = 0,
    PATH_REL_PATH = 1,
    PATH_ABS_PATH = 2,
} FSPathOptions;

#define DEFAULT_PATH_OPTIONS PATH_DEFAULT
#define RELATIVE_PATH_OPTIONS PATH_REL_PATH
#define ABSOLUTE_PATH_OPTIONS PATH_ABS_PATH

// This structure holds all of the functions in the jStd library
typedef struct jStd
{
    // File System Functions
    // <jstd_readFile>
    /// @brief Takes in a file path and reads the contents of the file into a buffer
    char *(*readFile)(const char *path, FSPathOptions options);

    // <jstd_getCWD>
    /// @brief Gets the current working directory
    char *(*getCWD)(void);

    // <jstd_getFileExtension>
    /// @brief Gets the file extension of a file
    char *(*getFileExtension)(const char *path);

} jStd;

// >==========---------------------------------------------==========< //
// >==========------------ Function Prototypes ------------==========< //
// >==========---------------------------------------------==========< //

/**
 *  @brief Takes in a file path and reads
 * the contents of the file into a buffer
 * @param path The path to the file
 * @param options The path options to use
 *
 * Options:
 *
 * - PATH_DEFAULT: The path is the default path
 *
 * - PATH_REL_PATH: The path is relative to the current working directory
 *
 * - PATH_ABS_PATH: The path is an absolute path
 *
 * @return The contents of the file
 */
char *jstd_readFile(const char *path, FSPathOptions options);

/**
 *  @brief Checks if a file exists at the given path
 * @param void
 * @return True if the file exists, false otherwise
 */
char *jstd_getCWD(void);

/**
 *  @brief Gets the file extension of a file
 * @param path The path to the file
 * @return The file extension
 */
char *jstd_getFileExtension(const char *path);

#endif // JSTD_H